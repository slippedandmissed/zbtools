/*
 * picker (0x42f920-0x433510): 'Picker.MHK', 'New Game', 'Snoids to practice with = '
 */

#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "picker.h"
#include "platform.h"
#include "roster.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Opens scene 1, the map: its sounds, backdrop and saved areas, the
   map's views (their placed callbacks), and loads sounds 998-999 from
   the sounds' map. */
/* @zoombi32 0x0042fa4b */
void openScene1()
{
    basePort *port;
    short i;
    View *view;
    long saved;

    g_4afb14 = 0;
    resetMap();
    addSoundRange(20000, 29999, 1);
    party()->count = 0;
    openGameFile(&g_4afb10, "Map.MHK");
    setCurrentMap(g_4afb10);
    drawBackdrop(300);
    port = getPort();
    setPort(viewPort);
    for (i = 0; i < 6; i++)
        saveRect(&g_4afb18[i], &g_4a1f54[i], 1, 0);
    setPort(port);
    loadFeatureGroup(1000, 0, 0);
    loadScripts(1000, 6);
    copyPaletteRange(10, 236);
    g_4afb3a = addView(0x8108000, drawCels, runViewScript, 1000, 6, 0, 0, 0);
    g_4afb3c = addView(0x8108000, drawCels, runViewScript, 1001, 6, 0, 0, 0);
    view = findView(g_4afb3a);
    if (view)
        view->placed = placeOpenHotspots;
    view = findView(g_4afb3c);
    if (view)
        view->placed = placeHotspotLevels;
    g_4afb3e = addView(0x100000, drawTextView, updateTextView, 1004, 6, 0, 0, 0);
    view = findView(g_4afb3e);
    if (view)
        view->body.running = 0;
    g_4afb40 = addView(0x1000, drawCels, runViewScript, 1005, 3, 0, 0, 0);
    setGroupLists(pickerGroups, 1, (short)0xc000);
    makeMapViews(1);
    view = findView(g_4afb40);
    if (view) {
        g_4afb42 = view->body.bounds;
        view->placed = placePressed;
    }
    showRect(&g_4aa7b8);
    fadeInViews();
    g_4afb14 = 1;
    g_4a7410 = 0;
    saved = currentMapFile;
    setCurrentMap(g_4b7b4c);
    loadSoundByKey(998, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(999, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    currentMapFile = saved;
}

/* A view's placed callback: with interval 2, shows the image before its
   first cel's, a pixel right and two down. */
/* @zoombi32 0x0042fc89 */
void placePressed(View *view)
{
    ViewCel *cel = view->body.cels;

    if (view->interval == 2) {
        cel->image--;
        cel->x++;
        cel->y += 2;
    }
}

/* @zoombi32 0x004320da */
long catchKey(long)
{
    return 0;
}

/* The notify of a target bursting (scene20Frame): when its script ends,
   marks it done (targetBursting positive). */
/* @zoombi32 0x004334f0 */
void burstNotify(View *, short event)
{
    if (event == -1 && targetBursting < 0)
        targetBursting = -targetBursting;
}

/* Opens scene 19, catching Zoombinis: 9 throws of 99, the views, and
   the opening line. */
/* @zoombi32 0x0043169b */
void openScene19()
{
    Point at;
    short i;
    View *view;

    g_4b0d52 = 0;
    pickerData.game.leave.left = pickerData.game.leave.top = 7;
    pickerData.game.leave.right = pickerData.game.leave.bottom = 41;
    pickerData.game.speed = 8;
    for (i = 0; i < 3; i++)
        g_4afb60[i] = 0;
    pickerData.game.caught = 0;
    g_4afb14 = 0;
    pickerData.game.overView = g_4afb3a = g_4afb3c = g_4afb3e = 0;
    pickerData.game.count = 0;
    pickerData.game.throws = 9;
    pickerData.game.remaining = 99 - pickerData.game.count - pickerData.game.throws;
    pickerData.game.streak = 0;
    g_4afb68 = 0;
    openGameFile(&g_4afb10, "Picker.MHK");
    setCurrentMap(g_4afb10);
    drawBackdrop(1001);
    loadFeatureGroup(1100, 0, 0);
    loadFeatureGroup(1200, 1, 1);
    loadScripts(1100, 6);
    addScripts(1200, 5, 0);
    loadSoundByKey(1200, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1201, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    for (i = 1100; i <= 1104; i++)
        addView(0, drawCels, runViewScript, i, 0, 0, 0, 0);
    at.x = 14;
    at.y = 13;
    pickerData.game.againView = addView(0x801000, drawCels, updateCursorView, 1105, 1, &at, 0, 0);
    at.x = 0;
    at.y = 480;
    g_4afb3c = addView(0x1980000, drawCels, runViewScript, 1200, 1, &at, 0, 0);
    g_4afb3e = addView(0x1981000, drawCels, runViewScript, 1201, 1, &at, 0, 0);
    addView(0, drawCels, runViewScript, 1202, 0, 0, 0, 0);
    levelListView = addView(0x100000, drawCels, runViewScript, 1203, 1, 0, 0, 0);
    view = findView(levelListView);
    if (view)
        view->placed = placeCatchScore;
    copyPaletteRange(10, 236);
    updateViews();
    hideCursor();
    setGroupLists(catchGroups, 1, (short)0xc000);
    showRect(&g_4aa7b8);
    fadeInViews();
    g_4afb14 = 1;
    queueViewSound(30025, 0);
}

/* Closes scenes 19 and 21 (Picker.MHK). */
/* @zoombi32 0x0043190d */
void closeScene19()
{
    if (g_4afb14) {
        showCursor();
        g_4afb14 = 0;
        short saved = setFreeAtOnce(1);

        removeDeadViews();
        clearViews();
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&g_4afb10);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Opens scene 20, the targets: the state, the backdrop (filled first),
   the views and sounds, and the opening line. */
/* @zoombi32 0x004323cc */
void openScene20()
{
    short i;
    View *view;

    g_4b0d52 = 0;
    g_4afbbe = 0;
    g_4afbbc = 0;
    g_4afbb8 = g_4a4b98;
    g_4a4b98 = 0;
    for (i = 0; i < 6; i++)
        targetBounds[i] = 0;
    g_4afb7a = g_4afb78 = targetScore = g_4afbba = 0;
    shipsLeft = 3;
    g_4afb74 = 100;
    setViewsLocked(0);
    shotsStarted = firstShotStopped = targetHit = targetBursting = 0;
    g_4afb14 = 0;
    openGameFile(&g_4afb10, "Picker.MHK");
    setCurrentMap(g_4afb10);
    drawBackdrop(2000);
    loadFeatureGroup(1000, 0, 1);
    loadScripts(1000, 31);
    fillPortRect(Rect(gameRect), Color(44), 0);
    copyBits(viewPort, workPort, &gameRect);
    resetShip();
    addView(0, drawCels, runViewScript, 1012, 6, 0, 0, 0);
    g_4afb78 = addView(0x100000, drawCels, runViewScript, 1014, 6, 0, 0, 0);
    view = findView(g_4afb78);
    if (view)
        view->placed = placeTargetScore;
    loadSoundByKey(3000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(3001, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(3002, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    copyPaletteRange(10, 236);
    setGroupLists(targetGroups, 1, (short)0xc000);
    showRect(&g_4aa7b8);
    fadeInViews();
    g_4afb14 = 1;
    queueViewSound(30035, 0);
}

/* Closes scene 20 (Picker.MHK), keeping g_4afbb8 in g_4a4b98. */
/* @zoombi32 0x004325c4 */
void closeScene20()
{
    if (g_4afb14) {
        g_4a4b98 = g_4afbb8;
        requestViewSort();
        g_4afb14 = 0;
        short saved = setFreeAtOnce(1);

        removeDeadViews();
        clearViews();
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&g_4afb10);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Copies g_4a1f74 from the port *g_4afb28 to the screen and starts the
   view mapBoxView on script 1002. */
/* @zoombi32 0x0043151e */
void showMapBox()
{
    copyPortBits(viewPort, *g_4afb28, g_4a1f74, g_4a1f74, 0);
    startView(mapBoxView, 1002, 0, 0);
}

/* A notify: 0 deletes the view caught (pickerData.game.caught); at the end
   (-1), g_4afb3a (2
   calls requestViewSort) is cleared. */
/* @zoombi32 0x00431e5e */
void caughtNotify(View *, short event)
{
    short id;

    switch (event) {
    case 0:
        id = pickerData.game.caught;
        pickerData.game.caught = 0;
        deleteView(id);
        break;
    case -1:
        if (g_4afb3a == 2)
            requestViewSort();
        g_4afb3a = 0;
        break;
    }
}

/* A view's placed callback: raises cel g_4b754a (1-4) by four images and
   moves it two pixels up and left. */
/* @zoombi32 0x00430f8e */
void placeLevelMarker(View *view)
{
    ViewBody *body = &view->body;

    switch (g_4b754a) {
    case 1:
        body->cels[1].image += 4;
        body->cels[1].x += -2;
        body->cels[1].y += -2;
        break;
    case 2:
        body->cels[2].image += 4;
        body->cels[2].x += -2;
        body->cels[2].y += -2;
        break;
    case 3:
        body->cels[3].image += 4;
        body->cels[3].x += -2;
        body->cels[3].y += -2;
        break;
    case 4:
        body->cels[4].image += 4;
        body->cels[4].x += -2;
        body->cels[4].y += -2;
        break;
    }
}

/* A drifting view's placed callback: moves it by its speed, wrapping round
   the screen (-10 to 650 across, -10 to 490 down), and puts its cel
   there. */
/* @zoombi32 0x00433388 */
void driftView(View *view)
{
    DriftingBody *body = (DriftingBody *)&view->body;

    body->x += body->dx;
    body->y += body->dy;
    if (body->x > 650)
        body->x = -10;
    else if (body->x < -10)
        body->x = 650;
    if (body->y > 490)
        body->y = -10;
    else if (body->y < -10)
        body->y = 490;
    body->cels[0].x = body->x;
    body->cels[0].y = body->y;
}

/* Resets shipDirection-g_4afb88 (g_4afb82, g_4afb84: the screen's centre) and
   makes the view shipView again (script 1010, placed callback
   placeShip). */
/* @zoombi32 0x00432905 */
void resetShip()
{
    View *view;

    g_4afb80 = 0;
    g_4afb82 = 320;
    g_4afb84 = 240;
    shipDirection = 0;
    g_4afb86 = g_4afb88 = 0;
    deleteView(shipView);
    shipView = addView(0, drawCels, runViewCels, 1010, 4, 0, 0, 0);
    view = findView(shipView);
    if (view)
        view->placed = placeShip;
}

/* Draws a view with its cels and, in colour 45, its text (kept in its
   body from +0x3c) centred in its bounds, when it's running and to be
   redrawn. */
/* @zoombi32 0x00430ff2 */
void drawTextView(View *view)
{
    Color saved;

    if (view->body.running && view->reset) {
        view->reset = 0;
        drawCels(view);
        saved = setForeColor(Color(45));
        drawText(view->body.bounds, 0x22, (char *)&view->body.cels[10], 0xffff);
        setForeColor(saved);
    }
}

/* A view's placed callback: shows three two-digit numbers in its first six
   cels (images counted from the first cel's): the most in the roster
   (+0x22, raised to pickerData.game.count if need be; that wraps
   at 100), pickerData.game.count and pickerData.game.throws. */
/* @zoombi32 0x004320e3 */
void placeCatchScore(View *view)
{
    ViewBody *body;
    short first;
    short tens;

    if (pickerData.game.count > 99)
        pickerData.game.count = 0;
    if (*(short *)(g_4a4ba0 + 0x22) < pickerData.game.count)
        *(short *)(g_4a4ba0 + 0x22) = pickerData.game.count;
    body = &view->body;
    first = body->cels[0].image;
    tens = *(short *)(g_4a4ba0 + 0x22) / 10;
    body->cels[0].image = first + tens;
    body->cels[1].image = *(short *)(g_4a4ba0 + 0x22) - tens * 10 + first;
    tens = pickerData.game.count / 10;
    body->cels[2].image = first + tens;
    body->cels[3].image = pickerData.game.count - tens * 10 + first;
    tens = pickerData.game.throws / 10;
    body->cels[4].image = first + tens;
    body->cels[5].image = pickerData.game.throws - tens * 10 + first;
}

/* Resets the picker: its counts and state, and the hotspots, 40 by 30
   around sixteen points, and the whole screen. */
/* @zoombi32 0x0042f920 */
void resetMap()
{
    Point points[16] = {{60, 400},  {170, 343}, {160, 261}, {222, 313}, {236, 269}, {234, 146},
                        {286, 82},  {362, 185}, {321, 345}, {427, 352}, {484, 325}, {442, 211},
                        {490, 182}, {487, 95},  {540, 125}, {578, 65}};
    short i;

    g_4afb16 = 0;
    g_4afb5e = 0;
    for (i = 0; i < 6; i++)
        g_4afb18[i] = 0;
    g_4b0d52 = 0;
    levelListView = mapBoxView = pickedHotspot = 0;
    g_4afb36 = -1;
    for (i = 0; i <= 15; i++) {
        pickerData.hotspots[i].rect.left = points[i].x - 20;
        pickerData.hotspots[i].rect.right = points[i].x + 20;
        pickerData.hotspots[i].rect.top = points[i].y - 15;
        pickerData.hotspots[i].rect.bottom = points[i].y + 15;
    }
    pickerData.hotspots[16].rect.left = 0;
    pickerData.hotspots[16].rect.top = 0;
    pickerData.hotspots[16].rect.right = 640;
    pickerData.hotspots[16].rect.bottom = 480;
    g_4afb3a = g_4afb3c = g_4afb3e = g_4afb40 = 0;
    g_4afb42 = noRect;
}

/* Picks hotspot `n` (1-16; -1 for none) as pickedHotspot and redraws the view
   g_4afb3a: with g_4b754a any of them, except 5, 12 and 16 until the
   roster says they're open (+0x50, +0x52, +0x51); otherwise only 1, 5, 12
   and 16. */
/* @zoombi32 0x00430030 */
void pickHotspot(short n)
{
    View *view;
    short picked;
    short allowed;

    picked = 0;
    if (n >= 1 && n <= 16) {
        if (g_4b754a) {
            allowed = 1;
            switch (n) {
            case 16:
                if (!(g_4a4ba0[0x51] & 0xf))
                    allowed = 0;
                break;
            case 12:
                if (!(g_4a4ba0[0x52] & 0xff))
                    allowed = 0;
                break;
            case 5:
                if (!(g_4a4ba0[0x50] & 0xf))
                    allowed = 0;
                break;
            }
            if (allowed) {
                pickedHotspot = n;
                picked = 1;
            }
        } else if (n == 1 || n == 5 || n == 12 || n == 16) {
            pickedHotspot = n;
            picked = 1;
        }
    } else if (n == -1) {
        pickedHotspot = 0;
        picked = 1;
    } else {
        picked = 0;
    }
    if (picked) {
        view = startView(g_4afb3a, 0, 0, 0);
        if (view)
            view->reset = 1;
    }
}

/* A view's placed callback: shows two three-digit numbers in its first six
   cels (images counted from the first cel's), the most in the roster
   (+0x24, raised to targetScore if need be; that wraps at 1000) and targetScore,
   and in its seventh the image shipsLeft on. */
/* @zoombi32 0x004333ef */
void placeTargetScore(View *view)
{
    ViewBody *body;
    short first;
    short digit;
    short counted;

    if (targetScore > 999)
        targetScore = 0;
    if (*(short *)(g_4a4ba0 + 0x24) < targetScore)
        *(short *)(g_4a4ba0 + 0x24) = targetScore;
    body = &view->body;
    first = body->cels[0].image;
    digit = *(short *)(g_4a4ba0 + 0x24) / 100;
    body->cels[0].image = first + digit;
    counted = digit * 100;
    digit = (*(short *)(g_4a4ba0 + 0x24) - counted) / 10;
    body->cels[1].image = first + digit;
    counted += digit * 10;
    body->cels[2].image = *(short *)(g_4a4ba0 + 0x24) - counted + first;
    digit = targetScore / 100;
    body->cels[3].image = first + digit;
    counted = digit * 100;
    digit = (targetScore - counted) / 10;
    body->cels[4].image = first + digit;
    counted += digit * 10;
    body->cels[5].image = targetScore - counted + first;
    body->cels[6].image = first + shipsLeft;
}

/* Starts a drifting view (script 1011, placed callback placeShot) from the
   centre view's place (g_4afb82, g_4afb84), moving 14 a step in direction
   shipDirection (0 up, clockwise in eighths), and counts it (shotsStarted). */
/* @zoombi32 0x0043297f */
short fireShot()
{
    short dx;
    short dy;
    short x;
    short y;
    short id;
    View *view;
    DriftingBody *body;

    dy = 0;
    dx = 0;
    switch (shipDirection) {
    case 0:
        dy = -14;
        break;
    case 1:
        dy = -14;
        dx = 14;
        break;
    case 2:
        dx = 14;
        break;
    case 3:
        dy = 14;
        dx = 14;
        break;
    case 4:
        dy = 14;
        break;
    case 5:
        dy = 14;
        dx = -14;
        break;
    case 6:
        dx = -14;
        break;
    case 7:
        dy = -14;
        dx = -14;
        break;
    }
    x = g_4afb82 + dx;
    y = g_4afb84 + dy;
    id = addView(0, drawCels, runViewCels, 1011, 3, 0, 0, 0);
    view = findView(id);
    if (view) {
        body = (DriftingBody *)&view->body;
        body->unknown28 = 0;
        body->x = x;
        body->y = y;
        body->dx = dx;
        body->dy = dy;
        view->placed = placeShot;
        shotsStarted++;
    }
    return id;
}

/* A view's placed callback: keeps its cels for the hotspots open
   (openHotspots, 11 also by 16; 4, 11 and 15 always with g_4b754a), with the
   one picked (pickedHotspot) lit (93 images on; for 1, image 109), and drops
   the others. */
/* @zoombi32 0x00430cb3 */
void placeOpenHotspots(View *view)
{
    short images[18];
    ViewCel *cel;
    short i;

    for (i = 1; i <= 10; i++)
        if (!openHotspots[i])
            images[i] = 0;
        else
            images[i] = i;
    i = 11;
    if (!openHotspots[11] && !openHotspots[16])
        images[i] = 0;
    else
        images[i] = i;
    for (i = 12; i <= 15; i++)
        if (!openHotspots[i])
            images[i] = 0;
        else
            images[i] = i;
    if (g_4b754a) {
        images[4] = 4;
        images[11] = 11;
        images[15] = 15;
    }
    images[16] = 0;
    if (pickedHotspot) {
        if (pickedHotspot == 1)
            images[16] = 109;
        else if (images[pickedHotspot - 1])
            images[pickedHotspot - 1] += 93;
    }
    cel = view->body.cels;
    while (cel->image)
        if (images[cel->image]) {
            cel->image = images[cel->image];
            cel++;
        } else {
            removeFirstCel(cel);
        }
}

/* A view's placed callback: keeps its cels 17-32 for the hotspots open
   (openHotspots, as placeOpenHotspots orders them; all with g_4b754a), each moved on
   16 images for each level past the first (g_4b754a, else the hotspot's
   own), and drops the others. */
/* @zoombi32 0x00430dc0 */
void placeHotspotLevels(View *view)
{
    short images[33];
    short levels[34];
    ViewCel *cel;
    short i;
    short n;
    short level;
    short step;

    for (i = 17; i <= 32; i++)
        images[i] = i;
    if (!g_4b754a) {
        n = 0;
        for (i = 17; i <= 32; i++)
            switch (i) {
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
                levels[n] = openHotspots[i - 16];
                if (!levels[n++])
                    images[i] = 0;
                break;
            case 24:
                levels[n] = openHotspots[11];
                if (!levels[n++])
                    images[i] = 0;
                break;
            case 25:
            case 26:
            case 27:
            case 29:
            case 30:
            case 31:
            case 32:
                levels[n] = openHotspots[i - 17];
                if (!levels[n++])
                    images[i] = 0;
                break;
            case 28:
                levels[n] = openHotspots[16];
                if (!levels[n++])
                    images[i] = 0;
                break;
            }
    }
    n = 0;
    for (i = 17; i <= 32; i++, n++)
        if (images[i]) {
            if (g_4b754a)
                level = g_4b754a;
            else
                level = levels[n];
            if (level)
                level--;
            step = 0;
            while (level) {
                step += 16;
                level--;
            }
            images[i] += step;
        }
    cel = view->body.cels;
    while (cel->image)
        if (images[cel->image]) {
            cel->image = images[cel->image];
            cel++;
        } else {
            removeFirstCel(cel);
        }
}

/* Fills `open` (17 bytes, one per hotspot from 0) with what the roster says
   is open, level by level: each group of hotspots is at its level's count
   plus one if the roster has one (+0xc2-+0xc8), else at the flags in the
   roster's low or high nibbles. With g_4b754a, all are at that level. */
/* @zoombi32 0x004312e2 */
void findOpenHotspots(char *open)
{
    short i;
    short count;

    if (g_4b754a) {
        for (i = 0; i <= 16; i++)
            open[i] = (char)g_4b754a;
        return;
    }
    open[0] = 1;
    count = *(short *)(g_4a4ba0 + 0xc2);
    if (!count) {
        for (i = 1; i <= 3; i++)
            open[i] = g_4a4ba0[i + 0x55] & 0xf;
        open[4] = g_4a4ba0[0x50] & 0xf;
    } else {
        for (i = 1; i <= 4; i++)
            open[i] = count + 1;
    }
    count = *(short *)(g_4a4ba0 + 0xc4);
    if (!count) {
        for (i = 5; i <= 7; i++)
            open[i] = g_4a4ba0[i + 0x54] & 0xf;
        open[11] = g_4a4ba0[0x52] & 0xf;
    } else {
        for (i = 5; i <= 7; i++)
            open[i] = count + 1;
        open[11] = count + 1;
    }
    count = *(short *)(g_4a4ba0 + 0xc6);
    if (!count) {
        for (i = 8; i <= 10; i++)
            open[i] = g_4a4ba0[i + 0x54] & 0xf;
        open[16] = (short)(*(short *)(g_4a4ba0 + 0x52) & 0xf0) >> 4;
    } else {
        for (i = 8; i <= 10; i++)
            open[i] = count + 1;
        open[16] = count + 1;
    }
    count = *(short *)(g_4a4ba0 + 0xc8);
    if (!count) {
        for (i = 12; i <= 14; i++)
            open[i] = g_4a4ba0[i + 0x53] & 0xf;
        open[15] = g_4a4ba0[0x51] & 0xf;
    } else {
        for (i = 12; i <= 14; i++)
            open[i] = count + 1;
        open[15] = count + 1;
    }
}

/* Draws the levels' list in `rect`: a title ("terrain key", or with
   g_4b754a "choose a level") and the four levels, the current one
   (g_4b754a) outlined (levelTexts 0-5). */
/* @zoombi32 0x00430b31 */
void drawLevelList(ShortRect *rect)
{
    Color saved;
    ShortRect line;
    ShortRect title;
    short outlines[4] = {236, 234, 232, 238};
    short i;

    saved = setForeColor(Color(45));
    title.left = rect->left;
    title.right = rect->right;
    title.top = rect->top + 3;
    title.bottom = title.top + 18;
    line.top = rect->top + 22;
    line.bottom = line.top + 14;
    line.left = rect->left + 36;
    line.right = rect->right;
    i = 0;
    if (g_4b754a)
        i = 1;
    drawText(title, 0x22, levelTexts[i], 0xffff);
    for (i = 2; i <= 5; i++) {
        if (i - 1 == g_4b754a) {
            drawOutlinedText(outlines[i - 2], 45, line, 1, levelTexts[i]);
        } else {
            setForeColor(Color(45));
            drawText(line, 1, levelTexts[i], 0xffff);
        }
        line.top += 14;
        line.bottom += 14;
    }
    setForeColor(saved);
}

/* Scene 20's buttons: 1 leaves (g_4b0d50 = 1). */
/* @zoombi32 0x004328e2 */
void targetsClicked(short which)
{
    switch (which) {
    case 1:
        g_4b0d50 = 1;
        setCurrentMap(0);
        closeScene20();
        break;
    }
}

/* A view's update: while running, runs its script when it's to be redrawn;
   stopped, a second after its last update (unless g_4b9684), redraws it and
   picks no hotspot (pickHotspot). */
/* @zoombi32 0x0043108f */
void updateTextView(View *view, volatile short region)
{
    if (view->body.running) {
        if (view->reset) {
            runViewScript(view, region);
            unionRgnRect(region, &view->body.bounds);
            view->body.running = 1;
            view->reset = 1;
        }
    } else if (!g_4b9684 && !view->reset && clockTime() > view->nextUpdate + 60) {
        unionRgnRect(region, &view->body.bounds);
        view->reset = 1;
        pickHotspot(-1);
    }
}

/* Draws the levels' list view once (while its kind is positive, which it
   then negates): its cels and the list (drawLevelList), straight to the
   screen. */
/* @zoombi32 0x0043160a */
void drawLevelListView(View *view)
{
    if (view->kind > 0) {
        view->body.running = 1;
        drawCels(view);
        drawLevelList(&view->body.bounds);
        copyPortBits(viewPort, workPort, view->body.bounds, view->body.bounds, 0);
        view->kind = view->kind * -1;
        view->body.running = 0;
    }
}

/* Draws the names of the terrains with a hotspot open (all with
   g_4b754a) outlined, straight to the screen. */
/* @zoombi32 0x00431111 */
void drawTerrainNames()
{
    Color saved;
    short shown[4];
    short i;

    saved = setForeColor(Color(45));
    for (i = 0; i < 4; i++)
        shown[i] = g_4b754a != 0;
    for (i = 0; !g_4b754a && i <= 15; i++)
        if (openHotspots[i]) {
            if (i >= 1 && i <= 3)
                shown[0] = 1;
            if (i >= 5 && i <= 7)
                shown[1] = 1;
            if (i >= 8 && i <= 10)
                shown[2] = 1;
            if (i >= 12 && i <= 14)
                shown[3] = 1;
        }
    for (i = 0; i < 4; i++)
        if (shown[i])
            drawOutlinedText(45, 10, g_4a1f54[i], 0x22, levelTexts[6 + i]);
    setForeColor(saved);
    for (i = 0; i < 4; i++) {
        unionRgnRect(removedRgn, &g_4a1f54[i]);
        if (shown[i]) {
            g_4a1f54[i].left--;
            g_4a1f54[i].top--;
            g_4a1f54[i].right++;
            g_4a1f54[i].bottom++;
            copyPortBits(viewPort, workPort, g_4a1f54[i], g_4a1f54[i], 0);
            g_4a1f54[i].left++;
            g_4a1f54[i].top++;
            g_4a1f54[i].right--;
            g_4a1f54[i].bottom--;
        }
    }
}

/* A shot's placed callback (fireShot): moves it like driftView for 12
   steps, then shows its burst (images 25-27) and stops it, keeping the
   first stopped in firstShotStopped. While flying, the first of the six targets
   (targetBounds) it touches is hit (targetHit): it scores by the target's kind
   (targetScore; every 100 raises shipsLeft, up to 9) and bursts at once. */
/* @zoombi32 0x00432eff */
void placeShot(View *view)
{
    ShortRect target;
    short hit;
    DriftingBody *body;
    View *other;
    short i;

    body = (DriftingBody *)&view->body;
    body->unknown28++;
    if (body->unknown28 > 15) {
        body->cels[0].image = 0;
        if (!firstShotStopped) {
            view->body.running = 0;
            firstShotStopped = view->id;
        }
        return;
    }
    body->x += body->dx;
    body->y += body->dy;
    if (body->x > 650)
        body->x = -10;
    else if (body->x < -10)
        body->x = 650;
    if (body->y > 490)
        body->y = -10;
    else if (body->y < -10)
        body->y = 490;
    if (body->unknown28 >= 13) {
        body->cels[0].image = body->unknown28 + 12;
        body->unknown28++;
    }
    hit = !(unsigned short)(body->unknown28 < 13);
    for (i = 0; !hit && !targetHit && i < 6; i++) {
        if (!targetBounds[i])
            continue;
        target = *targetBounds[i];
        if (!sectRect(&target, &view->body.bounds))
            continue;
        body->unknown28 = 12;
        targetHit = i + 1;
        hit = 1;
        queueViewSound(3000, 0);
        other = findView(g_4afbac[i]);
        if (other) {
            if (other->kind >= 1021)
                targetScore += 15;
            else if (other->kind >= 1016)
                targetScore++;
            else if (other->kind >= 1005)
                targetScore += 10;
            else if (other->kind >= 1000)
                targetScore += 5;
        }
        if (targetScore >= g_4afb74) {
            g_4afb74 += 100;
            if (shipsLeft < 9)
                shipsLeft++;
        }
        startView(g_4afb78, 0, 0, 0);
    }
    body->cels[0].x = body->x;
    body->cels[0].y = body->y;
}

/* Sends a random Zoombini across the screen, along one of three paths
   below the cursor or above it (at a random height, either way round), at
   the speed pickerData.game.speed (now and then a little faster), with a
   random remark. Returns its view. */
/* @zoombi32 0x00431ea0 */
short sendRandomZoombini()
{
    short sound;
    Point where;
    Snoid snoid;
    View *view;
    short n;
    short from;
    short to;
    short swap;
    short speed;

    fillMemory(&snoid, 0, sizeof snoid);
    for (n = 0; n < 4; n++)
        snoid.features[n] = randomBetween(1, 5);
    getCursorPosition(&where);
    if (where.y > 240)
        n = randomBetween(4, 6);
    else
        n = randomBetween(1, 3);
    switch (n) {
    case 1:
        n = randomBetween(230, 270);
        from = -20;
        to = 660;
        break;
    case 2:
        n = randomBetween(380, 450);
        from = -20;
        to = 660;
        break;
    case 3:
        n = randomBetween(325, 335);
        from = 325;
        to = 660;
        break;
    case 4:
        n = randomBetween(165, 185);
        from = 328;
        to = 557;
        break;
    case 5:
        n = randomBetween(165, 185);
        from = -20;
        to = 557;
        break;
    case 6:
        n = randomBetween(165, 185);
        from = -20;
        to = 250;
        break;
    }
    if (randomBetween(1, 100) <= 50) {
        swap = from;
        from = to;
        to = swap;
    }
    n = placeSnoid(&snoid, 0, from, n, to, n);
    view = findView(n);
    if (view) {
        speed = pickerData.game.speed;
        view->interval = speed;
        if (speed > 1 && randomBetween(1, 100) <= 25)
            view->interval = speed - 1;
        switch (randomBetween(1, 9)) {
        case 1:
            sound = 6;
            break;
        case 2:
            sound = 12;
            break;
        case 3:
            sound = 10;
            break;
        case 4:
            sound = 9;
            break;
        case 5:
            sound = 0;
            break;
        case 6:
            sound = 1;
            break;
        case 7:
            sound = 11;
            break;
        case 8:
            sound = 2;
            break;
        case 9:
            sound = 8;
            break;
        }
        queueViewSound(snoidSound(viewSnoid(view), sound), 0);
    }
    return n;
}

/* Starts a target in a free slot of the six (targetBounds, g_4afbac): of kind
   1-4 (script 1000, 1016, 1021 or 1026 on, and 1005 otherwise, plus 0-4;
   3 and 4 fly fast one way), at a random speed and direction from a random
   edge, or where g_4afb6c-g_4afb70 say when `preset`. Returns its view (0
   if there's no free slot). */
/* @zoombi32 0x004330f3 */
short startTarget(short kind, short preset)
{
    short dx;
    short dy;
    short speed;
    short x;
    short y;
    View *view;
    DriftingBody *body;
    short slot;
    short i;
    short direction;
    short script;

    slot = 0;
    for (i = 0; !slot && i < 6; i++)
        if (!targetBounds[i])
            slot = i + 1;
    if (!slot)
        return 0;
    slot--;
    g_4afbac[slot] = 0;
    targetBounds[slot] = 0;
    switch (randomBetween(1, 10)) {
    case 1:
    case 2:
        speed = 8;
        break;
    case 9:
    case 10:
        speed = 16;
        break;
    default:
        speed = 12;
        break;
    }
    direction = randomBetween(0, 7);
    script = randomBetween(0, 4);
    switch (kind) {
    case 4:
        script += 1026;
        direction = 2;
        speed = 20;
        break;
    case 3:
        script += 1021;
        direction = 6;
        speed = 20;
        break;
    case 2:
        script += 1016;
        break;
    case 1:
        script += 1000;
        break;
    default:
        script += 1005;
        break;
    }
    if (preset) {
        x = g_4afb6c;
        y = g_4afb6e;
        direction = g_4afb70;
    } else if (direction == 0 || direction == 4) {
        x = randomBetween(20, 620);
        y = -10;
    } else {
        x = -10;
        y = randomBetween(20, 460);
    }
    dy = 0;
    dx = 0;
    switch (direction) {
    case 0:
        dy = -speed;
        break;
    case 1:
        dy = -speed;
        dx = speed;
        break;
    case 2:
        dx = speed;
        break;
    case 3:
        dy = speed;
        dx = speed;
        break;
    case 4:
        dy = speed;
        break;
    case 5:
        dy = speed;
        dx = -speed;
        break;
    case 6:
        dx = -speed;
        break;
    case 7:
        dy = -speed;
        dx = -speed;
        break;
    }
    g_4afbac[slot] = addView(0, drawCels, runViewCels, script, 5, 0, 0, 0);
    view = findView(g_4afbac[slot]);
    if (view) {
        body = (DriftingBody *)&view->body;
        body->unknown28 = 0;
        body->x = x;
        body->y = y;
        body->dx = dx;
        body->dy = dy;
        body->unknown32 = direction;
        view->placed = driftView;
        targetBounds[slot] = &view->body.bounds;
        g_4afbba++;
    }
    return g_4afbac[slot];
}

/* Closes scene 1 (the map). Leaving it for a level (g_4b754a), the first
   time switches the user file to ZBtemp (keeping the player's in
   savedUserFile) and saves; then fills the roster's party with 16 (or
   g_4afb5e) Zoombinis at random (with the 0x800 modifier, all alike by
   fives). */
/* @zoombi32 0x0042fca8 */
void closeScene1()
{
    short saved;
    short i;
    short j;
    short alike;

    if (g_4afb14) {
        g_4afb14 = 0;
        saved = setFreeAtOnce(1);
        if (!g_4b754a && !viewsLocked) {
            viewsLocked = 1;
            *(short *)(g_4a4ba0 + 0xa92e) = 0;
            *(short *)(g_4a4ba0 + 0xa930) = 1;
            *(short *)(g_4a4ba0 + 0xa932) = 1;
        }
        clearViews();
        if (g_4b754a) {
            if (!g_4afb30) {
                strcpy(savedUserFile, userFile);
                g_4a48e8 = 1;
                strcpy(userFile, "ZBtemp");
                viewsLocked = 0;
                g_4afb32 = 1;
                saveRoster();
                g_4afb30 = 1;
            }
            *(short *)(g_4a4ba0 + 0xa92e) = 16;
            if (g_4afb5e)
                *(short *)(g_4a4ba0 + 0xa92e) = g_4afb5e;
            alike = addModifierKeys(0) == 0x800;
            for (i = 0; i < *(short *)(g_4a4ba0 + 0xa92e); i++) {
                for (j = 0; j < 4; j++)
                    if (alike)
                        (g_4a4ba0 + i * 19)[j + 0xa934] = i % 5 + 1;
                    else
                        (g_4a4ba0 + i * 19)[j + 0xa934] = randomBetween(1, 5);
                g_4a4ba0[i * 19 + 0xa93d] = 0;
                g_4a4ba0[i * 19 + 0xa93c] = 1;
                *(short *)(g_4a4ba0 + 0xa930) = 0;
            }
        }
        for (i = 0; i < 6; i++)
            freeSave(&g_4afb18[i]);
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&g_4afb10);
        fadeOutViews();
        showBusyCursor();
    }
}

/* The ship's placed callback (the view shipView): moves it by g_4afb86,
   g_4afb88 wrapping round the screen and shows it facing shipDirection (images
   17-24). Touching a target, it bursts (g_4afb80 counts images 11-14,
   then it's gone) and loses one of shipsLeft; the last starts the view
   g_4afb7a (script 1013). */
/* @zoombi32 0x00432cec */
void placeShip(View *view)
{
    ShortRect target;
    ViewBody *body;
    short i;

    body = &view->body;
    if (g_4afb80) {
        if (g_4afb80 > 4) {
            body->cels[0].image = 0;
            return;
        }
        g_4afb82 += g_4afb86;
        g_4afb84 += g_4afb88;
        if (g_4afb82 > 650)
            g_4afb82 = -10;
        else if (g_4afb82 < -10)
            g_4afb82 = 650;
        if (g_4afb84 > 490)
            g_4afb84 = -10;
        else if (g_4afb84 < -10)
            g_4afb84 = 490;
        body->cels[0].image = g_4afb80 + 10;
        g_4afb80++;
        body->cels[0].x = g_4afb82;
        body->cels[0].y = g_4afb84;
        return;
    }
    for (i = 0; !g_4afb80 && i < 6; i++) {
        if (!targetBounds[i])
            continue;
        target = *targetBounds[i];
        if (!sectRect(&target, &view->body.bounds))
            continue;
        g_4afb80 = 1;
        queueViewSound(3001, 0);
        shipsLeft--;
        if (!shipsLeft)
            g_4afb7a = addView(0, drawCels, runViewScript, 1013, 6, 0, 0, 0);
        startView(g_4afb78, 0, 0, 0);
    }
    g_4afb82 += g_4afb86;
    g_4afb84 += g_4afb88;
    if (g_4afb82 > 650)
        g_4afb82 = -10;
    else if (g_4afb82 < -10)
        g_4afb82 = 650;
    if (g_4afb84 > 490)
        g_4afb84 = -10;
    else if (g_4afb84 < -10)
        g_4afb84 = 490;
    body->cels[0].image = shipDirection + 17;
    body->cels[0].x = g_4afb82;
    body->cels[0].y = g_4afb84;
}

/* A view's update like runViewCels, drawing its script's frame at the
   cursor (relative to the view's own place with flag 0x800000) whenever it
   runs. */
/* Not exact: register allocation, as in runViewCels (the original keeps the
   frame's word in eax, the position in the script in edx and the count in
   ecx). */
/* @zoombi32 0x004321ac */
void updateCursorView(View *view, short region)
{
    ShortRect rect;
    short x;
    short y;
    short *cels;
    ImageBank *bank;
    Point where;
    short *cel;

    if (view->body.running) {
        getCursorPosition(&where);
        view->changed = 1;
        if (view->reset) {
            setViewScript(view, view->kind, 1);
            view->unknown2e = 0;
        } else {
            unionRgnRect(region, &view->body.bounds);
        }
        {
            short *at;
            short word;
            short left;

            at = scripts[view->body.script] + view->body.frameOffset;
            bank = groupBanks[view->body.scriptGroup];
            x = where.x;
            y = where.y;
            if (view->flags & 0x800000) {
                x -= view->body.x;
                y -= view->body.y;
            }
            cels = cel = (short *)view->body.cels;
            left = 24;
            do {
                left--;
                word = *at++;
                if (!word) {
                    at += 2;
                    *cel++ = 0;
                    *cel++ = 0;
                    *cel++ = 0;
                } else if (word > 0) {
                    *cel++ = word;
                    *cel++ = x;
                    *cel++ = y;
                    at++;
                    at++;
                } else {
                    if (word < -0x100)
                        at++;
                    if (left)
                        *cel = left = 0;
                }
            } while (left);
        }
        cel = cels;
        if (*cel) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

            cel++;
            view->body.bounds.left = *cel++;
            view->body.bounds.right = swapShort(image[0]) + view->body.bounds.left;
            view->body.bounds.top = *cel++;
            view->body.bounds.bottom = swapShort(image[1]) + view->body.bounds.top;
        }
        while (*cel) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

            cel++;
            rect.left = *cel++;
            rect.right = swapShort(image[0]) + rect.left;
            rect.top = *cel++;
            rect.bottom = swapShort(image[1]) + rect.top;
            unionRect(&view->body.bounds, &rect);
        }
    }
}

/* Scene 20's keys (the practice game): space starts again (once the game
   is over, g_4afb7a) or brings back a burst ship, 5 fires (up to three
   shots), 4 and 6 turn the ship, 8 pushes it on (up to 12 each way). */
/* @zoombi32 0x00432a79 */
short scene20Key(unsigned short key)
{
    short id;
    short i;

    switch (key) {
    case ' ':
        if (g_4afb7a) {
            if (g_4afbbe) {
                deleteView(g_4afbbe);
                g_4afbba = 0;
                g_4afbbe = 0;
                for (i = 0; i < 6; i++)
                    targetBounds[i] = 0;
            }
            id = g_4afb7a;
            g_4afb7a = 0;
            deleteView(id);
            targetScore = 0;
            shipsLeft = 3;
            g_4afb74 = 100;
            g_4afbbc = 0;
            startView(g_4afb78, 0, 0, 0);
            resetShip();
        } else if (g_4afb80) {
            resetShip();
        }
        break;
    case '5':
        if (shotsStarted < 3 && !g_4afb80) {
            fireShot();
            queueViewSound(3002, 0);
        }
        break;
    case '4':
        if (!g_4afb80)
            shipDirection = (shipDirection - 1) & 7;
        break;
    case '6':
        if (!g_4afb80)
            shipDirection = (shipDirection + 1) & 7;
        break;
    case '8':
        if (!g_4afb80) {
            switch (shipDirection) {
            case 0:
                g_4afb88 += -4;
                break;
            case 1:
                g_4afb88 += -4;
                g_4afb86 += 4;
                break;
            case 2:
                g_4afb86 += 4;
                break;
            case 3:
                g_4afb88 += 4;
                g_4afb86 += 4;
                break;
            case 4:
                g_4afb88 += 4;
                break;
            case 5:
                g_4afb88 += 4;
                g_4afb86 += -4;
                break;
            case 6:
                g_4afb86 += -4;
                break;
            case 7:
                g_4afb88 += -4;
                g_4afb86 += -4;
                break;
            }
            if (g_4afb88 < -12)
                g_4afb88 = -12;
            if (g_4afb88 > 12)
                g_4afb88 = 12;
            if (g_4afb86 < -12)
                g_4afb86 = -12;
            if (g_4afb86 > 12)
                g_4afb86 = 12;
        }
        break;
    }
    return 0;
}

/* Draws the map's box in `rect`: the game's name and how many Zoombinis
   are still to come (625 less those at the camps) and at each camp; in
   practice (g_4b754a), "practice mode" and how to get back to the game
   from the furthest level reached. */
/* @zoombi32 0x00430878 */
void drawMapBox(ShortRect *rect)
{
    Color saved;
    char number[8];
    ShortRect line;
    ShortRect count;
    ShortRect title;
    char name[64];
    short i;
    short n;

    saved = setForeColor(Color(45));
    title.left = rect->left;
    title.right = rect->right;
    title.top = rect->top + 3;
    title.bottom = title.top + 18;
    line.top = count.top = rect->top + 26;
    line.bottom = count.bottom = line.top + 18;
    line.left = rect->left + 7;
    line.right = line.left + 115;
    count.left = line.right;
    count.right = rect->right + -7;
    if (!g_4b754a)
        strcpy(name, gameName);
    else
        strcpy(name, mapTexts[4]);
    drawText(title, 0x22, name, 0xffff);
    if (!g_4b754a) {
        for (i = 0; i < 4; i++) {
            switch (i) {
            case 0:
                n = 625 - (*(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c)
                           + *(short *)(g_4a4ba0 + 0x4e));
                break;
            case 1:
                n = *(short *)(g_4a4ba0 + 0x4a);
                break;
            case 2:
                n = *(short *)(g_4a4ba0 + 0x4c);
                break;
            case 3:
                n = *(short *)(g_4a4ba0 + 0x4e);
                break;
            }
            drawText(line, 1, mapTexts[i], 0xffff);
            intToDecimal(n, number);
            drawText(count, 4, number, 0xffff);
            count.top += 18;
            count.bottom += 18;
            line.top += 18;
            line.bottom += 18;
        }
    } else {
        if (g_4a4ba0[0x51] & 0xf)
            n = 17;
        else if (g_4a4ba0[0x52] & 0xff)
            n = 13;
        else if (g_4a4ba0[0x50] & 0xf)
            n = 9;
        else
            n = 5;
        line.left = rect->left + 5;
        line.right = rect->right;
        line.top = rect->top + 26;
        line.bottom = line.top + 18;
        for (i = 0; i < 4; i++) {
            drawText(line, 1, mapTexts[n + i], 0xffff);
            line.top += 18;
            line.bottom += 18;
        }
    }
    setForeColor(saved);
}

/* Draws the map's box view once (while its kind is set, which it then
   clears): its cels and the box (drawMapBox), straight to the screen. */
/* @zoombi32 0x0043157f */
void drawMapBoxView(View *view)
{
    if (view->kind) {
        view->body.running = 1;
        drawCels(view);
        drawMapBox(&view->body.bounds);
        copyPortBits(viewPort, workPort, view->body.bounds, view->body.bounds, 0);
        view->kind = 0;
        view->body.running = 0;
    }
}

/* Makes the map's views again: the open hotspots (findOpenHotspots) and the
   terrains' names, the levels' list (levelListView, script 1003, animated in
   practice) and the box (mapBoxView, script 1002); with `update`, shows
   them. */
/* @zoombi32 0x0043145f */
void makeMapViews(short update)
{
    View *view;
    long interval;

    deleteView(levelListView);
    deleteView(mapBoxView);
    levelListView = mapBoxView = 0;
    findOpenHotspots(openHotspots);
    drawTerrainNames();
    interval = 0;
    if (g_4b754a)
        interval = 6;
    levelListView = addView(0x100000, drawLevelListView, runViewScript, 1003, interval, 0, 0, 0);
    view = findView(levelListView);
    if (view)
        view->placed = placeLevelMarker;
    mapBoxView = addView(0x100000, drawMapBoxView, runViewScript, 1002, 0, 0, 0, 0);
    if (update)
        updateViews();
}

/* Scene 19's frame: deletes the Zoombinis that have finished crossing
   (g_4afb60) and, while pickerData.game.throws, now and then (every
   20-120 ticks) sends new ones, more of them together as
   pickerData.game.count grows. */
/* @zoombi32 0x0043195a */
void scene19Frame()
{
    short id;
    short i;

    if (!g_4a2066 && g_4afb14) {
        g_4a2066 = 1;
        updateViews();
        for (i = 0; i < 3; i++)
            if (idleSnoidView(g_4afb60[i])) {
                id = g_4afb60[i];
                g_4afb60[i] = 0;
                deleteView(id);
            }
        if (clockTime() > g_4afb68 && pickerData.game.throws) {
            for (i = 0; viewsSorted && i < 3; i++) {
                if (!g_4afb60[i]) {
                    g_4afb60[i] = sendRandomZoombini();
                    g_4afb68 = randomBetween(20, 120) + clockTime();
                }
                if (pickerData.game.count < 10) {
                    i = 3;
                } else if (pickerData.game.count < 15) {
                    if (i == 1)
                        i = 3;
                } else if (pickerData.game.count < 20) {
                    if (i == 2)
                        i = 3;
                } else if (pickerData.game.count < 40) {
                    if (i == 1)
                        i = 3;
                } else if (pickerData.game.count < 60) {
                    i = 3;
                } else if (pickerData.game.count < 80) {
                    if (i == 1)
                        i = 3;
                } else if (pickerData.game.count > 80) {
                    i = 3;
                }
            }
        }
        g_4a2066 = 0;
    }
}

/* Scene 1's frame (the map): shows the name of the open hotspot under the
   cursor in the view g_4afb3e and picks it (pickHotspot), or hides the name;
   leaves when a choice was made (g_4b0d52). */
/* @zoombi32 0x0042feaf */
void scene1Frame()
{
    Point where;
    View *view;
    short i;
    short open;

    if (!g_4a2008 && g_4afb14) {
        g_4a2008 = 1;
        view = 0;
        if (!g_4b9684)
            view = findView(g_4afb3e);
        if (view && !g_4b9684) {
            getCursorPosition(&where);
            for (i = 0; i < 16; i++) {
                if (i == 11)
                    open = openHotspots[11] || openHotspots[16];
                else
                    open = openHotspots[i];
                if (open && ptInRect(&pickerData.hotspots[i].rect, where)) {
                    view->nextUpdate = clockTime();
                    if (i != g_4afb36) {
                        view->body.running = 0;
                        view->reset = 1;
                        pickHotspot(-1);
                    }
                    if (!view->body.running && view->reset) {
                        char *to;
                        char *from;

                        g_4afb36 = i;
                        to = (char *)&view->body.cels[10];
                        from = placeNames[i];
                        while (*from)
                            *to++ = *from++;
                        *to = 0;
                        view->body.running = 1;
                        pickHotspot(++i);
                    }
                    i = 17;
                }
            }
            if (i < 17)
                view->body.running = 0;
        }
        updateViews();
        if (g_4b0d52) {
            g_4b0d50 = g_4b0d52;
            g_4b0d52 = 0;
            setCurrentMap(0);
            closeScene1();
        }
        g_4a2008 = 0;
    }
}

/* Scene 20's frame (the practice game): starts targets when there are none
   (one or two of kind 2), clears the stopped shot, and bursts the target
   hit (targetHit): a large one splits into two or three of the next size,
   flying off on either side; when the last goes, every other time a big
   one (kind 3 or 4) crosses. */
/* @zoombi32 0x0043261d */
void scene20Frame()
{
    DriftingBody *body;
    short split;
    View *view;
    short n;
    short size;
    short turn;

    if (!g_4a20b0 && g_4afb14) {
        g_4a20b0 = 1;
        updateViews();
        if (!g_4afbba) {
            startTarget(2, 0);
            if (randomBetween(1, 10) <= 4)
                startTarget(2, 0);
        }
        if (firstShotStopped) {
            n = firstShotStopped;
            firstShotStopped = 0;
            deleteView(n);
            shotsStarted--;
            if (shotsStarted < 0)
                shotsStarted = 0;
        }
        if (targetHit) {
            if (!targetBursting) {
                n = targetHit;
                n--;
                targetBursting = -g_4afbac[n];
                view = findView(-targetBursting);
                if (view) {
                    size = 0;
                    if (view->kind >= 1016 && view->kind < 1021)
                        size = 2;
                    if (view->kind < 1005)
                        size = 1;
                    if (size) {
                        body = (DriftingBody *)&view->body;
                        g_4afb6c = body->x;
                        g_4afb6e = body->y;
                        if (randomBetween(1, 100) <= 33) {
                            turn = 2;
                            split = 6;
                        } else {
                            turn = 1;
                            split = 7;
                        }
                        g_4afb70 = (turn + body->unknown32) & 7;
                        startTarget(size - 1, 1);
                        g_4afb70 = (body->unknown32 + split) & 7;
                        startTarget(size - 1, 1);
                        if (size == 1 && randomBetween(1, 100) <= 33) {
                            g_4afb70 = (body->unknown32 + 4) & 7;
                            startTarget(--size, 1);
                        }
                    }
                    view->flags = 0x100000;
                    view->update = runViewScript;
                    setViewScript(view, 1015, 1);
                    view->notify = burstNotify;
                    view->notifyEnd = 1;
                }
                g_4afbac[n] = 0;
                targetBounds[n] = 0;
                g_4afbba--;
                if (g_4afbba <= 0) {
                    g_4afbba = 0;
                    g_4afbbc = !g_4afbbc;
                    if (g_4afbbc) {
                        if (randomBetween(1, 10) <= 5)
                            n = 3;
                        else
                            n = 4;
                        g_4afbbe = startTarget(n, 0);
                    }
                }
            } else if (targetBursting > 0) {
                if (targetBursting == g_4afbbe) {
                    g_4afbbc = 0;
                    g_4afbbe = 0;
                }
                n = targetBursting;
                targetBursting = targetHit = 0;
                deleteView(n);
            }
        }
        g_4a20b0 = 0;
    }
}

/* Leaves practice for the game: puts back the map's saved areas, makes its
   views again (shown as in the game), and the first time, the player's
   user file and roster (readRoster). */
/* @zoombi32 0x00430724 */
void leavePractice()
{
    short level;
    View *view;
    short i;
    short id;

    level = g_4b754a;
    g_4b754a = 0;
    for (i = 0; i < 6; i++)
        copyPortBits(viewPort, g_4afb18[i]->port, g_4a1f54[i], g_4a1f54[i], 0);
    makeMapViews(0);
    view = findView(levelListView);
    if (view) {
        view->kind = abs(view->kind);
        view->nextUpdate = 0;
        setViewScript(view, 0, 1);
        view->reset = 1;
    }
    for (i = 0; i < 2; i++) {
        switch (i) {
        case 0:
            id = g_4afb3c;
            break;
        case 1:
            id = g_4afb3a;
            break;
        }
        startView(id, 0, 0, 0);
        view = findView(id);
        if (view) {
            view->reset = 1;
            view->nextUpdate = 0;
        }
    }
    updateViews();
    g_4b754a = level;
    if (g_4afb30) {
        g_4afb30 = 0;
        readRoster();
        viewsLocked = 1;
        g_4b0d52 = 0;
        strcpy(userFile, savedUserFile);
    }
}

/* Scene 1's keys (the map): 1-4 pick the practice level while practising,
   0x10 starts practice (level 1); with debugging on, + and - change how
   many Zoombinis to practise with (g_4afb5e, 1-16), T asks for a
   transition, and a-p then shows it (scene 7). Returns whether the level
   changed. */
/* @zoombi32 0x0043041f */
short scene1Key(unsigned short key)
{
    short used;
    short old;
    View *view;
    short i;
    short id;

    used = 0;
    if (g_4b8803 && g_4afb16) {
        unionRgnRect(removedRgn, &debugRect);
        g_4afb16 = 0;
        g_4a7e68 = 0;
        if (key >= 'a' && key <= 'p') {
            g_4a7e68 = key - 0x60;
            g_4b0d50 = 7;
            setCurrentMap(0);
            closeScene1();
            return 1;
        }
    }
    switch (key) {
    case '+':
        if (!g_4b8803)
            return 0;
        g_4afb5e += 2;
    case '-':
        if (!g_4b8803)
            return 0;
        g_4afb5e--;
        if (g_4afb5e < 1)
            g_4afb5e = 1;
        if (g_4afb5e > 16)
            g_4afb5e = 16;
        debugMessage(g_4afb5e, "Snoids to practice with = ", 0, 0, 0);
        break;
    case '1':
    case '2':
    case '3':
    case '4':
        if (!g_4b754a)
            break;
    case 0x10:
        old = g_4b754a;
        if (key == 0x10) {
            if (g_4b754a)
                break;
            g_4b754a = 1;
        } else {
            g_4b754a = key - '0';
        }
        if (old == g_4b754a)
            break;
        if ((!old && g_4b754a) || (old && !g_4b754a)) {
            for (i = 0; i < 6; i++)
                copyPortBits(viewPort, g_4afb18[i]->port, g_4a1f54[i], g_4a1f54[i], 0);
            makeMapViews(0);
        }
        if (g_4b754a) {
            copyPortBits(viewPort, *g_4afb2c, g_4a1f7c, g_4a1f7c, 0);
            findOpenHotspots(openHotspots);
            view = findView(levelListView);
            if (view) {
                view->kind = abs(view->kind);
                view->nextUpdate = 0;
                setViewScript(view, 0, 1);
                view->reset = 1;
            }
        }
        for (i = 0; i < 2; i++) {
            switch (i) {
            case 0:
                id = g_4afb3c;
                break;
            case 1:
                id = g_4afb3a;
                break;
            }
            startView(id, 0, 0, 0);
            view = findView(id);
            if (view) {
                view->reset = 1;
                view->nextUpdate = 0;
            }
        }
        used = 1;
        break;
    case 'T':
        if (g_4b8803 && g_4b754a) {
            debugMessage(-1, "Which Transition (a-p):", 0, 0, 0);
            g_4afb16 = 1;
        }
        break;
    }
    return used;
}

/* Scene 19's clicks (catching Zoombinis): the leave area goes back to the
   map; otherwise, with throws left, throws at the cursor (the view g_4afb3e,
   or g_4afb3c on a catch): a Zoombini within 12 pixels of its middle (and
   not behind the areas g_4a2068) is caught, scoring and, every other catch
   in a row, bonus throws; the Zoombinis speed up as more are caught. Out of
   throws, "again" (pickerData.game.again) starts over. */
/* @zoombi32 0x00431ab5 */
void scene19Clicked(short)
{
    Point where;
    short thrown;
    View *view;
    View *shown;
    short x;
    short y;
    short bonus;

    getCursorPosition(&where);
    if (ptInRect(&pickerData.game.leave, where)) {
        g_4b0d50 = 1;
        setCurrentMap(0);
        closeScene19();
        return;
    }
    view = viewAt(where, 1, 1);
    if (view) {
        x = (view->body.bounds.left + view->body.bounds.right) / 2;
        y = (view->body.bounds.top + view->body.bounds.bottom) / 2;
        x = abs(x - where.x);
        y = abs(y - where.y);
        if (x > 12 || y > 12)
            view = 0;
        for (y = 0; view && y < 3; y++)
            if (ptInRect(&g_4a2068[y], where))
                view = 0;
    }
    if (!g_4afb3a && pickerData.game.throws) {
        pickerData.game.throws--;
        g_4afb3a = 1;
        if (view) {
            pickerData.game.streak++;
            switch (pickerData.game.streak) {
            case 0:
            case 1:
                bonus = 0;
                break;
            case 2:
            case 3:
                bonus = 5;
                break;
            case 4:
            case 5:
                bonus = 10;
                break;
            case 6:
            case 7:
                bonus = 15;
                break;
            default:
                bonus = 20;
                break;
            }
            if (bonus && pickerData.game.remaining) {
                if (bonus <= pickerData.game.remaining)
                    pickerData.game.throws += bonus;
                else
                    pickerData.game.throws += pickerData.game.remaining;
            }
            thrown = g_4afb3c;
            view->nextUpdate = clockTime() + 240;
            pickerData.game.caught = view->id;
            for (y = 0; y < 3; y++)
                if (g_4afb60[y] == pickerData.game.caught)
                    g_4afb60[y] = 0;
            pickerData.game.count++;
            if (pickerData.game.count == 5)
                pickerData.game.speed = 7;
            if (pickerData.game.count == 10)
                pickerData.game.speed = 6;
            if (pickerData.game.count == 20)
                pickerData.game.speed = 5;
            if (pickerData.game.count == 40)
                pickerData.game.speed = 4;
            if (pickerData.game.count == 60)
                pickerData.game.speed = 3;
            if (pickerData.game.count == 80)
                pickerData.game.speed = 2;
            if (pickerData.game.count == 90)
                pickerData.game.speed = 1;
            setViewsLocked(0);
            g_4afb3a = 2;
        } else {
            pickerData.game.streak = 0;
            thrown = g_4afb3e;
        }
        shown = findView(thrown);
        if (shown)
            *(Point *)&shown->body.x = where;
        startView(thrown, 0, caughtNotify, 1);
        if (g_4afb3a == 2)
            moveView(thrown, 1, pickerData.game.caught);
        pickerData.game.remaining = 99 - pickerData.game.count - pickerData.game.throws;
        startView(levelListView, 0, 0, 0);
        if (!pickerData.game.throws && !pickerData.game.overView) {
            pickerData.game.overView = addView(0, drawCels, runViewScript, 1204, 1, 0, 0, 0);
            shown = findView(pickerData.game.againView);
            if (shown) {
                pickerData.game.again = shown->body.bounds;
                shown->body.running = 0;
                showCursor();
            }
        }
    } else if (!pickerData.game.throws && ptInRect(&pickerData.game.again, where)
               && pickerData.game.overView) {
        pickerData.game.count = 0;
        pickerData.game.throws = 9;
        pickerData.game.remaining = 99 - pickerData.game.count - pickerData.game.throws;
        pickerData.game.speed = 8;
        pickerData.game.streak = 0;
        deleteView(pickerData.game.overView);
        pickerData.game.overView = 0;
        startView(levelListView, 0, 0, 0);
        shown = findView(pickerData.game.againView);
        if (shown) {
            shown->body.running = 1;
            hideCursor();
        }
    }
}

/* Scene 1's clicks, by hotspot (`which`): each goes to its scene (in
   practice; the camps also in the game once the roster has reached them,
   leaving practice), two with cheats to other scenes; 17 (the rest of the
   screen) presses the help button (g_4afb42) or picks a level from the
   list. */
/* @zoombi32 0x0043010b */
void scene1Clicked(short which)
{
    short clicked;
    Point where;
    View *view;
    short scene;
    short go;
    short i;

    scene = 0;
    go = 0;
    getCursorPosition(&where);
    if (g_4b754a)
        go = 1;
    switch (which) {
    case 1:
        scene = 3;
        leavePractice();
        g_4b754a = 0;
        go = 1;
        break;
    case 2:
        scene = 7;
        break;
    case 3:
        scene = 8;
        break;
    case 4:
        scene = 9;
        break;
    case 5:
        scene = 4;
        go = g_4a4ba0[0x50] & 0xf;
        if (g_4b754a && go) {
            leavePractice();
            g_4b754a = 0;
        }
        break;
    case 6:
        scene = 10;
        break;
    case 7:
        scene = 11;
        break;
    case 8:
        if ((short)isCheat((long)0xc07a877d, (long)0xedfa7273))
            scene = 20;
        else
            scene = 12;
        break;
    case 9:
        if ((short)isCheat(0x469110d3, 0x1e1c32f2))
            scene = 19;
        else
            scene = 13;
        break;
    case 10:
        scene = 14;
        break;
    case 11:
        scene = 15;
        break;
    case 12:
        scene = 5;
        go = *(short *)(g_4a4ba0 + 0x52) & 0xff;
        if (g_4b754a && go) {
            leavePractice();
            g_4b754a = 0;
        }
        break;
    case 13:
        scene = 16;
        break;
    case 14:
        scene = 17;
        break;
    case 15:
        scene = 18;
        break;
    case 16:
        scene = 6;
        go = g_4a4ba0[0x51] & 0xf;
        if (g_4b754a && go) {
            leavePractice();
            g_4b754a = 0;
        }
        break;
    case 17:
        clicked = 0;
        if (ptInRect(&g_4afb42, where)) {
            view = findView(g_4afb40);
            if (view) {
                queueViewSound(999, 0);
                setViewScript(view, 0, 1);
                view->interval = 2;
                updateViews();
                waitForEventFor(0, 2, 0, 1);
                setViewScript(view, 0, 1);
                view->interval = 3;
                updateViews();
                view->body.running = 0;
                showDialog(1, 0, 0, 0);
            }
        } else if (g_4b754a) {
            for (i = 0; !clicked && i < 4; i++)
                if (ptInRect(&g_4a1fa8[i], where)) {
                    scene1Key(i + '1');
                    clicked = 1;
                }
        }
        go = 0;
        break;
    }
    if (go) {
        if (!g_4b9684)
            pickHotspot(which);
        queueViewSound(998, 0);
        waitForEventFor(0, 2, 0, 1);
        g_4b0d50 = scene;
        setCurrentMap(0);
        closeScene1();
    }
}
