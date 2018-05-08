/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 *
 * GitHub repository: https://github.com/stbowers/Alcubierre
 */
#include <engine.h>
#include <stdlib.h>
#include <locale.h>
#include <AlcubierreGame.h>
#include <string.h>

/* The regular getch function is not thread safe, so this
 * function makes sure the proper locks are heald by this
 * thread before calling getch()
 */
int getch_safe(Engine* engine);

int main(int argc, char* argv[]){
	/* Block for attaching debugger or resizing window before running code */
    #ifndef __LINUX__
	getchar();
    #endif

    /* Set locale for proper ncurses use */
    setlocale(LC_CTYPE, "");

    /* Initialize engine */
    // Run in a 256x72 window (~16x9 with chars that are twice as tall as they are wide)
    Engine* engine = initializeEngine(256, 72);
    
    /* Write some debug output to stdscr, and a pause for debugging before starting the game */
    wprintw(engine->stdscr, "DEBUG INFO:\n");
    wprintw(engine->stdscr, "Term supports %d colors\n", COLORS);
    wprintw(engine->stdscr, "Term supports %d color pairs\n", COLOR_PAIRS);
    wprintw(engine->stdscr, "Term can change color: %s\n", (can_change_color())?"true":"false");
    wprintw(engine->stdscr, "Terminal Size: %dx%d\n", COLS, LINES);

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    wprintw(engine->stdscr, "Testing Color - blue with black background...\n");
    attroff(COLOR_PAIR(1));
    
    printw("Press any key to start...\n");
    wgetch(engine->stdscr); // block on debug stuff until key is pressed

    wclear(engine->stdscr);

    /* Send engine ready for rendering signal to start rendering */
    lockThreadLock(&engine->renderThreadData.dataLock);
    broadcastConditionSignal(&engine->renderThreadData.engineRenderReady);
    unlockThreadLock(&engine->renderThreadData.dataLock);

    /* Run the game */
    // if the program is run with --skipintro, skip the intro sequence (speeds up debugging the actual game)
    // if the program is run with --unlockfps, set MS_PER_FRAME to 0, which makes the game render at it's maximum possible fps
    bool skipIntro = false;
    bool unlockFPS = false;

    for (int i = 1; i < argc; i++){
        // loop through args
        if ((strncmp(argv[i], "--skipintro", 11) == 0)){
            skipIntro = true;
        } else if ((strncmp(argv[i], "--unlockfps", 11) == 0)){
            unlockFPS = true;
        }
    }

    // set MS_PER_FRAME if unlockFPS is true
    if (unlockFPS){
        MS_PER_FRAME = 0;
    }

    // call to startGame in AlcubierreGame.c
    startGame(engine, skipIntro);
    
    /* Main thread is done - now loop getting keyboard input and sending
     * keyboard events to the engine. If F1 is pressed, quit
     */
    timeout(0); // don't block on getch()
    int input;
    int lastUpdate = getTimems();
    while ((input = getch_safe(engine)) != KEY_F(1)){
        lockThreadLock(&gameStateLock);
        if (gameState.exit){
            break;
        }
        unlockThreadLock(&gameStateLock);
        // create keyboard input event if key pressed
        if (input != ERR){
            // the memory is managed by the event thread after we send the event, so no need to free the event here
            Event* keyEvent = (Event*)malloc(sizeof(Event));
            keyEvent->eventType.mask = 0;
            keyEvent->eventType.values.keyboardEvent = TRUE;
            keyEvent->eventData = (void*)(uintptr_t)input; // we don't want to send a pointer in this case - just the char data which will fit into the size of a void pointer
			keyEvent->next = NULL;

            // send events
            engine->handleEvent(engine, keyEvent);
        }

        // create a timer event 
        Event* timeEvent = (Event*) malloc(sizeof(Event));
        timeEvent->eventType.mask = 0;
        timeEvent->eventType.values.timerEvent = true;
        timeEvent->eventData = (void*)(uintptr_t)(getTimems() - lastUpdate); // data for time event is the time in ms since the last timer event
        lastUpdate = getTimems();
        timeEvent->next = NULL;

        // send event
        engine->handleEvent(engine, timeEvent);

        // Sleep so we don't use too much cpu for the main thread
        sleepms(10);
    }

    /* Exit after cleaning up the engine */
    destroyEngine(engine);

    return 0;
}

// Thread safe getch()
int getch_safe(Engine* engine){
    lockThreadLock(&engine->renderThreadData.drawLock);
    int ch = wgetch(engine->stdscr);
    unlockThreadLock(&engine->renderThreadData.drawLock);
    return ch;
}
