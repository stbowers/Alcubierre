/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <engine.h>
#include <objects/ui.h>
#include <stdlib.h>
#include <string.h>

void defaultDrawProgressBar(Object* self, CursesChar* buffer);

// Label: [###----]
// if minWidth is 0, or less than the width of label + 4, will always draw Label: [#], and use a higher width accordingly
// to make progress bar longer, use a minWidth that is long enough for the bar
GameObject* createProgressBar(const char* label, float percentage, attr_t attributes, int minWidth, int x, int y, int z, Engine* engine){
    GameObject* newObject = (GameObject*) malloc(sizeof(GameObject));

    /* Initialize object properties */
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.show = true;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = x;
    newObject->objectProperties.y = y;
    newObject->objectProperties.z = z;
    newObject->objectProperties.drawObject = defaultDrawProgressBar;
    newObject->objectProperties.handleEvent = NULL;

    /* Initialize user data */
    ProgressBarData* data = (ProgressBarData*) malloc(sizeof(ProgressBarData));
    newObject->userData = data;
    
    // Calculate width
    int labelWidth = strlen(label);
    // use labelWidth + 3 or minWidth, whichever is larger
    // label[#]
    data->bufferWidth = (labelWidth+3>minWidth)?labelWidth+3:minWidth;
    data->bufferHeight = 1; // 1 tall object
    data->label = (char*) malloc(sizeof(char)*(strlen(label)+1));
    strcpy(data->label, label);

    // initialize buffer
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar)*data->bufferWidth*data->bufferHeight);

    // draw progress bar to buffer (done in update function)
    updateProgressBar(newObject, percentage, attributes);
    
    return newObject;
}

void destroyProgressBar(GameObject* progressBar){

    free(progressBar->userData);

    free(progressBar);
}

// updates the percentage for the progress bar
void updateProgressBar(GameObject* progressBar, float newPercentage, attr_t attributes){
    ProgressBarData* data = (ProgressBarData*)progressBar->userData;

    data->percentage = newPercentage;
    // remove the label from the rest of the width, minus 2 for the left and right brackets contianing the progress bar
    int progressBarWidth = (data->bufferWidth - strlen(data->label)) - 2;

    bufferPrintf(data->buffer, data->bufferWidth, data->bufferHeight, data->bufferHeight, 0, 0, 0, "%s[", data->label);
    int progressBarStartX = strlen(data->label) + 1;
    for (int i = 0; i < progressBarWidth; i++){
        if (((float)i / (float)progressBarWidth) < (data->percentage)){
            // if i/width is inside of the percentage, draw a full character
            bufferPrintf(data->buffer, data->bufferWidth, data->bufferHeight, data->bufferHeight, progressBarStartX+i, 0, attributes, "%c", '#');
        } else {
            // if i/width is outside of the percentage, draw a blank character
            bufferPrintf(data->buffer, data->bufferWidth, data->bufferHeight, data->bufferHeight, progressBarStartX+i, 0, attributes, "%c", ' ');
        }
    }

    // print closing bracket
    data->buffer[data->bufferWidth - 1].character = ']';
}

void defaultDrawProgressBar(Object* self, CursesChar* buffer){
    ProgressBarData* data = (ProgressBarData*)((GameObject*)self)->userData;

    /* Draw buffer */
    for (int x = 0; x < data->bufferWidth; x++){
        for (int y = 0; y < data->bufferHeight; y++){
            CursesChar* bufferChar = &data->buffer[(data->bufferHeight * x) + y];
            // if char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (bufferChar->character != L'\u00A0'){
                writecharToBuffer(buffer, x, y, bufferChar);
            }
        }
    }
}
