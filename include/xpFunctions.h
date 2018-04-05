/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Defines functionality to load an xp file - the native file format of REXPaint, an ascii art editor */

#include <engine.h>
#include <panel.h>
#include <inttypes.h>

/* XP File structures */
struct XPChar_s;
struct XPLayer_s;
struct XPFile_s;

/* the individual character and layer structs need to be packed, since they mirror how the data 
 * is stored in the .xp file, and they are not padded there.
 * Structs are packed by:
 *  in GCC: struct __atribute__((__packed__)) struct_name{...
 *  in MSVC: #pragma pack(push,1) ...define structures... #pragma pack(pop)
 */

#define PACK_STRUCT
#ifdef WIN32
#pragma pack(push,1)
#endif
#ifdef __UNIX__
#undef PACK_STRUCT
#define PACK_STRUCT __attribute__((__packed__))
#endif

typedef struct PACK_STRUCT XPChar_s{
    int32_t value; // 32 bits for signed character value (ASCII CP437)
    uint8_t fr; // Foreground colors, 8 bits each, unsigned, red green blue order
    uint8_t fg;
    uint8_t fb;
    uint8_t br; // Background colors
    uint8_t bg;
    uint8_t bb;
} XPChar;

typedef struct PACK_STRUCT XPLayer_s{
    int32_t width; // 32 bits each for width and height of the image
    int32_t height;
    struct XPChar_s* data; // 2d matrix of character values - stored in column major order
} XPLayer;

#ifdef WIN32
#pragma pack(pop)
#endif

/* The struct for an xp file itself doesn't need to be packed
 */
typedef struct XPFile_s{
    int32_t version; // xp version - not important to us
    int32_t numLayers; // number of layers in the image
    struct XPLayer_s* layers; // data for each layer
} XPFile;

/* The structure of a .axp file (animated xp file) is simply just a header
 * right before the normal .xp file; note: axp is a format I created for
 * this engine
 */
typedef struct AXPFile_s{
    int32_t version; // version of axp - the only valid version right now is 0x01
    int32_t fps; // fps the animation should be played at
    struct XPFile_s* xpFile; // normal xp file
} AXPFile;

/* Functions for xp and axp files */
XPFile* getXPFile(const char* filename);
void freeXPFile(XPFile* file);

AXPFile* getAXPFile(const char* filename);
void freeAXPFile(AXPFile* file);

/* Draws a given layer to the given panel
 * clearPanel: if true the panel will be cleared before drawing
 *      so that previous chars won't be visible. If painting a
 *      normal xp file, this should be false and this function
 *      should be called for every layer in the image. If painting
 *      a frame of an axp file, this should be true to avoid
 *      messing up the frame.
 * NOTE: This function relys on some computationally expensive
 *      calculations, so you should call it once for any given
 *      layer and keep a reference to the panel to show on screen
 *      at the proper time (showing/hiding panels is almost
 *      instant compared to painting a whole layer)
 */
void drawLayerToPanel(XPLayer* layer, Panel* panel, bool clearPanel);
