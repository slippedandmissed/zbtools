/*
 * events's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef EVENTS_H
#define EVENTS_H

extern short breakpointKey; /* @data 0x4a0708 */
extern short breakpointKeyEnabled; /* @data 0x4aa5d4 */
extern Event eventQueue[32]; /* @data 0x4aa5da */
extern short eventHead; /* @data 0x4aa79a */
extern short eventTail; /* @data 0x4aa79c */
extern Fade *defaultFade; /* @data 0x4aa7a0 */
short queuedEvents();
void postEvent(Event *event);
short getEvent(Event *event);
short hasEvent(short type);
void removeEvents(short type);
void postKeyEvent(short key);
void postMouseEvent(Point *where, short button);
short isEventWaiting(short type, short discard);
void discardEvents(short type);
short handleNextEvent();
void getMousePosition(Point *where);
void waitForEvent(short type, short discard);
void __cdecl nextEventIndex(short *index);
void freeFade(Fade **fade);
void fadeTo(PALETTEENTRY *to);
void fadePalette(PALETTEENTRY *to, unsigned short first, unsigned short count, short duration,
                 short byTime, Fade **fade);
void startFade(Fade **fade, PALETTEENTRY *to, unsigned short first, unsigned short count,
               short duration, short byTime);
void runFade(Fade **fade);
short stepFade(Fade *fade);

#endif
