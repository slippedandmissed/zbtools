/*
 * killtimer (Mohawk engine): killTimer, freeTimer, timerEvent
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_refcount.h"

/* Stops a timer. If its call is waiting to run under the lock, it's freed
   when it does. */
/* @zoombi32 0x00492ec0 */
short killTimer(LONG_PTR timer)
{
    TimerEvent *event;
    short error;

    enterLock(&timerState.lock);
    if ((event = timerEvent(timer)) == 0)
        error = 0x280b;
    else if (event->active) {
        timeKillEvent(event->id);
        event->active = 0;
        if (event->next)
            event->next->prev = event->prev;
        if (event->prev)
            event->prev->next = event->next;
        else
            timerState.active = event->next;
        if (event->call.count > 0) {
            event->call.count = 1;
            event->call.proc = (void (*)(void *))freeTimer;
        } else {
            event->next = timerState.free;
            timerState.free = event;
        }
        error = 0;
    } else if (event == timerState.current)
        error = 0;
    else
        error = 0x280b;
    leaveLock(&timerState.lock);
    return setTimerError(error);
}

/* @zoombi32 0x00492f69 */
void freeTimer(TimerEvent *event)
{
    event->next = timerState.free;
    timerState.free = event;
}

/* A timer's event; 0 if it isn't one. */
/* @zoombi32 0x00492f81 */
TimerEvent *timerEvent(LONG_PTR timer)
{
    TimerEvent *event = (TimerEvent *)timer;

    if (event && event->tag == 0x54457674)
        return event;
    return 0;
}
