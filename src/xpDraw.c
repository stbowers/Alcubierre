/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation for the drawLayerToPanel() function in xpFunctions.h*/

#include <xpFunctions.h>
#include <string.h>

/* Unicode helper functions */
/* Array of wide chars (unicode characters) where the
 * index is the extended ascii (CP437) value, and the
 * unicode char associated with that index is the
 * corrosponding utf-8 character.
 */
#define CP437_NUMCHARS 256
wchar_t CP437_UTF8_CODE[CP437_NUMCHARS];

/* Since large parts of the above conversion table can
 * be filled out programatically, this bool keeps track
 * of wether or not the table has been initialized yet.
 * If false, the data in the array will be meaningless.
 */
bool CP437_UTF8_CODE_INITIALIZED = false;

/* Given a CP437 ascii value, returns the utf8 unicode
 * character for that value.
 */
wchar_t getUTF8CharForCP437Value(int value){
    /* Make sure conversion table is initialized */
    if (!CP437_UTF8_CODE_INITIALIZED){
        /* Fill array with null bytes so undefined chars 
         * won't map to random unicode values */
        memset(CP437_UTF8_CODE, 0x00, CP437_NUMCHARS * sizeof(wchar_t));

        /* values 0-127 have the same unicode vaule */
        for (int i = 0; i < 128; i++){
            CP437_UTF8_CODE[i] = (char)i;
        }

        /* Manually entered unicode conversions */
        // block chars
        CP437_UTF8_CODE[176] = L'\u2591';
        CP437_UTF8_CODE[177] = L'\u2592';
        CP437_UTF8_CODE[178] = L'\u2593';
        CP437_UTF8_CODE[176] = L'\u2588';

        // 
        
        /* set init bool to true */
        CP437_UTF8_CODE_INITIALIZED = true;
    }

    /* Simply return the wchar_t at the index vaule */
    return CP437_UTF8_CODE[value];
}

/* Color helper functions */

/* Draw function */
void drawLayerToPanel(XPLayer* layer, Panel* panel, bool clearPanel){
    WINDOW* win = panel->window;

    /* Set up panel for drawing */
    if (clearPanel){
        panel->clearPanel(panel);
    }

    /* Draw to panel */
    /* The addch family of functions will place a character onto the
     * window, and move the cursor right once. For minimal effort we
     * want to avoid moving the cursor as much as possible. To do this
     * we want to render characters in lines, so each character can
     * call addch, one after another. At the end of the line we will
     * print a newline character, '\n', to move the cursor to the
     * next line.
     * Outer loop: y
     *      print all chars for this line
     *      print newline char
     * Inner loop: x
     *      print the char at x,y
     * Note: the char at x,y might more accurately described as the
     * char at y,x since the data is stored in column major order.
     * So incrementing the index of the data by one moves to the next
     * row in the same column. So index of (x,y) = (x*height) + y
     */
    for (int y = 0; y < layer->height; y++){
        /* Draw line */
        for (int x = 0; x < layer->width; x++){
            int index = (x*layer->height) + y;

            /* Get char data */
            XPChar* xpChar = &layer->data[index];
            wchar_t wch = getUTF8CharForCP437Value(xpChar->value);

            /* Draw char */
            cchar_t cursesChar = {0, {wch}, 0};
            wadd_wch(win, &cursesChar);
        }

        /* Move cursor to next line */
        waddch(win, '\n');
    }
}
