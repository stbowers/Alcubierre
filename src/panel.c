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
void defaultAddObject(Panel* self, Object* newObject);
void defaultRemoveObject(Panel* self, Object* toRemove);
void defaultDrawPanel(Object* self, CursesChar* buffer);
void defaultPanelAddListener(Panel* self, EventTypeMask mask, void (*handleEvent)(Object* self, Event* event), Object* listener);
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
    newPanel->objectProperties.parent = NULL;
    newPanel->objectProperties.show = true;
    newPanel->objectProperties.drawObject = defaultDrawPanel;
    newPanel->objectProperties.handleEvent = defaultPanelHandleEvent;

    /* Assign function pointers */
    newPanel->addObject = defaultAddObject;
    newPanel->removeObject = defaultRemoveObject;
    newPanel->registerEventListener = defaultPanelAddListener;

    /* Set up event listener list */
    newPanel->listeners = NULL;
    newPanel->nextListener = &newPanel->listeners; // next listener should be put at newPanel->listeners

	/* Set up children list */
	newPanel->childrenList = NULL;

    /* Create background buffer */
    newPanel->width = width;
    newPanel->height = height;
    newPanel->backgroundBuffer = (CursesChar*) malloc(sizeof(CursesChar)*width*height);

    /* Fill background buffer */
    for (int x = 0; x < newPanel->width; x++){
        for (int y = 0; y < newPanel->height; y++){
            CursesChar* currentChar = &newPanel->backgroundBuffer[(newPanel->height * x) + y];
            currentChar->attributes = 0;
            // clear char array
			currentChar->character = L'-';//L'\u00A0';
        }
    }

    /* Return new panel */
    return newPanel;
}

void destroyPanel(Panel* panel){
	/* Free background buffer */
	free(panel->backgroundBuffer);

	/* Free event listeners (see add listener function for details on what memory we own and need to free) */
	EventListener* current = panel->listeners;
	while (current != NULL) {
		EventListener* next = current->next;
		free(current);
		current = next;
	}

	/* Free panel */
    free(panel);
}

/* default functions implementation */
void defaultAddObject(Panel* self, Object* newObject){
    /* Set self as parent to newObject */
    newObject->parent = (Object*)self;

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

void defaultRemoveObject(Panel* self, Object* toRemove){
    /* traverse list looking for toRemove */
    Object* current = self->childrenList;
    Object** previousPointer = &self->childrenList;
    while (current != NULL){
        if (current == toRemove){
            // if a match is found, remove it from the list.
            *previousPointer = current->next;
            current->next = NULL;
            return;
        }
        previousPointer = &current->next;
        current = current->next;
    }
}

void defaultDrawPanel(Object* self, CursesChar* buffer){
    /* Draw background buffer */
    for (int x = 0; x < ((Panel*)self)->width; x++){
        for (int y = 0; y < ((Panel*)self)->height; y++){
            CursesChar* backgroundChar = &((Panel*)self)->backgroundBuffer[((((Panel*)self)->height) * x) + y];
            // if background char is not NBSP (\u00A0), draw it. (NBSP is transparent character for our case)
            if (backgroundChar->character != L'\u00A0'){
                writecharToBuffer(buffer, x, y, backgroundChar);
            }
        }
    }

    /* Crawl list of objects, drawing each */
    Object* current = ((Panel*)self)->childrenList;
    while (current != NULL){
        if (current->show){
            // get the offset into buffer at the x,y position of the object
            CursesChar* bufferAtObject = &buffer[(LINES * current->x) + current->y];
            // draw the object at it's location
            current->drawObject(current, bufferAtObject);
        }
        current = current->next;
    }
}

void defaultPanelAddListener(Panel* self, EventTypeMask mask, void (*handleEvent)(Object* self, Event* event), Object* listener){
	/* NOTE: We own the memory on the heap for the event listeners list, since we call malloc here. However during the game
	 * the event listener list is swapped in and out to change state. When that happens, whoever switches the list implicitly
	 * takes ownership of the memory for the old list, and gives us control of the memory for the new list. I.E. we always clean
	 * up the list we have, but any other part of the code which switches out lists will need to free the memory for the lists
	 * that are not in use when destroyPanel is called.
	 */
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
