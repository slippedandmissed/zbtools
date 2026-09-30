/*
 * roster (0x41c09c-0x41f8cc): Saved players: 'ZBUser', 'Could not Open/Create Roster file.', 'Zoombini.who'
 */

#include <stdio.h>
#include <string.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "jointext.h"
#include "loading.h"
#include "mainloop.h"
#include "net.h"
#include "platform.h"
#include "roster.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

Scene g_4a0fbc[1] = {{openCaves, closeCaves, cavesFrame, 0, cavesKey}};
long cavesButtonResource = 0;
long glyphShape = 0;
short unusedCaves2 = 0;
short cavesFullPartyAtOpen = 0;
short cavesGoReady = 0;
short cavesFullParty = 0;
short cavesOpen = 0;
short cavesBusy = 0;
short caveValueCount = 5;
short caveFeatureCount = 0;
short unusedCaves1 = 0;
short walkerFrontView = 0;
short walkScriptsBase = 0;
short walkScript = 0;
short firstFrame = 0;
short finalFrame = 0;
short currentFrame = 0;
short walkOnDue = 0;
short unusedCaves3 = 0;
short unusedCaves4 = 0;
short firstCave = 0;
short cavesPlacedCount = 0;
short droppedCave = 0;
short assignedCave = 0;
short chosenCount = 0;
short missingSnoids = 0;
short forceHairFirst = 1;
short featureTableShown = 0;
short glyphView = 0;
ImageBank *cavesButtonImages = 0;
SceneButton cavesButtons[3] = {
    {{589, 392, 628, 429}}, {{589, 430, 628, 467}}, {{0, 0, 640, 480}},
};
Group g_4a1090[1] = {{g_4a0766, (InputItem *)cavesButtons, 3, 0x2068}};
GroupList caveGroups[1] = {{g_4a1090, 1, 0, cavesClicked}};
Point cavePoints[20] = {
    {254, 140}, {296, 148}, {340, 146}, {373, 163}, {364, 187}, {337, 212}, {316, 234}, {301, 263},
    {314, 292}, {346, 311}, {388, 316}, {429, 301}, {458, 281}, {482, 261}, {521, 247}, {556, 263},
    {567, 290}, {543, 314}, {529, 342}, {554, 359},
};
ShortRect caveWaitAreas[12] = {
    {0}, {0, 0, 195, 130}, {0, 128, 175, 147}, {0, 146, 155, 165}, {0, 164, 135, 191},
    {0, 190, 120, 214}, {0, 213, 100, 236}, {0, 235, 87, 250}, {0, 249, 67, 269}, {0, 268, 40, 289},
    {0, 288, 27, 357}, {0, 356, 36, 394},
};
Point chosenSpots[20] = {
    {180, 110}, {160, 136}, {130, 167}, {106, 193}, {86, 232}, {140, 100}, {120, 126}, {100, 157},
    {76, 183}, {46, 222}, {100, 90}, {80, 116}, {60, 147}, {36, 173}, {60, 80}, {40, 106},
    {20, 137}, {10, 167}, {20, 90}, {20, 116},
};
ShortRect glyphArea = {314, 24, 436, 102};
short caveSnoidF1[21] = {
    0, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1,
};
short caveSnoidF2[21] = {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};
short inCavesFrame = 0;
short cavesButton2Lit = 0;
short cavesButton1Drawn = 0;
short kind4Images[6] = {0, 383, 493, 671, 721, 823};
short kind3Images[6] = {0, 343, 351, 359, 367, 375};
short kind2Images[6] = {0, 183, 215, 248, 279, 312};
short kind1Images[6] = {0, 23, 55, 87, 119, 151};
unsigned long tunnelRemarks0Used = 0;
unsigned long tunnelRemarksUnused = 0;
unsigned long tunnelRemarks2Used = 0;
unsigned long tunnelRemarks3bUsed = 0;
unsigned long tunnelRemarks3cUsed = 0;
unsigned long tunnelRemarks3aUsed = 0;

long cavesFile;
short glyphPlaced[11];
short glyphImages[11];
short blinkingGlyph;
short lastBlinkedGlyph;
short exitDue;
short exitStage;
short walkerView;
short walkDue;
short cavesLevel;
short caveFeatures[2];
short caveValues[2][5];
short caveValueCounts[6][6];
short walkFromView;
short walkToView;
Point claimedSpotPoint;
Point *walkerAnchor;
Point *cheerAnchor;
short spotSnoids[21];
short cavePlaceValues[2][21];
short caveViews[21];
short framesChanged;
short cheerQueue[20];
short cheerQueueCount;
short cavesNextWalker;
short frameAnchorViews[22];
short cavesView6000;
short cavesView6001;
short cavesView6002;
short frameView;
unsigned long lastCheerTime;
unsigned long cheerersUsed;
short cheersAllowed;
short cheersDone;
short allPlaced;
long unusedCaves5;
Point walkBackPoints[20];
short walkBackCount;
short *caveRegs200;
short *glyphRaise;
long caveRegsResources[2];
short caveRegsHandles[2];
long rosterFile;
char *rosterError;
short clickTime;

/*
 * The notify of the Zoombinis cheering on the roster screen (cavesFrame),
 * which returns its event plus one (ignored).
 * The original adds one with `sub eax, -1`; BCC32 turns every way of writing
 * it tried so far (+ 1, - -1, enums, consts, unsigned, compound assignment,
 * locals, other -O options, and Borland C++ 4.52 as well as 4.5) into `inc eax`.
 */
/* @zoombi32 0x0041d3e6 */
int cheerNotify(View *, short value)
{
    return value + 1;
}

/* @zoombi32 0x0041d9e4 */
void cavesNoDraw(View *)
{
}

/* @zoombi32 0x0041d9eb */
void cavesNoUpdate(View *, short)
{
}

/* Loads the roster's two REGS resources (200-201: big-endian words,
   swapped here, locked in caveRegs200 and glyphRaise) and its hieroglyphs
   (shape 10000). */
/* @zoombi32 0x0041dbce */
void loadCaveResources()
{
    short i;
    unsigned short *data;
    unsigned long size;

    for (i = 0; i < 2; i++) {
        caveRegsResources[i] = 0;
        loadResourceAs(&caveRegsResources[i], RESOURCE_TYPE('R', 'E', 'G', 'S'), i + 200, 0, 1);
        caveRegsHandles[i] = usedResourceHandle(caveRegsResources[i]);
        switch (i) {
        case 0:
            caveRegs200 = (short *)lockHandle(caveRegsHandles[i]);
            break;
        case 1:
            glyphRaise = (short *)lockHandle(caveRegsHandles[i]);
            break;
        }
        data = (unsigned short *)handleData(caveRegsHandles[i]);
        for (size = handleSize(caveRegsHandles[i]); size; size -= 2) {
            *data = swapShort(*data);
            data++;
        }
    }
    loadShape(&glyphShape, 10000, "Hieroglyphs");
}

/* Frees the resource glyphShape, if loaded. */
/* @zoombi32 0x0041dccb */
void freeGlyphShape()
{
    if (glyphShape) {
        freeResource(&glyphShape);
        glyphShape = 0;
    }
}

/* A view's update: redraws glyphArea when the view asks to (reset). */
/* @zoombi32 0x0041dbab */
void updateGlyphArea(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &glyphArea);
    }
}

/* Puts view `id` in placed spot n (1-20), noting the spot's point in
   claimedSpotPoint. */
/* @zoombi32 0x0041e8f3 */
void claimSpot(short id, short n)
{
    if (n > 0 && n < 21) {
        placedViewPoint(&claimedSpotPoint, n);
        claimPlacedView(n, id);
    }
}

/* A new saved game's file name: "ZOOM" and the next number (*nextId,
   four digits); the game's name isn't used. */
/* @zoombi32 0x0041f514 */
void newSaveFileName(const char *, char *file, short *nextId)
{
    file[0] = 'Z';
    file[1] = 'O';
    file[2] = 'O';
    file[3] = 'M';
    sprintf(file + 4, "%04d", (*nextId)++);
    file[8] = 0;
}

/* Reports a roster error. */
/* @zoombi32 0x0041f195 */
void reportRosterError(const char *message)
{
    char none[48] = "";

    joinText(&rosterError, message, none);
    reportJoinedError(rosterError);
    freeText((void **)&rosterError);
}

/* Starts the view walkerView's Snoid on script `script` (by walkerAnchor,
   idleTicks `f8`), in group `group`, with notify `notify` if given. */
/* @zoombi32 0x0041d167 */
void startWalkerScript(short group, short script, ViewNotify notify, char f8)
{
    View *view = findView(walkerView);

    if (view) {
        startSnoidScript((Snoid *)&view->body, script, walkerAnchor, f8);
        view->body.group = group;
        if (notify)
            view->notify = notify;
    }
}

/* Shows frame n (up to finalFrame) of the view frameView (script firstFrame
   on), if it's not running, with notify frameNotify. */
/* @zoombi32 0x0041dd37 */
void showFrame(volatile short n)
{
    View *view = findView(frameView);

    if (view && !view->body.running && n <= finalFrame) {
        setViewScript(view, firstFrame + n, 1);
        view->notify = frameNotify;
    }
}

/* Releases the two locked handles (caveRegsHandles) and their resources
   (caveRegsResources). */
/* @zoombi32 0x0041dce6 */
void freeCaveResources()
{
    short i;

    for (i = 0; i < 2; i++)
        if (caveRegsHandles[i]) {
            unlockHandle(caveRegsHandles[i]);
            freeResource(&caveRegsResources[i]);
            caveRegsHandles[i] = 0;
            caveRegsResources[i] = 0;
        }
}

/* A view's update: redraws button 2 when cavesGoReady changes, and button 1
   once. */
/* @zoombi32 0x0041d972 */
void updateCavesButtons(View *, short region)
{
    if (cavesGoReady) {
        if (!cavesButton2Lit) {
            cavesButton2Lit = 1;
            unionRgnRect(region, &cavesButtons[1].rect);
        }
    } else if (cavesButton2Lit) {
        cavesButton2Lit = 0;
        unionRgnRect(region, &cavesButtons[1].rect);
    }
    if (!cavesButton1Drawn) {
        cavesButton1Drawn = 1;
        unionRgnRect(region, &cavesButtons[0].rect);
    }
}

/* Applies the player's settings from the game state: click time (stored
   big-endian), sound and music, the drag options, debugging messages,
   and the scenes. */
/* @zoombi32 0x0041f668 */
void applyPlayerSettings()
{
    clickTime = swapShort(*(unsigned short *)(gameState + 2));
    soundOn = gameState[4];
    musicOn = gameState[5];
    clickToDragOption = gameState[6];
    hideDragCursor = gameState[7];
    debugMessagesOn = gameState[8];
    dragClicks = gameState[9];
    transitionsOn = *(short *)(gameState + 0xa);
    sceneDue = *(short *)(gameState + 0xcc);
    journeyFrom = *(short *)(gameState + 0xca);
}

/* Opens the roster file `path` (mode `mode`) as rosterFile, making its
   directory first if need be: 0 if it opened, 1 if it did after making
   the directory, 2 if it didn't. */
/* @zoombi32 0x0041f100 */
short openRosterFile(const char *path, short mode)
{
    fileSpec spec(path);
    short result = 0;

    rosterFile = openFile(&spec, mode);
    if (!rosterFile) {
        createPath(spec, 0);
        rosterFile = openFile(&spec, mode);
        if (!rosterFile)
            result = 2;
        else
            result = 1;
    }
    return result;
}

/* Draws button `which` (1: 5, 2: 2, or 1 if cavesGoReady isn't set; the next
   image if lit) from the bank cavesButtonImages, showing it if `show`. */
/* @zoombi32 0x0041d8bc */
void drawCavesButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!cavesGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(cavesButtonImages->offsets[image] + (char *)cavesButtonImages), cavesButtons[which - 1].rect.left,
                      cavesButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&cavesButtons[which - 1].rect);
    }
}

/* Draws feature n of kind `kind` (1-4: a table each; kind 1 five pixels
   further up and left) from the Zoombini images at `rect`, and shows it. */
/* @zoombi32 0x0041ed59 */
void drawFeatureImage(short kind, short n, ShortRect rect)
{
    short handle;
    short image;
    ImageBank *bank;

    switch (kind) {
    case 1:
        rect.left += -5;
        rect.right += -5;
        image = kind1Images[n];
        break;
    case 2:
        image = kind2Images[n];
        break;
    case 3:
        image = kind3Images[n];
        break;
    case 4:
        image = kind4Images[n];
        break;
    }
    handle = usedResourceHandle(snoidImagesResource);
    lockHandle(handle);
    bank = (ImageBank *)handleData(handle);
    drawImageData((unsigned short *)((char *)bank + bank->offsets[image]), rect.left, rect.top, 8);
    unlockHandle(handle);
    showRect(&rect);
}

/* Sends up to three of the placed Zoombinis (spotSnoids, from the 20th)
   that are ready off to (x, y), `interval` apart. */
/* @zoombi32 0x0041d80e */
void sendReadyOff(short x, short y, long interval)
{
    unsigned long when;
    short count;
    short i;
    View *view;

    count = 0;
    when = clockTime();
    for (i = 20; i > 0 && count < 3; i--) {
        view = findView(spotSnoids[i]);
        if (view) {
            view->flags = 1;
            Snoid *snoid = (Snoid *)&view->body;

            if (snoid->chosen) {
                *(Point *)&snoid->body.waypointX = *(Point *)&snoid->body.x;
                snoid->targetX = x;
                snoid->targetY = y;
                setSnoidAction(snoid, 10, 0);
                view->nextUpdate = when;
                when += interval;
                count++;
            }
        }
    }
    sortViews();
    snoidsOnTheirWay = 1;
    snoidsArrived = 0;
}

/* The view drawing the two buttons. */
/* @zoombi32 0x0041d955 */
void drawCavesButtons(View *)
{
    drawCavesButton(1, 0, 0);
    drawCavesButton(2, 0, 0);
}

/* Opens scene 16, the caves (Caves.MHK): the state, the scripts and
   sounds, the views for 20 places (the party's, and the cave's rows), the
   roster's resources, and a line by the level. */
/* @zoombi32 0x0041c09c */
void openCaves()
{
    short i;
    View *view;

    resetCavesState(cavesLevel = sceneLevel() + 1);
    cavesOpen = 0;
    sceneDue = 0;
    cavesGoReady = 0;
    walkOnDue = 0;
    unusedCaves3 = 0;
    unusedCaves4 = 0;
    hintSound = 0;
    lastCheerTime = 0;
    cheerersUsed = 0;
    cheersAllowed = 0;
    cheersDone = 0;
    allPlaced = 0;
    unusedCaves5 = 0;
    cheerQueueCount = 0;
    framesChanged = 0;
    cheerAnchor = 0;
    fillMemory(frameAnchorViews, 0, 44);
    fillMemory(cheerQueue, 0, 40);
    openGameFile(&cavesFile, "Caves.MHK");
    setCurrentMap(cavesFile);
    cavesButtonImages = loadImageBank(11000, &cavesButtonResource);
    loadTerrain(100);
    loadPaths(1000);
    drawBackdrop(5000);
    loadFeatureGroup(6000, 0, 0);
    loadFeatureGroup(9000, 1, 0);
    loadFeatureGroup(7000, 2, 0);
    walkScript = 1;
    walkScriptsBase = walkScript * 200 + 8000;
    walkScript = walkScript * 4 + 12000;
    loadFeatureGroup(walkScriptsBase, 3, 0);
    loadFeatureGroup(9025, 4, 0);
    loadScripts(6000, 13);
    addScripts(9000, 20, 0);
    addScripts(7000, 20, 0);
    addScripts(walkScriptsBase, 80, 0);
    addScripts(9025, 4, 0);
    loadSnoidScripts(12000, 14, 0);
    addSnoidScripts(13000, 5, 0);
    copyPaletteRange(10, 236);
    cavesView6000 = addView(0x4088000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
    cavesView6001 = addView(0x4088000, drawCels, runViewScript, 6001, 6, 0, 0, 0);
    cavesView6002 = addView(0x4188000, drawCels, runViewScript, 6002, 8, 0, 0, 0);
    setViewPlaces(20, chosenSpots, 1);
    makePartySnoids(0);
    chosenCount = listChosenSnoids()->count;
    cheersAllowed = chosenCount - 1;
    missingSnoids = 20 - chosenCount;
    if (missingSnoids > 4)
        missingSnoids = 4;
    if (missingSnoids)
        caveViews[0] = addView(0x4108000, drawCels, runViewScript, missingSnoids + 9024, 6, 0, 0, 0);
    firstCave = 21 - chosenCount;
    {
        short rows = firstCave;

        if (rows > 5)
            ; /* the original tests this and does nothing */
    }
    for (i = 0; i < 4; i++)
        placedViews[i] = addView(0x508a000, drawCels, runViewScript, i + 7000, 7, &cavePoints[i], 0, 0);
    for (i = 5; i < 12; i++) {
        caveViews[i] = addView(0x4108000, drawCels, runViewScript, i + 8999, 6, 0, 0, 0);
        frameAnchorViews[i] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
        placedViews[i - 1] = addView(0x508a000, drawCels, runViewScript, i + 6999, 7, &cavePoints[i - 1], 0, 0);
    }
    caveViews[15] = addView(0x4108000, drawCels, runViewScript, 9014, 6, 0, 0, 0);
    frameAnchorViews[15] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    caveViews[14] = addView(0x4108000, drawCels, runViewScript, 9013, 6, 0, 0, 0);
    frameAnchorViews[14] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    caveViews[13] = addView(0x4108000, drawCels, runViewScript, 9012, 6, 0, 0, 0);
    frameAnchorViews[13] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    caveViews[12] = addView(0x4108000, drawCels, runViewScript, 9011, 6, 0, 0, 0);
    frameAnchorViews[12] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    placedViews[11] = addView(0x508a000, drawCels, runViewScript, 7011, 7, &cavePoints[11], 0, 0);
    placedViews[12] = addView(0x508a000, drawCels, runViewScript, 7012, 7, &cavePoints[12], 0, 0);
    placedViews[13] = addView(0x508a000, drawCels, runViewScript, 7013, 7, &cavePoints[13], 0, 0);
    placedViews[14] = addView(0x508a000, drawCels, runViewScript, 7014, 7, &cavePoints[14], 0, 0);
    moveView(placedViews[11], 1, frameAnchorViews[12]);
    moveView(placedViews[12], 1, frameAnchorViews[13]);
    moveView(placedViews[13], 1, frameAnchorViews[14]);
    moveView(placedViews[14], 1, frameAnchorViews[15]);
    for (i = 16; i < 21; i++) {
        caveViews[i] = addView(0x4108000, drawCels, runViewScript, i + 8999, 6, 0, 0, 0);
        frameAnchorViews[i] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
        placedViews[i - 1] = addView(0x508a000, drawCels, runViewScript, i + 6999, 7, &cavePoints[i - 1], 0, 0);
    }
    frameAnchorViews[21] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    for (i = 0; i < missingSnoids; i++)
        placeClaims[i] = caveViews[0];
    frameView = addView(0x4000000, drawCels, runViewScript, 6012, 0, 0, 0, 0);
    frameView = addView(0x8180000, drawCels, runViewScript, firstFrame + 1, 9, 0, 0, 0);
    fadeOutViews();
    copyPaletteRange(10, 236);
    loadCaveResources();
    setUpCaves();
    placeGlyphs(cavesLevel);
    glyphView = addView(0x8000, drawGlyphs, updateGlyphArea, 0, 0, 0, 0, 0);
    addView(0x1000, drawCavesButtons, updateCavesButtons, 0, 0, 0, 0, 0);
    enterSnoids(0);
    updateViews();
    staggerSnoids(45, 30);
    cavesFullPartyAtOpen = cavesFullParty = countChosenSnoids() >= 20;
    setGroupLists(caveGroups, 1, (short)0xc000);
    drawCavesButton(1, 0, 0);
    drawCavesButton(2, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    view = findView(glyphView);
    if (view)
        view->nextUpdate = clockTime() + 120;
    cavesOpen = 1;
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(6006, 6006, 0);
    addSoundRange(6001, 6001, 0);
    addSoundRange(6008, 6008, 0);
    addSoundRange(6000, 6000, 0);
    addSoundRange(6005, 6005, 0);
    addSoundRange(6004, 6004, 0);
    addSoundRange(6003, 6003, 0);
    addSoundRange(6007, 6007, 0);
    addSoundRange(6002, 6002, 0);
    addSoundRange(8200, 12001, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(600, 799, 0);
    if (cavesLevel < 4)
        queueViewSound(sceneLevel() + 30025, 0);
    campHint((short *)(gameState + 0x40));
    hintSound = 20065;
}

/* Closes the roster screen. */
/* @zoombi32 0x0041c9ed */
void closeCaves()
{
    if (cavesOpen) {
        cavesOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        freeGlyphShape();
        freeCaveResources();
        freeResource(&cavesButtonResource);
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&cavesFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Picks the roster's features: one (two when cavesLevel is above 2) of the
   four at random, the first hair (2) when forceHairFirst asks, and for each
   caveValueCount different values (1-5) at random. */
/* @zoombi32 0x0041e0f3 */
void pickCaveFeatures()
{
    short left;
    volatile short unused; /* set, never read: volatile keeps the store */
    short features[4];
    short values[7];
    short i;
    short j;
    short k;
    short valuesLeft;

    if (cavesLevel > 2)
        caveFeatureCount = 2;
    else
        caveFeatureCount = 1;
    for (i = 0; i < 2; i++)
        caveFeatures[i] = 0;
    for (j = 0; j < caveFeatureCount; j++)
        for (i = 0; i < caveValueCount; i++)
            caveValues[j][i] = 0;
    left = 3;
    for (i = 0; i < 4; i++)
        features[i] = i;
    unused = 0;
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 7; i++)
            values[i] = i;
        if (j == 0) {
            k = randomBetween(0, left);
            if (forceHairFirst) {
                k = 2;
                forceHairFirst = 0;
            }
            caveFeatures[0] = features[k];
            for (; k < left + 1; k++)
                features[k] = features[k + 1];
            left--;
        } else {
            caveFeatures[1] = features[randomBetween(0, left)];
        }
        valuesLeft = 5;
        for (i = 0; i < caveValueCount; i++) {
            k = randomBetween(1, valuesLeft);
            caveValues[j][i] = values[k];
            for (; k < valuesLeft + 1; k++)
                values[k] = values[k + 1];
            valuesLeft--;
        }
    }
}

/* Counts the chosen Zoombinis (chosenCount of them) by feature
   caveFeatures[0], and when there's a second (cavesLevel above 2), by both
   it and caveFeatures[1], into
   caveValueCounts. */
/* @zoombi32 0x0041e273 */
void countByCaveFeatures()
{
    short second;
    ChosenSnoids *chosen = listChosenSnoids();
    short i;
    short j;
    short first;

    if (cavesLevel > 2)
        caveFeatureCount = 2;
    else
        caveFeatureCount = 1;
    for (i = 0; i < 6; i++)
        for (j = 0; j < 6; j++)
            caveValueCounts[i][j] = 0;
    for (j = 0; j < chosenCount; j++) {
        first = chosen->features[j][caveFeatures[0]];
        caveValueCounts[0][first]++;
        if (caveFeatureCount > 1) {
            second = chosen->features[j][caveFeatures[1]];
            caveValueCounts[second][first]++;
        }
    }
}

/* The notify of the roster's walking Zoombini (walkerView): 1 and 2 start
   the scripts walkScript and the next (in its group); 4 moves on to the next frame
   (showFrame) and deletes the view walkerFrontView; 5 sets walkOnDue; 10 walks
   back to the last point pushed on walkBackPoints (script 12013); 20 and 21 as
   in frameNotify. */
/* @zoombi32 0x0041d1b1 */
void walkerNotify(View *view, short event)
{
    switch (event) {
    case 1:
        startWalkerScript(view->body.group, walkScript, walkerNotify, 1);
        break;
    case 5:
        walkOnDue = 1;
        break;
    case 2:
        startWalkerScript(view->body.group, walkScript + 1, walkerNotify, 0);
        break;
    case 4:
        requestViewSort();
        framesChanged = 1;
        currentFrame++;
        showFrame(currentFrame);
        deleteView(walkerFrontView);
        break;
    case 10:
        walkBackCount--;
        walkerAnchor = &walkBackPoints[walkBackCount];
        startWalkerScript(view->body.group, 12013, walkerNotify, 0);
        break;
    case 20:
        walkerView = 0;
        if (currentFrame == finalFrame)
            cavesBusy = 1;
        else
            cavesBusy = 0;
        requestViewSort();
        break;
    case 21:
        walkerView = 0;
        cavesBusy = 1;
        break;
    }
}

/* The notify of the view frameView's frames (showFrame): 10 sets exitStage
   to 2; 20 notes the view done and, on its last frame, remarks (now and
   then) if not all are chosen, and sets cavesBusy; 21 likewise, ending the
   walk when practiceLevel. */
/* @zoombi32 0x0041d30b */
void frameNotify(View *, short event)
{
    switch (event) {
    case 10:
        exitStage = 2;
        break;
    case 20:
        walkerView = 0;
        if (currentFrame == finalFrame) {
            if (countChosenSnoids() < chosenCount) {
                if (randomBetween(0, 4) > cavesLevel - 1 || (*(short *)(gameState + 0x40) & 0xfff) <= 3)
                    queueViewSound(randomBetween(20045, 20048), 0);
            }
            cavesBusy = 1;
        } else {
            cavesBusy = 0;
        }
        break;
    case 21:
        walkerView = 0;
        cavesBusy = 1;
        if (practiceLevel) {
            snoidsOnTheirWay = 0;
            snoidsArrived = 1;
        }
        break;
    }
}

/* Draws image `image` (from the resource glyphShape; big-endian offsets
   and sizes) at place `which` (1-10), centred across it and raised by
   glyphRaise. */
/* @zoombi32 0x0041d9f2 */
void drawGlyph(short which, short image, long)
{
    short xs[11] = {0, 326, 348, 375, 397, 423, 324, 347, 373, 395, 422};
    short ys[11] = {0, 36, 39, 42, 44, 46, 77, 80, 83, 86, 90};
    short handle;
    ImageBank *bank;
    unsigned short *data;
    short x;
    short y;

    handle = usedResourceHandle(glyphShape);
    lockHandle(handle);
    bank = (ImageBank *)handleData(handle);
    data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);
    x = xs[which] - swapShort(data[0]) / 2;
    y = ys[which] - glyphRaise[which];
    drawImageData(data, x, y, 8);
    unlockHandle(handle);
}

/* Draws the images placed (glyphPlaced) at each of the ten places,
   after adding glyphArea to the region to redraw (`unused` is passed on to
   drawGlyph, which ignores it). */
/* @zoombi32 0x0041db60 */
void redrawGlyphs(long unused)
{
    short i;

    unionRgnRect(removedRgn, &glyphArea);
    for (i = 1; i < 11; i++)
        if (glyphPlaced[i])
            drawGlyph(i, glyphImages[i], unused);
}

/* Draws the images placed at the ten places, except at place blinkingGlyph
   when cavesLevel is 1 and it's one of the first five. */
/* @zoombi32 0x0041dadf */
void drawGlyphs(View *)
{
    short i;

    if (cavesLevel == 1 && blinkingGlyph < 6) {
        for (i = 1; i < 11; i++)
            if (glyphPlaced[i] && i != blinkingGlyph)
                drawGlyph(i, glyphImages[i], 0);
    } else {
        for (i = 1; i < 11; i++)
            if (glyphPlaced[i])
                drawGlyph(i, glyphImages[i], 0);
    }
}

/* Resets the screen for cavesLevel (resetCavesState) and shows the frame before
   the first (or the first, 6003) of the view frameView if it's not
   running. */
/* @zoombi32 0x0041eaf1 */
void resetCavesScreen()
{
    View *view;

    resetCavesState(cavesLevel);
    view = findView(frameView);
    if (view && !view->body.running) {
        if (firstFrame > 6003)
            setViewScript(view, firstFrame - 1, 1);
        else
            setViewScript(view, firstFrame, 1);
    }
}

/* Reads the roster file (into gameState) unless the user file is the
   default one (ZBUser.txt), checks its version (107) and applies its
   settings. */
/* @zoombi32 0x0041f5d0 */
void readRoster()
{
    char name[32] = "ZBUser";

    strcat(name, ".txt");
    if (strncmp(userFileName, name, strlen(userFileName))) {
        readWriteRoster(gameState, 1);
        if (swapShort(*(unsigned short *)gameState) != 107)
            reportRosterError("Invalid user file, delete and try again: ");
        applyPlayerSettings();
        journeyTo = 0;
    }
}

/* Lays out, from place firstCave on, which values of the roster's
   features each of the 21 places wants (cavePlaceValues): for each value of the
   first feature as many places as chosen Zoombinis have it (caveValueCounts),
   and for the second feature likewise within them. */
/* @zoombi32 0x0041e5e1 */
void layOutCaves()
{
    short v;
    short start;
    short first;
    short second;
    short i;
    short w;

    for (i = 0; i < 21; i++)
        spotSnoids[i] = 0;
    first = 0;
    second = 1;
    start = firstCave;
    for (v = 0; v < caveValueCount; v++) {
        for (i = start; i < caveValueCounts[first][caveValues[first][v]] + start; i++)
            cavePlaceValues[first][i] = caveValues[first][v];
        start = i;
    }
    start = firstCave;
    for (v = 0; v < caveValueCount; v++) {
        for (w = 0; w < caveValueCount; w++) {
            for (i = start; i < caveValueCounts[caveValues[second][w]][caveValues[first][v]] + start; i++)
                if (caveValues[second][w])
                    cavePlaceValues[second][i] = caveValues[second][w];
            start = i;
        }
    }
}

/* The place (from firstCave) for the Zoombini of view `id`: `n` if it's
   free and wants the Zoombini's values of the roster's features, else a
   free one that does, at random; 1 if none. */
/* @zoombi32 0x0041e771 */
short pickCave(short id, short n)
{
    short places[21];
    View *view;
    short count;
    short first;
    short second;
    short i;

    count = 0;
    fillMemory(places, 0, sizeof places);
    view = findView(id);
    first = 0;
    second = 0;
    for (i = 0; i < caveValueCount; i++) {
        if (viewSnoid(view)->features[caveFeatures[0]] == caveValues[0][i])
            first = caveValues[0][i];
        if (first)
            i = caveValueCount;
    }
    for (i = 0; i < caveValueCount; i++) {
        if (viewSnoid(view)->features[caveFeatures[1]] == caveValues[1][i])
            second = caveValues[1][i];
        if (second)
            i = caveValueCount;
    }
    for (i = firstCave; i < 21; i++)
        if (first == cavePlaceValues[0][i] && i == n && !spotSnoids[i]) {
            if (caveFeatureCount <= 1)
                return i;
            if (second != cavePlaceValues[1][i])
                continue;
            return i;
        }
    for (i = firstCave; i < 21; i++)
        if (!spotSnoids[i] && first == cavePlaceValues[0][i]) {
            if (caveFeatureCount > 1) {
                if (second == cavePlaceValues[1][i])
                    places[count++] = i;
            } else {
                places[count++] = i;
            }
        }
    if (count)
        return places[randomBetween(0, --count)];
    return 1;
}

/* Fills in the roster's header (gameState): when `reset`, a new one (version
   107, default settings) with the sound slots (allocateSlot) and more reset, else the
   player's current settings; then the scene (or, in scene 2, journeyTo). */
/* @zoombi32 0x0041f6fc */
void fillRosterHeader(short reset)
{
    if (reset) {
        fillMemory(gameState, 0, 0xae05);
        *(unsigned short *)gameState = swapShort(107);
        *(unsigned short *)(gameState + 2) = swapShort(30);
        gameState[4] = 1;
        gameState[5] = 1;
        gameState[6] = 1;
        gameState[7] = 1;
        gameState[8] = 0;
        gameState[9] = 0;
        *(short *)(gameState + 0xa) = 0;
        *(short *)(gameState + 0x20) = fidgetPaceFlag;
        rosterChanged = 1;
        switchedToTemp = 0;
        wPressed = 0;
        speaker0BackLinesUsed = speaker0RepliesUsed = speaker2BackLinesUsed = doors34LinesUsed = speaker3BackLinesUsed = 0;
        speaker3RepliesUsed = speaker1BackLinesUsed = doors16LinesUsed = tunnelRemarks0Used = tunnelRemarks1Used = 0;
        tunnelRemarksUnused = tunnelRemarks2Used = tunnelRemarks3bUsed = tunnelRemarks3cUsed = 0;
        tunnelRemarks3aUsed = 0;
        ferryVisits = 0;
        returnRoutesUsed = 0;
    } else {
        *(unsigned short *)(gameState + 2) = swapShort(clickTime);
        gameState[4] = soundOn;
        gameState[5] = musicOn;
        gameState[6] = clickToDragOption;
        gameState[7] = hideDragCursor;
        gameState[8] = debugMessagesOn;
        gameState[9] = dragClicks;
        *(short *)(gameState + 0xa) = transitionsOn;
    }
    *(short *)(gameState + 0xcc) = currentScene;
    *(short *)(gameState + 0xca) = journeyFrom;
    if (currentScene == 2)
        *(short *)(gameState + 0xcc) = journeyTo;
}

/* Reads or writes the list of saved games (the file rosterFileName in the
   directory rosterDirectory) into or from `list` (or a list of its own): creates
   it if it's new; `mode` 0 reads the count of games, 1 writes it
   (savedGames), 2 reads the count and the next id, 3 writes the list. */
/* @zoombi32 0x0041f2c8 */
void readWriteSavedGames(SavedGameList *list, short mode)
{
    long size;
    char path[256];
    SavedGameList own;
    SavedGameList *games;
    short result;
    short error;

    if (!list)
        games = &own;
    else
        games = list;
    size = sizeof(SavedGameList);
    result = 3;
    strcpy(path, rosterDirectory);
    strcat(path, rosterFileName);
    result = openRosterFile(path, result);
    if (result == 2)
        reportRosterError("Could not Open/Create Roster file.");
    if (result == 2)
        return;
    if (result == 1) {
        fillMemory(games, 0, size);
        games->version = 107;
        if (writeFile(rosterFile, games, &size))
            reportRosterError("Problem writing file: disk may be full");
    } else {
        if (seekFile(rosterFile, 0, 0) == -1)
            reportRosterError("Seek Error");
        switch (mode) {
        case 0:
            error = readFile(rosterFile, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107)
                savedGames = games->count;
            else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 1:
            error = readFile(rosterFile, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107) {
                if (seekFile(rosterFile, 0, 0) == -1)
                    reportRosterError("Seek Error");
                games->count = savedGames;
                if (writeFile(rosterFile, games, &size))
                    reportRosterError("Problem writing file: disk may be full");
            } else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 2:
            error = readFile(rosterFile, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107) {
                savedGames = games->count;
                nextSaveId = games->nextId;
            } else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 3:
            if (writeFile(rosterFile, games, &size))
                reportRosterError("Problem writing file: disk may be full");
            break;
        }
    }
    closeFile(rosterFile, 0);
}

/* Walks the roster's next Zoombini (cavesNextWalker) on: 0 from the view
   walkFromView (script walkScriptsBase on, for frame droppedCave, with a view of its
   own in front, walkerFrontView), 1 likewise from walkToView (for frame assignedCave,
   after claimSpot), 2 off toward the view frameView (script 12012). */
/* @zoombi32 0x0041cf14 */
void walkNext(short which)
{
    View *view;

    if (cavesNextWalker) {
        walkerView = cavesNextWalker;
        switch (which) {
        case 0:
            view = findView(walkFromView);
            break;
        case 1:
            view = findView(walkToView);
            break;
        case 2:
            view = findView(walkerView);
            break;
        }
        if (view) {
            switch (which) {
            case 0:
                walkerAnchor = 0;
                setViewScript(view, (droppedCave - 1) * 4 + walkScriptsBase, 1);
                view->notify = walkerNotify;
                deleteView(walkerFrontView);
                moveView(walkerView, 1, frameAnchorViews[droppedCave]);
                walkerFrontView = addView(0x4108000, drawCels, runViewScript, (droppedCave - 1) * 4 + walkScriptsBase + 1, 6,
                                   0, 1, walkerView);
                groupViews(view->id, walkerFrontView, 0, 0, 0, 0);
                break;
            case 1:
                claimSpot(walkerView, assignedCave);
                walkerAnchor = &claimedSpotPoint;
                setViewScript(view, (assignedCave - 1) * 4 + walkScriptsBase + 2, 1);
                view->notify = walkerNotify;
                deleteView(walkerFrontView);
                moveView(walkerView, 1, frameAnchorViews[assignedCave]);
                walkerFrontView = addView(0x4108000, drawCels, runViewScript, (assignedCave - 1) * 4 + walkScriptsBase + 3, 6,
                                   0, 1, walkerView);
                groupViews(view->id, walkerFrontView, 0, 0, 0, 0);
                break;
            case 2:
                setViewsLocked(0);
                walkerAnchor = 0;
                viewSnoid(view)->facingLeft = 0;
                view->notify = walkerNotify;
                startSnoidScript(viewSnoid(view), 12012, walkerAnchor, 1);
                groupViews(view->id, view->id, 0, 0, 0, 0);
                moveView(view->id, 0, frameView);
                break;
            }
        }
    }
}

/* Places the roster's feature images: `kind` 1 at the first five
   places, 2 at two of them at random, 3 at two of each row of five; each
   showing the value there of the row's feature (images 5 apart per
   feature). */
/* @zoombi32 0x0041e326 */
void placeGlyphs(short kind)
{
    short count;
    short offset;
    short j;
    short order[7];
    short i;
    short left;
    short k;

    for (i = 0; i < 7; i++)
        order[i] = i;
    for (i = 0; i < 11; i++) {
        glyphPlaced[i] = 0;
        glyphImages[i] = 0;
    }
    switch (kind) {
    case 1:
        for (i = 1; i < 6; i++)
            glyphPlaced[i] = 1;
        break;
    case 2:
        left = 5;
        count = randomBetween(2, 2);
        for (i = 0; i < count; i++) {
            k = randomBetween(1, left);
            glyphPlaced[order[k]] = 1;
            for (; k < left + 1; k++)
                order[k] = order[k + 1];
            left--;
        }
        break;
    case 3:
        for (j = 0; j < 2; j++) {
            if (j)
                offset = 5;
            else
                offset = 0;
            for (i = 0; i < 7; i++)
                order[i] = i;
            left = 5;
            count = randomBetween(2, 2);
            for (i = 0; i < count; i++) {
                k = randomBetween(1, left);
                glyphPlaced[offset + order[k]] = 1;
                for (; k < left + 1; k++)
                    order[k] = order[k + 1];
                left--;
            }
        }
        break;
    case 4:
        break;
    }
    for (i = 1; i < 6; i++)
        if (glyphPlaced[i])
            switch (caveFeatures[0]) {
            case 0:
                glyphImages[i] = caveValues[0][i - 1];
                break;
            case 1:
                glyphImages[i] = caveValues[0][i - 1] + 5;
                break;
            case 2:
                glyphImages[i] = caveValues[0][i - 1] + 10;
                break;
            case 3:
                glyphImages[i] = caveValues[0][i - 1] + 15;
                break;
            }
    for (i = 6; i < 11; i++)
        if (glyphPlaced[i])
            switch (caveFeatures[1]) {
            case 0:
                glyphImages[i] = caveValues[1][i - 6];
                break;
            case 1:
                glyphImages[i] = caveValues[1][i - 6] + 5;
                break;
            case 2:
                glyphImages[i] = caveValues[1][i - 6] + 10;
                break;
            case 3:
                glyphImages[i] = caveValues[1][i - 6] + 15;
                break;
            }
}

/* Draws the roster's feature table: for each feature asked about, its
   letter and the pictures of its values, and under the first, how many of
   the chosen Zoombinis have each (caveValueCounts). */
/* @zoombi32 0x0041edf7 */
void drawFeatureTable()
{
    ShortRect firstName = {120, 360, 260, 386};
    ShortRect firstValues = {120, 390, 260, 416};
    ShortRect firstCounts = {120, 420, 260, 446};
    ShortRect secondName = {275, 360, 395, 386};
    ShortRect secondValues = {275, 390, 395, 416};
    ShortRect all = {120, 360, 395, 446};
    ShortRect rect;
    char letters[4][2] = {"H", "E", "N", "F"};
    char numbers[21][3] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "11", "12", "13", "14", "15", "16", "17", "18", "19", "20"};
    Color saved;
    short i;
    short j;

    saved = setForeColor(Color(11));
    for (i = 0; i < caveFeatureCount; i++) {
        if (!i)
            rect = firstName;
        else
            rect = secondName;
        fillPortRect(rect, Color(14), 0);
        frameRect(rect);
        drawText(rect, 0x22, letters[caveFeatures[i]], 0xffff);
        if (!i)
            rect = firstValues;
        else
            rect = secondValues;
        rect.top += 5;
        for (j = 0; j < caveValueCount; j++) {
            drawFeatureImage(caveFeatures[i] + 1, caveValues[i][j], rect);
            rect.left += 30;
            rect.right = rect.left + 25;
        }
        if (!i) {
            rect = firstCounts;
            fillPortRect(rect, Color(14), 0);
            rect.left += 10;
            rect.top += 5;
            for (j = 0; j < caveValueCount; j++) {
                if (caveValueCounts[0][caveValues[i][j]])
                    drawText(rect, 1, numbers[caveValueCounts[0][caveValues[i][j]]], 0xffff);
                else
                    drawText(rect, 1, numbers[0], 0xffff);
                rect.left += 30;
                rect.right = rect.left + 25;
            }
        }
    }
    setForeColor(saved);
    showRect(&all);
}

/* Picks the roster's features and lays out the places for them. */
/* @zoombi32 0x0041e0e3 */
void setUpCaves()
{
    pickCaveFeatures();
    countByCaveFeatures();
    layOutCaves();
}

/* Walks the chosen Zoombinis (up to chosenCount + 1) to their spots on the
   roster screen (chosenSpots), resets the view glyphView, and clears the
   places from firstCave on and the screen's state. */
/* @zoombi32 0x0041ec69 */
void walkToSpots()
{
    volatile short unused; /* never used; volatile keeps its stack slot */
    View *view;
    View *walker;
    short n;
    short i;

    n = 0;
    for (walker = viewListEnd(1); walker && n <= chosenCount; walker = walker->next)
        if (walker->flags == 1) {
            setSnoidAction(viewSnoid(walker), 0, &chosenSpots[n]);
            n++;
        }
    view = findView(glyphView);
    if (view)
        view->reset = 1;
    unionRgnRect(removedRgn, &glyphArea);
    mainLoopEvents();
    for (i = firstCave; i < 21; i++) {
        spotSnoids[i] = 0;
        placeClaims[i - 1] = 0;
    }
    for (i = 0; i < missingSnoids; i++)
        placeClaims[i] = caveViews[0];
    resetCavesScreen();
    cavesFullParty = 0;
    cavesGoReady = 0;
    cavesBusy = 0;
    exitDue = 0;
    cavesPlacedCount = 0;
}

/* Saves the roster (with the player's settings, fillRosterHeader) if it changed
   (rosterChanged) and the user file isn't the default one (ZBUser.txt). */
/* @zoombi32 0x0041f551 */
void saveRoster()
{
    char name[32] = "ZBUser";

    strcat(name, ".txt");
    if (strncmp(userFileName, name, strlen(userFileName)) && rosterChanged) {
        if (gameState) {
            fillRosterHeader(0);
            readWriteRoster(gameState, 0);
        }
        rosterChanged = 0;
    }
}

/* Gives each Zoombini on the roster screen without a place (from
   firstCave) one (pickCave) and walks it there, then claims the places
   taken and lets the others be chosen again. */
/* @zoombi32 0x0041eb43 */
void sendToCaves()
{
    View *view;
    View *other;
    short found;
    short i;

    for (view = viewListEnd(1); view; view = view->next)
        if (view->flags == 1) {
            found = 0;
            for (i = firstCave; i < 21; i++)
                if (spotSnoids[i] == view->id) {
                    found = 1;
                    i = 21;
                }
            if (!found) {
                assignedCave = pickCave(view->id, 0);
                placedViewPoint(&claimedSpotPoint, assignedCave);
                setSnoidAction(viewSnoid(view), 5, &claimedSpotPoint);
                spotSnoids[assignedCave] = view->id;
            }
        }
    other = findView(glyphView);
    if (other)
        other->reset = 1;
    for (i = firstCave; i < 21; i++)
        if (spotSnoids[i])
            claimPlacedView(i, spotSnoids[i]);
        else
            placeClaims[i] = 0;
    for (i = 0; i < missingSnoids; i++)
        placeClaims[i] = caveViews[0];
    chooseSnoids(1, 0);
    unionRgnRect(removedRgn, &gameRect);
    mainLoopEvents();
}

/* Changes the roster's first feature to `feature` (0-3; -1 keeps it; the
   second moves on if they'd be the same), lays the places out again and
   walks the Zoombinis that had places to their new ones. */
/* @zoombi32 0x0041e920 */
void changeCaveFeature(short feature)
{
    short placed[21];
    View *view;
    short i;

    if (feature != -1) {
        switch (feature) {
        case 0:
            caveFeatures[0] = 0;
            break;
        case 1:
            caveFeatures[0] = 1;
            break;
        case 2:
            caveFeatures[0] = 2;
            break;
        case 3:
            caveFeatures[0] = 3;
            break;
        }
        if (caveFeatures[0] == caveFeatures[1])
            caveFeatures[1]++;
        if (caveFeatures[1] > 3)
            caveFeatures[1] = 0;
    }
    for (i = firstCave; i < 21; i++)
        placed[i] = spotSnoids[i];
    chooseSnoids(1, 0);
    countByCaveFeatures();
    layOutCaves();
    chooseSnoids(0, 0);
    for (i = firstCave; i < 21; i++)
        if (placed[i]) {
            droppedCave = i;
            assignedCave = pickCave(placed[i], droppedCave);
            placedViewPoint(&claimedSpotPoint, assignedCave);
            setSnoidAction((Snoid *)&findView(placed[i])->body, 5, &claimedSpotPoint);
            spotSnoids[assignedCave] = placed[i];
        }
    for (i = firstCave; i < 21; i++)
        if (spotSnoids[i])
            claimPlacedView(i, spotSnoids[i]);
        else
            placeClaims[i - 1] = 0;
    for (i = 0; i < missingSnoids; i++)
        placeClaims[i] = caveViews[0];
    placeGlyphs(cavesLevel);
    view = findView(glyphView);
    if (view)
        view->reset = 1;
    unionRgnRect(removedRgn, &cavesButtons[0].rect);
}

/* The roster screen's clicks: button 1 asks whether to keep the party,
   button 2 (once any Zoombini is placed) goes on; otherwise a Zoombini
   not yet placed is dragged to a place (walked to the right one if it
   doesn't belong there) or, dropped outside the places, walked back. A
   click after one of the buttons (sceneDue) leaves the screen. */
/* @zoombi32 0x0041d3f4 */
void cavesClicked(short which)
{
    Point where;
    Point from;
    View *view;
    Snoid *snoid;
    short free;
    short found;
    short i;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeCaves();
        return;
    }
    view = 0;
    getCursorPosition(&where);
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawCavesButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawCavesButton(which, 0, 1);
        snoidsArrived = 1;
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (cavesGoReady) {
            if (!exitStage) {
                exitDue = 1;
                cavesBusy = 1;
            }
            drawCavesButton(which, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawCavesButton(which, 0, 1);
            markPlacedSnoids();
            sceneDue = 17;
        }
        break;
    case 3:
        if (!view)
            view = viewAt(where, 1, 1);
        if (view) {
            free = 1;
            for (i = 1; i < 21; i++)
                if (spotSnoids[i] == view->id)
                    free = 0;
            if (free == 1 && !cavesBusy && snoidsOnTheirWay <= 0) {
                from = *(Point *)&view->body.x;
                dragSnoid(view, where, 0, 0);
                droppedCave = heldPlaceNumber();
                if (droppedCave) {
                    snoid = viewSnoid(view);
                    snoid->chosen = 1;
                    view->flags = 0x4008001;
                    if ((assignedCave = pickCave(view->id, droppedCave)) == droppedCave) {
                        spotSnoids[droppedCave] = view->id;
                        snoid->angle = caveSnoidF1[droppedCave];
                        snoid->facingLeft = caveSnoidF2[droppedCave];
                        cavesPlacedCount++;
                        walkerView = 0;
                        if (cavesPlacedCount == 1) {
                            cavesGoReady = 1;
                            unionRgnRect(removedRgn, &cavesButtons[1].rect);
                            cheerQueue[cheerQueueCount] = view->id;
                            cheerQueueCount++;
                            cheerAnchor = &cavePoints[droppedCave - 1];
                        } else if (cavesPlacedCount == chosenCount) {
                            allPlaced = 1;
                            cavesBusy = 1;
                            queueViewSound(randomBetween(20055, 20063), 0);
                        } else {
                            cheerQueue[cheerQueueCount] = view->id;
                            cheerQueueCount++;
                            cheerAnchor = &cavePoints[droppedCave - 1];
                        }
                        moveView(view->id, 1, frameAnchorViews[droppedCave]);
                    } else {
                        spotSnoids[assignedCave] = view->id;
                        cavesNextWalker = view->id;
                        walkFromView = caveViews[droppedCave];
                        walkToView = caveViews[assignedCave];
                        cavesPlacedCount++;
                        cavesBusy = 1;
                        walkDue = 1;
                        releaseHeldPlace();
                        if (cavesPlacedCount == 1) {
                            cavesGoReady = 1;
                            unionRgnRect(removedRgn, &cavesButtons[1].rect);
                        } else if (cavesPlacedCount == chosenCount) {
                            queueViewSound(randomBetween(20055, 20063), 0);
                        }
                    }
                } else {
                    for (i = 0, found = 0; i < 12; i++)
                        if (ptInRect(&caveWaitAreas[i], *(Point *)&view->body.x)) {
                            found = i;
                            i = 12;
                        }
                    if (!found) {
                        cavesBusy = 1;
                        cavesNextWalker = view->id;
                        walkBackPoints[walkBackCount] = from;
                        walkBackCount++;
                        walkNext(2);
                    }
                }
            }
        }
        break;
    }
}

/* The roster screen's frame (scenes[]): leaves the screen after a button
   (sceneDue) once the sound and the Zoombinis are done, starts the walk
   (exitDue), shows the frames' changes (framesChanged), walks Zoombinis on
   (walkNext), blinks the place blinkingGlyph, has placed Zoombinis (cheerQueue)
   and, when all are placed (allPlaced), the others cheer, and plays the
   ambient sounds. */
/* @zoombi32 0x0041ca44 */
void cavesFrame()
{
    View *view;
    Snoid *snoid;
    short started;
    short i;
    short n;

    if (!inCavesFrame && cavesOpen) {
        inCavesFrame = 1;
        updateViews();
        if (sceneDue) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                inCavesFrame = 0;
                return;
            }
            if (!dialogQuestion || dialogQuestion == 3) {
                if (dialogQuestion == 3)
                    chooseSnoids(0, 0);
                if ((viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1)
                    && (exitStage == 3 || sceneDue == 1 || viewsLocked)) {
                    pendingScene = sceneDue;
                    sceneDue = 0;
                    setCurrentMap(0);
                    closeCaves();
                    inCavesFrame = 0;
                    return;
                }
                if (exitStage == 2) {
                    exitStage = 3;
                    queueViewSound(996, 0);
                    sendReadyOff(660, 376, 30);
                }
            } else if (dialogQuestion == 2) {
                dialogQuestion = 0;
                sceneDue = 0;
            }
        }
        if (dialogFlags) {
            playAmbientSound();
            inCavesFrame = 0;
            return;
        }
        if (exitDue && !walkerView) {
            exitDue = 0;
            exitStage = 1;
            setViewsLocked(0);
            view = removeView(frameAnchorViews[24], 0);
            if (view) {
                cavesFullParty = 1;
                setViewScript(view, 6002, 1);
                view->notify = frameNotify;
                insertViewAtEnd(view);
            }
        }
        if (framesChanged) {
            framesChanged = 0;
            view = findView(walkFromView);
            if (view) {
                setViewScript(view, droppedCave + 8999, 0);
                view->body.running = 1;
            }
            view = findView(walkToView);
            if (view) {
                setViewScript(view, assignedCave + 8999, 0);
                view->body.running = 1;
            }
            view = findView(walkerView);
            if (view) {
                snoid = viewSnoid(view);
                snoid->angle = caveSnoidF1[assignedCave];
                snoid->facingLeft = caveSnoidF2[assignedCave];
            }
        }
        if (walkDue) {
            walkDue = 0;
            setViewsLocked(0);
            walkNext(0);
        }
        if (walkOnDue) {
            walkOnDue = 0;
            walkNext(1);
        }
        if (cavesLevel == 1 && blinkingGlyph < 6) {
            view = findView(glyphView);
            if (view && clockTime() >= view->nextUpdate) {
                if (!blinkingGlyph) {
                    view->nextUpdate = clockTime() + 30;
                    unionRgnRect(removedRgn, &glyphArea);
                    lastBlinkedGlyph++;
                    blinkingGlyph = lastBlinkedGlyph;
                } else {
                    view->nextUpdate = clockTime() + 30;
                    unionRgnRect(removedRgn, &glyphArea);
                    blinkingGlyph = 0;
                }
            }
        }
        while (cheerQueueCount) {
            view = findView(cheerQueue[--cheerQueueCount]);
            if (view) {
                view->flags = 0x4008001;
                snoid = viewSnoid(view);
                startSnoidScript(viewSnoid(view), snoid->features[3] + 12999, cheerAnchor, 0);
                view->notifyEnd = 1;
                view->notify = (ViewNotify)cheerNotify;
                cheerAnchor = 0;
            }
        }
        if (allPlaced && cheersDone < cheersAllowed) {
            if (clockTime() - lastCheerTime > 30) {
                started = 0;
                lastCheerTime = clockTime();
                for (i = 0; i < chosenCount && !started; i++) {
                    n = allocateSlot(&cheerersUsed, chosenCount, 0);
                    if (partyViews[n]) {
                        view = idleSnoidView(partyViews[n]);
                        /* The original's test is always true: == binds before |. */
                        if (view && view->body.running && ((view->flags == 0x8000) | 0x4000001)) {
                            snoid = viewSnoid(view);
                            startSnoidScript(viewSnoid(view), snoid->features[3] + 12999, 0, 0);
                            view->notifyEnd = 1;
                            view->notify = (ViewNotify)cheerNotify;
                            cheersDone++;
                            started = 1;
                        }
                    }
                }
            }
        } else if (cheersDone >= cheersAllowed) {
            cheersDone = allPlaced = lastCheerTime = cheerersUsed = 0;
        }
        playAmbientSound();
        inCavesFrame = 0;
    }
}

/* The roster screen's keys (with debugging on, debugMessagesOn, or else only
   0x16f): 1-4 set the level (cavesLevel) and show it, L shows it, space
   resets the frames (resetCavesScreen), 0x171 walks the Zoombinis to their spots,
   0x173-0x176 change the first feature, 0x16f (below level 4) calls
   replayHint. Returns whether the key was used. */
/* @zoombi32 0x0041dd83 */
short cavesKey(unsigned short key)
{
    Color saved;
    char digits[32] = "01234";
    char level[32] = "Level x ";
    short shown = 0;
    ShortRect rect = {275, 0, 375, 18};
    short used;

    if (!debugMessagesOn && key != 0x16f)
        return 0;
    switch (key) {
    case '1':
    case '2':
    case '3':
    case '4':
        cavesLevel = key - '0';
        changeCaveFeature(-1);
        redrawGlyphs(1);
        if (featureTableShown)
            drawFeatureTable();
    case 'L':
        level[6] = digits[cavesLevel];
        shown = 1;
        used = 1;
        break;
    case ' ':
        resetCavesScreen();
        used = 1;
        break;
    case 0x170:
        used = 1;
        break;
    case 0x171:
        walkToSpots();
        used = 1;
        break;
    case 0x172:
        used = 1;
        break;
    case 0x173:
        changeCaveFeature(0);
        drawFeatureTable();
        redrawGlyphs(1);
        used = 1;
        break;
    case 0x174:
        changeCaveFeature(1);
        drawFeatureTable();
        redrawGlyphs(1);
        used = 1;
        break;
    case 0x175:
        changeCaveFeature(2);
        drawFeatureTable();
        redrawGlyphs(1);
        used = 1;
        break;
    case 0x176:
        changeCaveFeature(3);
        drawFeatureTable();
        redrawGlyphs(1);
        used = 1;
        break;
    case 0x16f:
        if (cavesLevel < 4)
            replayHint();
        used = 1;
        break;
    default:
        used = 0;
        break;
    }
    if (shown) {
        saved = setForeColor(Color(11));
        fillPortRect(rect, Color(14), 0);
        drawText(rect, 0x22, level, 0xffff);
        showRect(&rect);
        setForeColor(saved);
    }
    return used;
}

/* Reads (`read`) or writes the roster (`data`, 0xae05 bytes) from or to
   the roster file next to the program (userFileName). */
/* @zoombi32 0x0041f1da */
void readWriteRoster(void *data, short read)
{
    long size;
    char path[256];
    short result;

    if (!data)
        reportRosterError("Invalid Data Pointer");
    size = 0xae05;
    result = 3;
    strcpy(path, moduleFileName);
    strcat(path, userFileName);
    result = openRosterFile(path, result);
    if (result == 2)
        reportRosterError("Could not Open/Create Roster file.");
    if (result != 2) {
        if (seekFile(rosterFile, 0, 0) == -1)
            reportRosterError("Seek Error");
        if (read) {
            if (readFile(rosterFile, data, &size))
                reportRosterError("Problem reading file");
        } else if (writeFile(rosterFile, data, &size)) {
            reportRosterError("Problem writing file: disk may be full");
        }
        closeFile(rosterFile, 0);
    }
}

/* Resets the roster screen's state for mode `which` (1-4: the frames
   firstFrame-finalFrame it shows). */
/* @zoombi32 0x0041dfe3 */
void resetCavesState(short which)
{
    exitDue = 0;
    exitStage = 0;
    walkBackCount = 0;
    walkerView = 0;
    blinkingGlyph = 0;
    lastBlinkedGlyph = 0;
    cavesPlacedCount = 0;
    droppedCave = 0;
    assignedCave = 0;
    caveValueCount = 5;
    caveFeatureCount = 1;
    featureTableShown = 0;
    walkerFrontView = 0;
    unusedCaves1 = 0;
    currentFrame = 0;
    unusedCaves2 = 0;
    cavesBusy = 0;
    switch (which) {
    case 1:
        finalFrame = 4;
        firstFrame = 6006;
        break;
    case 2:
        finalFrame = 5;
        firstFrame = 6005;
        break;
    case 3:
        finalFrame = 6;
        firstFrame = 6004;
        break;
    case 4:
        finalFrame = 7;
        firstFrame = 6003;
        break;
    }
}
