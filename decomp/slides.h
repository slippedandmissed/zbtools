/*
 * slides's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SLIDES_H
#define SLIDES_H

void placeCellViewImages(View *view);
extern long slidesFile; /* @data 0x4b1928: Slides.MHK */
extern short stoneRiseOpen; /* @data 0x4b1930: the scene is open */
extern short slidesGoReady; /* @data 0x4b1932 */
extern long slidesButtonResource; /* @data 0x4a3fc8 */
extern SceneButton slidesButtons[3]; /* @data 0x4a3f28: buttons 1 and 2, then the whole screen (the input group's items) */
extern short slidesButton2Lit; /* @data 0x4a41e0: button 2 is drawn lit */
extern short slidesButton1Drawn; /* @data 0x4a41e2: button 1 has been drawn */
extern short listedCells[27]; /* @data 0x4b1ab4: cells (from 1, listedCount of them) */
extern short listedCount; /* @data 0x4b240e */
extern short lastLitCount; /* @data 0x4b1a42 */
extern short litSum; /* @data 0x4b1a44: the sum of the counted cells' numbers */
extern short lastLitSum; /* @data 0x4b1a46 */
void closeStoneRise();
void updateSlidesButtons(View *, short region);
void markLitSnoids();
void noteAnyLit();
short countLitCells();
void remarkOnLit();
extern ImageBank *slidesButtonImages; /* @data 0x4a3fc4: the buttons' images */
extern short featureValueCounts[4]; /* @data 0x4b251c: how many values of each feature the party shows */
extern ChosenSnoids *slidesChosen; /* @data 0x4b192c: the party */
void drawSlidesButton(short which, short lit, short show);
void countPartyValues();
void cycleColors();
extern short groupCount; /* @data 0x4b2450 */
extern short pairFeatures[16]; /* @data 0x4b2452 */
void readPartyFeatures();
void orderPartyByAlike();
void placeListedCell(View *view);
extern Point slidesPlaces[16]; /* @data 0x4a41a0: where the Zoombinis wait */
void pickSlidesPlace(Point *where);
short findAlike(short who);
extern short linkImageOffset; /* @data 0x4b2514 */
void layerSnoidViews();
void placeCellImages(View *view);
extern Point cellPoints[117]; /* @data 0x4a3fcc: where each cell is drawn */
extern short cellMarked; /* @data 0x4b1a3e: a cell is marked */
extern short markedCell; /* @data 0x4b1a36: the cell marked */
extern short markerView; /* @data 0x4b1a34: the marker's view */
extern short slidesRowViews[10]; /* @data 0x4b1936: views, by row */
void walkToMarkNotify(View *view, short event);
void drawSlidesButtons(View *);
void cycleColorsUpdate(View *view, short);
void markCellAt(short x, short y);
extern short markWalker; /* @data 0x4b1a38: the Zoombini walking to the marked cell */
extern short pendingMarkFacing; /* @data 0x4b1a3a: the facing to take (from 1) */
void groupInThrees();
short sharedStone(short a, short b);

short placeUnalike(short cell, short dir);
extern short inStoneRiseFrame; /* @data 0x4a41e4: stoneRiseFrame is running */
extern short cyclingColours; /* @data 0x4b1a3c: cycle colours */
extern unsigned long lastColourCycle; /* @data 0x4b2534: when they last cycled */
extern short finishGroup; /* @data 0x4b251a: the group whose arrival ends the puzzle */
extern short stoneRiseLevel; /* @data 0x4b1934: the level */
extern short slidesFidgetsAllowed; /* @data 0x4b253c: fidgets to do */
extern short slidesFidgets; /* @data 0x4b253e: fidgets done */
extern unsigned long lastSlidesFidgetTime; /* @data 0x4b252c: when a Zoombini last fidgeted */
extern unsigned long slidesFidgetersUsed; /* @data 0x4b2538: slots used (allocateSlot) */
void stoneRiseFrame();

void pairByFeatures();

void tryMove(short from, short via, short to);

void seatPair(short cell);
extern short pathStart; /* @data 0x4b2410: the start, in listedCells */
void lightPath();

void linkCells();

short membersShare(short a, short b);
void tryMovesAround(short cell);

void followRoutes(short cell);
extern short lightCells[31]; /* @data 0x4a41e6: cells (lightCellCount of them) */
extern short lightCellCount; /* @data 0x4a4224 */
void lightFromStarts();
void lightFrom(short cell);
extern short solveTyped; /* @data 0x4b2412: letters of the cheat "solve" typed */
void relightPath();
short stoneRiseKey(unsigned short key);
extern short slidesDragLocked; /* @data 0x4b1a40 */
extern short alikeTarget; /* @data 0x4b2524 */
extern short savedPartyFlags; /* @data 0x4b2526 */
extern short slidesGoPressed; /* @data 0x4b2528 */
extern short startCellsGroup; /* @data 0x4b2518 */
extern short startCellCount; /* @data 0x4b241a */
extern short unusedCellTable[117]; /* @data 0x4b194a */
extern short startCells[10]; /* @data 0x4b241c */
extern Point listedCellPlaces[26]; /* @data 0x4b1a4c: where the listed cells' views go */
extern GroupList slidesGroups[1]; /* @data 0x4a3fa4 */
void openStoneRise();

void stoneRiseClicked(short which);

extern Group g_4a3f94[1]; /* pointed to by initialised data */
extern Scene g_4a3fb0[1]; /* pointed to by initialised data */

#endif
