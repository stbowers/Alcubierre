/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#include <engine.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <string.h>

#include <xpFunctions.h>

void drawShip(Object* self);

void testEventHandler(Object* self, const Event* e){
    // draw character to screen at 1,1 if keyboard event
    if (e->eventType.mask && EVENT_KEYBOARD){
        mvwprintw(((Panel*)self)->window, 2, 1, "%c", *((char*)e->eventData));
    }
}

int main(){
    setlocale(LC_CTYPE, "");
    // Our game runs in a 128x72 (16:9) window
    Engine* engine = initializeEngine(128, 72);
    
    printf("test\n");
    if (!has_colors()){
        printf("Term doesn't support color...\n");
    }
    printf("Term supports %d colors\n", COLORS);
    printf("Term supports %d color pairs\n", COLOR_PAIRS);
    printf("Term can change color: %d\n", can_change_color());
    mvprintw(30, 10, "Terminal Size: %dx%d", COLS, LINES);
    if (init_color(COLOR_RED, 0, 0, 1000) == ERR){
        printf("Error changing red\n");
    }

    init_pair(1, COLOR_RED, COLOR_GREEN);

	attron(COLOR_PAIR(1));
	mvwprintw(stdscr, 10, 10, "Viola !!! In color ...");
	attroff(COLOR_PAIR(1));
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    mvwprintw(stdscr, 11, 10, "Another line in color");
    attroff(COLOR_PAIR(1));
    getch();
    
    //start_color(); // move this shit to initializeEngine when i'm done fucking around with stuff in main
    
    GameObject testobj;
    testobj.userData = createPanel(100, 100, 10, 10, 1);
    testobj.objectProperties.drawObject = drawShip;
    testobj.objectProperties.next = NULL;
    testobj.objectProperties.previous = NULL;
    engine->mainPanel->addObject(engine->mainPanel, (Object*)&testobj);
    // set custom window event handler
    if (engine->activePanel){
        engine->activePanel->objectProperties.handleEvent = testEventHandler;
    }

    int run = 1;
    while (run){
        timeout(0);
        char input = getch();
        if (input == 'q'){
            run = 0;
            break;
        }
        
        // create keyboard input event if key pressed
        if (input != ERR){
            Event keyEvent;
            keyEvent.eventType.mask = 0;
            keyEvent.eventType.values.keyboardEvent = TRUE;
            keyEvent.eventData = &input;

            // send events
            engine->handleEvent(engine, &keyEvent);
        }
        // draw objects
        // Drawing the main panel will cause it to draw its children
        engine->mainPanel->objectProperties.drawObject((Object*)engine->mainPanel);
        // 10ms sleep ~= 100 fps
        sleepms(10);
    }
    
    destroyEngine(engine);

    return 0;
}

void drawShip(Object* self){
    static XPFile* testfile = NULL;
    Panel* testpanel = (Panel*)(((GameObject*)self)->userData);
    if (testfile == NULL){
        testfile = getXPFile("./test_text.xp");
        mvwprintw(testpanel->window, 10, 10, "Testing text on a panel...");
        //drawLayerToPanel(&testfile->layers[0], &testpanel, true);
    }

    update_panels();
    doupdate();
    getch();
}
