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
#include <ctype.h>
#include <string.h>

#include <objects/sprites.h>
#include <objects/ui.h>

void drawShip(Object* self, Panel* panel);

void testEventHandler(Object* self, const Event* e){
    // draw character to screen at 1,1 if keyboard event
    if (e->eventType.mask && EVENT_KEYBOARD){
        mvwprintw(((Panel*)self)->window, 2, 1, "%c", *((char*)e->eventData));
    }
}

int main(){
    /* Run some tests before starting the engine */
    //AXPFile* animation1 = getAXPFile("./animation1.axp");
    //printf("AXP loaded...\nVersion: %d\nFPS: %d\nXP Version: %d\nFrames: %d\n", animation1->version, animation1->fps, animation1->xpFile->version, animation1->xpFile->numLayers);

    setlocale(LC_CTYPE, "");
    // Our game runs in a 128x72 (16:9) window
    // start the engine as 130x74, to give room for the border
    Engine* engine = initializeEngine(130, 74);
    
    if (!has_colors()){
        printw("Term doesn't support color...\n");
    }
    printw("Term supports %d colors\n", COLORS);
    printw("Term supports %d color pairs\n", COLOR_PAIRS);
    printw("Term can change color: %s\n", (can_change_color())?"true":"false");
    printw("Terminal Size: %dx%d\n", COLS, LINES);

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    wprintw(stdscr, "Testing Color...\n");
    attroff(COLOR_PAIR(1));
    
    //start_color(); // move this shit to initializeEngine when i'm done fucking around with stuff in main
    
    //GameObject testobj;
    //testobj.userData = createPanel(126, 70, 16, 10, 1);
    //testobj.objectProperties.drawObject = drawShip;
    //testobj.objectProperties.next = NULL;
    //testobj.objectProperties.previous = NULL;
    //engine->mainPanel->addObject(engine->mainPanel, (Object*)&testobj);
    XPFile* splashTexture = getXPFile("./Alcubierre_Title.xp");
    
    // determine dimentions and position
    int splashWidth = splashTexture->layers[0].width;
    int splashHeight = splashTexture->layers[0].height;
    int splashX = (COLS - splashWidth) / 2;
    int splashY = (LINES - splashHeight) / 2;
    
    GameObject* splashSprite = createXPSprite(splashTexture, splashX, splashY);
    engine->mainPanel->addObject(engine->mainPanel, (Object*)splashSprite);

    // create menu
    const char* items[] = {"(P)lay", "(I)nstructions", "(B)ackstory", "(E)xit"};
    const char keys[] = {'p', 'i', 'b', 'e'};
    int menuWidth = 20; // hardcoded for now - will be changed later
    int menuHeight = 4;
    int menuX = (COLS - menuWidth) / 2; // centered in x
    int menuY = (float)(LINES - menuHeight) * (3.0f/4.0f); // 3/4 down screen
    GameObject* menu = createSelectionWindow(items, keys, 4, menuX, menuY);
    engine->mainPanel->addObject(engine->mainPanel, (Object*)menu);
    // register events with the selection menu
    //engine->activePanel = ((SelectionWindowData*)menu->userData)->panel;

    //GameObject* animationTest = createAXPSprite(animation1, 9, 25);
    //engine->mainPanel->addObject(engine->mainPanel, (Object*)animationTest);

    // set custom window event handler
    //if (engine->activePanel){
    //    engine->activePanel->objectProperties.handleEvent = testEventHandler;
    //}

    //top_panel(engine->mainPanel->panel);
    //update_panels();
    //doupdate();

    printw("Pres any key to start...\n");
    getch(); // block on debug stuff until key is pressed
    
    // signal render thread to start
    pthread_mutex_lock(&engine->renderThreadData.dataMutex);
    engine->renderThreadData.render = true;
    pthread_cond_signal(&engine->renderThreadData.renderSignal);
    pthread_mutex_unlock(&engine->renderThreadData.dataMutex);

    /*
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
    */
    
    timeout(0); // don't block on getch()
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
    WINDOW* getchWindow = subwin(stdscr, 1, 1, 0, 0);
    char input;
    while ((input = wgetch(getchWindow)) != 'q'){
        // create keyboard input event if key pressed
        if (input != ERR){
            // Event's aren't working correctly right now, so just process user input manually
            static Panel* newPanel = NULL;
            if (newPanel == NULL){
                int width = 100;
                int height = 50;
                // center the panel
                int xpos = (COLS - width) / 2;
                int ypos = (LINES - height) / 2;
                
                // create a panel for the border
                Panel* borderPanel = createPanel(width + 2, height + 2, xpos - 1, ypos - 1, 10);
                engine->mainPanel->addObject(engine->mainPanel, (Object*)borderPanel);
                // draw border
                box(borderPanel->window, 0, 0);

                // Create content panel
                newPanel = createPanel(width, height, xpos, ypos, 10);
                engine->mainPanel->addObject(engine->mainPanel, (Object*)newPanel);
            }

            switch (toupper(input)){
                case 'P':
                    mvwprintw(newPanel->window, 0, 0, "Game still in development...");
                    mvwprintw(newPanel->window, 1, 0, "Press q or e to exit...");
                    break;
                case 'I':
                    mvwprintw(newPanel->window, 0, 0, "----HOW TO PLAY----");
                    mvwprintw(newPanel->window, 1, 0, "Game is still in development, and the controls have not been worked out.");
                    mvwprintw(newPanel->window, 2, 0, "Press q or e to exit...");
                    break;
                case 'B':
                    mvwprintw(newPanel->window, 0, 0, "Backstory: ");
                    mvwprintw(newPanel->window, 1, 0, "(Press q or e to exit)");
                    mvwprintw(newPanel->window, 2, 0, "It has been 150 years since the first invasion of the aliens, and the future of humanity looks bleak.\n"
                            "However, the rebels finally have secured a key peice of technology that they think will help them win the fight.\n"
                            "This device somehow weakens the alien shields, potentially giving the rebels a way to inflict large ammounts of damage on key locations.\n"
                            "You have been tasked with piloting the RSS Alcubbiere - a scouting vessel with limited fighting capability - to various locations "
                            "The rebels are interested in securing. The rebels only have one sheild weakening device, and they have put it on your ship to aid "
                            "your mission.\n"
                            "Your goal is to inflict as much damage as possible while making your way to the edge of the solar system, where the aliens have set up "
                            "a stargate, which allows them to send renforcements in a matter of hours rather than years. If you can make it to this stargate and "
                            "destroy it, humanity will have a real chance at overthrowing the aliens occupying the rest of the solar system.");
                    break;
                case 'E':
                    // clean up
                    destroyEngine(engine);

                    // exit
                    exit(0);
                    break;
            }
            /*
            Event* keyEvent = (Event*)malloc(sizeof(Event));
            keyEvent->eventType.mask = 0;
            keyEvent->eventType.values.keyboardEvent = TRUE;
            keyEvent->eventData = &input;

            // send events
            engine->handleEvent(engine, keyEvent);
            */
        }
        sleepms(10);
    }

    destroyEngine(engine);

    return 0;
}

void drawShip(Object* self, Panel* panel){
    Panel* testpanel = (Panel*)(((GameObject*)self)->userData);
    mvwprintw(testpanel->window, 0, 0, "Testing text on a panel...");
    /*make this the top panel whenever we draw*/
    top_panel(testpanel->panel);
    /* update_panels() replaces the refresh() call, but it doesn't write to the screen yet*/
    update_panels();
}
