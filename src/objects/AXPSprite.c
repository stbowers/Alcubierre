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
void AXPSpriteDraw(Object* self, CursesChar* buffer);

/* Implementation of sprites.h functions */

GameObject* createAXPSprite(AXPFile* texture, int xpos, int ypos, Engine* engine){
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
    newObject->objectProperties.z = 1;
    newObject->objectProperties.show = true;

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

    /* Create panel for each frame */
  //data->textureData->frames = (Panel**) malloc(sizeof(Panel*) * texture->xpFile->numLayers);
  //for (int layer = 0; layer < texture->xpFile->numLayers; layer++){
  //    data->textureData->frames[layer] = createPanel(data->textureData->width, data->textureData->height, 0, 0, 0);
  //    /* Draw layer */
  //    drawLayerToPanel(&texture->xpFile->layers[layer], data->textureData->frames[layer], false); 
  //}

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

  ///* Get mutex */
  //pthread_mutex_lock(&data->spriteMutex);

  ///* Hide other frames */
  //for (int frame = 0; frame < data->texture->xpFile->numLayers; frame++){
  //    hide_panel(data->textureData->frames[frame]->panel);
  //}

  ///* Render current frame */
  //int currentFrame = data->textureData->currentFrame;
  //top_panel(data->textureData->frames[currentFrame]->panel);
  //show_panel(data->textureData->frames[currentFrame]->panel);

  ///* Release mutex */
  //pthread_mutex_unlock(&data->spriteMutex);
}
