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
extern short smokeButton2Lit; /* @data 0x4a483e */
extern short smokeButton1Drawn; /* @data 0x4a4840 */
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
extern ShortRect smokeGoRect; /* @data 0x4a4750 */
extern short moverScript; /* @data 0x4b2728 */
extern short leaderGroup; /* @data 0x4b279a: the group the leaders move in */
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
extern short view11036; /* @data 0x4b25ac */
extern short view11008; /* @data 0x4b25ae */
extern short crossingSnoid; /* @data 0x4b26b2 */
extern Point crossingStart; /* @data 0x4a44ac */
short takeNextTwoFeatures();
void startNextCrossing(short);
short shareFeature(short first, short second);
void startGrid(short cell);
short takeRandomFeatures();
extern ShortRect zoneMessageRect; /* @data 0x4a447a */
void showZoneMessage();
extern short movePlace; /* @data 0x4b2788 */
extern short useSecondMover; /* @data 0x4b2742 */
extern short moverView1; /* @data 0x4b25a4 */
extern short moverView2; /* @data 0x4b25a6 */
extern short moverScripts[3]; /* @data 0x4b2724 */
extern short pairView1; /* @data 0x4b258c */
extern short pairView2; /* @data 0x4b258e */
extern short pairScripts[4]; /* @data 0x4b2714 */
extern short pairMismatch; /* @data 0x4b273c */
short applySlotFeatures();
void startNextMove();
extern short randomViews[8]; /* @data 0x4b2672 */
extern Point randomPlaces[8]; /* @data 0x4a44cc */
void dealRandomFeatures(short count);
void advanceLeftFeatures();
void advanceRightFeatures();
extern ShortRect smokeButtonsRect; /* @data 0x4a48b2 */
extern short cheatMode; /* @data 0x4b2798: cheating */
short smokeKey(unsigned short key);
short placeAlike(short cell, volatile short direction);
extern short slotsDiffer; /* @data 0x4b2740 */
extern short view11018; /* @data 0x4b2590 */
extern short view11019; /* @data 0x4b2592 */
void drawSmokeButtons(View *);
short idleMovie();
void setPairFeatures();
void fillFreeCells();
void clearLine(short first, short second, short middle);
short playMovie(const char *path);
void updateCellLinks();
void settleCells();
extern short randomDragSlot; /* @data 0x4b26b0 */
extern short unusedSpot4Block; /* @data 0x4b2704 */
extern Point spot4Point; /* @data 0x4a4528 */
extern ShortRect spot4Rect; /* @data 0x4a4534 */
short dragSnoidToSpot(View *view, Point where);
void placePartyOnGrid();
void markSharedFeature(short neighbour, short index, short cell);
extern short *smokeHotSpotsX; /* @data 0x4b2658: the scene's Zoombini images' hot spots: x */
extern short *smokeHotSpotsY; /* @data 0x4b265c: and y */
void layOutSmokeSnoid(Snoid *snoid);
extern short featureOrder[5]; /* @data 0x4b27ac */
extern short valueOrder[6]; /* @data 0x4b27b6 */
extern short featurePick; /* @data 0x4b27c2 */
extern short featurePickMax; /* @data 0x4b27c4 */
extern short valuePick; /* @data 0x4b27c6 */
extern short valuePickMax; /* @data 0x4b27c8 */
extern short leftChosenValues[4]; /* @data 0x4b279c */
extern short rightChosenValues[4]; /* @data 0x4b27a4 */
extern Point dealtPlaces[4]; /* @data 0x4a4514 */
void giveSlotFeatures(Snoid *snoid, short n);
extern short dealtViews[4]; /* @data 0x4b269a */
void dealFeatures();
void updateSmokeSnoid(View *view, short region);
short addSmokeSnoidView(Snoid *snoid);
extern short view11017Due; /* @data 0x4b2644 */
extern short dealDimDue; /* @data 0x4b274a */
extern short crossScripts[2]; /* @data 0x4b271c */
extern Point movePlaces1[]; /* @data 0x4a46a4 */
extern Point movePlaces2[]; /* @data 0x4a46e8 */
extern Point movePlaces3[]; /* @data 0x4a4634 */
extern Point movePlaces4[]; /* @data 0x4a466c */
extern Point crossingStart2; /* @data 0x4a44b0 */
extern short crossScript1; /* @data 0x4b272a */
extern short crossScript2; /* @data 0x4b272c */
extern short crossScript3; /* @data 0x4b272e */
extern short dealerScript; /* @data 0x4b2730 */
extern short crossOnceFlag; /* @data 0x4b266e */
extern short level4Stage; /* @data 0x4b2744 */
extern short dealerView; /* @data 0x4b2594 */
extern short dealerRunning; /* @data 0x4a483c */
extern short featuresTaken; /* @data 0x4b273a */
void stepBackNotify(View *, short event);
extern short leftTargetFeatures[4]; /* @data 0x4b28a2 */
extern short rightTargetFeatures[4]; /* @data 0x4b28aa */
extern short leftFeatureMarks[4]; /* @data 0x4b28b2 */
extern short rightFeatureMarks[4]; /* @data 0x4b28ba */
void makeSmokeRows(Snoid *snoid, short n);
extern short rowPlaceOrder[8]; /* @data 0x4b2768: where each of views 1-6 stands (rowPlaces) */
extern short rowViewCount; /* @data 0x4b2662 */
extern Point rowPlaces[8]; /* @data 0x4a44f0 */
extern Point smokeRowStart; /* @data 0x4a4530 */
void setOutSmokeSnoids();
extern Point madePlaces1[2]; /* @data 0x4a4524 */
extern Point madePlaces2[2]; /* @data 0x4a462c */
extern short randomViewCount; /* @data 0x4b2660 */
extern short dealtViewCount; /* @data 0x4b2664 */
extern short slotPairViewCount; /* @data 0x4b2666 */
extern short comparedViewCount; /* @data 0x4b2668 */
void addSmokeSnoids(short kind, short count);
extern short dealLightDue; /* @data 0x4b2748 */
extern short dealerScript2; /* @data 0x4b2732 */
void addLevelSnoids(short level);
short startRound();
extern short unusedSmoke1; /* @data 0x4b264c */
extern short crossedChosen; /* @data 0x4b2670 */
extern short unusedSmoke2; /* @data 0x4b2738 */
extern short unusedSmoke3; /* @data 0x4b274c */
extern short leadersDue; /* @data 0x4b2750 */
extern short dealLit; /* @data 0x4b2752 */
extern short soundOnBeforeSmoke; /* @data 0x4b274e */
extern long lastSmokeFidgetTime; /* @data 0x4b2758 */
extern unsigned long smokeFidgetersUsed; /* @data 0x4b275c: slots used (allocateSlot) */
extern short smokeFidgets; /* @data 0x4b2760 */
extern short smokeFidgeting; /* @data 0x4b2762 */
extern long unusedSmoke4; /* @data 0x4b2764 */
extern long smokeDragOrigin; /* @data 0x4b2794 */
extern long smokeDragOriginStart; /* @data 0x4a47cc */
extern short view11009; /* @data 0x4b258a */
extern short crossedMarkers[20]; /* @data 0x4b25b4 */
extern short crossedSnoids[20]; /* @data 0x4b25dc */
extern short pairScriptIndex; /* @data 0x4b273e */
extern short view11077; /* @data 0x4b2598 */
extern Point smokeSpotPoint; /* @data 0x4a44b4 */
extern GroupList smokeGroups; /* @data 0x4a47a8 */
void openSmoke();
extern ShortRect dealButtonRect; /* @data 0x4a4614 */
extern ShortRect smokeWaitArea; /* @data 0x4a47d0 */
extern short smokeWaitRows[5]; /* @data 0x4a47e0 */
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
extern Point freeSpotOrigin; /* @data 0x4a4846 */
extern Point smokePlaces[20]; /* @data 0x4a47ec */
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
extern short level3OpenCells[26]; /* @data 0x4a4238: level 3's open cells */
extern short level3BlockedCells[43]; /* @data 0x4a426c: level 3's blocked cells */
extern short level3Links36Cells[20]; /* @data 0x4a42c2 */
extern short level3Links9Cells[20]; /* @data 0x4a42ea */
extern short rowFirstCells[14]; /* @data 0x4a4374: levels 0-1: the first row's cell, by rows */
extern short rowSteps[14]; /* @data 0x4a4390: and the cells between rows */
extern short level2BlockedCells[18]; /* @data 0x4a43ac: level 2's blocked cells */
extern short level2OpenCells[18]; /* @data 0x4a43d0: its open cells */
extern short level2StartCells[3]; /* @data 0x4a43f4: its start cells */
extern short level2Links5Cells[3]; /* @data 0x4a43fa */
extern short level2Links18Cells[12]; /* @data 0x4a4400 */

#endif
