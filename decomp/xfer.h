/*
 * xfer's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef XFER_H
#define XFER_H

extern long g_4b98d4; /* @data 0x4b98d4: xfer.MHK */
extern short g_4b98d8; /* @data 0x4b98d8: the scene is open */
extern short g_4b98da; /* @data 0x4b98da: Zoombiniville's population, when the scene opened */
extern short g_4b98dc; /* @data 0x4b98dc */
extern long g_4b98e0; /* @data 0x4b98e0 */
extern char g_4b98e4[17]; /* @data 0x4b98e4: the places' levels (fn_46b084) */
extern GroupList xferGroups[1]; /* @data 0x4a7e9e */
extern ShortRect mapTitleRects[4]; /* @data 0x4a7ebe: where each map's name goes */
extern short g_4a7ede; /* @data 0x4a7ede: in scene2Frame */
extern short g_4a7ee0[4][5]; /* @data 0x4a7ee0: the places on each of g_4b9916's maps */
extern Point g_4a7f08[16]; /* @data 0x4a7f08: where each map's grid starts */
extern short g_4b98f6; /* @data 0x4b98f6 */
extern short g_4b98f8[4]; /* @data 0x4b98f8 */
extern short g_4b9900[2]; /* @data 0x4b9900 */
extern short g_4b9904; /* @data 0x4b9904 */
extern short g_4b9906; /* @data 0x4b9906 */
extern short g_4b9908; /* @data 0x4b9908 */
extern short g_4b990a; /* @data 0x4b990a */
extern short g_4b990c[3]; /* @data 0x4b990c */
extern short g_4b9910; /* @data 0x4b9910 */
extern short g_4b9912; /* @data 0x4b9912 */
extern short g_4b9914; /* @data 0x4b9914 */
extern short g_4b9916; /* @data 0x4b9916 */
extern short g_4b9918; /* @data 0x4b9918 */
extern short g_4b991a; /* @data 0x4b991a */
extern short g_4b991c; /* @data 0x4b991c */
extern short g_4b991e; /* @data 0x4b991e */
extern short g_4b9920; /* @data 0x4b9920 */
extern short g_4b9922; /* @data 0x4b9922 */
extern short g_4b9924; /* @data 0x4b9924 */
extern short g_4b9926; /* @data 0x4b9926 */
extern short g_4b9928; /* @data 0x4b9928 */
extern unsigned long g_4b992c; /* @data 0x4b992c: fn_46bc51's progress (per mille) */
extern unsigned long g_4b9930; /* @data 0x4b9930 */
extern unsigned long g_4b9934; /* @data 0x4b9934 */
extern unsigned long g_4b9938; /* @data 0x4b9938: the grid's stride */
extern unsigned long g_4b993c; /* @data 0x4b993c: its rows */
extern unsigned long g_4b9940; /* @data 0x4b9940: its columns */
extern Point g_4b9944[24]; /* @data 0x4b9944 */
extern char g_4b99a4[24]; /* @data 0x4b99a4: g_4b9944's in use */
extern char *g_4b99bc; /* @data 0x4b99bc: the grid */
extern char g_4b99c0; /* @data 0x4b99c0 */
extern char g_4b99c1; /* @data 0x4b99c1 */
extern char g_4b99c2; /* @data 0x4b99c2 */
extern char g_4b99c3; /* @data 0x4b99c3 */

void resetScene2();
void openScene2();
void closeScene2();
void fn_46b761(View *view);
void fn_46b084(char *levels);
void fn_46b326(View *view);
void scene2Frame();
void scene2Clicked(short which);
unsigned long fn_46b9a2(long permille);
void fn_46b5ce(View *view, short event);
void fn_46b872(char *grid, unsigned long stride, unsigned long rows, unsigned long columns,
                unsigned char from1, unsigned char from2, char to1, char to2, char taken1,
                char taken2, Point &start);
void fn_46bb0c(char *cell, long x, long y);
void fn_46bbce(View *view);
void fn_46bc51(View *view);
void fn_46bdde(View *view, short region);
long scene2Key(long);
void fn_46b747(View *, short event);

#endif
