/*
 * bctwo's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BCTWO_H
#define BCTWO_H

/* One of the Zoombinis waiting at the camp (22 bytes). */
struct CampEntry
{
    long zoombini; /* 0: none */
    char unknown4[18];
};

/* The Zoombinis waiting at the camp, shown in rows of 5. */
struct CampEntries
{
    short row; /* the first shown (g_4ab640) */
    short count; /* g_4ab646 */
    CampEntry entries[625];
};

extern short g_4ab640; /* @data 0x4ab640 */
extern short g_4ab642; /* @data 0x4ab642 */
extern short g_4ab644; /* @data 0x4ab644 */
extern short g_4ab646; /* @data 0x4ab646 */
extern short g_4ab648; /* @data 0x4ab648 */
extern short g_4ab64a; /* @data 0x4ab64a */
extern CampEntries *g_4ab64c;
extern short g_4ab650; /* @data 0x4ab650 */
extern short g_4ab652; /* @data 0x4ab652 */
extern short g_4ab65c; /* @data 0x4ab65c */
extern short g_4ab65e; /* @data 0x4ab65e */
extern short g_4ab662; /* @data 0x4ab662 */
extern short g_4ab664; /* @data 0x4ab664 */
extern short g_4ab666; /* @data 0x4ab666 */
extern short g_4ab668; /* @data 0x4ab668 */
extern short g_4ab67e; /* @data 0x4ab67e */
extern short g_4a0abc; /* @data 0x4a0abc: the scroll button pressed (1-4) */
extern SceneButton campButtons[7]; /* @data 0x4a0adc: fn_4196b1 draws them */
extern ResourceList *g_4a0ac4; /* @data 0x4a0ac4: the camp's images */
extern ShortRect g_4a0cea; /* @data 0x4a0cea */
void resetScene5();
void fn_419867(View *view, short region);
void fn_419e49(short n);
long fn_4196a8(long);
void fn_41a225();
void fn_41a024();
void fn_4196b1(short button, short lit, short group, short show);
void fn_41983f(View *view);
void fn_419853(View *view);
void fn_41a11b(short quiet, short);
short fn_419f1a();

#endif
