/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <AlcubierreGame.h>
#include <objects/sprites.h>
#include <objects/ui.h>
#include <stdlib.h>

void playCallback();
void infoCallback();
void backstoryCallback();
void exitCallback();

void runIntroSequence();
void buildOverviewScreen();

AlcubierreGameState gameState;

void startGame(Engine* engine){
    /* Set the engine in the game state, so other functions can use it */
    gameState.engine = engine;

    /* Build other screens before presenting the main menu */
    buildOverviewScreen();

    /* Run the intro sequence - this returns once the intro is done and we can render the menu */
    runIntroSequence();

    /* Set up main menu */
    // create menu
    char* items[] = {"(P)lay", "(I)nstructions", "(B)ackstory", "(E)xit"};
    char keys[] = {'p', 'i', 'b', 'e'};
    pfn_SelectionCallback callbacks[] = {playCallback, infoCallback, backstoryCallback, exitCallback};
    int menuWidth = 20; // hardcoded for now - will be changed later
    int menuHeight = 4;
    int menuX = (gameState.engine->width - menuWidth) / 2; // centered in x
    int menuY = (float)(gameState.engine->height - menuHeight) * (3.0f/4.0f); // 3/4 down screen
    GameObject* menu = createSelectionWindow(items, keys, callbacks, 4, menuX, menuY, engine);
    engine->mainPanel->addObject(engine->mainPanel, (Object*)menu);
}

void runIntroSequence(){
    /* Load title screen */
    XPFile* titleTexture = getXPFile("./Alcubierre_Title.xp");
    
    // determine dimentions and position
    int splashWidth = gameState.engine->width;
    int splashHeight = gameState.engine->height;
    int splashX = ((float)(COLS - splashWidth) / 2.0f);
    int splashY = ((float)(LINES - splashHeight) / 2.0f);
    
    GameObject* splashSprite = createXPSprite(titleTexture, 1, 1, 0);
    gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)splashSprite);
    gameState.titleScreen = splashSprite;

    /* Replace title screen panel while playing intro sequence */
  //Panel** spritePanel = &((XPSpriteData*)splashSprite->userData)->textureData->panel;
  //Panel* titlePanel = *spritePanel;

  //*spritePanel = createPanel(splashWidth, splashHeight, splashX, splashY, 0);
  //
  ///* Run static for 1 second, update every 1/10th of a second */
  //for (int i = 0; i < 10; i++){
  //    // move cursor to start of window
  //    wmove((*spritePanel)->window, 0, 0);
  //    // draw random chars to the window
  //    for (int y = 0; y < splashHeight; y++){
  //        for (int x = 0; x < splashHeight; x++){
  //            waddch((*spritePanel)->window, 48 + i);
  //        }
  //        waddch((*spritePanel)->window, '\n');
  //    }
  //    sleepms(100);
  //}

  ///* Go back to title screen */
  //destroyPanel(*spritePanel);
  //*spritePanel = titlePanel;
}

void buildOverviewScreen(){
    /* Load basic overview texture */
    XPFile* overviewTexture = getXPFile("./Overview.xp");

    /* Create sprite for background */
    GameObject* backgroundSprite = createXPSprite(overviewTexture, 1, 1, 1);

    /* Hide overview screen */
    XPSpriteData* backgroundData = (XPSpriteData*)backgroundSprite->userData;
    hide_panel(backgroundData->textureData->panel->panel);

    gameState.overviewScreen = backgroundSprite;
}

Panel* infoPanel = NULL;
Panel* borderPanel = NULL;
void infoPanelHandleEvents(Object* self, Event* event){
    if (event->eventType.values.keyboardEvent){
        if (*(char*)event->eventData == 'b'){
            // hide info window
            infoPanel->objectProperties.show = false;
            borderPanel->objectProperties.show = false;
            hide_panel(infoPanel->panel);
            hide_panel(borderPanel->panel);

            // change active window back to main window
            gameState.engine->activePanel = gameState.engine->mainPanel;
        }
    }
}

void clearInfoPanel(){
    if (infoPanel == NULL){
        int width = 100;
        int height = 50;
        // center the panel
        int xpos = (COLS - width) / 2;
        int ypos = (LINES - height) / 2;
        
        // create a panel for the border
        borderPanel = createPanel(width + 2, height + 2, xpos - 1, ypos - 1, 10);
        gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)borderPanel);
        // draw border
        box(borderPanel->window, 0, 0);

        // Create content panel
        infoPanel = createPanel(width, height, xpos, ypos, 10);
        gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)infoPanel);
        infoPanel->objectProperties.handleEvent = infoPanelHandleEvents;
    }
   
    borderPanel->objectProperties.show = true;
    infoPanel->objectProperties.show = true;
    // capture events from engine
    gameState.engine->activePanel = infoPanel;
    wclear(infoPanel->window);
}

void playCallback(){
    /* Remove title screen */
    gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)gameState.titleScreen);

    /* Add overview screen */
    gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)gameState.overviewScreen);
}

void infoCallback(){
    clearInfoPanel();
    mvwprintw(infoPanel->window, 0, 0, "----HOW TO PLAY----");
    mvwprintw(infoPanel->window, 1, 0, "Game is still in development, and the controls have not been worked out.");
    mvwprintw(infoPanel->window, 1, 0, "Press q or e to exit, press b to go back to the main menu...");
}

void backstoryCallback(){
    clearInfoPanel();
    mvwprintw(infoPanel->window, 0, 0, "Backstory: ");
    mvwprintw(infoPanel->window, 1, 0, "(Press q or e to exit, press b to go back to the main menu)");
    mvwprintw(infoPanel->window, 2, 0, "It has been 150 years since the first invasion of the aliens, and the future of humanity looks bleak.\n"
            "However, the rebels finally have secured a key peice of technology that they think will help them win the fight.\n"
            "This device somehow weakens the alien shields, potentially giving the rebels a way to inflict large ammounts of damage on key locations.\n"
            "You have been tasked with piloting the RSS Alcubbiere - a scouting vessel with limited fighting capability - to various locations "
            "The rebels are interested in securing. The rebels only have one sheild weakening device, and they have put it on your ship to aid "
            "your mission.\n"
            "Your goal is to inflict as much damage as possible while making your way to the edge of the solar system, where the aliens have set up "
            "a stargate, which allows them to send renforcements in a matter of hours rather than years. If you can make it to this stargate and "
            "destroy it, humanity will have a real chance at overthrowing the aliens occupying the rest of the solar system.");
}

void exitCallback(){
    endwin();
    printf("Exit callback called\n");
    exit(0);
}
