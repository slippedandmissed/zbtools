/*
 * roster's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ROSTER_H
#define ROSTER_H

void newSaveFileName(const char *, char *file, short *nextId);
void readRoster();
void saveRoster();
int cheerNotify(View *, short value);
void cavesNoDraw(View *);
void cavesNoUpdate(View *, short);
extern long glyphShape; /* @data 0x4a0fd4 */
extern ShortRect glyphArea; /* @data 0x4a11ac */
extern Point claimedSpotPoint; /* @data 0x4ab8e0 */
void freeGlyphShape();
void loadCaveResources();
void openCaves();
void updateGlyphArea(View *view, short region);
void claimSpot(short id, short n);
extern char *rosterError; /* @data 0x4aba80 */
extern Point *walkerAnchor; /* @data 0x4ab8e4 */
extern short frameView; /* @data 0x4ab9f8 */
extern short caveRegsHandles[2]; /* @data 0x4aba78 */
extern long caveRegsResources[2]; /* @data 0x4aba70 */
void reportRosterError(const char *message);
void startWalkerScript(short group, short script, ViewNotify notify, char f8);
void showFrame(volatile short n);
void frameNotify(View *view, short event);
void freeCaveResources();
extern short cavesGoReady; /* @data 0x4a0fe8 */
extern short cavesButton2Lit; /* @data 0x4a120a */
extern short cavesButton1Drawn; /* @data 0x4a120c */
extern long rosterFile; /* @data 0x4aba7c: the roster file */
void updateCavesButtons(View *, short region);
void applyPlayerSettings();
short openRosterFile(const char *path, short mode);
extern short firstFrame; /* @data 0x4a1000: the first of the frames showFrame shows */
extern short finalFrame; /* @data 0x4a1002: their number */
extern short currentFrame; /* @data 0x4a1004: the frame shown */
extern SceneButton cavesButtons[2]; /* @data 0x4a1024: buttons 1 and 2 */
extern ImageBank *cavesButtonImages; /* @data 0x4a1020 */
extern short kind4Images[6]; /* @data 0x4a1282 */
extern short kind3Images[6]; /* @data 0x4a128e */
extern short kind2Images[6]; /* @data 0x4a129a */
extern short kind1Images[6]; /* @data 0x4a12a6 */
extern short spotSnoids[21]; /* @data 0x4ab8ec */
void drawCavesButton(short which, short lit, short show);
void drawFeatureImage(short kind, short n, ShortRect rect);
void sendReadyOff(short x, short y, long interval);
extern short cavesOpen; /* @data 0x4a0fec: the roster screen is open */
extern long cavesButtonResource; /* @data 0x4a0fd0 */
extern long cavesFile; /* @data 0x4ab83c */
extern short cavesLevel; /* @data 0x4ab878 */
extern short caveFeatureCount; /* @data 0x4a0ff4 */
extern short caveFeatures[2]; /* @data 0x4ab87a: the features (0-3) the roster asks about */
extern short chosenCount; /* @data 0x4a1014 */
void drawCavesButtons(View *);
void closeCaves();
void countByCaveFeatures();
extern short exitStage; /* @data 0x4ab872 */
extern short *caveRegs200; /* @data 0x4aba68: REGS 200 */
extern short *glyphRaise; /* @data 0x4aba6c: REGS 201 */
void drawGlyph(short which, short image, long);
extern short exitDue; /* @data 0x4ab870 */
extern short walkBackCount; /* @data 0x4aba64 */
extern short blinkingGlyph; /* @data 0x4ab86c */
extern short lastBlinkedGlyph; /* @data 0x4ab86e */
extern short cavesPlacedCount; /* @data 0x4a100e */
extern short droppedCave; /* @data 0x4a1010 */
extern short assignedCave; /* @data 0x4a1012 */
extern short caveValueCount; /* @data 0x4a0ff2 */
extern short featureTableShown; /* @data 0x4a101a */
extern short walkerFrontView; /* @data 0x4a0ffa */
extern short unusedCaves1; /* @data 0x4a0ff8 */
extern short unusedCaves2; /* @data 0x4a0fe4 */
void readWriteRoster(void *data, short read);
void resetCavesState(short which);
extern short walkScript; /* @data 0x4a0ffe */
extern short walkOnDue; /* @data 0x4a1006 */
extern short framesChanged; /* @data 0x4ab994 */
extern Point walkBackPoints[20]; /* @data 0x4aba14: a stack of points, walkBackCount of them */
void walkerNotify(View *view, short event);
extern short forceHairFirst; /* @data 0x4a1018 */
void pickCaveFeatures();
extern short glyphPlaced[11]; /* @data 0x4ab840: an image is placed at place 1-10 */
extern short glyphImages[11]; /* @data 0x4ab856: the image placed there */
void redrawGlyphs(long unused);
void drawGlyphs(View *);
void resetCavesScreen();
extern short firstCave; /* @data 0x4a100c */
void layOutCaves();
short pickCave(short id, short n);
extern unsigned long tunnelRemarksUnused; /* @data 0x4a78cc */
void fillRosterHeader(short reset);
void readWriteSavedGames(SavedGameList *list, short mode);
extern short cavesNextWalker; /* @data 0x4ab9c2: the roster's next Zoombini's view */
extern short frameAnchorViews[26]; /* @data 0x4ab9c4: views, by frame */
extern short walkFromView; /* @data 0x4ab8da */
extern short walkToView; /* @data 0x4ab8dc */
extern short walkScriptsBase; /* @data 0x4a0ffc: the first of the walking scripts */
void walkNext(short which);
void placeGlyphs(short kind);
void drawFeatureTable();
extern Point chosenSpots[20]; /* @data 0x4a115c: the chosen Zoombinis' spots on the roster screen */
extern short glyphView; /* @data 0x4a101c */
extern short missingSnoids; /* @data 0x4a1016 */
extern short cavesFullParty; /* @data 0x4a0fea */
extern short caveViews[21]; /* @data 0x4ab96a: views, by place */
void setUpCaves();
void walkToSpots();
void sendToCaves();
void changeCaveFeature(short feature);
extern Point cavePoints[21]; /* @data 0x4a10a8 */
extern ShortRect caveWaitAreas[12]; /* @data 0x4a10fc: the places' areas */
extern short caveSnoidF1[21]; /* @data 0x4a11b4 */
extern short caveSnoidF2[21]; /* @data 0x4a11de */
extern short cheerQueue[20]; /* @data 0x4ab996: cheerQueueCount of them */
extern short cheerQueueCount; /* @data 0x4ab9be */
extern short unusedCaves3; /* @data 0x4a1008 */
extern short unusedCaves4; /* @data 0x4a100a */
extern long unusedCaves5; /* @data 0x4aba0c */
extern short cavesView6000; /* @data 0x4ab9f0 */
extern short cavesView6001; /* @data 0x4ab9f2 */
extern short cavesView6002; /* @data 0x4ab9f4 */
extern short cavesFullPartyAtOpen; /* @data 0x4a0fe6 */
extern GroupList caveGroups[1]; /* @data 0x4a10a0 */
extern short allPlaced; /* @data 0x4aba08 */
extern short walkDue; /* @data 0x4ab876 */
extern Point *cheerAnchor; /* @data 0x4ab8e8 */
void cavesClicked(short which);
extern short inCavesFrame; /* @data 0x4a1208: cavesFrame is running */
extern unsigned long lastCheerTime; /* @data 0x4ab9fc: when a Zoombini last cheered */
extern unsigned long cheerersUsed; /* @data 0x4aba00: slots used (allocateSlot) */
extern short cheersAllowed; /* @data 0x4aba04: how many cheer */
extern short cheersDone; /* @data 0x4aba06: how many have */
void cavesFrame();
short cavesKey(unsigned short key);

#endif
