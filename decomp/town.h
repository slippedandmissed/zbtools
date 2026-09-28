/*
 * town's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TOWN_H
#define TOWN_H

extern GroupList townGroups[1]; /* @data 0x4a73f0 */
extern SceneButton townButtons[1]; /* @data 0x4a7428 */
extern unsigned char clockMinute; /* @data 0x4a751e: the clock's minute hand (0-11) */
extern unsigned char clockHour; /* @data 0x4a751f: and hour hand (0-11) */
extern short g_4b7cec; /* @data 0x4b7cec */
extern unsigned long g_4b7cf0; /* @data 0x4b7cf0 */
extern short g_4b7cf4; /* @data 0x4b7cf4: the scene is open */
extern unsigned short g_4b7cf6; /* @data 0x4b7cf6 */
extern unsigned long g_4b7efc; /* @data 0x4b7efc: when the clock was read (ticks) */
extern short g_4b7e10; /* @data 0x4b7e10 */
extern short g_4b7ece[20]; /* @data 0x4b7ece: views (negated once notified, fn_45e29e) */
extern short g_4b7f02; /* @data 0x4b7f02: party views to set running (fn_45ccca) */
extern short g_4b7f10; /* @data 0x4b7f10: views notified (fn_45e29e) */

extern long g_4a74c4; /* @data 0x4a74c4 */
extern ImageBank *g_4a74c8; /* @data 0x4a74c8: the buttons' images */
extern char g_4a7582[]; /* @data 0x4a7582 */
extern long g_4b7dfc; /* @data 0x4b7dfc: Town.MHK */
extern short g_4b7e00; /* @data 0x4b7e00: scene 6 is open */
extern ShortRect g_4b7e12[16]; /* @data 0x4b7e12: hotspots */
extern short g_4b7e92[16]; /* @data 0x4b7e92: their numbers */
extern short g_4b7eb2; /* @data 0x4b7eb2: how many */
extern short g_4b7eb4; /* @data 0x4b7eb4: the cursor is on one */
extern short g_4b7eb6; /* @data 0x4b7eb6: its number (from 1) */
extern short g_4b7eb8; /* @data 0x4b7eb8: a script for it */

void openScene0();
void closeScene0();
void scrollTown(short left);
void drawTownButton(short which, short lit, short show);
void closeScene6();
void fn_45d715(Point *where);
void scene0Clicked(short which);
void fn_45c4c9();
void fn_45ccca(short running);
void fn_45cf8b(View *view, short region);
short fn_45d04c();
void fn_45daf7(View *view);
void fn_45e29e(View *view, short event);

#endif
