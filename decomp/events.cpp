/*
 * module_413dc0 (0x413dc0-0x4144d0): no strings; a 32-entry ring buffer (an event queue?): queuedEvents, nextEventIndex
 */

#include "zoombinis.h"

/* How many events are queued (eventHead is where reading starts, eventTail
   where writing does). */
/* @zoombi32 0x00413dc0 */
short queuedEvents()
{
    short count = eventTail - eventHead;
    if (count < 0)
        count += 32;
    return count;
}

/* Queues an event, unless the queue is full. */
/* @zoombi32 0x00413dd7 */
void postEvent(Event *event)
{
    if (queuedEvents() < 31) {
        eventQueue[eventTail] = *event;
        nextEventIndex(&eventTail);
    }
}

/* Takes the next event; whether there was one. */
/* @zoombi32 0x00413e1b */
short getEvent(Event *event)
{
    if (eventTail == eventHead)
        return 0;
    *event = eventQueue[eventHead];
    nextEventIndex(&eventHead);
    return 1;
}

/* Whether an event of a type (1 a key, 2 a mouse button, 3 either) is queued. */
/* @zoombi32 0x00413e6b */
short hasEvent(short type)
{
    short i;

    if (eventHead == eventTail)
        return 0;
    if (type == 3)
        return 1;
    for (i = eventHead; i != eventTail; nextEventIndex(&i))
        if (eventQueue[i].type == type)
            return 1;
    return 0;
}

/* Removes the queued events of a type (3: all), keeping the others in order. */
/* @zoombi32 0x00413ed7 */
void removeEvents(short type)
{
    short removed, from, to;

    if (eventTail == eventHead)
        return;
    if (type == 3) {
        eventTail = eventHead;
        return;
    }
    removed = 0;
    for (from = to = eventHead; from != eventTail && eventQueue[from].type != type;) {
        if (++from > 31)
            from = 0;
        if (++to > 31)
            to = 0;
    }
    while (from != eventTail) {
        if (eventQueue[from].type == type)
            removed++;
        else {
            eventQueue[to] = eventQueue[from];
            if (++to > 31)
                to = 0;
        }
        if (++from > 31)
            from = 0;
    }
    if (eventTail >= removed)
        eventTail -= removed;
    else
        eventTail += 31 - removed;
}

/* A key was pressed: queue it, or handle it now if events are being
   dispatched. The breakpoint key (in debug builds) requests a breakpoint. */
/* @zoombi32 0x00413fdd */
void postKeyEvent(short key)
{
    Event event;
    short dispatching = dispatchingEvents;

    dispatchingEvents = 0;
    if (breakpointKeyEnabled && key == breakpointKey)
        breakpointRequested = 1;
    else if (dispatching)
        fn_46293a(key);
    else {
        event.type = 1;
        event.code = key;
        postEvent(&event);
    }
}

/* A mouse button was pressed: queue it, or handle it now if events are being
   dispatched. */
/* @zoombi32 0x0041403a */
void postMouseEvent(Point *where, short button)
{
    Event event;
    short dispatching = dispatchingEvents;

    dispatchingEvents = 0;
    if (dispatching) {
        if (!fn_455e85(where, button))
            fn_4624bd(where, button);
    } else {
        event.type = 2;
        event.where = *where;
        event.code = button;
        postEvent(&event);
    }
}

/* Whether an event of a type is waiting, queued or not yet taken from
   Windows; with `discard`, throws such events away. */
/* @zoombi32 0x00414091 */
short isEventWaiting(short type, short discard)
{
    waitWhilePaused();
    if (!type)
        return 0;
    if (hasEvent(type) || isInputWaiting(type)) {
        if (discard)
            discardEvents(type);
        return 1;
    }
    return 0;
}

/* Throws away the events of a type, queued or not yet taken from Windows. */
/* @zoombi32 0x004140d6 */
void discardEvents(short type)
{
    if (type) {
        removeEvents(type);
        fn_455ab0(type);
    }
}

/* Handles the next queued event (or else one from Windows); its type. */
/* @zoombi32 0x004140f3 */
short handleNextEvent()
{
    Event event;

    dispatchingEvents = 1;
    if (getEvent(&event)) {
        if (event.type == 1)
            postKeyEvent(event.code);
        else
            postMouseEvent(&event.where, event.code);
        return event.type;
    }
    return fn_45590b();
}

/* @zoombi32 0x00414140 */
void getMousePosition(Point *where)
{
    getCursorPosition(where);
}

/* Runs the main loop's events until an event of a type is waiting. */
/* @zoombi32 0x0041414f */
void waitForEvent(short type, short discard)
{
    do
        mainLoopEvents();
    while (!isEventWaiting(type, discard));
}

/* Advances an index into the queue, wrapping to 0. */
/* @zoombi32 0x0041416f */
void __cdecl nextEventIndex(short *index)
{
    if (++*index > 31)
        *index = 0;
}

/* @zoombi32 0x00414358 */
void fn_414358(void **block)
{
    freeAndClear(block);
}
