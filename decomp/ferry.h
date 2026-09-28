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

void fn_421bfc(View *view, short region);
short scene13Key(unsigned short key);
void fn_4224ea(View *view);
void loadFerryScripts();
void loadFerryScript(short id);
void fn_4234c9(View *, short event);

#endif
