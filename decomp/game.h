/*
 * game's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef GAME_H
#define GAME_H

extern short aboveWindows311; /* @data 0x4a494a */
extern short cursorAnimation[12]; /* @data 0x4a4976 */
extern char msgRequiresQuickTime[]; /* @data 0x4a4dc7 */
extern char msgInitOs[]; /* @data 0x4a4e28 */
extern char msgInitTimer[]; /* @data 0x4a4e40 */
extern char msgInitHeap[]; /* @data 0x4a4e5b */
extern char msgNotEnoughMemory[]; /* @data 0x4a4e75 */
extern char msgNotEnoughPhysicalMemory[]; /* @data 0x4a4e8c */
extern char msgInitFileManager[]; /* @data 0x4a4eac */
extern char msgInitResourceManager[]; /* @data 0x4a4ece */
extern char msgInitConfiguration[]; /* @data 0x4a4ef4 */
extern char msgInitSound[]; /* @data 0x4a4f24 */
extern char msgNoWaveDevices[]; /* @data 0x4a4f3f */
extern char msgNoMidiDevices[]; /* @data 0x4a4f5e */
extern Font *fonts[3]; /* @data 0x4b28c8 */
extern unsigned short instanceAtom; /* @data 0x4b2ae0 */
extern short quickTimeReady; /* @data 0x4b2ae8 */
extern short startedWithoutModifier; /* @data 0x4b2aec */
extern short cursorFrame; /* @data 0x4b2aee */
extern char *appCommandLine; /* @data 0x4b2af8 */
extern long cursors[6]; /* @data 0x4b80ac */
void quitSilently();
extern short shuttingDown; /* @data 0x4a494c: shutting down (shutDownGame) */
extern short saveBeforeQuitting; /* @data 0x4a494e: save before quitting */
void shutDownGame();
void drawMemoryStats(short clear);
extern unsigned long leastFreeMemory; /* @data 0x4a48e0: the least free memory seen */
extern ShortRect memoryStatsRect; /* @data 0x4a498e: where the memory statistics go */
void __cdecl shutDownAtExit();
void gameFrame();
short noteOutOfMemory(unsigned long size, short error);
extern short slotPairViews[]; /* @data 0x4b26a6 */
extern short comparedViews[2]; /* @data 0x4b26ac */
extern short rowViews[9]; /* @data 0x4b26ba */
void startSmokeSnoidScript(short id, short script, short group, ViewNotify notify, char unknownF8);
void recordSlotFeatures(short id, short n);
void deleteTempFile();
void emptySlotView(short n);
void emptyPairViews();
extern short smokeOpen; /* @data 0x4b2790: the scene is open */
extern short smokeGoReady; /* @data 0x4b2792 */
extern short g_4a483e;
extern short g_4a4840;
extern SceneButton smokeButtons[3]; /* @data 0x4a4708 */
extern long smokeButtonResource; /* @data 0x4a47c8 */
extern long smokeImagesResource; /* @data 0x4b2638 */
extern long smokeHotSpotsXResource; /* @data 0x4b2650 */
extern long smokeHotSpotsYResource; /* @data 0x4b2654 */
extern long smokeFile; /* @data 0x4b278c */
extern ImageBank *smokeImages; /* @data 0x4b2634 */
void updateSmokeButtons(View *, short region);
void closeSmoke();
void smokeFrame();
extern short inSmokeFrame; /* @data 0x4a4842: in smokeFrame */
extern ShortRect g_4a4750; /* @data 0x4a4750 */
extern short g_4b2728; /* @data 0x4b2728 */
extern short g_4b279a; /* @data 0x4b279a: the group the leaders move in */
void drawSmokeSnoid(View *view);
short sceneToReturnTo();
extern PlacedSnoid placedSnoids[]; /* @data 0x4b2544 */
extern basePort *portBeforeMovie; /* @data 0x4b2ae4 */
void standFilledCells();
void standPlacedSnoids();
long loadMovie(const char *path);
void stopMovie(short shutdown);
extern short smokeLevel; /* @data 0x4b2630 */
extern short slotViews[7]; /* @data 0x4b2776 */
void copyToSlotView(short id, short n);
void recordLeftSlots();
void recordRightSlots();
extern short nextCrossing; /* @data 0x4b2734 */
extern short crossingCount; /* @data 0x4b262e */
extern short crossingViews[21]; /* @data 0x4b2604 */
extern char targetFeatures[8]; /* @data 0x4b263c */
extern short g_4b25ac;
extern short g_4b25ae;
extern short g_4b26b2;
extern Point g_4a44ac;
short takeNextTwoFeatures();
void startNextCrossing(short);
short shareFeature(short first, short second);
void startGrid(short cell);
short takeRandomFeatures();
extern ShortRect zoneMessageRect; /* @data 0x4a447a */
void showZoneMessage();
extern short g_4b2788;
extern short g_4b2742;
extern short g_4b25a4;
extern short g_4b25a6;
extern short g_4b2724[3];
extern short g_4b258c;
extern short g_4b258e;
extern short g_4b2714[4];
extern short g_4b273c;
short applySlotFeatures();
void startNextMove();
extern short randomViews[8]; /* @data 0x4b2672 */
extern Point randomPlaces[8]; /* @data 0x4a44cc */
void dealRandomFeatures(short count);
void advanceLeftFeatures();
void advanceRightFeatures();
extern ShortRect g_4a48b2;
extern short cheatMode; /* @data 0x4b2798: cheating */
short smokeKey(unsigned short key);
short placeAlike(short cell, volatile short direction);
extern short g_4b2740;
extern short g_4b2590;
extern short g_4b2592;
void drawSmokeButtons(View *);
short idleMovie();
void setPairFeatures();
void fillFreeCells();
void clearLine(short first, short second, short middle);
short playMovie(const char *path);
void updateCellLinks();
void settleCells();
extern short g_4b26b0;
extern short g_4b2704;
extern Point g_4a4528;
extern ShortRect g_4a4534;
short dragSnoidToSpot(View *view, Point where);
void placePartyOnGrid();
void markSharedFeature(short neighbour, short index, short cell);
extern short *smokeHotSpotsX; /* @data 0x4b2658: the scene's Zoombini images' hot spots: x */
extern short *smokeHotSpotsY; /* @data 0x4b265c: and y */
void layOutSmokeSnoid(Snoid *snoid);
extern short g_4b27ac[5];
extern short g_4b27b6[6];
extern short g_4b27c2;
extern short g_4b27c4;
extern short g_4b27c6;
extern short g_4b27c8;
extern short g_4b279c[4];
extern short g_4b27a4[4];
extern Point dealtPlaces[4]; /* @data 0x4a4514 */
void giveSlotFeatures(Snoid *snoid, short n);
extern short dealtViews[4]; /* @data 0x4b269a */
void dealFeatures();
void updateSmokeSnoid(View *view, short region);
short addSmokeSnoidView(Snoid *snoid);
extern short g_4b2644;
extern short g_4b274a;
extern short g_4b271c[2];
extern Point g_4a46a4[];
extern Point g_4a46e8[];
extern Point g_4a4634[];
extern Point g_4a466c[];
extern Point g_4a44b0;
extern short g_4b272a;
extern short g_4b272c;
extern short g_4b272e;
extern short g_4b2730;
extern short g_4b266e;
extern short g_4b2744;
extern short g_4b2594;
extern short g_4a483c;
extern short g_4b273a;
void stepBackNotify(View *, short event);
extern short g_4b28a2[4];
extern short g_4b28aa[4];
extern short g_4b28b2[4];
extern short g_4b28ba[4];
void makeSmokeRows(Snoid *snoid, short n);
extern short rowPlaceOrder[8]; /* @data 0x4b2768: where each of views 1-6 stands (rowPlaces) */
extern short g_4b2662;
extern Point rowPlaces[8]; /* @data 0x4a44f0 */
extern Point g_4a4530;
void setOutSmokeSnoids();
extern Point g_4a4524[2];
extern Point g_4a462c[2];
extern short g_4b2660;
extern short g_4b2664;
extern short g_4b2666;
extern short g_4b2668;
void addSmokeSnoids(short kind, short count);
extern short g_4b2748;
extern short g_4b2732;
void addLevelSnoids(short level);
short startRound();
extern short g_4b264c;
extern short g_4b2670;
extern short g_4b2738;
extern short g_4b274c;
extern short g_4b2750;
extern short g_4b2752;
extern short g_4b274e;
extern long g_4b2758;
extern unsigned long g_4b275c; /* @data 0x4b275c: slots used (allocateSlot) */
extern short g_4b2760;
extern short g_4b2762;
extern long g_4b2764;
extern long g_4b2794;
extern long g_4a47cc;
extern short g_4b258a;
extern short g_4b25b4[20];
extern short g_4b25dc[20];
extern short g_4b273e;
extern short g_4b2598;
extern Point g_4a44b4;
extern GroupList g_4a47a8;
void openSmoke();
extern ShortRect dealButtonRect; /* @data 0x4a4614 */
extern ShortRect g_4a47d0;
extern short g_4a47e0[5];
void smokeClicked(short action);
void drawSmokeButton(short which, short lit, short show);
short anyLeftToPlace();
short growGrid(short cell);
void resetCell(short cell);
void cutLink(short cell, short direction, short bit);
void checkAllFilled();
void unlinkCell(short cell);
void clearFeatureSlot(short n);
void clearFeatureSlots();
extern Point g_4a4846;
extern Point g_4a47ec[20];
extern short dealButtonView; /* @data 0x4b25b0 */
extern short leftRowView; /* @data 0x4b25a8 */
extern short rightRowView; /* @data 0x4b25aa */
extern short view11076; /* @data 0x4b2588 */
extern short leftRow; /* @data 0x4b26b4 */
extern short rightRow; /* @data 0x4b26b6 */
void smokeViewNotify(View *view, short event);
void pickFreeSpot(short *result);
void lightDealButton(short script);
void pressDealButton(short script, short running);
void dimDealButton(short script);
void startRowViews();
void stopRowViews();
void startView11076();
void stopView11076();

void layOutGrid();
extern short g_4a4238[26]; /* @data 0x4a4238: level 3's open cells */
extern short g_4a426c[43]; /* @data 0x4a426c: level 3's blocked cells */
extern short g_4a42c2[20]; /* @data 0x4a42c2 */
extern short g_4a42ea[20]; /* @data 0x4a42ea */
extern short g_4a4374[14]; /* @data 0x4a4374: levels 0-1: the first row's cell, by rows */
extern short g_4a4390[14]; /* @data 0x4a4390: and the cells between rows */
extern short g_4a43ac[18]; /* @data 0x4a43ac: level 2's blocked cells */
extern short g_4a43d0[18]; /* @data 0x4a43d0: its open cells */
extern short g_4a43f4[3]; /* @data 0x4a43f4: its start cells */
extern short g_4a43fa[3]; /* @data 0x4a43fa */
extern short g_4a4400[12]; /* @data 0x4a4400 */

#endif
