/*
 * ferry's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FERRY_H
#define FERRY_H

extern SceneButton ferryButtons[2]; /* @data 0x4a15b4 */
extern short g_4a16cc; /* @data 0x4a16cc: button 2 is drawn lit */
extern short g_4a16ce; /* @data 0x4a16ce: button 1 is drawn */
extern short g_4abb30; /* @data 0x4abb30 */
extern short g_4abb32; /* @data 0x4abb32 */
extern short g_4abb6a; /* @data 0x4abb6a */
extern short g_4abb7a; /* @data 0x4abb7a */
extern short g_4abb7c; /* @data 0x4abb7c */
extern ImageBank *g_4abb94; /* @data 0x4abb94 */
extern long ferryScriptResources[59]; /* @data 0x4abbd4 */
extern short *ferryScripts[59]; /* @data 0x4abcc0: the 'SCRS' scripts 4000-4058, as loaded */

extern ImageBank *g_4a1650; /* @data 0x4a1650: the buttons' images */
extern short g_4abb18; /* @data 0x4abb18: the facing for after the next turn (from 1) */
extern short g_4abb1e; /* @data 0x4abb1e */
extern short g_4abb2e; /* @data 0x4abb2e */
extern short g_4abb3a; /* @data 0x4abb3a */
extern short g_4abb3c; /* @data 0x4abb3c */
extern long g_4abb74; /* @data 0x4abb74: Ferry.MHK */
extern short g_4abb78; /* @data 0x4abb78: the scene is open */
extern long g_4abb84; /* @data 0x4abb84 */
extern long g_4abb88; /* @data 0x4abb88 */
extern long g_4abb8c; /* @data 0x4abb8c */
extern long g_4abb90; /* @data 0x4abb90 */

void drawFerryButton(short which, short lit, short show);
void closeScene13();
short ferrySnoidScript(View *view, short which);
void fn_423d9d(View *view, short event);
void fn_423e2c(View *view, short event);
void fn_424104(View *view, short event);
void fn_421bfc(View *view, short region);
short scene13Key(unsigned short key);
void fn_4224ea(View *view);
void loadFerryScripts();
void loadFerryScript(short id);
void fn_4234c9(View *, short event);

#endif
