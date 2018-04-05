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
/* Two fields:
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
 *      event
 */
typedef struct event{
    union {
        unsigned int mask;
        struct {
            /* Each value is a 1 bit boolean */
            /* Up to 32 types can be specified */
            unsigned int keyboardEvent  : 1;
            unsigned int gameMsgEvent   : 1;
            unsigned int timerEvent     : 1;
        } values;
    } eventType;
    void* eventData;
} Event;

/* Event type masks */
const static unsigned int EVENT_KEYBOARD        = (0x1 << 0);
const static unsigned int EVENT_GAMEMSG         = (0x1 << 1);
const static unsigned int EVENT_TIMER           = (0x1 << 2);

#endif //__EVENTS_H__
