/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __ROOM_H__
#define __ROOM_H__

#include <engine.h>

/* Not used as a full object - but a common set of data used by other objects
 * to draw a room and keep track of it's health
 */

typedef struct RoomData_s{
    wchar_t roomSymbol;
    float health; // 0-1
    void* userData; // used with the callback
    void (*roomDamagedCallback)(void* userData, float health);
} RoomData;

void drawRoom(RoomData* room, CursesChar* buffer);

#endif //__ROOM_H__
