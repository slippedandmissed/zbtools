/*
 * module_4623b8's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef MODULE_4623B8_H
#define MODULE_4623B8_H

extern char aboutText[]; /* @data 0x4a5156: "Logical Journey of the Zoombinis\rVersion 1.0..." */
extern short g_4a79c0;
extern unsigned long g_4a79c8;
extern unsigned long g_4b80d8;
extern unsigned long g_4b80dc;
void fn_4624bd(Point *where, short button);
void fn_4624f4();
short setCursorMode(long mode);
void fn_4624fc();
void fn_4625b8();
void fn_46258a();
void debugMessage(short value, const char *after, short *number, const char *before, short wait);
void fn_46293a(unsigned short key);
short mainLoopUpdate();

#endif
