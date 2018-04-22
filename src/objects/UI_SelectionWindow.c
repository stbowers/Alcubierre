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

/* Helper function for drawing the buffer */
void drawSelectionWindowBuffer(GameObject* selectionWindow);

/* ui.h implementation */

GameObject* createSelectionWindow(char** list, char* keys, bool bordered, bool arrowSelection, pfn_SelectionCallback* callbacks, pfn_SelectionCallback selectionChangedCallback, bool registerForEvents, int numOptions, int minWidth, int xpos, int ypos, int z, Engine* engine){
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
    
    data->numOptions = numOptions;
    data->bordered = bordered;
    data->arrowSelection = arrowSelection;
    data->currentSelection = 0;

    /* Set width & height */
    data->height = numOptions;
    data->width = minWidth;
    for (int i = 0; i < numOptions; i++){
        if (strlen(list[i]) > data->width){
            data->width = strlen(list[i]);
        }
    }

    /* add to width & height if we need a border or selection arrows */
    if (bordered){
        data->width += 2;
        data->height += 2;
    }
    
    if (arrowSelection){
        data->width += 2;
    }

    
    // Since we're passed pointers that we don't control the memory of, we need to allocate new space on the heap and copy the data over
    data->list = (char**) malloc(sizeof(char*) * numOptions);
    memcpy(data->list, list, sizeof(char*) * numOptions);
    data->keys = (char*) malloc(sizeof(char) * numOptions);
    memcpy(data->keys, keys, sizeof(char) * numOptions);
    data->callbacks = (pfn_SelectionCallback*) malloc (sizeof(pfn_SelectionCallback) * numOptions);
    memcpy(data->callbacks, callbacks, sizeof(pfn_SelectionCallback) * numOptions);
    data->selectionChangedCallback = selectionChangedCallback;

    /* Set up buffer */
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar) * data->width * data->height);
    
    // fill buffer with default chars
    for (int x = 0; x < data->width; x++){
        for (int y = 0; y < data->height; y++){
            CursesChar* currentChar = &data->buffer[(data->height * x) + y];
            currentChar->attributes = 0;
            // clear char array
            // if bordered, fill with spaces, else fill with transparent NBSP char
            currentChar->character = (bordered)?L' ':L'\u00A0';
        }
    }

    /* draw buffer */
    drawSelectionWindowBuffer(newObject);

    /* Register to receive events */
    if (registerForEvents){
        EventTypeMask typeMask;
        typeMask.mask = 0; // initialize mask - set all feilds to 0
        typeMask.values.keyboardEvent = true; // we're interested in keyboard events
        engine->mainPanel->registerEventListener(engine->mainPanel, typeMask, selectionWindowHandleEvents, (Object*)newObject);
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

/* Draw buffer */
void drawSelectionWindowBuffer(GameObject* selectionWindow){
    SelectionWindowData* data = (SelectionWindowData*)selectionWindow->userData;
    /* Print border to buffer if needed */
    if (data->bordered){
        // print top and bottom
        for (int x = 1; x < (data->width-1); x++){
            CursesChar* topChar = &data->buffer[(x * data->height) + 0];
            CursesChar* bottomChar = &data->buffer[(x * data->height) + data->height - 1];
            topChar->character = bottomChar->character = L'─'; // set char to horizontal line
        }

        // print corners
        CursesChar* topLeft = &data->buffer[(0 * data->height) + 0];
        CursesChar* topRight = &data->buffer[((data->width-1) * data->height) + 0];
        CursesChar* bottomLeft = &data->buffer[(0 * data->height) + (data->height-1)];
        CursesChar* bottomRight = &data->buffer[((data->width-1) * data->height) + (data->height-1)];

        topLeft->character = L'┌';
        topRight->character = L'┐';
        bottomLeft->character = L'└';
        bottomRight->character = L'┘';
    
        // sides are printed with options
    }

    /* Print options to buffer */
    for (int i = 0; i < data->numOptions; i++){
        /* Get x,y position to print string at */
        int x = 0;
        int y = i;
        if (data->bordered){
            x += 1;
            y += 1;
        }
        if (data->arrowSelection){
            x += 1;
        }

        /* Pre-option (border and/or selection arrows) */
        if (data->bordered){
            CursesChar* borderChar = &data->buffer[ (0 * data->height) + y];
            borderChar->character = L'│';
        }
        if (data->arrowSelection){
            CursesChar* arrowChar = &data->buffer[ (1 * data->height) + y];
            if (i==data->currentSelection){
                arrowChar->character = L'♦';
            } else {
                arrowChar->character = L' ';
            }
        }

        /* Print option */
        bufferPrintf(data->buffer, data->width, data->height, data->height, x, y, 0, "%s", data->list[i]);
        
        /* Post-option (border and/or selection arrows) */
        if (data->bordered){
            CursesChar* borderChar = &data->buffer[ ((data->width-1) * data->height) + y];
            borderChar->character = L'│';
        }
        if (data->arrowSelection){
            CursesChar* arrowChar = &data->buffer[ ((data->width-2) * data->height) + y];
            // default selection is the first one
            if (i==data->currentSelection){
                arrowChar->character = L'♦';
            } else {
                arrowChar->character = L' ';
            }
        }
    }

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
    int ch = (int)event->eventData;

    // if ch is up, down, or enter with arrowSelections
    if (data->arrowSelection){
        switch (ch){
            case KEY_UP:
                // move selection up if current selection is not zero
                if (data->currentSelection > 0){
                    data->currentSelection--;
                    if (data->selectionChangedCallback != NULL){
                        data->selectionChangedCallback(data->currentSelection);
                    }
                }

                // update buffer
                drawSelectionWindowBuffer((GameObject*)self);
                break;
            case KEY_DOWN:
                // move selection down if current selection is not the last option
                if (data->currentSelection < (data->numOptions - 1)){
                    data->currentSelection++;
                    if (data->selectionChangedCallback != NULL){
                        data->selectionChangedCallback(data->currentSelection);
                    }
                }

                // update buffer
                drawSelectionWindowBuffer((GameObject*)self);
                break;
            case KEY_ENTER:
            case 10:
                // call callback for the curent selection
                data->callbacks[data->currentSelection](data->currentSelection);
                break;
            default:
                // do nothing
                break;
        }
    }

    // check keys for ch
    for (int i = 0; i < data->numOptions; i++){
        if (ch == data->keys[i]){
            data->callbacks[i](i);
        }
    }
}
