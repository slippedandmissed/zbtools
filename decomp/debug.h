/*
 * debug's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef DEBUG_H
#define DEBUG_H

extern unsigned long starvationLimit; /* @data 0x4a07c8: longest gap between main loop passes */
extern char msgStarvation[]; /* @data 0x4a07cc */
extern Callback g_4a07c4;
extern unsigned long clockStoppedAt; /* @data 0x4ab484 */
extern unsigned long clockOffset; /* @data 0x4ab488 */
extern unsigned long timers[4]; /* @data 0x4ab48c: when each expires */
extern short g_4ab49c;
extern short g_4ab49e;
extern unsigned long lastCheck; /* @data 0x4ab4a0 */
extern unsigned long thisCheck; /* @data 0x4ab4a4 */
void fn_415910();
unsigned long fn_41571f(); /* a tick count */
unsigned long fn_415772(); /* a tick count */
void checkStarvation();
short waitForEventOrTimer(short timer, short type, short discard);
short fn_4156a3(short timer, short type, short discard);
short waitForEventFor(unsigned short timer, long ticks, short type, short discard);
short fn_4156e3(unsigned short timer, long ticks, short type, short discard);
void runMainLoop(short passes);
unsigned long clockTime();
unsigned long clockMs();
unsigned long clockTicks();
void setTimer(unsigned short timer, long ticks);
short timerExpired(unsigned short timer);
void setStarvationLimit(unsigned long limit);
void fn_415916();
short isLastRepeated(char *items, unsigned short count, unsigned short size);
short allocateSlot(unsigned long *used, short count, unsigned long reserved);
void fn_41585f();
void runClock(short running);
void fn_415604(Callback callback);
short fn_4157f3();
void fn_415811();
void fn_41581b(short flag);
unsigned short toLowerAscii(unsigned short c);
unsigned short toUpperAscii(unsigned short c);
void fn_415a11(Callback callback);
void fn_415a20(Callback callback);
void mainLoopEvents();

#endif
