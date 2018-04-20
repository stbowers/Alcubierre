/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation of the basic XP sprite (sprites.h) */

#include <objects/sprites.h>
#include <stdlib.h>

/* GameObject functions */
void XPSpriteUpdate(Object* self);
void XPSpriteDraw(Object* self, cchar_t* buffer);

/* Implementation of sprites.h functions */

GameObject* createXPSprite(XPFile* texture, int xpos, int ypos, int z, Engine* engine){
    /* Create Game Object */
    GameObject* newObject = (GameObject*)malloc(sizeof(GameObject));
    newObject->timeCreated = getTimems();
    
    /* Base Object Properties */
    newObject->objectProperties.drawObject = XPSpriteDraw;
    newObject->objectProperties.update = XPSpriteUpdate;
    newObject->objectProperties.handleEvent = NULL;
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = xpos;
    newObject->objectProperties.y = ypos;
    newObject->objectProperties.z = z;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.show = true;

    /* Game Object properties */
    XPSpriteData* data = (XPSpriteData*) malloc(sizeof(XPSpriteData));
    newObject->userData = data;

    data->texture = texture;
    data->textureData = (XPSpriteTextureData*) malloc(sizeof(XPSpriteTextureData));
    data->textureData->width = texture->layers[0].width;
    data->textureData->height = texture->layers[0].height;

    /* Create texture buffer */
    data->textureData->textureBuffer = (cchar_t*) malloc(sizeof(cchar_t)*data->textureData->width*data->textureData->height);

    /* Initialize texture buffer with transparent cells */
    for (int x = 0; x < data->textureData->width; x++){
        for (int y = 0; y < data->textureData->height; y++){
            cchar_t* backgroundChar = &data->textureData->textureBuffer[(data->textureData->height * x) + y];
            backgroundChar->attr = 0;
            // transparent cell denoted by NBSP unicode character
            backgroundChar->chars[0] = L'\u00A0';
        }
    }

    /* Draw texture to buffer */
    for (int layer = 0; layer < data->texture->numLayers; layer++){
        drawLayerToBuffer(&data->texture->layers[layer], data->textureData->textureBuffer, true, engine);
    }

    return newObject;
}

void destroyXPSprite(GameObject* sprite){
    free(((XPSpriteData*)sprite->userData)->textureData->textureBuffer);
    free(((XPSpriteData*)sprite->userData)->textureData);
    free(sprite->userData);
    free(sprite);
}

/* Implementation of custom functions */
void XPSpriteUpdate(Object* self){

}

void XPSpriteDraw(Object* self, cchar_t* buffer){
    XPSpriteData* data = (XPSpriteData*)((GameObject*)self)->userData;

    /* Add chars from textureBuffer to buffer */
    for (int x = 0; x < data->textureData->width; x++){
        for (int y = 0; y < data->textureData->height; y++){
            cchar_t* textureChar = &data->textureData->textureBuffer[(data->textureData->height * x) + y];
            // if char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (textureChar->chars[0] != L'\u00A0'){
                writecharToBuffer(buffer, x, y, *textureChar);
            }
        }
    }
}
