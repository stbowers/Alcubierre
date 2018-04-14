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

/* GameObject functions */
void AXPSpriteUpdate(Object* self);
void AXPSpriteDraw(Object* self, Panel* panel);

/* Implementation of sprites.h functions */

GameObject* createAXPSprite(AXPFile* texture, int xpos, int ypos){
    /* Create Game Object */
    GameObject* newObject = (GameObject*)malloc(sizeof(GameObject));
    newObject->timeCreated = getTimems();
    
    /* Base Object Properties */
    newObject->objectProperties.drawObject = AXPSpriteDraw;
    newObject->objectProperties.update = AXPSpriteUpdate;
    newObject->objectProperties.handleEvent = NULL;
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;

    /* Game Object properties */
    newObject->timeCreated = getTimems();
    AXPSpriteData* data = (AXPSpriteData*) malloc(sizeof(AXPSpriteData));
    newObject->userData = data;

    data->texture = texture;
    data->textureData = (AXPSpriteTextureData*) malloc(sizeof(AXPSpriteTextureData));
    data->textureData->width = texture->xpFile->layers[0].width;
    data->textureData->height = texture->xpFile->layers[0].height;
    data->textureData->currentFrame = 0;
    data->fps = texture->fps;
    pthread_mutex_init(&data->spriteMutex, NULL);

    /* Create panel for each frame */
    data->textureData->frames = (Panel**) malloc(sizeof(Panel*) * texture->xpFile->numLayers);
    for (int layer = 0; layer < texture->xpFile->numLayers; layer++){
        data->textureData->frames[layer] = createPanel(data->textureData->width, data->textureData->height, 0, 0, 0);
        /* Draw layer */
        drawLayerToPanel(&texture->xpFile->layers[layer], data->textureData->frames[layer], false); 
    }

    /* Move the sprite */
    moveAXPSprite(newObject, xpos, ypos);

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

void moveAXPSprite(GameObject* self, int x, int y){
    AXPSpriteData* data = (AXPSpriteData*)self->userData;
    for (int layer = 0; layer < data->texture->xpFile->numLayers; layer++){
        move_panel(data->textureData->frames[layer]->panel, x, y);
    }
}

/* Implementation of custom functions */
void AXPSpriteUpdate(Object* self){
    uint64_t timePassed = ((GameObject*)self)->timeCreated - getTimems();
    AXPSpriteData* data = ((AXPSpriteData*)((GameObject*)self)->userData);
    
    /* Get mutex */
    pthread_mutex_lock(&data->spriteMutex);
    
    /* Calculate what frame we should be on */
    // timepassed ms |  fps frames  |   1 s   |  = frames passed  -> frames passed % number of frames = current frame
    //      1        |      s       | 1000 ms | 
    int framesPassed = (timePassed*data->fps)/(1000.0f);
    data->textureData->currentFrame++;//(framesPassed)%(data->texture->xpFile->numLayers);

    /* Unlock mutex */
    pthread_mutex_unlock(&data->spriteMutex);
}

void AXPSpriteDraw(Object* self, Panel* panel){
    AXPSpriteData* data = (AXPSpriteData*)((GameObject*)self)->userData;

    /* Get mutex */
    pthread_mutex_lock(&data->spriteMutex);

    /* Hide other frames */
    for (int frame = 0; frame < data->texture->xpFile->numLayers; frame++){
        hide_panel(data->textureData->frames[frame]->panel);
    }

    /* Render current frame */
    int currentFrame = data->textureData->currentFrame;
    top_panel(data->textureData->frames[currentFrame]->panel);
    show_panel(data->textureData->frames[currentFrame]->panel);

    /* Release mutex */
    pthread_mutex_unlock(&data->spriteMutex);
}
