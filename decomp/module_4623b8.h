/*
 * module_4623b8's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef MODULE_4623B8_H
#define MODULE_4623B8_H

extern char aboutText[]; /* @data 0x4a5156: "Logical Journey of the Zoombinis\rVersion 1.0..." */
extern short g_4a79c0;
extern const char *toggleTexts[15]; /* @data 0x4a5880: "*", "music on", "music off", ... */
extern unsigned short midiTests[18]; /* @data 0x4a7ad4: the sounds the MIDI test plays */
extern short g_4a7af8; /* @data 0x4a7af8 */
extern unsigned short midiTest; /* @data 0x4b80e4: the space bar plays midiTests */
extern short midiTestIndex; /* @data 0x4b80e6 */
extern short viewStep; /* @data 0x4b80e8: in step mode, the view labelled */
extern unsigned long g_4a79c8;
extern unsigned long g_4b80d8;
extern unsigned long g_4b80dc;
void mousePressed(Point *where, short button);
void showNormalCursor();
short setCursorMode(long mode);
void showBusyCursor();
void showAboutBox();
void setModeCursor();
void debugMessage(short value, const char *after, short *number, const char *before, short wait);
void gameKey(unsigned short key);
short mainLoopUpdate();

#endif
