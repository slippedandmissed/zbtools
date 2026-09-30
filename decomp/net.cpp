/*
 * net (0x439560-0x43e620): 'Net.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "maze.h"
#include "module_4623b8.h"
#include "net.h"
#include "random.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/*
 * Lays out a maze Zoombini's cels (unless it's in state 1; 3 and others
 * become 1), by its move (word 30), placed by the hot spots in
 * g_4afbd0/g_4afbd4: its body (from word 40), with the ways open (words
 * 34-37) and its direction (38), and a helper part (words 41, 42); or a
 * turn (1: word 38 cycling 0-3), a spin (5: word 38 cycling 0-5), or
 * falling (6, 7: then, in state 2, one of four more frames by word 46,
 * back to state 1 after). Then its bounds from the images in mazeImages.
 */
/* @zoombi32 0x0043a7a6 */
void layOutMazeCels(Snoid *snoid)
{
    short k;
    ShortRect rect;
    Point where;
    short *cel;
    short *parts;
    short part;
    ImageBank *bank;

    snoid->body.bounds.left = 0;
    snoid->body.bounds.top = 0;
    snoid->body.bounds.right = 0;
    snoid->body.bounds.bottom = 0;
    parts = cel = (short *)snoid->body.cels;
    switch (snoid->unknownF4) {
    case 1:
        return;
    case 2:
        break;
    case 3:
        snoid->unknownF4 = 1;
        break;
    default:
        snoid->unknownF4 = 1;
        break;
    }
    where = *(Point *)&snoid->body.x;
    switch (cel[30]) {
    case 2:
    case 3:
    case 4:
        part = parts[40];
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        for (k = 1; k < 5; k++)
            if (parts[33 + k]) {
                part = parts[40] + k;
                *cel++ = part;
                *cel++ = where.x - g_4afbd0[part];
                *cel++ = where.y - g_4afbd4[part];
            }
        part = parts[40] + parts[38] + 5;
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        if (parts[41]) {
            part = parts[41] * 5 + parts[42] + 5;
            *cel++ = part;
            *cel++ = where.x - g_4afbd0[part];
            *cel++ = where.y - g_4afbd4[part];
        }
        break;
    case 1:
        parts[38]++;
        if (parts[38] > 3)
            parts[38] = 0;
        part = parts[38] + 2;
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        break;
    case 5:
        parts[38]++;
        if (parts[38] > 5)
            parts[38] = 0;
        part = parts[40] + parts[38] + 9;
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        break;
    case 6:
        part = parts[40] + 19;
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        if (snoid->unknownF4 == 2) {
            if (parts[46] >= 0 && parts[46] < 4) {
                part = parts[40] + parts[46] + 15;
                *cel++ = part;
                *cel++ = where.x - g_4afbd0[part];
                *cel++ = where.y - g_4afbd4[part];
                parts[46]++;
            } else {
                snoid->unknownF4 = 1;
            }
        }
        break;
    case 7:
        part = parts[40] + 20;
        *cel++ = part;
        *cel++ = where.x - g_4afbd0[part];
        *cel++ = where.y - g_4afbd4[part];
        if (snoid->unknownF4 == 2) {
            if (parts[46] >= 0 && parts[46] < 4) {
                part = parts[40] + parts[46] + 15;
                *cel++ = part;
                *cel++ = where.x - g_4afbd0[part];
                *cel++ = where.y - g_4afbd4[part];
                parts[46]++;
            } else {
                snoid->unknownF4 = 1;
            }
        }
        break;
    }
    *cel = 0;
    cel = (short *)snoid->body.cels;
    bank = mazeImages;
    while (*cel && *cel <= bank->count) {
        unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

        cel++;
        rect.left = *cel++;
        rect.top = *cel++;
        rect.right = swapShort(image[0]) + rect.left;
        rect.bottom = swapShort(image[1]) + rect.top;
        unionRect(&snoid->body.bounds, &rect);
    }
}

/*
 * Two maze Zoombinis meeting (`a` and `b`): by the directions they face
 * (word 20) picks each one's script (15035 on, from word 45) and its
 * helper view's (10004 on), which also gets an extra view (by the square's
 * g_4afc24) until its script ends (startPairNotify); clears a's square in
 * squareOccupants and regroups each with its helper.
 */
/* @zoombi32 0x00439666 */
void mazeZoombinisMeet(View *a, View *b)
{
    View *volatile helperB;
    Point where;
    volatile short aExtraScript;
    volatile short bExtraScript;
    volatile short offset;
    volatile short aHelperScript;
    volatile short aScript;
    volatile short bHelperScript;
    volatile short bScript;
    short *partsA = (short *)&a->body;
    short *partsB = (short *)&b->body;
    View *helperA;

    aExtraScript = 0;
    bExtraScript = 0;
    aScript = 0;
    aHelperScript = 0;
    bScript = 0;
    bHelperScript = 0;
    short direction = partsA[20];

    switch (direction) {
    case 0:
        switch (partsB[20]) {
        case 0:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10018;
            bExtraScript = 10012;
            break;
        case 1:
            aScript = 15050;
            aHelperScript = 10008;
            bScript = 15045;
            bHelperScript = 10006;
            aExtraScript = 10027;
            bExtraScript = 10018;
            break;
        case 2:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10018;
            bExtraScript = 10012;
            break;
        case 3:
            aScript = 15040;
            aHelperScript = 10010;
            bScript = 15045;
            bHelperScript = 10006;
            aExtraScript = 10024;
            bExtraScript = 10018;
            break;
        }
        break;
    case 1:
        switch (partsB[20]) {
        case 0:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15050;
            bHelperScript = 10008;
            aExtraScript = 10018;
            bExtraScript = 10027;
            break;
        case 1:
            aScript = 15040;
            aHelperScript = 10010;
            bScript = 15050;
            bHelperScript = 10009;
            aExtraScript = 10024;
            bExtraScript = 10027;
            break;
        case 2:
            aScript = 15035;
            aHelperScript = 10004;
            bScript = 15050;
            bHelperScript = 10008;
            aExtraScript = 10012;
            bExtraScript = 10027;
            break;
        case 3:
            aScript = 15040;
            aHelperScript = 10005;
            bScript = 15050;
            bHelperScript = 10007;
            aExtraScript = 10021;
            bExtraScript = 10015;
            break;
        }
        break;
    case 2:
        switch (partsB[20]) {
        case 0:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10018;
            bExtraScript = 10012;
            break;
        case 1:
            aScript = 15050;
            aHelperScript = 10008;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10027;
            bExtraScript = 10012;
            break;
        case 2:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10018;
            bExtraScript = 10012;
            break;
        case 3:
            aScript = 15040;
            aHelperScript = 10011;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10024;
            bExtraScript = 10012;
            break;
        }
        break;
    case 3:
        switch (partsB[20]) {
        case 0:
            aScript = 15045;
            aHelperScript = 10006;
            bScript = 15040;
            bHelperScript = 10010;
            aExtraScript = 10018;
            bExtraScript = 10024;
            break;
        case 1:
            aScript = 15040;
            aHelperScript = 10005;
            bScript = 15050;
            bHelperScript = 10007;
            aExtraScript = 10021;
            bExtraScript = 10015;
            break;
        case 2:
            aScript = 15040;
            aHelperScript = 10011;
            bScript = 15035;
            bHelperScript = 10004;
            aExtraScript = 10024;
            bExtraScript = 10012;
            break;
        case 3:
            aScript = 15040;
            aHelperScript = 10010;
            bScript = 15050;
            bHelperScript = 10009;
            aExtraScript = 10024;
            bExtraScript = 10027;
            break;
        }
        break;
    }
    helperA = findView(partsA[41]);
    if (helperA && aHelperScript) {
        helperA->flags = 0x988000;
        setViewScript(helperA, aHelperScript, 1);
        *(Point *)&helperA->body.x = *(Point *)&a->body.x;
        helperA->placed = placeOnHotSpot35;
        offset = (g_4afc24 + partsA[33] * 12)[partsA[34]];
        where = *(Point *)&helperA->body.x;
        partsA[42] = addView(0x4988000, drawCels, runViewScript, aExtraScript + offset, 7, &where, 0, 0);
        helperA = findView(partsA[42]);
        if (helperA) {
            short *its = (short *)&helperA->body;

            its[50] = a->id;
            setViewScript(helperA, aExtraScript + offset, 1);
            helperA->placed = placeOnHotSpot35;
            helperA->notify = startPairNotify;
            runViewScript(helperA, removedRgn);
        }
    }
    helperB = findView(partsB[41]);
    if (helperB && bHelperScript) {
        helperB->flags = 0x988000;
        setViewScript(helperB, bHelperScript, 1);
        *(Point *)&helperB->body.x = *(Point *)&b->body.x;
        helperB->placed = placeOnHotSpot35;
        offset = (g_4afc24 + partsB[33] * 12)[partsB[34]];
        where = *(Point *)&helperB->body.x;
        partsB[42] = addView(0x4988000, drawCels, runViewScript, bExtraScript + offset, 7, &where, 0, 0);
        helperB = findView(partsB[42]);
        if (helperB) {
            short *its = (short *)&helperB->body;

            its[50] = b->id;
            setViewScript(helperB, bExtraScript + offset, 1);
            helperB->placed = placeOnHotSpot35;
            helperB->notify = startPairNotify;
            runViewScript(helperB, removedRgn);
        }
    }
    squareOccupants[partsA[33]][partsA[34]][0] = 0;
    squareOccupants[partsA[33]][partsA[34]][1] = 0;
    if (aScript)
        startSnoidScript((Snoid *)&a->body, aScript + partsA[45], 0, 1);
    if (bScript)
        startSnoidScript((Snoid *)&b->body, bScript + partsB[45], 0, 1);
    moveView(b->id, 0, g_4afd8c[partsB[34]]);
    moveView(partsB[41], 1, b->id);
    moveView(a->id, 0, g_4afd8c[partsA[34]]);
    moveView(partsA[41], 1, a->id);
    if (helperA)
        groupViews(partsA[41], a->id, partsA[42], 0, 0, 0);
    if (helperB)
        groupViews(partsB[41], b->id, partsB[42], 0, 0, 0);
}

/* Splices a list in after another. */
/* @zoombi32 0x0043a772 */
void spliceList(Link *other, Link *list)
{
    if (other && list) {
        Link *last = list;
        while (last->next)
            last = last->next;
        list->prev = other;
        last->next = other->next;
        other->next = list;
        last->next->prev = last;
    }
}

/*
 * Which of the four groups of three scenes (7-18) the current scene is in
 * (1-4; 0 if none), and in *last whether it's the group's last.
 */
/* @zoombi32 0x0043af02 */
short sceneGroup(short *last)
{
    short group = 0;

    *last = 0;
    if (currentScene >= 7 && currentScene <= 18) {
        if (currentScene == 9 || currentScene == 12 || currentScene == 15 || currentScene == 18)
            *last = 1;
        group = ((currentScene - 7) / 3 & 3) + 1;
    }
    return group;
}

/* Sets up a maze Zoombini's parts (its body's words 20-45): its scripts
   for each move, by its feet. */
/* @zoombi32 0x00439560 */
void setUpMazeParts(Snoid *snoid)
{
    short unused[2];
    short *parts = (short *)snoid;

    parts[20] = -1;
    parts[21] = snoid->features[3] + 15014;
    parts[22] = snoid->features[3] + 15019;
    parts[23] = snoid->features[3] + 15024;
    parts[24] = snoid->features[3] + 15029;
    parts[25] = snoid->features[3] + 15055;
    parts[26] = snoid->features[3] + 15060;
    parts[27] = snoid->features[3] + 15065;
    parts[28] = snoid->features[3] + 15070;
    parts[29] = 0;
    parts[31] = 0;
    parts[32] = 0;
    parts[33] = 0;
    parts[34] = 0;
    parts[35] = 0;
    parts[36] = 0;
    parts[37] = snoid->features[3] + 14999;
    parts[38] = snoid->features[3] + 15004;
    parts[39] = snoid->features[3] + 15009;
    parts[41] = 0;
    parts[42] = 0;
    parts[45] = snoid->features[3] - 1;
}

/* Sorts a list of views by where they stand (their bounds' bottom, then
   left), for drawing back to front; returns the new head. */
/* @zoombi32 0x0043a69a */
View *sortViewsByDepth(View *list)
{
    View *sorted;
    View *view;
    View *at;
    ShortRect other;
    ShortRect bounds;

    view = at = 0;
    if (list) {
        sorted = at = list;
        list = list->next;
        sorted->prev = 0;
        sorted->next = 0;
    } else {
        return 0;
    }
    while (list) {
        view = list;
        list = list->next;
        bounds = view->body.bounds;
        for (at = sorted; at;) {
            other = at->body.bounds;
            if (bounds.bottom < other.bottom || bounds.bottom == other.bottom && bounds.left < other.left) {
                view->prev = at->prev;
                view->next = at;
                at->prev = view;
                if (view->prev)
                    view->prev->next = view;
                else
                    sorted = view;
                at = 0;
            } else if (!at->next) {
                at->next = view;
                view->prev = at;
                view->next = 0;
                at = 0;
            } else {
                at = at->next;
            }
        }
    }
    return sorted;
}

/* Takes the views with exactly `flags` out of the view list, sorts them
   (sortViewsByDepth) and puts them back after `after`. */
/* @zoombi32 0x0043a5f6 */
void sortFlaggedViews(View *after, unsigned long flags)
{
    View *first;
    View *view;
    View *prev;
    View *following;
    View *last;
    View *next;

    if (after && flags) {
        next = viewListEnd(1)->next;
        first = 0;
        last = 0;
        while (next) {
            view = next;
            next = next->next;
            if (view->flags == flags) {
                if (!first) {
                    first = last = view;
                    prev = view->prev;
                    following = view->next;
                    if (prev)
                        prev->next = following;
                    if (following)
                        following->prev = prev;
                    view->prev = 0;
                    view->next = 0;
                } else {
                    last->next = view;
                    prev = view->prev;
                    following = view->next;
                    if (prev)
                        prev->next = following;
                    if (following)
                        following->prev = prev;
                    view->prev = last;
                    view->next = 0;
                    last = view;
                }
            }
        }
        if (first)
            spliceList((Link *)after, (Link *)sortViewsByDepth(first));
    }
}

/* A view's placing: a second cel, from its word 20, where the first is. */
/* @zoombi32 0x0043d6e2 */
void placeSecondCel(View *view)
{
    short *parts = (short *)&view->body;

    parts[3] = parts[20];
    parts[4] = parts[1];
    parts[5] = parts[2];
    parts[6] = 0;
}

/* Closes the scene. */
/* @zoombi32 0x0043b820 */
void closeNet()
{
    if (g_4b12a8) {
        g_4b12a8 = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&g_4a2e54);
        setFreeAtOnce(saved);
        closeGameFile(&g_4b12a4);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Fills column n % 5 of codeColumns with `a` and row n / 5 of codeRows with
   `b` (5 by 5). */
/* Not exact: the original keeps `n` in esi and `row` in edi; BCC puts `n`
   in ecx and `row` in esi. */
/* @zoombi32 0x0043c8e2 */
void fillCodeRowColumn(short a, short b, short n)
{
    short row;
    short rowStart;
    short column;
    short i;

    row = n / 5;
    rowStart = n - n % 5;
    column = n - row * 5;
    for (i = 0; i < 5; i++) {
        codeColumns[column + i * 5] = a;
        codeRows[rowStart + i] = b;
    }
}

/* The same for cube n (5 by 5 by 5): its row in codeColumns and codeRows,
   and `c` in codeLayers by its column. */
/* @zoombi32 0x0043c94f */
void fillCodeCube(short a, short b, short c, short n)
{
    short rest;
    short plane;
    short column;
    short i;

    plane = n / 25;
    rest = n % 25;
    plane *= 5;
    rest /= 5;
    column = n % 5;
    for (i = 0; i < 5; i++) {
        codeColumns[rest + i * 5] = a;
        codeRows[plane + i] = b;
    }
    codeLayers[column] = c;
}

/*
 * Opens the net (scene 15): resets its state, picks three random
 * codes, loads Net.MHK's backdrop (by level), features, scripts and
 * images, brings the party in, sets up the codes and the net, and shows
 * the hint (sound 20064 next).
 */
/* @zoombi32 0x0043b20c */
void openNet()
{
    Point places[16] = {{233, 392}, {209, 378}, {196, 390}, {185, 365}, {167, 380}, {160, 408},
                        {135, 397}, {121, 407}, {115, 368}, {114, 342}, {99, 375},  {97, 394},
                        {95, 346},  {91, 411},  {79, 355},  {62, 404}};

    snoidsOnTheirWay = snoidsArrived = hintSound = 0;
    netLevel = sceneLevel();
    g_4b142e = 25;
    if (netLevel > 1)
        g_4b142e = 125;
    g_4b141a = g_4b141c = g_4b1420 = g_4b1422 = 0;
    g_4b1424 = g_4b1426 = g_4b1428 = g_4b141e = 0;
    g_4b1410 = g_4b1416 = g_4b142c = g_4b11a0 = 0;
    g_4b142a = g_4b1408 = g_4b1450 = g_4b13c8 = g_4b1456 = 0;
    g_4b1466 = g_4b12ae = g_4b1414 = g_4b1412 = 0;
    g_4b13fe = g_4b1400 = g_4b1402 = g_4b1404 = 0;
    g_4b13ca = g_4b0d5c = g_4b119e = -1;
    g_4b11a4 = g_4b1418 = 1;
    g_4b1440 = g_4b1444 = g_4b1448 = -1;
    g_4b144c = g_4b144e = g_4b11a6 = 0;
    g_4b1438[0] = g_4b1438[1] = g_4b1438[2] = g_4b144a = 0;
    g_4b12b0[0] = g_4b12b0[1] = g_4b12b0[2] = 0;
    g_4b1430 = g_4b1462 = g_4b11a8 = g_4b147e = 0;
    g_4b1432 = g_4b1434 = g_4b1436 = 0;
    g_4b119a = g_4b119c = g_4b0e6a = g_4b0e6c = 0;
    g_4b145e = g_4b1460 = g_4b1458 = g_4b145a = 0;
    g_4b1464 = g_4b145c = 0;
    g_4b1406 = 1;
    sceneDue = g_4b147c = 0;
    g_4b12a8 = g_4b12aa = 0;
    g_4b143e = randomUpTo(4);
    g_4b1442 = randomUpTo(4);
    g_4b1446 = randomUpTo(4);
    fillMemory(g_4b13cc, 0, 50);
    unloadSounds();
    openGameFile(&g_4b12a4, "Net.MHK");
    setCurrentMap(g_4b12a4);
    drawBackdrop((netLevel >= 2) + 5000);
    loadFeatureGroup(7000, 0, 1);
    loadFeatureGroup(8000, 1, 0);
    loadFeatureGroup(9000, 2, 1);
    loadFeatureGroup(10000, 3, 0);
    loadScripts(7000, 48);
    addScripts(8000, 8, 0);
    addScripts(9000, 154, 0);
    addScripts(10000, 19, 0);
    g_4a2e60 = loadImageBank(6000, &g_4a2e54);
    loadSnoidScripts(14000, 3, 0);
    addSnoidScripts(13000, 51, 0);
    addView(0x1000, drawNetButtons, updateNetButtons, 0, 0, 0, 0, 0);
    setViewPlaces(16, places, 1);
    copyPaletteRange(10, 236);
    makePartySnoids(0);
    enterSnoids(0);
    g_4b0d68 = listChosenSnoids();
    netPartySize = g_4b0d68->count;
    g_4b1478 = 3;
    if (*(short *)(gameState + 0x20))
        g_4b1478 = 2;
    g_4b147a = 0;
    g_4b0e68 = 0;
    splitIntoGroups();
    g_4b140a = (netLevel == 3) + g_4b0e76 + 7;
    g_4b140e = g_4b140a;
    g_4b140c = 16 - g_4b140a;
    addNetViews();
    updateViews();
    staggerSnoids(30, 0);
    chooseSnoids(0, 0);
    setGroupLists(g_4a2e32, 1, (short)0xc000);
    drawNetButton(1, 0, 0);
    drawNetButton(2, 0, 0);
    visitAllItems();
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(300, 324, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(8000, 8002, 0);
    addSoundRange(9000, 10999, 0);
    addSoundRange(7000, 7999, 0);
    addSoundRange(10000, 10099, 0);
    showRect(&shownGameRect);
    fadeInViews();
    g_4b12a8 = 1;
    setViewsLocked(0);
    campHint((short *)(gameState + 0x3c));
    hintSound = 20064;
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   g_4b12aa), lit or not, and with `show` shows it. */
/* @zoombi32 0x0043b6f8 */
void drawNetButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b12aa) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a2e60->offsets[image] + (char *)g_4a2e60), netButtons[which].rect.left,
                      netButtons[which].rect.top, 8);
        if (show)
            showRect(&netButtons[which].rect);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x0043b791 */
void drawNetButtons(View *)
{
    drawNetButton(1, 0, 0);
    drawNetButton(2, 0, 0);
}

/* A view's update: adds buttons 3 (when g_4b12aa changes) and 2 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0043b7ae */
void updateNetButtons(View *, short region)
{
    if (g_4b12aa) {
        if (!g_4a2ea4) {
            g_4a2ea4 = 1;
            unionRgnRect(region, &netButtons[3].rect);
        }
    } else if (g_4a2ea4) {
        g_4a2ea4 = 0;
        unionRgnRect(region, &netButtons[3].rect);
    }
    if (!g_4a2ea6) {
        g_4a2ea6 = 1;
        unionRgnRect(region, &netButtons[2].rect);
    }
}

/* Splits the netPartySize Zoombinis into groups (netGroups, g_4b0e76 of them)
   of three, two and one in turn, then evens out the overshoot by taking
   one from groups of two or more; if that can't be done, one each. */
/* Not exact: the original caches g_4b0e76's address in edi, and negates
   `left` through a 32-bit copy (movsx eax, dx / mov edx, eax / neg eax /
   mov edx, eax), perhaps an inline function's parameter. */
/* @zoombi32 0x0043e370 */
void splitIntoGroups()
{
    short i;
    short size;
    short n;
    short left;

    for (i = 0; i < 12; i++)
        netGroups[i] = 0;
    left = netPartySize;
    size = 4;
    n = 0;
    do {
        size--;
        if (size < 1)
            size = 3;
        netGroups[n] = size;
        n++;
        left -= size;
    } while (left > 0);
    g_4b0e76 = n;
    if (left) {
        left = -left;
        do {
            for (i = 0; i < g_4b0e76; i++)
                if (netGroups[i] >= 2 && left) {
                    netGroups[i]--;
                    left--;
                }
            if (left) {
                n = 0;
                for (i = 0; i < g_4b0e76; i++)
                    if (g_4b11aa[i] > 1)
                        n++;
                if (!n) {
                    g_4b0e76 = netPartySize;
                    for (i = 0; i < g_4b0e76; i++)
                        netGroups[i] = 1;
                    left = 0;
                }
            }
        } while (left);
    }
}

/*
 * Steps a maze Zoombini on a square in its direction (word 20, set from
 * word 38 of the view `other`, which, if its word 39 is set, first turns
 * to its next open way (words 34-37; sound 5101/5102 in turn) and gets
 * pose 3). Lands on a square of kind 5 (squareKinds): that square's view
 * gives up its partner (word 43, listed in g_4b0930), which takes the
 * direction. Then places it and starts its walking script (from word 21)
 * with its helper view's (script 10000 on), grouped. Reads `other`'s words
 * even when there's no such view.
 */
/* Not exact: the original keeps `other` in eax from the start (loading it
   before `view`); otherwise the same. */
/* @zoombi32 0x0043a2c8 */
void stepMazeSnoid(View *view, short other)
{
    short *parts = (short *)&view->body;
    View *paired = findView(other);
    short *its;
    View *helper;

    if (paired)
        its = (short *)&paired->body;
    parts[20] = its[38];
    if (its[39]) {
        queueViewSound(g_4a2116 + 5101, 0);
        g_4a2116++;
        if (g_4a2116 > 1)
            g_4a2116 = 0;
        its[38]++;
        if (its[38] > 3)
            its[38] = 0;
        while (!its[34 + its[38]]) {
            its[38]++;
            if (its[38] > 3)
                its[38] = 0;
        }
        ((Snoid *)&paired->body)->unknownF4 = 3;
    }
    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34] = 0;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33] = 12;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34] = 12;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33] = 0;
        break;
    }
    short kind = squareKinds[parts[33]][parts[34]];

    if (kind == 5) {
        paired = findView(squareViews[parts[33]][parts[34]]);
        if (paired) {
            its = (short *)&paired->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                paired = findView(its[43]);
                its[43] = 0;
                if (paired) {
                    its = (short *)&paired->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (squarePlaces + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = placeOnHotSpot35;
    }
    short script = parts[21 + parts[20]];

    startSnoidScript((Snoid *)&view->body, script, 0, 0);
    view->notify = mazeSnoidNotify;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* Moves a maze Zoombini on to the square it's heading for (words 33 and
   34), pairs the view `other` with it, and starts its script for its pose
   (from words 25 on) with its helper view's (script 10036 on), grouped. */
/* @zoombi32 0x0043a510 */
void moveMazeSnoidOn(View *view, short other)
{
    short *parts = (short *)&view->body;
    View *helper;
    View *paired;

    parts[31] = parts[33];
    parts[32] = parts[34];
    paired = findView(other);
    if (paired) {
        short *its = (short *)&paired->body;

        its[43] = view->id;
    }
    *(Point *)&view->body.x = (squarePlaces + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10036, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = placeOnHotSpot35;
    }
    startSnoidScript((Snoid *)&view->body, parts[25 + parts[20]], 0, 0);
    view->notify = mazeSnoidNotify;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* Sends the next Zoombini of the party (g_4b0e68 of netPartySize) to the
   first free place of three (g_4b1438, from the last), while g_4b144a lets
   it (for g_4b145c more); counts in g_4b11a0 the places left when none
   are left to send. */
/* @zoombi32 0x0043cfc3 */
void sendNextToNet()
{
    Point target;
    short i;
    View *view;

    target.x = 233;
    target.y = 392;
    for (i = 2; i >= 0; i--)
        if (!g_4b1438[i]) {
            if (g_4b0e68 < netPartySize) {
                if (!g_4b144a)
                    return;
                if (!--g_4b145c)
                    g_4b144a = 0;
                view = findView(partyViews[g_4b0e68]);
                if (view) {
                    setSnoidAction((Snoid *)&view->body, 10, 0);
                    *(Point *)&((Snoid *)&view->body)->targetX = target;
                    g_4b0d5c = g_4b0e68;
                    g_4b1438[i] = partyViews[g_4b0e68];
                    g_4b0e68++;
                    g_4b119a = i;
                    g_4b1462 = 1;
                    return;
                }
            } else {
                g_4b11a0++;
            }
        }
    g_4b145c = 0;
    g_4b144a = 0;
}

/* Puts a maze Zoombini on its square (words 33 and 34) with its helper
   view (script 10030, told turnOrStartPairNotify) and a second view it adds (word 42:
   10031), and starts its script 14006 (then told helperDoneNotify), grouped. */
/* @zoombi32 0x00439fc3 */
void putOnSquare(View *view, short)
{
    short *parts = (short *)&view->body;
    View *helper;
    Point where;

    *(Point *)&view->body.x = (squarePlaces + parts[34])[parts[33] * 13];
    view->body.x += 3;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, 10030, 1);
        *(Point *)&helper->body.x = *(Point *)&view->body.x;
        helper->placed = placeOnHotSpot35;
        helper->notify = turnOrStartPairNotify;
        where = *(Point *)&helper->body.x;
        parts[42] = addView(0x4988000, drawCels, runViewScript, 10031, 7, &where, 0, 0);
        helper = findView(parts[42]);
        if (helper) {
            setViewScript(helper, 10031, 1);
            helper->placed = placeOnHotSpot35;
        }
        view->body.x += 17;
        view->body.y += 5;
        startSnoidScript((Snoid *)&view->body, 14006, 0, 1);
        view->notify = helperDoneNotify;
        moveView(parts[42], 0, view->id);
        groupViews(parts[41], view->id, parts[42], 0, 0, 0);
    }
}

/* Stops a maze Zoombini (state 2) and moves on the Zoombinis in its
   line's list (by its word 33): those in pose 4 turn to their next
   direction (word 38) with a way open (words 34-37); those in pose 5 with
   a partner (word 43) list it in g_4b0930. */
/* @zoombi32 0x00439e55 */
void stopMazeSnoid(short id)
{
    View *view = findView(id);
    short *list;
    short n;

    if (view) {
        ((Snoid *)&view->body)->unknownF4 = 2;
        short *parts = (short *)&view->body;

        parts[46] = 0;
        switch (parts[33]) {
        case 1:
            list = g_4b0a10;
            n = 0;
            break;
        case 2:
            list = g_4b0b6e;
            n = g_4b0d00;
            break;
        case 3:
            list = g_4b0ba0;
            n = g_4b0d02;
            break;
        case 4:
            list = g_4b0bd2;
            n = g_4b0d04;
            break;
        case 5:
            list = g_4b0c04;
            n = g_4b0d06;
            break;
        case 6:
            list = g_4b0c36;
            n = g_4b0d08;
            break;
        case 7:
            list = g_4b0c68;
            n = g_4b0d0a;
            break;
        case 8:
            list = g_4b0c9a;
            n = g_4b0d0c;
            break;
        default:
            list = g_4b0a10;
            n = 0;
            break;
        }
        while (n) {
            n--;
            view = findView(list[n]);
            if (view) {
                parts = (short *)&view->body;
                if (parts[30] == 4) {
                    parts[38]++;
                    if (parts[38] > 3)
                        parts[38] = 0;
                    while (!parts[34 + parts[38]]) {
                        parts[38]++;
                        if (parts[38] > 3)
                            parts[38] = 0;
                    }
                    ((Snoid *)&view->body)->unknownF4 = 3;
                }
                if (parts[30] == 5 && parts[43]) {
                    g_4b0930[g_4b09fe] = parts[43];
                    g_4b09fe++;
                    parts[43] = 0;
                }
            }
        }
    }
}

/* Sends a Zoombini's marker flying from its place (g_4a2b7e, or g_4a2be2
   at the higher levels) to (484, 318) in six steps (updateFlyingMarker), unless one
   is flying already. */
/* @zoombi32 0x0043e435 */
void flyMarker(short n)
{
    short scripts[3] = {1, 0, 2};

    if (!g_4b1456) {
        g_4b119e = n;
        g_4b1456++;
        if (netLevel <= 1) {
            g_4b1452 = g_4a2b7e[n].x;
            g_4b1454 = g_4a2b7e[n].y;
        } else {
            g_4b1452 = g_4a2be2[n].x;
            g_4b1454 = g_4a2be2[n].y;
        }
        g_4b117e = (484 - g_4b1452) / 6;
        g_4b1180 = (318 - g_4b1454) / 6;
        g_4b1452 = 484;
        g_4b1454 = 318;
        g_4b13ca++;
        g_4b13cc[g_4b13ca] = addView(0x4100000, drawCels, updateFlyingMarker, scripts[g_4a2e58] + 7020, 6, 0, 0, 0);
        View *view = findView(g_4b13cc[g_4b13ca]);

        if (view) {
            view->unknown1e = n;
            g_4b1450++;
            view->placed = markerPlaced;
            view->interval = 3;
            moveView(g_4b13cc[g_4b13ca], 0, g_4b12b6);
        }
    }
}

/* The flying marker's update: steps it on (g_4b1452/4 by g_4b117e/80),
   and after five steps (or when the game says) lands it (landMarker). */
/* @zoombi32 0x0043e5a7 */
void updateFlyingMarker(View *view, short region)
{
    if (++g_4b1456 > 5 || *(short *)(gameState + 0x20)) {
        g_4b1456 = 0;
        view->update = runViewScript;
        landMarker(view->unknown1e);
    } else {
        g_4b1452 -= g_4b117e;
        g_4b1454 -= g_4b1180;
    }
    runViewCels(view, region);
    waitForEventFor(0, 2, 0, 1);
}

/* Lands the flying marker for place n: shows it (script 7023, or 7024 at
   the higher levels) there, and counts that place's group (g_4b11aa, by
   g_4b119e) into g_4b0e6c; a place with none counts in g_4b144e. */
/* @zoombi32 0x0043da30 */
void landMarker(short n)
{
    View *view;

    if (n >= 0) {
        if (netLevel <= 1) {
            g_4b1452 = g_4a2b7e[n].x;
            g_4b1454 = g_4a2b7e[n].y;
        } else {
            g_4b1452 = g_4a2be2[n].x;
            g_4b1454 = g_4a2be2[n].y;
        }
        g_4b1450++;
        view = findView(g_4b13cc[g_4b13ca]);
        if (view) {
            if (netLevel < 2)
                setViewScript(view, 7023, 1);
            else
                setViewScript(view, 7024, 1);
        } else {
            if (netLevel < 2)
                g_4b13cc[g_4b13ca] = addView(0x4108000, drawCels, runViewScript, 7023, 6, 0, 0, 0);
            else
                g_4b13cc[g_4b13ca] = addView(0x4108000, drawCels, runViewScript, 7024, 6, 0, 0, 0);
            view = findView(g_4b13cc[g_4b13ca]);
        }
        if (view) {
            view->placed = markerPlaced;
            moveView(g_4b13cc[g_4b13ca], 0, g_4b12ba[0]);
        }
        g_4b144e = 0;
        if ((g_4b145a = g_4b11aa[g_4b119e]) < 1) {
            g_4b145a = 0;
            g_4b1466 = 0;
            g_4b144e++;
        } else {
            g_4b147c++;
        }
        g_4b0e6c += g_4b145a;
        g_4b11aa[n] = -1;
    }
}

/* Steps a maze Zoombini one square on in its direction (word 20; within
   the 13 by 13 board); a square of kind 5 there hands its partner on (to
   g_4b0930, turned the same way). Then puts it there with its helper view
   (script 10000 on) and starts its script. */
/* @zoombi32 0x00439cb4 */
void stepMazeSnoidOn(View *view)
{
    short *parts = (short *)&view->body;
    short *its;
    View *helper;

    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34]++;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33]--;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34]--;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33]++;
        break;
    }
    short kind = squareKinds[parts[33]][parts[34]];

    if (kind == 5) {
        View *other = findView(squareViews[parts[33]][parts[34]]);

        if (other) {
            its = (short *)&other->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                View *partner = findView(its[43]);

                its[43] = 0;
                if (partner) {
                    its = (short *)&partner->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (squarePlaces + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = placeOnHotSpot35;
    }
    startSnoidScript((Snoid *)&view->body, parts[21 + parts[20]], 0, 0);
    view->notify = mazeSnoidNotify;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* As stepMazeSnoidOn for a Zoombini reaching a turning view (`other`): if its
   feature (the view's word 41) is the view's (word 42), it turns the
   view's way (word 38) first; it stops at the board's edge. */
/* Not exact: the original keeps `other` in eax from the start (loading it
   before `snoid` and `parts`); this loads it at the call. */
/* @zoombi32 0x0043a0e8 */
void stepAtTurning(View *view, short other)
{
    Snoid *snoid;
    short *parts;
    short *its;
    View *helper;
    View *turning;

    snoid = (Snoid *)&view->body;
    parts = (short *)&view->body;
    turning = findView(other);
    if (turning)
        its = (short *)&turning->body;
    if (snoid->features[its[41] - 1] == its[42])
        parts[20] = its[38];
    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34] = 0;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33] = 12;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34] = 12;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33] = 0;
        break;
    }
    short kind = squareKinds[parts[33]][parts[34]];

    if (kind == 5) {
        View *square = findView(squareViews[parts[33]][parts[34]]);

        if (square) {
            its = (short *)&square->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                View *partner = findView(its[43]);

                its[43] = 0;
                if (partner) {
                    its = (short *)&partner->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (squarePlaces + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = placeOnHotSpot35;
    }
    short script = parts[21 + parts[20]];

    startSnoidScript((Snoid *)&view->body, script, 0, 0);
    view->notify = mazeSnoidNotify;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* Draws the box at the top right with two codes ("SC", "SH" or "MC", by
   g_4b1178 and g_4b117a) and a third (by g_4b117c from level 2, else
   "PR"). */
/* @zoombi32 0x0043d524 */
void drawCodesBox()
{
    ShortRect whole = {500, 1, 600, 27};
    ShortRect left = {500, 1, 549, 27};
    ShortRect right = {550, 1, 600, 27};
    Color saved;
    char names[3][3] = {"SC", "SH", "MC"};

    saved = setForeColor(Color(0xb));
    fillPortRect(Rect(whole), Color(0xe), 0);
    frameRect(Rect(whole));
    drawText(Rect(left), 0x22, names[g_4b1178], 0xffff);
    drawText(Rect(whole), 0x22, names[g_4b117a], 0xffff);
    if (netLevel >= 2)
        drawText(Rect(right), 0x22, names[g_4b117c], 0xffff);
    else
        drawText(Rect(right), 0x22, "PR", 0xffff);
    setForeColor(saved);
    showRect(&whole);
}

/* Adds the scene's standing views: five at 8000 on (g_4b12ba), 8005, the
   guide (9151, or 9153 at the higher levels; grouped), 7018, 10018, the
   three code views (10002, 10007 and 10012 on, by g_4b143e/42/46; the
   first from level 2) and 7000 (grouped). */
/* @zoombi32 0x0043c6df */
void addNetViews()
{
    short i;

    for (i = 0; i < 5; i++)
        g_4b12ba[i] = addView(0x4188000, drawCels, runViewScript, i + 8000, 6, 0, 0, 0);
    g_4b12b8 = addView(0x4188000, drawCels, runViewScript, 8005, 6, 0, 0, 0);
    if (netLevel <= 1)
        g_4b12ca = addView(0x4108000, drawCels, runViewScript, 9151, 6, 0, 0, 0);
    else
        g_4b12ca = addView(0x4108000, drawCels, runViewScript, 9153, 6, 0, 0, 0);
    g_4b141e = groupViews(g_4b12ca, g_4b12ca, 0, 0, 0, 0);
    g_4b12b6 = addView(0x4181000, drawCels, runViewScript, 7018, 6, 0, 0, 0);
    g_4b13fe = addView(0x188000, drawCels, runViewScript, 10018, 6, 0, 0, 0);
    if (netLevel >= 2)
        g_4b1400 = addView(0x4108000, drawCels, runViewScript, g_4b143e + 10002, 6, 0, 0, 0);
    g_4b1402 = addView(0x4108000, drawCels, runViewScript, g_4b1442 + 10007, 6, 0, 0, 0);
    g_4b1404 = addView(0x4108000, drawCels, runViewScript, g_4b1446 + 10012, 6, 0, 0, 0);
    g_4b13c6 = addView(0x4188000, drawCels, runViewScript, 7000, 6, 0, 0, 0);
    g_4b142a = groupViews(g_4b13c6, g_4b13c6, 0, 0, 0, 0);
}

/* Scene 15's keys (debugging ones only while debugging messages are on). */
/* @zoombi32 0x0043c655 */
short netKey(unsigned short key)
{
    if (!debugMessagesOn && key != 367)
        return 0;
    switch (key) {
    case 367:
        replayHint();
        return 1;
    case 'L':
    case 'l':
        drawCodesBox();
        return 1;
    case ' ':
        g_4b140a = g_4b140e;
        g_4b140c = 17 - g_4b140a;
        if (g_4b142c) {
            g_4b142c = 0;
            g_4b1410++;
        }
        return 1;
    }
    return 0;
}

/*
 * Which entry (of g_4b142e) of the tables codeColumns and codeRows (and, from
 * level 2 (netLevel), codeLayers) holds the codes g_4b1446 and g_4b1442 (and
 * g_4b143e), in the order g_4b1178 (levels 0-1) or g_4b1468 (from level 2)
 * says; -1 if none.
 */
/* @zoombi32 0x0043dbf3 */
short findCodeEntry()
{
    short i;

    switch (netLevel) {
    case 0:
    case 1:
        if (g_4b1178 == 2) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b1446 && codeRows[i] == g_4b1442)
                    return i;
        } else {
            for (i = 0; i < g_4b142e; i++)
                if (codeRows[i] == g_4b1446 && codeColumns[i] == g_4b1442)
                    return i;
        }
        break;
    case 2:
    case 3:
        if (g_4b1468 == 0) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b1446 && codeRows[i] == g_4b1442 && codeLayers[i] == g_4b143e)
                    return i;
        } else if (g_4b1468 == 1) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b1442 && codeRows[i] == g_4b1446 && codeLayers[i] == g_4b143e)
                    return i;
        } else if (g_4b1468 == 2) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b143e && codeRows[i] == g_4b1446 && codeLayers[i] == g_4b1442)
                    return i;
        } else if (g_4b1468 == 3) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b1446 && codeRows[i] == g_4b143e && codeLayers[i] == g_4b1442)
                    return i;
        } else if (g_4b1468 == 4) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b1442 && codeRows[i] == g_4b143e && codeLayers[i] == g_4b1446)
                    return i;
        } else if (g_4b1468 == 5) {
            for (i = 0; i < g_4b142e; i++)
                if (codeColumns[i] == g_4b143e && codeRows[i] == g_4b1442 && codeLayers[i] == g_4b1446)
                    return i;
        }
        break;
    }
    return -1;
}

/*
 * Plays a scene's ambient sounds: every 3-4 seconds, unless the last is
 * still playing, a random one of the scene's (none in scenes 6 and 14),
 * not repeating one until all have played; every 16th time, first unloads
 * sounds 900-944.
 */
/* @zoombi32 0x0043af6b */
void playAmbientSound()
{
    unsigned long now;
    short sound;
    short i;

    if (soundOn && musicOn) {
        now = clockTime();
        if (now >= ambientSoundTime) {
            if (isSoundPlaying(ambientSound, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                ambientSoundTime = now + randomBetween(180, 240);
                return;
            }
            ambientSoundTime = now + randomBetween(180, 240);
            sound = 0;
            switch (currentScene) {
            case 7:
                sound = bridgeSounds[allocateSlot(&bridgeSoundsUsed, 9, 0)];
                break;
            case 8:
                sound = tunnelsSounds[allocateSlot(&tunnelsSoundsUsed, 9, 0)];
                break;
            case 9:
                sound = pizzaSounds[allocateSlot(&pizzaSoundsUsed, 12, 0)];
                break;
            case 4:
                sound = campSounds[allocateSlot(&campSoundsUsed, 15, 0)];
                break;
            case 10:
                sound = ferrySounds[allocateSlot(&ferrySoundsUsed, 19, 0)];
                break;
            case 11:
                sound = lillySounds[allocateSlot(&lillySoundsUsed, 20, 0)];
                break;
            case 12:
                sound = stoneRiseSounds[allocateSlot(&stoneRiseSoundsUsed, 13, 0)];
                break;
            case 5:
                sound = camp2Sounds[allocateSlot(&camp2SoundsUsed, 10, 0)];
                break;
            case 13:
                sound = fleensSounds[allocateSlot(&fleensSoundsUsed, 13, 0)];
                break;
            case 15:
                sound = netSounds[allocateSlot(&netSoundsUsed, 17, 0)];
                break;
            case 16:
                sound = cavesSounds[allocateSlot(&cavesSoundsUsed, 10, 0)];
                break;
            case 18:
                sound = mazeSounds[allocateSlot(&mazeSoundsUsed, 10, 0)];
                break;
            case 17:
                sound = smokeSounds[allocateSlot(&smokeSoundsUsed, 10, 0)];
                break;
            }
            if (sound) {
                ambientSoundCount++;
                ambientSoundCount %= 16;
                if (!ambientSoundCount)
                    for (i = 900; i <= 944; i++)
                        unloadSoundNow(i, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                queueViewSound(sound, 0);
                ambientSound = sound;
            }
        }
    }
}

/*
 * Moves on to the next scene (pendingScene): leaving a group's last puzzle
 * (9, 12, 15, 18) for its camp notes the level passed in the game state
 * (puzzleLeft: the puzzle left). Whether to go by the map (scene 2) first
 * depends on the scenes left and entered (not while practiceLevel, g_4b7562 or
 * transitionsOn; always while journeyRoute). Then unlocks the views, sets the next
 * ambient sound 15 seconds off, clips to the game's area and opens the
 * scene.
 */
/* @zoombi32 0x0043ac20 */
void enterNextScene()
{
    short viaMap = 0;

    if (pendingScene == -1)
        return;
    if (!practiceLevel) {
        short level = sceneLevel();

        if (g_4b7558 && level)
            level--;
        level &= 3;
        short bit = 1 << level;

        switch (currentScene) {
        case 9:
            if (pendingScene == 4) {
                puzzleLeft = 9;
                gameState[0x50] |= bit;
            }
            break;
        case 12:
            if (pendingScene == 5) {
                puzzleLeft = 12;
                *(short *)(gameState + 0x52) |= (char)bit;
            }
            break;
        case 15:
            if (pendingScene == 5) {
                puzzleLeft = 15;
                *(short *)(gameState + 0x52) |= bit << 4;
            }
            break;
        case 18:
            if (pendingScene == 6) {
                puzzleLeft = 18;
                gameState[0x51] |= bit;
            }
            break;
        }
    }
    if (practiceLevel) {
        viaMap = 0;
        if (currentScene != 1 && pendingScene != 3 && pendingScene != 0)
            pendingScene = 1;
    } else if (currentScene != 1 && currentScene != 2 && currentScene != 6 && pendingScene != 1) {
        switch (currentScene) {
        case 0:
            viaMap = 0;
            break;
        case 3:
            viaMap = 1;
            break;
        case 7:
        case 8:
        case 9:
            viaMap = 1;
            break;
        case 4:
            viaMap = 1;
            break;
        case 10:
        case 11:
        case 12:
            viaMap = 1;
            break;
        case 13:
        case 14:
        case 15:
            viaMap = 1;
            break;
        case 5:
            viaMap = 1;
            break;
        case 16:
        case 17:
        case 18:
            viaMap = 1;
            break;
        }
    }
    *(short *)(gameState + 0xca) = currentScene;
    if (pendingScene != 0 && pendingScene != 2)
        savedScene() = pendingScene;
    if (g_4b7562 || transitionsOn)
        viaMap = 0;
    if (journeyRoute)
        viaMap = 1;
    if (viaMap) {
        journeyFrom = currentScene;
        journeyTo = pendingScene;
        currentScene = 2;
        pendingScene = -1;
    } else {
        journeyFrom = currentScene;
        currentScene = pendingScene;
        pendingScene = -1;
        journeyTo = -1;
    }
    if (practiceLevel) {
        *(short *)(gameState + 0x54) = 0;
    } else {
        if (!viewsLocked)
            rosterChanged = 1;
        switch (currentScene) {
        case 7:
        case 10:
        case 13:
        case 16:
            *(short *)(gameState + 0x54) = 1;
            break;
        }
    }
    viewsLocked = 0;
    viewsPaused = fillViews = 0;
    ambientSoundTime = clockTime() + 900;
    setClipRect(gameRect);
    if (scenes[currentScene]->open)
        scenes[currentScene]->open();
}

/*
 * The marker's placing: turns each of its cels' images (1-184) into the
 * ones for the codes chosen (g_4b143e, g_4b1442 and g_4b1446, and their
 * second parts g_4b1440, g_4b1444 and g_4b1448; -1: none), by two tables
 * of five; with g_4b1450 set, also moves the cels to the marker's place
 * (g_4b1452, g_4b1454). A cel below 1 stops it there for good.
 */
/* @zoombi32 0x0043d70d */
void markerPlaced(View *view)
{
    volatile short row2;
    volatile short col1b;
    volatile short row2b;
    volatile short row0b;
    short columns[5] = {2, 3, 0, 1, 4};
    short rows[5] = {4, 0, 2, 1, 3};
    short col1;
    short row0;
    short *cels;
    short i;

    col1 = row2 = row0 = -1;
    col1b = row2b = row0b = -1;
    if (g_4b1442 != -1)
        col1 = columns[g_4b1442];
    if (g_4b1446 != -1)
        row2 = rows[g_4b1446];
    if (g_4b143e != -1)
        row0 = rows[g_4b143e];
    if (g_4b1444 != -1)
        col1b = columns[g_4b1444];
    if (g_4b1448 != -1)
        row2b = rows[g_4b1448];
    if (g_4b1440 != -1)
        row0b = rows[g_4b1440];
    cels = (short *)&view->body;
    i = 0;
    while (cels[i]) {
        if (cels[i] < 1)
            continue;
        if (cels[i] < 185) {
            if (cels[i] < 6 && col1b >= 0 && row0b != -1)
                cels[i] = row0b * 12 + col1b + 6;
            else if (cels[i] < 6 && col1b != -1 && row0b == -1)
                cels[i] = col1b + 1;
            else if (cels[i] >= 6 && cels[i] < 11 && col1 >= 0 && row0 != -1)
                cels[i] += row0 * 12 + col1;
            else if (cels[i] >= 6 && cels[i] < 11 && col1 != -1)
                cels[i] = col1 + 1;
            else if (cels[i] >= 11 && cels[i] < 18 && row0 != -1)
                cels[i] += row0 * 12;
            else if (cels[i] >= 66 && cels[i] < 88 && row2 >= 0)
                cels[i] += row2 * 22;
            else if (cels[i] && cels[i] >= 176 && row2b != -1)
                cels[i] = row2b * 22 + 66;
            if (g_4b1450) {
                if (!i) {
                    cels[i + 1] = g_4b1452;
                    cels[i + 2] = g_4b1454;
                } else if (!g_4b1456) {
                    if (netLevel < 2)
                        cels[i + 1] = g_4b1452 + 21;
                    else
                        cels[i + 1] = g_4b1452 + 3;
                    cels[i + 2] = g_4b1454 + 7;
                } else {
                    cels[i + 1] = g_4b1452 + 4;
                    cels[i + 2] = g_4b1454 + 3;
                }
            }
        }
        i += 3;
    }
}

/*
 * The notify of the Zoombinis crossing (g_4b0e6a the one moving): 0 flips
 * which way it faces and turns it the way g_4b11a2 says (then clears it); 2 runs
 * flyMarker; 4 starts the step onto the net (script 14000 on, by the place
 * g_4b119a) and puts the marker and the three places' views in order;
 * 20 takes the next Zoombini from place g_4b119a (a walk by its feet,
 * 13016 on); 30 starts its crossing (13031 on) and stacks it on the one
 * before (g_4b147e); 240-243 note a turn to make (g_4b11a2), 250-253 turn
 * it. When a script ends: after a crossing, it walks on to the next spot
 * of g_4a2dd6, with sounds when the last one's across; else the Zoombini
 * waiting on the net in the view's place stops, and the net's view starts
 * (10018) once they're all gone.
 */
/* @zoombi32 0x0043de4d */
void crossingNotify(View *view, short event)
{
    Point anchor;
    Point onto[3] = {{203, 42}, {242, 35}, {283, 28}};
    Point across[3] = {{220, 41}, {259, 34}, {300, 27}};
    Snoid *snoid = (Snoid *)&view->body;
    short i;
    short script;

    switch (event) {
    case 250:
    case 251:
    case 252:
    case 253:
        setSnoidFacing(snoid, event - 250);
        break;
    case 240:
    case 241:
    case 242:
    case 243:
        g_4b11a2 = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b11a2) {
            setSnoidFacing(snoid, g_4b11a2 - 1);
            g_4b11a2 = 0;
        }
        break;
    case 2:
        flyMarker(g_4b119e);
        break;
    case 4:
        view = findView(g_4b0e6a);
        anchor = onto[g_4b119a];
        startSnoidScript((Snoid *)&view->body, g_4b119a + 14000, &anchor, 0);
        view->notifyEnd = 0;
        view->notify = crossingNotify;
        moveView(g_4b12ba[0], 1, g_4b13cc[g_4b13ca]);
        moveView(g_4b12ba[1], 1, g_4b12ba[0]);
        moveView(g_4b12ba[2], 1, g_4b12ba[1]);
        moveView(g_4b0e6a, 1, g_4b12ba[2]);
        break;
    case 20:
        g_4b0e6a = g_4b1438[g_4b119a];
        view = findView(g_4b0e6a);
        script = ((Snoid *)&view->body)->features[3] - 1;
        script = 2 - g_4b119a + script * 3 + 13016;
        startSnoidScript((Snoid *)&view->body, script, 0, 0);
        ((Snoid *)&view->body)->unknownF7 = 1;
        view->notifyEnd = 0;
        view->notify = crossingNotify;
        g_4b0d5c = g_4b0e68;
        g_4b0e6a = g_4b1438[g_4b119a];
        g_4b1438[g_4b119a] = 0;
        if (!g_4b11a0 && g_4b0e68 < netPartySize)
            g_4b145c++;
        break;
    case 30:
        anchor = across[g_4b119a];
        view = findView(g_4b0e6a);
        script = ((Snoid *)&view->body)->features[3] - 1;
        script = script * 3 + g_4b119a + 13031;
        startSnoidScript((Snoid *)&view->body, script, &anchor, 0);
        view->notifyEnd = 1;
        view->notify = crossingNotify;
        g_4b1414++;
        if (g_4b147e)
            moveView(g_4b0e6a, 0, g_4b147e);
        g_4b147e = g_4b0e6a;
        break;
    case -1:
        if (!g_4b1414) {
            for (i = 2; i >= 0; i--)
                if (g_4b12b0[i] == view->id) {
                    View *waiting = findView(g_4b12b0[i]);

                    g_4b12b0[i] = 0;
                    if (waiting) {
                        setSnoidAction((Snoid *)&waiting->body, 2, 0);
                        if (!g_4b12b0[0] && !g_4b12b0[1] && !g_4b12b0[2]) {
                            startView(g_4b13fe, 10018, 0, 0);
                            g_4b1466 = g_4b144c = 0;
                        }
                        return;
                    }
                }
            if (!g_4b145c && !g_4b1438[0])
                g_4b1466 = g_4b144c = 0;
        } else {
            anchor = g_4a2dd6[g_4b11a8];
            g_4b11a8++;
            view = findView(g_4b0e6a);
            setSnoidAction((Snoid *)&view->body, 7, 0);
            *(Point *)&((Snoid *)&view->body)->targetX = anchor;
            ((Snoid *)&view->body)->unknownF7 = 1;
            view->notifyEnd = 0;
            view->notify = crossingNotify;
            g_4b1412 = g_4b0e6a;
            g_4b12aa = 1;
            g_4b1414 = 0;
            if (g_4b0e6c >= netPartySize || g_4b11a0)
                g_4b1466 = g_4b144c = 0;
            if (!g_4b145c && !g_4b1438[0] && !g_4b1438[1] && !g_4b1438[2]) {
                if (g_4b0e6c >= netPartySize)
                    queueViewSound(randomBetween(20055, 20063), 0);
                g_4b11a6++;
                g_4b147c++;
                g_4b1478 = netPartySize;
            }
            if (g_4a28d0) {
                g_4a28d0 = 0;
                if (g_4b0e6c >= 1 && g_4b0e6c < netPartySize)
                    queueViewSound(randomBetween(20045, 20048), 0);
            }
        }
        break;
    }
}

/*
 * Sets up the codes: picks for each of five rows a column value and a row
 * value (and from level 2 a third) that no row has yet (levels 0-1: only
 * the first two must be new, tested together), and fills the tables with
 * them: codeColumns and codeRows (5 by 5), or from level 2 codeColumns, codeRows
 * and codeLayers (5 by 5 by 5). At levels 1 and 3 the rows are then rotated
 * by 2-3 places each (level 3's other way, rotating codeLayers, is never
 * taken). Then places the party's Zoombinis (netGroups) at random free
 * entries of g_4b11aa, adds a view for each (script 9000 on, or 9025 on),
 * and picks the order of the codes (g_4b1178-g_4b117c, distinct from
 * level 3) and, from level 3, g_4b1468.
 */
/* @zoombi32 0x0043c9e2 */
void setUpCodes()
{
    volatile short m;
    volatile short ok;
    volatile short scriptBase;
    short columns[5];
    short rows[5];
    short thirds[5];
    short y;
    short shifts;
    short saved;
    short i;
    short j;
    short k;
    short x;
    short z;
    short mode;
    short distinct;
    short image;

    fillMemory(g_4b11aa, 0, 250);
    for (i = 0; i < 5; i++) {
        columns[i] = 0;
        rows[i] = 0;
        thirds[i] = 0;
    }
    for (i = 0; i < 5; i++) {
        ok = 0;
        do {
            x = randomUpTo(4);
            y = randomUpTo(4);
            z = randomUpTo(4);
            if (netLevel < 2) {
                if (!columns[x] & !rows[y])
                    ok = 1;
            } else if (!columns[x] && !rows[y] && !thirds[z]) {
                ok = 1;
            }
            if (ok) {
                columns[x]++;
                rows[y]++;
                thirds[z]++;
            }
        } while (!ok);
        switch (netLevel) {
        case 0:
        case 1:
            for (j = 0; j < 5; j++) {
                codeColumns[i * 5 + j] = x;
                codeRows[j * 5 + i] = y;
            }
            break;
        case 2:
        case 3:
            for (j = 0; j < 25; j++)
                codeColumns[i * 25 + j] = x;
            for (j = 0; j < 5; j++)
                for (k = 0; k < 5; k++)
                    codeRows[i * 5 + j * 25 + k] = y;
            for (j = 0; j < 5; j++)
                for (k = 0; k < 5; k++)
                    codeLayers[j * 5 + k * 25 + i] = y;
            break;
        }
    }
    switch (netLevel) {
    case 1:
        shifts = randomUpTo(1) + 2;
        for (j = 0; j < 5; j++)
            g_4b1166[j] = codeRows[j];
        for (k = 1; k < 5; k++) {
            for (i = 0; i < shifts; i++) {
                saved = g_4b1166[4];
                for (j = 4; j > 0; j--)
                    g_4b1166[j] = g_4b1166[j - 1];
                g_4b1166[0] = saved;
            }
            for (j = 0; j < 5; j++)
                codeRows[k * 5 + j] = g_4b1166[j];
        }
        break;
    case 3:
        shifts = randomUpTo(1) + 2;
        mode = randomUpTo(1);
        mode = 0;
        if (!mode) {
            for (i = 0; i < 5; i++)
                for (k = 1; k < 5; k++) {
                    for (j = 0; j < 5; j++)
                        g_4b1166[j] = codeColumns[k + i * 5 + j * 25 - 1];
                    for (m = 0; m < shifts; m++) {
                        saved = g_4b1166[4];
                        for (j = 4; j > 0; j--)
                            g_4b1166[j] = g_4b1166[j - 1];
                        g_4b1166[0] = saved;
                    }
                    for (j = 0; j < 5; j++)
                        codeColumns[i * 5 + k + j * 25] = g_4b1166[j];
                }
        } else {
            for (k = 5; k < g_4b142e; k += 5) {
                for (j = 0; j < 5; j++)
                    g_4b1166[j] = codeLayers[k + j - 5];
                for (i = 0; i < shifts; i++) {
                    saved = g_4b1166[4];
                    for (j = 4; j > 0; j--)
                        g_4b1166[j] = g_4b1166[j - 1];
                    g_4b1166[0] = saved;
                }
                for (j = 0; j < 5; j++)
                    codeLayers[k + j] = g_4b1166[j];
            }
        }
        break;
    }
    scriptBase = 0;
    if (netLevel > 1)
        scriptBase = 25;
    for (i = 0; i < g_4b0e76; i++) {
        do {
            if (netLevel < 2)
                k = randomUpTo(24);
            else
                k = randomUpTo(124);
        } while (g_4b11aa[k]);
        g_4b11aa[k] = netGroups[i];
    }
    for (i = 0; i < g_4b142e; i++) {
        g_4b12cc[i] = 0;
        if (g_4b11aa[i]) {
            g_4b12cc[i] = addView(0x4188000, drawCels, runViewScript, scriptBase + i + 9000, 6, 0, 0, 0);
            image = g_4b11aa[i] + 150;
            if (netLevel > 1)
                image += 3;
            View *view = findView(g_4b12cc[i]);

            if (view) {
                view->placed = placeSecondCel;
                short *parts = (short *)&view->body;

                parts[20] = image;
            }
            moveView(g_4b12cc[i], 0, g_4b12ba[0]);
        }
    }
    if (netLevel <= 2) {
        if (randomUpTo(1)) {
            g_4b1178 = 2;
            g_4b117a = 1;
            g_4b117c = 0;
        } else {
            g_4b1178 = 1;
            g_4b117a = 2;
            g_4b117c = 0;
        }
    } else {
        distinct = 0;
        do {
            g_4b1178 = randomUpTo(2);
            g_4b117a = randomUpTo(2);
            g_4b117c = randomUpTo(2);
            if (g_4b1178 != g_4b117a && g_4b117a != g_4b117c && g_4b117c != g_4b1178)
                distinct++;
        } while (!distinct);
    }
    if (netLevel >= 3)
        g_4b1468 = randomUpTo(5);
}

/*
 * A code chosen: `which` (1-3) sets the first, second or third code to
 * `value` (keeping the previous ones in g_4b1440-g_4b1448) and shows it
 * (script 10002, 10007 or 10012 on), and once all the codes the level
 * needs are set, shows the marker (7026, 7027 or 7019) and the net's view
 * (10018). `which` 0 sends the marker off (10001) to the entry for the
 * codes (g_4b119e; script 7028 on by its column) with crossingNotify.
 */
/* @zoombi32 0x0043d0b4 */
void chooseCode(short which, short value)
{
    View *view;
    short ready = 0;
    short column;

    if ((netLevel <= 1 && g_4b1442 >= 0 && g_4b1446 >= 0) || (g_4b143e >= 0 && g_4b1442 >= 0 && g_4b1446 >= 0))
        ready = 1;
    switch (which) {
    case 1:
        if (netLevel >= 2) {
            g_4b1444 = g_4b1442;
            g_4b1440 = g_4b143e;
            g_4b143e = value;
            if (g_4b1440 != g_4b143e) {
                startView(g_4b1400, value + 10002, 0, 0);
                if (!g_4b142c && ready) {
                    view = startView(g_4b12b6, 7027, 0, 0);
                    if (view) {
                        g_4b1450 = 0;
                        view->placed = markerPlaced;
                    }
                    if (!g_4b1466)
                        startView(g_4b13fe, 10018, 0, 0);
                }
            }
        }
        break;
    case 2:
        g_4b1444 = g_4b1442;
        g_4b1442 = value;
        g_4b1440 = g_4b143e;
        if (g_4b1444 != g_4b1442) {
            startView(g_4b1402, value + 10007, 0, 0);
            if (!g_4b142c && ready) {
                view = findView(g_4b12b6);
                if (view) {
                    setViewScript(view, 7019, 1);
                    view->interval = 3;
                } else {
                    g_4b12b6 = addView(0x4108000, drawCels, runViewScript, 7019, 3, 0, 0, 0);
                    view = findView(g_4b12b6);
                }
                if (view) {
                    g_4b1450 = 0;
                    view->placed = markerPlaced;
                }
                if (!g_4b1466)
                    startView(g_4b13fe, 10018, 0, 0);
            }
        }
        break;
    case 3:
        g_4b1448 = g_4b1446;
        g_4b1444 = g_4b1442;
        g_4b1440 = g_4b143e;
        g_4b1446 = value;
        if (g_4b1448 != g_4b1446) {
            startView(g_4b1404, value + 10012, 0, 0);
            if (!g_4b142c && ready) {
                view = startView(g_4b12b6, 7026, 0, 0);
                if (view) {
                    g_4b1450 = 0;
                    view->placed = markerPlaced;
                }
                if (!g_4b1466)
                    startView(g_4b13fe, 10018, 0, 0);
            }
        }
        break;
    case 0:
        g_4b1448 = g_4b1446;
        g_4b1444 = g_4b1442;
        g_4b1440 = g_4b143e;
        if ((netLevel <= 1 && g_4b1442 >= 0 && g_4b1446 >= 0)
            || (g_4b143e >= 0 && g_4b1442 >= 0 && g_4b1446 >= 0)) {
            startView(g_4b13fe, 10001, 0, 0);
            g_4b1416 = 0;
            g_4b119e = findCodeEntry();
            if (netLevel < 2)
                column = g_4b119e % 5;
            else
                column = g_4b119e % 25 / 5;
            if (!column)
                g_4a2e58 = 1;
            else if (column >= 1 && column < 4)
                g_4a2e58 = 0;
            else
                g_4a2e58 = 2;
            view = startView(g_4b12b6, g_4a2e58 + 7028, crossingNotify, 0);
            if (view) {
                g_4b1450 = 0;
                view->placed = markerPlaced;
                view->interval = 6;
                g_4b1464 = groupViews(g_4b12b6, g_4b12b6, 0, 0, 0, 0);
            }
        }
        break;
    }
    if (!g_4b1416 && !g_4b142c) {
        g_4b1410++;
        g_4b1416++;
    }
}

/*
 * The net's frame (scene 15): leaves once asked to (and sound 996 is
 * done), then steps the scene's sequences on as each group of views
 * finishes (groupLeader): the net's views appearing (7001 on), the codes
 * being set up, the marker's moves and the Zoombinis crossing; starts
 * waiting Zoombinis fidgeting (13046 on) now and then, and repeats the
 * net's prompt (10018) after 12 seconds, or 2 minutes when idle.
 */
/* @zoombi32 0x0043b86d */
void netFrame()
{
    short done;
    View *view;
    short column;
    short i;
    short tries;
    short script;
    long fidget;

    if (g_4a2ea8 || !g_4b12a8)
        return;
    g_4a2ea8 = g_4b1480 = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            g_4a2ea8 = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeNet();
                g_4a2ea8 = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (g_4b1406 && g_4b142a && !groupLeader[g_4b142a]) {
        if (++g_4b1408 < g_4b140a) {
            startView(g_4b13c6, g_4b1408 + 7000, 0, 0);
            g_4b142a = groupViews(g_4b13c6, g_4b13c6, 0, 0, 0, 0);
        } else {
            g_4b1406 = g_4b142a = 0;
            g_4b145c = 3;
            g_4b144a++;
            sendNextToNet();
            g_4b1410 = 1;
            g_4b1416 = 1;
        }
    }
    if (g_4b141e && !groupLeader[g_4b141e]) {
        g_4b141e = 0;
        setUpCodes();
    }
    if (g_4b1464) {
        if (!groupLeader[g_4b1464]) {
            g_4b1464 = 0;
            if (--g_4b140a < 0) {
                g_4b142c++;
                if (randomBetween(0, 4) > netLevel || (*(short *)(gameState + 0x3c) & 0xfff) <= 3) {
                    if (g_4b145a < 1) {
                        if (g_4b0e6c >= 1 && g_4b0e6c < netPartySize)
                            queueViewSound(randomBetween(20045, 20048), 0);
                    } else if (g_4b0e68 < netPartySize) {
                        g_4a28d0++;
                    }
                }
            }
            if (g_4b145a >= 1)
                g_4b1458++;
            else
                g_4b145a = 0;
        }
    } else if (g_4b1410 && !g_4b142c) {
        if ((netLevel <= 1 && g_4b1442 >= 0 && g_4b1446 >= 0) || (g_4b143e >= 0 && g_4b1442 >= 0 && g_4b1446 >= 0)) {
            g_4b1410 = 0;
            startView(g_4b13c6, g_4b140c + 7031, 0, 0);
            g_4b141a = groupViews(g_4b13c6, g_4b13c6, 0, 0, 0, 0);
            if (++g_4b140c > 16)
                g_4b140c = 16;
        }
    } else if (g_4b141a) {
        g_4b141a = 0;
        g_4b1444 = g_4b1442;
        g_4b1448 = -1;
        g_4b1440 = -1;
        if (netLevel <= 1)
            g_4b143e = -1;
        if (!g_4b142c) {
            view = startView(g_4b12b6, 7018, 0, 0);
            if (view) {
                g_4b1450 = 0;
                view->interval = 3;
                view->placed = markerPlaced;
                g_4b141c = groupViews(g_4b12b6, g_4b12b6, 0, 0, 0, 0);
            }
        }
    } else if (g_4b141c) {
        if (!groupLeader[g_4b141c]) {
            g_4b141c = 0;
            sendNextToNet();
            view = startView(g_4b12b6, 7025, 0, 0);
            if (view) {
                view->interval = 2;
                g_4b1420 = groupViews(g_4b12b6, g_4b12b6, 0, 0, 0, 0);
            }
        }
    } else if (g_4b1420) {
        if (!groupLeader[g_4b1420]) {
            g_4b1420 = 0;
            view = startView(g_4b12b6, 7026, 0, 0);
            if (view) {
                g_4b1450 = 0;
                view->placed = markerPlaced;
                view->interval = 6;
                g_4b1422 = groupViews(g_4b12b6, g_4b12b6, 0, 0, 0, 0);
            }
        }
    } else if (g_4b1422) {
        if (!groupLeader[g_4b1422]) {
            g_4b1422 = g_4b11a4 = 0;
            if (netLevel <= 1) {
                g_4b1424 = 0;
                if (g_4b1418) {
                    startView(g_4b13fe, 10017, 0, 0);
                    g_4b1418 = 0;
                    g_4b1470 = clockTime();
                } else if (g_4b144e || g_4b0e6c >= netPartySize) {
                    g_4b144c = g_4b144e = 0;
                    startView(g_4b13fe, 10018, 0, 0);
                } else {
                    g_4b144c = g_4b144e = 0;
                }
            } else {
                g_4b1440 = -1;
                view = startView(g_4b12b6, 7027, 0, 0);
                if (view) {
                    g_4b1424 = groupViews(g_4b12b6, g_4b12b6, 0, 0, 0, 0);
                    g_4b1450 = 0;
                    view->placed = markerPlaced;
                }
            }
        }
    } else if (g_4b1424) {
        if (!groupLeader[g_4b1424]) {
            g_4b1424 = 0;
            if (g_4b1418) {
                startView(g_4b13fe, 10017, 0, 0);
                g_4b1418 = 0;
                g_4b1470 = clockTime();
            } else if (g_4b144e || g_4b0e6c >= netPartySize) {
                g_4b144c = g_4b144e = 0;
                startView(g_4b13fe, 10018, 0, 0);
            } else {
                g_4b144c = g_4b144e = 0;
            }
        }
    }
    if (g_4b1458 && g_4b145a) {
        g_4b1458 = 0;
        if (g_4b0e68 >= netPartySize)
            g_4b145c = 0;
        if (--g_4b145a <= 0) {
            g_4b145a = 0;
            if (g_4b0d5c < 0 && g_4b145c)
                sendNextToNet();
        }
        if (netLevel < 2)
            column = g_4b119e % 5;
        else
            column = g_4b119e % 25 / 5;
        startView(g_4b12ba[column], column + 8000, 0, 0);
        if (g_4b13ca)
            moveView(g_4b12ba[column], 1, g_4b13cc[g_4b13ca]);
        g_4b145e = groupViews(g_4b12ba[column], g_4b12ba[column], 0, 0, 0, 0);
        g_4b1466++;
        g_4b1470 = clockTime();
    } else if (g_4b145e) {
        if (!groupLeader[g_4b145e]) {
            g_4b145e = 0;
            g_4b119a = 0;
            for (i = 0; i < 3; i++)
                if (g_4b1438[i]) {
                    g_4b119a = i;
                    break;
                }
            if (g_4b1438[g_4b119a]) {
                view = startView(g_4b12b8, g_4b119a + 8005, crossingNotify, 0);
                if (view)
                    g_4b1460 = groupViews(g_4b12b8, g_4b12b8, 0, 0, 0, 0);
            }
        }
    } else if (g_4b1460) {
        if (!groupLeader[g_4b1460]) {
            g_4b1460 = 0;
            if (g_4b145a)
                g_4b1458++;
            else
                g_4b144a++;
            if (g_4b0e6c >= netPartySize && !g_4b145a)
                g_4b145a = g_4b1458 = 0;
        }
    } else if (g_4b0d5c >= 0 && !g_4b142c && g_4b0d5c < netPartySize) {
        view = idleSnoidView(partyViews[g_4b0d5c]);
        if (view) {
            script = ((Snoid *)&view->body)->features[3] - 1;
            script = script + (2 - g_4b119a) * 5 + 13001;
            startSnoidScript((Snoid *)&view->body, script, 0, 0);
            view->notify = crossingNotify;
            view->notifyEnd = 1;
            g_4b1430++;
            g_4b12b0[g_4b119a] = g_4b1438[g_4b119a];
            g_4b0d5c = -1;
            g_4b1462 = 0;
        }
    }
    if (g_4b0d5c < 0 && g_4b145c)
        sendNextToNet();
    if (g_4b147c && g_4b147a < g_4b1478) {
        if (clockTime() - g_4b146c > 30) {
            done = 0;
            tries = 0;
            g_4b146c = clockTime();
            do {
                i = allocateSlot(&g_4b1474, netPartySize, 0);
                if (partyViews[i] != g_4b1438[0] && partyViews[i] != g_4b1438[1] && partyViews[i] != g_4b1438[2]) {
                    g_4b0d60 = idleSnoidView(partyViews[i]);
                    if (g_4b0d60 && g_4b0d60->body.running && g_4b0d60->flags == 1) {
                        if (!((Snoid *)&g_4b0d60->body)->unknownF7 || g_4b11a6) {
                            fidget = ((Snoid *)&g_4b0d60->body)->features[3] - 1;
                            fidget += 13046;
                            startSnoidScript((Snoid *)&g_4b0d60->body, fidget, 0, 0);
                            g_4b147a++;
                            done = 1;
                        }
                    } else if (++tries > 20) {
                        done = 1;
                    }
                } else if (++tries > 20) {
                    done = 1;
                }
            } while (!done);
        }
    } else if (g_4b147a >= g_4b1478) {
        g_4b147a = g_4b147c = g_4b146c = g_4b1474 = 0;
    }
    if (g_4b1466) {
        if (clockTime() - g_4b1470 > 720) {
            g_4b1466 = g_4b144c = 0;
            startView(g_4b13fe, 10018, 0, 0);
            g_4b1470 = clockTime();
        }
    } else if (clockTime() - g_4b1470 > 7200) {
        g_4b1466 = g_4b144c = g_4b1470 = 0;
        startView(g_4b13fe, 10018, 0, 0);
        g_4b1470 = clockTime();
    }
    playAmbientSound();
    g_4a2ea8 = 0;
}

/*
 * Scene 15's buttons: leaves at once if asked to; unless g_4b11a4, 3 sends
 * the marker off with the codes chosen, and 4-8, 9-13 and 14-18 choose the
 * first (from level 2), second and third code. 1 asks to leave for the map
 * (999, keeping the party); 2, once allowed (g_4b12aa), sends the
 * Zoombinis off (996) and asks to leave for scene 5.
 */
/* @zoombi32 0x0043c48b */
void netClicked(short button)
{
    short code;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeNet();
        return;
    }
    if (!g_4b11a4) {
        switch (button) {
        case 3:
            if (!g_4b1466 && !g_4b144c && !g_4b142c) {
                g_4b1466++;
                g_4b144c++;
                g_4b1470 = clockTime();
                code = 0;
                chooseCode(0, code);
            }
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            if (netLevel > 1 && (!g_4b144c || g_4b142c)) {
                code = button - 4;
                chooseCode(1, code);
            }
            break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            if (!g_4b144c || g_4b142c) {
                code = button - 9;
                chooseCode(2, code);
            }
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            if (!g_4b144c || g_4b142c) {
                code = button - 14;
                chooseCode(3, code);
            }
            break;
        }
    }
    switch (button) {
    case 1:
        queueViewSound(999, 0);
        drawNetButton(button, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawNetButton(button, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (g_4b12aa) {
            queueViewSound(0, 0);
            drawNetButton(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawNetButton(button, 0, 1);
            queueViewSound(996, 0);
            sendSnoids(600, -100, 45);
            sceneDue = 5;
        }
        break;
    }
}
