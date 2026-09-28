/*
 * picker's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PICKER_H
#define PICKER_H

/* A place to click on the picker's screens (scenes 19 and 21). */
struct PickerHotspot
{
    ShortRect rect; /* 40 by 30 around its point */
    char unknown8[28];
};

/* The picker's data from 0x4af8ac, laid out differently by its uses. */
struct PickerData
{
    union {
        PickerHotspot hotspots[17]; /* scenes 19 and 21; the last is the whole screen */
        short view; /* deleted by fn_431e5e */
        struct
        {
            char unknown0[0x24];
            short unknown24; /* shown by fn_4320e3 */
            short unknown26; /* 0-99, shown by fn_4320e3; the most is kept in the roster (+0x22) */
            short unknown28;
            short speed; /* +0x2a: the walking Zoombinis' interval (fn_431ea0) */
        } counts;
    };
};

extern PickerData pickerData; /* @data 0x4af8ac */

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

void fn_4320e3(View *view);
extern short g_4afb16; /* @data 0x4afb16 */
extern MapSave *g_4afb18[6]; /* @data 0x4afb18 */
extern short g_4afb36; /* @data 0x4afb36 */
extern short g_4afb38; /* @data 0x4afb38 */
extern short g_4afb3c; /* @data 0x4afb3c */
extern short g_4afb3e; /* @data 0x4afb3e */
extern short g_4afb40; /* @data 0x4afb40 */
extern ShortRect g_4afb42; /* @data 0x4afb42 */
extern short g_4afb5c; /* @data 0x4afb5c */
extern short g_4afb5e; /* @data 0x4afb5e */
void fn_42f920();
extern short g_4afb72; /* @data 0x4afb72 */
extern short g_4afb76; /* @data 0x4afb76 */
void fn_430030(short n);
void fn_4333ef(View *view);
extern short g_4afb8c; /* @data 0x4afb8c: drifting views started */
short fn_43297f();
extern char g_4afb4a[17]; /* @data 0x4afb4a: the hotspots open (from 1) */
void fn_430cb3(View *view);
void fn_430dc0(View *view);
void fn_4312e2(char *open);
void fn_430b31(ShortRect *rect);
void fn_4328e2(short which);
void fn_43108f(View *view, volatile short region);
void fn_43160a(View *view);
extern ShortRect g_4a1f54[4]; /* @data 0x4a1f54: where the terrains' names go */
void fn_431111();
extern short g_4afb74; /* @data 0x4afb74: the next hundred to score */
extern short g_4afb78; /* @data 0x4afb78 */
extern short g_4afb8a; /* @data 0x4afb8a: the first shot stopped */
extern short g_4afb8e; /* @data 0x4afb8e: the target hit (from 1) */
extern ShortRect *g_4afb94[6]; /* @data 0x4afb94: the targets' bounds */
extern short g_4afbac[6]; /* @data 0x4afbac: the targets' views */
void fn_432eff(View *view);

short fn_431ea0();
extern short g_4afb6c; /* @data 0x4afb6c */
extern short g_4afb6e; /* @data 0x4afb6e */
extern short g_4afb70; /* @data 0x4afb70 */
extern short g_4afbba; /* @data 0x4afbba: targets started */
short fn_4330f3(short kind, short preset);
extern char savedUserFile[]; /* @data 0x4a1f84: the user file while practising (in ZBtemp) */
void closeScene1();

#endif
