/*
 * maze's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef MAZE_H
#define MAZE_H

extern short valueCounts[21];
void mazeNoDraw(View *);
void mazeNoUpdate(View *, short);
int valueLess50(long, short value);
short indexOfLargestExcept(short exclude);
short valueCount(short i);
short countValuesPresent();
/* maze */
extern short g_4afc6a;
extern short g_4a25c4;
extern short g_4a25c6;
extern long g_4afc18;
extern short g_4afc20;
extern long g_4afbd8;
extern long g_4afbe0;
extern long g_4afbe4;
extern long g_4afbc4;
extern long g_4afbc8;
extern long g_4afbcc;
extern long g_4afc64;
void loadMazeTable(long *resource, short *handle, short id, short **locked);
void freeMazeTable(long *resource, short *handle);
void drawMazeButton(short which, short lit, short show);
void drawMazeButtons(View *);
void updateMazeButtons(View *, short region);
void closeMaze();
short mazeKey(unsigned short key);
extern short *mazeHotSpotsX; /* hot spots: x */
extern short *mazeHotSpotsY; /* and y */
void startPairedSnoidScript(View *view, short group, ViewNotify, char unknownF8);
void startPairedSnoidPoseScript(View *view, short group, ViewNotify, char unknownF8);
void startPairedSnoid14003(View *view, short group, ViewNotify, char unknownF8);
void placeOnHotSpot35(View *view);
void placeOnHotSpot25(View *view);
extern short g_4b0a0a;
extern short g_4b0a0c;
extern short emptyValues[21];
extern short emptyValues2[21];
extern short emptyValueList[21];
extern short emptyValueList2[21];
extern short emptyValueCount;
extern short anyEmptyValue;
extern short emptyValueCount2;
extern short anyEmptyValue2;
void mazeArrivalHook(short id);
void listEmptyValues();
short packEmptyValues();
void listEmptyValues2();
short packEmptyValues2();
void listAllValues();
extern short valueKinds[21];
extern short featureOffsets[4];
extern short featureRowCount;
short largestOfKind(short kind, short low, short high);
short smallestFrom(short least);
short largestBetween(short low, short high);
short smallestPositiveExcept(short exclude);
short firstFeatureNotIn(short which);
extern short g_4afc2e;
void mazeEndNotify(View *, short event);
void noteEvent10(View *, short event);
void drawMazeSnoid(View *view);
void updateMazeSnoid(View *view, short region);
void copyChosenFeatures();
short copyChosenWithFeature(short id);
extern short g_4b0908[];
extern short g_4b09fc;
extern short g_4a210c;
extern short g_4a210e[4];
void helperDoneNotify(View *view, short event);
short firstFeatureIn(short which, short ignore);
void startPairNotify(View *view, short event);
short *loadHotSpotTable(short which);
void poseDoneNotify(View *view, short event);
void turnOrStartPairNotify(View *view, short event);
void leaveSquareNotify(View *view, short event);
extern short g_4afd2c[];
extern short g_4afd48[];
extern short g_4a2308[];
extern short g_4a22d0[];
extern short g_4afc4a[12];
extern short g_4afc60;
extern short g_4b0958[];
extern short g_4b0a00;
void mazeViewNotify(View *view, short event);
void startMazeView(short n);
extern short g_4b09d0[];
extern short g_4b0a06;
short clearRowsWithFeature(short id);
extern short g_4b0d26;
void putSnoidInMaze(View *view, short pose);
extern short g_4afd26[];
extern short g_4a2324[];
void moveSnoidToSquare(View *view, short group, ViewNotify, char unknownF8);
void mazeSnoidNotify(View *view, short event);
extern short g_4b00d0; /* how many */
short takeRarestValue(short exclude);
short takeCommonestValue(short low, short high);
short takeCommonestValueCopy(short low, short high);
extern short g_4afc44;
extern short g_4afc2a;
extern short g_4b08b4;
extern short g_4b0cfe;
extern short g_4b0d0e;
extern short g_4b0d10[11]; /* each line's value */
extern short mazeSequence[20]; /* the maze's sequence of values */
extern short g_4b00c2;
void addMazeSnoidView(Snoid *snoid);
short takeRareRow(short exclude, short whole);
extern short g_4b0980[];
extern short g_4b0a02;
extern short sequenceLength; /* how many values in mazeSequence */
extern short g_4b00c0;
extern short g_4a2666[];
void addMazeSnoids(short count);
void chooseSequence1();
void chooseSequence2();
extern short g_4afc32;
void chooseSequence3();
void chooseSequence4();
extern short g_4a26aa[];
void chooseSequence5();
extern short g_4a25ca[11];
extern short g_4a239a[18];
void setUpMaze(short level);
extern short g_4afc3c;
extern short g_4afc3e;
extern short g_4afc40;
extern short g_4afc38;
extern short g_4afc3a;
extern short g_4afc48;
extern short g_4afc46;
extern short g_4afc2c;
extern short g_4b00ce;
extern short g_4b00aa[10];
extern short g_4a2550;
extern short g_4a2552;
extern short g_4b08b2;
extern short g_4afe52;
extern short g_4afe54;
extern short g_4afe56;
extern short g_4afe58;
extern short g_4b09a8[20];
extern short g_4b0a04;
extern short g_4b0a08;
extern short g_4afdac[3];
extern short g_4afc92[14];
extern short g_4b0ccc[25];
extern short g_4a22ec[];
extern Point g_4a21f0[];
extern Point g_4a232a[];
void openMaze();
/* Maze starting places (1-14, from 0): */
extern short g_4a2228[14];
extern short g_4a2260[14];
extern short g_4a227c[14]; /* the direction faced */
extern short g_4a2298[14];
extern Point g_4a2362[14]; /* the square */
extern ShortRect g_4a2554[];
extern unsigned long g_4a2548;
extern unsigned long g_4a254c;
void mazeButtonClicked(short button);
void mazeFrame();

#endif
