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
void XPSpriteDraw(Object* self, Panel* panel);
void moveXPSprite(Object* self, int absX, int absY);

/* Implementation of sprites.h functions */

GameObject* createXPSprite(XPFile* texture, int xpos, int ypos, int z){
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
    newObject->objectProperties.moveAbsolute = moveXPSprite;
    newObject->objectProperties.show = true;

    /* Game Object properties */
    XPSpriteData* data = (XPSpriteData*) malloc(sizeof(XPSpriteData));
    newObject->userData = data;

    data->texture = texture;
    data->textureData = (XPSpriteTextureData*) malloc(sizeof(XPSpriteTextureData));
    data->textureData->width = texture->layers[0].width;
    data->textureData->height = texture->layers[0].height;

    /* Create panel */
    data->textureData->panel = createPanel(data->textureData->width, data->textureData->height, xpos, ypos, 0);
    for (int layer = 0; layer < texture->numLayers; layer++){
        /* Draw layer */
        drawLayerToPanel(&texture->layers[layer], data->textureData->panel, false); 
    }

    return newObject;
}

void destroyXPSprite(GameObject* sprite){
    free(((XPSpriteData*)sprite->userData)->textureData);
    free(sprite->userData);
    free(sprite);
}

void moveXPSprite(Object* self, int x, int y){
    XPSpriteData* data = (XPSpriteData*)((GameObject*)self)->userData;
    move_panel(data->textureData->panel->panel, y, x);
}

/* Implementation of custom functions */
void XPSpriteUpdate(Object* self){

}

void XPSpriteDraw(Object* self, Panel* panel){
    XPSpriteData* data = (XPSpriteData*)((GameObject*)self)->userData;
    top_panel(data->textureData->panel->panel);
    show_panel(data->textureData->panel->panel);
}
