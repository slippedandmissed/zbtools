/*
 * picker's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PICKER_H
#define PICKER_H

/* A place to click on the picker's screens (scenes 19 and 21). */
struct PickerHotspot
{
    ShortRect rect; /* 40 by 30 around its point */
    char unknown8[28];
};

/* The picker's data from 0x4af8ac, laid out differently by its scenes. */
struct PickerData
{
    union {
        PickerHotspot hotspots[17]; /* scene 1; the last is the whole screen */
        struct
        {
            short caught; /* the view of the Zoombini just caught (caughtNotify deletes it) */
            short overView; /* +2: shown when the throws run out */
            short againView; /* +4: its "again" button */
            short streak; /* +6: catches in a row */
            char unknown8[0x1c];
            short throws; /* +0x24: left */
            short count; /* +0x26: caught; the most is kept in the roster (+0x22) */
            short remaining; /* +0x28: of 99, neither caught nor to throw */
            short speed; /* +0x2a: the walking Zoombinis' interval (sendRandomZoombini) */
            char unknown2c[0x1c];
            ShortRect leave; /* +0x48: click here to go back to the map */
            char unknown50[0xac];
            ShortRect again; /* +0xfc: the "again" button's bounds */
        } game; /* scene 19: catching Zoombinis */
    };
};

extern PickerData pickerData; /* @data 0x4af8ac */

void showMapBox();
void placePressed(View *view);
long catchKey(long);
void burstNotify(View *, short event);
extern long g_4afb10; /* @data 0x4afb10: Picker.MHK */
extern short g_4afb14; /* @data 0x4afb14: the scene is open */
extern GroupList pickerGroups[1]; /* @data 0x4a1f34 */
extern GroupList catchGroups[1]; /* @data 0x4a2046 */
extern GroupList targetGroups[1]; /* @data 0x4a2090 */
extern short g_4afbb8; /* @data 0x4afbb8 */
extern basePort **g_4afb28; /* @data 0x4afb28 */
extern short mapBoxView; /* @data 0x4afb34 */
extern short g_4afb3a; /* @data 0x4afb3a */
extern ShortRect g_4a1f74; /* @data 0x4a1f74 */
void closeScene19();
void closeScene20();
void caughtNotify(View *, short event);
extern short shipView; /* @data 0x4afb7c */
extern short shipDirection; /* @data 0x4afb7e */
extern short g_4afb80; /* @data 0x4afb80 */
extern short g_4afb82; /* @data 0x4afb82 */
extern short g_4afb84; /* @data 0x4afb84 */
extern short g_4afb88; /* @data 0x4afb88 */
void placeLevelMarker(View *view);
void driftView(View *view);
void resetShip();
void drawTextView(View *view);

void placeCatchScore(View *view);
extern short g_4afb16; /* @data 0x4afb16 */
extern MapSave *g_4afb18[6]; /* @data 0x4afb18 */
extern short g_4afb36; /* @data 0x4afb36 */
extern short levelListView; /* @data 0x4afb38 */
extern short g_4afb3c; /* @data 0x4afb3c */
extern short g_4afb3e; /* @data 0x4afb3e */
extern short g_4afb40; /* @data 0x4afb40 */
extern ShortRect g_4afb42; /* @data 0x4afb42 */
extern short pickedHotspot; /* @data 0x4afb5c */
extern short g_4afb5e; /* @data 0x4afb5e */
void resetMap();
void openScene1();
void openScene19();
void openScene20();
extern short targetScore; /* @data 0x4afb72 */
extern short shipsLeft; /* @data 0x4afb76 */
void pickHotspot(short n);
void placeTargetScore(View *view);
extern short shotsStarted; /* @data 0x4afb8c: drifting views started */
short fireShot();
extern char openHotspots[17]; /* @data 0x4afb4a: the hotspots open (from 1) */
void placeOpenHotspots(View *view);
void placeHotspotLevels(View *view);
void findOpenHotspots(char *open);
void drawLevelList(ShortRect *rect);
void targetsClicked(short which);
void updateTextView(View *view, volatile short region);
void drawLevelListView(View *view);
extern ShortRect g_4a1f54[6]; /* @data 0x4a1f54: the map's areas saved (g_4afb18); the first four hold the terrains' names */
void drawTerrainNames();
extern short g_4afb74; /* @data 0x4afb74: the next hundred to score */
extern short g_4afb78; /* @data 0x4afb78 */
extern short firstShotStopped; /* @data 0x4afb8a: the first shot stopped */
extern short targetHit; /* @data 0x4afb8e: the target hit (from 1) */
extern ShortRect *targetBounds[6]; /* @data 0x4afb94: the targets' bounds */
extern short g_4afbac[6]; /* @data 0x4afbac: the targets' views */
void placeShot(View *view);

short sendRandomZoombini();
extern short g_4afb6c; /* @data 0x4afb6c */
extern short g_4afb6e; /* @data 0x4afb6e */
extern short g_4afb70; /* @data 0x4afb70 */
extern short g_4afbba; /* @data 0x4afbba: targets started */
short startTarget(short kind, short preset);
extern char savedUserFile[]; /* @data 0x4a1f84: the user file while practising (in ZBtemp) */
void closeScene1();
extern short g_4afb7a; /* @data 0x4afb7a */
void placeShip(View *view);

void updateCursorView(View *view, short region);
extern unsigned short g_4afbbc; /* @data 0x4afbbc: a big target is out */
extern unsigned short g_4afbbe; /* @data 0x4afbbe: its view */
short scene20Key(unsigned short key);
extern char *placeNames[16]; /* @data 0x4a5278: the hotspots' names ("zoombini isle", ...) */
/* The map's box: 0-3 the camps ("zoombini isle", "shelter rock", "shade
   tree", "zoombiniville"), 4 "practice mode", then from 5, 9, 13 and 17 how
   to get back to the game from each level. */
extern char *mapTexts[21]; /* @data 0x4a52b8 */
void drawMapBox(ShortRect *rect);
void drawMapBoxView(View *view);
void makeMapViews(short update);
extern short g_4a2008; /* @data 0x4a2008: scene1Frame is running */
extern short g_4a2066; /* @data 0x4a2066: scene19Frame is running */
extern short g_4afb60[3]; /* @data 0x4afb60: the Zoombinis crossing */
extern unsigned long g_4afb68; /* @data 0x4afb68: when to send more */
void scene19Frame();
void scene1Frame();
extern short g_4a20b0; /* @data 0x4a20b0: scene20Frame is running */
extern short targetBursting; /* @data 0x4afb90: the target hit bursting (negated until it's done) */
void scene20Frame();

void leavePractice();
extern basePort **g_4afb2c; /* @data 0x4afb2c */
extern ShortRect g_4a1f7c; /* @data 0x4a1f7c */
short scene1Key(unsigned short key);
extern ShortRect g_4a2068[3]; /* @data 0x4a2068: where a click catches nothing */
void scene19Clicked(short);
extern ShortRect g_4a1fa8[4]; /* @data 0x4a1fa8: the levels' lines in the list */
void scene1Clicked(short which);

#endif
