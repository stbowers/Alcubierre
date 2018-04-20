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
    // Draw the object to the given buffer, called from the render thread
    // NOTE: buffer can point anywhere inside of a stdscr buffer. This function
    // should draw relative to that pointer instead of trying to calculate
    // absolute positions. This can be done with writewchToBuffer();
    void (*drawObject)(struct Object_s* self, cchar_t* buffer);

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

    /* Width and height of this panel (mostly for background) */
    int width, height;

    /* Background buffer - rendered below any objects in this panel */
    cchar_t* backgroundBuffer;

    /* Event delegation */
    EventListener* listeners;
    EventListener** nextListener;
    void (*registerEventListener)(struct Panel_s* self, EventTypeMask mask, void (*handleEvent)(Object* self, Event* event), Object* listener);

    /* Custom window functions */
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
    /* The WINDOW* refernce returned by initscr
     * in most cases this is stdscr - but this should be used
     * for better functionality
     */
    WINDOW* stdscr;

    /* The main window for the engine */
    Panel* mainPanel;
    // width and height of the main window
    int width, height;

    /* The window currently receiving event notifications from the engine */
    /* Only one active window is specified in order to avoid conflicts from
     * several parts of the game wanting to handle specific events. Windows
     * can, if they choose, forward events to other objects or windows.
     */
    Panel* activePanel;

    /* Buffer of ncurses characters to print to the screen */
    // Array stored in column-major order - char at (x, y) = screenBuffer + (height*x) + y
    // Two buffers allocated for double buffer rendering system
    cchar_t* stdscrBuffer1;
    cchar_t* stdscrBuffer2;
    // Buffer copied to the render buffer (stdscrBuffer1/2) before rendering; effectively the background
    cchar_t* backgroundBuffer;
    // size of the above buffers in bytes
    int stdscrBufferSize;
    // width and height of the screen
    int stdscrWidth, stdscrHeight;

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
     *
     * renderThread is the thread used to render to a buffer
     * drawingThread is the thread used to draw the non-rendering buffer to the screen
     * renderTimerThread is the timer thread which syncs the other threads to an fps
     */
    pthread_t renderThread;
    pthread_t drawingThread;
    pthread_t renderTimerThread;
    struct RenderThreadData_s{
        pthread_mutex_t dataMutex;

        /* dataMutex resources */
        bool render; // should the render thread be rendering right now?
        int fps_calculated; // The current fps being rendered
        bool renderDrawSyncWaiting; // if the render thread or drawing thread is waiting for the other to sync
        pthread_cond_t renderDrawSync; // signal the render or drawing thread has caught up with the other
        pthread_cond_t renderSignal; // Used to signal a change in render to the render thread
        /* end of dataMutex resources */

        pthread_mutex_t renderMutex;
        /* resources accessed by render thread */
        cchar_t** renderBuffer; // pointer to the cchar_t buffer to render to
        /* end of renderMutex resources */

        pthread_mutex_t drawingMutex;
        /* resources accessed by drawing thread */
        cchar_t** drawingBuffer; // pointer to the cchar_t buffer to draw from
        /* end of renderMutex resources */
        
        pthread_mutex_t timerMutex;
        /* timerMutex resources */
        pthread_cond_t timerSignal;
        bool timerExit;
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

/* Gets the absolute position (relative to stdscr) of a given coordinate
 * relative to a given object
 */
void getAbsolutePosition(Object* parent, int relX, int relY, int* absX, int* absY);

/* Writes a wchar_t with the given attributes to the location in the given buffer
 * NOTE: because of the way the stdscr buffers are layed out this function can
 * take a pointer to any part of the buffer, and it will use x and y as offsets from
 * that location, in other words x and y need not be absolute, only relative to the
 * pointer passed here.
 */
void writewchToBuffer(cchar_t* buffer, int x, int y, unsigned int attr, wchar_t wch[5]);

/* Same as above, but accepts a cchar_t
 */
void writecharToBuffer(cchar_t* buffer, int x, int y, cchar_t ch);

/* printf style formatting to print text to a buffer
 * NOTE: this function only accepts normal width (1 byte) characters
 * NOTE: unlike above functions, this function can be used with any buffer,
 *  not just buffers that are COLS wide and LINES tall. length is the width
 *  each line should be (doesn't need to be the width of the buffer), 
 *  and height is the height of the buffer (does need to be the actual height
 *  for proper formatting)
 */
void bufferPrintf(cchar_t* buffer, int width, int height, int x, int y, unsigned int attr, const char* format, ...);

/* Returns a timestamp in milliseconds, from an undefined
 * starting time. (Monotonic clock)
 */
uint64_t getTimems();

/* Sleeps for a given number of milliseconds
 */
void sleepms(int msec);

#endif //__ENGINE_H_
