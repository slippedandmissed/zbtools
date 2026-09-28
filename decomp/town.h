/*
 * town's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TOWN_H
#define TOWN_H

extern GroupList townGroups[1]; /* @data 0x4a73f0 */
extern ShortRect g_4a7428; /* @data 0x4a7428 */
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

void openScene0();
void scene0Clicked(short which);
void fn_45c4c9();
void fn_45ccca(short running);
void fn_45cf8b(View *view, short region);
short fn_45d04c();
void fn_45daf7(View *view);
void fn_45e29e(View *view, short event);

#endif
