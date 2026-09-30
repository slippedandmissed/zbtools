/*
 * fleens's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FLEENS_H
#define FLEENS_H

extern SceneButton fleensButtons[2]; /* @data 0x4a15b4 */
extern short fleensButton2Lit; /* @data 0x4a16cc: button 2 is drawn lit */
extern short fleensButton1Drawn; /* @data 0x4a16ce: button 1 is drawn */
extern short activeFleen; /* @data 0x4abb30 */
extern short activeSnoid; /* @data 0x4abb32 */
extern short fleensLevel; /* @data 0x4abb6a */
extern short fleensGoReady; /* @data 0x4abb7a */
extern short fleensEntered; /* @data 0x4abb7c */
extern ImageBank *fleenImages; /* @data 0x4abb94: the fleens' images */
extern long fleenScriptResources[59]; /* @data 0x4abbd4 */
extern short *fleenScripts[59]; /* @data 0x4abcc0: the 'SCRS' scripts 4000-4058, as loaded */

extern ImageBank *fleensButtonImages; /* @data 0x4a1650: the buttons' images */
extern short pendingFleensFacing; /* @data 0x4abb18: the facing for after the next turn (from 1) */
extern short pickedFleensFound; /* @data 0x4abb1e */
extern short fleensView0Started; /* @data 0x4abb2e */
extern short walkerStep9Due; /* @data 0x4abb3a */
extern short walkerStep3Due; /* @data 0x4abb3c */
extern long fleensFile; /* @data 0x4abb74: Fleens.MHK */
extern short fleensOpen; /* @data 0x4abb78: the scene is open */
extern long fleensButtonResource; /* @data 0x4abb84 */
extern long fleenImagesResource; /* @data 0x4abb88 */
extern long fleenHotXResource; /* @data 0x4abb8c */
extern long fleenHotYResource; /* @data 0x4abb90 */

extern short fleensFidgetsAllowed; /* @data 0x4abb1a */
extern short walkerStep4Due; /* @data 0x4abb3e */
extern short lineMoveDue; /* @data 0x4abb40 */
extern short fleenBehindDue; /* @data 0x4abb42 */
extern short snoidBehindDue; /* @data 0x4abb44 */
extern short lineLength; /* @data 0x4abb46 */
extern short fleensFidgets; /* @data 0x4abb48 */
extern short lineSnoids[7]; /* @data 0x4abb4a */
extern short lineFleens[7]; /* @data 0x4abb58 */
extern short walkerSnoid; /* @data 0x4abb66 */
extern short walkerFleen; /* @data 0x4abb68 */
extern short lineStepDue; /* @data 0x4abb6c */
extern short leaderBusy; /* @data 0x4abb7e */
extern short leaderWalking; /* @data 0x4abb80 */
extern short fleenViews[16]; /* @data 0x4abba2 */
extern char fleenClicked[16]; /* @data 0x4abbc2 */
extern short fleenScriptsToLoad; /* @data 0x4abdac */
extern unsigned long lastFleensFidgetTime; /* @data 0x4abdb0 */
extern unsigned long fleensFidgetInterval; /* @data 0x4abdb4 */
extern unsigned long fleensFidgetersUsed; /* @data 0x4abdb8 */

void fleensMovingOnNotify(View *view, short event);
extern short fleensViews[7]; /* @data 0x4abb20: the backdrop's views (scripts 1200-1206) */

extern short feetLayers[6]; /* @data 0x4a1654: the feature layers by value, for layOutFleen */
extern short noseLayers[6]; /* @data 0x4a1660 */
extern short eyesLayers[6]; /* @data 0x4a166c */
extern short hairLayers[6]; /* @data 0x4a1678 */
extern short *fleenHotX; /* @data 0x4abb98: the fleens' images' hot spots */
extern short *fleenHotY; /* @data 0x4abb9c */

extern short swapFeatures[4]; /* @data 0x4a16d2: the features to swap in (fleen rules 5-7) */
extern short pickedFleens[3]; /* @data 0x4abb34: the fleens picked to stand apart (from 1) */
extern short fleensTravellers; /* @data 0x4abdbc: travellers aboard */

extern short putDownSnoid; /* @data 0x4abb6e: the Zoombini last put down at a place */
extern short fleensPartySize; /* @data 0x4abba0: the Zoombinis' views (fleens are made from as many travellers) */

extern short view1000; /* @data 0x4abb1c */

extern GroupList fleensGroups[1]; /* @data 0x4a1630 */

extern short inFleensFrame; /* @data 0x4a16d0: fleensFrame is running */
extern short putDownFleen; /* @data 0x4abb70: the fleen of the Zoombini put down (putDownSnoid) */

void resetFleens();
void fleensFrame();
void openFleens();
void fleensLeaderNotify(View *view, short event);
void fleensExtraNotify(View *view, short event);
void fleensClicked(short which);
void addFleens();
void updateFleen(View *view, short region);
short addFleen(Snoid *snoid);
void fleensWalkerNotifyE(View *view, short event);
void drawFleensButtons(View *);
void startFleenScript(View *view, short id, Point *anchor);
void moveFleenZoombinisOn();
short layOutFleen(Snoid *snoid, short *event);
void fleensViewNotify(View *view, short event);
short fleenScript(View *view, short which);
void drawFleensButton(short which, short lit, short show);
void closeFleens();
short fleensSnoidScript(View *view, short which);
void fleensWalkerNotifyC(View *view, short event);
void fleensWalkerNotifyA(View *view, short event);
void fleensWalkerStopNotify(View *view, short event);
void updateFleensButtons(View *view, short region);
short fleensKey(unsigned short key);
void drawFleen(View *view);
void loadFleenScripts();
void loadFleenScript(short id);
void fleensStartNotify(View *, short event);

#endif
