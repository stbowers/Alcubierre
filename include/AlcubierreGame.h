/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __ALCUBIERREGAME_H__
#define __ALCUBIERREGAME_H__

#include <engine.h>

// Stores the game state - used by AlcubierreGame.c not by engine
typedef struct AlcubierreGameState_s{
    Engine* engine;
    GameObject* titleScreen;
    GameObject* overviewScreen;
} AlcubierreGameState;

void startGame(Engine* engine);

#endif //__ALCUBIERREGAME_H__
