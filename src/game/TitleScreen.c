/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/TitleScreen.h>
#include <AlcubierreGame.h>
#include <engine.h>
#include <objects/sprites.h>
#include <objects/ui.h>

TitleScreenState titleScreenState;
ThreadLock_t titleScreenStateLock;

const char* easyDescription = "Easy: For players new to the genere of video\n    games with any difficulty";
const char* mediumDescription = "Medium: For player who have some skill, but are\n    too scared to use hard difficulty";
const char* hardDescription = "Hard: For masochistic players who want to suffer\n    at the hand of the random number generator";
const char* demoDescription = "Demo: Easy difficulty with some modifications to make game faster for demos";

void buildTitleScreen(){
    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Build the title screen panel */
    gameState.titleScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 0);

    /* Load title screen texture */
    XPFile* titleTexture = getXPFile("./assets/Alcubierre_Title.xp");
    GameObject* titleTextureObject = createXPSprite(titleTexture, 0, 0, 0, gameState.engine);
    centerObject((Object*)titleTextureObject, gameState.titleScreen, titleTexture->layers[0].width, titleTexture->layers[0].height);
    gameState.titleScreen->addObject(gameState.titleScreen, (Object*)titleTextureObject);
    
    /* Create main menu */
    char* items[] = {"(P) Play", "(I) Instructions", "(B) Backstory", "(E) Exit"};
    char keys[] = {'p', 'i', 'b', 'e'};
    pfn_SelectionCallback callbacks[] = {playCallback, infoCallback, backstoryCallback, exitCallback};
    GameObject* menu = createSelectionWindow(items, keys, true, true, callbacks, NULL, true, 4, 40, 0, 0, 1, gameState.engine);
    allignObjectX((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->width, .5);
    allignObjectY((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->height, .75);
    gameState.titleScreen->addObject(gameState.titleScreen, (Object*)menu);



    /* Create difficulty selection screen (play option) */
    int difficultySelectionWidth = 52;
    int difficultySelectionHeight = 12;
    int difficultySelectionMenuHeight = 6; // 4 options + border
    titleScreenState.difficultySelectPanel = createPanel(difficultySelectionWidth, difficultySelectionHeight, 0, 0, 2);
    centerObject((Object*)titleScreenState.difficultySelectPanel, gameState.titleScreen, difficultySelectionWidth, difficultySelectionHeight);

    // Create difficulty info text box
    titleScreenState.difficultyInfoTextBox = createTextBox(easyDescription, 0, true, difficultySelectionWidth, difficultySelectionHeight-(difficultySelectionMenuHeight + 1), 0, 0, 1, gameState.engine);
    titleScreenState.difficultySelectPanel->addObject(titleScreenState.difficultySelectPanel, (Object*)titleScreenState.difficultyInfoTextBox);

    // Create difficulty selection menu
    char* difficulties[] = {"(E) Easy", "(M) Medium", "(H) Hard", "(D) Demo", "(B) Back"};
    char difficultyKeys[] = {'e', 'm', 'h', 'd', 'b'};
    pfn_SelectionCallback difficultyCallbacks[] = {difficultyChosenCallback, difficultyChosenCallback, difficultyChosenCallback, difficultyChosenCallback, difficultyBackCallback};
    titleScreenState.difficultySelectMenu = createSelectionWindow(difficulties, difficultyKeys, true, true, difficultyCallbacks, difficultyChangedCallback, false, 4, 0, 0, difficultySelectionHeight-6, 1, gameState.engine);
    allignObjectX((Object*)titleScreenState.difficultySelectMenu, titleScreenState.difficultySelectPanel, ((SelectionWindowData*)titleScreenState.difficultySelectMenu->userData)->width, .5);
    titleScreenState.difficultySelectPanel->addObject(titleScreenState.difficultySelectPanel, (Object*)titleScreenState.difficultySelectMenu);

    // Register difficulty select menu for events from difficulty panel
    EventTypeMask eventTypes;
    eventTypes.mask = 0;
    eventTypes.values.keyboardEvent = true;
    titleScreenState.difficultySelectPanel->registerEventListener(titleScreenState.difficultySelectPanel, eventTypes, (Object*)titleScreenState.difficultySelectMenu);



    /* Save listener list */
    gameState.titleScreenListenerList = gameState.engine->mainPanel->listeners;

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
}

void updateTitleScreen(){
    // Title screen doesn't change state
}

Panel* infoPanel = NULL;
Panel* borderPanel = NULL;
void infoPanelHandleEvents(Object* self, Event* event){
    if (event->eventType.values.keyboardEvent){
        if (*(char*)event->eventData == 'b'){
            // hide info window
            infoPanel->objectProperties.show = false;
            borderPanel->objectProperties.show = false;
            gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)infoPanel);
            gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)borderPanel);

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
        // draw border

        // Create content panel
        infoPanel = createPanel(width, height, xpos, ypos, 10);
        infoPanel->objectProperties.handleEvent = infoPanelHandleEvents;
    }
   
    borderPanel->objectProperties.show = true;
    infoPanel->objectProperties.show = true;
    gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)infoPanel);
    gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)borderPanel);
    // capture events from engine
    gameState.engine->activePanel = infoPanel;
}

void playCallback(){
	/* Get render lock so we're not changing the main panel's state while rendering */
	lockThreadLock(&gameState.engine->renderThreadData.renderLock);

    /* Add the difficulty selection panel to mainPanel's children */
    gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)titleScreenState.difficultySelectPanel);

    /* Set the difficulty selection panel as the active panel - keep events from going to the regular menu */
    gameState.engine->activePanel = titleScreenState.difficultySelectPanel;

	/* Release render lock */
	unlockThreadLock(&gameState.engine->renderThreadData.renderLock);
}

void infoCallback(){
    clearInfoPanel();
  //mvwprintw(infoPanel->window, 0, 0, "----HOW TO PLAY----");
  //mvwprintw(infoPanel->window, 1, 0, "Game is still in development, and the controls have not been worked out.");
  //mvwprintw(infoPanel->window, 1, 0, "Press q or e to exit, press b to go back to the main menu...");
}

void backstoryCallback(){
    clearInfoPanel();
  //mvwprintw(infoPanel->window, 0, 0, "Backstory: ");
  //mvwprintw(infoPanel->window, 1, 0, "(Press q or e to exit, press b to go back to the main menu)");
  //mvwprintw(infoPanel->window, 2, 0, "It has been 150 years since the first invasion of the aliens, and the future of humanity looks bleak.\n"
  //        "However, the rebels finally have secured a key peice of technology that they think will help them win the fight.\n"
  //        "This device somehow weakens the alien shields, potentially giving the rebels a way to inflict large ammounts of damage on key locations.\n"
  //        "You have been tasked with piloting the RSS Alcubbiere - a scouting vessel with limited fighting capability - to various locations "
  //        "The rebels are interested in securing. The rebels only have one sheild weakening device, and they have put it on your ship to aid "
  //        "your mission.\n"
  //        "Your goal is to inflict as much damage as possible while making your way to the edge of the solar system, where the aliens have set up "
  //        "a stargate, which allows them to send renforcements in a matter of hours rather than years. If you can make it to this stargate and "
  //        "destroy it, humanity will have a real chance at overthrowing the aliens occupying the rest of the solar system.");
}

void exitCallback(){
    lockThreadLock(&gameStateLock);
    gameState.exit = true;
    unlockThreadLock(&gameStateLock);
}

/* Difficulty selection callbacks */
void difficultyChangedCallback(int index){
    switch (index){
    case 0:
        updateTextBox(titleScreenState.difficultyInfoTextBox, easyDescription, 0, false);
        break;
    case 1:
        updateTextBox(titleScreenState.difficultyInfoTextBox, mediumDescription, 0, false);
        break;
    case 2:
        updateTextBox(titleScreenState.difficultyInfoTextBox, hardDescription, 0, false);
        break;
    case 3:
        updateTextBox(titleScreenState.difficultyInfoTextBox, demoDescription, 0, false);
        break;
    case 4:
        updateTextBox(titleScreenState.difficultyInfoTextBox, "Go back to main menu", 0, false);
        break;
    }
}

void difficultyChosenCallback(int index){
    // index: 0 = easy, 1 = medium, 2 = hard, 3 = demo
    // starting difficulty: easy: 1, medium: 4, hard: 7
    
    /* Set difficulty */
    // demo difficulty (index 4) sets difficulty to 0
    if (index != 3){
        gameState.difficulty = 1 + (index * 3);
    } else {
        gameState.difficulty = 0;
    }

    /* Reset active panel to be the main panel */
    gameState.engine->activePanel = gameState.engine->mainPanel;

    /* Remove difficulty selection panel from mainPanel's objects */
    gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)titleScreenState.difficultySelectPanel);
    
    /* Start game */
    /* Change main panel's children list to the overview screen */
    gameState.engine->mainPanel->childrenList = (Object*)gameState.overviewScreen;

    /* Change event listeners to those for the overview screen */
    gameState.engine->mainPanel->listeners = gameState.overviewScreenListenerList;

}

void difficultyBackCallback(){
    /* Reset active panel to be the main panel */
    gameState.engine->activePanel = gameState.engine->mainPanel;

    /* Remove difficulty selection panel from mainPanel's objects */
    gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)titleScreenState.difficultySelectPanel);
}
