/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __GAMEOVERSCREEN_H__
#define __GAMEOVERSCREEN_H__

#include <engine.h>
#include <AlcubierreGame.h>
#include <objects/sprites.h>

typedef enum GameResult_e{
    ENDING_CRITICALFAIL,
    ENDING_LIMITEDFAIL,
    ENDING_LIMITEDSUCCESS,
    ENDING_CRITICALSUCCESS,
} GameResult;

typedef struct GameOverScreenState_s{
    GameObject* backgroundTexture;

    // Game Over Text
    GameObject* gameOverTextBox;
} GameOverScreenState;
extern GameOverScreenState gameOverScreenState;
extern ThreadLock_t gameOverScreenStateLock;

/* Main functions */
void buildGameOverScreen();
void updateGameOverScreen(GameResult result);

/* Handle events */
void gameOverScreenHandleEvents(Object* screen, Event* event);

#endif //__GAMEOVERSCREEN_H_
