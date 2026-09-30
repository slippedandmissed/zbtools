/*
 * events (0x413dc0-0x4144d0): the event queue, a 32-entry ring buffer: queuedEvents, nextEventIndex
 */

#include <string.h>
#include "zoombinis.h"
#include "debug.h"
#include "events.h"
#include "graphics.h"
#include "mainloop.h"
#include "os_fixed.h"
#include "platform.h"

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
        gameKey(key);
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
        if (!platformHandlesMouse(where, button))
            mousePressed(where, button);
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
        flushInput(type);
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
    return handleNextMessage();
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

/* Fades the whole palette to `to` (black without it) over half a second. */
/* @zoombi32 0x0041419d */
void fadeTo(PALETTEENTRY *to)
{
    fadePalette(to, 0, 0x100, 0, 0, 0);
}

/* Fades `count` colours from `first` to `to`, running the main loop until
   it's done (see startFade); `fade` defaults to defaultFade. */
/* @zoombi32 0x004141b9 */
void fadePalette(PALETTEENTRY *to, unsigned short first, unsigned short count, short duration,
                 short byTime, Fade **fade)
{
    if (!fade)
        fade = &defaultFade;
    startFade(fade, to, first, count, duration, byTime);
    runFade(fade);
    freeFade(fade);
}

/*
 * Starts fading `count` colours from `first` from what they are to `to`
 * (black without it) over `duration` ms (ticks, with clockInTicks; 500 ms
 * when 0).
 */
/* @zoombi32 0x004141f7 */
void startFade(Fade **fade, PALETTEENTRY *to, unsigned short first, unsigned short count,
               short duration, short byTime)
{
    if (*fade)
        freeFade(fade);
    if (allocateBlock((void **)fade, sizeof(Fade))) {
        getColors((*fade)->start, first, count);
        memcpy((*fade)->current, (*fade)->start, count * sizeof(PALETTEENTRY));
        if (to)
            memcpy((*fade)->target, &to[first], count * sizeof(PALETTEENTRY));
        else
            memset((*fade)->target, 0, count * sizeof(PALETTEENTRY));
        if (!duration)
            duration = 500;
        else if (clockInTicks)
            duration = duration * 50 / 3;
        (*fade)->step = fixedDiv(0x1000000, (long)duration << 16);
        (*fade)->first = first;
        (*fade)->count = count;
        (*fade)->byTime = byTime;
        (*fade)->progress = 0;
        (*fade)->level = (*fade)->steps = 0;
        (*fade)->lastTime = clockMs();
    }
}

/* Runs the main loop until a fade is done (if it changes anything). */
/* @zoombi32 0x00414310 */
void runFade(Fade **fade)
{
    if (*fade && memcmp((*fade)->start, (*fade)->target, sizeof((*fade)->start)))
        while (*fade && !stepFade(*fade))
            mainLoopEvents();
}

/* Advances a fade by the time since the last step and shows the new colours
   if the level changed. Whether it's finished. */
/* @zoombi32 0x00414367 */
short stepFade(Fade *fade)
{
    unsigned long elapsed;
    unsigned short level;
    PALETTEENTRY *target;
    PALETTEENTRY *current;
    PALETTEENTRY *start;
    short i;

    unsigned long now = clockMs();
    elapsed = now - fade->lastTime;
    fade->lastTime = now;
    for (i = 0; elapsed > i; i++)
        fade->progress += fade->step;
    level = fade->progress >> 16;
    if (fade->byTime) {
        if (level == fade->level)
            return 0;
        if (level >= 0x100)
            level = 0x100;
    } else {
        if (level < (fade->steps + 1) * 4)
            return 0;
        fade->steps++;
        level = fade->steps << 2;
    }
    fade->level = level;
    start = fade->start;
    target = fade->target;
    current = fade->current;
    for (i = 0; i < fade->count; i++) {
        current[i].peRed =
            ((unsigned)((target[i].peRed - start[i].peRed) * level) >> 8) + start[i].peRed;
        current[i].peGreen =
            ((unsigned)((target[i].peGreen - start[i].peGreen) * level) >> 8) + start[i].peGreen;
        current[i].peBlue =
            ((unsigned)((target[i].peBlue - start[i].peBlue) * level) >> 8) + start[i].peBlue;
    }
    setColors(fade->current, fade->first, fade->count);
    return level == 0x100;
}

/* @zoombi32 0x00414358 */
void freeFade(Fade **fade)
{
    freeAndClear((void **)fade);
}
