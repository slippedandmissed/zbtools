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
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "roster.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

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
   swapped here, locked in g_4aba68 and g_4aba6c) and its hieroglyphs
   (shape 10000). */
/* @zoombi32 0x0041dbce */
void loadCaveResources()
{
    short i;
    unsigned short *data;
    unsigned long size;

    for (i = 0; i < 2; i++) {
        g_4aba70[i] = 0;
        fn_46c4fe(&g_4aba70[i], RESOURCE_TYPE('R', 'E', 'G', 'S'), i + 200, 0, 1);
        g_4aba78[i] = fn_46beac(g_4aba70[i]);
        switch (i) {
        case 0:
            g_4aba68 = (short *)lockHandle(g_4aba78[i]);
            break;
        case 1:
            g_4aba6c = (short *)lockHandle(g_4aba78[i]);
            break;
        }
        data = (unsigned short *)handleData(g_4aba78[i]);
        for (size = handleSize(g_4aba78[i]); size; size -= 2) {
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
        fn_46c602(&glyphShape);
        glyphShape = 0;
    }
}

/* A view's update: redraws g_4a11ac when the view asks to (reset). */
/* @zoombi32 0x0041dbab */
void updateGlyphArea(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &g_4a11ac);
    }
}

/* Puts view `id` in placed spot n (1-20), noting the spot's point in
   g_4ab8e0. */
/* @zoombi32 0x0041e8f3 */
void claimSpot(short id, short n)
{
    if (n > 0 && n < 21) {
        placedViewPoint(&g_4ab8e0, n);
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

/* Starts the view walkerView's Snoid on script `script` (by g_4ab8e4,
   unknownF8 `f8`), in group `group`, with notify `notify` if given. */
/* @zoombi32 0x0041d167 */
void startWalkerScript(short group, short script, ViewNotify notify, char f8)
{
    View *view = findView(walkerView);

    if (view) {
        startSnoidScript((Snoid *)&view->body, script, g_4ab8e4, f8);
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

/* Releases the two locked handles (g_4aba78) and their resources
   (g_4aba70). */
/* @zoombi32 0x0041dce6 */
void freeCaveResources()
{
    short i;

    for (i = 0; i < 2; i++)
        if (g_4aba78[i]) {
            unlockHandle(g_4aba78[i]);
            fn_46c602(&g_4aba70[i]);
            g_4aba78[i] = 0;
            g_4aba70[i] = 0;
        }
}

/* A view's update: redraws button 2 when g_4a0fe8 changes, and button 1
   once. */
/* @zoombi32 0x0041d972 */
void updateCavesButtons(View *, short region)
{
    if (g_4a0fe8) {
        if (!g_4a120a) {
            g_4a120a = 1;
            unionRgnRect(region, &cavesButtons[1].rect);
        }
    } else if (g_4a120a) {
        g_4a120a = 0;
        unionRgnRect(region, &cavesButtons[1].rect);
    }
    if (!g_4a120c) {
        g_4a120c = 1;
        unionRgnRect(region, &cavesButtons[0].rect);
    }
}

/* Applies the player's settings from the game state: click time (stored
   big-endian), sound and music, the drag options, debugging messages,
   and the scenes. */
/* @zoombi32 0x0041f668 */
void applyPlayerSettings()
{
    clickTime = swapShort(*(unsigned short *)(g_4a4ba0 + 2));
    g_4b87fe = g_4a4ba0[4];
    g_4b87ff = g_4a4ba0[5];
    clickToDragOption = g_4a4ba0[6];
    hideDragCursor = g_4a4ba0[7];
    g_4b8803 = g_4a4ba0[8];
    dragClicks = g_4a4ba0[9];
    g_4b0d4a = *(short *)(g_4a4ba0 + 0xa);
    g_4b0d52 = *(short *)(g_4a4ba0 + 0xcc);
    g_4b0d56 = *(short *)(g_4a4ba0 + 0xca);
}

/* Opens the roster file `path` (mode `mode`) as g_4aba7c, making its
   directory first if need be: 0 if it opened, 1 if it did after making
   the directory, 2 if it didn't. */
/* @zoombi32 0x0041f100 */
short openRosterFile(const char *path, short mode)
{
    fileSpec spec(path);
    short result = 0;

    g_4aba7c = openFile(&spec, mode);
    if (!g_4aba7c) {
        createPath(spec, 0);
        g_4aba7c = openFile(&spec, mode);
        if (!g_4aba7c)
            result = 2;
        else
            result = 1;
    }
    return result;
}

/* Draws button `which` (1: 5, 2: 2, or 1 if g_4a0fe8 isn't set; the next
   image if lit) from the bank g_4a1020, showing it if `show`. */
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
        if (!g_4a0fe8) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a1020->offsets[image] + (char *)g_4a1020), cavesButtons[which - 1].rect.left,
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
        image = g_4a12a6[n];
        break;
    case 2:
        image = g_4a129a[n];
        break;
    case 3:
        image = g_4a128e[n];
        break;
    case 4:
        image = g_4a1282[n];
        break;
    }
    handle = fn_46beac(snoidImagesResource);
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

            if (snoid->unknownF7) {
                *(Point *)&snoid->body.unknownAa = *(Point *)&snoid->body.x;
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
    g_4b755a = 1;
    g_4b755c = 0;
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
    g_4b0d52 = 0;
    g_4a0fe8 = 0;
    g_4a1006 = 0;
    g_4a1008 = 0;
    g_4a100a = 0;
    g_4b966e = 0;
    g_4ab9fc = 0;
    g_4aba00 = 0;
    g_4aba04 = 0;
    g_4aba06 = 0;
    g_4aba08 = 0;
    g_4aba0c = 0;
    g_4ab9be = 0;
    g_4ab994 = 0;
    g_4ab8e8 = 0;
    fillMemory(g_4ab9c4, 0, 44);
    fillMemory(g_4ab996, 0, 40);
    openGameFile(&g_4ab83c, "Caves.MHK");
    fn_46be2e(g_4ab83c);
    g_4a1020 = loadImageBank(11000, &g_4a0fd0);
    loadTerrain(100);
    loadPaths(1000);
    drawBackdrop(5000);
    loadFeatureGroup(6000, 0, 0);
    loadFeatureGroup(9000, 1, 0);
    loadFeatureGroup(7000, 2, 0);
    g_4a0ffe = 1;
    g_4a0ffc = g_4a0ffe * 200 + 8000;
    g_4a0ffe = g_4a0ffe * 4 + 12000;
    loadFeatureGroup(g_4a0ffc, 3, 0);
    loadFeatureGroup(9025, 4, 0);
    loadScripts(6000, 13);
    addScripts(9000, 20, 0);
    addScripts(7000, 20, 0);
    addScripts(g_4a0ffc, 80, 0);
    addScripts(9025, 4, 0);
    loadSnoidScripts(12000, 14, 0);
    addSnoidScripts(13000, 5, 0);
    fn_4148da(10, 236);
    g_4ab9f0 = addView(0x4088000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
    g_4ab9f2 = addView(0x4088000, drawCels, runViewScript, 6001, 6, 0, 0, 0);
    g_4ab9f4 = addView(0x4188000, drawCels, runViewScript, 6002, 8, 0, 0, 0);
    setViewPlaces(20, chosenSpots, 1);
    makePartySnoids(0);
    chosenCount = listChosenSnoids()->count;
    g_4aba04 = chosenCount - 1;
    g_4a1016 = 20 - chosenCount;
    if (g_4a1016 > 4)
        g_4a1016 = 4;
    if (g_4a1016)
        g_4ab96a[0] = addView(0x4108000, drawCels, runViewScript, g_4a1016 + 9024, 6, 0, 0, 0);
    firstCave = 21 - chosenCount;
    {
        short rows = firstCave;

        if (rows > 5)
            ; /* the original tests this and does nothing */
    }
    for (i = 0; i < 4; i++)
        placedViews[i] = addView(0x508a000, drawCels, runViewScript, i + 7000, 7, &g_4a10a8[i + 1], 0, 0);
    for (i = 5; i < 12; i++) {
        g_4ab96a[i] = addView(0x4108000, drawCels, runViewScript, i + 8999, 6, 0, 0, 0);
        g_4ab9c4[i] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
        placedViews[i - 1] = addView(0x508a000, drawCels, runViewScript, i + 6999, 7, &g_4a10a8[i], 0, 0);
    }
    g_4ab96a[15] = addView(0x4108000, drawCels, runViewScript, 9014, 6, 0, 0, 0);
    g_4ab9c4[15] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    g_4ab96a[14] = addView(0x4108000, drawCels, runViewScript, 9013, 6, 0, 0, 0);
    g_4ab9c4[14] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    g_4ab96a[13] = addView(0x4108000, drawCels, runViewScript, 9012, 6, 0, 0, 0);
    g_4ab9c4[13] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    g_4ab96a[12] = addView(0x4108000, drawCels, runViewScript, 9011, 6, 0, 0, 0);
    g_4ab9c4[12] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    placedViews[11] = addView(0x508a000, drawCels, runViewScript, 7011, 7, &g_4a10a8[12], 0, 0);
    placedViews[12] = addView(0x508a000, drawCels, runViewScript, 7012, 7, &g_4a10a8[13], 0, 0);
    placedViews[13] = addView(0x508a000, drawCels, runViewScript, 7013, 7, &g_4a10a8[14], 0, 0);
    placedViews[14] = addView(0x508a000, drawCels, runViewScript, 7014, 7, &g_4a10a8[15], 0, 0);
    moveView(placedViews[11], 1, g_4ab9c4[12]);
    moveView(placedViews[12], 1, g_4ab9c4[13]);
    moveView(placedViews[13], 1, g_4ab9c4[14]);
    moveView(placedViews[14], 1, g_4ab9c4[15]);
    for (i = 16; i < 21; i++) {
        g_4ab96a[i] = addView(0x4108000, drawCels, runViewScript, i + 8999, 6, 0, 0, 0);
        g_4ab9c4[i] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
        placedViews[i - 1] = addView(0x508a000, drawCels, runViewScript, i + 6999, 7, &g_4a10a8[i], 0, 0);
    }
    g_4ab9c4[21] = addView(0x4008000, cavesNoDraw, cavesNoUpdate, 6000, 0, 0, 0, 0);
    for (i = 0; i < g_4a1016; i++)
        g_4b83e4[i] = g_4ab96a[0];
    frameView = addView(0x4000000, drawCels, runViewScript, 6012, 0, 0, 0, 0);
    frameView = addView(0x8180000, drawCels, runViewScript, firstFrame + 1, 9, 0, 0, 0);
    fadeOutViews();
    fn_4148da(10, 236);
    loadCaveResources();
    setUpCaves();
    placeGlyphs(cavesLevel);
    g_4a101c = addView(0x8000, drawGlyphs, updateGlyphArea, 0, 0, 0, 0, 0);
    addView(0x1000, drawCavesButtons, updateCavesButtons, 0, 0, 0, 0, 0);
    enterSnoids(0);
    updateViews();
    staggerSnoids(45, 30);
    g_4a0fe6 = g_4a0fea = countChosenSnoids() >= 20;
    setGroupLists(caveGroups, 1, (short)0xc000);
    drawCavesButton(1, 0, 0);
    drawCavesButton(2, 0, 0);
    showRect(&g_4aa7b8);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    view = findView(g_4a101c);
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
    campHint((short *)(g_4a4ba0 + 0x40));
    g_4b966e = 20065;
}

/* Closes the roster screen. */
/* @zoombi32 0x0041c9ed */
void closeCaves()
{
    if (cavesOpen) {
        cavesOpen = 0;
        short saved = fn_46bee9(1);

        clearViews();
        freeGlyphShape();
        freeCaveResources();
        fn_46c602(&g_4a0fd0);
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4ab83c);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Picks the roster's features: one (two when cavesLevel is above 2) of the
   four at random, the first hair (2) when g_4a1018 asks, and for each
   g_4a0ff2 different values (1-5) at random. */
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
        g_4a0ff4 = 2;
    else
        g_4a0ff4 = 1;
    for (i = 0; i < 2; i++)
        caveFeatures[i] = 0;
    for (j = 0; j < g_4a0ff4; j++)
        for (i = 0; i < g_4a0ff2; i++)
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
            if (g_4a1018) {
                k = 2;
                g_4a1018 = 0;
            }
            caveFeatures[0] = features[k];
            for (; k < left + 1; k++)
                features[k] = features[k + 1];
            left--;
        } else {
            caveFeatures[1] = features[randomBetween(0, left)];
        }
        valuesLeft = 5;
        for (i = 0; i < g_4a0ff2; i++) {
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
        g_4a0ff4 = 2;
    else
        g_4a0ff4 = 1;
    for (i = 0; i < 6; i++)
        for (j = 0; j < 6; j++)
            caveValueCounts[i][j] = 0;
    for (j = 0; j < chosenCount; j++) {
        first = chosen->features[j][caveFeatures[0]];
        caveValueCounts[0][first]++;
        if (g_4a0ff4 > 1) {
            second = chosen->features[j][caveFeatures[1]];
            caveValueCounts[second][first]++;
        }
    }
}

/* The notify of the roster's walking Zoombini (walkerView): 1 and 2 start
   the scripts g_4a0ffe and the next (in its group); 4 moves on to the next frame
   (showFrame) and deletes the view g_4a0ffa; 5 sets g_4a1006; 10 walks
   back to the last point pushed on g_4aba14 (script 12013); 20 and 21 as
   in frameNotify. */
/* @zoombi32 0x0041d1b1 */
void walkerNotify(View *view, short event)
{
    switch (event) {
    case 1:
        startWalkerScript(view->body.group, g_4a0ffe, walkerNotify, 1);
        break;
    case 5:
        g_4a1006 = 1;
        break;
    case 2:
        startWalkerScript(view->body.group, g_4a0ffe + 1, walkerNotify, 0);
        break;
    case 4:
        fn_465175();
        g_4ab994 = 1;
        currentFrame++;
        showFrame(currentFrame);
        deleteView(g_4a0ffa);
        break;
    case 10:
        g_4aba64--;
        g_4ab8e4 = &g_4aba14[g_4aba64];
        startWalkerScript(view->body.group, 12013, walkerNotify, 0);
        break;
    case 20:
        walkerView = 0;
        if (currentFrame == finalFrame)
            g_4a0ff0 = 1;
        else
            g_4a0ff0 = 0;
        fn_465175();
        break;
    case 21:
        walkerView = 0;
        g_4a0ff0 = 1;
        break;
    }
}

/* The notify of the view frameView's frames (showFrame): 10 sets g_4ab872
   to 2; 20 notes the view done and, on its last frame, remarks (now and
   then) if not all are chosen, and sets g_4a0ff0; 21 likewise, ending the
   walk when g_4b754a. */
/* @zoombi32 0x0041d30b */
void frameNotify(View *, short event)
{
    switch (event) {
    case 10:
        g_4ab872 = 2;
        break;
    case 20:
        walkerView = 0;
        if (currentFrame == finalFrame) {
            if (countChosenSnoids() < chosenCount) {
                if (randomBetween(0, 4) > cavesLevel - 1 || (*(short *)(g_4a4ba0 + 0x40) & 0xfff) <= 3)
                    queueViewSound(randomBetween(20045, 20048), 0);
            }
            g_4a0ff0 = 1;
        } else {
            g_4a0ff0 = 0;
        }
        break;
    case 21:
        walkerView = 0;
        g_4a0ff0 = 1;
        if (g_4b754a) {
            g_4b755a = 0;
            g_4b755c = 1;
        }
        break;
    }
}

/* Draws image `image` (from the resource glyphShape; big-endian offsets
   and sizes) at place `which` (1-10), centred across it and raised by
   g_4aba6c. */
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

    handle = fn_46beac(glyphShape);
    lockHandle(handle);
    bank = (ImageBank *)handleData(handle);
    data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);
    x = xs[which] - swapShort(data[0]) / 2;
    y = ys[which] - g_4aba6c[which];
    drawImageData(data, x, y, 8);
    unlockHandle(handle);
}

/* Draws the images placed (glyphPlaced) at each of the ten places,
   after adding g_4a11ac to the region to redraw (`unused` is passed on to
   drawGlyph, which ignores it). */
/* @zoombi32 0x0041db60 */
void redrawGlyphs(long unused)
{
    short i;

    unionRgnRect(removedRgn, &g_4a11ac);
    for (i = 1; i < 11; i++)
        if (glyphPlaced[i])
            drawGlyph(i, glyphImages[i], unused);
}

/* Draws the images placed at the ten places, except at place g_4ab86c
   when cavesLevel is 1 and it's one of the first five. */
/* @zoombi32 0x0041dadf */
void drawGlyphs(View *)
{
    short i;

    if (cavesLevel == 1 && g_4ab86c < 6) {
        for (i = 1; i < 11; i++)
            if (glyphPlaced[i] && i != g_4ab86c)
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

/* Reads the roster file (into g_4a4ba0) unless the user file is the
   default one (ZBUser.txt), checks its version (107) and applies its
   settings. */
/* @zoombi32 0x0041f5d0 */
void readRoster()
{
    char name[32] = "ZBUser";

    strcat(name, ".txt");
    if (strncmp(userFile, name, strlen(userFile))) {
        readWriteRoster(g_4a4ba0, 1);
        if (swapShort(*(unsigned short *)g_4a4ba0) != 107)
            reportRosterError("Invalid user file, delete and try again: ");
        applyPlayerSettings();
        g_4b0d54 = 0;
    }
}

/* Lays out, from place firstCave on, which values of the roster's
   features each of the 21 places wants (g_4ab916): for each value of the
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
    for (v = 0; v < g_4a0ff2; v++) {
        for (i = start; i < caveValueCounts[first][caveValues[first][v]] + start; i++)
            g_4ab916[first][i] = caveValues[first][v];
        start = i;
    }
    start = firstCave;
    for (v = 0; v < g_4a0ff2; v++) {
        for (w = 0; w < g_4a0ff2; w++) {
            for (i = start; i < caveValueCounts[caveValues[second][w]][caveValues[first][v]] + start; i++)
                if (caveValues[second][w])
                    g_4ab916[second][i] = caveValues[second][w];
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
    for (i = 0; i < g_4a0ff2; i++) {
        if (viewSnoid(view)->features[caveFeatures[0]] == caveValues[0][i])
            first = caveValues[0][i];
        if (first)
            i = g_4a0ff2;
    }
    for (i = 0; i < g_4a0ff2; i++) {
        if (viewSnoid(view)->features[caveFeatures[1]] == caveValues[1][i])
            second = caveValues[1][i];
        if (second)
            i = g_4a0ff2;
    }
    for (i = firstCave; i < 21; i++)
        if (first == g_4ab916[0][i] && i == n && !spotSnoids[i]) {
            if (g_4a0ff4 <= 1)
                return i;
            if (second != g_4ab916[1][i])
                continue;
            return i;
        }
    for (i = firstCave; i < 21; i++)
        if (!spotSnoids[i] && first == g_4ab916[0][i]) {
            if (g_4a0ff4 > 1) {
                if (second == g_4ab916[1][i])
                    places[count++] = i;
            } else {
                places[count++] = i;
            }
        }
    if (count)
        return places[randomBetween(0, --count)];
    return 1;
}

/* Fills in the roster's header (g_4a4ba0): when `reset`, a new one (version
   107, default settings) with the sound slots (allocateSlot) and more reset, else the
   player's current settings; then the scene (or, in scene 2, g_4b0d54). */
/* @zoombi32 0x0041f6fc */
void fillRosterHeader(short reset)
{
    if (reset) {
        fillMemory(g_4a4ba0, 0, 0xae05);
        *(unsigned short *)g_4a4ba0 = swapShort(107);
        *(unsigned short *)(g_4a4ba0 + 2) = swapShort(30);
        g_4a4ba0[4] = 1;
        g_4a4ba0[5] = 1;
        g_4a4ba0[6] = 1;
        g_4a4ba0[7] = 1;
        g_4a4ba0[8] = 0;
        g_4a4ba0[9] = 0;
        *(short *)(g_4a4ba0 + 0xa) = 0;
        *(short *)(g_4a4ba0 + 0x20) = g_4b2b00;
        g_4afb32 = 1;
        g_4afb30 = 0;
        g_4b807e = 0;
        g_4a75e4 = g_4a7600 = g_4a7614 = g_4a7628 = g_4a763c = 0;
        g_4a764c = g_4a7658 = g_4a7668 = g_4a78c4 = g_4a78c8 = 0;
        g_4a78cc = g_4a78d0 = g_4a78d4 = g_4a78d8 = 0;
        g_4a78dc = 0;
        g_4abafc = 0;
        g_4abb00 = 0;
    } else {
        *(unsigned short *)(g_4a4ba0 + 2) = swapShort(clickTime);
        g_4a4ba0[4] = g_4b87fe;
        g_4a4ba0[5] = g_4b87ff;
        g_4a4ba0[6] = clickToDragOption;
        g_4a4ba0[7] = hideDragCursor;
        g_4a4ba0[8] = g_4b8803;
        g_4a4ba0[9] = dragClicks;
        *(short *)(g_4a4ba0 + 0xa) = g_4b0d4a;
    }
    *(short *)(g_4a4ba0 + 0xcc) = currentScene;
    *(short *)(g_4a4ba0 + 0xca) = g_4b0d56;
    if (currentScene == 2)
        *(short *)(g_4a4ba0 + 0xcc) = g_4b0d54;
}

/* Reads or writes the list of saved games (the file rosterFileName in the
   directory g_4b29d4) into or from `list` (or a list of its own): creates
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
    strcpy(path, g_4b29d4);
    strcat(path, rosterFileName);
    result = openRosterFile(path, result);
    if (result == 2)
        reportRosterError("Could not Open/Create Roster file.");
    if (result == 2)
        return;
    if (result == 1) {
        fillMemory(games, 0, size);
        games->version = 107;
        if (writeFile(g_4aba7c, games, &size))
            reportRosterError("Problem writing file: disk may be full");
    } else {
        if (seekFile(g_4aba7c, 0, 0) == -1)
            reportRosterError("Seek Error");
        switch (mode) {
        case 0:
            error = readFile(g_4aba7c, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107)
                savedGames = games->count;
            else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 1:
            error = readFile(g_4aba7c, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107) {
                if (seekFile(g_4aba7c, 0, 0) == -1)
                    reportRosterError("Seek Error");
                games->count = savedGames;
                if (writeFile(g_4aba7c, games, &size))
                    reportRosterError("Problem writing file: disk may be full");
            } else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 2:
            error = readFile(g_4aba7c, games, &size);
            if (error && error != 10303)
                reportRosterError("Problem reading file");
            else if (games->version == 107) {
                savedGames = games->count;
                nextSaveId = games->nextId;
            } else
                reportRosterError("Delete the file 'Zoombini.who' and try again!");
            break;
        case 3:
            if (writeFile(g_4aba7c, games, &size))
                reportRosterError("Problem writing file: disk may be full");
            break;
        }
    }
    closeFile(g_4aba7c, 0);
}

/* Walks the roster's next Zoombini (g_4ab9c2) on: 0 from the view
   g_4ab8da (script g_4a0ffc on, for frame g_4a1010, with a view of its
   own in front, g_4a0ffa), 1 likewise from g_4ab8dc (for frame g_4a1012,
   after claimSpot), 2 off toward the view frameView (script 12012). */
/* @zoombi32 0x0041cf14 */
void walkNext(short which)
{
    View *view;

    if (g_4ab9c2) {
        walkerView = g_4ab9c2;
        switch (which) {
        case 0:
            view = findView(g_4ab8da);
            break;
        case 1:
            view = findView(g_4ab8dc);
            break;
        case 2:
            view = findView(walkerView);
            break;
        }
        if (view) {
            switch (which) {
            case 0:
                g_4ab8e4 = 0;
                setViewScript(view, (g_4a1010 - 1) * 4 + g_4a0ffc, 1);
                view->notify = walkerNotify;
                deleteView(g_4a0ffa);
                moveView(walkerView, 1, g_4ab9c4[g_4a1010]);
                g_4a0ffa = addView(0x4108000, drawCels, runViewScript, (g_4a1010 - 1) * 4 + g_4a0ffc + 1, 6,
                                   0, 1, walkerView);
                groupViews(view->id, g_4a0ffa, 0, 0, 0, 0);
                break;
            case 1:
                claimSpot(walkerView, g_4a1012);
                g_4ab8e4 = &g_4ab8e0;
                setViewScript(view, (g_4a1012 - 1) * 4 + g_4a0ffc + 2, 1);
                view->notify = walkerNotify;
                deleteView(g_4a0ffa);
                moveView(walkerView, 1, g_4ab9c4[g_4a1012]);
                g_4a0ffa = addView(0x4108000, drawCels, runViewScript, (g_4a1012 - 1) * 4 + g_4a0ffc + 3, 6,
                                   0, 1, walkerView);
                groupViews(view->id, g_4a0ffa, 0, 0, 0, 0);
                break;
            case 2:
                setViewsLocked(0);
                g_4ab8e4 = 0;
                viewSnoid(view)->unknownF2 = 0;
                view->notify = walkerNotify;
                startSnoidScript(viewSnoid(view), 12012, g_4ab8e4, 1);
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
    for (i = 0; i < g_4a0ff4; i++) {
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
        for (j = 0; j < g_4a0ff2; j++) {
            drawFeatureImage(caveFeatures[i] + 1, caveValues[i][j], rect);
            rect.left += 30;
            rect.right = rect.left + 25;
        }
        if (!i) {
            rect = firstCounts;
            fillPortRect(rect, Color(14), 0);
            rect.left += 10;
            rect.top += 5;
            for (j = 0; j < g_4a0ff2; j++) {
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
   roster screen (chosenSpots), resets the view g_4a101c, and clears the
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
    view = findView(g_4a101c);
    if (view)
        view->reset = 1;
    unionRgnRect(removedRgn, &g_4a11ac);
    mainLoopEvents();
    for (i = firstCave; i < 21; i++) {
        spotSnoids[i] = 0;
        g_4b83e4[i - 1] = 0;
    }
    for (i = 0; i < g_4a1016; i++)
        g_4b83e4[i] = g_4ab96a[0];
    resetCavesScreen();
    g_4a0fea = 0;
    g_4a0fe8 = 0;
    g_4a0ff0 = 0;
    g_4ab870 = 0;
    g_4a100e = 0;
}

/* Saves the roster (with the player's settings, fillRosterHeader) if it changed
   (g_4afb32) and the user file isn't the default one (ZBUser.txt). */
/* @zoombi32 0x0041f551 */
void saveRoster()
{
    char name[32] = "ZBUser";

    strcat(name, ".txt");
    if (strncmp(userFile, name, strlen(userFile)) && g_4afb32) {
        if (g_4a4ba0) {
            fillRosterHeader(0);
            readWriteRoster(g_4a4ba0, 0);
        }
        g_4afb32 = 0;
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
                g_4a1012 = pickCave(view->id, 0);
                placedViewPoint(&g_4ab8e0, g_4a1012);
                setSnoidAction(viewSnoid(view), 5, &g_4ab8e0);
                spotSnoids[g_4a1012] = view->id;
            }
        }
    other = findView(g_4a101c);
    if (other)
        other->reset = 1;
    for (i = firstCave; i < 21; i++)
        if (spotSnoids[i])
            claimPlacedView(i, spotSnoids[i]);
        else
            g_4b83e4[i] = 0;
    for (i = 0; i < g_4a1016; i++)
        g_4b83e4[i] = g_4ab96a[0];
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
            g_4a1010 = i;
            g_4a1012 = pickCave(placed[i], g_4a1010);
            placedViewPoint(&g_4ab8e0, g_4a1012);
            setSnoidAction((Snoid *)&findView(placed[i])->body, 5, &g_4ab8e0);
            spotSnoids[g_4a1012] = placed[i];
        }
    for (i = firstCave; i < 21; i++)
        if (spotSnoids[i])
            claimPlacedView(i, spotSnoids[i]);
        else
            g_4b83e4[i - 1] = 0;
    for (i = 0; i < g_4a1016; i++)
        g_4b83e4[i] = g_4ab96a[0];
    placeGlyphs(cavesLevel);
    view = findView(g_4a101c);
    if (view)
        view->reset = 1;
    unionRgnRect(removedRgn, &cavesButtons[0].rect);
}

/* The roster screen's clicks: button 1 asks whether to keep the party,
   button 2 (once any Zoombini is placed) goes on; otherwise a Zoombini
   not yet placed is dragged to a place (walked to the right one if it
   doesn't belong there) or, dropped outside the places, walked back. A
   click after one of the buttons (g_4b0d52) leaves the screen. */
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

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
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
        g_4b755c = 1;
        g_4b0d52 = 1;
        askKeepParty();
        break;
    case 2:
        if (g_4a0fe8) {
            if (!g_4ab872) {
                g_4ab870 = 1;
                g_4a0ff0 = 1;
            }
            drawCavesButton(which, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawCavesButton(which, 0, 1);
            markPlacedSnoids();
            g_4b0d52 = 17;
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
            if (free == 1 && !g_4a0ff0 && g_4b755a <= 0) {
                from = *(Point *)&view->body.x;
                dragSnoid(view, where, 0, 0);
                g_4a1010 = heldPlaceNumber();
                if (g_4a1010) {
                    snoid = viewSnoid(view);
                    snoid->unknownF7 = 1;
                    view->flags = 0x4008001;
                    if ((g_4a1012 = pickCave(view->id, g_4a1010)) == g_4a1010) {
                        spotSnoids[g_4a1010] = view->id;
                        snoid->unknownF1 = g_4a11b4[g_4a1010];
                        snoid->unknownF2 = g_4a11de[g_4a1010];
                        g_4a100e++;
                        walkerView = 0;
                        if (g_4a100e == 1) {
                            g_4a0fe8 = 1;
                            unionRgnRect(removedRgn, &cavesButtons[1].rect);
                            g_4ab996[g_4ab9be] = view->id;
                            g_4ab9be++;
                            g_4ab8e8 = &g_4a10a8[g_4a1010];
                        } else if (g_4a100e == chosenCount) {
                            g_4aba08 = 1;
                            g_4a0ff0 = 1;
                            queueViewSound(randomBetween(20055, 20063), 0);
                        } else {
                            g_4ab996[g_4ab9be] = view->id;
                            g_4ab9be++;
                            g_4ab8e8 = &g_4a10a8[g_4a1010];
                        }
                        moveView(view->id, 1, g_4ab9c4[g_4a1010]);
                    } else {
                        spotSnoids[g_4a1012] = view->id;
                        g_4ab9c2 = view->id;
                        g_4ab8da = g_4ab96a[g_4a1010];
                        g_4ab8dc = g_4ab96a[g_4a1012];
                        g_4a100e++;
                        g_4a0ff0 = 1;
                        g_4ab876 = 1;
                        releaseHeldPlace();
                        if (g_4a100e == 1) {
                            g_4a0fe8 = 1;
                            unionRgnRect(removedRgn, &cavesButtons[1].rect);
                        } else if (g_4a100e == chosenCount) {
                            queueViewSound(randomBetween(20055, 20063), 0);
                        }
                    }
                } else {
                    for (i = 0, found = 0; i < 12; i++)
                        if (ptInRect(&g_4a10fc[i], *(Point *)&view->body.x)) {
                            found = i;
                            i = 12;
                        }
                    if (!found) {
                        g_4a0ff0 = 1;
                        g_4ab9c2 = view->id;
                        g_4aba14[g_4aba64] = from;
                        g_4aba64++;
                        walkNext(2);
                    }
                }
            }
        }
        break;
    }
}

/* The roster screen's frame (scenes[]): leaves the screen after a button
   (g_4b0d52) once the sound and the Zoombinis are done, starts the walk
   (g_4ab870), shows the frames' changes (g_4ab994), walks Zoombinis on
   (walkNext), blinks the place g_4ab86c, has placed Zoombinis (g_4ab996)
   and, when all are placed (g_4aba08), the others cheer, and plays the
   ambient sounds. */
/* @zoombi32 0x0041ca44 */
void cavesFrame()
{
    View *view;
    Snoid *snoid;
    short started;
    short i;
    short n;

    if (!g_4a1208 && cavesOpen) {
        g_4a1208 = 1;
        updateViews();
        if (g_4b0d52) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                g_4a1208 = 0;
                return;
            }
            if (!g_4b9688 || g_4b9688 == 3) {
                if (g_4b9688 == 3)
                    chooseSnoids(0, 0);
                if ((viewsLocked || !g_4b755a || g_4b755c >= 1)
                    && (g_4ab872 == 3 || g_4b0d52 == 1 || viewsLocked)) {
                    g_4b0d50 = g_4b0d52;
                    g_4b0d52 = 0;
                    fn_46be2e(0);
                    closeCaves();
                    g_4a1208 = 0;
                    return;
                }
                if (g_4ab872 == 2) {
                    g_4ab872 = 3;
                    queueViewSound(996, 0);
                    sendReadyOff(660, 376, 30);
                }
            } else if (g_4b9688 == 2) {
                g_4b9688 = 0;
                g_4b0d52 = 0;
            }
        }
        if (g_4b9684) {
            playAmbientSound();
            g_4a1208 = 0;
            return;
        }
        if (g_4ab870 && !walkerView) {
            g_4ab870 = 0;
            g_4ab872 = 1;
            setViewsLocked(0);
            view = removeView(g_4ab9c4[24], 0);
            if (view) {
                g_4a0fea = 1;
                setViewScript(view, 6002, 1);
                view->notify = frameNotify;
                insertViewAtEnd(view);
            }
        }
        if (g_4ab994) {
            g_4ab994 = 0;
            view = findView(g_4ab8da);
            if (view) {
                setViewScript(view, g_4a1010 + 8999, 0);
                view->body.running = 1;
            }
            view = findView(g_4ab8dc);
            if (view) {
                setViewScript(view, g_4a1012 + 8999, 0);
                view->body.running = 1;
            }
            view = findView(walkerView);
            if (view) {
                snoid = viewSnoid(view);
                snoid->unknownF1 = g_4a11b4[g_4a1012];
                snoid->unknownF2 = g_4a11de[g_4a1012];
            }
        }
        if (g_4ab876) {
            g_4ab876 = 0;
            setViewsLocked(0);
            walkNext(0);
        }
        if (g_4a1006) {
            g_4a1006 = 0;
            walkNext(1);
        }
        if (cavesLevel == 1 && g_4ab86c < 6) {
            view = findView(g_4a101c);
            if (view && clockTime() >= view->nextUpdate) {
                if (!g_4ab86c) {
                    view->nextUpdate = clockTime() + 30;
                    unionRgnRect(removedRgn, &g_4a11ac);
                    g_4ab86e++;
                    g_4ab86c = g_4ab86e;
                } else {
                    view->nextUpdate = clockTime() + 30;
                    unionRgnRect(removedRgn, &g_4a11ac);
                    g_4ab86c = 0;
                }
            }
        }
        while (g_4ab9be) {
            view = findView(g_4ab996[--g_4ab9be]);
            if (view) {
                view->flags = 0x4008001;
                snoid = viewSnoid(view);
                startSnoidScript(viewSnoid(view), snoid->features[3] + 12999, g_4ab8e8, 0);
                view->notifyEnd = 1;
                view->notify = (ViewNotify)cheerNotify;
                g_4ab8e8 = 0;
            }
        }
        if (g_4aba08 && g_4aba06 < g_4aba04) {
            if (clockTime() - g_4ab9fc > 30) {
                started = 0;
                g_4ab9fc = clockTime();
                for (i = 0; i < chosenCount && !started; i++) {
                    n = allocateSlot(&g_4aba00, chosenCount, 0);
                    if (partyViews[n]) {
                        view = idleSnoidView(partyViews[n]);
                        /* The original's test is always true: == binds before |. */
                        if (view && view->body.running && ((view->flags == 0x8000) | 0x4000001)) {
                            snoid = viewSnoid(view);
                            startSnoidScript(viewSnoid(view), snoid->features[3] + 12999, 0, 0);
                            view->notifyEnd = 1;
                            view->notify = (ViewNotify)cheerNotify;
                            g_4aba06++;
                            started = 1;
                        }
                    }
                }
            }
        } else if (g_4aba06 >= g_4aba04) {
            g_4aba06 = g_4aba08 = g_4ab9fc = g_4aba00 = 0;
        }
        playAmbientSound();
        g_4a1208 = 0;
    }
}

/* The roster screen's keys (with debugging on, g_4b8803, or else only
   0x16f): 1-4 set the level (cavesLevel) and show it, L shows it, space
   resets the frames (resetCavesScreen), 0x171 walks the Zoombinis to their spots,
   0x173-0x176 change the first feature, 0x16f (below level 4) calls
   fn_466b93. Returns whether the key was used. */
/* @zoombi32 0x0041dd83 */
short cavesKey(unsigned short key)
{
    Color saved;
    char digits[32] = "01234";
    char level[32] = "Level x ";
    short shown = 0;
    ShortRect rect = {275, 0, 375, 18};
    short used;

    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case '1':
    case '2':
    case '3':
    case '4':
        cavesLevel = key - '0';
        changeCaveFeature(-1);
        redrawGlyphs(1);
        if (g_4a101a)
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
            fn_466b93();
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
   the roster file next to the program (userFile). */
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
    strcat(path, userFile);
    result = openRosterFile(path, result);
    if (result == 2)
        reportRosterError("Could not Open/Create Roster file.");
    if (result != 2) {
        if (seekFile(g_4aba7c, 0, 0) == -1)
            reportRosterError("Seek Error");
        if (read) {
            if (readFile(g_4aba7c, data, &size))
                reportRosterError("Problem reading file");
        } else if (writeFile(g_4aba7c, data, &size)) {
            reportRosterError("Problem writing file: disk may be full");
        }
        closeFile(g_4aba7c, 0);
    }
}

/* Resets the roster screen's state for mode `which` (1-4: the frames
   firstFrame-finalFrame it shows). */
/* @zoombi32 0x0041dfe3 */
void resetCavesState(short which)
{
    g_4ab870 = 0;
    g_4ab872 = 0;
    g_4aba64 = 0;
    walkerView = 0;
    g_4ab86c = 0;
    g_4ab86e = 0;
    g_4a100e = 0;
    g_4a1010 = 0;
    g_4a1012 = 0;
    g_4a0ff2 = 5;
    g_4a0ff4 = 1;
    g_4a101a = 0;
    g_4a0ffa = 0;
    g_4a0ff8 = 0;
    currentFrame = 0;
    g_4a0fe4 = 0;
    g_4a0ff0 = 0;
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
