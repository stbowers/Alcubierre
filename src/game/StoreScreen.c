/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/StoreScreen.h>

void buildStoreScreen(){
    /* Initialize lock and state */
    createLock(&storeScreenStateLock);
    lockThreadLock(&storeScreenStateLock);

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
    
    /* Update listeners */
    gameState.storeScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Release lock */
    unlockThreadLock(&storeScreenStateLock);
}

void updateStoreScreen(){

}
