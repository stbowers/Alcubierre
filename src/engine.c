/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Provides implementation for miscellaneous functionality in engine.h */
// define required for unicode ncurses support
#include <engine.h>
#ifdef __UNIX__
#include <unistd.h>
#elif __WIN32__
#include <windows.h>
#endif
#include <threads.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

int MS_PER_FRAME = 10; // how many milliseconds corrospond to one frame - framerate

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, Event* event);

/* Thread functions */
int eventThreadFunction(void* data);
int renderThreadFunction(void* data);
int drawingThreadFunction(void* data);
int renderTimerThreadFunction(void* data);

/* engine.h implementations */
/* initializes ncurses, and returns a struct with
 * all information needed to run the engine
 */
Engine* initializeEngine(int width, int height){
    /* Create new engine - freed by destroyEngine() method */
    Engine* newEngine = (Engine*) malloc(sizeof(Engine));
    newEngine->width = width;
    newEngine->height = height;

    /* Initialize ncurses */
    newEngine->stdscr = initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    /* Initialize rng */
    srand(time(NULL));

    // Check the size of the terminal window is large enough for a widthxheight window with 1 wide border
    if (!((COLS > (width + 2)) && (LINES > (height + 2)))){
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
    
    /* Set up screen buffers */
    newEngine->stdscrWidth  = COLS;
    newEngine->stdscrHeight = LINES;
    newEngine->stdscrBufferSize = newEngine->stdscrWidth * newEngine->stdscrHeight * sizeof(CursesChar);
    newEngine->stdscrBuffer1 = (CursesChar*) malloc(newEngine->stdscrBufferSize);
    newEngine->stdscrBuffer2 = (CursesChar*) malloc(newEngine->stdscrBufferSize);
    newEngine->backgroundBuffer = (CursesChar*) malloc(newEngine->stdscrBufferSize);

    /* Fill background buffer */
    // default background char: space with black bg and white fg color
    for (int x = 0; x < newEngine->stdscrWidth; x++){
        for (int y = 0; y < newEngine->stdscrHeight; y++){
            CursesChar* currentChar = &newEngine->backgroundBuffer[(newEngine->stdscrHeight * x) + y];
            currentChar->attributes = 0;
            // clear char array
            currentChar->character = L' ';
        }
    }

    /* Initialize buffers with background buffer data */
    memcpy(newEngine->stdscrBuffer1, newEngine->backgroundBuffer, newEngine->stdscrBufferSize);
    memcpy(newEngine->stdscrBuffer2, newEngine->backgroundBuffer, newEngine->stdscrBufferSize);

    /* Create the main window */
    int start_x = (int)((COLS - width) / 2.0f);
    int start_y = (int)((LINES - height) / 2.0f);
    // create two wider & two higher, and move up & left one for border
    newEngine->mainPanel = createPanel(width, height, start_x, start_y, 0);
    newEngine->activePanel = newEngine->mainPanel;

    /* Set engine functions */
    newEngine->handleEvent = defaultEngineHandleEvent;

    /* Set up threads */
    /* Set up event thread */
    // Locks
    createLock(&newEngine->eventThreadData.dataLock);

    // Condition variables
    createConditionVariable(&newEngine->eventThreadData.eventQueueChanged);

    // Initialize shared resources
    newEngine->eventThreadData.queuedEvents = NULL;
    newEngine->eventThreadData.queueEnd = &newEngine->eventThreadData.queuedEvents;
    newEngine->eventThreadData.exit = false;

    // Start thread
    createThread(&newEngine->eventThread, (ThreadProcess_t)eventThreadFunction, newEngine);

    /* Set up render thread */
    // Barriers
    createBarrier(&newEngine->renderThreadData.renderDrawBarrier, 2);
    createBarrier(&newEngine->renderThreadData.timerBarrier, 3);

    // Locks
    createLock(&newEngine->renderThreadData.dataLock);
    createLock(&newEngine->renderThreadData.renderLock);
    createLock(&newEngine->renderThreadData.drawLock);

    // Conditions
    createConditionVariable(&newEngine->renderThreadData.engineRenderReady);

    // Initialize shared resources
    newEngine->renderThreadData.exit = false;
    newEngine->renderThreadData.fps_calculated = 0.0f;
    newEngine->renderThreadData.framesRendered = 0;
	newEngine->renderThreadData.renderBuffer = &newEngine->stdscrBuffer1;
	newEngine->renderThreadData.drawingBuffer = &newEngine->stdscrBuffer2;

    // Start threads
    createThread(&newEngine->renderThread, (ThreadProcess_t)renderThreadFunction, newEngine);
    createThread(&newEngine->drawingThread, (ThreadProcess_t)drawingThreadFunction, newEngine);
    createThread(&newEngine->renderTimerThread, (ThreadProcess_t)renderTimerThreadFunction, newEngine);
    
    /* Return a copy of the new engine struct */
    return newEngine;
}

void destroyEngine(Engine* engine){
	/* Tell threads to exit */
	// event thread
	lockThreadLock(&engine->eventThreadData.dataLock);
	engine->eventThreadData.exit = true;
	// we also need to signal the event thread, as it may be waiting for a signal that would otherwise never come
	sendConditionSignal(&engine->eventThreadData.eventQueueChanged);
	unlockThreadLock(&engine->eventThreadData.dataLock);

	// render thread
	lockThreadLock(&engine->renderThreadData.dataLock);
	engine->renderThreadData.exit = true;
	unlockThreadLock(&engine->renderThreadData.dataLock);

	/* Join threads */
	joinThread(&engine->eventThread);
	// The render thread joins the draw thread and render timer thread, so we only need to join the base render thread
	joinThread(&engine->renderThread);

    /* Free any memory we control */
	// we own the memory for mainPanel, but not it's children. destroying a panel doesn't free it's children
	// so if a thread calls destroyEngine they should take care of any objects they've added to mainPanel first.
    destroyPanel(engine->mainPanel);

    free(engine);

    /* End ncurses mode */
    endwin();
}

// center object in panel
void centerObject(Object* toCenter, Panel* parent, int objectWidth, int objectHeight){
    allignObjectX(toCenter, parent, objectWidth, .5);
    allignObjectY(toCenter, parent, objectHeight, .5);
}

// center an object horizontally
void allignObjectX(Object* toAllign, Panel* parent, int objectWidth, float position){
    int newX = ((float)(parent->width - objectWidth) * position);

    toAllign->x = newX;
}

// center an object vertically
void allignObjectY(Object* toAllign, Panel* parent, int objectHeight, float position){
    int newY = ((float)(parent->height - objectHeight) * position);

    toAllign->y = newY;
}

// Used by moveRelativeTo to get the absolute coordinates
void getAbsolutePosition(Object* parent, int relX, int relY, int* absX, int* absY){
    /* add relX and relY to the given object's x and y */
    int newX = parent->x + relX;
    int newY = parent->y + relY;

    /* If the given object has a parent, we need to convert 
     * the new coordinates relative to the parent
     */
    if (parent->parent != NULL){
		getAbsolutePosition(parent->parent, newX, newY, absX, absY); 
    } else {
        // if given object doen't have a parent, we can assume newX and newY are absolute
        *absX = newX;
        *absY = newY;
    }
}

// writes a wchar with attributes to the buffer
void writewcharToBuffer(CursesChar* buffer, int x, int y, attr_t attr, wchar_t wch){
    /* Call writecharToBuffer with a new CursesChar struct */
    CursesChar ch;
    ch.attributes = attr;
    ch.character = wch;
    writecharToBuffer(buffer, x, y, &ch);
}

// Writes a CursesChar to buffer
void writecharToBuffer(CursesChar* buffer, int x, int y, CursesChar* ch){
    /* Get CursesChar at (x,y) */
    // Assumes buffer points somewhere inside a stdscr buffer
    CursesChar* charAt = &buffer[(LINES * x) + y];

    /* Set CursesChar - copy not change address */
    *charAt = *ch;
}

// printf to buffer
int bufferPrintf(CursesChar* buffer, int width, int height, int maxHeight, int x, int y, unsigned int attr, const char* format, ...){
    /* Get variadic args */
    va_list args;
    va_start(args, format);

    /* Create string */
    // maximum length of string is width*height (allocate +1 for null terminator)
    char* str = (char*) malloc(sizeof(char) * ((width * height) + 1));
    vsnprintf(str, width * height, format, args);

    /* Draw string to buffer */
    int deltaX = 0;
    int deltaY = 0;
    for (int i = 0; i <= width*height; i++){
        if (!str[i] || (deltaY >= maxHeight)){
            // if we've reached a null byte or printed maxHeight lines we're done
            free(str);
            return deltaY;
        } else if (str[i] == '\n') {
            // move to next line
            deltaX = 0;
            deltaY++;
        } else {
            // print char, advance x by 1, if x == width go to next line
            CursesChar* charAt = &buffer[((height) * (x + deltaX)) + (y + deltaY)];
            charAt->attributes = attr;
            charAt->character = str[i];

            deltaX++;
            if (deltaX == width){
                deltaX = 0;
                deltaY++;
            }
        }
    }

	return deltaY;
}

/* Gets a timestamp in milliseconds, from a monotonic clock.
 * The time from which the returned timestamp is counting up
 * from is undefined, but it will be correct relative to
 * earlier/later values received from this function.
 */
uint64_t getTimems(){
    #ifdef __UNIX__
        /* UNIX-like systems */
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return (time.tv_sec * 1.0e3) + (uint64_t)((double)time.tv_nsec / 1.0e6f);
    #elif __WIN32__
		return GetTickCount64();
    #endif
}

/* Waits for a given number of milliseconds
 */
void sleepms(int msec){
    #ifdef __UNIX__
        /* UNIX-like systems */
        usleep(msec * 1000);
    #elif __WIN32__
		/* Windows sleep */
		Sleep(msec);
	#endif
}

/* Color helper functions */
/* Searches through the available terminal colors for the one that is closest
 * to the given rgb value (by euclidian distance), or if the terminal supports
 * changing colors start changing colors past 16 (standard colors)
 */
int nextColor = 16;
int getBestColor(int r, int g, int b, Engine* engine){
    int bestColor = 0;
    float bestr2 = 2e6; // max distance in color space is ~1.96e5, so all colors should be closer than this initial value
    
    // number of colors to search through (full range if can't change colors, else only the colors that have been set)
    int numColors = (can_change_color())?nextColor:COLORS;

    /* Loop through avaliable terminal colors */
    for (int color = 0; color < numColors; color++){
        /* Get rgb value of terminal color */
        short tr, tg, tb;
        color_content(color, &tr, &tg, &tb);

        /* Normalize terminal color */
        /* color_content returns values between 0 and 1000,
         * so normalize them to be between 0 and 255
         */
        tr = tr / (3.9f);
        tg = tg / (3.9f);
        tb = tb / (3.9f);

        /* Get the square of the euclidian distance
         * r^2 = (x^2 + y^2 + z^2) 
         */

        float r2 = pow((tr -r), 2) + pow((tg - g), 2) + pow((tb - b), 2);

        /* Compare to bestr2 */
        if (r2 < bestr2){
            bestColor = color;
            bestr2 = r2;
        }
    }
    
    if (bestr2 < 150 || !can_change_color()){
        // if we found a close enough match, or can't change colors return that
        return bestColor;
    }

    // else change the next color and return that
    // We need to have the drawing mutex before calling init_color, because init_color sends control characters to the terminal
    lockThreadLock(&engine->renderThreadData.drawLock);
    init_color(nextColor, r*3.9, g*3.9, b*3.9);
    unlockThreadLock(&engine->renderThreadData.drawLock);
    nextColor++;
    return nextColor - 1;
}

/* Search for an existing color pair with the given colors, and if
 * one isn't found make a new one at nextColorPair
 */
int nextColorPair = 1;
int getColorPair(int fg, int bg, Engine* engine){
    for (int pair = 0; pair < nextColorPair; pair++){
        /* Get colors in pair */
        short pfg, pbg;
        pair_content(pair, &pfg, &pbg);

        /* If they match, return */
        if (pfg == fg && pbg == bg){
            return pair;
        }
    }

    /* If we leave loop, no matching pair was found.
     * Create a new one at nextColorPair and increment
     * nextColorPair
     */
    // We need the drawing mutex to use init_pair, since it sends control characters to the terminal
    lockThreadLock(&engine->renderThreadData.drawLock);
    init_pair(nextColorPair, fg, bg);
    unlockThreadLock(&engine->renderThreadData.drawLock);
    nextColorPair++;
    return nextColorPair - 1;
}

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, Event* event){
    /* Lock the event thread's data lock */
    lockThreadLock(&self->eventThreadData.dataLock);

    /* Add event to the end of the event queue, and update the end pointer */
    *self->eventThreadData.queueEnd = event;
    self->eventThreadData.queueEnd = &event->next;

    /* Send new event signal and unlock mutex */
    sendConditionSignal(&self->eventThreadData.eventQueueChanged);
    unlockThreadLock(&self->eventThreadData.dataLock);
}

/* Thread functions */
/* Handles the dealing of events sent to the engine. When the engine
 * gets an event (from any thread) it will put the event in the queue
 * and send this thread a signal. This way sending an event to the
 * engine won't block the sending thread while the event is dispatched
 */
int eventThreadFunction(void* data){
    // data passed to the thread is a pointer to the engine
    Engine* engine = (Engine*)data;

    // continuously run a loop checking for events
    while (true){
        /* Lock the event thread data lock */
        lockThreadLock(&engine->eventThreadData.dataLock);

        /* Process events if there are any, else wait for new events */
        if (engine->eventThreadData.queuedEvents != NULL){
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
        } else {
            waitForConditionSignal(&engine->eventThreadData.eventQueueChanged, &engine->eventThreadData.dataLock);
        }
        
        /* Check if we should exit */
        if (engine->eventThreadData.exit){
            /* Clean up resources */
			if (engine->eventThreadData.queuedEvents != NULL) {
				// If any new events were added to queue, free the memory
				Event* current = engine->eventThreadData.queuedEvents;
				while (current != NULL) {
					Event* next = current->next;
					free(current);
					current = next;
				}
			}

			unlockThreadLock(&engine->eventThreadData.dataLock);

            /* Exit */
            exitThread(0);
        }

        /* Unlock mutex */
        unlockThreadLock(&engine->eventThreadData.dataLock);
    }
}

// Renders the contents of the engine to the screen on a regular interval
int renderThreadFunction(void* data){
    // The data passed to this function should be a pointer to the engine
    Engine* engine = (Engine*)data;
    uint64_t lastUpdate = getTimems();

    /* Wait for render ready signal */
    lockThreadLock(&engine->renderThreadData.dataLock);
    waitForConditionSignal(&engine->renderThreadData.engineRenderReady, &engine->renderThreadData.dataLock);
    unlockThreadLock(&engine->renderThreadData.dataLock);
    
    /* Keep looping until exitThread() is called */
    while (true){
        /* Get the render lock */
        lockThreadLock(&engine->renderThreadData.renderLock);

        /* Render */
        // Clear the buffer by copying the background buffer to it
        memcpy(*engine->renderThreadData.renderBuffer, engine->backgroundBuffer, engine->stdscrBufferSize);

        // Render the main panel
        CursesChar* bufferAtMainPanel = &(*engine->renderThreadData.renderBuffer)[(LINES * engine->mainPanel->objectProperties.x) + engine->mainPanel->objectProperties.y];
        ((Object*)engine->mainPanel)->drawObject((Object*)engine->mainPanel, bufferAtMainPanel);
        
        /* Sync with the draw thread */
        enterThreadBarrier(&engine->renderThreadData.renderDrawBarrier);

        /* Get the draw lock - so we hold both draw and render locks */
        lockThreadLock(&engine->renderThreadData.drawLock);

        /* Swap buffers */
        CursesChar** tmp = engine->renderThreadData.renderBuffer;
        engine->renderThreadData.renderBuffer = engine->renderThreadData.drawingBuffer;
        engine->renderThreadData.drawingBuffer = tmp;

        /* Release draw and render locks */
        unlockThreadLock(&engine->renderThreadData.renderLock);
        unlockThreadLock(&engine->renderThreadData.drawLock);

        /* Sync with the timer and draw thread */
        enterThreadBarrier(&engine->renderThreadData.timerBarrier);

        /* Get data lock */
        lockThreadLock(&engine->renderThreadData.dataLock);

        /* Update data */
        // increment render count
        engine->renderThreadData.framesRendered++;

        // every 50 frames update the fps
        if (!(engine->renderThreadData.framesRendered % 50)){
            uint64_t msPassed = getTimems() - lastUpdate;
            lastUpdate = getTimems();

            // calculate fps
            // 50 frames   | 1000 ms |  = (50 * 1000) / msPassed fps
            // msPassed ms |   1 s   |
            engine->renderThreadData.fps_calculated = (50.0f*1000.0f) / (float)(msPassed);
        }

        /* Check if we should exit */
        if (engine->renderThreadData.exit){
            /* Clean up */
            unlockThreadLock(&engine->renderThreadData.dataLock);

			/* Join draw thread & timer thread */
			joinThread(&engine->drawingThread);
			joinThread(&engine->renderTimerThread);

            /* Exit */
            exitThread(0);
        }

        /* Release data lock */
        unlockThreadLock(&engine->renderThreadData.dataLock);
    }
}

int drawingThreadFunction(void* data){
    Engine* engine = (Engine*)data;

    /* Wait for render ready signal */
    lockThreadLock(&engine->renderThreadData.dataLock);
    waitForConditionSignal(&engine->renderThreadData.engineRenderReady, &engine->renderThreadData.dataLock);
    unlockThreadLock(&engine->renderThreadData.dataLock);

    /* Keep looping until exitThread() is called */
    while (true){
        /* Get drawing lock */
        lockThreadLock(&engine->renderThreadData.drawLock);

        /* Draw to screen */
        // move to top left and start adding chars from buffer
        wmove(engine->stdscr, 0, 0);
        for (int y = 0; y < engine->stdscrHeight; y++){
            for (int x = 0; x < engine->stdscrWidth; x++){
                CursesChar* currentChar = &(*engine->renderThreadData.drawingBuffer)[(engine->stdscrHeight * x) + y];
                #ifdef __WIN32__
				cchar_t pdcursesChar = currentChar->character | currentChar->attributes;
                wadd_wch(engine->stdscr, &pdcursesChar);
                #elif __UNIX__
                cchar_t ncursesChar;
                ncursesChar.attr = currentChar->attributes;
                ncursesChar.chars[0] = currentChar->character;
                ncursesChar.chars[1] = 0;
                wadd_wch(engine->stdscr, &ncursesChar);
                #endif
            }
        }

        // Print debug info at top left
        wmove(engine->stdscr, 0,0);
		lockThreadLock(&engine->renderThreadData.dataLock);
        wprintw(engine->stdscr, "FPS: %.2f", engine->renderThreadData.fps_calculated);
		unlockThreadLock(&engine->renderThreadData.dataLock);

        wrefresh(engine->stdscr);

        /* Release draw lock */
        unlockThreadLock(&engine->renderThreadData.drawLock);

        /* Sync with render thread */
        enterThreadBarrier(&engine->renderThreadData.renderDrawBarrier);

        /* Sync with timer & render threads */
        enterThreadBarrier(&engine->renderThreadData.timerBarrier);

        /* Get data lock */
        lockThreadLock(&engine->renderThreadData.dataLock);

        /* Check if we should exit */
        if (engine->renderThreadData.exit){
            /* Clean up */
			unlockThreadLock(&engine->renderThreadData.dataLock);

            /* Exit */
            exitThread(0);
        }

        /* Release data lock */
        unlockThreadLock(&engine->renderThreadData.dataLock);
    }
}

// Keeps track of time for the render loop
int renderTimerThreadFunction(void* data){
    // This thread is passed a pointer to the engine
    Engine* engine = (Engine*)data;

    /* Wait for render ready signal */
    lockThreadLock(&engine->renderThreadData.dataLock);
    waitForConditionSignal(&engine->renderThreadData.engineRenderReady, &engine->renderThreadData.dataLock);
    unlockThreadLock(&engine->renderThreadData.dataLock);

    /* Keep looping until exitThread() is called */
    while (true){
        /* Wait for MS_PER_FRAME */
        sleepms(MS_PER_FRAME);

        /* Sync with render & draw threads */
        enterThreadBarrier(&engine->renderThreadData.timerBarrier);

        /* Get data lock */
        lockThreadLock(&engine->renderThreadData.dataLock);

        /* Check if we should exit */
        if (engine->renderThreadData.exit){
            /* Clean up */
			unlockThreadLock(&engine->renderThreadData.dataLock);

            /* Exit */
            exitThread(0);
        }

        /* Release data lock */
        unlockThreadLock(&engine->renderThreadData.dataLock);
    }
}
