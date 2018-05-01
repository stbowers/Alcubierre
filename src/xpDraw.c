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
 * The table is initialized and this bool set to true
 * on the first call to getUTF8CharForCP437Value()
 */
bool CP437_UTF8_CODE_INITIALIZED = false;

/* Given a CP437 ascii value, returns the utf8 unicode
 * character for that value.
 *
 * Data provided by Wikipedia (https://en.wikipedia.org/w/index.php?title=Code_page_437&oldid=565442465#Characters)
 */
wchar_t getUTF8CharForCP437Value(int value){
    /* Make sure conversion table is initialized */
    if (!CP437_UTF8_CODE_INITIALIZED){
        /* values 0-127 have the same unicode vaule for
         * drawable characters
         */
        for (int i = 0; i < 128; i++){
            CP437_UTF8_CODE[i] = (char)i;
        }

        /* Code points 1-31 and 127 are ASCII control characters
         * but represent graphical characters for our use
         */
        CP437_UTF8_CODE[1] = L'\u263A'; // ☺
        CP437_UTF8_CODE[2] = L'\u263B'; // ☻
        CP437_UTF8_CODE[3] = L'\u2665'; // ♥
        CP437_UTF8_CODE[4] = L'\u2666'; // ♦
        CP437_UTF8_CODE[5] = L'\u2663'; // ♣
        CP437_UTF8_CODE[6] = L'\u2660'; // ♠
        CP437_UTF8_CODE[7] = L'\u2022'; // •
        CP437_UTF8_CODE[8] = L'\u25D8'; // ◘
        CP437_UTF8_CODE[9] = L'\u25CB'; // ○
        CP437_UTF8_CODE[10] = L'\u25D9'; // ◙
        CP437_UTF8_CODE[11] = L'\u2642'; // ♂
        CP437_UTF8_CODE[12] = L'\u2640'; // ♀
        CP437_UTF8_CODE[13] = L'\u266A'; // ♪
        CP437_UTF8_CODE[14] = L'\u266B'; // ♫
        CP437_UTF8_CODE[15] = L'\u263C'; // ☼
        CP437_UTF8_CODE[16] = L'\u25BA'; // ►
        CP437_UTF8_CODE[17] = L'\u25C4'; // ◄
        CP437_UTF8_CODE[18] = L'\u2195'; // ↕
        CP437_UTF8_CODE[19] = L'\u203C'; // ‼
        CP437_UTF8_CODE[20] = L'\u00B6'; // ¶
        CP437_UTF8_CODE[21] = L'\u00A7'; // §
        CP437_UTF8_CODE[22] = L'\u25AC'; // ▬
        CP437_UTF8_CODE[23] = L'\u21A8'; // ↨
        CP437_UTF8_CODE[24] = L'\u2191'; // ↑
        CP437_UTF8_CODE[25] = L'\u2193'; // ↓
        CP437_UTF8_CODE[26] = L'\u2192'; // →
        CP437_UTF8_CODE[27] = L'\u2190'; // ←
        CP437_UTF8_CODE[28] = L'\u221F'; // ∟
        CP437_UTF8_CODE[29] = L'\u2194'; // ↔
        CP437_UTF8_CODE[30] = L'\u25B2'; // ▲
        CP437_UTF8_CODE[31] = L'\u25BC'; // ▼
        CP437_UTF8_CODE[127] = L'\u2302'; // ⌂
        
        /* The rest of the chars (128-256) are set to ?
         * as a default value, to make it visually clear
         * a character is missing in the table
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
        CP437_UTF8_CODE[220] = L'\u2584'; // ▄
        CP437_UTF8_CODE[221] = L'\u258C'; // ▌
        CP437_UTF8_CODE[222] = L'\u2590'; // ▐
        CP437_UTF8_CODE[223] = L'\u2580'; // ▀

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
        CP437_UTF8_CODE[198] = L'\u255E'; // ╞
        CP437_UTF8_CODE[199] = L'\u255F'; // ╟
        CP437_UTF8_CODE[200] = L'\u255A'; // ╚
        CP437_UTF8_CODE[201] = L'\u2554'; // ╔
        CP437_UTF8_CODE[202] = L'\u2569'; // ╩
        CP437_UTF8_CODE[203] = L'\u2566'; // ╦
        CP437_UTF8_CODE[204] = L'\u2560'; // ╠
        CP437_UTF8_CODE[205] = L'\u2550'; // ═
        CP437_UTF8_CODE[206] = L'\u256C'; // ╬
        CP437_UTF8_CODE[207] = L'\u2567'; // ╧
        CP437_UTF8_CODE[208] = L'\u2568'; // ╨
        CP437_UTF8_CODE[209] = L'\u2564'; // ╤
        CP437_UTF8_CODE[210] = L'\u2565'; // ╥
        CP437_UTF8_CODE[211] = L'\u2559'; // ╙
        CP437_UTF8_CODE[212] = L'\u2558'; // ╘
        CP437_UTF8_CODE[213] = L'\u2552'; // ╒
        CP437_UTF8_CODE[214] = L'\u2553'; // ╓
        CP437_UTF8_CODE[215] = L'\u256B'; // ╫
        CP437_UTF8_CODE[216] = L'\u256A'; // ╪
        CP437_UTF8_CODE[217] = L'\u2518'; // ┘
        CP437_UTF8_CODE[218] = L'\u250C'; // ┌

        // Non-english alphabet letters
        CP437_UTF8_CODE[128] = L'\u00C7'; // Ç
        CP437_UTF8_CODE[129] = L'\u00FC'; // ü
        CP437_UTF8_CODE[130] = L'\u00E9'; // é
        CP437_UTF8_CODE[131] = L'\u00E2'; // â
        CP437_UTF8_CODE[132] = L'\u00E4'; // ä
        CP437_UTF8_CODE[133] = L'\u00E0'; // à
        CP437_UTF8_CODE[134] = L'\u00E5'; // å
        CP437_UTF8_CODE[135] = L'\u00E7'; // ç
        CP437_UTF8_CODE[136] = L'\u00EA'; // ê
        CP437_UTF8_CODE[137] = L'\u00EB'; // ë
        CP437_UTF8_CODE[138] = L'\u00E8'; // è
        CP437_UTF8_CODE[139] = L'\u00EF'; // ï
        CP437_UTF8_CODE[140] = L'\u00EE'; // î
        CP437_UTF8_CODE[141] = L'\u00EC'; // ì
        CP437_UTF8_CODE[142] = L'\u00C4'; // Ä
        CP437_UTF8_CODE[143] = L'\u00C5'; // Å
        CP437_UTF8_CODE[144] = L'\u00C9'; // É
        CP437_UTF8_CODE[145] = L'\u00E6'; // æ
        CP437_UTF8_CODE[146] = L'\u00C6'; // Æ
        CP437_UTF8_CODE[147] = L'\u00F4'; // ô
        CP437_UTF8_CODE[148] = L'\u00F6'; // ö
        CP437_UTF8_CODE[149] = L'\u00F2'; // ò
        CP437_UTF8_CODE[150] = L'\u00FB'; // û
        CP437_UTF8_CODE[151] = L'\u00F9'; // ù
        CP437_UTF8_CODE[152] = L'\u00FF'; // ÿ
        CP437_UTF8_CODE[153] = L'\u00D6'; // Ö
        CP437_UTF8_CODE[154] = L'\u00DC'; // Ü
        CP437_UTF8_CODE[155] = L'\u00A2'; // ¢
        CP437_UTF8_CODE[156] = L'\u00A3'; // £
        CP437_UTF8_CODE[157] = L'\u00A5'; // ¥
        CP437_UTF8_CODE[158] = L'\u20A7'; // ₧
        CP437_UTF8_CODE[159] = L'\u0192'; // ƒ
        CP437_UTF8_CODE[160] = L'\u00E1'; // á
        CP437_UTF8_CODE[161] = L'\u00ED'; // í
        CP437_UTF8_CODE[162] = L'\u00F3'; // ó
        CP437_UTF8_CODE[163] = L'\u00FA'; // ú
        CP437_UTF8_CODE[164] = L'\u00F1'; // ñ
        CP437_UTF8_CODE[165] = L'\u00D1'; // Ñ
        CP437_UTF8_CODE[166] = L'\u00AA'; // ª
        CP437_UTF8_CODE[167] = L'\u00BA'; // º
        CP437_UTF8_CODE[168] = L'\u00BF'; // ¿
        CP437_UTF8_CODE[169] = L'\u2310'; // ⌐
        CP437_UTF8_CODE[170] = L'\u00AC'; // ¬
        CP437_UTF8_CODE[171] = L'\u00BD'; // ½
        CP437_UTF8_CODE[172] = L'\u00BC'; // ¼
        CP437_UTF8_CODE[173] = L'\u00A1'; // ¡
        CP437_UTF8_CODE[174] = L'\u00AB'; // «
        CP437_UTF8_CODE[175] = L'\u00BB'; // »

        // Math symbols
        CP437_UTF8_CODE[224] = L'\u03B1'; // α
        CP437_UTF8_CODE[225] = L'\u00DF'; // ß
        CP437_UTF8_CODE[226] = L'\u0393'; // Γ
        CP437_UTF8_CODE[227] = L'\u03C0'; // π
        CP437_UTF8_CODE[228] = L'\u03A3'; // Σ
        CP437_UTF8_CODE[229] = L'\u03C3'; // σ
        CP437_UTF8_CODE[230] = L'\u00B5'; // µ
        CP437_UTF8_CODE[231] = L'\u03C4'; // τ
        CP437_UTF8_CODE[232] = L'\u03A6'; // Φ
        CP437_UTF8_CODE[233] = L'\u0398'; // Θ
        CP437_UTF8_CODE[234] = L'\u03A9'; // Ω
        CP437_UTF8_CODE[235] = L'\u03B4'; // δ
        CP437_UTF8_CODE[236] = L'\u221E'; // ∞
        CP437_UTF8_CODE[237] = L'\u03C6'; // φ
        CP437_UTF8_CODE[238] = L'\u03B5'; // ε
        CP437_UTF8_CODE[239] = L'\u2229'; // ∩
        CP437_UTF8_CODE[240] = L'\u2261'; // ≡
        CP437_UTF8_CODE[241] = L'\u00B1'; // ±
        CP437_UTF8_CODE[242] = L'\u2265'; // ≥
        CP437_UTF8_CODE[243] = L'\u2264'; // ≤
        CP437_UTF8_CODE[244] = L'\u2320'; // ⌠
        CP437_UTF8_CODE[245] = L'\u2321'; // ⌡
        CP437_UTF8_CODE[246] = L'\u00F7'; // ÷
        CP437_UTF8_CODE[247] = L'\u2248'; // ≈
        CP437_UTF8_CODE[248] = L'\u00B0'; // °
        CP437_UTF8_CODE[249] = L'\u2219'; // ∙
        CP437_UTF8_CODE[250] = L'\u00B7'; // ·
        CP437_UTF8_CODE[251] = L'\u221A'; // √
        CP437_UTF8_CODE[252] = L'\u207F'; // ⁿ
        CP437_UTF8_CODE[253] = L'\u00B2'; // ²
        CP437_UTF8_CODE[254] = L'\u25A0'; // ■
        CP437_UTF8_CODE[255] = L'\u25A1'; // □
        
        /* set init bool to true */
        CP437_UTF8_CODE_INITIALIZED = true;
    }

    /* Simply return the wchar_t at the index vaule */
    return CP437_UTF8_CODE[value];
}

/* Draw function */
void drawLayerToBuffer(XPLayer* layer, CursesChar* buffer, bool transparent, Engine* engine){
    /* Draw to buffer */
    for (int x = 0; x < layer->width; x++){
        for (int y = 0; y < layer->height; y++){
            int index = (layer->height * x) + y;
            CursesChar* charAt = &buffer[index];

            /* Get char data */
            XPChar* xpChar = &layer->data[index];
            wchar_t wch = getUTF8CharForCP437Value(xpChar->value);

            /* Draw char */
            int bg = getBestColor(xpChar->br, xpChar->bg, xpChar->bb, engine);
            int fg = getBestColor(xpChar->fr, xpChar->fg, xpChar->fb, engine);
            int colorPair = getColorPair(fg, bg, engine);
            CursesChar cursesChar;
            cursesChar.attributes = COLOR_PAIR(colorPair);
            cursesChar.character = wch;
            //wadd_wch(win, &cursesChar);
            if ((xpChar->br == 255 && xpChar->bg == 0 && xpChar->bb == 255)
                || (xpChar->value == 0)){
                // if the background is (255, 0, 255) or the character is null that's REXPaint's signal that the char is transparent
                if (transparent){
                    // don't overwrite chars below us, so do nothing
                } else {
                    // write transparent char
                    charAt->attributes = 0;
                    charAt->character = L'\u00A0';
                }
            } else {
                // copy char data to the buffer
                *charAt = cursesChar;
            }
        }
    }
}
