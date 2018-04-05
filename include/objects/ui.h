/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Definitions for UI objects (base object type defined in engine.h) */
#ifndef __UI_H__
#define __UI_H_

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
Panel* createSelectionWindow();
void destroySelectionWindow();

/* */

#endif //__UI_H_
