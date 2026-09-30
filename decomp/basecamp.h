/*
 * basecamp's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BASECAMP_H
#define BASECAMP_H

extern short preloaded; /* @data 0x4ab4a8: a handle of the resources preloaded */
extern short preloadedCount; /* @data 0x4ab4aa */
extern Wipe wipe; /* @data 0x4ab4ac */
extern long cheatCode; /* @data 0x4ab4d8: the last keys typed, 7 bits each */
extern Blinds blinds; /* @data 0x4ab4dc */
extern long shapeListKind; /* @data 0x4a07f0: 'SHPL' */
extern long soundListKind; /* @data 0x4a07f4: 'SNDL' */
extern long noPreloadKind; /* @data 0x4a07f8 */
extern long cheatHash; /* @data 0x4a07fc */
extern short campRow; /* @data 0x4ab508: the first row of the camp shown */
extern short campRows; /* @data 0x4ab50a */
extern short campShown; /* @data 0x4ab50c: slots shown (campRows * 5) */
extern short campCount; /* @data 0x4ab50e: Zoombinis in the camp */
extern short campLast; /* @data 0x4ab510: the last slot used */
extern short g_4ab512;
extern Camp *camp; /* @data 0x4ab514 */
extern short g_4ab518;
extern short g_4ab51a;
extern short g_4ab524;
extern short g_4ab526;
extern short g_4ab52a;
extern short g_4ab52c;
extern short g_4ab52e;
extern SceneButton campButtons[7]; /* @data 0x4a0824 */
extern ImageBank *campButtonImages; /* @data 0x4a0970 */
extern long campButtonsResource; /* @data 0x4a0968: holding campButtonImages */
extern long campFrameResource; /* @data 0x4a096c: holding g_4a0974 */
extern long campMap; /* @data 0x4ab520: BaseCamp.MHK */
extern short campActive; /* @data 0x4ab528 */
extern short campBusy; /* @data 0x4a0a9c: in campIdle */
extern ImageBank *g_4a0974; /* the camp's frame */
extern ShortRect campButtonsBounds; /* @data 0x4a0a9e */
extern ShortRect campArrival; /* @data 0x4a0aa6: where Zoombinis back from the journey stand */
extern short g_4a080c;
extern short g_4a080e; /* the camp is scrolled half a row */
extern ShortRect campArea; /* @data 0x4a0920 */
extern Snoid draggedSnoid; /* @data 0x4ab53a */
extern GroupList campGroupLists[2]; /* @data 0x4a0998 */
extern long g_4ab51c; /* the way the camp is asked to scroll (1-4) */
extern short campX[10]; /* @data 0x4a09b0: each row's x, in two layouts (4a080e) */
extern short primes[5]; /* @data 0x4a0800 */
/* basecamp */
unsigned short loadWave(short key);
unsigned short findAndLoadWave(short key);
void unloadWave(short key);
void unloadWaveNow(short key);
short playWaveOn(short key, short channel);
short findAndPlayWave(short key, short channel);
void stopWaves(unsigned short id);
void endWaveLoops(unsigned short id);
short isWavePlaying(unsigned short id);
short playWave(short key, short channel, short eventType, short discard);
short loadAndPlayWave(short key, short channel, short eventType, short discard);
short waitForWave(unsigned short id, short eventType, short discard);
short awaitWave(unsigned short id, short eventType, short discard);
short wavePlayingOrStop(unsigned short id, short stop);
short waveValueReached(char value);
short waitForWaveValue(char value, short eventType, short discard);
short awaitWaveValue(char value, short eventType, short discard);
short runWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
              short duration, unsigned short direction, short eventType, short discard);
void startWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
               short duration, unsigned short direction);
short stepWipe();
void drawWipe(short upTo);
short wipeScreen(const ShortRect *rect, short duration, unsigned short direction, short eventType,
                 short discard);
void startScreenWipe(const ShortRect *rect, short duration, unsigned short direction);
void noteCheatKey(unsigned short key);
int isCheat(long hash, long code);
void getShapeSize(ResourceList *list, unsigned short index, short *height, short *width);
short randomBetween(short low, short high);
void reduceFraction(short *numerator, short *denominator, unsigned short largest);
short runBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                short duration, short stripe, short eventType, short discard);
void startBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                 short duration, short stripe);
short stepBlinds();
void drawBlinds(short upTo);
short blindsScreen(const ShortRect *rect, short duration, short stripe, short eventType,
                   short discard);
void startScreenBlinds(const ShortRect *rect, short duration, short stripe);
void preloadResource(long type, short id, long *kind);
void *preloadDone(long event, long id, void *kind);
void freePreloaded(short release);
void copyRegion(basePort *to, basePort *from, short region);
void showRegion(short region);
void getDateTime(short *year, char *month, char *day, char *hour, char *minute);
void drawOutlinedText(unsigned short outline, unsigned short color, ShortRect rect,
                      unsigned short flags, const char *text);
void nudgeRect(ShortRect *rect, short direction);
void resetCamp();
long campKey(long);
void enterCamp();
void leaveCamp();
void campIdle();
void campButtonClicked(short button);
void campMouse(short action);
void drawSceneButtons(short button, short pressed, short group, short show); /* 0x41790f */
void drawSceneButtons1(View *);
void drawSceneButtons2(View *);
void updateCampButtons(View *, short region);
short findCampSlot(short start, ShortRect rect, short occupied);
void scrollCamp(View *view, short);
void drawCamp(View *);
void noteCampSlot(short slot);
short campSlotsUsed();
void insertCampRow();
void compactCamp();
void updateCampScroll(short stop);
void refreshCampView();
short returnToCamp();

#endif
