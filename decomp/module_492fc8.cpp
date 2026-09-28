/*
 * module_492fc8 (Mohawk engine): the timers (multimedia timer events)
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "debug.h"
#include "os_localmem.h"
#include "os_refcount.h"

TimerState timerState;

/* The multimedia timer's callback: runs the event's call under the lock. */
/* @zoombi32 0x00492fc8 */
void CALLBACK timerCallback(UINT, UINT, DWORD data, DWORD, DWORD)
{
    deferCall(&timerState.lock, (Deferred *)data);
}

/* A timer event's step: counts down a long delay, else calls its procedure
   (and reschedules it or, if it fires once, frees it). */
/* @zoombi32 0x00492fdf */
void fireTimer(void *data)
{
    TimerEvent *previous;

    enterLock(&timerState.lock);
    TimerEvent *event = (TimerEvent *)data;
    if (event->interval >= event->remaining) {
        if ((event->remaining = event->period) == 0) {
            event->active = 0;
            if (event->next)
                event->next->prev = event->prev;
            if (event->prev)
                event->prev->next = event->next;
            else
                timerState.active = event->next;
            event->next = timerState.free;
            timerState.free = event;
        } else if (event->oneShot)
            startTimer(event);
        else
            event->due = timeGetTime() + event->interval;
        previous = timerState.current;
        timerState.current = event;
        event->proc(timerId(event), event->data);
        timerState.current = previous;
    } else {
        event->remaining -= event->interval;
        startTimer(event);
    }
    leaveLock(&timerState.lock);
}

/* @zoombi32 0x0049308f */
short timerError()
{
    return timerState.error;
}

/* Starts the timers, at the timer device's finest resolution. */
/* @zoombi32 0x00493096 */
short initTimers()
{
    memset(&timerState, 0, sizeof timerState);
    if (timeGetDevCaps(&timerState.caps, sizeof(TIMECAPS)))
        return setTimerError(0x27d8);
    if (timeBeginPeriod(timerState.caps.wPeriodMin))
        return setTimerError(0x27d8);
    initLock(&timerState.lock, 1);
    timerState.ready = 1;
    return setTimerError(0);
}

/* @zoombi32 0x00493102 */
short timerBufferSize()
{
    return timerState.ready ? 0x500 : 0;
}

/* Stops every timer and the timerState. */
/* @zoombi32 0x00493116 */
void closeTimers()
{
    TimerEvent *event;

    enterLock(&timerState.lock);
    while (timerState.active)
        killTimer(timerId(timerState.active));
    leaveLock(&timerState.lock);
    timeEndPeriod(timerState.caps.wPeriodMin);
    while (timerState.free) {
        event = timerState.free;
        timerState.free = event->next;
        localFree(event);
    }
    removeLock(&timerState.lock);
    timerState.ready = 0;
}

/* Sets a multimedia timer for the event's next step: its delay, clamped to
   what the device allows, and made shorter if it's late (a one-shot timer
   then, reset each step); non-zero on error. */
/* @zoombi32 0x0049317b */
unsigned short startTimer(TimerEvent *event)
{
    unsigned long delay;
    unsigned long now;

    event->interval = event->remaining < timerState.caps.wPeriodMax
                          ? event->remaining > timerState.caps.wPeriodMin ? event->remaining
                                                                      : timerState.caps.wPeriodMin
                          : timerState.caps.wPeriodMax;
    delay = event->interval;
    now = timeGetTime();
    if (event->oneShot && now > event->due) {
        unsigned long late = now - event->due;
        if (delay <= late || (delay -= late) < timerState.caps.wPeriodMin)
            delay = timerState.caps.wPeriodMin;
    }
    event->due = now + delay;
    event->oneShot = event->remaining != event->period || event->remaining != event->interval
                     || delay != event->interval;
    event->id = timeSetEvent(delay, 0, timerCallback, (DWORD)&event->call,
                             event->oneShot ? TIME_ONESHOT : TIME_PERIODIC);
    return !event->id;
}

/* @zoombi32 0x00493231 */
short setTimerError(short error)
{
    return timerState.error = error;
}

/* @zoombi32 0x00493242 */
long timerId(TimerEvent *event)
{
    return (long)event;
}
