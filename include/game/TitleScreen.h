/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include <engine.h>
#include <SFML/Audio.h>

typedef struct TitleScreenState_s{
    Panel* infoPanel;
    Panel* difficultySelectPanel;

    /* Info panel objects */

    /* difficulty selection panel objects */
    GameObject* difficultyInfoTextBox;
    GameObject* difficultySelectMenu;

    /* Other resources */
    sfMusic* gameMusic;
} TitleScreenState;
extern TitleScreenState titleScreenState;
extern ThreadLock_t titleScreenStateLock;

/* Main functions */
void buildTitleScreen();
void updateTitleScreen();

/* Main menu callbacks */
void playCallback();
void infoCallback();
void backstoryCallback();
void exitCallback();

/* Event listeners */
void infoPanelHandleEvents(Object* self, Event* event);
void difficultySelectPanelHandleEvents(Object* self, Event* event);

/* Difficulty selection callbacks */
void difficultyChangedCallback(int index);
void difficultyChosenCallback(int index);
void difficultyBackCallback();

#endif //__TITLESCREEN_H__
