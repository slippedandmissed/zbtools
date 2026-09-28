/*
 * ferry's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FERRY_H
#define FERRY_H

extern short g_4a1440[]; /* @data 0x4a1440: scripts by g_4abaee */
extern short g_4a1424[5]; /* @data 0x4a1424: sounds (g_4a1430 picks) */
extern unsigned long g_4a1430; /* @data 0x4a1430: slots used (allocateSlot) */
extern short g_4a1454[10]; /* @data 0x4a1454: scripts by g_4abaee */
extern short g_4a1468[10]; /* @data 0x4a1468: and the next */
extern ImageBank *g_4a147c; /* @data 0x4a147c: the buttons' images */
extern SceneButton ferryButtons[2]; /* @data 0x4a1480 */
extern long g_4a151c; /* @data 0x4a151c */
extern short g_4a1570; /* @data 0x4a1570: button 2 is drawn lit */
extern short g_4a1572; /* @data 0x4a1572: button 1 is drawn */
extern short g_4aba88; /* @data 0x4aba88 */
extern short g_4aba8a; /* @data 0x4aba8a: the level */
extern Point g_4aba8c; /* @data 0x4aba8c */
extern short g_4aba90; /* @data 0x4aba90 */
extern Point g_4aba92; /* @data 0x4aba92 */
extern Point *g_4aba98; /* @data 0x4aba98 */
extern Point g_4aba9c; /* @data 0x4aba9c */
extern short g_4abaa0; /* @data 0x4abaa0 */
extern short g_4abaa2; /* @data 0x4abaa2 */
extern short g_4abaa4; /* @data 0x4abaa4 */
extern short g_4abaa6; /* @data 0x4abaa6 */
extern long g_4abaa8; /* @data 0x4abaa8: Ferry.MHK */
extern short g_4abaac; /* @data 0x4abaac: the scene is open */
extern short g_4abaae; /* @data 0x4abaae: button 2 is live */
extern short g_4abab0; /* @data 0x4abab0 */
extern short g_4abab4; /* @data 0x4abab4: Captain Cajun's view */
extern short g_4abab6; /* @data 0x4abab6 */
extern short g_4abab8; /* @data 0x4abab8 */
extern short g_4ababa; /* @data 0x4ababa */
extern short g_4ababc; /* @data 0x4ababc */
extern short g_4ababe; /* @data 0x4ababe */
extern short g_4abac0; /* @data 0x4abac0 */
extern short g_4abac2; /* @data 0x4abac2 */
extern short g_4abac6[20]; /* @data 0x4abac6: the placed views */
extern short g_4abaee; /* @data 0x4abaee */
extern short g_4abaf0; /* @data 0x4abaf0 */
extern short g_4abaf2; /* @data 0x4abaf2 */
extern short g_4abaf4; /* @data 0x4abaf4 */
extern char (*ferryLinks)[8]; /* @data 0x4abaf8: for each placed view, those it touches (from 1; linkFerryPlaces) */
extern short g_4abb04; /* @data 0x4abb04 */
extern short g_4abb06; /* @data 0x4abb06 */
extern short g_4abb08; /* @data 0x4abb08 */
extern short g_4abb0a; /* @data 0x4abb0a */
extern short g_4abb0c; /* @data 0x4abb0c */
extern short g_4abb10; /* @data 0x4abb10 */
extern short g_4abb16; /* @data 0x4abb16: Captain Cajun's script */
extern Point ferryPlaces[20]; /* @data 0x4a1520: where the Zoombinis wait */

void resetScene10();
void linkFerryPlaces(short draw);
void findFerryPlace(short *spot);
void fn_420f85(short n);
void fn_420a60(View *view, short event);
void drawFerryButton(short which, short lit, short show);
void fn_41fea4(View *, short region);
void closeScene10();
void fn_4209b8();
void fn_420a08(short group, short script, ViewNotify notify, char unknownF8);
void fn_420c82(View *view, short event);
void fn_42113f(View *, short dx);

#endif
