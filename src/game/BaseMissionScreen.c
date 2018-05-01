/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/BaseMissionScreen.h>
#include <game/OverviewScreen.h>
#include <game/GameOverScreen.h>
#include <engine.h>
#include <objects/ui.h>
#include <objects/Ship.h>
#include <objects/EnemyBase.h>
#include <stdlib.h>
#include <math.h>

BaseMissionScreenState baseMissionScreenState;
ThreadLock_t baseMissionScreenStateLock;

const char baseMissionInstructions[] = "-----Instructions-----\n"
    "While scouting this region of space you came across an alien base!\n\n"

    "You are collecting information for the rebellion while you are here, but you must wait for your engines to charge before you\ncan jump away.\n\n"

    "Your engines will only charge while you have dedicated at least some power to both the engines and the helm\n(see controls below).\n\n"

    "You may also dedicate power to weapons or shields, which may help you complete other objectives. Your shields will reduce the ammount of damage "
    "you take, and giving power to the weapons system will allow you to do damage to the enemy base.\n\n"

    "The weapons system must have at least one power to charge the lasers, and two to charge the missiles. Less power will cause\nthem to slowly discharge.\n\n"

    "Shields are critical, as damage you take during these battles will stay with you for the duration of the game. There are very few opportuniteis to fix your ship.\n\n"
    
    "You only have 8 power to distribute amongst the various subsystems, so you must be stratigic about your priorities.\n\n"
    
    "\n-----Controls-----\n"
    "(Space) Pause/unpause Game\n"
    "   [while paused](I) Show this instructions overviewScreen\n"
    "(Enter) Jump out of sector (only works when engines are fully charged)\n"
    "Power Distribution:\n"
    "   The below keys add 1 power to the associated subsystem:\n"
    "       e - Engines\n"
    "       s - Shields\n"
    "       w - Weapons\n"
    "       p - Helm (Pilot)\n"
    "   Pressing Shift + any of the above keys removes 1 power from the associated subsystem\n"
    "\nPress any key to exit this screen";

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
    
    /* Info screen */
    int infoWidth = gameState.engine->width / 2;
    int infoHeight = gameState.engine->height / 2;
    baseMissionScreenState.infoScreen = createTextBox(baseMissionInstructions, 0, true, infoWidth, infoHeight, 0, 0, 10, gameState.engine);
    centerObject((Object*)baseMissionScreenState.infoScreen, gameState.baseMissionScreen, infoWidth, infoHeight);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.infoScreen);
    // by default events go to info screen
    gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEventsInfoScreen;

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
    int objectivesWidth = 57;
    baseMissionScreenState.objectivesTextBox = createTextBox("Scout Sector for Resistance [Done]\n"
            "    +10\% strength to resistance fleet"
            , 0, false, objectivesWidth, gameState.engine->height - 2 /* subtract 2 for border*/, 1, 1, 5, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.objectivesTextBox);
    /* Player Ship */
    baseMissionScreenState.shipObject = createPlayerShip(130, 5, 3, gameState.engine);
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
    
    // status indicators
    baseMissionScreenState.enemyWeaponsProgressBar = createProgressBar("", .33, 0, 17, 168, 45, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.enemyWeaponsProgressBar);
    baseMissionScreenState.alienStrengthProgressBar = createProgressBar("Alien Strength: ", .33, 0, 50, 160, 40, 2, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.alienStrengthProgressBar);



    /* Update listeners */
    gameState.baseMissionScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Release lock */
    unlockThreadLock(&baseMissionScreenStateLock);
}

void refreshBaseMissionScreen(){
    ShipData* shipData = (ShipData*)baseMissionScreenState.shipObject->userData;
    EnemyBaseData* enemyData = (EnemyBaseData*)baseMissionScreenState.enemyBase->userData;

    /* Mode */
    switch (baseMissionScreenState.mode){
    case MODE_NORMAL:
        baseMissionScreenState.modeTextBox->objectProperties.show = false;
        break;
    case MODE_PAUSED:
        baseMissionScreenState.modeTextBox->objectProperties.show = true;
        updateTextBox(baseMissionScreenState.modeTextBox, " { PAUSED - (I) Instructions } ", 0, true);
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
    int fullChargeAttr = COLOR_PAIR(getColorPair(green, black, gameState.engine));
    // engine charge
    updateProgressBar(baseMissionScreenState.engineChargeProgressBar, shipData->engineCharge, (shipData->engineCharge >= 1.0f)?fullChargeAttr:0);
    // ship health
    updateProgressBar(baseMissionScreenState.shipHelathProgressBar, gameState.shipHealth / 100.0f, 0);
    // weapons
    updateProgressBar(baseMissionScreenState.weapon1ChargeProgressBar, shipData->weapons1Charge, 0);
    updateProgressBar(baseMissionScreenState.weapon2ChargeProgressBar, shipData->weapons2Charge, 0);

    /* Enemy stats */
    // weapons
    updateProgressBar(baseMissionScreenState.enemyWeaponsProgressBar, enemyData->weaponsCharge, 0);
    // TEMP - alien strength
    updateProgressBar(baseMissionScreenState.alienStrengthProgressBar, gameState.alienStrenth / 100.0f, 0);
}

void updateBaseMissionScreen(Mission* mission){
    ShipData* shipData = (ShipData*)baseMissionScreenState.shipObject->userData;
    EnemyBaseData* enemyData = (EnemyBaseData*)baseMissionScreenState.enemyBase->userData;

    // reset the field and load info from the given mission
    
    // start paused
    baseMissionScreenState.mode = MODE_PAUSED;

    shipData->totalPower = 12; // you have 12 total power
    shipData->availablePower = 9; // of which you can use 8
    shipData->usedPower = 0;
    shipData->enginePower = 0;
    shipData->shieldPower = 0;
    shipData->weaponsPower = 0;
    shipData->pilotPower = 0;

    shipData->engineCharge = 0;
    shipData->weapons1Charge = 0;
    shipData->weapons2Charge = 0;

    // enemy base data
    enemyData->weaponsCharge = 0;

    // This is called upon entering a mission, so we can update any objectives here
    // scout sector objective: +10% to fleet strength upon entering system
    gameState.fleetStrength += 10;

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
                    // lower by one if we're above 0
                    if (shipData->enginePower > 0){
                        shipData->enginePower--;
                        shipData->usedPower--;
                    }
                } else {
                    // if we have enough power, and are under max (3), raise by 1
                    if ((shipData->availablePower - shipData->usedPower > 0) && (shipData->enginePower < 3)){
                        shipData->enginePower++;
                        shipData->usedPower++;
                    }
                }

                // screen needs to be redrawn to update power distribution indicators 
                redraw = true;
                break;
            
            // Shields
            case 'S':
                shift = true;
            case 's':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one if we're above 0
                    if (shipData->shieldPower > 0){
                        shipData->shieldPower--;
                        shipData->usedPower--;
                    }
                } else {
                    // if we have enough power, and are under max (3), raise by 1
                    if ((shipData->availablePower - shipData->usedPower > 0) && (shipData->shieldPower < 3)){
                        shipData->shieldPower++;
                        shipData->usedPower++;
                    }
                }

                // screen needs to be redrawn to update power distribution indicators 
                redraw = true;
                break;
            
            // Weapons
            case 'W':
                shift = true;
            case 'w':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one if we're above 0
                    if (shipData->weaponsPower > 0){
                        shipData->weaponsPower--;
                        shipData->usedPower--;
                    }
                } else {
                    // if we have enough power, and are under max (3), raise by 1
                    if ((shipData->availablePower - shipData->usedPower > 0) && (shipData->weaponsPower < 3)){
                        shipData->weaponsPower++;
                        shipData->usedPower++;
                    }
                }

                // screen needs to be redrawn to update power distribution indicators 
                redraw = true;
                break;

            // Pilot
            case 'P':
                shift = true;
            case 'p':
                // if shift is pressed, lower by 1, otherwise raise by 1
                if (shift){
                    // lower by one if we're above 0
                    if (shipData->pilotPower > 0){
                        shipData->pilotPower--;
                        shipData->usedPower--;
                    }
                } else {
                    // if we have enough power, and are under max (3), raise by 1
                    if ((shipData->availablePower - shipData->usedPower > 0) && (shipData->pilotPower < 3)){
                        shipData->pilotPower++;
                        shipData->usedPower++;
                    }
                }

                // screen needs to be redrawn to update power distribution indicators 
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
                    // if we're on the last sector, end game
                    if (gameState.currentSector >= 8){
                        // finished last sector, show win/lose screen based on how many sectors are green/red
                        if (gameState.fleetStrength > gameState.alienStrenth){
                            // you win
                            // update game over screen
                            updateGameOverScreen(ENDING_CRITICALSUCCESS);

                            // switch to game over screen
                            gameState.engine->mainPanel->childrenList = (Object*)gameState.gameOverScreen;
                            gameState.engine->mainPanel->listeners = gameState.gameOverScreenListenerList;
                        } else {
                            // you lose
                            // update game over screen
                            updateGameOverScreen(ENDING_CRITICALFAIL);

                            // switch to game over screen
                            gameState.engine->mainPanel->childrenList = (Object*)gameState.gameOverScreen;
                            gameState.engine->mainPanel->listeners = gameState.gameOverScreenListenerList;
                        }
                    } else {
                        // go back to overview screen
                        /* Make sure overview screen is updated */
                        updateOverviewScreen();

                        /* Move to overview screen */
                        gameState.engine->mainPanel->childrenList = (Object*)gameState.overviewScreen;
                        gameState.engine->mainPanel->listeners = gameState.overviewScreenListenerList;
                    }
                }
                break;
        }
    }
    /* if the event is a timer event, update values that change with time */
    else if (event->eventType.values.timerEvent){
        /* Calculate how much charge to add to engines */
        // more power in the engines causes faster charging,
        // as does more power to the pilot
        float dEdt = .01f * (shipData->enginePower) * (shipData->pilotPower);

        // if we have full engines and pilot power and are in demo mode, make dEdt very high so we can skip the wait
        if ((shipData->enginePower + shipData->pilotPower == 6) && (gameState.difficulty == 0)){
            dEdt = .4f;
        }

        // dE | deltat ms |   1 s   | = deltaE
        // --------------------------
        // dt |     1     | 1000 ms |
        int deltat = (int)event->eventData; // ammount of elapsed ms since last update
        shipData->engineCharge += dEdt*((float)deltat/1000.0f);
        if (shipData->engineCharge >= 1.0f){
            shipData->engineCharge = 1.0f;
        }

        /* Calculate weapons charge */
        // dW1dt and dW2dt describe how much each weapon (1 being lasers, 2 being missiles)
        // should charge (charge/time).
        // Behaviour:
        // 0 power to weapons:
        //      both weapons discharge - lasers slowly, missiles fast
        // 1 power to weapons:
        //      lasers charge slowly
        //      missiles discharge slowly
        // 2 power to weapons:
        //      lasers charge fast
        //      missiles charge slowly
        // 3 power to weapons:
        //      lasers charge very fast
        //      missiles charge fast
        float dW1dt = .08f * ((float)shipData->weaponsPower - 0.75f);
        float dW2dt = .07f * ((float)shipData->weaponsPower - 1.25f);

        // calculate new values for weapon charges - uses the same calculation as above for engine charge, deltat is the same
        shipData->weapons1Charge += dW1dt*((float)deltat/1000.0f);
        shipData->weapons2Charge += dW2dt*((float)deltat/1000.0f);
        
        // If weapon charge goes above 1 and we have a target then fire, otherwise cap at 1
        if (shipData->weapons1Charge >= 1.0f){
            bool target = true; // hardcoded for now - will depend on room target later
            if (target){
                int damage = 3; // 2% damage to alien strength
                gameState.alienStrenth -= damage;
                // reset weapons
                shipData->weapons1Charge = 0.0f;
            } else {
                // cap weapons charge at 1
                shipData->weapons1Charge = 1.0f;
            }
        }
        if (shipData->weapons2Charge >= 1.0f){
            bool target = true; // hardcoded for now - will depend on room target later
            if (target){
                int damage = 6; // 5% damage to alien strength
                gameState.alienStrenth -= damage;
                // reset weapons
                shipData->weapons2Charge = 0.0f;
            } else {
                // cap weapons charge at 1
                shipData->weapons2Charge = 1.0f;
            }
        }
        // If weapon charge goes below 0 cap at 0
        if (shipData->weapons1Charge <= 0.0f){
            shipData->weapons1Charge = 0.0f;
        }
        if (shipData->weapons2Charge <= 0.0f){
            shipData->weapons2Charge = 0.0f;
        }

        /* Calculate enemy weapons chage */
        // charge/s is .1 * (random number between 0 and 1 - biased towards higher numbers)
        float dEWdt = .3 * (1 - pow((float)rand() / (float)RAND_MAX, 4));
        ((EnemyBaseData*)baseMissionScreenState.enemyBase->userData)->weaponsCharge += dEWdt*((float)deltat/1000.0f);
        if (((EnemyBaseData*)baseMissionScreenState.enemyBase->userData)->weaponsCharge >= 1.0f){
            ((EnemyBaseData*)baseMissionScreenState.enemyBase->userData)->weaponsCharge = 1.0f;
        }

        /* If the enemy weapons are fully charged, fire on our ship */
        if (((EnemyBaseData*)baseMissionScreenState.enemyBase->userData)->weaponsCharge >= 1.0f){
            int damage = 25;
            // reduce damage by 7*shield power
            damage -= 7*shipData->shieldPower;

            // deal damage
            gameState.shipHealth -= damage;

            // if health is at 0, end game
            if (gameState.shipHealth <= 0){
                // update game over screen
                updateGameOverScreen(ENDING_CRITICALFAIL);

                // switch to game over screen
                gameState.engine->mainPanel->childrenList = (Object*)gameState.gameOverScreen;
                gameState.engine->mainPanel->listeners = gameState.gameOverScreenListenerList;
            }

            // reset enemy weapons charge
            ((EnemyBaseData*)baseMissionScreenState.enemyBase->userData)->weaponsCharge = 0.0f;
        }

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
            // show instructions
            case 'I':
            case 'i':
                // show info object, switch event handler, refresh screen
                baseMissionScreenState.infoScreen->objectProperties.show = true;
                gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEventsInfoScreen;
                refreshBaseMissionScreen();
                break;

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

void baseMissionScreenHandleEventsInfoScreen(Object* screen, Event* event){
    // we're only interested in keyboard events
    if (event->eventType.values.keyboardEvent){
        // on any keyboard event hide the info window and go to pause state
        baseMissionScreenState.infoScreen->objectProperties.show = false;
        baseMissionScreenState.mode = MODE_PAUSED;
        gameState.baseMissionScreen->objectProperties.handleEvent = baseMissionScreenHandleEventsPaused;
        refreshBaseMissionScreen();
    }
}
