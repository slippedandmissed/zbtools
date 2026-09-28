/*
 * picker's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PICKER_H
#define PICKER_H

extern short g_4afb90;
void fn_43151e();
void fn_42fc89(Counters *object);
long fn_4320da(long);
void fn_4334f0(long, short value);
extern long g_4afb10; /* @data 0x4afb10: Picker.MHK */
extern short g_4afb14; /* @data 0x4afb14: the scene is open */
extern short g_4afbb8; /* @data 0x4afbb8 */
extern basePort **g_4afb28; /* @data 0x4afb28 */
extern short g_4afb34; /* @data 0x4afb34 */
extern short g_4afb3a; /* @data 0x4afb3a */
extern short g_4af8ac; /* @data 0x4af8ac */
extern ShortRect g_4a1f74; /* @data 0x4a1f74 */
void closeScene19();
void closeScene20();
void fn_431e5e(View *, short event);
extern short g_4afb7c; /* @data 0x4afb7c */
extern short g_4afb7e; /* @data 0x4afb7e */
extern short g_4afb80; /* @data 0x4afb80 */
extern short g_4afb82; /* @data 0x4afb82 */
extern short g_4afb84; /* @data 0x4afb84 */
extern short g_4afb88; /* @data 0x4afb88 */
void fn_432cec(View *view);
void fn_430f8e(View *view);
void driftView(View *view);
void fn_432905();
void fn_430ff2(View *view);

#endif
