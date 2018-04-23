/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __OVERVIEWSCREEN_H__
#define __OVERVIEWSCREEN_H__

#include <AlcubierreGame.h>
#include <objects/sprites.h>

typedef struct OverviewScreenState_s{
    Panel* missionSelectionPanel;
    Panel* endGameScreen;

    /* Mission selection panel objects */
    GameObject* missionSelectionMenu;
    GameObject* missionSelectionInfo;

    /* End screen objects */
    GameObject* endText;

    GameObject* locationMarkers[9];
    // true for any locations that need to be updated when updateOverviewScreen is called
    bool locationStatusChanged[9];
    int locationMarkerStartX;
    XPFile* locationUnknownTexture;
    XPFile* locationCurrentFrames[2];
    XPFile* locationCompletedTexture;
    XPFile* locationSkippedTexture;
} OverviewScreenState;
extern OverviewScreenState overviewScreenState;
extern ThreadLock_t overviewScreenStateLock;

/* Main functions */
void buildOverviewScreen();
void updateOverviewScreen();

/* Event handlers */
void overviewScreenHandleEvents(Object* overviewScreen, Event* event);

/* Sector mission selection callbacks */
void missionSelectionChanged(int index);
void missionSelected(int index);
void sectorSkipped();

#endif //__OVERVIEWSCREEN_H__
