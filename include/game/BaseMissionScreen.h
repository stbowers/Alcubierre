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

typedef struct BaseMissionScreenState_s{
    GameObject* backgroundTexture;
} BaseMissionScreenState;
extern BaseMissionScreenState baseMissionScreenState;
extern ThreadLock_t baseMissionScreenStateLock;

/* Main functions */
void buildBaseMissionScreen();
void updateBaseMissionScreen();

#endif //__BASEMISSIONSCREEN_H_
