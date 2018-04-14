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

/* SelectionWindow functions */
void drawSelectionWindow(Object* self, Panel* panel);
void updateSelectionWindow(Object* self);
void selectionWindowHandleEvents(Object* self, Event* event);

/* ui.h implementation */

GameObject* createSelectionWindow(const char** list, const char* keys, int numOptions, int xpos, int ypos){
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

    /* Initialize SelectionWindowData */
    SelectionWindowData* data = (SelectionWindowData*) malloc(sizeof(SelectionWindowData));
    newObject->userData = data;

    int width = 20;
    int height = numOptions;
    data->panel = createPanel(width, height, xpos, ypos, 1);
    data->panel->objectProperties.handleEvent = selectionWindowHandleEvents;
    data->list = list;
    data->keys = keys;
    data->numOptions = numOptions;

    /* Register to receive events */


    /* Draw options to panel */
    for (int opt = 0; opt < numOptions; opt++){
        wprintw(data->panel->window, "%s\n", list[opt]);
    }

    return newObject;
}

void destroySelectionWindow(GameObject* selectionWindow){
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
    /* NOTE: For testing purposes this is currently being called as a panel
     * object, so self refers to data->panel, not the actual game object.
     * This doesn't matter for us right now.
     */
    // Test - print char if event is keyboard event
    if (event->eventType.values.keyboardEvent){
        mvwprintw(((Panel*)self)->window, 0, 0, "%c", *((char*)event->eventData));
    }
}
