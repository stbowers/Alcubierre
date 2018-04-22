/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <AlcubierreGame.h>
#include <game/OverviewScreen.h>
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

/* Introduction text, plays during the intro sequence. Scrolls up line by line, hence it being stored in lines here.
 */
const char* introText =
	"-----ENCRYPTED COMMUNICATION FROM INTERPLANETARY RESISTANCE-----\n"
    "Good evening, Recruit.\n\n"
    "You have been promoted to Commander of the IRSS Alcubierre, our most advanced scout ship.\n\n"
    "Your mission is to scout several sectors on the way to the outer edge of the solar system, laying down opportunities for the rest of our fleet to take a hold in those sectors.\n\n"
    "Your ship has been equipped with a new device which disrupts the enemy's shields. This will allow you to quickly deal large amounts of damage to critical systems, giving you the upper hand in battle.\n\n"
    "Do not become too reckless, however. The Alcubierre is our only ship outfitted with this shield weakening device. It will play a critical role in our final battle to take out the enemy’s stargate at the edge of the Solar System.\n\n"
    "You're survival is critical to the Interplanetary Resistance's plans to take back our home.\n\n"
    "Don't let us down, Commander.\n\n"
    "-----END OF ENCRYPTED MESSAGE-----\n"
    "Press any key to continue...";

void startGame(Engine* engine, bool skipIntro){
    /* Initialize gameState mutex and lock it */
    createLock(&gameStateLock);
    lockThreadLock(&gameStateLock);
    
	/* Set the engine in the game state, so other functions can use it */
    gameState.engine = engine;
    
    /* Initialize basic gameState info */
    gameState.exit = false;
    gameState.currentSector = 0;

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
    if (!skipIntro){
        runIntroSequence();
    }
    
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
    /* Run static for 2 seconds (animated xp sprite) */
    XPFile* staticFrames[3] = {getXPFile("./assets/Static1.xp"), getXPFile("./assets/Static2.xp"), getXPFile("./assets/Static3.xp")};
    GameObject* staticAnimation = createAXPSprite(staticFrames, 3, 10, 0, 0, 1, gameState.engine);
    
    lockThreadLock(&gameState.engine->renderThreadData.renderLock);
    gameState.engine->mainPanel->childrenList = (Object*) staticAnimation;
    unlockThreadLock(&gameState.engine->renderThreadData.renderLock);

    sleepms(2000);

    /* Replace static animation with static hacked animation, run for 2 seconds */
    XPFile* hackFrames[4] = {getXPFile("./assets/Static_Hack1.xp"), getXPFile("./assets/Static_Hack2.xp"), getXPFile("./assets/Static_Hack3.xp"), getXPFile("./assets/Static_Hack4.xp")};
    GameObject* hackAnimation = createAXPSprite(hackFrames, 4, 100, 0, 0, 1, gameState.engine);

    lockThreadLock(&gameState.engine->renderThreadData.renderLock);
    gameState.engine->mainPanel->childrenList = (Object*) hackAnimation;
    unlockThreadLock(&gameState.engine->renderThreadData.renderLock);

    sleepms(2000);

	/* Freeze hack animation by setting numFrames to 1 */
	((AXPSpriteData*)hackAnimation->userData)->numFrames = 1;

    /* Text crawl */
    // write our text to the texture buffer of the first frame of the hack animation
	CursesChar* buffer = ((AXPSpriteData*)hackAnimation->userData)->textureData->frames[0];
    int bufferWidth = ((AXPSpriteData*)hackAnimation->userData)->textureData->width;
    int bufferHeight = ((AXPSpriteData*)hackAnimation->userData)->textureData->height;
    int startX = 10; // the text portion is inset into the texture, so we don't want to start at 0,0
    int startY = 5;
    int textHeight = 61;
    int textWidth = 236;

    // get colors for text
    //lockThreadLock(&gameState.engine->renderThreadData.drawLock);
    int bg = getBestColor(0, 0, 0, gameState.engine);
    int fg = getBestColor(0, 217, 0, gameState.engine);
    int colorPair = getColorPair(fg, bg, gameState.engine);
    //unlockThreadLock(&gameState.engine->renderThreadData.drawLock);

    // loop up how many lines of text we're drawing at a time
    for (int lines = 1; lines <= textHeight; lines++){
        // get the y value we need to start drawing at to print lines to the end of textHeight
        int y = startY + (textHeight - lines);
        
        // blank out the lines from y to startY + textHeight
        for (int yPos = y; yPos < startY + textHeight; yPos++){
            for (int x = startX; x < startX + textWidth; x++){
                CursesChar* charAt = &buffer[(x * bufferHeight) + yPos];
                charAt->attributes = 0;
                charAt->character = ' ';
            }
        }
        
        // and draw the text
        int linesDrawn = bufferPrintf(buffer, textWidth, bufferHeight, lines, startX, y, COLOR_PAIR(colorPair), "%s", introText);
        if (linesDrawn == lines){
            // Still printing more, slower print speed
            sleepms(200);
        } else {
            sleepms(50);
        }
    }

    getch();
}

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
    char* items[] = {"(P)lay", "(I)nstructions", "(B)ackstory", "(E)xit"};
    char keys[] = {'p', 'i', 'b', 'e'};
    pfn_SelectionCallback callbacks[] = {playCallback, infoCallback, backstoryCallback, exitCallback};
    GameObject* menu = createSelectionWindow(items, keys, true, true, callbacks, true, 4, 20, 0, 0, 1, gameState.engine);
    allignObjectX((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->width, .5);
    allignObjectY((Object*)menu, gameState.titleScreen, ((SelectionWindowData*)menu->userData)->height, .75);
    gameState.titleScreen->addObject(gameState.titleScreen, (Object*)menu);

    /* Save listener list */
    gameState.titleScreenListenerList = gameState.engine->mainPanel->listeners;

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
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
