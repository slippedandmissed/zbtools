/*
 * ferry's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FERRY_H
#define FERRY_H

extern short placeViewScripts[]; /* @data 0x4a1440: scripts by returnRoute */
extern short cajunGreetings[4]; /* @data 0x4a13e4: Captain Cajun's scripts (cajunGreetingsUsed picks) */
extern unsigned long cajunGreetingsUsed; /* @data 0x4a13ec: slots used (allocateSlot) */
extern GroupList ferryGroups[1]; /* @data 0x4a14fc */
extern short cajunIdleRemarks[5]; /* @data 0x4a13f0: remarks (cajunIdleRemarksUsed picks) */
extern unsigned long cajunIdleRemarksUsed; /* @data 0x4a13fc: slots used (allocateSlot) */
extern short inFerryFrame; /* @data 0x4a1574: ferryFrame is running */
extern short goodPlacingRemarks[2]; /* @data 0x4a1400: remarks for a good placing (goodPlacingRemarksUsed picks) */
extern unsigned long goodPlacingRemarksUsed; /* @data 0x4a1404 */
extern short badPlacingRemarks[11]; /* @data 0x4a1408: remarks for a bad one (badPlacingRemarksUsed picks) */
extern unsigned long badPlacingRemarksUsed; /* @data 0x4a1420 */
extern short movedRemarks[3]; /* @data 0x4a1434: remarks (movedRemarksUsed picks) */
extern unsigned long movedRemarksUsed; /* @data 0x4a143c */
extern short returnSounds[5]; /* @data 0x4a1424: sounds (returnSoundsUsed picks) */
extern unsigned long returnSoundsUsed; /* @data 0x4a1430: slots used (allocateSlot) */
extern short returnScripts[10]; /* @data 0x4a1454: scripts by returnRoute */
extern short returnNextScripts[10]; /* @data 0x4a1468: and the next */
extern ImageBank *ferryButtonImages; /* @data 0x4a147c: the buttons' images */
extern SceneButton ferryButtons[2]; /* @data 0x4a1480 */
extern long ferryButtonResource; /* @data 0x4a151c */
extern short ferryButton2Lit; /* @data 0x4a1570: button 2 is drawn lit */
extern short ferryButton1Drawn; /* @data 0x4a1572: button 1 is drawn */
extern unsigned long nextIdleRemarkTime; /* @data 0x4aba84: when to make the next idle remark (view ticks) */
extern short forcedFerryCount; /* @data 0x4aba88 */
extern short ferryLevel; /* @data 0x4aba8a: the level */
extern Point returnPlace; /* @data 0x4aba8c */
extern short returnUnderway; /* @data 0x4aba90 */
extern Point returnTarget; /* @data 0x4aba92 */
extern Point *returnAnchor; /* @data 0x4aba98 */
extern Point returnLanding; /* @data 0x4aba9c */
extern short cajunRemarkDue; /* @data 0x4abaa0 */
extern short returnDue; /* @data 0x4abaa2 */
extern short ferryHelpersDue; /* @data 0x4abaa4 */
extern short cajunLeavingGroup; /* @data 0x4abaa6 */
extern long ferryFile; /* @data 0x4abaa8: Ferry.MHK */
extern short ferryOpen; /* @data 0x4abaac: the scene is open */
extern short ferryHasPassengers; /* @data 0x4abaae: button 2 is live */
extern short cajunGreeted; /* @data 0x4abab0 */
extern short view1601; /* @data 0x4abab2 */
extern short cajunView; /* @data 0x4abab4: Captain Cajun's view */
extern short view1602; /* @data 0x4abab6 */
extern short view1603; /* @data 0x4abab8 */
extern short view1704; /* @data 0x4ababa */
extern short view1705; /* @data 0x4ababc */
extern short view1706; /* @data 0x4ababe */
extern short returnPlaceView; /* @data 0x4abac0 */
extern short movingPlaceView; /* @data 0x4abac2 */
extern short lastSceneryView; /* @data 0x4abac4 */
extern short ferryPlaceViews[20]; /* @data 0x4abac6: the placed views */
extern short returnRoute; /* @data 0x4abaee */
extern short nextReturner; /* @data 0x4abaf0 */
extern short returner; /* @data 0x4abaf2 */
extern short ferryLeaving; /* @data 0x4abaf4 */
extern char (*ferryLinks)[8]; /* @data 0x4abaf8: for each placed view, those it touches (from 1; linkFerryPlaces) */
extern short sharedFeatureBits; /* @data 0x4abb04 */
extern short sharedFeatureView; /* @data 0x4abb06 */
extern short goodPlacings; /* @data 0x4abb08 */
extern short badPlacings; /* @data 0x4abb0a */
extern short nextPraiseAt; /* @data 0x4abb0c */
extern short ferrySnoidCount; /* @data 0x4abb0e */
extern short praisedOnce; /* @data 0x4abb10 */
extern short cajunRemarkGroup; /* @data 0x4abb12 */
extern short debugCajunScript; /* @data 0x4abb14: the script F plays */
extern short cajunScript; /* @data 0x4abb16: Captain Cajun's script */
extern Point ferryPlaces[20]; /* @data 0x4a1520: where the Zoombinis wait */

void resetFerry();
void ferryClicked(short which);
void ferryFrame();
void openFerry();
void drawFerryButtons(View *);
short ferryKey(unsigned short key);
void layOutFerry(short id);
void layOutFerryLevel();
void linkFerryPlaces(short draw);
void findFerryPlace(short *spot);
void startNextCrosser(short n);
void crosserNotify(View *view, short event);
void drawFerryButton(short which, short lit, short show);
void updateFerryButtons(View *, short region);
void closeFerry();
void moveFerryOn();
void startCrosserScript(short group, short script, ViewNotify notify, char idleTicks);
void ferryHelperNotify(View *view, short event);
void slideFerryViews(View *, short dx);

#endif
