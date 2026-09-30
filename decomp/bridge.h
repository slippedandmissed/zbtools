/*
 * bridge's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BRIDGE_H
#define BRIDGE_H

extern GroupList bridgeGroups[1]; /* @data 0x4a0e04 */
extern SceneButton bridgeButtons[2]; /* @data 0x4a0d88 */
extern long bridgeButtonResource; /* @data 0x4a0e24 */
extern Point upperPlaces[16]; /* @data 0x4a0e28: where the Zoombinis across the upper bridge stand */
extern Point lowerPlaces[16]; /* @data 0x4a0e68: and the lower */
extern ShortRect upperWaitArea; /* @data 0x4a0ea8: where to wait by the upper bridge */
extern ShortRect lowerWaitArea; /* @data 0x4a0eb0: and the lower */
extern short bridgeButton2Lit; /* @data 0x4a0f08: button 2 is drawn lit */
extern short inBridgeFrame; /* @data 0x4a0f0c: bridgeFrame is running */
extern short bridgeButton1Drawn; /* @data 0x4a0f0a: button 1 is drawn */
extern long bridgeFile; /* @data 0x4ab784: bridge.mhk */
extern short bridgeOpen; /* @data 0x4ab788: the scene is open */
extern short bridgeGoReady; /* @data 0x4ab78a: button 2 is live */
extern short upperCount; /* @data 0x4ab78c */
extern short lowerCount; /* @data 0x4ab78e */
extern short bridgeLevel; /* @data 0x4ab790: the level */
extern short upperViews[16]; /* @data 0x4ab792 */
extern short lowerViews[16]; /* @data 0x4ab7b2 */
extern unsigned long bridgeTimerStart; /* @data 0x4ab7d4: when the timer started (clockTime) */
extern short bridgeDragStarted; /* @data 0x4ab7d8 */
extern short sentBackCount; /* @data 0x4ab7da: Zoombinis sent back (up to 6) */
extern short view1201; /* @data 0x4ab7dc */
extern short view1200; /* @data 0x4ab7e0 */
extern short view1202; /* @data 0x4ab7de */
extern short view1105; /* @data 0x4ab7e2 */
extern short view1103; /* @data 0x4ab7e4 */
extern short cliffSpoke; /* @data 0x4ab7e6 */
extern short crosserPasses; /* @data 0x4ab7e8: the Zoombini crossing passes */
extern short crossingUnderway; /* @data 0x4ab7ea */
extern short crossingBridge; /* @data 0x4ab7ec: the bridge being crossed (1 upper) */
extern short crossersOut; /* @data 0x4ab7ee */
extern short crossingGroup; /* @data 0x4ab7f0 */
extern short reactingView; /* @data 0x4ab7f2 */
extern short queueBridges[2]; /* @data 0x4ab7f4: the Zoombinis queued to cross: the bridge (1 upper), */
extern short queueViews[2]; /* @data 0x4ab7f8: the view */
extern short queuePasses[2]; /* @data 0x4ab7fc: and whether it passes (turnedBack) */
extern short queuedCount; /* @data 0x4ab800: how many are queued */
extern short crossingEvent; /* @data 0x4ab802 */
extern FeatureRules bridgeRules; /* @data 0x4ab804 */
extern ImageBank *bridgeButtonImages; /* @data 0x4ab820: the buttons' images */
extern short sentBackWalking; /* @data 0x4ab824 */
extern short debugBridgeScript; /* @data 0x4ab826 */
extern short debugBridgeEvent; /* @data 0x4ab828 */
extern short bridgeFidgetsAllowed; /* @data 0x4ab82a */
extern short bridgeFidgets; /* @data 0x4ab82c */
extern short bridgePartySize; /* @data 0x4ab82e */
extern unsigned long lastBridgeFidgetTime; /* @data 0x4ab830 */
extern unsigned long bridgeFidgetInterval; /* @data 0x4ab834 */
extern unsigned long bridgeFidgetersUsed; /* @data 0x4ab838 */

void startBridgeTimer();
unsigned long bridgeTimer();
extern ShortRect bridgeDragArea; /* @data 0x4a0eb8: where a Zoombini can be dragged */

void resetBridge();
void bridgeClicked(short which);
void bridgeFrame();
void drawBridgeButtons(View *);
void openBridge();
void drawBridgeButton(short which, short lit, short show);
void updateBridgeButtons(View *, short region);
void closeBridge();
void bridgeViewNotify(View *view, short event);
short bridgeKey(unsigned short key);
void bridgeSnoidNotify(View *view, short event);
void makeBridgeRule();
short turnedBack(FeatureRules *rules, short edge, Snoid *snoid);

#endif
