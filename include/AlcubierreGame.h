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

#define MAX_MISSION_TITLE 30
/* Structs for various game state objects */
typedef struct Objective_s{
    enum {
        /* Scouting objectives */
        OBJECTIVE_SCOUT, // basic scout sector, +fleet strength
        /* Attack objectives */
        OBJECTIVE_DESTROYWEAPONS, // destroy the enemy's weapons systems, -alien strength
        OBJECTIVE_CRIPPLESHIPPING, // destroy the enemy's landing pad, -alien strenght
        OBJECTIVE_DESTROYRESOURCES, // destroy the enemy's storeroom, -alien strength
        OBJECTIVE_CRIPPLEINTEL, // destroy the enemy's control room, -alien strength
    } objectiveType;
} Objective;

typedef struct Mission_s{
    char missionTitle[MAX_MISSION_TITLE];
    enum {
        MISSION_BASE,
        MISSION_STATION,
        MISSION_STORE,
        MISSION_STARGATE,
    } missionType;

    /* Each mission has one scouting objective, one attack objective, and on aid objective */
    Objective objectives[3];
} Mission;

// Stores the game state - used by AlcubierreGame.c not by engine
typedef struct AlcubierreGameState_s{
    Engine* engine;

    // used to send a signal to the main thread that the game should exit now
    bool exit;

    /* Screens - each has its own .h and .c file in the game directory */
    Panel* titleScreen;
    Panel* overviewScreen;
    Panel* baseMissionScreen;
    Panel* stationMissionScreen;
    Panel* storeScreen;
    Panel* gameOverScreen;

    /* Listener lists - overwrite lister list for the active panel
     * allowing our game to easily switch modes
     */
    EventListener* titleScreenListenerList;
    EventListener* overviewScreenListenerList;
    EventListener* baseMissionScreenListenerList;
    EventListener* stationMissionScreenListenerList;
    EventListener* storeScreenListenerList;
    EventListener* gameOverScreenListenerList;

    /* Game State */
    /* There are 15 difficulty levels; 3 for easy, 3 for medium, 3 for hard, and 6 are above hard, but can't be chosen as a starting difficulty
     * Easy starts the game at 1
     * Medium starts the game at 4
     * Hard starts the game at 7
     * each sector increases the difficulty, so 
     * easy ends the game on 9
     * medium ends the game on 12
     * hard ends the game 15
     *
     * there is also a 'reserved' difficulty of 0 used for demos and testing, which means you don't take damage, and the enemy is always set to easy
     */
    int difficulty;

    /* World State */
    enum LocationSate_e{
        LOCATION_UNKNOWN,
        LOCATION_CURRENT,
        LOCATION_COMPLETED,
        LOCATION_SKIPPED,
    } locations[9];
    int currentSector;

    /* Stores the missions available at each location
     * - three missions for every sector, 9 sectors -
     * [sector][mission]
     */
    Mission missions[9][3];

    /* Stats */
    int shipHealth;
    int fleetStrength;
    int alienStrenth;
    int credits;

} AlcubierreGameState;
extern AlcubierreGameState gameState;
extern ThreadLock_t gameStateLock;

void startGame(Engine* engine, bool skipIntro);
// Should be called before destroyEngine to clean up any resources we own.
void cleanUpGame();

void initializeWorldState();
void runIntroSequence();

#endif //__ALCUBIERREGAME_H__
