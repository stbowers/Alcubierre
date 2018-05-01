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
#include <objects/Ship.h>

static void defaultPlayerShipDraw(Object* self, CursesChar* buffer);

GameObject* createPlayerShip(int x, int y, int z, Engine* engine){
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
    newObject->objectProperties.drawObject = defaultPlayerShipDraw;
    newObject->objectProperties.handleEvent = NULL;

    /* User data */
    ShipData* data = (ShipData*) malloc(sizeof(ShipData));
    newObject->userData = data;

    // default game data
    data->engineCharge = 0;

    // load ship texture
    data->texture = getXPFile("./assets/Alcubierre.xp");

    // set up buffer
    data->bufferWidth = data->texture->layers[0].width;
    data->bufferHeight = data->texture->layers[0].height;
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar)*data->bufferWidth*data->bufferHeight);

    // create rooms
    data->engineRoom = (RoomData*) malloc(sizeof(RoomData));
    data->shieldRoom = (RoomData*) malloc(sizeof(RoomData));
    data->weaponsRoom = (RoomData*) malloc(sizeof(RoomData));
    data->pilotRoom = (RoomData*) malloc(sizeof(RoomData));

    data->engineRoom->health = 1;
    data->engineRoom->roomSymbol = L'Φ';
    data->engineRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->engineRoom->userData = NULL;

    data->shieldRoom->health = 1;
    data->shieldRoom->roomSymbol = L'Θ';
    data->shieldRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->shieldRoom->userData = NULL;

    data->weaponsRoom->health = 1;
    data->weaponsRoom->roomSymbol = L'φ';
    data->weaponsRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->weaponsRoom->userData = NULL;

    data->engineRoom->health = 1;
    data->engineRoom->roomSymbol = L'δ';
    data->engineRoom->roomDamagedCallback = NULL; // should be replaced by useful callback by user of object
    data->engineRoom->userData = NULL;

    // draw ship to buffer
    updatePlayerShip(newObject, engine);

    return newObject;
}

void updatePlayerShip(GameObject* ship, Engine* engine){
    ShipData* data = ((ShipData*)ship->userData);
    // draw texture to buffer
    drawLayerToBuffer(&data->texture->layers[0], data->buffer, false, engine);

    // draw rooms to buffer
    drawRoom(data->engineRoom, data->buffer);
    drawRoom(data->shieldRoom, data->buffer);
    drawRoom(data->weaponsRoom, data->buffer);
    drawRoom(data->pilotRoom, data->buffer);
}

void destroyPlayerShip(GameObject* ship){
    ShipData* data = ((ShipData*)((GameObject*)ship)->userData);

    free(data->engineRoom);
    free(data->shieldRoom);
    free(data->weaponsRoom);
    free(data->pilotRoom);

    free(data->buffer);

    freeXPFile(data->texture);

    free(data);

    free(ship);
}

void defaultPlayerShipDraw(Object* self, CursesChar* buffer){
    ShipData* data = (ShipData*)((GameObject*)self)->userData;

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
