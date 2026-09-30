/*
 * bridge's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BRIDGE_H
#define BRIDGE_H

extern GroupList bridgeGroups[1]; /* @data 0x4a0e04 */
extern SceneButton bridgeButtons[2]; /* @data 0x4a0d88 */
extern long g_4a0e24; /* @data 0x4a0e24 */
extern Point upperPlaces[16]; /* @data 0x4a0e28: where the Zoombinis across the upper bridge stand */
extern Point lowerPlaces[16]; /* @data 0x4a0e68: and the lower */
extern ShortRect g_4a0ea8; /* @data 0x4a0ea8: where to wait by the upper bridge */
extern ShortRect g_4a0eb0; /* @data 0x4a0eb0: and the lower */
extern short g_4a0f08; /* @data 0x4a0f08: button 2 is drawn lit */
extern short g_4a0f0c; /* @data 0x4a0f0c: bridgeFrame is running */
extern short g_4a0f0a; /* @data 0x4a0f0a: button 1 is drawn */
extern long g_4ab784; /* @data 0x4ab784: bridge.mhk */
extern short g_4ab788; /* @data 0x4ab788: the scene is open */
extern short g_4ab78a; /* @data 0x4ab78a: button 2 is live */
extern short g_4ab78c; /* @data 0x4ab78c */
extern short g_4ab78e; /* @data 0x4ab78e */
extern short g_4ab790; /* @data 0x4ab790: the level */
extern short g_4ab792[16]; /* @data 0x4ab792 */
extern short g_4ab7b2[16]; /* @data 0x4ab7b2 */
extern unsigned long g_4ab7d4; /* @data 0x4ab7d4: when the timer started (clockTime) */
extern short g_4ab7d8; /* @data 0x4ab7d8 */
extern short g_4ab7da; /* @data 0x4ab7da: Zoombinis sent back (up to 6) */
extern short g_4ab7dc; /* @data 0x4ab7dc */
extern short g_4ab7e0; /* @data 0x4ab7e0 */
extern short g_4ab7de; /* @data 0x4ab7de */
extern short g_4ab7e2; /* @data 0x4ab7e2 */
extern short g_4ab7e4; /* @data 0x4ab7e4 */
extern short g_4ab7e6; /* @data 0x4ab7e6 */
extern short g_4ab7e8; /* @data 0x4ab7e8: the Zoombini crossing passes */
extern short g_4ab7ea; /* @data 0x4ab7ea */
extern short g_4ab7ec; /* @data 0x4ab7ec: the bridge being crossed (1 upper) */
extern short g_4ab7ee; /* @data 0x4ab7ee */
extern short g_4ab7f0; /* @data 0x4ab7f0 */
extern short g_4ab7f2; /* @data 0x4ab7f2 */
extern short queueBridges[2]; /* @data 0x4ab7f4: the Zoombinis queued to cross: the bridge (1 upper), */
extern short queueViews[2]; /* @data 0x4ab7f8: the view */
extern short queuePasses[2]; /* @data 0x4ab7fc: and whether it passes (turnedBack) */
extern short g_4ab800; /* @data 0x4ab800: how many are queued */
extern short g_4ab802; /* @data 0x4ab802 */
extern FeatureRules bridgeRules; /* @data 0x4ab804 */
extern ImageBank *g_4ab820; /* @data 0x4ab820: the buttons' images */
extern short g_4ab824; /* @data 0x4ab824 */
extern short g_4ab826; /* @data 0x4ab826 */
extern short g_4ab828; /* @data 0x4ab828 */
extern short g_4ab82a; /* @data 0x4ab82a */
extern short g_4ab82c; /* @data 0x4ab82c */
extern short g_4ab82e; /* @data 0x4ab82e */
extern unsigned long g_4ab830; /* @data 0x4ab830 */
extern unsigned long g_4ab834; /* @data 0x4ab834 */
extern unsigned long bridgeFidgetersUsed; /* @data 0x4ab838 */

void startBridgeTimer();
unsigned long bridgeTimer();
extern ShortRect g_4a0eb8; /* @data 0x4a0eb8: where a Zoombini can be dragged */

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
