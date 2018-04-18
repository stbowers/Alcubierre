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

/* Data Structures */
struct Panel_s;

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
    // Parent object the x and y coordinates are relative to
    struct Object_s* parent;
    // If false don't render the object
    bool show;

    /* Object Functions */
    // Draw the object to the given panel, called from the render thread
    void (*drawObject)(struct Object_s* self, struct Panel_s* panel);

    // Move the object to the absolute coordinates on the screen
    void (*moveAbsolute)(struct Object_s* self, int xpos, int ypos);

    // Handle an event, called from the events thread
    /* NOTE: any intensive processing that needs to be
     * done in response to an event should be run on
     * another thread, so that it doesn't hold up the
     * event thread.
     */
    void (*handleEvent)(struct Object_s* self, Event* event);

    // Update the object's data. Called once per 'tick' from the main game loop
    void (*update)(struct Object_s* self);
} Object;

typedef struct EventListener_s{
    EventTypeMask mask;
    void (*handleEvent)(Object* self, Event* event);
    Object* listener;
    struct EventListener_s* next;
} EventListener;

// Structure to hold data for an ncurses panel
typedef struct Panel_s{
    /* Panel 'extends' Object */
    Object objectProperties;

    /* Panel ref */
    PANEL* panel;
    
    /* Window this panel is attached to */
    WINDOW* window;

    /* Event delegation */
    EventListener* listeners;
    EventListener** nextListener;
    void (*registerEventListener)(struct Panel_s* self, EventTypeMask mask, void (*handleEvent)(Object* self, Event* event), Object* listener);

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

    /* Removes an object
     */
    void (*removeObject)(struct Panel_s* self, Object* toRemove);

    /* Linked list of children (Objects that draw to this panel)*/
    Object* childrenList;
} Panel;

// Structure to hold any and all data needed to run the engine
typedef struct Engine_s{
    /* The main window for the engine */
    Panel* mainPanel;

    /* Panel controlling the stdscr window, not the same as mainPanel
     * We don't need the extra functionality of our panel struct, so
     * just use an ncurses panel.
     */
    PANEL* stdPanel;

    /* The window currently receiving event notifications from the engine */
    /* Only one active window is specified in order to avoid conflicts from
     * several parts of the game wanting to handle specific events. Windows
     * can, if they choose, forward events to other objects or windows.
     */
    Panel* activePanel;

    /* width and height of main panel */
    int width, height;

    /* Event handler */
    /* Called for every event at the start of the game loop
     */
    void (*handleEvent)(struct Engine_s* self, Event* event);

    /* Threads */
    /* The event thread runs continuously on the same tickrate as the game
     * thread (see below). The event thread and main thread may become out
     * of sync, but they should *on average* loop at the same tick rate.
     */
    pthread_t eventThread;
    struct EventThreadData_s{
        pthread_mutex_t dataMutex;
        pthread_cond_t newEventSignal;
        Event* queuedEvents;
        Event** queueEnd; // pointer to the pointer that should be set to add a new event to the end of the queue (the last node's ->next, or the start if queue is empty)
        bool runEventThread;
    } eventThreadData;

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
        pthread_mutex_t dataMutex;

        /* dataMutex resources */
        bool render; // should the render thread be rendering right now?
        int fps_calculated; // The current fps being rendered
        pthread_cond_t renderSignal; // Used to signal a change in render to the render thread
        /* end of dataMutex resources */

        pthread_mutex_t timerMutex;
        /* timerMutex resources */
        pthread_cond_t timerSignal;
        /* end of timerMutex resources */
    } renderThreadData;

    /* Global thread data
     * Contains info about the whole program that any thread might be intersted in
     */
    struct GlobalThreadData_s{
        pthread_mutex_t dataMutex;
        /* bool to hold if the engine is currently running. If false,
         * threads should shutdown
         */
        bool isRunning;
        pthread_cond_t exitSignal; // signaled when isRunning is set to false (should broadcast, since multiple threads _might_ be waiting on this)
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
    /* The user can put any data they wish into userData,
     * which will be available whenever a refernce to the
     * game object is held.
     */
    void* userData;
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

/* Move the given object to coordinates relative to another object.
 * Note: Normal use is to pass an object as toMove, and it's parent as relativeTo,
 *  but the object can be moved relative to any other object in the game.
 * Note: If the same object is passed to toMove and relativeTo, this function
 *  effectively shifts the object to the right by relX and down by relY
 *  (or left/up if negative)
 */
void moveRelativeTo(Object* toMove, Object* relativeTo, int relX, int relY);

/* Returns a timestamp in milliseconds, from an undefined
 * starting time. (Monotonic clock)
 */
uint64_t getTimems();

/* Sleeps for a given number of milliseconds
 */
void sleepms(int msec);

#endif //__ENGINE_H_
