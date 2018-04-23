/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/StationMissionScreen.h>

void buildStationMissionScreen(){
    /* Initialize lock and state */
    createLock(&stationMissionScreenStateLock);
    lockThreadLock(&stationMissionScreenStateLock);

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
    
    /* Update listeners */
    gameState.stationMissionScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
    
    /* Release lock */
    unlockThreadLock(&stationMissionScreenStateLock);
}

void updateStationMissionScreen(){

}
