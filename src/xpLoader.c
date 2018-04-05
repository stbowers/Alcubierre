/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation to load xp files */

#include <xpFunctions.h>
#include <stdlib.h>
#include <zlib.h>

XPFile* getXPFile_gz(gzFile* rawFile){
    XPFile* newFile = (XPFile*) malloc(sizeof(XPFile));
    int status;

    /* Read Header */
    // Read just the header (version & number of layers)
    status = gzread(*rawFile, newFile, 8);
    // check for error
    if (status == -1){
        // get error code
        int error;
        gzerror(*rawFile, &error);
        
        // report error to console
        switch (error){
            case Z_BUF_ERROR:
                printf("Zlib buffer error\n");
                break;
            default:
                printf("Unrecognized zlilb error: %d\n", error);
        }

        // free allocated memory, and return null pointer
        free(newFile);
        return NULL;
    }

    /* Read Image Data */
    // The version should be negative, if not fail
    if (newFile->version < 0){
        // Allocate space for all layers.
        newFile->layers = (XPLayer*) malloc(sizeof(XPLayer) * newFile->numLayers);

        // read each layer
        for (int layer = 0; layer < newFile->numLayers; layer++){
            XPLayer* thisLayer = &newFile->layers[layer];
            
            // Read next 8 bytes - width & height - into layer
            status = gzread(*rawFile, thisLayer, 8);
            // Check for error
            if (status == -1){
                // Get error code
                int error;
                gzerror(*rawFile, &error);

                // report error to console
                switch (error){
                    case Z_BUF_ERROR:
                        printf("Zlib buffer error\n");
                        break;
                    default:
                        printf("Unrecognized zlilb error: %d\n", error);
                }

                // free allocated memory, and return null pointer
                free(newFile->layers);
                free(newFile);
                return NULL;
            }

            // allocate width*height XPChars for data
            size_t dataSize = sizeof(XPChar) * thisLayer->width * thisLayer->height;
            thisLayer->data = (XPChar*) malloc(dataSize);

            // read all chars into array
            status = gzread(*rawFile, thisLayer->data, dataSize);
            
            // Check for error
            if (status == -1){
                // Get error code
                int error;
                gzerror(*rawFile, &error);

                // report error to console
                switch (error){
                    case Z_BUF_ERROR:
                        printf("Zlib buffer error\n");
                        break;
                    default:
                        printf("Unrecognized zlilb error: %d\n", error);
                }

                // free allocated memory, and return null pointer
                free(thisLayer->data);
                free(newFile->layers);
                free(newFile);
                return NULL;
            }
        }
    }else{
        // free allocated memory and return null
        free(newFile);
        return NULL;
    }
    
    /* Return the finished struct */
    return newFile;
}

XPFile* getXPFile(const char* filename){
    gzFile rawFile = gzopen(filename, "rb");
    return getXPFile_gz(&rawFile);
}

void freeXPFile(XPFile* file){
    // For each layer in file free the char data
    for (int i = 0; i < file->numLayers; i++){
        XPLayer* thisLayer = &file->layers[i];
        free(thisLayer->data);
    }

    // Free all layer structs
    free(file->layers);

    // And finally free the file struct
    free(file);
}

AXPFile* getAXPFile(const char* filename){
    AXPFile* newFile = (AXPFile*) malloc(sizeof(AXPFile));
    int status;

    // Open the file as a gz stream
    gzFile rawFile = gzopen(filename, "rb");

    // Read just the header (version & fps)
    status = gzread(rawFile, newFile, 8);
    // check for error
    if (status == -1){
        // get error code
        int error;
        gzerror(rawFile, &error);
        
        // report error to console
        switch (error){
            case Z_BUF_ERROR:
                printf("Zlib buffer error\n");
                break;
            default:
                printf("Unrecognized zlilb error: %d\n", error);
        }

        // free allocated memory, and return null pointer
        free(newFile);
        return NULL;
    }
    
    // Read the rest of the file as an xp file (use gz stream instead of file path)
    newFile->xpFile = getXPFile_gz(&rawFile);

    // Check to make sure the xp file was read correctly (NULL if not)
    if (newFile->xpFile == NULL){
        // report error
        printf("Error reading xp file inside axp file...\n");

        // free memory and return null
        free(newFile);
        return NULL;
    }

    return newFile;
}

void freeAXPFile(AXPFile* file){
    // Free the XPFile inside
    free(file->xpFile);

    // Free the struct
    free(file);
}
