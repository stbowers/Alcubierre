/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Definitions for UI objects (base object type defined in engine.h) */
#ifndef __UI_H__
#define __UI_H__

#include <engine.h>

/* Objects in this file are only described by createObject() and destroyObject()
 * methods, which create and manage an object (or window) struct with preset
 * function pointers and data providing the resources needed to produce
 * the specified functionality. For implementation see src/objects/ui.c
 */

/* Selection window */
/* Draws a window with several options in it, captures keyboard while active
 * +-------------------------------------------+
 * | [ **USER TEXT**                         ] |
 * | [ **USER TEXT**                         ] |
 * | { **USER TEXT**                         } |
 * +-------------------------------------------+
 * Note: selected text is boxed with curly brackets {}, and colored with a
 * selection color, while unselected options are boxed with square brackets
 * [] and colored with the normal text coloring
 */

/* the callback function is passed the index of the selection it was called
 * for, so that the same function can be used for multiple selecitons which
 * are of the same category, but might have different properties.
 */
typedef void(*pfn_SelectionCallback)(int index);

typedef struct SelectionWindowData_s{
    CursesChar* buffer;
    char** list;
    char* keys; // array of chars - not string
    pfn_SelectionCallback* callbacks;
    int width, height;
    int numOptions;
    bool bordered;
    bool arrowSelection;
    int currentSelection; // only used with arrowSelection
} SelectionWindowData;

// Bordered: should we draw a border around the options (also makes the panel opaque in blank spaces)
// arrowSelection: should we allow the arrow keys to select options (change highlighting), and call callback when enter is pressed
GameObject* createSelectionWindow(char** list, char* keys, bool bordered, bool arrowSelection, pfn_SelectionCallback* callbacks, bool registerForEvents, int numOptions, int minWidth, int xpos, int ypos, int z, Engine* engine);
void destroySelectionWindow(GameObject* selectionWindow);

// call if updating any data from the outside
void drawSelectionWindowBuffer(GameObject* selectionWindow);

#endif //__UI_H_
