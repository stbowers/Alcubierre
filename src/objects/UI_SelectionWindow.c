/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation for selection window in ui.h */

#include <engine.h>
#include <objects/ui.h>
#include <stdlib.h>
#include <string.h>

/* SelectionWindow functions */
void drawSelectionWindow(Object* self, CursesChar* buffer);
void selectionWindowHandleEvents(Object* self, Event* event);

/* ui.h implementation */

GameObject* createSelectionWindow(char** list, char* keys, pfn_SelectionCallback* callbacks, int numOptions, int xpos, int ypos, int z, Engine* engine){
    /* Create new game object */
    GameObject* newObject = (GameObject*) malloc(sizeof(GameObject));
    newObject->timeCreated = getTimems();

    /* Initialize object properties */
    newObject->objectProperties.drawObject = drawSelectionWindow;
    newObject->objectProperties.handleEvent = selectionWindowHandleEvents;
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = xpos;
    newObject->objectProperties.y = ypos;
    newObject->objectProperties.z = z;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.show = true;

    /* Initialize SelectionWindowData */
    SelectionWindowData* data = (SelectionWindowData*) malloc(sizeof(SelectionWindowData));
    newObject->userData = data;
    
    data->height = numOptions;
    data->width = 0;
    for (int i = 0; i < numOptions; i++){
        if (strlen(list[i]) > data->width){
            data->width = strlen(list[i]);
        }
    }

    data->numOptions = numOptions;
    
    // Since we're passed pointers that we don't control the memory of, we need to allocate new space on the heap and copy the data over
    data->list = (char**) malloc(sizeof(char*) * numOptions);
    memcpy(data->list, list, sizeof(char*) * numOptions);
    data->keys = (char*) malloc(sizeof(char) * numOptions);
    memcpy(data->keys, keys, sizeof(char*) * numOptions);
    data->callbacks = (pfn_SelectionCallback*) malloc (sizeof(pfn_SelectionCallback) * numOptions);
    memcpy(data->callbacks, callbacks, sizeof(pfn_SelectionCallback) * numOptions);

    /* Set up buffer */
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar) * data->width * data->height);
    
    /* default char is transparent */
    for (int x = 0; x < data->width; x++){
        for (int y = 0; y < data->height; y++){
            CursesChar* currentChar = &data->buffer[(data->height * x) + y];
            currentChar->attributes = 0;
            // clear char array
            currentChar->character = L'\u00A0';
        }
    }

    for (int i = 0; i < numOptions; i++){
        bufferPrintf(data->buffer, data->width, data->height, 0, i, 0, "%s", list[i]);
    }

    /* Register to receive events */
    EventTypeMask typeMask;
    typeMask.mask = 0; // initialize mask - set all feilds to 0
    typeMask.values.keyboardEvent = 1; // we're interested in keyboard events
    engine->mainPanel->registerEventListener(engine->mainPanel, typeMask, selectionWindowHandleEvents, (Object*)newObject);

    return newObject;
}

void destroySelectionWindow(GameObject* selectionWindow){
    SelectionWindowData* data = (SelectionWindowData*) selectionWindow->userData;

    // free memory allocated inside data
    free(data->list);
    free(data->keys);
    free(data->callbacks);

    // free data
    free(data);
    free(selectionWindow);
}

/* SelectionWindow function implementations */
void drawSelectionWindow(Object* self, CursesChar* buffer){
    SelectionWindowData* data = (SelectionWindowData*)((GameObject*)self)->userData;

    /* Draw buffer */
    for (int x = 0; x < data->width; x++){
        for (int y = 0; y < data->height; y++){
            CursesChar* bufferChar = &data->buffer[(data->height * x) + y];
            // if char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (bufferChar->character != L'\u00A0'){
                writecharToBuffer(buffer, x, y, bufferChar);
            }
        }
    }
}

void selectionWindowHandleEvents(Object* self, Event* event){
    SelectionWindowData* data = (SelectionWindowData*)((GameObject*)self)->userData;
    // we assume event is a key event here, so data is the char
    char ch = *(char*)event->eventData;
    for (int i = 0; i < data->numOptions; i++){
        if (ch == data->keys[i]){
            data->callbacks[i]();
        }
    }
}
