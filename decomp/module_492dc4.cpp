/*
 * module_492dc4 (Mohawk engine): newTimer
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A timer calling `proc` after `delay` ms and then every `period` ms (0:
   once); 0 on error. */
/* @zoombi32 0x00492dc4 */
long newTimer(unsigned long delay, unsigned long period, TimerProc proc, long data)
{
    TimerEvent *event;

    enterLock(&timerState.lock);
    if ((event = timerState.free) != 0) {
        timerState.free = event->next;
        if (event != timerState.current)
            memset(event, 0, sizeof(TimerEvent));
    } else if ((event = (TimerEvent *)localAlloc(sizeof(TimerEvent))) != 0)
        memset(event, 0, sizeof(TimerEvent));
    else {
        leaveLock(&timerState.lock);
        setTimerError(localMemError());
        return 0;
    }
    event->tag = 0x54457674;
    event->remaining = delay;
    event->period = period;
    event->proc = proc;
    event->data = data;
    event->call.proc = fireTimer;
    event->call.data = event;
    if ((event->active = !startTimer(event)) != 0) {
        setTimerError(0);
        if ((event->next = timerState.active) != 0)
            timerState.active->prev = event;
        event->prev = 0;
        timerState.active = event;
    } else {
        setTimerError(0x27d8);
        event->next = timerState.free;
        timerState.free = event;
        event = 0;
    }
    leaveLock(&timerState.lock);
    return timerId(event);
}
