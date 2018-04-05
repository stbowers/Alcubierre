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

/* Main panel functions */
void mainPanelRefresh(Panel* self);
void mainPanelClear(Panel* self);

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, const Event* event);

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
    return (time.tv_sec * 1.0e3) + (time.tv_nsec / 1.0e6);
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

    /* Refresh window */
    wrefresh(self->window);
}

void mainPanelClear(Panel* self){
    /* Clear window */
    wclear(self->window);
}

/* Engine functions */
void defaultEngineHandleEvent(Engine* self, const Event* event){
    // Default functionality is to simply pass the event to the active window
    self->activePanel->objectProperties.handleEvent((Object*)self->activePanel, event);
}
