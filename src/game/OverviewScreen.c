/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/OverviewScreen.h>
#include <game/BaseMissionScreen.h>
#include <AlcubierreGame.h>
#include <engine.h>
#include <xpFunctions.h>
#include <objects/sprites.h>
#include <objects/ui.h>

OverviewScreenState overviewScreenState;
ThreadLock_t overviewScreenStateLock;

void buildOverviewScreen(){
    /* Initialize lock and state */
    createLock(&overviewScreenStateLock);
    lockThreadLock(&overviewScreenStateLock);

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Create overview screen panel */
    gameState.overviewScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 0);
    // overwrite the default panel handle event function pointer
    gameState.overviewScreen->objectProperties.handleEvent = overviewScreenHandleEvents;
    // set up overview screen to get event notifications
    EventTypeMask eventTypes;
    eventTypes.mask = 0;
    eventTypes.values.keyboardEvent = true;
    gameState.engine->mainPanel->registerEventListener(gameState.engine->mainPanel, eventTypes, (Object*)gameState.overviewScreen);

    /* Load basic overview texture */
    XPFile* overviewTexture = getXPFile("./assets/Overview.xp");

    /* Create sprite for background */
    GameObject* backgroundSprite = createXPSprite(overviewTexture, 0, 0, 0, gameState.engine);
    centerObject((Object*)backgroundSprite, gameState.overviewScreen, overviewTexture->layers[0].width, overviewTexture->layers[0].height);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)backgroundSprite);

    /* Stats */
    int statsWidth = 50;
    int statsX = 77;
    int statsY = 1;
    overviewScreenState.shipHealthProgressBar = createProgressBar("", .33, 0, statsWidth, statsX, statsY, 2, gameState.engine);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.shipHealthProgressBar);
    statsY += 2;
    overviewScreenState.fleetStrengthProgressBar = createProgressBar("", .33, 0, statsWidth, statsX, statsY, 2, gameState.engine);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.fleetStrengthProgressBar);
    statsY += 2;
    overviewScreenState.alienStrengthProgressBar = createProgressBar("", .33, 0, statsWidth, statsX, statsY, 2, gameState.engine);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.alienStrengthProgressBar);
    statsY += 2;
    overviewScreenState.creditsTextBox = createTextBox("", 0, false, 6, 1, statsX + 44, statsY, 2, gameState.engine);
    gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.creditsTextBox);

    /* Initialize locationMarkers array */
    for (int i = 0; i < 9; i++){
        overviewScreenState.locationMarkers[i] = NULL;
        // will automatically be updated if marker is NULL, so set to false for now
        overviewScreenState.locationStatusChanged[i] = false;
    }

    /* Get location marker textures */
    overviewScreenState.locationUnknownTexture = getXPFile("./assets/Location_Unknown.xp");
    overviewScreenState.locationCurrentFrames[0] = getXPFile("./assets/Location_Current1.xp");
    overviewScreenState.locationCurrentFrames[1] = getXPFile("./assets/Location_Current2.xp");
    overviewScreenState.locationCompletedTexture = getXPFile("./assets/Location_Completed.xp");
    overviewScreenState.locationSkippedTexture = getXPFile("./assets/Location_Skipped.xp");

    // The starting x coordinate for location markers
    // This is kinda a magic variable, tweaked until it looks right
    overviewScreenState.locationMarkerStartX = backgroundSprite->objectProperties.x + 22;
    
    /* Update screen - draws objects that change depending on game state */
    // updateOverviewScreen() expects the current thread to not own the overviewScreenStateLock, so release it while calling the function
    unlockThreadLock(&overviewScreenStateLock);
    updateOverviewScreen();
    lockThreadLock(&overviewScreenStateLock);



    /* Create end screen */
    overviewScreenState.endGameScreen = createPanel(100, 50, 0, 0, 2);
    centerObject((Object*)overviewScreenState.endGameScreen, gameState.engine->mainPanel, 100, 50);

    overviewScreenState.endText = createTextBox("You Win!", 0, true, 100, 50, 0, 0, 1, gameState.engine);
    overviewScreenState.endGameScreen->addObject(overviewScreenState.endGameScreen, (Object*)overviewScreenState.endText);



    /* Create mission select panel for the current sector */
    int missionPanelWidth = gameState.engine->width * (.6f);
    int missionPanelHeight = gameState.engine->height * (.9f);
    int missionPanelInfoWidth = missionPanelWidth * (.6f);
    int missionPanelMenuWidth = missionPanelWidth - (missionPanelInfoWidth + 1);
    overviewScreenState.missionSelectionPanel = createPanel(missionPanelWidth, missionPanelHeight, 0, 0, 2);
    centerObject((Object*)overviewScreenState.missionSelectionPanel, gameState.engine->mainPanel, missionPanelWidth, missionPanelHeight);
    
    // add mission info text box to panel
    overviewScreenState.missionSelectionInfo = createTextBox("", 0, true, missionPanelInfoWidth, missionPanelHeight, 0, 0, 2, gameState.engine);
    overviewScreenState.missionSelectionPanel->addObject(overviewScreenState.missionSelectionPanel, (Object*)overviewScreenState.missionSelectionInfo);
    
    // add ui selection window to panel
    char* items[] = {"(1) Mission 1", "(2) Mission 2", "(3) Mission 3", "(S) Skip Sector"};
    char keys[] = {'1', '2', '3', 's'};
    pfn_SelectionCallback callbacks[] = {missionSelected, missionSelected, missionSelected, sectorSkipped};
    overviewScreenState.missionSelectionMenu = createSelectionWindow(items, keys, true, true, callbacks, missionSelectionChanged, false, 4, missionPanelMenuWidth, missionPanelWidth - missionPanelMenuWidth, 0, 3, gameState.engine);
    missionSelectionChanged(0); // call to initialize the info text box to the first selection
    allignObjectY((Object*)overviewScreenState.missionSelectionMenu, gameState.titleScreen, ((SelectionWindowData*)overviewScreenState.missionSelectionMenu->userData)->height, .5);
    overviewScreenState.missionSelectionPanel->addObject(overviewScreenState.missionSelectionPanel, (Object*)overviewScreenState.missionSelectionMenu);
    
    // Register menu to receive events from missionSelectionPanel (not mainPanel, since we want to change the focus during mission selection)
    EventTypeMask eventMask;
    eventMask.mask = 0;
    eventMask.values.keyboardEvent = true;
    overviewScreenState.missionSelectionPanel->registerEventListener(overviewScreenState.missionSelectionPanel, eventMask, (Object*)overviewScreenState.missionSelectionMenu);

    /* Save listener list */
    gameState.overviewScreenListenerList = gameState.engine->mainPanel->listeners;

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* unlock overview state mutex */
    unlockThreadLock(&overviewScreenStateLock);
}

void updateOverviewScreen(){
    lockThreadLock(&overviewScreenStateLock);

    /* Update stats */
    updateProgressBar(overviewScreenState.shipHealthProgressBar, gameState.shipHealth / 100.0f, 0);
    updateProgressBar(overviewScreenState.fleetStrengthProgressBar, gameState.fleetStrength / 100.0f, 0);
    updateProgressBar(overviewScreenState.alienStrengthProgressBar, gameState.alienStrenth / 100.0f, 0);

    char creditsText[6]; // tmp buffer for output of snprintf to format the credits with leading 0s
    snprintf(creditsText, 6, "%05d", gameState.credits);
    updateTextBox(overviewScreenState.creditsTextBox, creditsText, 0, false);

    /* Update location markers */
    // Get height and y location for each marker (same for all of them)
    int markerHeight = overviewScreenState.locationUnknownTexture->layers[0].height;
    int markerY = (float)(gameState.overviewScreen->height - markerHeight) * (3.0f/8.0f);
    int markerX = overviewScreenState.locationMarkerStartX;
    int markerSpacingNormal = 20;
    int markerSpacingLarge = 44;

    for (int i = 0; i < 9; i++){
        GameObject* currentMarker = overviewScreenState.locationMarkers[i];
        bool updateMarker = overviewScreenState.locationStatusChanged[i];

        /* if the gameobject for this marker is null, we need to update */
        if (currentMarker == NULL){
            updateMarker = true;
        }

        /* if the marker needs to be updated, create a new sprite for it */
        if (updateMarker){
            switch (gameState.locations[i]){
            case LOCATION_UNKNOWN:
                overviewScreenState.locationMarkers[i] = createXPSprite(overviewScreenState.locationUnknownTexture, markerX, markerY, 3, gameState.engine);
                break;
            case LOCATION_CURRENT:
                overviewScreenState.locationMarkers[i] = createAXPSprite(overviewScreenState.locationCurrentFrames, 2, 500, markerX, markerY, 3, gameState.engine);
                break;
            case LOCATION_COMPLETED:
                overviewScreenState.locationMarkers[i] = createXPSprite(overviewScreenState.locationCompletedTexture, markerX, markerY, 3, gameState.engine);
                break;
            case LOCATION_SKIPPED:
                overviewScreenState.locationMarkers[i] = createXPSprite(overviewScreenState.locationSkippedTexture, markerX, markerY, 3, gameState.engine);
                break;
            default:
                overviewScreenState.locationMarkers[i] = createXPSprite(overviewScreenState.locationUnknownTexture, markerX, markerY, 3, gameState.engine);
                break;
            }

            /* If the old marker wasn't null, remove it from the screen and delete it */
            if (currentMarker != NULL){
                gameState.overviewScreen->removeObject(gameState.overviewScreen, (Object*)currentMarker);
                destroyXPSprite(currentMarker);
            }
            
            /* add the new marker to the screen */
            gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.locationMarkers[i]);

            /* unset the update marker flag */
            overviewScreenState.locationStatusChanged[i] = false;
        }

        // move markerX up for every marker
        // if we're right before a multiple of 3 (first marker in new section), add a larger gap to accomidate the split between sections
        markerX += ((i+1)%3)?markerSpacingNormal:markerSpacingLarge;
    }
    
    unlockThreadLock(&overviewScreenStateLock);
}

void overviewScreenHandleEvents(Object* overviewScreen, Event* event){
    /* If the event is a keyboard event, check if it's a valid action */
    if (event->eventType.values.keyboardEvent){
        // keyboard events store the result of getch() in the eventData pointer
        switch ((int)event->eventData){
            case 'e':
            case 'E':
                // exit
                lockThreadLock(&gameStateLock);
                gameState.exit = true;
                unlockThreadLock(&gameStateLock);
                break;
            case KEY_ENTER:
            case 10:
                // Enter sector (show missions available in sector)
                /* Show missionSelectionPanel */
                gameState.engine->mainPanel->addObject(gameState.engine->mainPanel, (Object*)overviewScreenState.missionSelectionPanel);

                /* Make missionSelectionPanel the active panel */
                gameState.engine->activePanel = overviewScreenState.missionSelectionPanel;
                break;
        }
    }
}

/* Called when the selected mission is changed */
void missionSelectionChanged(int index){
    /* Change the text in the info window */
    char missionInfo[256];
    if (index != 3){
        snprintf(missionInfo, 255, 
                "------MISSION--------\n"
                "%s",
                gameState.missions[gameState.currentSector][index].missionTitle);
    } else {
        snprintf(missionInfo, 255, 
                "------SKIP SECTOR----\n"
                "You will not receive any benefits from this sector. If you skip too many sectors you will lose the game.");
    }
    updateTextBox(overviewScreenState.missionSelectionInfo, missionInfo, 0, false);
}

/* Called when a mission is selected from the mission selection menu */
void missionSelected(int index){
    /* Remove mission selection menu from screen */
    gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)overviewScreenState.missionSelectionPanel);

    /* Change active panel back to mainPanel */
    gameState.engine->activePanel = gameState.engine->mainPanel;

    /* Get selected mission */
    Mission* selectedMission = &gameState.missions[gameState.currentSector][index];

    /* If the 'mission' is a store, go to the store screen, otherwise go to the mission screen */
    switch (selectedMission->missionType){
    case MISSION_BASE:
        break;
    case MISSION_STATION:
        gameState.credits += 10;
        break;
    case MISSION_STORE:
        break;
    }

    /* Update mission screen before moving to next sector */
    updateBaseMissionScreen(&gameState.missions[gameState.currentSector][index]);

    /* Set sector status to complete, move to next sector */
    gameState.locations[gameState.currentSector] = LOCATION_COMPLETED;
    overviewScreenState.locationStatusChanged[gameState.currentSector] = true;
    gameState.currentSector++;
    gameState.locations[gameState.currentSector] = LOCATION_CURRENT;
    overviewScreenState.locationStatusChanged[gameState.currentSector] = true;

    /* Reset selection menu index, since it will be reused for the next sector */
    ((SelectionWindowData*)overviewScreenState.missionSelectionMenu->userData)->currentSelection = 0;
    drawSelectionWindowBuffer(overviewScreenState.missionSelectionMenu);

    /* Update overview screen */
    updateOverviewScreen();

	/* Move to base mission screen */
	gameState.engine->mainPanel->childrenList = (Object*)gameState.baseMissionScreen;
	gameState.engine->mainPanel->listeners = gameState.baseMissionScreenListenerList;
}

/* Called when the sector is skipped from the selection menu */
void sectorSkipped(){
    /* Remove mission selection menu from screen */
    gameState.engine->mainPanel->removeObject(gameState.engine->mainPanel, (Object*)overviewScreenState.missionSelectionPanel);

    /* Change active panel back to mainPanel */
    gameState.engine->activePanel = gameState.engine->mainPanel;

    /* Set sector status to skipped, move to next sector */
    gameState.locations[gameState.currentSector] = LOCATION_SKIPPED;
    overviewScreenState.locationStatusChanged[gameState.currentSector] = true;
    gameState.currentSector++;
    if (gameState.currentSector >= 9){
        // finished last sector, show win/lose screen based on how many sectors are green/red
        int sectorsWon = 0;
        int sectorsLost = 0;
        for (int sector = 0; sector < 9; sector++){
            if (gameState.locations[sector] == LOCATION_COMPLETED){
                sectorsWon++;
            }else{
                sectorsLost++;
            }
        }
        if (sectorsWon > sectorsLost){
            // you win
            gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.endGameScreen);
        } else {
            // you lose
            updateTextBox(overviewScreenState.endText, "You Lose!", 0, false);
            gameState.overviewScreen->addObject(gameState.overviewScreen, (Object*)overviewScreenState.endGameScreen);
        }
    }
    gameState.locations[gameState.currentSector] = LOCATION_CURRENT;
    overviewScreenState.locationStatusChanged[gameState.currentSector] = true;
    
    /* Reset selection menu index, since it will be reused for the next sector */
    ((SelectionWindowData*)overviewScreenState.missionSelectionMenu->userData)->currentSelection = 0;
    drawSelectionWindowBuffer(overviewScreenState.missionSelectionMenu);

    /* Update overview screen */
    updateOverviewScreen();
}
