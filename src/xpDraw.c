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
#include <math.h>

/* Unicode helper functions */
/* Array of wide chars (unicode characters) where the
 * index is the extended ascii (CP437) value, and the
 * unicode char associated with that index is the
 * corrosponding utf-8 character.
 *
 * Since REXPaint uses extended ascii in the form of
 * IBM's Code Page 437 (https://en.wikipedia.org/w/index.php?title=Code_page_437&oldid=565442465#Characters)
 * we need a way to convert those characters to unicode, since
 * that's an easier way to deal with special characters.
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
        /* values 0-127 have the same unicode vaule */
        for (int i = 0; i < 128; i++){
            CP437_UTF8_CODE[i] = (char)i;
        }

        /* Code points 1-31 and 127 are ASCII control characters
         * but represent graphical characters for our use
         */
        CP437_UTF8_CODE[1] = L'?';
        CP437_UTF8_CODE[2] = L'?';
        CP437_UTF8_CODE[3] = L'?';
        CP437_UTF8_CODE[4] = L'?';
        CP437_UTF8_CODE[5] = L'?';
        CP437_UTF8_CODE[6] = L'?';
        CP437_UTF8_CODE[7] = L'?';
        CP437_UTF8_CODE[8] = L'?';
        CP437_UTF8_CODE[9] = L'?';
        CP437_UTF8_CODE[10] = L'?';
        CP437_UTF8_CODE[11] = L'?';
        CP437_UTF8_CODE[12] = L'?';
        CP437_UTF8_CODE[13] = L'?';
        CP437_UTF8_CODE[14] = L'?';
        CP437_UTF8_CODE[15] = L'?';
        CP437_UTF8_CODE[16] = L'?';
        CP437_UTF8_CODE[17] = L'?';
        CP437_UTF8_CODE[18] = L'?';
        CP437_UTF8_CODE[19] = L'?';
        CP437_UTF8_CODE[20] = L'?';
        CP437_UTF8_CODE[21] = L'?';
        CP437_UTF8_CODE[22] = L'?';
        CP437_UTF8_CODE[23] = L'?';
        CP437_UTF8_CODE[24] = L'?';
        CP437_UTF8_CODE[25] = L'?';
        CP437_UTF8_CODE[26] = L'?';
        CP437_UTF8_CODE[27] = L'?';
        CP437_UTF8_CODE[28] = L'?';
        CP437_UTF8_CODE[29] = L'?';
        CP437_UTF8_CODE[30] = L'?';
        CP437_UTF8_CODE[31] = L'?';
        CP437_UTF8_CODE[127] = L'?';
        
        /* The rest of the chars (128-256) are set to ?
         * to indicate that the proper value hasn't been
         * assigned - the ? will be overwritten by other
         * assignments.
         */
        for (int i = 0; i < 128; i++){
            CP437_UTF8_CODE[i+128] = '?';
        }

        /* Manually entered unicode conversions */
        // block chars
        CP437_UTF8_CODE[176] = L'\u2591'; // light checkerboard
        CP437_UTF8_CODE[177] = L'\u2592'; // med checkerboard
        CP437_UTF8_CODE[178] = L'\u2593'; // dark checkerboard
        CP437_UTF8_CODE[219] = L'\u2588'; // full block

        // lines
        CP437_UTF8_CODE[179] = L'\u2502'; // │
        CP437_UTF8_CODE[180] = L'\u2524'; // ┤
        CP437_UTF8_CODE[181] = L'\u2561'; // ╡
        CP437_UTF8_CODE[182] = L'\u2562'; // ╢
        CP437_UTF8_CODE[183] = L'\u2556'; // ╖
        CP437_UTF8_CODE[184] = L'\u2555'; // ╕
        CP437_UTF8_CODE[185] = L'\u2563'; // ╣
        CP437_UTF8_CODE[186] = L'\u2551'; // ║
        CP437_UTF8_CODE[187] = L'\u2557'; // ╗
        CP437_UTF8_CODE[188] = L'\u255D'; // ╝
        CP437_UTF8_CODE[189] = L'\u255C'; // ╜
        CP437_UTF8_CODE[190] = L'\u255B'; // ╛
        CP437_UTF8_CODE[191] = L'\u2510'; // ┐
        CP437_UTF8_CODE[192] = L'\u2514'; // └
        CP437_UTF8_CODE[193] = L'\u2534'; // ┴
        CP437_UTF8_CODE[194] = L'\u252C'; // ┬
        CP437_UTF8_CODE[195] = L'\u251C'; // ├
        CP437_UTF8_CODE[196] = L'\u2500'; // ─
        CP437_UTF8_CODE[197] = L'\u253C'; // ┼
        CP437_UTF8_CODE[198] = L'?';
        CP437_UTF8_CODE[199] = L'?';
        CP437_UTF8_CODE[200] = L'\u255A'; // ╚
        CP437_UTF8_CODE[201] = L'\u2554'; // ╔
        CP437_UTF8_CODE[202] = L'\u2569'; // ╩
        CP437_UTF8_CODE[203] = L'\u2566'; // ╦
        CP437_UTF8_CODE[204] = L'\u2560'; // ╠
        CP437_UTF8_CODE[205] = L'\u2550'; // ═
        CP437_UTF8_CODE[206] = L'\u256C'; // ╬
        CP437_UTF8_CODE[207] = L'?';
        CP437_UTF8_CODE[208] = L'?';
        CP437_UTF8_CODE[209] = L'?';
        CP437_UTF8_CODE[210] = L'?';
        CP437_UTF8_CODE[211] = L'?';
        CP437_UTF8_CODE[212] = L'?';
        CP437_UTF8_CODE[213] = L'?';
        CP437_UTF8_CODE[214] = L'?';
        CP437_UTF8_CODE[215] = L'?';
        CP437_UTF8_CODE[216] = L'?';
        CP437_UTF8_CODE[217] = L'\u2518'; // ┘
        CP437_UTF8_CODE[218] = L'\u250C'; // ┌

        
        /* set init bool to true */
        CP437_UTF8_CODE_INITIALIZED = true;
    }

    /* Simply return the wchar_t at the index vaule */
    return CP437_UTF8_CODE[value];
}

/* Color helper functions */
/* Searches through the available terminal colors for the one that is closest
 * to the given rgb value (by euclidian distance), or if the terminal supports
 * changing colors start changing colors past 16 (standard colors)
 */
int nextColor = 16;
int getBestColor(int r, int g, int b){
    int bestColor = 0;
    float bestr2 = 2e6; // max distance in color space is ~1.96e5, so all colors should be closer than this initial value
    
    // number of colors to search through (full range if can't change colors, else only the colors that have been set)
    int numColors = (can_change_color())?nextColor:COLORS;

    /* Loop through avaliable terminal colors */
    for (int color = 0; color < numColors; color++){
        /* Get rgb value of terminal color */
        short tr, tg, tb;
        color_content(color, &tr, &tg, &tb);

        /* Normalize terminal color */
        /* color_content returns values between 0 and 1000,
         * so normalize them to be between 0 and 255
         */
        tr = tr / (3.9f);
        tg = tg / (3.9f);
        tb = tb / (3.9f);

        /* Get the square of the euclidian distance
         * r^2 = (x^2 + y^2 + z^2) 
         */

        float r2 = pow((tr -r), 2) + pow((tg - g), 2) + pow((tb - b), 2);

        /* Compare to bestr2 */
        if (r2 < bestr2){
            bestColor = color;
            bestr2 = r2;
        }
    }
    
    if (bestr2 < 150 || !can_change_color()){
        // if we found a close enough match, or can't change colors return that
        return bestColor;
    }

    // else change the next color and return that
    init_color(nextColor, r*3.9, g*3.9, b*3.9);
    nextColor++;
    return nextColor - 1;
}

/* Search for an existing color pair with the given colors, and if
 * one isn't found make a new one at nextColorPair
 */
int nextColorPair = 1;
int getColorPair(int fg, int bg){
    for (int pair = 0; pair < nextColorPair; pair++){
        /* Get colors in pair */
        short pfg, pbg;
        pair_content(pair, &pfg, &pbg);

        /* If they match, return */
        if (pfg == fg && pbg == bg){
            return pair;
        }
    }

    /* If we leave loop, no matching pair was found.
     * Create a new one at nextColorPair and increment
     * nextColorPair
     */
    init_pair(nextColorPair, fg, bg);
    nextColorPair++;
    return nextColorPair - 1;
}

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
        /* Move cursor to the begining of the line */
        wmove(win, y, 0);

        /* Draw line */
        for (int x = 0; x < layer->width; x++){
            int index = (x*layer->height) + y;

            /* Get char data */
            XPChar* xpChar = &layer->data[index];
            wchar_t wch = getUTF8CharForCP437Value(xpChar->value);

            /* Draw char */
            int bg = getBestColor(xpChar->br, xpChar->bg, xpChar->bb);
            int fg = getBestColor(xpChar->fr, xpChar->fg, xpChar->fb);
            int colorPair = getColorPair(fg, bg);
            cchar_t cursesChar = {COLOR_PAIR(colorPair), {wch}};
            //wadd_wch(win, &cursesChar);
            if (xpChar->br == 255 && xpChar->bg == 0 && xpChar->bb == 255){
                // if the background is (255, 0, 255) that's REXPaint's signal that the char is transparent, so just skip over it
                wmove(win, y, x+1);
            } else {
                wadd_wch(win, &cursesChar);
            }
        }
    }
}
