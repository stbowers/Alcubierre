/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Implementation of createPanel and destroyPanel from engine.h */

#include <engine.h>
#include <stdlib.h>

/* Default panel functions */
void defaultRefreshPanel(Panel* self);
void defaultClearPanel(Panel* self);
void defaultAddObject(Panel* self, Object* newObject);
void defaultDrawPanel(Object* self, Panel* panel);
void defaultPanelAddListener(Panel* self, EventTypeMask mask, void (*handleEvent)(Object* self, const Event* event), Object* listener);
void defaultPanelHandleEvent(Object* self, Event* event);

/* engine.h impementation */

Panel* createPanel(int width, int height, int x, int y, int z){
    Panel* newPanel = (Panel*) malloc(sizeof(Panel));

    /* Initialize object properties */
    newPanel->objectProperties.next = NULL;
    newPanel->objectProperties.previous = NULL;
    newPanel->objectProperties.type = OBJECT_PANEL;
    newPanel->objectProperties.x = x;
    newPanel->objectProperties.y = y;
    newPanel->objectProperties.z = z;
    newPanel->objectProperties.drawObject = defaultDrawPanel;
    newPanel->objectProperties.handleEvent = defaultPanelHandleEvent;

    /* Assign function pointers */
    newPanel->refreshPanel = defaultRefreshPanel;
    newPanel->clearPanel = defaultClearPanel;
    newPanel->addObject = defaultAddObject;
    newPanel->registerEventListener = defaultPanelAddListener;

    /* Create ncurses window */
    newPanel->window = newwin(height, width, y, x);

    /* Link panel to window */
    newPanel->panel = new_panel(newPanel->window);

    /* Return new panel */
    return newPanel;
}

/* default functions implementation */

void defaultRefreshPanel(Panel* self){
    update_panels();
}

void defaultClearPanel(Panel* self){
    wclear(self->window);
}

void defaultAddObject(Panel* self, Object* newObject){
    /* If our list of children is empty, simply assign newObject as the start of the list */
    if (self->childrenList == NULL){
        self->childrenList = newObject;
        return;
    }

    /* Crawl through list until current's z order is larger than ours*/
    Object* current = self->childrenList;
    while (current->z <= newObject->z){
        /* If we reach the end of the list without finding a larger z value, put newObject at the end */
        if (current->next == NULL){
            current->next = newObject;
            newObject->previous = current;
            return;
        }

        current = current->next;
    }
    
    /* Place newObject before current */
    Object* last = current->previous;
    last->next = newObject;
    current->previous = newObject;
    newObject->previous = last;
    newObject->next = current;
}

void defaultDrawPanel(Object* self, Panel* panel){
    /* Put this panel on top of the panel stack */
    top_panel(((Panel*)self)->panel);

    /* Crawl list of objects, drawing each */
    Object* current = ((Panel*)self)->childrenList;
    while (current != NULL){
        current->drawObject(current, (Panel*)self);
        current = current->next;
    }
}

void defaultPanelAddListener(Panel* self, EventTypeMask mask, void (*handleEvent)(Object* self, const Event* event), Object* listener){
    *self->nextListener = (EventListener*) malloc(sizeof(EventListener));
    (*self->nextListener)->mask = mask;
    (*self->nextListener)->handleEvent = handleEvent;
    (*self->nextListener)->listener = listener;
    (*self->nextListener)->next = NULL;
    
    self->nextListener = &(*self->nextListener)->next;
}

void defaultPanelHandleEvent(Object* self, Event* event){
    // move through list, send event to each listener if the mask matches
    EventListener* current = ((Panel*)self)->listeners;
    while (current != NULL){
        if (event->eventType.mask & current->mask.mask){
            current->handleEvent(current->listener, event);
        }

        current = current->next;
    }
}
