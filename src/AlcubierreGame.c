/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <AlcubierreGame.h>
#include <game/OverviewScreen.h>
#include <game/TitleScreen.h>
#include <game/BaseMissionScreen.h>
#include <game/StationMissionScreen.h>
#include <game/StoreScreen.h>
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
    "Do not become too reckless, however. The Alcubierre is our only ship outfitted with this shield weakening device. It will play a critical role in our final battle to take out the enemy's stargate at the edge of the Solar System.\n\n"
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
    buildBaseMissionScreen();
    buildStationMissionScreen();
    buildStoreScreen();

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
    
    /* We start of in sector 0 */
    gameState.currentSector = 0;

    /* Create missions */
    for (int sector = 0; sector < 9; sector++){
        for (int mission = 0; mission < 3; mission++){
            strcpy(gameState.missions[sector][mission].missionTitle, "Assault Alien Base");
            gameState.missions[sector][mission].missionType = MISSION_BASE;
        }
    }
    strcpy(gameState.missions[0][0].missionTitle, "Scout Alien Solar Station");
    strcpy(gameState.missions[0][2].missionTitle, "Protect Resistance Base");

    /* Basic stats */
    gameState.shipHealth = 100;
    gameState.fleetStrength = 25;
    gameState.alienStrenth = 75;
    gameState.credits = 0;
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
