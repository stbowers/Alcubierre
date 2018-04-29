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

/* Selection window */
/* Draws a window with several options in it, captures keyboard while active
 * +-------------------------------------------+
 * | [ **USER TEXT**                         ] |
 * | [ **USER TEXT**                         ] |
 * | { **USER TEXT**                         } |
 * +-------------------------------------------+
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
    pfn_SelectionCallback selectionChangedCallback;
    int width, height;
    int numOptions;
    bool bordered;
    bool arrowSelection;
    int currentSelection; // only used with arrowSelection
} SelectionWindowData;

// Bordered: should we draw a border around the options (also makes the panel opaque in blank spaces)
// arrowSelection: should we allow the arrow keys to select options (change highlighting), and call callback when enter is pressed
GameObject* createSelectionWindow(char** list, char* keys, bool bordered, bool arrowSelection, pfn_SelectionCallback* callbacks, pfn_SelectionCallback selectionChangedCallback, bool registerForEvents, int numOptions, int minWidth, int xpos, int ypos, int z, Engine* engine);
void destroySelectionWindow(GameObject* selectionWindow);

// call if updating any data from the outside
void drawSelectionWindowBuffer(GameObject* selectionWindow);



/* Text box */
/* A simple gameObject which renders text to the screen. Can optionally have a border
 */

typedef struct TextBoxData_s{
    char* text;
    attr_t attributes;
    CursesChar* buffer;
    int bufferWidth, bufferHeight;
    int textWidth, textHeight;
    bool bordered;
} TextBoxData;

GameObject* createTextBox(const char* text, attr_t attributes, bool bordered, int width, int height, int x, int y, int z, Engine* engine);
void destroyTextBox(GameObject* textBox);

// changes the text in the text box
void updateTextBox(GameObject* textBox, const char* newText, attr_t attributes, bool center);



/* Progress bar */
// Label: [###----]
typedef struct ProgressBarData_s{
    char* label;
    float percentage;
    CursesChar* buffer;
    int bufferWidth, bufferHeight;
} ProgressBarData;

GameObject* createProgressBar(const char* label, float percentage, attr_t attributes, int width, int x, int y, int z, Engine* engine);
void destroyProgressBar(GameObject* progressBar);

// updates the percentage for the progress bar
void updateProgressBar(GameObject* progressBar, float newPercentage, attr_t attributes);

#endif //__UI_H_
