/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <game/BaseMissionScreen.h>

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
    
    /* Draw background */
    XPFile* backgroundTexture = getXPFile("./assets/BaseMissionScreen.xp");
    baseMissionScreenState.backgroundTexture = createXPSprite(backgroundTexture, 0, 0, 1, gameState.engine);
    gameState.baseMissionScreen->addObject(gameState.baseMissionScreen, (Object*)baseMissionScreenState.backgroundTexture);



    /* Update listeners */
    gameState.baseMissionScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Release lock */
    unlockThreadLock(&baseMissionScreenStateLock);
}

void updateBaseMissionScreen(){

}
