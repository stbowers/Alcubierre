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
void drawSelectionWindow(Object* self, Panel* panel);
void updateSelectionWindow(Object* self);
void selectionWindowHandleEvents(Object* self, Event* event);
void moveSelectionWindow(Object* self, int x, int y);

/* ui.h implementation */

GameObject* createSelectionWindow(char** list, char* keys, pfn_SelectionCallback* callbacks, int numOptions, int xpos, int ypos, Engine* engine){
    /* Create new game object */
    GameObject* newObject = (GameObject*) malloc(sizeof(GameObject));
    newObject->timeCreated = getTimems();

    /* Initialize object properties */
    newObject->objectProperties.drawObject = drawSelectionWindow;
    newObject->objectProperties.handleEvent = selectionWindowHandleEvents;
    newObject->objectProperties.update = updateSelectionWindow;
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = xpos;
    newObject->objectProperties.y = ypos;
    newObject->objectProperties.z = 1;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.moveAbsolute = moveSelectionWindow;
    newObject->objectProperties.show = true;

    /* Initialize SelectionWindowData */
    SelectionWindowData* data = (SelectionWindowData*) malloc(sizeof(SelectionWindowData));
    newObject->userData = data;

    int width = 20;
    int height = numOptions;
    data->panel = createPanel(width, height, xpos, ypos, 1);
    data->panel->objectProperties.handleEvent = selectionWindowHandleEvents;
    data->numOptions = numOptions;
    
    // Since we're passed pointers that we don't control the memory of, we need to allocate new space on the heap and copy the data over
    data->list = (char**) malloc(sizeof(char*) * numOptions);
    memcpy(data->list, list, sizeof(char*) * numOptions);
    data->keys = (char*) malloc(sizeof(char) * numOptions);
    memcpy(data->keys, keys, sizeof(char*) * numOptions);
    data->callbacks = (pfn_SelectionCallback*) malloc (sizeof(pfn_SelectionCallback) * numOptions);
    memcpy(data->callbacks, callbacks, sizeof(pfn_SelectionCallback) * numOptions);

    /* Register to receive events */
    EventTypeMask typeMask;
    typeMask.mask = 0; // initialize mask - set all feilds to 0
    typeMask.values.keyboardEvent = 1; // we're interested in keyboard events
    engine->mainPanel->registerEventListener(engine->mainPanel, typeMask, selectionWindowHandleEvents, (Object*)newObject);

    /* Draw options to panel */
    for (int opt = 0; opt < numOptions; opt++){
        wprintw(data->panel->window, "%s\n", list[opt]);
    }

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
void drawSelectionWindow(Object* self, Panel* panel){
    SelectionWindowData* data = (SelectionWindowData*)((GameObject*)self)->userData;
    top_panel(data->panel->panel);
    show_panel(data->panel->panel);
}

void updateSelectionWindow(Object* self){

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

void moveSelectionWindow(Object* self, int x, int y){
    SelectionWindowData* data = (SelectionWindowData*)((GameObject*)self)->userData;
    move_panel(data->panel->panel, y, x);
}
