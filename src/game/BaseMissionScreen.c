/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/BaseMissionScreen.h>
#include <engine.h>
#include <objects/ui.h>
#include <objects/Ship.h>
#include <objects/EnemyBase.h>
#include <stdlib.h>

BaseMissionScreenState baseMissionScreenState;
ThreadLock_t baseMissionScreenStateLock;

void buildBaseMissionScreen(){
    /* Initialize lock and state */
    createLock(&baseMissionScreenStateLock);
    lockThreadLock(&baseMissionScreenStateLock);

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
    

    /* Create panel for base mission screen */
    gameState.baseMissionScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 1);

    // regiter panel for events
    gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEvents;
    EventTypeMask eventTypes;
    eventTypes.mask = 0;
    eventTypes.values.keyboardEvent = true;
    eventTypes.values.timerEvent = true;
    gameState.engine->mainPanel->registerEventListener(gameState.engine->mainPanel, eventTypes, (Object*)gameState.baseMissionScreen);
    
    /* Draw background */
    XPFile* backgroundTexture = getXPFile("./assets/BaseMissionScreen.xp");
    baseMissionScreenState.backgroundTexture = createXPSprite(backgroundTexture, 0, 0, 1, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.backgroundTexture);

    /* Mode text box */
    // text box 1 high, lenght of screen at bottom
    baseMissionScreenState.modeTextBox = createTextBox("", 0, false, gameState.engine->width, 1, 0, gameState.engine->height - 1, 5, gameState.engine);
    baseMissionScreenState.modeTextBox->objectProperties.show = false; // hidden by default
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.modeTextBox);



    /* Objectives */
    /* Player Ship */
    baseMissionScreenState.shipObject = createPlayerShip(100, 8, 3, gameState.engine);
    baseMissionScreenState.playerEngineRoom = ((ShipData*)baseMissionScreenState.shipObject->userData)->engineRoom;
    baseMissionScreenState.playerShieldRoom = ((ShipData*)baseMissionScreenState.shipObject->userData)->shieldRoom;
    baseMissionScreenState.playerWeaponsRoom = ((ShipData*)baseMissionScreenState.shipObject->userData)->weaponsRoom;
    baseMissionScreenState.playerPilotRoom = ((ShipData*)baseMissionScreenState.shipObject->userData)->pilotRoom;
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.shipObject);

    /* Player status */
    // hull (health)
    int healthBarWidth = 49; // width of space in background texture
    int healthBarX = 59;
    baseMissionScreenState.shipHelathProgressBar = createProgressBar("", 0, 0, healthBarWidth, healthBarX, 1, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.shipHelathProgressBar);
    
    // engine charge
    int engineChargeWidth = 15; // width of space in background texture
    int engineChargeX = 124;
    baseMissionScreenState.engineChargeProgressBar = createProgressBar("", 0, 0, engineChargeWidth, engineChargeX, 1, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.engineChargeProgressBar);

    // weapon charges
    int weaponsChargeWidth = 33; // width of space in background texture
    int weaponsChargeX = 222;
    int weaponsChargeY = 32;
    baseMissionScreenState.weapon1ChargeProgressBar = createProgressBar("", .33, 0, weaponsChargeWidth, weaponsChargeX, weaponsChargeY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.weapon1ChargeProgressBar);
    baseMissionScreenState.weapon2ChargeProgressBar = createProgressBar("", .33, 0, weaponsChargeWidth, weaponsChargeX, weaponsChargeY+2, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.weapon2ChargeProgressBar);
    
    // power status
    int powerStatusWidth = 7; // _ [___] - 7
    int powerTotalStatusWidth = 27; // Unused Power [___ ___ ___ ___] - 27
    int powerStatusStartX = 59;
    int powerMeterX = powerStatusStartX; // where to draw the next power meter
    int powerMeterY = 34;
    int deltaPowerMeterX = 9; // how far apart to space them
    baseMissionScreenState.enginePowerProgressBar = createProgressBar("E ", .33, 0, powerStatusWidth, powerMeterX, powerMeterY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.enginePowerProgressBar);
    powerMeterX += deltaPowerMeterX;
    baseMissionScreenState.shieldPowerProgressBar = createProgressBar("S ", .33, 0, powerStatusWidth, powerMeterX, powerMeterY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.shieldPowerProgressBar);
    powerMeterX += deltaPowerMeterX;
    baseMissionScreenState.weaponsPowerProgressBar = createProgressBar("W ", .33, 0, powerStatusWidth, powerMeterX, powerMeterY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.weaponsPowerProgressBar);
    powerMeterX += deltaPowerMeterX;
    baseMissionScreenState.pilotPowerProgressBar = createProgressBar("P ", .33, 0, powerStatusWidth, powerMeterX, powerMeterY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.pilotPowerProgressBar);
    powerMeterX += deltaPowerMeterX;

    baseMissionScreenState.unusedPowerProgressBar = createProgressBar("Unused Power ", .33, 0, powerTotalStatusWidth, powerMeterX, powerMeterY, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.unusedPowerProgressBar);

    /* Ally Base */
    /* Enemy Base */
    baseMissionScreenState.enemyBase = createEnemyBase(168, 44, 3, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.enemyBase);



    /* Update listeners */
    gameState.baseMissionScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Release lock */
    unlockThreadLock(&baseMissionScreenStateLock);
}

void refreshBaseMissionScreen(){
    ShipData* shipData = (ShipData*)baseMissionScreenState.shipObject->userData;

    /* Mode */
    switch (baseMissionScreenState.mode){
    case MODE_NORMAL:
        baseMissionScreenState.modeTextBox->objectProperties.show = false;
        break;
    case MODE_PAUSED:
        baseMissionScreenState.modeTextBox->objectProperties.show = true;
        updateTextBox(baseMissionScreenState.modeTextBox, " { PAUSED } ", 0, true);
        break;
    case MODE_TARGET_ENEMY:
        baseMissionScreenState.modeTextBox->objectProperties.show = true;
        updateTextBox(baseMissionScreenState.modeTextBox, " { Choose Target for Weapon } ", 0, true);
        break;
    case MODE_TARGET_ASSIST:
        baseMissionScreenState.modeTextBox->objectProperties.show = true;
        updateTextBox(baseMissionScreenState.modeTextBox, " { Choose Room to Send Personnel } ", 0, true);
        break;
    }

    /* Power systems */
    updateProgressBar(baseMissionScreenState.enginePowerProgressBar, (float)shipData->enginePower / 3.0f, 0);
    updateProgressBar(baseMissionScreenState.shieldPowerProgressBar, (float)shipData->shieldPower / 3.0f, 0);
    updateProgressBar(baseMissionScreenState.weaponsPowerProgressBar, (float)shipData->weaponsPower / 3.0f, 0);
    updateProgressBar(baseMissionScreenState.pilotPowerProgressBar, (float)shipData->pilotPower / 3.0f, 0);
    updateProgressBar(baseMissionScreenState.unusedPowerProgressBar, (float)(shipData->availablePower - shipData->usedPower) / (float)(shipData->totalPower), 0);

    /* Player stats */
    int black = getBestColor(0, 0, 0, gameState.engine);
    int green = getBestColor(100, 255, 100, gameState.engine);
    int cp = getColorPair(green, black, gameState.engine);
    // engine charge
    updateProgressBar(baseMissionScreenState.engineChargeProgressBar, shipData->engineCharge, (shipData->engineCharge >= 1.0f)?COLOR_PAIR(cp):0);
    // ship health
    updateProgressBar(baseMissionScreenState.shipHelathProgressBar, gameState.shipHealth / 100.0f, 0);
    
}

void updateBaseMissionScreen(Mission* mission){
    ShipData* shipData = (ShipData*)baseMissionScreenState.shipObject->userData;

    // reset the field and load info from the given mission
    
    baseMissionScreenState.mode = MODE_NORMAL;

    shipData->totalPower = 12; // you have 12 total power
    shipData->availablePower = 8; // of which you can use 8
    shipData->usedPower = 0;
    shipData->enginePower = 0;
    shipData->shieldPower = 0;
    shipData->weaponsPower = 0;
    shipData->pilotPower = 0;

    shipData->engineCharge = 0;

    // remake screen
    refreshBaseMissionScreen();
}

void baseMissionScreenHandleEvents(Object* overviewScreen, Event* event){
    // data for the player's ship
    ShipData* shipData = (ShipData*)baseMissionScreenState.shipObject->userData;

    // used to signal the screen should be redrawn after processing events
    bool redraw = false;

    /* If the event is a keyboard event, check if it's a valid action */
    if (event->eventType.values.keyboardEvent){
        // used when both the shifted and non-shifted cases run the same code, but the state needs to be known 
        bool shift = false;
        
        // keyboard events store the result of getch() in the eventData pointer
        switch ((int)event->eventData){
            /* Power distribution */

            // Engines
            case 'E':
                shift = true;
            case 'e':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one
                    shipData->enginePower--;
                    shipData->usedPower--;
                } else {
                    // if we have enough power, raise by 1
                    if (shipData->availablePower - shipData->usedPower > 0){
                        shipData->enginePower++;
                        shipData->usedPower++;
                    }
                }

                // make sure engine power is capped between 0 and 3
                if (shipData->enginePower > 3){
                    shipData->enginePower = 3;
                } else if (shipData->enginePower < 0) {
                    shipData->enginePower = 0;
                }
                redraw = true;
                break;
            
            // Shields
            case 'S':
                shift = true;
            case 's':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one
                    shipData->shieldPower--;
                    shipData->usedPower--;
                } else {
                    // if we have enough power, raise by 1
                    if (shipData->availablePower - shipData->usedPower > 0){
                        shipData->shieldPower++;
                        shipData->usedPower++;
                    }
                }

                // make sure power is capped between 0 and 3
                if (shipData->shieldPower > 3){
                    shipData->shieldPower = 3;
                } else if (shipData->shieldPower < 0) {
                    shipData->shieldPower = 0;
                }
                redraw = true;
                break;
            
            // Weapons
            case 'W':
                shift = true;
            case 'w':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one
                    shipData->weaponsPower--;
                    shipData->usedPower--;
                } else {
                    // if we have enough power, raise by 1
                    if (shipData->availablePower - shipData->usedPower > 0){
                        shipData->weaponsPower++;
                        shipData->usedPower++;
                    }
                }

                // make sure power is capped between 0 and 3
                if (shipData->weaponsPower > 3){
                    shipData->weaponsPower = 3;
                } else if (shipData->weaponsPower < 0) {
                    shipData->weaponsPower = 0;
                }
                redraw = true;
                break;

            // Pilot
            case 'P':
                shift = true;
            case 'p':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one
                    shipData->pilotPower--;
                    shipData->usedPower--;
                } else {
                    // if we have enough power, raise by 1
                    if (shipData->availablePower - shipData->usedPower > 0){
                        shipData->pilotPower++;
                        shipData->usedPower++;
                    }
                }

                // make sure power is capped between 0 and 3
                if (shipData->pilotPower > 3){
                    shipData->pilotPower = 3;
                } else if (shipData->pilotPower < 0) {
                    shipData->pilotPower = 0;
                }
                redraw = true;
                break;

            // Pause
            case ' ':
            case 27:
                baseMissionScreenState.mode = MODE_PAUSED;
                // change event listener
                gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEventsPaused;
                redraw = true;
                break;

            case KEY_ENTER:
            case 10:
                // jump if engine is fully charged
                if (shipData->engineCharge >= 1.0f){
                    /* Move to overview screen */
                    gameState.engine->mainPanel->childrenList = (Object*)gameState.overviewScreen;
                    gameState.engine->mainPanel->listeners = gameState.overviewScreenListenerList;
                }
                break;
        }
    }
    /* if the event is a timer event, update values that change with time */
    else if (event->eventType.values.timerEvent){
        // more power in the engines causes faster charging,
        // as does more power to the pilot
        float dEdt = .01f * (shipData->enginePower) * (shipData->pilotPower);

        // dE | deltat ms |   1 s   | = deltaE
        // --------------------------
        // dt |     1     | 1000 ms |
        int deltat = (int)event->eventData; // ammount of elapsed ms since last update
        shipData->engineCharge += dEdt*(deltat/1000.0f);
        if (shipData->engineCharge >= 1.0f){
            shipData->engineCharge = 1.0f;
        }

        // randomly make the ship take 5 damage (2%)
        bool damage = ((float)rand() / (float)RAND_MAX) > .996;
        gameState.shipHealth -= (damage)?5:0;

        redraw = true;
    }

    // refresh the screen if data has changed
    if (redraw){
        refreshBaseMissionScreen();
    }
}

// modal event hadlers
void baseMissionScreenHandleEventsPaused(Object* screen, Event* event){
    /* If the event is a keyboard event, check if it's a valid action */
    if (event->eventType.values.keyboardEvent){
        
        // keyboard events store the result of getch() in the eventData pointer
        switch ((int)event->eventData){
            case ' ':
            case 27:
                // unpause game
                baseMissionScreenState.mode = MODE_NORMAL;
                gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEvents;
                refreshBaseMissionScreen();
                break;
        }
    }

    // since we're paused, we ignore other events
}
