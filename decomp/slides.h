/*
 * slides's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SLIDES_H
#define SLIDES_H

void placeCellViewImages(View *view);
extern long g_4b1928; /* @data 0x4b1928: Slides.MHK */
extern short g_4b1930; /* @data 0x4b1930: the scene is open */
extern short slidesGoReady; /* @data 0x4b1932 */
extern long g_4a3fc8; /* @data 0x4a3fc8 */
extern SceneButton slidesButtons[2]; /* @data 0x4a3f28: buttons 1 and 2 */
extern short g_4a41e0; /* @data 0x4a41e0: button 2 is drawn lit */
extern short g_4a41e2; /* @data 0x4a41e2: button 1 has been drawn */
extern Point listedPoints[27]; /* @data 0x4b1a48: where to show them (from 1) */
extern short listedCells[27]; /* @data 0x4b1ab4: cells (from 1, listedCount of them) */
extern short listedCount; /* @data 0x4b240e */
extern short lastLitCount; /* @data 0x4b1a42 */
extern short litSum; /* @data 0x4b1a44: the sum of the counted cells' numbers */
extern short g_4b1a46; /* @data 0x4b1a46 */
void closeScene12();
void updateSlidesButtons(View *, short region);
void markLitSnoids();
void noteAnyLit();
short countLitCells();
void remarkOnLit();
extern ImageBank *g_4a3fc4; /* @data 0x4a3fc4: the buttons' images */
extern short g_4b251c[4]; /* @data 0x4b251c: how many values of each feature the party shows */
extern ChosenSnoids *g_4b192c; /* @data 0x4b192c: the party */
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
extern short g_4b2514; /* @data 0x4b2514 */
void layerSnoidViews();
void placeCellImages(View *view);
extern Point cellPoints[117]; /* @data 0x4a3fcc: where each cell is drawn */
extern short g_4b1a3e; /* @data 0x4b1a3e: a cell is marked */
extern short g_4b1a36; /* @data 0x4b1a36: the cell marked */
extern short g_4b1a34; /* @data 0x4b1a34: the marker's view */
extern short g_4b1936[10]; /* @data 0x4b1936: views, by row */
void walkToMarkNotify(View *view, short event);
void drawSlidesButtons(View *);
void cycleColorsUpdate(View *view, short);
void markCellAt(short x, short y);
extern short g_4b1a38; /* @data 0x4b1a38: the Zoombini walking to the marked cell */
extern short g_4b1a3a; /* @data 0x4b1a3a: the facing to take (from 1) */
void groupInThrees();
short sharedStone(short a, short b);

short placeUnalike(short cell, short dir);
extern short g_4a41e4; /* @data 0x4a41e4: scene12Frame is running */
extern short g_4b1a3c; /* @data 0x4b1a3c: cycle colours */
extern unsigned long g_4b2534; /* @data 0x4b2534: when they last cycled */
extern short g_4b251a; /* @data 0x4b251a: the group whose arrival ends the puzzle */
extern short g_4b1934; /* @data 0x4b1934: the level */
extern short g_4b253c; /* @data 0x4b253c: fidgets to do */
extern short g_4b253e; /* @data 0x4b253e: fidgets done */
extern unsigned long g_4b252c; /* @data 0x4b252c: when a Zoombini last fidgeted */
extern unsigned long g_4b2538; /* @data 0x4b2538: slots used (allocateSlot) */
void scene12Frame();

void pairByFeatures();

void tryMove(short from, short via, short to);

void seatPair(short cell);
extern short g_4b2410; /* @data 0x4b2410: the start, in listedCells */
void lightPath();

void linkCells();

short membersShare(short a, short b);
void tryMovesAround(short cell);

void followRoutes(short cell);
extern short g_4a41e6[31]; /* @data 0x4a41e6: cells (g_4a4224 of them) */
extern short g_4a4224; /* @data 0x4a4224 */
void lightFromStarts();
void lightFrom(short cell);
extern short g_4b2412; /* @data 0x4b2412: letters of the cheat "solve" typed */
void relightPath();
short scene12Key(unsigned short key);
extern short g_4b1a40; /* @data 0x4b1a40 */
extern short g_4b2524; /* @data 0x4b2524 */
extern short g_4b2526; /* @data 0x4b2526 */
extern short g_4b2528; /* @data 0x4b2528 */
extern short g_4b2518; /* @data 0x4b2518 */
extern short g_4b241a; /* @data 0x4b241a */
extern short g_4b194a[117]; /* @data 0x4b194a */
extern short g_4b241c[10]; /* @data 0x4b241c */
extern Point g_4b1a4c[27]; /* @data 0x4b1a4c: where the listed cells' views go */
extern GroupList slidesGroups[1]; /* @data 0x4a3fa4 */
void openScene12();

void scene12Clicked(short which);

#endif
