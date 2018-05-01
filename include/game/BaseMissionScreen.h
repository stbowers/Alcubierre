/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __BASEMISSIONSCREEN_H__
#define __BASEMISSIONSCREEN_H__

#include <engine.h>
#include <AlcubierreGame.h>
#include <objects/sprites.h>
#include <objects/Room.h>

typedef struct BaseMissionScreenState_s{
    GameObject* backgroundTexture;

    // info screen
    GameObject* infoScreen;

    // side screen
    GameObject* objectivesTextBox;

    // mode text box
    GameObject* modeTextBox;
    enum {
        MODE_NORMAL,
        MODE_PAUSED,
        MODE_TARGET_ENEMY,
        MODE_TARGET_ASSIST,
    } mode;

    // Ship
    GameObject* shipObject;
    RoomData* playerEngineRoom;
    RoomData* playerShieldRoom;
    RoomData* playerWeaponsRoom;
    RoomData* playerPilotRoom;

    // enemy base
    GameObject* enemyBase;
    RoomData* enemyBaseCommandRoom;
    RoomData* enemyBaseWeaponsRoom;
    RoomData* enemyBaseLandingPadRoom;
    RoomData* enemyBaseStorageRoom;

    // player status
    GameObject* engineChargeProgressBar;
    GameObject* shipHelathProgressBar;
    GameObject* weapon1ChargeProgressBar;
    GameObject* weapon2ChargeProgressBar;
    
    // power
    GameObject* enginePowerProgressBar;
    GameObject* shieldPowerProgressBar;
    GameObject* weaponsPowerProgressBar;
    GameObject* pilotPowerProgressBar;
    GameObject* unusedPowerProgressBar;

    // Ally base
    GameObject* allyBaseObject;

    // Enemy base
    GameObject* enemyBaseObject;
    RoomData* enemyCommandRoom;
    RoomData* enemyWeaponsRoom;
    RoomData* enemyStorageRoom;

    // Enemy Status
    GameObject* enemyHealthProgressBar;
    GameObject* enemyWeaponsProgressBar;
    GameObject* alienStrengthProgressBar;

    // Targets
    RoomData* playerWeaponsTarget;
    RoomData* playerAssistTarget;
    RoomData* enemyWeaponsTarget;
} BaseMissionScreenState;
extern BaseMissionScreenState baseMissionScreenState;
extern ThreadLock_t baseMissionScreenStateLock;

/* Main functions */
void buildBaseMissionScreen();
void updateBaseMissionScreen();

/* Handle events */
void baseMissionScreenHandleEvents(Object* screen, Event* event);
// secondary listeners for different modes (paused, selection, etc)
void baseMissionScreenHandleEventsPaused(Object* screen, Event* event);
void baseMissionScreenHandleEventsInfoScreen(Object* screen, Event* event);
void baseMissionScreenHandleEventsTargetSelectionWeapons(Object* screen, Event* event);
void baseMissionScreenHandleEventsTargetSelectionPersonel(Object* screen, Event* event);

#endif //__BASEMISSIONSCREEN_H_
