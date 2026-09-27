/*
 * module_492f9c (Mohawk engine): lockTimers, unlockTimers, timerTime
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Holds timers' procedures until unlockTimers. */
/* @zoombi32 0x00492f9c */
void lockTimers()
{
    enterLock(&timerState.lock);
}

/* @zoombi32 0x00492fac */
void unlockTimers()
{
    leaveLock(&timerState.lock);
}

/* @zoombi32 0x00492fbc */
unsigned long timerTime()
{
    return timeGetTime();
}
