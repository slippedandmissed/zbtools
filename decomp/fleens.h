/*
 * fleens's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FLEENS_H
#define FLEENS_H

extern SceneButton fleensButtons[2]; /* @data 0x4a15b4 */
extern short g_4a16cc; /* @data 0x4a16cc: button 2 is drawn lit */
extern short g_4a16ce; /* @data 0x4a16ce: button 1 is drawn */
extern short g_4abb30; /* @data 0x4abb30 */
extern short g_4abb32; /* @data 0x4abb32 */
extern short g_4abb6a; /* @data 0x4abb6a */
extern short g_4abb7a; /* @data 0x4abb7a */
extern short g_4abb7c; /* @data 0x4abb7c */
extern ImageBank *fleenImages; /* @data 0x4abb94: the fleens' images */
extern long fleenScriptResources[59]; /* @data 0x4abbd4 */
extern short *fleenScripts[59]; /* @data 0x4abcc0: the 'SCRS' scripts 4000-4058, as loaded */

extern ImageBank *g_4a1650; /* @data 0x4a1650: the buttons' images */
extern short g_4abb18; /* @data 0x4abb18: the facing for after the next turn (from 1) */
extern short g_4abb1e; /* @data 0x4abb1e */
extern short g_4abb2e; /* @data 0x4abb2e */
extern short g_4abb3a; /* @data 0x4abb3a */
extern short g_4abb3c; /* @data 0x4abb3c */
extern long g_4abb74; /* @data 0x4abb74: Fleens.MHK */
extern short g_4abb78; /* @data 0x4abb78: the scene is open */
extern long g_4abb84; /* @data 0x4abb84 */
extern long g_4abb88; /* @data 0x4abb88 */
extern long g_4abb8c; /* @data 0x4abb8c */
extern long g_4abb90; /* @data 0x4abb90 */

extern short g_4abb1a; /* @data 0x4abb1a */
extern short g_4abb3e; /* @data 0x4abb3e */
extern short g_4abb40; /* @data 0x4abb40 */
extern short g_4abb42; /* @data 0x4abb42 */
extern short g_4abb44; /* @data 0x4abb44 */
extern short g_4abb46; /* @data 0x4abb46 */
extern short g_4abb48; /* @data 0x4abb48 */
extern short g_4abb4a[7]; /* @data 0x4abb4a */
extern short g_4abb58[7]; /* @data 0x4abb58 */
extern short g_4abb66; /* @data 0x4abb66 */
extern short g_4abb68; /* @data 0x4abb68 */
extern short g_4abb6c; /* @data 0x4abb6c */
extern short g_4abb7e; /* @data 0x4abb7e */
extern short g_4abb80; /* @data 0x4abb80 */
extern short g_4abba2[16]; /* @data 0x4abba2 */
extern char g_4abbc2[16]; /* @data 0x4abbc2 */
extern short g_4abdac; /* @data 0x4abdac */
extern unsigned long g_4abdb0; /* @data 0x4abdb0 */
extern unsigned long g_4abdb4; /* @data 0x4abdb4 */
extern unsigned long g_4abdb8; /* @data 0x4abdb8 */

void fn_42403b(View *view, short event);
extern short fleensViews[7]; /* @data 0x4abb20: the backdrop's views (scripts 1200-1206) */

extern short feetLayers[6]; /* @data 0x4a1654: the feature layers by value, for layOutFleen */
extern short noseLayers[6]; /* @data 0x4a1660 */
extern short eyesLayers[6]; /* @data 0x4a166c */
extern short hairLayers[6]; /* @data 0x4a1678 */
extern short *fleenHotX; /* @data 0x4abb98: the fleens' images' hot spots */
extern short *fleenHotY; /* @data 0x4abb9c */

extern short g_4a16d2[4]; /* @data 0x4a16d2: the features to swap in (fleen rules 5-7) */
extern short pickedFleens[3]; /* @data 0x4abb34: the fleens picked to stand apart (from 1) */
extern short g_4abdbc; /* @data 0x4abdbc: travellers aboard */

extern short g_4abb6e; /* @data 0x4abb6e: the Zoombini last put down at a place */
extern short g_4abba0; /* @data 0x4abba0: the Zoombinis' views (fleens are made from as many travellers) */

extern short g_4abb1c; /* @data 0x4abb1c */

extern GroupList fleensGroups[1]; /* @data 0x4a1630 */

extern short g_4a16d0; /* @data 0x4a16d0: scene13Frame is running */
extern short g_4abb70; /* @data 0x4abb70: the fleen of the Zoombini put down (g_4abb6e) */

void resetScene13();
void scene13Frame();
void openScene13();
void fn_42365a(View *view, short event);
void fn_424195(View *view, short event);
void scene13Clicked(short which);
void addFleens();
void updateFleen(View *view, short region);
short addFleen(Snoid *snoid);
void fn_423ebb(View *view, short event);
void drawFleensButtons(View *);
void startFleenScript(View *view, short id, Point *anchor);
void fn_423f84();
short layOutFleen(Snoid *snoid, short *event);
void fn_423512(View *view, short event);
short fleenScript(View *view, short which);
void drawFleensButton(short which, short lit, short show);
void closeScene13();
short fleensSnoidScript(View *view, short which);
void fn_423d9d(View *view, short event);
void fn_423e2c(View *view, short event);
void fn_424104(View *view, short event);
void fn_421bfc(View *view, short region);
short scene13Key(unsigned short key);
void drawFleen(View *view);
void loadFleenScripts();
void loadFleenScript(short id);
void fn_4234c9(View *, short event);

#endif
