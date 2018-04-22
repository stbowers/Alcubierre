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
#include <string.h>
#include <math.h>

/* gameState is a global variable (defined as extern in AlcubierreGame.h) for use by any
 * functions that need references to game objects or information about the state of the
 * game world. Access to gameState is protected by gameStateLock.
 */
AlcubierreGameState gameState;
ThreadLock_t gameStateLock;

void startGame(Engine* engine){
    /* Initialize gameState mutex and lock it */
    createLock(&gameStateLock);
    lockThreadLock(&gameStateLock);
    
	/* Set the engine in the game state, so other functions can use it */
    gameState.engine = engine;
    
    /* Initialize basic gameState info */
    gameState.exit = false;

    /* Run a loading animation while setting up the game */
    XPFile* loadingAnimationFrames[4] = {getXPFile("./assets/Loading1.xp"), getXPFile("./assets/Loading2.xp"), getXPFile("./assets/Loading3.xp"), getXPFile("./assets/Loading4.xp")};
    GameObject* loadingAnimation = createAXPSprite(loadingAnimationFrames, 4, 100, 0, 0, 1, engine);
    centerObject((Object*)loadingAnimation, engine->mainPanel, ((XPSpriteData*)loadingAnimation->userData)->textureData->width, ((XPSpriteData*)loadingAnimation->userData)->textureData->height);
    engine->mainPanel->childrenList = (Object*) loadingAnimation;
	
    /* Initialize world state */
    initializeWorldState();

    /* Build screens */
    buildTitleScreen();
    buildOverviewScreen();

    /* Run the intro sequence */
    runIntroSequence();
    
	/* Unlock game state lock */
	unlockThreadLock(&gameStateLock);

    /* Show title screen */
    engine->mainPanel->childrenList = (Object*)gameState.titleScreen;

    /* Use title screen listeners */
    engine->mainPanel->listeners = gameState.titleScreenListenerList;
}

void cleanUpGame() {
	/* Clean up resources we own (gameObjects and any eventlistener lists that are not in use)
	 */

	/* GameObjects - mainPanel never owns any objects, not even it's children, so we clean up all of our objects */
	destroyPanel(gameState.titleScreen);
	destroyPanel(gameState.overviewScreen);
	// NOTE: we're not freeing the memory for any children of the above screens, even though we should. This should be fixed later somehow.

	/* Free */
}

void initializeWorldState(){
    /* All locations start off as unknown, except 0 which we're at */
    gameState.locations[0] = LOCATION_CURRENT;
    gameState.locations[1] = LOCATION_UNKNOWN;
    gameState.locations[2] = LOCATION_UNKNOWN;
    gameState.locations[3] = LOCATION_UNKNOWN;
    gameState.locations[4] = LOCATION_UNKNOWN;
    gameState.locations[5] = LOCATION_UNKNOWN;
    gameState.locations[6] = LOCATION_UNKNOWN;
    gameState.locations[7] = LOCATION_UNKNOWN;
    gameState.locations[8] = LOCATION_UNKNOWN;
}

void runIntroSequence(){
    sleepms(5000); // simulate loading the game
    /* Run static for 2 seconds (animated xp sprite) */
    XPFile* staticFrames[3] = {getXPFile("./assets/Static1.xp"), getXPFile("./assets/Static2.xp"), getXPFile("./assets/Static3.xp")};
    GameObject* staticAnimation = createAXPSprite(staticFrames, 3, 10, 0, 0, 1, gameState.engine);
    
    lockThreadLock(&gameState.engine->renderThreadData.renderLock);
    gameState.engine->mainPanel->childrenList = (Object*) staticAnimation;
    unlockThreadLock(&gameState.engine->renderThreadData.renderLock);

    sleepms(2000);

    /* Replace static animation with static hacked animation, run for 2 seconds */
    XPFile* hackFrames[4] = {getXPFile("./assets/Static_Hack1.xp"), getXPFile("./assets/Static_Hack2.xp"), getXPFile("./assets/Static_Hack3.xp"), getXPFile("./assets/Static_Hack4.xp")};
    GameObject* hackAnimation = createAXPSprite(hackFrames, 4, 10, 0, 0, 1, gameState.engine);

    lockThreadLock(&gameState.engine->renderThreadData.renderLock);
    gameState.engine->mainPanel->childrenList = (Object*) hackAnimation;
    unlockThreadLock(&gameState.engine->renderThreadData.renderLock);

    /* Text crawl (move an xp sprite with text up the screen) */
    sleepms(5000);
}

void buildTitleScreen(){
    /* Remove listeners from the main panel, so only our listeners remain when we're done */
    gameState.engine->mainPanel->listeners = NULL;

    /* Build the title screen panel */
    gameState.titleScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 0);

    /* Load title screen texture */
    XPFile* titleTexture = getXPFile("./assets/Alcubierre_Title.xp");
    GameObject* titleTextureObject = createXPSprite(titleTexture, 0, 0, 0, gameState.engine);
    centerObject((Object*)titleTextureObject, gameState.titleScreen, titleTexture->layers[0].width, titleTexture->layers[0].height);
    gameState.titleScreen->addObject(gameState.titleScreen, (Object*)titleTextureObject);
    
    /* Create main menu */
    char* items[] = {"(P)lay", "(I)nstructions", "(B)ackstory", "(E)xit"};
    char keys[] = {'p', 'i', 'b', 'e'};
    pfn_SelectionCallback callbacks[] = {playCallback, infoCallback, backstoryCallback, exitCallback};
    GameObject* menu = createSelectionWindow(items, keys, callbacks, 4, 0, 0, 1, gameState.engine);
    allignObjectX((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->width, .5);
    allignObjectY((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->height, .75);
    gameState.titleScreen->addObject(gameState.titleScreen, (Object*)menu);

    /* Save listener list */
    gameState.titleScreenListenerList = gameState.engine->mainPanel->listeners;
}

void buildOverviewScreen(){
    /* Remove listeners */
    gameState.engine->mainPanel->listeners = NULL;

    /* Create overview screen panel */
    gameState.overviewScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 0);

    /* Load basic overview texture */
    XPFile* overviewTexture = getXPFile("./assets/Overview.xp");

    /* Create sprite for background */
    GameObject* backgroundSprite = createXPSprite(overviewTexture, 0, 0, 0, gameState.engine);
    centerObject((Object*)backgroundSprite, gameState.overviewScreen, overviewTexture->layers[0].width, overviewTexture->layers[0].height);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)backgroundSprite);

    /* Add location markers to overview screen */
    XPFile* locationUnknown = getXPFile("./assets/Location_Unknown.xp");
    XPFile* locationCurrentFrames[2] = {getXPFile("./assets/Location_Current1.xp"), getXPFile("./assets/Location_Current2.xp")};
    XPFile* locationCompleted = getXPFile("./assets/Location_Completed.xp");
    XPFile* locationSkipped = getXPFile("./assets/Location_Skipped.xp");

    // Get height and y location for each marker (same for all of them)
    int markerHeight = locationUnknown->layers[0].height;
    int markerY = (float)(gameState.overviewScreen->height - markerHeight) * (5.0f/8.0f);
    
    // The starting value of markerX, which will change to keep track of each new marker
    // This is kinda a magic variable, tweaked until it looks right
    int markerX = backgroundSprite->objectProperties.x + 8;

    for (int i = 0; i < 9; i++){
        GameObject* marker;
        switch (gameState.locations[i]){
        case LOCATION_UNKNOWN:
            marker = createXPSprite(locationUnknown, markerX, markerY, 1, gameState.engine);
            break;
        case LOCATION_CURRENT:
            marker = createAXPSprite(locationCurrentFrames, 2, 500, markerX, markerY, 1, gameState.engine);
            break;
        case LOCATION_COMPLETED:
            marker = createXPSprite(locationCompleted, markerX, markerY, 1, gameState.engine);
            break;
        case LOCATION_SKIPPED:
            marker = createXPSprite(locationSkipped, markerX, markerY, 1, gameState.engine);
            break;
        default:
            marker = createXPSprite(locationUnknown, markerX, markerY, 1, gameState.engine);
            break;
        }
        gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)marker);
        markerX += ((i+1)%3)?12:18;
    }

    /* Save listener list */
    gameState.overviewScreenListenerList = gameState.engine->mainPanel->listeners;
}

void updateOverviewScreen(){
    
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

    /* Change main panel's children list to the overview screen */
    gameState.engine->mainPanel->childrenList = (Object*)gameState.overviewScreen;

    /* Change event listeners to those for the overview screen */
    gameState.engine->mainPanel->listeners = gameState.overviewScreenListenerList;

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
