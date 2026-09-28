/*
 * slides's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SLIDES_H
#define SLIDES_H

void fn_4489ce(View *view);
extern long g_4b1928; /* @data 0x4b1928: Slides.MHK */
extern short g_4b1930; /* @data 0x4b1930: the scene is open */
extern short g_4b1932; /* @data 0x4b1932 */
extern long g_4a3fc8; /* @data 0x4a3fc8 */
extern SceneButton slidesButtons[2]; /* @data 0x4a3f28: buttons 1 and 2 */
extern short g_4a41e0; /* @data 0x4a41e0: button 2 is drawn lit */
extern short g_4a41e2; /* @data 0x4a41e2: button 1 has been drawn */
extern short g_4b1ab4[27]; /* @data 0x4b1ab4: cells (from 1, g_4b240e of them) */
extern short g_4b240e; /* @data 0x4b240e */
extern short g_4b1a42; /* @data 0x4b1a42 */
extern short g_4b1a44; /* @data 0x4b1a44: the sum of the counted cells' numbers */
extern short g_4b1a46; /* @data 0x4b1a46 */
void closeScene12();
void fn_4470b2(View *, short region);
void fn_44943b();
void fn_449475();
short fn_44b261();
void fn_44b2a4();
extern ImageBank *g_4a3fc4; /* @data 0x4a3fc4: the buttons' images */
extern short g_4b251c[4]; /* @data 0x4b251c: how many values of each feature the party shows */
extern ChosenSnoids *g_4b192c; /* @data 0x4b192c: the party */
void drawSlidesButton(short which, short lit, short show);
void fn_4488e8();
void fn_448bf5();
extern short g_4b2450; /* @data 0x4b2450 */
extern short g_4b2452[16]; /* @data 0x4b2452 */
void fn_449b40();
void fn_449c18();
void fn_448c81(View *view);
extern Point slidesPlaces[16]; /* @data 0x4a41a0: where the Zoombinis wait */
void fn_44b3ee(Point *where);
short fn_449a21(short who);
extern short g_4b2514; /* @data 0x4b2514 */
void fn_44aa79();
void fn_448d9d(View *view);
extern Point cellPoints[117]; /* @data 0x4a3fcc: where each cell is drawn */
extern short g_4b1a3e; /* @data 0x4b1a3e: a cell is marked */
extern short g_4b1a36; /* @data 0x4b1a36: the cell marked */
extern short g_4b1a34; /* @data 0x4b1a34: the marker's view */
extern short g_4b1936[8]; /* @data 0x4b1936: views, by row */
void fn_44af15(View *view, short event);
void drawSlidesButtons(View *);
void fn_4489a8(View *view, short);
void fn_44b0fc(short x, short y);
extern short g_4b1a38; /* @data 0x4b1a38: the Zoombini walking to the marked cell */
extern short g_4b1a3a; /* @data 0x4b1a3a: the facing to take (from 1) */
void fn_44986f();
short fn_449f96(short a, short b);

#endif
