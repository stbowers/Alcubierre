/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __STORESCREEN_H__
#define __STORESCREEN_H__

#include <engine.h>
#include <AlcubierreGame.h>
#include <objects/sprites.h>

typedef struct StoreScreenState_s{
	GameObject* storeList;
} StoreScreenState;
extern StoreScreenState storeScreenState;
extern ThreadLock_t storeScreenStateLock;

/* Main functions */
void buildStoreScreen();
void updateStoreScreen();

#endif //__STORESCREEN_H_
