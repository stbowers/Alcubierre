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

typedef void(*pfn_SelectionCallback)();

typedef struct SelectionWindowData_s{
    cchar_t* buffer;
    char** list;
    char* keys; // array of chars - not string
    pfn_SelectionCallback* callbacks;
    int width, height;
    int numOptions;
} SelectionWindowData;

GameObject* createSelectionWindow(char** list, char* keys, pfn_SelectionCallback* callbacks, int numOptions, int xpos, int ypos, int z, Engine* engine);
void destroySelectionWindow(GameObject* selectionWindow);

/* */

#endif //__UI_H_
