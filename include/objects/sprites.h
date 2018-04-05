/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Definitions for sprite objects */
#ifndef __SPRITE_H__
#define __SPRITE_H_

#include <engine.h>

/* Objects in this file are only described by createObject() and destroyObject()
 * methods, which create and manage an object (or window) struct with preset
 * function pointers and data providing the resources needed to produce
 * the specified functionality.
 */

/* XP Sprite - sprite based on .xp file */

typedef struct XPSpriteData_s{
    
} XPSpriteData;

Object* createXPSprite(const char* file);
void destroyXPSprite(Object* sprite);

#endif //__SPRITE_H_
