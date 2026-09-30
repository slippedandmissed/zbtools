/*
 * net's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef NET_H
#define NET_H

void enterNextScene();
short sceneGroup(short *last);
void spliceList(Link *other, Link *list);
void layOutMazeCels(Snoid *snoid);
/* net */
void setUpMazeParts(Snoid *snoid);
View *sortViewsByDepth(View *list);
void sortFlaggedViews(View *after, unsigned long flags);
extern short netOpen; /* @data 0x4b12a8: the scene is open */
extern long netButtonResource; /* @data 0x4a2e54 */
extern long netFile; /* @data 0x4b12a4 */
void placeSecondCel(View *view);
void closeNet();
extern short codeColumns[125]; /* @data 0x4b0e78 */
extern short codeRows[125]; /* @data 0x4b0f72 */
extern short codeLayers[125]; /* @data 0x4b106c */
extern short netGoAllowed; /* @data 0x4b12aa */
extern ImageBank *netButtonImages; /* @data 0x4a2e60 */
extern short netButton2Lit; /* @data 0x4a2ea4 */
extern short netButton1Drawn; /* @data 0x4a2ea6 */
void fillCodeRowColumn(short a, short b, short n);
void fillCodeCube(short a, short b, short c, short n);
void drawNetButton(short which, short lit, short show);
void drawNetButtons(View *);
void updateNetButtons(View *, short region);
extern short netGroups[20]; /* @data 0x4b1182 */
extern short placeGroups[]; /* @data 0x4b11aa */
extern short netGroupCount; /* @data 0x4b0e76 */
extern short netPartySize; /* @data 0x4b0e66 */
void splitIntoGroups();
void moveMazeSnoidOn(View *view, short other);
extern short nextToSend; /* @data 0x4b0e68 */
extern short netPlaces[3]; /* @data 0x4b1438 */
extern short sendAllowed; /* @data 0x4b144a */
extern short sendsLeft; /* @data 0x4b145c */
extern short sentIndex; /* @data 0x4b0d5c */
extern short currentNetPlace; /* @data 0x4b119a */
extern short sendUnderway; /* @data 0x4b1462 */
extern short emptyNetPlaces; /* @data 0x4b11a0 */
void sendNextToNet();
void putOnSquare(View *view, short);
void stopMazeSnoid(short id);
extern short markerStep; /* @data 0x4b1456 */
extern short markerPlace; /* @data 0x4b119e */
extern short netLevel; /* @data 0x4b12ac */
extern Point markerPlaces[]; /* @data 0x4a2b7e */
extern Point markerPlaces3d[]; /* @data 0x4a2be2 */
extern short markerX; /* @data 0x4b1452 */
extern short markerY; /* @data 0x4b1454 */
extern short markerDx; /* @data 0x4b117e */
extern short markerDy; /* @data 0x4b1180 */
extern short markerCount; /* @data 0x4b13ca */
extern short markerViews[]; /* @data 0x4b13cc */
extern short markerColumnKind; /* @data 0x4a2e58 */
extern short markerMoved; /* @data 0x4b1450 */
extern short netMarkerView; /* @data 0x4b12b6 */
void markerPlaced(View *view);
void landMarker(short n);
void updateFlyingMarker(View *view, short region);
void flyMarker(short n);
extern short standingViews[5]; /* @data 0x4b12ba */
extern short markerMissed; /* @data 0x4b144e */
extern short groupToCross; /* @data 0x4b145a */
extern short promptHeld; /* @data 0x4b1466 */
extern short netFidgetsOn; /* @data 0x4b147c */
extern short snoidsFound; /* @data 0x4b0e6c */
void stepMazeSnoidOn(View *view);
void stepAtTurning(View *view, short other);
extern short codeOrder1; /* @data 0x4b1178 */
extern short codeOrder2; /* @data 0x4b117a */
extern short codeOrder3; /* @data 0x4b117c */
void drawCodesBox();
extern short stepView; /* @data 0x4b12b8 */
extern short netGuideView; /* @data 0x4b12ca */
extern short guideGroup; /* @data 0x4b141e */
extern short promptView; /* @data 0x4b13fe */
extern short code1View; /* @data 0x4b1400 */
extern short code2View; /* @data 0x4b1402 */
extern short code3View; /* @data 0x4b1404 */
extern short chosenCode1; /* @data 0x4b143e */
extern short chosenCode2; /* @data 0x4b1442 */
extern short chosenCode3; /* @data 0x4b1446 */
extern short revealView; /* @data 0x4b13c6 */
extern short revealGroup; /* @data 0x4b142a */
void addNetViews();
extern short revealSteps; /* @data 0x4b140a */
extern short revealScript; /* @data 0x4b140c */
extern short revealStepsAtOpen; /* @data 0x4b140e */
extern short codesDue; /* @data 0x4b1410 */
extern short netTriesOver; /* @data 0x4b142c */
short netKey(unsigned short key);
void stepMazeSnoid(View *view, short other);
extern short codeEntryCount; /* @data 0x4b142e */
extern short codeOrderHigh; /* @data 0x4b1468 */
short findCodeEntry();
/* Each scene's ambient sounds, and which have played (a bit each). */
extern short bridgeSounds[9]; /* @data 0x4a2740 */
extern unsigned long bridgeSoundsUsed; /* @data 0x4a2754 */
extern short tunnelsSounds[9]; /* @data 0x4a2758 */
extern unsigned long tunnelsSoundsUsed; /* @data 0x4a276c */
extern short pizzaSounds[12]; /* @data 0x4a2770 */
extern unsigned long pizzaSoundsUsed; /* @data 0x4a2788 */
extern short campSounds[15]; /* @data 0x4a278c */
extern unsigned long campSoundsUsed; /* @data 0x4a27ac */
extern short ferrySounds[19]; /* @data 0x4a27b0 */
extern unsigned long ferrySoundsUsed; /* @data 0x4a27d8 */
extern short lillySounds[20]; /* @data 0x4a27dc */
extern unsigned long lillySoundsUsed; /* @data 0x4a2804 */
extern short stoneRiseSounds[13]; /* @data 0x4a2808 */
extern unsigned long stoneRiseSoundsUsed; /* @data 0x4a2824 */
extern short camp2Sounds[10]; /* @data 0x4a2828 */
extern unsigned long camp2SoundsUsed; /* @data 0x4a283c */
extern short fleensSounds[13]; /* @data 0x4a2840 */
extern unsigned long fleensSoundsUsed; /* @data 0x4a285c */
extern short netSounds[17]; /* @data 0x4a2860 */
extern unsigned long netSoundsUsed; /* @data 0x4a2884 */
extern short cavesSounds[10]; /* @data 0x4a2888 */
extern unsigned long cavesSoundsUsed; /* @data 0x4a289c */
extern short mazeSounds[10]; /* @data 0x4a28a0 */
extern unsigned long mazeSoundsUsed; /* @data 0x4a28b4 */
extern short smokeSounds[10]; /* @data 0x4a28b8 */
extern unsigned long smokeSoundsUsed; /* @data 0x4a28cc */
extern unsigned long ambientSoundTime; /* @data 0x4b0d44: when to try the next */
void playAmbientSound();
extern short previousCode1; /* @data 0x4b1440 */
extern short previousCode2; /* @data 0x4b1444 */
extern short previousCode3; /* @data 0x4b1448 */
extern Point acrossSpots[]; /* @data 0x4a2dd6 */
extern short waitingOnNet[3]; /* @data 0x4b12b0 */
extern short netFacing; /* @data 0x4b11a2 */
extern short movingSnoid; /* @data 0x4b0e6a */
extern short lastAcross; /* @data 0x4b1412 */
extern short crossingStarted; /* @data 0x4b1414 */
extern short lastCrossed; /* @data 0x4b147e */
extern short allAcross; /* @data 0x4b11a6 */
extern short acrossCount; /* @data 0x4b11a8 */
extern short netFidgetsAllowed; /* @data 0x4b1478 */
extern short promptHeld2; /* @data 0x4b144c */
extern short remarkAfterCross; /* @data 0x4a28d0 */
void crossingNotify(View *view, short event);
extern short codeRowsCopy[5]; /* @data 0x4b1166 */
extern short placeGroupViews[]; /* @data 0x4b12cc */
void setUpCodes();
void mazeZoombinisMeet(View *a, View *b);
extern short codesShown; /* @data 0x4b1416 */
extern short markerGroup; /* @data 0x4b1464 */
void chooseCode(short which, short value);
extern short codesLocked; /* @data 0x4b11a4 */
extern unsigned long lastPromptTime; /* @data 0x4b1470 */
void netClicked(short button);
extern short markerStep1Group; /* @data 0x4b141a */
extern short markerStep2Group; /* @data 0x4b141c */
extern short markerStep3Group; /* @data 0x4b1420 */
extern short markerStep4Group; /* @data 0x4b1422 */
extern short markerStep5Group; /* @data 0x4b1424 */
extern short unusedNet1; /* @data 0x4b1426 */
extern short unusedNet2; /* @data 0x4b1428 */
extern short revealStep; /* @data 0x4b1408 */
extern short unusedNet3; /* @data 0x4b13c8 */
extern short unusedNet4; /* @data 0x4b12ae */
extern short firstPrompt; /* @data 0x4b1418 */
extern short sentCount; /* @data 0x4b1430 */
extern short unusedNet5; /* @data 0x4b1432 */
extern short unusedNet6; /* @data 0x4b1434 */
extern short unusedNet7; /* @data 0x4b1436 */
extern short unusedNet8; /* @data 0x4b119c */
extern short standingGroup; /* @data 0x4b145e */
extern short stepGroup; /* @data 0x4b1460 */
extern short crossDue; /* @data 0x4b1458 */
extern short revealing; /* @data 0x4b1406 */
extern short netFidgets; /* @data 0x4b147a */
extern ChosenSnoids *netChosen; /* @data 0x4b0d68 */
extern GroupList netGroupList[1]; /* @data 0x4a2e32 */
void openNet();
extern short inNetFrame; /* @data 0x4a2ea8 */
extern short netFrameEntered; /* @data 0x4b1480 */
extern unsigned long lastNetFidgetTime; /* @data 0x4b146c */
extern unsigned long netFidgetersUsed; /* @data 0x4b1474 */
extern View *netFidgeter; /* @data 0x4b0d60 */
void netFrame();

#endif
