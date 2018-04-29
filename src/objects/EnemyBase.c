/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <engine.h>
#include <stdlib.h>
#include <xpFunctions.h>
#include <objects/EnemyBase.h>

static void defaultEnemyBaseDraw(Object* self, CursesChar* buffer);

GameObject* createEnemyBase(int x, int y, int z, Engine* engine){
    GameObject* newObject = (GameObject*) malloc(sizeof(GameObject));

    /* Object Properties */
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.show = true;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = x;
    newObject->objectProperties.y = y;
    newObject->objectProperties.z = z;
    newObject->objectProperties.drawObject = defaultEnemyBaseDraw;
    newObject->objectProperties.handleEvent = NULL;

    /* User data */
    EnemyBaseData* data = (EnemyBaseData*) malloc(sizeof(EnemyBaseData));
    newObject->userData = data;

    // default game data
    data->weaponsCharge = 0;

    // load texture
    data->texture = getXPFile("./assets/EnemyBase.xp");

    // set up buffer
    data->bufferWidth = data->texture->layers[0].width;
    data->bufferHeight = data->texture->layers[0].height;
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar)*data->bufferWidth*data->bufferHeight);

    // create rooms
    data->controlRoom = (RoomData*) malloc(sizeof(RoomData));
    data->storageRoom = (RoomData*) malloc(sizeof(RoomData));
    data->weaponsRoom = (RoomData*) malloc(sizeof(RoomData));
    data->landingRoom = (RoomData*) malloc(sizeof(RoomData));

    data->controlRoom->health = 1;
    data->controlRoom->roomSymbol = L'Φ';
    data->controlRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->controlRoom->userData = NULL;

    data->storageRoom->health = 1;
    data->storageRoom->roomSymbol = L'Θ';
    data->storageRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->storageRoom->userData = NULL;

    data->weaponsRoom->health = 1;
    data->weaponsRoom->roomSymbol = L'φ';
    data->weaponsRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->weaponsRoom->userData = NULL;

    data->landingRoom->health = 1;
    data->landingRoom->roomSymbol = L'δ';
    data->landingRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->landingRoom->userData = NULL;

    // draw ship to buffer
    updateEnemyBase(newObject, engine);

    return newObject;
}

void updateEnemyBase(GameObject* ship, Engine* engine){
    EnemyBaseData* data = ((EnemyBaseData*)ship->userData);
    // draw texture to buffer
    drawLayerToBuffer(&data->texture->layers[0], data->buffer, false, engine);

    // draw rooms to buffer
    drawRoom(data->controlRoom, data->buffer);
    drawRoom(data->storageRoom, data->buffer);
    drawRoom(data->weaponsRoom, data->buffer);
    drawRoom(data->landingRoom, data->buffer);
}

void destroyEnemyBase(GameObject* ship){
    EnemyBaseData* data = ((EnemyBaseData*)((GameObject*)ship)->userData);

    free(data->controlRoom);
    free(data->storageRoom);
    free(data->weaponsRoom);
    free(data->landingRoom);

    free(data->buffer);

    freeXPFile(data->texture);

    free(data);

    free(ship);
}

void defaultEnemyBaseDraw(Object* self, CursesChar* buffer){
    EnemyBaseData* data = (EnemyBaseData*)((GameObject*)self)->userData;

    /* Draw buffer */
    for (int x = 0; x < data->bufferWidth; x++){
        for (int y = 0; y < data->bufferHeight; y++){
            CursesChar* bufferChar = &data->buffer[(data->bufferHeight * x) + y];
            // if char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (bufferChar->character != L'\u00A0'){
                writecharToBuffer(buffer, x, y, bufferChar);
            }
        }
    }
}
