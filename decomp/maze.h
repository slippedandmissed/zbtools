/*
 * maze's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef MAZE_H
#define MAZE_H

extern short valueCounts[21]; /* @data 0x4aff9a */
void mazeNoDraw(View *);
void mazeNoUpdate(View *, short);
int valueLess50(long, short value);
short indexOfLargestExcept(short exclude);
short valueCount(short i);
short countValuesPresent();
/* maze */
extern short mazeGoReady; /* @data 0x4afc6a */
extern short mazeButton2Lit; /* @data 0x4a25c4 */
extern short mazeButton1Drawn; /* @data 0x4a25c6 */
extern long hotSpotTableResource; /* @data 0x4afc18 */
extern short hotSpotTableHandle; /* @data 0x4afc20 */
extern long unusedMazeResource; /* @data 0x4afbd8 */
extern long mazeHotSpotsXResource; /* @data 0x4afbe0 */
extern long mazeHotSpotsYResource; /* @data 0x4afbe4 */
extern long mazeImagesResource; /* @data 0x4afbc4 */
extern long partHotXResource; /* @data 0x4afbc8 */
extern long partHotYResource; /* @data 0x4afbcc */
extern long mazeFile; /* @data 0x4afc64 */
void loadMazeTable(long *resource, short *handle, short id, short **locked);
void freeMazeTable(long *resource, short *handle);
void drawMazeButton(short which, short lit, short show);
void drawMazeButtons(View *);
void updateMazeButtons(View *, short region);
void closeMaze();
short mazeKey(unsigned short key);
extern short *mazeHotSpotsX; /* @data 0x4afbe8: hot spots: x */
extern short *mazeHotSpotsY; /* @data 0x4afbec: and y */
void startPairedSnoidScript(View *view, short group, ViewNotify, char idleTicks);
void startPairedSnoidPoseScript(View *view, short group, ViewNotify, char idleTicks);
void startPairedSnoid14003(View *view, short group, ViewNotify, char idleTicks);
void placeOnHotSpot35(View *view);
void placeOnHotSpot25(View *view);
extern short gate1CloseDue; /* @data 0x4b0a0a */
extern short gate3CloseDue; /* @data 0x4b0a0c */
extern short emptyValues[21]; /* @data 0x4affc4 */
extern short emptyValues2[21]; /* @data 0x4affee */
extern short emptyValueList[21]; /* @data 0x4b0018 */
extern short emptyValueList2[21]; /* @data 0x4b0042 */
extern short emptyValueCount; /* @data 0x4b00c6 */
extern short anyEmptyValue; /* @data 0x4b00c8 */
extern short emptyValueCount2; /* @data 0x4b00ca */
extern short anyEmptyValue2; /* @data 0x4b00cc */
void mazeArrivalHook(short id);
void listEmptyValues();
short packEmptyValues();
void listEmptyValues2();
short packEmptyValues2();
void listAllValues();
extern short valueKinds[21]; /* @data 0x4a263c */
extern short featureOffsets[4]; /* @data 0x4a2634 */
extern short featureRowCount; /* @data 0x4afc36 */
short largestOfKind(short kind, short low, short high);
short smallestFrom(short least);
short largestBetween(short low, short high);
short smallestPositiveExcept(short exclude);
short firstFeatureNotIn(short which);
extern short mazeEvent10Seen; /* @data 0x4afc2e */
void mazeEndNotify(View *, short event);
void noteEvent10(View *, short event);
void drawMazeSnoid(View *view);
void updateMazeSnoid(View *view, short region);
void copyChosenFeatures();
short copyChosenWithFeature(short id);
extern short helperDoneList[]; /* @data 0x4b0908 */
extern short helperDoneCount; /* @data 0x4b09fc */
extern short hotSpotTableUsed; /* @data 0x4a210c */
extern short hotSpotTableKeys[4]; /* @data 0x4a210e */
void helperDoneNotify(View *view, short event);
short firstFeatureIn(short which, short ignore);
void startPairNotify(View *view, short event);
short *loadHotSpotTable(short which);
void poseDoneNotify(View *view, short event);
void turnOrStartPairNotify(View *view, short event);
void leaveSquareNotify(View *view, short event);
extern short pieceViews[]; /* @data 0x4afd2c */
extern short pieceSecondViews[]; /* @data 0x4afd48 */
extern short pieceScripts[]; /* @data 0x4a2308 */
extern short pieceHasSecond[]; /* @data 0x4a22d0 */
extern short mazeSnoidViews[12]; /* @data 0x4afc4a */
extern short mazeSnoidCount; /* @data 0x4afc60 */
extern short finishedList[]; /* @data 0x4b0958 */
extern short finishedCount; /* @data 0x4b0a00 */
void mazeViewNotify(View *view, short event);
void startMazeView(short n);
extern short pose3List[]; /* @data 0x4b09d0 */
extern short pose3Count; /* @data 0x4b0a06 */
short clearRowsWithFeature(short id);
extern short poseReachedCount; /* @data 0x4b0d26 */
void putSnoidInMaze(View *view, short pose);
extern short lineSecondViews[]; /* @data 0x4afd26 */
extern short lineSecondScripts[]; /* @data 0x4a2324 */
void moveSnoidToSquare(View *view, short group, ViewNotify, char idleTicks);
void mazeSnoidNotify(View *view, short event);
extern short takenRowCount; /* @data 0x4b00d0: how many */
short takeRarestValue(short exclude);
short takeCommonestValue(short low, short high);
short takeCommonestValueCopy(short low, short high);
extern short mazeAnchorView1; /* @data 0x4afc44 */
extern short mazeAnchorView2; /* @data 0x4afc2a */
extern short nextSnoidParts; /* @data 0x4b08b4 */
extern short lineViewCount; /* @data 0x4b0cfe */
extern short unusedMaze1; /* @data 0x4b0d0e */
extern short lineValues[11]; /* @data 0x4b0d10: each line's value */
extern short mazeSequence[20]; /* @data 0x4b0096: the maze's sequence of values */
extern short sequenceIndex; /* @data 0x4b00c2 */
void addMazeSnoidView(Snoid *snoid);
short takeRareRow(short exclude, short whole);
extern short meetingList[]; /* @data 0x4b0980 */
extern short meetingCount; /* @data 0x4b0a02 */
extern short sequenceLength; /* @data 0x4b00be: how many values in mazeSequence */
extern short unusedSequenceIndex; /* @data 0x4b00c0 */
extern short largestLimitByRows[]; /* @data 0x4a2666 */
void addMazeSnoids(short count);
void chooseSequence1();
void chooseSequence2();
extern short mazeLevel; /* @data 0x4afc32 */
void chooseSequence3();
void chooseSequence4();
extern short rowTotals[]; /* @data 0x4a26aa */
void chooseSequence5();
extern short lineOrder[11]; /* @data 0x4a25ca */
extern short squareKindTable[18]; /* @data 0x4a239a */
void setUpMaze(short level);
extern short dragFromStart; /* @data 0x4afc3c */
extern short unusedDragFlag; /* @data 0x4afc3e */
extern short startPlace; /* @data 0x4afc40 */
extern short partsSetUp; /* @data 0x4afc38 */
extern short unusedMaze2; /* @data 0x4afc3a */
extern short pieceView2; /* @data 0x4afc48 */
extern short pieceView1; /* @data 0x4afc46 */
extern short unusedMaze3; /* @data 0x4afc2c */
extern short unusedMaze4; /* @data 0x4b00ce */
extern short unusedMazeTable[10]; /* @data 0x4b00aa */
extern short unusedMaze5; /* @data 0x4a2550 */
extern short unusedMaze6; /* @data 0x4a2552 */
extern short unusedMaze7; /* @data 0x4b08b2 */
extern short exitSpotNext1; /* @data 0x4afe52 */
extern short exitSpotNext2; /* @data 0x4afe54 */
extern short exitSpotNext3; /* @data 0x4afe56 */
extern short exitSpotNext4; /* @data 0x4afe58 */
extern short placedQueue[20]; /* @data 0x4b09a8 */
extern short placedQueueCount; /* @data 0x4b0a04 */
extern short unusedMaze8; /* @data 0x4b0a08 */
extern short unusedMazeTable2[3]; /* @data 0x4afdac */
extern short pieceViews7000[14]; /* @data 0x4afc92 */
extern short unusedMazeTable3[25]; /* @data 0x4b0ccc */
extern short pieceLineKinds[]; /* @data 0x4a22ec */
extern Point startPlacePoints[]; /* @data 0x4a21f0 */
extern Point piecePoints[]; /* @data 0x4a232a */
void openMaze();
/* Maze starting places (1-14, from 0): */
extern short startPlaceF2[14]; /* @data 0x4a2228 */
extern short startPlaceF1[14]; /* @data 0x4a2260 */
extern short startPlaceDirections[14]; /* @data 0x4a227c: the direction faced */
extern short startPlaceIndex[14]; /* @data 0x4a2298 */
extern Point startPlaceSquares[14]; /* @data 0x4a2362: the square */
extern ShortRect startDragAreas[]; /* @data 0x4a2554 */
extern unsigned long sortFlags1; /* @data 0x4a2548 */
extern unsigned long sortFlags2; /* @data 0x4a254c */
void mazeButtonClicked(short button);
void mazeFrame();

#endif
