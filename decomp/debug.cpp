/*
 * debug (0x415604-0x415a30): 'generic breakpoint', 'System starvation warning!', 'e2GetPoolValue Error'
 */

#include <string.h>
#include "zoombinis.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "loading.h"
#include "mainloop.h"
#include "os_manager.h"
#include "platform.h"
#include "random.h"

Callback frameHook = 0;
unsigned long starvationLimit = 200;
char msgStarvation[] = "System starvation warning!";
Callback clickHook = 0;
Callback paintHook = 0;

short loadingAnimation;
short clockInTicks;
unsigned long clockStoppedAt;
unsigned long clockOffset;
unsigned long timers[4];
short starvationChecking;
short starvationPaused;
unsigned long lastCheck;
unsigned long thisCheck;

/* @zoombi32 0x00415604 */
void setFrameHook(Callback callback)
{
    frameHook = callback;
}

/* Runs the main loop until an event of a type is waiting (returning 0) or a
   timer runs out (returning 1). */
/* @zoombi32 0x00415666 */
short waitForEventOrTimer(short timer, short type, short discard)
{
    unsigned short found;

    do {
        mainLoopEvents();
        found = isEventWaiting(type, discard);
    } while (!found && !timerExpired(timer));
    return !found;
}

/* @zoombi32 0x004156a3 */
short waitForEventOrTimerWrapped(short timer, short type, short discard)
{
    return waitForEventOrTimer(timer, type, discard);
}

/* Sets a timer for `ticks`, then waits for an event or the timer. */
/* @zoombi32 0x004156be */
short waitForEventFor(unsigned short timer, long ticks, short type, short discard)
{
    setTimer(timer, ticks);
    return waitForEventOrTimer(timer, type, discard);
}

/* @zoombi32 0x004156e3 */
short waitForEventForWrapped(unsigned short timer, long ticks, short type, short discard)
{
    return waitForEventFor(timer, ticks, type, discard);
}

/* @zoombi32 0x00415701 */
void runMainLoop(short passes)
{
    while (passes--)
        mainLoopEvents();
}

/* The game's clock, which stops while the game is inactive (runClock): in
   ms, or in 60ths of a second with clockInTicks. */
/* @zoombi32 0x0041571f */
unsigned long clockTime()
{
    unsigned long now;

    waitWhilePaused();
    now = timerTime() - clockStoppedAt + clockOffset;
    if (clockInTicks)
        now /= 17;
    return now;
}

/* @zoombi32 0x00415753 */
unsigned long clockMs()
{
    short saved = clockInTicks;

    clockInTicks = 0;
    unsigned long now = clockTime();
    clockInTicks = saved;
    return now;
}

/* @zoombi32 0x00415772 */
unsigned long clockTicks()
{
    short saved = clockInTicks;

    clockInTicks = 1;
    unsigned long now = clockTime();
    clockInTicks = saved;
    return now;
}

/* @zoombi32 0x00415791 */
void setTimer(unsigned short timer, long ticks)
{
    timers[timer] = clockTime() + ticks;
}

/* @zoombi32 0x004157ab */
short timerExpired(unsigned short timer)
{
    return clockTime() >= timers[timer];
}

/* Starts or stops the clock. */
/* @zoombi32 0x004157c8 */
void runClock(short running)
{
    if (running)
        clockStoppedAt = timerTime();
    else
        clockOffset += timerTime() - clockStoppedAt;
}

/* Returns whether either flag was set, and clears both. */
/* @zoombi32 0x004157f3 */
short takeStarvationFlags()
{
    short either = starvationChecking | starvationPaused;
    starvationChecking = starvationPaused = 0;
    return either;
}

/* @zoombi32 0x00415811 */
void pauseStarvationCheck()
{
    starvationPaused = 1;
}

/*
 * The main loop's other half (see mainLoopUpdate): handles a waiting
 * message, calls the game's registered callback (frameHook, set by
 * setFrameHook), and in debug mode stops at a requested breakpoint.
 */
/* @zoombi32 0x00415613 */
void mainLoopEvents()
{
    handleWaitingMessage();
    if (!loadingAnimation && frameHook)
        frameHook();
    if (debugMode && breakpointRequested) {
        breakpointRequested = 0;
        debugPrintf("generic breakpoint");
        debugBreak(0);
    }
    checkStarvation();
}

/* @zoombi32 0x0041581b */
void pauseStarvationCheckIf(short flag)
{
    if (flag)
        pauseStarvationCheck();
}

/* Sets the longest time allowed between main loop passes, in ms (or ticks,
   converted). */
/* @zoombi32 0x0041582e */
void setStarvationLimit(unsigned long limit)
{
    starvationLimit = limit;
    if (clockInTicks)
        starvationLimit = starvationLimit * 1000 / 60;
}

/* @zoombi32 0x0041585f */
void checkStarvationKeepingFlags()
{
    short either = starvationChecking | starvationPaused;

    checkStarvation();
    takeStarvationFlags();
    pauseStarvationCheckIf(either);
}

/* In debug mode, stops in the debugger when the main loop hasn't run for
   longer than starvationLimit. */
/* @zoombi32 0x00415880 */
void checkStarvation()
{
    if (debugMode) {
        if (starvationChecking) {
            thisCheck = clockMs();
            if (lastCheck + starvationLimit < thisCheck) {
                starvationChecking = 0;
                starvationPaused = 1;
                debugPrintf(msgStarvation);
                debugBreak(0);
            }
            lastCheck = thisCheck;
        } else if (starvationPaused) {
            starvationChecking = 1;
            starvationPaused = 0;
            lastCheck = thisCheck = clockMs();
        }
    }
}

/* @zoombi32 0x00415910 */
void enterGameDirectory()
{
    enterProgramDirectory();
}

/* @zoombi32 0x00415916 */
void leaveGameDirectory()
{
    freeLoadTexts();
    restoreDirectory();
}

/* Whether the item just after `count` items (of `size` bytes) equals one of
   them. */
/* @zoombi32 0x00415921 */
short isLastRepeated(char *items, unsigned short count, unsigned short size)
{
    char *last;
    short found;
    short i;

    last = items + count * size;
    found = 0;
    for (i = 0; i < count && !found; i++) {
        found = memcmp(last, items, size) == 0;
        items += size;
    }
    return found;
}

/*
 * Picks a free slot at random, marks it used (a bit in *used) and returns
 * it; `reserved` bits count as used. When all are taken, the others are
 * freed; if none were, it's an error.
 */
/* @zoombi32 0x00415978 */
short allocateSlot(unsigned long *used, short count, unsigned long reserved)
{
    short start;
    short i;

    *used |= reserved;
    i = start = randomBelow(count);
    while (*used & (1L << i)) {
        if (++i == count)
            i = 0;
        if (i == start) {
            if (reserved != *used)
                *used = reserved;
            else
                fatalError("e2GetPoolValue Error");
        }
    }
    *used |= 1L << i;
    return i;
}

/* Lower-cases an ASCII letter. */
/* @zoombi32 0x004159dd */
unsigned short toLowerAscii(unsigned short c)
{
    if (c >= 'A' && c <= 'Z')
        c |= 0x20;
    return c;
}

/* Upper-cases an ASCII letter. */
/* @zoombi32 0x004159f7 */
unsigned short toUpperAscii(unsigned short c)
{
    if (c >= 'a' && c <= 'z')
        c &= 0xdf;
    return c;
}

/* @zoombi32 0x00415a11 */
void setClickHook(Callback callback)
{
    clickHook = callback;
}

/* @zoombi32 0x00415a20 */
void setPaintHook(Callback callback)
{
    paintHook = callback;
}
