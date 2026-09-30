/*
 * xfer's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef XFER_H
#define XFER_H

extern long journeyFile; /* @data 0x4b98d4: xfer.MHK */
extern short journeyOpen; /* @data 0x4b98d8: the scene is open */
extern short shownPopulation; /* @data 0x4b98da: Zoombiniville's population, when the scene opened */
extern short placeLevelShown; /* @data 0x4b98dc */
extern long nextJourneyMoveTime; /* @data 0x4b98e0 */
extern char placeLevels[17]; /* @data 0x4b98e4: the places' levels (readPlaceLevels) */
extern GroupList xferGroups[1]; /* @data 0x4a7e9e */
extern ShortRect mapTitleRects[4]; /* @data 0x4a7ebe: where each map's name goes */
extern short inJourneyFrame; /* @data 0x4a7ede: in journeyFrame */
extern short mapPlaces[4][5]; /* @data 0x4a7ee0: the places on each of xferMap's maps */
extern Point gridStarts[16]; /* @data 0x4a7f08: where each map's grid starts */
extern short view5108; /* @data 0x4b98f6 */
extern short views5104[4]; /* @data 0x4b98f8 */
extern short views5102[2]; /* @data 0x4b9900 */
extern short pendingJourneyFacing; /* @data 0x4b9904 */
extern short journeyAnchorView; /* @data 0x4b9906 */
extern short snoidsPastAnchor; /* @data 0x4b9908 */
extern short view6108; /* @data 0x4b990a */
extern short views5102Due[3]; /* @data 0x4b990c */
extern short journeyFirstMoveDone; /* @data 0x4b9912 */
extern short xferSound; /* @data 0x4b9914 */
extern short xferMap; /* @data 0x4b9916 */
extern short nextWalker; /* @data 0x4b9918 */
extern short journeyPartySize; /* @data 0x4b991a */
extern short destinationPlace; /* @data 0x4b991c */
extern short destinationLevel; /* @data 0x4b991e */
extern short gridView; /* @data 0x4b9920 */
extern short view6106; /* @data 0x4b9922 */
extern short view6107; /* @data 0x4b9924 */
extern short populationSignView; /* @data 0x4b9926 */
extern short destinationImage; /* @data 0x4b9928 */
extern unsigned long gridProgress; /* @data 0x4b992c: drawGridView's progress (per mille) */
extern unsigned long gridCellsTotal; /* @data 0x4b9930 */
extern unsigned long gridCellsLeft; /* @data 0x4b9934 */
extern unsigned long gridStride; /* @data 0x4b9938: the grid's stride */
extern unsigned long gridRows; /* @data 0x4b993c: its rows */
extern unsigned long gridColumns; /* @data 0x4b9940: its columns */
extern Point gridMarks[24]; /* @data 0x4b9944 */
extern char gridMarkUsed[24]; /* @data 0x4b99a4: gridMarks's in use */
extern char *gridCells; /* @data 0x4b99bc: the grid */
extern char gridFree1; /* @data 0x4b99c0 */
extern char gridTaken1; /* @data 0x4b99c1 */
extern char gridFree2; /* @data 0x4b99c2 */
extern char gridTaken2; /* @data 0x4b99c3 */

void resetJourney();
void openJourney();
void closeJourney();
void drawPopulationSign(View *view);
void readPlaceLevels(char *levels);
void placeMapImages(View *view);
void journeyFrame();
void journeyClicked(short which);
unsigned long spreadGridMarks(long permille);
void xferSnoidNotify(View *view, short event);
void setUpGrid(char *grid, unsigned long stride, unsigned long rows, unsigned long columns,
                unsigned char from1, unsigned char from2, char to1, char to2, char taken1,
                char taken2, Point &start);
void markGridCell(char *cell, long x, long y);
void placeMapPlace(View *view);
void drawGridView(View *view);
void updateGridView(View *view, short region);
short journeyKey(unsigned short);
void xferEndNotify(View *, short event);

extern InputItem g_4a7e6a[1]; /* pointed to by initialised data */
extern Group g_4a7e8e[1]; /* pointed to by initialised data */
extern Scene g_4a7eaa[1]; /* pointed to by initialised data */

#endif
