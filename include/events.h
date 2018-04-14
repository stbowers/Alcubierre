/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Defines behaviour of engine events */
#ifndef __EVENTS_H__
#define __EVENTS_H__

/* Event Structure */
/* NOTE: this structure should always be allocated on the heap with malloc,
 * since it will be shared between threads and the memory may be read and
 * modified after the stack has been poped on the original thread.
 *
 * Two fields:
 *      eventType - 4 bytes - describes the type of event
 *      eventData - void pointer - points to data for event
 * eventType:
 *      union allows the data to be interpreted as a:
 *      (1) unsigned 32 bit integer (mask for binary operations)
 *      or (2) a struct of booleans, for easier use and readability
 *     *Note: this is less efficient than something like an enum,
 *      because each bit is reserved for an individual type, but it
 *      makes it much easier, and faster, to filter events based
 *      on a mask, and some (most?) implementations of c only assign
 *      data in chuncks of 4 bytes, so this doesn't take up any more
 *      space in most situations.
 *     *Note: even though technically multiple types can be specified,
 *      only one should be set at a time, unless it is safe (and makes
 *      sense to) interpret the void pointer and event data as both
 *      types of event.
 * eventData:
 *      can point to any kind of data, depending on the type of
 *      event. This should never be the only reference to a pointer
 *      returned by malloc, since no effort is made to free this
 *      memory when freeing the memory for an event
 */
typedef union EventTypeMask_u{
    unsigned int mask;
    struct {
        /* Each value is a 1 bit boolean */
        bool keyboardEvent :1;
        bool gameMsgEvent  :1;
        bool timerEvent    :1;
    } values;
} EventTypeMask;

typedef struct Event_s{
    EventTypeMask eventType;
    void* eventData;
    struct Event_s* next; // Used to make a linked list event queue
} Event;

/* Event type masks */
const static unsigned int EVENT_KEYBOARD        = (0x1 << 0);
const static unsigned int EVENT_GAMEMSG         = (0x1 << 1);
const static unsigned int EVENT_TIMER           = (0x1 << 2);

#endif //__EVENTS_H__
