/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Provides implementation for miscellaneous functionality in engine.h */
#include <time.h>
#include <ncurses.h>
#include <stdlib.h>
#include <engine.h>
#include <pthread.h>

#define MS_PER_TICK 10 // how many milliseconds corrospond to one tick - tickrate (10ms/tick ~= 100 ticks per second)
// There is a weird bug with the render thread timer where it runs at exactly half the framerate expected from MS_PER_FRAME - so the value below should be halved
#define MS_PER_FRAME 5 // how many milliseconds corrospond to one frame - framerate

/* Main panel functions */
void mainPanelRefresh(Panel* self);
void mainPanelClear(Panel* self);

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, Event* event);

/* Thread functions */
void* eventThreadFunction(void* data);
void* renderThreadFunction(void* data);
void* renderTimerThreadFunction(void* data);

/* engine.h implementations */
/* initializes ncurses, and returns a struct with
 * all information needed to run the engine
 */
Engine* initializeEngine(int width, int height){
    /* Create new engine - freed by destroyEngine() method */
    Engine* newEngine = (Engine*) malloc(sizeof(Engine));

    /* Initialize ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    /* The refresh is used here because otherwise ncurses tends to pull
     * stdscr above our actual screen whenever getch() is called, thus
     * hiding anything we've drawn.
     */
    refresh();

    // Check the size of the terminal window
    if (!((COLS > width) && (LINES > height))){
        /* We don't have enough space.
         * Exit for now, a potential solution in the
         * future might be to attempt to open a new
         * terminal window to run the game in
         */
        endwin();
        free(newEngine);
        printf("This terminal window is not big enough to start the engine.\nA size of %dx%d was requested, but only %dx%d is available.\n",
                width, height, COLS, LINES);

        // Technically this exit is bad form, but it doesn't do any harm for now.
        exit(0);
    }
    
    /* Create stdscr panel */
    newEngine->stdPanel = new_panel(stdscr);

    /* Create the main window */
    int start_x = (int)((COLS - width) / 2.0f);
    int start_y = (int)((LINES - height) / 2.0f);
    newEngine->mainPanel = createPanel(width, height, start_x, start_y, 0);
    newEngine->activePanel = newEngine->mainPanel;

    /* Redefine main window functions */
    newEngine->mainPanel->clearPanel = mainPanelClear;
    newEngine->mainPanel->refreshPanel = mainPanelRefresh;
    
    /* Set engine functions */
    newEngine->handleEvent = defaultEngineHandleEvent;

    /* Set up pthreads */
    // Event thread
    pthread_create(&newEngine->eventThread, NULL, eventThreadFunction, newEngine);
    pthread_mutex_init(&newEngine->eventThreadData.dataMutex, NULL);
    pthread_cond_init(&newEngine->eventThreadData.newEventSignal, NULL);
    newEngine->eventThreadData.queuedEvents = NULL;
    newEngine->eventThreadData.queueEnd = &newEngine->eventThreadData.queuedEvents;
    newEngine->eventThreadData.runEventThread = true;

    // Render thread
    pthread_create(&newEngine->renderThread, NULL, renderThreadFunction, newEngine);
    pthread_mutex_init(&newEngine->renderThreadData.dataMutex, NULL);
    newEngine->renderThreadData.render = false;
    pthread_cond_init(&newEngine->renderThreadData.renderSignal, NULL);
    pthread_mutex_init(&newEngine->renderThreadData.timerMutex, NULL);
    pthread_cond_init(&newEngine->renderThreadData.timerSignal, NULL);

    // Global thread data
    pthread_mutex_init(&newEngine->globalThreadData.dataMutex, NULL);
    pthread_cond_init(&newEngine->globalThreadData.exitSignal, NULL);
    newEngine->globalThreadData.isRunning = true;

    /* Return a copy of the new engine struct */
    return newEngine;
}

void destroyEngine(Engine* engine){
    free(engine->mainPanel);
    endwin();
    free(engine);
}

/* Gets a timestamp in milliseconds, from a monotonic clock.
 * The time from which the returned timestamp is counting up
 * from is undefined, but it will be correct relative to
 * earlier/later values received from this function.
 */
uint64_t getTimems(){
    /* UNIX-like systems */
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (time.tv_sec * 1.0e3) + (uint64_t)((double)time.tv_nsec / 1.0e6f);
}

/* Waits for a given number of milliseconds
 */
#ifdef __UNIX__
    #include <unistd.h>
#elif __WIN32__
#endif
void sleepms(int msec){
    #ifdef __UNIX__
        /* UNIX-like systems */
        usleep(msec * 1000);
    #endif
}

/* Main window functions */
void mainPanelRefresh(Panel* self){
    /* Draw border first */
    box(self->window, 0, 0);

    /* Refresh panel */
    update_panels();
}

void mainPanelClear(Panel* self){
    /* Clear window */
    werase(self->window);
}

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, Event* event){
    /* Lock the event thread's data mutex */
    pthread_mutex_lock(&self->eventThreadData.dataMutex);

    /* Add event to the end of the event queue, and update the end pointer */
    *self->eventThreadData.queueEnd = event;
    self->eventThreadData.queueEnd = &event->next;

    /* Send new event signal and unlock mutex */
    pthread_cond_signal(&self->eventThreadData.newEventSignal);
    pthread_mutex_unlock(&self->eventThreadData.dataMutex);
}

/* Thread functions */
/* Handles the dealing of events sent to the engine. When the engine
 * gets an event (from any thread) it will put the event in the queue
 * and send this thread a signal. This way sending an event to the
 * engine won't block the sending thread while the event is dispatched
 */
void* eventThreadFunction(void* data){
    // data passed to the thread is a pointer to the engine
    Engine* engine = (Engine*)data;

    // continuously run a loop checking for events
    while (true){
        /* Lock the event thread data mutex */
        pthread_mutex_lock(&engine->eventThreadData.dataMutex);

        /* Check if we should exit */
        if (!engine->eventThreadData.runEventThread){
            // Clean up and exit thread
            pthread_exit(0);
        }

        /* Wait for new events */
        while (engine->eventThreadData.queuedEvents == NULL){
            // If no events are queued up wait for a signal, and then check again
            // this also frees the data mutex, allowing other threads to write to eventThreadData
            pthread_cond_wait(&engine->eventThreadData.newEventSignal, &engine->eventThreadData.dataMutex);
        }

        /* Process events */
        // move through event queue
        Event* current = engine->eventThreadData.queuedEvents;
        Event* previous;
        while (current != NULL){
            // send event to the active panel
            engine->activePanel->objectProperties.handleEvent((Object*)engine->activePanel, current);

            // move up list
            previous = current;
            current = current->next;

            // free the node that was just handled. see events.h for more details on how the memory for events should be handled
            free(previous);
        }

        /* Clear event queue */
        engine->eventThreadData.queuedEvents = NULL;
        engine->eventThreadData.queueEnd = &engine->eventThreadData.queuedEvents;

        /* Unlock mutex */
        pthread_mutex_unlock(&engine->eventThreadData.dataMutex);
    }
}

// Renders the contents of the engine to the screen on a regular interval
// timerExit should only be modified if the thread holds the timer mutex.
// if timerExit is true when the timer thread gets the timer mutex, the
// timer thread will exit
bool timerExit = false;
int timesSlept = 0;
void* renderThreadFunction(void* data){
    // The data passed to this function should be a pointer to the engine
    Engine* engine = (Engine*)data;
    
    /* Set up the timer thread */
    // We hold the timer mutex by default, and release it when waiting for timer signals
    pthread_mutex_lock(&engine->renderThreadData.timerMutex);
    
    // create timer thread
    pthread_t timerThread;
    pthread_create(&timerThread, NULL, renderTimerThreadFunction, engine);

    /* The render thread doesn't exit normally, instead pthread_exit(status)
     * is called when this thread is finished. The render thread will check
     * isRunning in the global thread data struct to determine if it should
     * finish its work. It only checks this when render is false in it's own
     * data structure, so it is good practice to stop the render thread by
     * setting render to false before setting isRunning to false and
     * shutting down the engine.
     */
    while (true){
        /* Run the render loop while we should still be rendering. We also need to lock the data mutex on each loop */
        // pthread_mutex_lock returns 0 (false) on a successful lock
        int mutex_status;
        uint64_t startTime = getTimems();
        uint64_t lastUpdate = startTime;
        int renders = 0;
        while ((!(mutex_status = pthread_mutex_lock(&engine->renderThreadData.dataMutex))) && engine->renderThreadData.render){
            /* Release render thread data mutex while rendering, since we don't need those resources right now */
            pthread_mutex_unlock(&engine->renderThreadData.dataMutex);
            
            /* Wait for timer ready signal */
            pthread_cond_wait(&engine->renderThreadData.timerSignal, &engine->renderThreadData.timerMutex);

            /* Do render */
            // Calculate fps if 1 second has passed since last update
            if (getTimems() - lastUpdate > 1000){
                lastUpdate = getTimems();
                pthread_mutex_lock(&engine->renderThreadData.dataMutex);
                engine->renderThreadData.fps_calculated = renders;
                pthread_mutex_unlock(&engine->renderThreadData.dataMutex);
                mvwprintw(stdscr, 6, 10, "FPS: %3d  ", renders);
                timesSlept = 0;
                renders = 0;
                engine->mainPanel->clearPanel(engine->mainPanel);
            }

            // For now print some debug info instead of actually rendering.
            //mvwprintw(stdscr, 10, 10, "FPS: %d", engine->renderThreadData.fps_calculated);
            //mvwprintw(stdscr, 11, 10, "Frame: %04d", renders);
            //mvwprintw(stdscr, 12, 10, "Time: %06dms", (getTimems() - startTime));
            //mvwprintw(stdscr, 13, 10, "MS_PER_FRAME: %dms", MS_PER_FRAME * 2);
            //mvwprintw(stdscr, 14, 10, "Times Slept this render: %d", timesSlept);
            //timesSlept = 0;
            //engine->mainPanel->clearPanel(engine->mainPanel);
            renders++;
            //sleepms(100);
            ((Object*)engine->mainPanel)->drawObject((Object*)engine->mainPanel, engine->mainPanel);

            /* Wait for timer signal to finish render and update screen */
            pthread_cond_wait(&engine->renderThreadData.timerSignal, &engine->renderThreadData.timerMutex);

            engine->mainPanel->refreshPanel(engine->mainPanel);
            doupdate();
        }
        
        /* If control reaches this part of the code we either have the data mutex and render is false,
         * or there was an error when getting the data mutex. We check the mutex_status variable for
         * an error
         */
        if (mutex_status != 0){
            // There was an error getting the mutex. This is a bug so crash and print the error to the console for debugging
            endwin();
            printf("Error when obtaining the data mutex for the render thread: %d\n", mutex_status);
            exit(-1);
        }

        /* render is false and we hold data mutex if control reaches this point */
        /* Check if we should be exiting */
        // get the global mutex
        pthread_mutex_lock(&engine->globalThreadData.dataMutex);

        // check for exit status
        if (!(engine->globalThreadData.isRunning)){
            /* We should exit.
             * Send an exit signal to the timer thread, join it,
             * and then exit.
             */
            // set timerExit to true and release the timer mutex - this will cause the timer thread to exit
            timerExit = true;
            pthread_mutex_unlock(&engine->renderThreadData.timerMutex);
            
            // join timer thread
            void* timerReturn;
            pthread_join(timerThread, &timerReturn);

            // exit
            pthread_exit(0);
        }

        /* If we should continue, wait for renderSignal and then release the data mutex */
        pthread_cond_wait(&engine->renderThreadData.renderSignal, &engine->renderThreadData.dataMutex);
        pthread_mutex_unlock(&engine->renderThreadData.dataMutex);
    }
}

// Keeps track of time for the render loop
void* renderTimerThreadFunction(void* data){
    // This thread is passed a pointer to the engine
    Engine* engine = (Engine*)data;

    while (true){
        /* Get timer mutex, check if we should exit, and if
         * not send ready signal to render thread
         */
        pthread_mutex_lock(&engine->renderThreadData.timerMutex);

        /* If timerExit is true when we get the mutex, exit */
        if (timerExit){
            pthread_exit(0);
        }

        /* Send timer ready signal */
        pthread_cond_signal(&engine->renderThreadData.timerSignal);
        pthread_mutex_unlock(&engine->renderThreadData.timerMutex);

        /* Wait for MS_PER_FRAME */
        sleepms(MS_PER_FRAME);
        timesSlept++;

        /* Get timer mutex and send timer done signal */
        pthread_mutex_lock(&engine->renderThreadData.timerMutex); // This will block until the render thread is ready for signal - if a render takes longer than MS_PER_FRAME
        pthread_cond_signal(&engine->renderThreadData.timerSignal);
        pthread_mutex_unlock(&engine->renderThreadData.timerMutex);
    }
}
