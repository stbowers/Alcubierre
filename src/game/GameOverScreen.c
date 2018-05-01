/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <engine.h>
#include <game/GameOverScreen.h>
#include <objects/ui.h>

GameOverScreenState gameOverScreenState;
ThreadLock_t gameOverScreenStateLock;

/* Text */
char critFailText[] = "-----GAME OVER - You Lost----\n"
    "The resistance has been utterly decimated";
char limitedFailText[] = "-----GAME OVER - You Lost-----\n"
    "The resistance has failed to secure the alien Stargate. The future is uncertain.";
char limitedSuccessText[] = "-----GAME OVER - You Won-----\n"
    "We have captured the alien Stargate. Now the hard work of taking back our home begins.";
char critSuccessText[] = "-----GAME OVER - You Won-----\n"
    "We have defeated the aliens and taken back our home. Good work, Commander.";

/* Main functions */
void buildGameOverScreen(){
    /* Initialize lock and state */
    createLock(&gameOverScreenStateLock);
    lockThreadLock(&gameOverScreenStateLock);

    /* Remove listeners and reset new listener pointer */
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;
    
    
    /* Build Panel */
    gameState.gameOverScreen = createPanel(gameState.engine->width, gameState.engine->height, 0, 0, 1);

    /* Add text box */
    gameOverScreenState.gameOverTextBox = createTextBox("", 0, true, 100, 50, 0, 0, 2, gameState.engine);
    centerObject((Object*)gameOverScreenState.gameOverTextBox, gameState.gameOverScreen, 100, 50);
    gameState.gameOverScreen->addObject(gameState.gameOverScreen, (Object*)gameOverScreenState.gameOverTextBox);


    /* Update listeners */
    gameState.gameOverScreenListenerList = gameState.engine->mainPanel->listeners;
    gameState.engine->mainPanel->listeners = NULL;
    gameState.engine->mainPanel->nextListener = &gameState.engine->mainPanel->listeners;

    /* Release lock */
    unlockThreadLock(&gameOverScreenStateLock);
}

void updateGameOverScreen(GameResult result){
    switch (result){
        case ENDING_CRITICALFAIL:
            updateTextBox(gameOverScreenState.gameOverTextBox, critFailText, 0, false);
            break;
        case ENDING_LIMITEDFAIL:
            updateTextBox(gameOverScreenState.gameOverTextBox, limitedFailText, 0, false);
            break;
        case ENDING_LIMITEDSUCCESS:
            updateTextBox(gameOverScreenState.gameOverTextBox, limitedSuccessText, 0, false);
            break;
        case ENDING_CRITICALSUCCESS:
            updateTextBox(gameOverScreenState.gameOverTextBox, critSuccessText, 0, false);
            break;
    }
}

/* Handle events */
void gameOverScreenHandleEvents(Object* screen, Event* event);
