/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation of the animated AXP sprite (sprites.h) */

#include <objects/sprites.h>
#include <stdlib.h>
#include <string.h>

/* GameObject functions */
void AXPSpriteDraw(Object* self, CursesChar* buffer);

/* Implementation of sprites.h functions */

GameObject* createAXPSprite(XPFile** textures, int numFrames, int msPerFrame, int xpos, int ypos, int zorder, Engine* engine){
    /* Create Game Object */
    GameObject* newObject = (GameObject*)malloc(sizeof(GameObject));
    newObject->timeCreated = getTimems();
    
    /* Base Object Properties */
    newObject->objectProperties.drawObject = AXPSpriteDraw;
    newObject->objectProperties.handleEvent = NULL;
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = xpos;
    newObject->objectProperties.y = ypos;
    newObject->objectProperties.z = zorder;
    newObject->objectProperties.show = true;

    /* Game Object properties */
    newObject->timeCreated = getTimems();
    AXPSpriteData* data = (AXPSpriteData*) malloc(sizeof(AXPSpriteData));
    newObject->userData = data;

    // copy the array of xpfile pointers
    data->textures = (XPFile**) malloc(sizeof(XPFile*) * numFrames);
    memcpy(data->textures, textures, sizeof(XPFile*)*numFrames);
    data->textureData = (AXPSpriteTextureData*) malloc(sizeof(AXPSpriteTextureData));
    data->textureData->width = textures[0]->layers[0].width;
    data->textureData->height = textures[0]->layers[0].height;
    data->currentFrame = 0;
    data->lastFrameTime = getTimems();
    data->numFrames = numFrames;
    data->msPerFrame = msPerFrame;

    /* Create buffer for each frame */
    data->textureData->frames = (CursesChar**) malloc(sizeof(CursesChar*) * numFrames);
    
    for (int layer = 0; layer < numFrames; layer++){
        /* Allocate buffer */
        data->textureData->frames[layer] = (CursesChar*) malloc(sizeof(CursesChar) * textures[0]->layers[0].width * textures[0]->layers[0].height);

        /* Initialize texture buffer with transparent cells */
        for (int x = 0; x < data->textureData->width; x++){
            for (int y = 0; y < data->textureData->height; y++){
                CursesChar* backgroundChar = &data->textureData->frames[layer][(data->textureData->height * x) + y];
                backgroundChar->attributes = 0;
                // transparent cell denoted by NBSP unicode character
                backgroundChar->character = L'\u00A0';
            }
        }
        /* Draw texture to buffer */
        drawLayerToBuffer(&textures[layer]->layers[0], data->textureData->frames[layer], true, engine);
    }

    return newObject;
}

void destroyAXPSprite(GameObject* sprite){
    AXPSpriteData* data = (AXPSpriteData*)sprite->userData;

    /* Free frames */
    free(data->textureData->frames);

    /* Free texture data */
    free(data->textureData);

    /* Free sprite data struct */
    free(data);

    /* Free the sprite itself */
    free(sprite);
}

/* Implementation of custom functions */
void AXPSpriteDraw(Object* self, CursesChar* buffer){
    AXPSpriteData* data = (AXPSpriteData*)((GameObject*)self)->userData;

    // If enough time has passed, move on to the next frame
    if ((getTimems() - data->lastFrameTime) >= (data->msPerFrame)){
        data->currentFrame++;
        data->lastFrameTime = getTimems();
        if (data->currentFrame >= data->numFrames){
            data->currentFrame = 0;
        }
    }

    // Draw frame[currentFrame]
    /* Add chars from frame to buffer */
    CursesChar* frame = data->textureData->frames[data->currentFrame];
    for (int x = 0; x < data->textureData->width; x++){
        for (int y = 0; y < data->textureData->height; y++){
            CursesChar* textureChar = &frame[(data->textureData->height * x) + y];
            // if char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (textureChar->character != L'\u00A0'){
                writecharToBuffer(buffer, x, y, textureChar);
            }
        }
    }
}
