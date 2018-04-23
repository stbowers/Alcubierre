/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __STATIONMISSIONSCREEN_H__
#define __STATIONMISSIONSCREEN_H__

#include <engine.h>
#include <AlcubierreGame.h>
#include <objects/sprites.h>

typedef struct StationMissionScreenState_s{
	GameObject thing;
} StationMissionScreenState;
extern StationMissionScreenState stationMissionScreenState;
extern ThreadLock_t stationMissionScreenStateLock;

/* Main functions */
void buildStationMissionScreen();
void updateStationMissionScreen();

#endif //__STATIONMISSIONSCREEN_H_
