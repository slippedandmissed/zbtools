/*
 * town's functions and globals: the declarations only its code and its
 * callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TOWN_H
#define TOWN_H

extern GroupList townGroups[1]; /* @data 0x4a73f0 */
extern SceneButton townButtons[3]; /* @data 0x4a7428: its button, the whole screen and an empty item (the input group's items) */
extern unsigned char clockMinute; /* @data 0x4a751e: the clock's minute hand (0-11) */
extern unsigned char clockHour; /* @data 0x4a751f: and hour hand (0-11) */
extern short introStep; /* @data 0x4b7cec */
extern unsigned long introStepDue; /* @data 0x4b7cf0 */
extern short introOpen; /* @data 0x4b7cf4: the scene is open */
extern unsigned short logoFailed; /* @data 0x4b7cf6 */
extern unsigned long lastClockRead; /* @data 0x4b7efc: when the clock was read (ticks) */
extern short highestTownCel; /* @data 0x4b7e10 */
extern short townsfolkViews[20]; /* @data 0x4b7ece: views (negated once notified, townsfolkNotify) */
extern short townPartySize; /* @data 0x4b7f02: party views to set running (setTownRunning) */
extern short townsfolkGone; /* @data 0x4b7f10: views notified (townsfolkNotify) */

extern long townButtonResource; /* @data 0x4a74c4 */
extern ImageBank *townButtonImages; /* @data 0x4a74c8: the buttons' images */
extern char monumentScripts[]; /* @data 0x4a7582 */
extern long townFile; /* @data 0x4b7dfc: Town.MHK */
extern short townOpen; /* @data 0x4b7e00: scene 6 is open */
extern ShortRect recordHotspots[16]; /* @data 0x4b7e12: hotspots */
extern short recordHotspotNumbers[16]; /* @data 0x4b7e92: their numbers */
extern short recordHotspotCount; /* @data 0x4b7eb2: how many */
extern short onRecordHotspot; /* @data 0x4b7eb4: the cursor is on one */
extern short hotspotRecord; /* @data 0x4b7eb6: its number (from 1) */
extern short hotspotScript; /* @data 0x4b7eb8: a script for it */

extern Camp *townSlots; /* @data 0x4b7e04: the town's Zoombinis */
extern short townViews[4]; /* @data 0x4b7e08 */
extern short cheatPlaque; /* @data 0x4b7eba */
extern unsigned long townSoundEnded; /* @data 0x4b7ec0 */
extern short townSound; /* @data 0x4b7ec4 */
extern short townSoundWasGreeting; /* @data 0x4b7ec6 */
extern short townSoundPlaying; /* @data 0x4b7ec8 */
extern short draggingInTown; /* @data 0x4b7eca */
extern short townFull; /* @data 0x4b7ecc */
extern short clockWinds; /* @data 0x4b7ef6 */
extern short clockShown; /* @data 0x4b7ef8 */
extern short townFidgetsLeft; /* @data 0x4b7f00 */
extern unsigned long lastTownFidgetTime; /* @data 0x4b7f04 */
extern unsigned long townFidgetInterval; /* @data 0x4b7f08 */
extern unsigned long townFidgetersUsed; /* @data 0x4b7f0c */
extern short townspeopleToAdd; /* @data 0x4b7f12: townspeople still to add */

extern Point recordHotspotPoints[16]; /* @data 0x4a74de: the groups' hotspots (placeRecordHotspots) */
extern unsigned char windStartMinute; /* @data 0x4b7f14: the clock's minute hand when winding started */
extern unsigned char windStartHour; /* @data 0x4b7f15: and hour hand */

extern short inIntroFrame; /* @data 0x4a7412: introFrame is running */
extern char logoPath[]; /* @data 0x4b7cfa */

extern char monumentBuildings[16]; /* @data 0x4a536c: the building for each record */
extern char *monumentTexts[16]; /* @data 0x4a537c: "this monument was made to honor the zoombinis who:", ... */
extern char *featTexts[16]; /* @data 0x4a53bc: by group and level: "ambled past allergic cliffs, ...", ... */
extern short plaqueLines[6]; /* @data 0x4a7594: the plaque's lines' tops */


extern char nextCheatRecord; /* @data 0x4a7592: the next record the . key makes */

/* Which of the town's six screens is shown (0-5). */
inline short &townScreen()
{
    return *(short *)(gameState + 0x1e);
}

/* Zoombiniville's population. */
inline short &population()
{
    return *(short *)(gameState + 0x4e);
}

extern GroupList townGroups6[1]; /* @data 0x4a74a4 */
extern unsigned long townSoundPause; /* @data 0x4b7ebc */

extern short townSounds[5]; /* @data 0x4a74cc: sounds for the town (townSoundsUsed picks) */
extern unsigned long townSoundsUsed; /* @data 0x4a74d8: slots used (allocateSlot) */
extern short inTownFrame; /* @data 0x4a7580: townFrame is running */

void openIntro();
void townFrame();
void openTown();
void townClicked(short which);
short townKey(unsigned short key);
void addTownsperson();
void setTownFrames(short frame);
void drawPlaque(View *view);
void introFrame();
void drawClock(View *view);
void placeRecordHotspots(View *view);
short introKey(unsigned short key);
void resetTown();
void drawTownButtons(View *);
void settleTravellers();
void closeIntro();
void scrollTown(short left);
void drawTownButton(short which, short lit, short show);
void closeTown();
void findTownHotspot(Point *where);
void introClicked(short which);
void readClock();
void setTownRunning(short running);
void updateTownButton(View *view, short region);
short townScript();
void placeTownCels(View *view);
void townsfolkNotify(View *view, short event);

extern InputItem g_4a73bc[1]; /* pointed to by initialised data */
extern Group g_4a73e0[1]; /* pointed to by initialised data */
extern Scene g_4a73fc[1]; /* pointed to by initialised data */
extern Group g_4a7494[1]; /* pointed to by initialised data */
extern Scene g_4a74b0[1]; /* pointed to by initialised data */

#endif
