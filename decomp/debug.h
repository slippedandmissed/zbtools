/*
 * debug's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef DEBUG_H
#define DEBUG_H

extern unsigned long starvationLimit; /* @data 0x4a07c8: longest gap between main loop passes */
extern char msgStarvation[]; /* @data 0x4a07cc */
extern Callback frameHook;
extern unsigned long clockStoppedAt; /* @data 0x4ab484 */
extern unsigned long clockOffset; /* @data 0x4ab488 */
extern unsigned long timers[4]; /* @data 0x4ab48c: when each expires */
extern short starvationChecking;
extern short starvationPaused;
extern unsigned long lastCheck; /* @data 0x4ab4a0 */
extern unsigned long thisCheck; /* @data 0x4ab4a4 */
void enterGameDirectory();
unsigned long fn_41571f(); /* a tick count */
unsigned long fn_415772(); /* a tick count */
void checkStarvation();
short waitForEventOrTimer(short timer, short type, short discard);
short waitForEventOrTimerWrapped(short timer, short type, short discard);
short waitForEventFor(unsigned short timer, long ticks, short type, short discard);
short waitForEventForWrapped(unsigned short timer, long ticks, short type, short discard);
void runMainLoop(short passes);
unsigned long clockTime();
unsigned long clockMs();
unsigned long clockTicks();
void setTimer(unsigned short timer, long ticks);
short timerExpired(unsigned short timer);
void setStarvationLimit(unsigned long limit);
void leaveGameDirectory();
short isLastRepeated(char *items, unsigned short count, unsigned short size);
short allocateSlot(unsigned long *used, short count, unsigned long reserved);
void checkStarvationKeepingFlags();
void runClock(short running);
void setFrameHook(Callback callback);
short takeStarvationFlags();
void pauseStarvationCheck();
void pauseStarvationCheckIf(short flag);
unsigned short toLowerAscii(unsigned short c);
unsigned short toUpperAscii(unsigned short c);
void setClickHook(Callback callback);
void setPaintHook(Callback callback);
void mainLoopEvents();

#endif
