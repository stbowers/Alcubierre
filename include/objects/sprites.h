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
    CursesChar* textureBuffer;
} XPSpriteTextureData;

typedef struct XPSpriteData_s{
    XPFile* texture; // Original texture file
    XPSpriteTextureData* textureData; // Optimized data for rendering
} XPSpriteData;

GameObject* createXPSprite(XPFile* texture, int xpos, int ypos, int zorder, Engine* engine);
void destroyXPSprite(GameObject* sprite);

/* AXP Sprite - animated sprite */

typedef struct AXPSpriteTextureData_s{
    int width, height, currentFrame;
    CursesChar** frames; // array of buffers for each frame
} AXPSpriteTextureData;

typedef struct AXPSpriteData_s{
    AXPFile* texture; // Original texture file
    AXPSpriteTextureData* textureData; // Data optimized for rendering
    int fps; // fps this animation runs at
} AXPSpriteData;

GameObject* createAXPSprite(AXPFile* texture, int xpos, int ypos, Engine* engine);
void destroyAXPSprite(GameObject* sprite);

#endif //__SPRITE_H_
