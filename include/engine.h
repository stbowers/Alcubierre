/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Provides various functionality of a graphics engine */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#include <panel.h>
#include <events.h>
#include <pthread.h>

#define MS_PER_TICK 10 // how many milliseconds corrospond to one tick - tickrate (10ms/tick ~= 100 ticks per second)
#define MS_PER_FRAME 10 // how many milliseconds corrospond to one frame - framerate

/* Data Structures */

/* Object structure, holds all data common to 'objects'
 * for the engine. Objects are anything that is drawn
 * onto the screen, such as a panel or a game object.
 * By including this struct at the top of any 'object'
 * this acts as a primitive form of inheritance and
 * polymorphism, so that the engine can generalize the
 * code used to draw any kind of object on screen
 */

typedef enum ObjectType_e{
    OBJECT_PANEL,
    OBJECT_GAMEOBJECT,
} ObjectType;

typedef struct Object_s{
    /* Double linked list pointers */
    struct Object_s* next;
    struct Object_s* previous;

    /* Object Type */
    ObjectType type;

    /* Basic data */
    // Object's position (relative to its parent)
    int x,y;
    // Object's z order (for sorting lists before drawing)
    int z;

    /* Object Functions */
    // Draw the object, called from the render thread
    void (*drawObject)(struct Object_s* self);

    // Handle an event, called from the events thread
    /* NOTE: any intensive processing that needs to be
     * done in response to an event should be run on
     * another thread, so that it doesn't hold up the
     * event thread.
     */
    void (*handleEvent)(struct Object_s* self, const Event* event);

    // Update the object's data. Called once per 'tick' from the main game loop
    void (*update)(struct Object_s* self);
} Object;

// Structure to hold data for an ncurses panel
typedef struct Panel_s{
    /* Panel 'extends' Object */
    Object objectProperties;

    /* Panel ref */
    PANEL* panel;
    
    /* Window this panel is attached to */
    WINDOW* window;

    /* Custom window functions */
    /* Called when the window needs to be refreshed,
     * takes place of wrefresh(windowPointer)
     */
    void (*refreshPanel)(struct Panel_s* self);

    /* Called when the window should be cleared,
     * takes place of wclear(windowPointer)
     */
    void (*clearPanel)(struct Panel_s* self);

    /* Adds an object to this panel
     */
    void (*addObject)(struct Panel_s* self, Object* newObject);

    /* Linked list of children (Objects that draw to this panel)*/
    Object* childrenList;
} Panel;

// Structure to hold any and all data needed to run the engine
typedef struct Engine_s{
    /* The main window for the engine */
    Panel* mainPanel;

    /* The window currently receiving event notifications from the engine */
    /* Only one active window is specified in order to avoid conflicts from
     * several parts of the game wanting to handle specific events. Windows
     * can, if they choose, forward events to other objects or windows.
     */
    Panel* activePanel;

    /* Event handler */
    /* Called for every event at the start of the game loop
     */
    void (*handleEvent)(struct Engine_s* self, const Event* event);

    /* Threads */
    /* The event thread runs continuously on the same tickrate as the game
     * thread (see below). The event thread and main thread may become out
     * of sync, but they should *on average* loop at the same tick rate.
     */
    pthread_t eventThread;

    /* The game thread runs continously on a tickrate defined by MS_PER_TICK
     * which determines how long a tick should last. Assuming MS_PER_TICK
     * is 10, there should be 100 ticks per second. The game thread tries
     * to keep its tickrate constant by only sleeping until its total loop
     * time is MS_PER_TICK, rather than sleeping for MS_PER_TICK every loop.
     */
    pthread_t gameThread;

    /* The render thread runs continously at a framerate defined by
     * MS_PER_FRAME, which determines how long a frame should last.
     * Implemented similar to the game thread's tickrate.
     */
    pthread_t renderThread;
    struct RenderThreadData_s{
        pthread_mutex_t dataMutex; // Threads should only read or write the data in this struct if they hold the lock for this mutex
        bool render; // should the render thread be rendering right now?
        pthread_cond_t renderSignal; // Used to signal a change in render to the render thread
    } renderThreadData;

    /* Global thread data
     * Contains info about the whole program that any thread might be intersted in
     */
    struct GlobalThreadData_s{
        pthread_mutex_t dataMutex;
        /* bool to hold if the engine is currently running. If false,
         * threads shutdown
         */
        bool isRunning;
    } globalThreadData;
} Engine;

// Structure to hold data for game objects
typedef struct GameObject_s{
    /* GameObject 'extends' Object */
    Object objectProperties;

    /* Basic Object Data */
    // Time created (ms)
    uint64_t timeCreated;

    /* Custom Behaviour */
    /* The user can put any data they wish into
     * userData, and it will be passed to the
     * update function, which is called in the
     * main game loop.
     */
    void* userData;
    void (*update)(struct GameObject_s* self, void* userData);
} GameObject;

/* Methods */
/* Creates and returns an Engine struct, setting it up to be
 * ready for drawing and other tasks.
 */
Engine* initializeEngine(int width, int height);
void destroyEngine(Engine* engine);

/* Creates and returns a panel, with the given width and height
 * and position relative to stdscr. The ncurses subwin and derwin
 * class of functions are not well implemented, according to
 * ncurses own man page, so we'll just avoid using ncures subwindows
 */
Panel* createPanel(int width, int height, int x, int y, int z);
void destroyPanel(Panel* panel);

/* Returns a timestamp in milliseconds, from an undefined
 * starting time. (Monotonic clock)
 */
uint64_t getTimems();

/* Sleeps for a given number of milliseconds
 */
void sleepms(int msec);

#endif //__ENGINE_H_
