/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __ENEMYBASE_H__
#define __ENEMYBASE_H__

#include <engine.h>
#include <objects/Room.h>
#include <xpFunctions.h>

typedef struct EnemyBaseData_s{
    /* Rendering data */
    RoomData* controlRoom;
    RoomData* weaponsRoom;
    RoomData* storageRoom;
    RoomData* landingRoom;
    CursesChar* buffer;
    int bufferWidth, bufferHeight;
    XPFile* texture;

    /* Game data */
    float weaponsCharge;
} EnemyBaseData;

GameObject* createEnemyBase(int x, int y, int z, Engine* engine);
void destroyEnemyBase(GameObject* base);

void updateEnemyBase(GameObject* base, Engine* engine);

#endif //__ENEMYBASE_H__
