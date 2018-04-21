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

    Panel* titleScreen;
    Panel* overviewScreen;

    /* Listener lists - overwrite lister list for the active panel
     * allowing our game to easily switch modes
     */
    EventListener* titleScreenListenerList;
    EventListener* overviewScreenListenerList;

    /* World State */
    enum LocationSate_e{
        LOCATION_UNKNOWN,
        LOCATION_CURRENT,
        LOCATION_COMPLETED,
        LOCATION_SKIPPED,
    } locations[9];
} AlcubierreGameState;
extern AlcubierreGameState gameState;
extern ThreadLock_t gameStateMutex;

void startGame(Engine* engine);
// Should be called before destroyEngine to clean up any resources we own.
void cleanUpGame();

void playCallback();
void infoCallback();
void backstoryCallback();
void exitCallback();

void initializeWorldState();
void runIntroSequence();
void buildTitleScreen();
void buildOverviewScreen();

void updateTitleScreen();
void updateOverviewScreen();

#endif //__ALCUBIERREGAME_H__
