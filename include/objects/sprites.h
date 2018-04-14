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
#include <xpFunctions.h>

/* Objects in this file are only described by createObject() and destroyObject()
 * methods, which create and manage an object (or window) struct with preset
 * function pointers and data providing the resources needed to produce
 * the specified functionality.
 */

/* XP Sprite - sprite based on .xp file */

typedef struct XPSpriteTextureData_s{
    int width, height;
    Panel* panel;
} XPSpriteTextureData;

typedef struct XPSpriteData_s{
    XPFile* texture; // Original texture file
    XPSpriteTextureData* textureData; // Optimized data for rendering
} XPSpriteData;

GameObject* createXPSprite(XPFile* texture, int xpos, int ypos);
void destroyXPSprite(GameObject* sprite);
// Move the sprite to absolute coordinates x, y
void moveXPSprite(GameObject* self, int x, int y);

/* AXP Sprite - animated sprite */

typedef struct AXPSpriteTextureData_s{
    int width, height, currentFrame;
    Panel** frames; // array of Panel* for each frame
} AXPSpriteTextureData;

typedef struct AXPSpriteData_s{
    AXPFile* texture; // Original texture file
    AXPSpriteTextureData* textureData; // Data optimized for rendering
    pthread_mutex_t spriteMutex; // lock data before modifying resources the render thread might use
    int fps; // fps this animation runs at
} AXPSpriteData;

GameObject* createAXPSprite(AXPFile* texture, int xpos, int ypos);
void destroyAXPSprite(GameObject* sprite);
void moveAXPSprite(GameObject* self, int x, int y);

#endif //__SPRITE_H_
