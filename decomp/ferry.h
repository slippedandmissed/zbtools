/*
 * ferry's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FERRY_H
#define FERRY_H

extern short g_4a1440[]; /* @data 0x4a1440: scripts by g_4abaee */
extern ImageBank *g_4a147c; /* @data 0x4a147c: the buttons' images */
extern SceneButton ferryButtons[2]; /* @data 0x4a1480 */
extern long g_4a151c; /* @data 0x4a151c */
extern short g_4a1570; /* @data 0x4a1570: button 2 is drawn lit */
extern short g_4a1572; /* @data 0x4a1572: button 1 is drawn */
extern Point *g_4aba98; /* @data 0x4aba98 */
extern short g_4abaa2; /* @data 0x4abaa2 */
extern short g_4abaa4; /* @data 0x4abaa4 */
extern long g_4abaa8; /* @data 0x4abaa8: Ferry.MHK */
extern short g_4abaac; /* @data 0x4abaac: the scene is open */
extern short g_4abaae; /* @data 0x4abaae: button 2 is live */
extern short g_4ababe; /* @data 0x4ababe */
extern short g_4abac0; /* @data 0x4abac0 */
extern short g_4abac2; /* @data 0x4abac2 */
extern short g_4abaee; /* @data 0x4abaee */
extern short g_4abaf2; /* @data 0x4abaf2 */
extern void *g_4abaf8; /* @data 0x4abaf8 */

void drawFerryButton(short which, short lit, short show);
void fn_41fea4(View *, short region);
void closeScene10();
void fn_4209b8();
void fn_420a08(short group, short script, ViewNotify notify, char unknownF8);
void fn_420c82(View *view, short event);
void fn_42113f(View *, short dx);

#endif
