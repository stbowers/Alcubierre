/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __SHIP_H__
#define __SHIP_H__

#include <engine.h>
#include <objects/Room.h>
#include <xpFunctions.h>

typedef struct ShipData_s{
    /* Rendering data */
    RoomData* engineRoom;
    RoomData* shieldRoom;
    RoomData* weaponsRoom;
    RoomData* pilotRoom;
    CursesChar* buffer;
    int bufferWidth, bufferHeight;
    XPFile* texture;

    /* Game data */
    // power
    int totalPower;
    int availablePower;
    int usedPower;
    int enginePower;
    int shieldPower;
    int weaponsPower;
    int pilotPower;

    float engineCharge;
} ShipData;

GameObject* createPlayerShip(int x, int y, int z, Engine* engine);
void destroyPlayerShip(GameObject* ship);

void updatePlayerShip(GameObject* ship, Engine* engine);

#endif //__SHIP_H__
