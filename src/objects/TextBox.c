/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#include <objects/ui.h>
#include <stdlib.h>
#include <string.h>

void defaultDrawTextBox(Object* self, CursesChar* buffer);
void defaultTextBoxHandleEvent(Object* self, Event* event);

GameObject* createTextBox(const char* text, attr_t attributes, bool bordered, int width, int height, int x, int y, int z, Engine* engine){
    GameObject* newObject = (GameObject*) malloc(sizeof(GameObject));
    TextBoxData* data = (TextBoxData*) malloc(sizeof(TextBoxData));
    newObject->userData = data;

    /* Object properties */
    newObject->objectProperties.next = NULL;
    newObject->objectProperties.parent = NULL;
    newObject->objectProperties.previous = NULL;
    newObject->objectProperties.show = true;
    newObject->objectProperties.type = OBJECT_GAMEOBJECT;
    newObject->objectProperties.x = x;
    newObject->objectProperties.y = y;
    newObject->objectProperties.z = z;
    newObject->objectProperties.drawObject = defaultDrawTextBox;
    newObject->objectProperties.handleEvent = defaultTextBoxHandleEvent;

    /* Text box data */
    data->attributes = attributes;
    data->text = (char*) malloc(sizeof(char) * (strlen(text) + 1));
    strncpy(data->text, text, strlen(text) + 1);
    data->bordered = bordered;
    data->textWidth = width;
    data->textHeight = height;

    /* Create buffer */
    data->bufferWidth = width;
    data->bufferHeight = height;
    
    // if bordered more width & height will be reserved for the border, so decrease text width & height
    if (bordered){
        data->textWidth -= 2;
        data->textHeight -=2;
    }
    
    data->buffer = (CursesChar*) malloc(sizeof(CursesChar) * data->bufferWidth * data->bufferHeight);

    // Fill buffer with either transparency if not bordered, or a border and spaces if bordered
    for (int x = 0; x < data->bufferWidth; x++){
        for (int y = 0; y < data->bufferHeight; y++){
            CursesChar* charAt = &data->buffer[(x * data->bufferHeight) + y];

            // set char
            if (bordered){
                charAt->attributes = 0;
                if (x == 0 && y == 0){
                    // top left
                    charAt->character = L'┌';
                } else if (x == 0 && y == (data->bufferHeight-1)){
                    // bottom left
                    charAt->character = L'└';
                } else if (x == (data->bufferWidth-1) && y == 0){
                    // top right
                    charAt->character = L'┐';
                } else if (x == (data->bufferWidth-1) && y == (data->bufferHeight-1)){
                    // bottom right
                    charAt->character = L'┘';
                } else if (x == 0 || x == (data->bufferWidth-1)){
                    // sides
                    charAt->character = L'│';
                } else if (y == 0 || y == (data->bufferHeight-1)){
                    // top & bottom
                    charAt->character = L'─';
                } else {
                    // inside
                    charAt->character = L' ';
                }
            } else {
                // set transparent
                charAt->attributes = 0;
                charAt->character = L'\u00A0';
            }
        }
    }

    /* Print text to buffer */
    int startX = (bordered)? 1: 0;
    int startY = (bordered)? 1: 0;
    bufferPrintf(data->buffer, width, data->bufferHeight, height, startX, startY, data->attributes, "%s", data->text);

    return newObject;
}

void updateTextBox(GameObject* textBox, const char* newText, attr_t attributes, bool center){
    TextBoxData* data = ((TextBoxData*)textBox->userData);
    
    /* Update data fields */
    data->attributes = attributes;
    free(data->text);
    data->text = (char*) malloc(sizeof(char) * (strlen(newText) + 1));
    strncpy(data->text, newText, strlen(newText) + 1);

    /* Redraw buffer */
    // Fill buffer with either transparency if not bordered, or a border and spaces if bordered
    for (int x = 0; x < data->bufferWidth; x++){
        for (int y = 0; y < data->bufferHeight; y++){
            CursesChar* charAt = &data->buffer[(x * data->bufferHeight) + y];

            // set char
            if (data->bordered){
                charAt->attributes = 0;
                if (x == 0 && y == 0){
                    // top left
                    charAt->character = L'┌';
                } else if (x == 0 && y == (data->bufferHeight-1)){
                    // bottom left
                    charAt->character = L'└';
                } else if (x == (data->bufferWidth-1) && y == 0){
                    // top right
                    charAt->character = L'┐';
                } else if (x == (data->bufferWidth-1) && y == (data->bufferHeight-1)){
                    // bottom right
                    charAt->character = L'┘';
                } else if (x == 0 || x == (data->bufferWidth-1)){
                    // sides
                    charAt->character = L'│';
                } else if (y == 0 || y == (data->bufferHeight-1)){
                    // top & bottom
                    charAt->character = L'─';
                } else {
                    // inside
                    charAt->character = L' ';
                }
            } else {
                // set transparent
                charAt->attributes = 0;
                charAt->character = L'\u00A0';
            }
        }
    }

    /* Print text to buffer */
    int startX;
    if (!center){
        startX = (data->bordered)? 1: 0;
    } else {
        startX = (data->bufferWidth - strlen(data->text)) / 2.0f;
    }
    int startY = (data->bordered)? 1: 0;
    bufferPrintf(data->buffer, data->textWidth, data->bufferHeight, data->textHeight, startX, startY, data->attributes, "%s", data->text);

}

void defaultDrawTextBox(Object* self, CursesChar* buffer){
    TextBoxData* data = (TextBoxData*)((GameObject*)self)->userData;

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

void defaultTextBoxHandleEvent(Object* self, Event* event){

}
