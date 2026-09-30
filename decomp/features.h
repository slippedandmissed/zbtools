/*
 * features's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FEATURES_H
#define FEATURES_H

/* features */
void loadFeatureGroup(short id, short group, short hotspots);
void freeFeatureGroups();
void drawCels(View *view);
void drawCelsOpaque(View *view);
void queueViewSound(short sound, char streamed);
void removeFirstCel(ViewCel *cels);
void tickView(View *view, short);
void runViewScript(View *view, short region);
void runViewCels(View *view, short region);
void strandParty();
void replayHint();
void loadDialogs();
void freeDialogs();
void askKeepParty();
void dialogClick(Point where);
void dialogKey(unsigned short key); /* 0x4682f9 */
void startNewGame();
void askNewGame();
void askLoadGame();
void askSaveGame();
void askQuit();
void drawDialogPart(View *view); /* 0x467745 */
void updateDialogPart(View *view, short region); /* 0x468033 */
void placeDialogButton(View *view); /* 0x4688a5 */
void placeDialogList(View *view); /* 0x468bde */
void drawCredits(View *view); /* 0x467227 */
void closeDialog(short kind); /* 0x4674cf */
void showDialog(short kind, const char *text, const char *button2, const char *button1); /* 0x466d7e */
extern long dialogResource; /* @data 0x4b9798 */
extern ImageBank *dialogImages; /* @data 0x4b979c */
extern long dialogScriptResources[11]; /* @data 0x4b97a0 */
extern short *dialogScripts[11]; /* @data 0x4b97cc */
extern short dialogView; /* @data 0x4b97fe */
extern short dialogButton1; /* @data 0x4b9800 */
extern short dialogButton2; /* @data 0x4b9802 */
extern short gamesDialogView; /* @data 0x4b9806 */
extern short gamesDialogPart2; /* @data 0x4b9808 */
extern short gamesDialogButtons; /* @data 0x4b980a */
extern short confirmDialogView; /* @data 0x4b980c */
extern short confirmDialogButtons; /* @data 0x4b980e */
extern SavedGameList *savedGameList; /* @data 0x4a7d4c */
extern const char *dialogTexts[]; /* @data 0x4a53fc */
extern short newGameAsked; /* @data 0x4b80e2 */
extern Point dialogWhere; /* @data 0x4b97f8 */
extern ShortRect dialogSpots[17]; /* @data 0x4b982a */
extern short thirdButtonPresses; /* @data 0x4b98d2 */
extern ShortRect saveField; /* @data 0x4a7d44: where the save dialog's name is typed */
extern MapSave *saveFieldSave; /* @data 0x4a7d50: what's under it */
extern ShortRect optionsTitleRect; /* @data 0x4a7d74 */
extern ShortRect loadTitleRect; /* @data 0x4a7d7c */
extern ShortRect saveTitleRect; /* @data 0x4a7d84 */
extern ShortRect saveAsRect; /* @data 0x4a7d8c */
extern ShortRect togglesRect; /* @data 0x4a7d94 */
extern ShortRect messageTitleRect; /* @data 0x4a7d9c */
extern ShortRect onRect; /* @data 0x4a7da4 */
extern ShortRect offRect; /* @data 0x4a7dac */
extern ShortRect menuItemRect; /* @data 0x4a7db4 */
extern short menuItemTops[8]; /* @data 0x4a7dbc */
extern short caretBlink; /* @data 0x4b9828 */
extern char buttonPressed[17]; /* @data 0x4b98b2: dialog hot spots shown pressed */
extern char confirmText[]; /* @data 0x4b9698 */
extern short loadCancelAlt; /* @data 0x4a7d3e */
extern short firstGameShown; /* @data 0x4b9664 */
extern short selectedGame; /* @data 0x4b9666 */
extern long lastGameClickTime; /* @data 0x4b9668 */
extern ImageBank *creditsImages; /* @data 0x4b9678 */
extern short *creditsBackdrop; /* @data 0x4b967c: the images, as cels (after a count) */
extern short creditsShowing; /* @data 0x4b9680 */
extern short creditHeading; /* @data 0x4b9682 */
extern short creditTick; /* @data 0x4b98ce */
extern short creditLine; /* @data 0x4b98d0 */
extern ShortRect creditsLineRect; /* @data 0x4a7d54 */
extern ShortRect creditsScrollFrom; /* @data 0x4a7d5c */
extern ShortRect creditsScrollTo; /* @data 0x4a7d64 */
extern ShortRect creditsClip; /* @data 0x4a7d6c */
extern const char *dialogText; /* @data 0x4b968c */
extern const char *dialogButton2Text; /* @data 0x4b9690 */
extern const char *dialogButton1Text; /* @data 0x4b9694 */
extern long lastCaretBlink; /* @data 0x4b98c4 */
extern short askingReplace; /* @data 0x4b98c8 */
extern short tooManyGames; /* @data 0x4b98ca */
extern char saveName[]; /* @data 0x4b9810 */
extern unsigned short saveNameLength; /* @data 0x4b9826 */
extern long groupBankResources[8]; /* @data 0x4b95a4 */
extern long groupHotXResources[8]; /* @data 0x4b95e4 */
extern long groupHotYResources[8]; /* @data 0x4b9604 */
extern short *groupHotX[8]; /* @data 0x4b9624 */
extern short *groupHotY[8]; /* @data 0x4b9644 */

#endif
