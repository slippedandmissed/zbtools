/*
 * roster's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ROSTER_H
#define ROSTER_H

void fn_41f195(const char *message);
void fn_41f668();
void newSaveFileName(const char *, char *file, short *nextId);
void readRoster();
void saveRoster();
int cheerNotify(View *, short value);
void cavesNoDraw(View *);
void cavesNoUpdate(View *, short);
extern long glyphShape;
extern ShortRect g_4a11ac;
extern Point g_4ab8e0;
void freeGlyphShape();
void loadCaveResources();
void openCaves();
void updateGlyphArea(View *view, short region);
void claimSpot(short id, short n);
extern char *rosterError; /* @data 0x4aba80 */
extern Point *g_4ab8e4;
extern short frameView;
extern short g_4aba78[2];
extern long g_4aba70[2];
void reportRosterError(const char *message);
void startWalkerScript(short group, short script, ViewNotify notify, char f8);
void showFrame(volatile short n);
void frameNotify(View *view, short event);
void freeCaveResources();
extern short g_4a0fe8;
extern short g_4a120a;
extern short g_4a120c;
extern long g_4aba7c; /* the roster file */
void updateCavesButtons(View *, short region);
void applyPlayerSettings();
short openRosterFile(const char *path, short mode);
extern short firstFrame; /* the first of the frames showFrame shows */
extern short finalFrame; /* their number */
extern short currentFrame; /* the frame shown */
extern SceneButton cavesButtons[2]; /* @data 0x4a1024: buttons 1 and 2 */
extern ImageBank *g_4a1020;
extern short g_4a1282[6];
extern short g_4a128e[6];
extern short g_4a129a[6];
extern short g_4a12a6[6];
extern short spotSnoids[21];
void drawCavesButton(short which, short lit, short show);
void drawFeatureImage(short kind, short n, ShortRect rect);
void sendReadyOff(short x, short y, long interval);
extern short cavesOpen; /* the roster screen is open */
extern long g_4a0fd0;
extern long g_4ab83c;
extern short cavesLevel;
extern short g_4a0ff4;
extern short caveFeatures[2]; /* @data 0x4ab87a: the features (0-3) the roster asks about */
extern short chosenCount;
void drawCavesButtons(View *);
void closeCaves();
void countByCaveFeatures();
extern short g_4ab872;
extern short *g_4aba68; /* @data 0x4aba68: REGS 200 */
extern short *g_4aba6c; /* @data 0x4aba6c: REGS 201 */
void drawGlyph(short which, short image, long);
extern short g_4ab870;
extern short g_4aba64;
extern short g_4ab86c;
extern short g_4ab86e;
extern short g_4a100e;
extern short g_4a1010;
extern short g_4a1012;
extern short g_4a0ff2;
extern short g_4a101a;
extern short g_4a0ffa;
extern short g_4a0ff8;
extern short g_4a0fe4;
void readWriteRoster(void *data, short read);
void resetCavesState(short which);
extern short g_4a0ffe; /* @data 0x4a0ffe */
extern short g_4a1006; /* @data 0x4a1006 */
extern short g_4ab994; /* @data 0x4ab994 */
extern Point g_4aba14[20]; /* @data 0x4aba14: a stack of points, g_4aba64 of them */
void walkerNotify(View *view, short event);
extern short g_4a1018; /* @data 0x4a1018 */
void pickCaveFeatures();
extern short glyphPlaced[11]; /* @data 0x4ab840: an image is placed at place 1-10 */
extern short glyphImages[11]; /* @data 0x4ab856: the image placed there */
void redrawGlyphs(long unused);
void drawGlyphs(View *);
void resetCavesScreen();
extern short firstCave; /* @data 0x4a100c */
void layOutCaves();
short pickCave(short id, short n);
extern unsigned long g_4a78cc; /* @data 0x4a78cc */
void fillRosterHeader(short reset);
void readWriteSavedGames(SavedGameList *list, short mode);
extern short g_4ab9c2; /* @data 0x4ab9c2: the roster's next Zoombini's view */
extern short g_4ab9c4[26]; /* @data 0x4ab9c4: views, by frame */
extern short g_4ab8da; /* @data 0x4ab8da */
extern short g_4ab8dc; /* @data 0x4ab8dc */
extern short g_4a0ffc; /* @data 0x4a0ffc: the first of the walking scripts */
void walkNext(short which);
void placeGlyphs(short kind);
void drawFeatureTable();
extern Point chosenSpots[20]; /* @data 0x4a115c: the chosen Zoombinis' spots on the roster screen */
extern short g_4a101c; /* @data 0x4a101c */
extern short g_4a1016; /* @data 0x4a1016 */
extern short g_4a0fea; /* @data 0x4a0fea */
extern short g_4ab96a[21]; /* @data 0x4ab96a: views, by place */
void setUpCaves();
void walkToSpots();
void sendToCaves();
void changeCaveFeature(short feature);
extern Point g_4a10a8[21]; /* @data 0x4a10a8 */
extern ShortRect g_4a10fc[12]; /* @data 0x4a10fc: the places' areas */
extern short g_4a11b4[21]; /* @data 0x4a11b4 */
extern short g_4a11de[21]; /* @data 0x4a11de */
extern short g_4ab996[20]; /* @data 0x4ab996: g_4ab9be of them */
extern short g_4ab9be; /* @data 0x4ab9be */
extern short g_4a1008; /* @data 0x4a1008 */
extern short g_4a100a; /* @data 0x4a100a */
extern long g_4aba0c; /* @data 0x4aba0c */
extern short g_4ab9f0; /* @data 0x4ab9f0 */
extern short g_4ab9f2; /* @data 0x4ab9f2 */
extern short g_4ab9f4; /* @data 0x4ab9f4 */
extern short g_4a0fe6; /* @data 0x4a0fe6 */
extern GroupList caveGroups[1]; /* @data 0x4a10a0 */
extern short g_4aba08; /* @data 0x4aba08 */
extern short g_4ab876; /* @data 0x4ab876 */
extern Point *g_4ab8e8; /* @data 0x4ab8e8 */
void cavesClicked(short which);
extern short g_4a1208; /* @data 0x4a1208: cavesFrame is running */
extern unsigned long g_4ab9fc; /* @data 0x4ab9fc: when a Zoombini last cheered */
extern unsigned long g_4aba00; /* @data 0x4aba00: slots used (allocateSlot) */
extern short g_4aba04; /* @data 0x4aba04: how many cheer */
extern short g_4aba06; /* @data 0x4aba06: how many have */
void cavesFrame();
short cavesKey(unsigned short key);

#endif
