/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <stdlib.h>
#include <locale.h>
#include <engine.h>
#include <AlcubierreGame.h>

int main(){
    /* Set locale for proper ncurses use */
    setlocale(LC_CTYPE, "");

    /* Initialize engine */
    // Our game runs in a 128x72 (16:9) window
    Engine* engine = initializeEngine(128, 72);
    
    /* Write some debug output to stdscr, and a pause for debugging before starting the game */
    printw("Term supports %d colors\n", COLORS);
    printw("Term supports %d color pairs\n", COLOR_PAIRS);
    printw("Term can change color: %s\n", (can_change_color())?"true":"false");
    printw("Terminal Size: %dx%d\n", COLS, LINES);

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    printw("Testing Color - blue with black background...\n");
    attroff(COLOR_PAIR(1));
    
    printw("Pres any key to start...\n");
    getch(); // block on debug stuff until key is pressed

    clear();
    refresh();

    /* Start the render thread */
    // Signal the render thread to start
    pthread_mutex_lock(&engine->renderThreadData.dataMutex);
    engine->renderThreadData.render = true;
    pthread_cond_signal(&engine->renderThreadData.renderSignal);
    pthread_mutex_unlock(&engine->renderThreadData.dataMutex);

    /* Run the game */
    // call to startGame in AlcubierreGame.c
    //startGame(engine);
    
    /* Main thread is done - wait for 'q' to be pressed to exit, send keyboard events to the engine */
    //timeout(0); // don't block on getch()
    /* Create window that isn't visible (shares memory with stdscr)
     * to get characters from. refreshing it will have no effect 
     * on the rest of the engine.
     *
     * We need to do this because getch and wgetch have an implicit
     * call to wrefresh, which can corrupt memory since we're updating
     * panels on another thread. So instead we refresh a 1x1 window in
     * the top left corner, so as to be unintrusive. By using subwin
     * instead of newwin, the 1x1 window will share memory with stdscr,
     * and thus anything printed on stdscr won't be blanked out by the
     * new window.
     */
    char input;
    while ((input = getchar()) != 'q'){
        // create keyboard input event if key pressed
        if (input != ERR){
            // the memory is managed by the event thread after we send the event, so no need to free the event here
            Event* keyEvent = (Event*)malloc(sizeof(Event));
            keyEvent->eventType.mask = 0;
            keyEvent->eventType.values.keyboardEvent = TRUE;
            keyEvent->eventData = &input;

            // send events
            //engine->handleEvent(engine, keyEvent);
            free(keyEvent);
        }

        // Sleep so we don't use too much cpu for the main thread
        sleepms(10);
    }

    /* Exit after cleaning up the engine */
    destroyEngine(engine);

    return 0;
}
