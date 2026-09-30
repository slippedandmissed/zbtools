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
 * partHotX/partHotY: its body (from word 40), with the ways open (words
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
    switch (snoid->action) {
    case 1:
        return;
    case 2:
        break;
    case 3:
        snoid->action = 1;
        break;
    default:
        snoid->action = 1;
        break;
    }
    where = *(Point *)&snoid->body.x;
    switch (cel[30]) {
    case 2:
    case 3:
    case 4:
        part = parts[40];
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        for (k = 1; k < 5; k++)
            if (parts[33 + k]) {
                part = parts[40] + k;
                *cel++ = part;
                *cel++ = where.x - partHotX[part];
                *cel++ = where.y - partHotY[part];
            }
        part = parts[40] + parts[38] + 5;
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        if (parts[41]) {
            part = parts[41] * 5 + parts[42] + 5;
            *cel++ = part;
            *cel++ = where.x - partHotX[part];
            *cel++ = where.y - partHotY[part];
        }
        break;
    case 1:
        parts[38]++;
        if (parts[38] > 3)
            parts[38] = 0;
        part = parts[38] + 2;
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        break;
    case 5:
        parts[38]++;
        if (parts[38] > 5)
            parts[38] = 0;
        part = parts[40] + parts[38] + 9;
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        break;
    case 6:
        part = parts[40] + 19;
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        if (snoid->action == 2) {
            if (parts[46] >= 0 && parts[46] < 4) {
                part = parts[40] + parts[46] + 15;
                *cel++ = part;
                *cel++ = where.x - partHotX[part];
                *cel++ = where.y - partHotY[part];
                parts[46]++;
            } else {
                snoid->action = 1;
            }
        }
        break;
    case 7:
        part = parts[40] + 20;
        *cel++ = part;
        *cel++ = where.x - partHotX[part];
        *cel++ = where.y - partHotY[part];
        if (snoid->action == 2) {
            if (parts[46] >= 0 && parts[46] < 4) {
                part = parts[40] + parts[46] + 15;
                *cel++ = part;
                *cel++ = where.x - partHotX[part];
                *cel++ = where.y - partHotY[part];
                parts[46]++;
            } else {
                snoid->action = 1;
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
 * squareOffsets) until its script ends (startPairNotify); clears a's square in
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
        offset = (squareOffsets + partsA[33] * 12)[partsA[34]];
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
        offset = (squareOffsets + partsB[33] * 12)[partsB[34]];
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
    moveView(b->id, 0, lineAnchorViews[partsB[34]]);
    moveView(partsB[41], 1, b->id);
    moveView(a->id, 0, lineAnchorViews[partsA[34]]);
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
    if (netOpen) {
        netOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&netButtonResource);
        setFreeAtOnce(saved);
        closeGameFile(&netFile);
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
    codeEntryCount = 25;
    if (netLevel > 1)
        codeEntryCount = 125;
    markerStep1Group = markerStep2Group = markerStep3Group = markerStep4Group = 0;
    markerStep5Group = unusedNet1 = unusedNet2 = guideGroup = 0;
    codesDue = codesShown = netTriesOver = emptyNetPlaces = 0;
    revealGroup = revealStep = markerMoved = unusedNet3 = markerStep = 0;
    promptHeld = unusedNet4 = crossingStarted = lastAcross = 0;
    promptView = code1View = code2View = code3View = 0;
    markerCount = sentIndex = markerPlace = -1;
    codesLocked = firstPrompt = 1;
    previousCode1 = previousCode2 = previousCode3 = -1;
    promptHeld2 = markerMissed = allAcross = 0;
    netPlaces[0] = netPlaces[1] = netPlaces[2] = sendAllowed = 0;
    waitingOnNet[0] = waitingOnNet[1] = waitingOnNet[2] = 0;
    sentCount = sendUnderway = acrossCount = lastCrossed = 0;
    unusedNet5 = unusedNet6 = unusedNet7 = 0;
    currentNetPlace = unusedNet8 = movingSnoid = snoidsFound = 0;
    standingGroup = stepGroup = crossDue = groupToCross = 0;
    markerGroup = sendsLeft = 0;
    revealing = 1;
    sceneDue = netFidgetsOn = 0;
    netOpen = netGoAllowed = 0;
    chosenCode1 = randomUpTo(4);
    chosenCode2 = randomUpTo(4);
    chosenCode3 = randomUpTo(4);
    fillMemory(markerViews, 0, 50);
    unloadSounds();
    openGameFile(&netFile, "Net.MHK");
    setCurrentMap(netFile);
    drawBackdrop((netLevel >= 2) + 5000);
    loadFeatureGroup(7000, 0, 1);
    loadFeatureGroup(8000, 1, 0);
    loadFeatureGroup(9000, 2, 1);
    loadFeatureGroup(10000, 3, 0);
    loadScripts(7000, 48);
    addScripts(8000, 8, 0);
    addScripts(9000, 154, 0);
    addScripts(10000, 19, 0);
    netButtonImages = loadImageBank(6000, &netButtonResource);
    loadSnoidScripts(14000, 3, 0);
    addSnoidScripts(13000, 51, 0);
    addView(0x1000, drawNetButtons, updateNetButtons, 0, 0, 0, 0, 0);
    setViewPlaces(16, places, 1);
    copyPaletteRange(10, 236);
    makePartySnoids(0);
    enterSnoids(0);
    netChosen = listChosenSnoids();
    netPartySize = netChosen->count;
    netFidgetsAllowed = 3;
    if (*(short *)(gameState + 0x20))
        netFidgetsAllowed = 2;
    netFidgets = 0;
    nextToSend = 0;
    splitIntoGroups();
    revealSteps = (netLevel == 3) + netGroupCount + 7;
    revealStepsAtOpen = revealSteps;
    revealScript = 16 - revealSteps;
    addNetViews();
    updateViews();
    staggerSnoids(30, 0);
    chooseSnoids(0, 0);
    setGroupLists(netGroupList, 1, (short)0xc000);
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
    netOpen = 1;
    setViewsLocked(0);
    campHint((short *)(gameState + 0x3c));
    hintSound = 20064;
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   netGoAllowed), lit or not, and with `show` shows it. */
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
        if (!netGoAllowed) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(netButtonImages->offsets[image] + (char *)netButtonImages), netButtons[which].rect.left,
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

/* A view's update: adds buttons 3 (when netGoAllowed changes) and 2 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0043b7ae */
void updateNetButtons(View *, short region)
{
    if (netGoAllowed) {
        if (!netButton2Lit) {
            netButton2Lit = 1;
            unionRgnRect(region, &netButtons[3].rect);
        }
    } else if (netButton2Lit) {
        netButton2Lit = 0;
        unionRgnRect(region, &netButtons[3].rect);
    }
    if (!netButton1Drawn) {
        netButton1Drawn = 1;
        unionRgnRect(region, &netButtons[2].rect);
    }
}

/* Splits the netPartySize Zoombinis into groups (netGroups, netGroupCount of them)
   of three, two and one in turn, then evens out the overshoot by taking
   one from groups of two or more; if that can't be done, one each. */
/* Not exact: the original caches netGroupCount's address in edi, and negates
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
    netGroupCount = n;
    if (left) {
        left = -left;
        do {
            for (i = 0; i < netGroupCount; i++)
                if (netGroups[i] >= 2 && left) {
                    netGroups[i]--;
                    left--;
                }
            if (left) {
                n = 0;
                for (i = 0; i < netGroupCount; i++)
                    if (placeGroups[i] > 1)
                        n++;
                if (!n) {
                    netGroupCount = netPartySize;
                    for (i = 0; i < netGroupCount; i++)
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
 * gives up its partner (word 43, listed in partnerList), which takes the
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
        queueViewSound(turnSoundToggle + 5101, 0);
        turnSoundToggle++;
        if (turnSoundToggle > 1)
            turnSoundToggle = 0;
        its[38]++;
        if (its[38] > 3)
            its[38] = 0;
        while (!its[34 + its[38]]) {
            its[38]++;
            if (its[38] > 3)
                its[38] = 0;
        }
        ((Snoid *)&paired->body)->action = 3;
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
                partnerList[partnerCount] = its[43];
                partnerCount++;
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

/* Sends the next Zoombini of the party (nextToSend of netPartySize) to the
   first free place of three (netPlaces, from the last), while sendAllowed lets
   it (for sendsLeft more); counts in emptyNetPlaces the places left when none
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
        if (!netPlaces[i]) {
            if (nextToSend < netPartySize) {
                if (!sendAllowed)
                    return;
                if (!--sendsLeft)
                    sendAllowed = 0;
                view = findView(partyViews[nextToSend]);
                if (view) {
                    setSnoidAction((Snoid *)&view->body, 10, 0);
                    *(Point *)&((Snoid *)&view->body)->targetX = target;
                    sentIndex = nextToSend;
                    netPlaces[i] = partyViews[nextToSend];
                    nextToSend++;
                    currentNetPlace = i;
                    sendUnderway = 1;
                    return;
                }
            } else {
                emptyNetPlaces++;
            }
        }
    sendsLeft = 0;
    sendAllowed = 0;
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
   a partner (word 43) list it in partnerList. */
/* @zoombi32 0x00439e55 */
void stopMazeSnoid(short id)
{
    View *view = findView(id);
    short *list;
    short n;

    if (view) {
        ((Snoid *)&view->body)->action = 2;
        short *parts = (short *)&view->body;

        parts[46] = 0;
        switch (parts[33]) {
        case 1:
            list = lineViews;
            n = 0;
            break;
        case 2:
            list = lineList1;
            n = lineCount1;
            break;
        case 3:
            list = lineList2;
            n = lineCount2;
            break;
        case 4:
            list = lineList3;
            n = lineCount3;
            break;
        case 5:
            list = lineList4;
            n = lineCount4;
            break;
        case 6:
            list = lineList5;
            n = lineCount5;
            break;
        case 7:
            list = lineList6;
            n = lineCount6;
            break;
        case 8:
            list = lineList7;
            n = lineCount7;
            break;
        default:
            list = lineViews;
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
                    ((Snoid *)&view->body)->action = 3;
                }
                if (parts[30] == 5 && parts[43]) {
                    partnerList[partnerCount] = parts[43];
                    partnerCount++;
                    parts[43] = 0;
                }
            }
        }
    }
}

/* Sends a Zoombini's marker flying from its place (markerPlaces, or markerPlaces3d
   at the higher levels) to (484, 318) in six steps (updateFlyingMarker), unless one
   is flying already. */
/* @zoombi32 0x0043e435 */
void flyMarker(short n)
{
    short scripts[3] = {1, 0, 2};

    if (!markerStep) {
        markerPlace = n;
        markerStep++;
        if (netLevel <= 1) {
            markerX = markerPlaces[n].x;
            markerY = markerPlaces[n].y;
        } else {
            markerX = markerPlaces3d[n].x;
            markerY = markerPlaces3d[n].y;
        }
        markerDx = (484 - markerX) / 6;
        markerDy = (318 - markerY) / 6;
        markerX = 484;
        markerY = 318;
        markerCount++;
        markerViews[markerCount] = addView(0x4100000, drawCels, updateFlyingMarker, scripts[markerColumnKind] + 7020, 6, 0, 0, 0);
        View *view = findView(markerViews[markerCount]);

        if (view) {
            view->unknown1e = n;
            markerMoved++;
            view->placed = markerPlaced;
            view->interval = 3;
            moveView(markerViews[markerCount], 0, netMarkerView);
        }
    }
}

/* The flying marker's update: steps it on (markerX/4 by markerDx/80),
   and after five steps (or when the game says) lands it (landMarker). */
/* @zoombi32 0x0043e5a7 */
void updateFlyingMarker(View *view, short region)
{
    if (++markerStep > 5 || *(short *)(gameState + 0x20)) {
        markerStep = 0;
        view->update = runViewScript;
        landMarker(view->unknown1e);
    } else {
        markerX -= markerDx;
        markerY -= markerDy;
    }
    runViewCels(view, region);
    waitForEventFor(0, 2, 0, 1);
}

/* Lands the flying marker for place n: shows it (script 7023, or 7024 at
   the higher levels) there, and counts that place's group (placeGroups, by
   markerPlace) into snoidsFound; a place with none counts in markerMissed. */
/* @zoombi32 0x0043da30 */
void landMarker(short n)
{
    View *view;

    if (n >= 0) {
        if (netLevel <= 1) {
            markerX = markerPlaces[n].x;
            markerY = markerPlaces[n].y;
        } else {
            markerX = markerPlaces3d[n].x;
            markerY = markerPlaces3d[n].y;
        }
        markerMoved++;
        view = findView(markerViews[markerCount]);
        if (view) {
            if (netLevel < 2)
                setViewScript(view, 7023, 1);
            else
                setViewScript(view, 7024, 1);
        } else {
            if (netLevel < 2)
                markerViews[markerCount] = addView(0x4108000, drawCels, runViewScript, 7023, 6, 0, 0, 0);
            else
                markerViews[markerCount] = addView(0x4108000, drawCels, runViewScript, 7024, 6, 0, 0, 0);
            view = findView(markerViews[markerCount]);
        }
        if (view) {
            view->placed = markerPlaced;
            moveView(markerViews[markerCount], 0, standingViews[0]);
        }
        markerMissed = 0;
        if ((groupToCross = placeGroups[markerPlace]) < 1) {
            groupToCross = 0;
            promptHeld = 0;
            markerMissed++;
        } else {
            netFidgetsOn++;
        }
        snoidsFound += groupToCross;
        placeGroups[n] = -1;
    }
}

/* Steps a maze Zoombini one square on in its direction (word 20; within
   the 13 by 13 board); a square of kind 5 there hands its partner on (to
   partnerList, turned the same way). Then puts it there with its helper view
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
                partnerList[partnerCount] = its[43];
                partnerCount++;
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
                partnerList[partnerCount] = its[43];
                partnerCount++;
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
   codeOrder1 and codeOrder2) and a third (by codeOrder3 from level 2, else
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
    drawText(Rect(left), 0x22, names[codeOrder1], 0xffff);
    drawText(Rect(whole), 0x22, names[codeOrder2], 0xffff);
    if (netLevel >= 2)
        drawText(Rect(right), 0x22, names[codeOrder3], 0xffff);
    else
        drawText(Rect(right), 0x22, "PR", 0xffff);
    setForeColor(saved);
    showRect(&whole);
}

/* Adds the scene's standing views: five at 8000 on (standingViews), 8005, the
   guide (9151, or 9153 at the higher levels; grouped), 7018, 10018, the
   three code views (10002, 10007 and 10012 on, by chosenCode1/42/46; the
   first from level 2) and 7000 (grouped). */
/* @zoombi32 0x0043c6df */
void addNetViews()
{
    short i;

    for (i = 0; i < 5; i++)
        standingViews[i] = addView(0x4188000, drawCels, runViewScript, i + 8000, 6, 0, 0, 0);
    stepView = addView(0x4188000, drawCels, runViewScript, 8005, 6, 0, 0, 0);
    if (netLevel <= 1)
        netGuideView = addView(0x4108000, drawCels, runViewScript, 9151, 6, 0, 0, 0);
    else
        netGuideView = addView(0x4108000, drawCels, runViewScript, 9153, 6, 0, 0, 0);
    guideGroup = groupViews(netGuideView, netGuideView, 0, 0, 0, 0);
    netMarkerView = addView(0x4181000, drawCels, runViewScript, 7018, 6, 0, 0, 0);
    promptView = addView(0x188000, drawCels, runViewScript, 10018, 6, 0, 0, 0);
    if (netLevel >= 2)
        code1View = addView(0x4108000, drawCels, runViewScript, chosenCode1 + 10002, 6, 0, 0, 0);
    code2View = addView(0x4108000, drawCels, runViewScript, chosenCode2 + 10007, 6, 0, 0, 0);
    code3View = addView(0x4108000, drawCels, runViewScript, chosenCode3 + 10012, 6, 0, 0, 0);
    revealView = addView(0x4188000, drawCels, runViewScript, 7000, 6, 0, 0, 0);
    revealGroup = groupViews(revealView, revealView, 0, 0, 0, 0);
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
        revealSteps = revealStepsAtOpen;
        revealScript = 17 - revealSteps;
        if (netTriesOver) {
            netTriesOver = 0;
            codesDue++;
        }
        return 1;
    }
    return 0;
}

/*
 * Which entry (of codeEntryCount) of the tables codeColumns and codeRows (and, from
 * level 2 (netLevel), codeLayers) holds the codes chosenCode3 and chosenCode2 (and
 * chosenCode1), in the order codeOrder1 (levels 0-1) or codeOrderHigh (from level 2)
 * says; -1 if none.
 */
/* @zoombi32 0x0043dbf3 */
short findCodeEntry()
{
    short i;

    switch (netLevel) {
    case 0:
    case 1:
        if (codeOrder1 == 2) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode3 && codeRows[i] == chosenCode2)
                    return i;
        } else {
            for (i = 0; i < codeEntryCount; i++)
                if (codeRows[i] == chosenCode3 && codeColumns[i] == chosenCode2)
                    return i;
        }
        break;
    case 2:
    case 3:
        if (codeOrderHigh == 0) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode3 && codeRows[i] == chosenCode2 && codeLayers[i] == chosenCode1)
                    return i;
        } else if (codeOrderHigh == 1) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode2 && codeRows[i] == chosenCode3 && codeLayers[i] == chosenCode1)
                    return i;
        } else if (codeOrderHigh == 2) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode1 && codeRows[i] == chosenCode3 && codeLayers[i] == chosenCode2)
                    return i;
        } else if (codeOrderHigh == 3) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode3 && codeRows[i] == chosenCode1 && codeLayers[i] == chosenCode2)
                    return i;
        } else if (codeOrderHigh == 4) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode2 && codeRows[i] == chosenCode1 && codeLayers[i] == chosenCode3)
                    return i;
        } else if (codeOrderHigh == 5) {
            for (i = 0; i < codeEntryCount; i++)
                if (codeColumns[i] == chosenCode1 && codeRows[i] == chosenCode2 && codeLayers[i] == chosenCode3)
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
 * depends on the scenes left and entered (not while practiceLevel, skipJourneyMap or
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

        if (levelJustRaised && level)
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
    if (skipJourneyMap || transitionsOn)
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
 * ones for the codes chosen (chosenCode1, chosenCode2 and chosenCode3, and their
 * second parts previousCode1, previousCode2 and previousCode3; -1: none), by two tables
 * of five; with markerMoved set, also moves the cels to the marker's place
 * (markerX, markerY). A cel below 1 stops it there for good.
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
    if (chosenCode2 != -1)
        col1 = columns[chosenCode2];
    if (chosenCode3 != -1)
        row2 = rows[chosenCode3];
    if (chosenCode1 != -1)
        row0 = rows[chosenCode1];
    if (previousCode2 != -1)
        col1b = columns[previousCode2];
    if (previousCode3 != -1)
        row2b = rows[previousCode3];
    if (previousCode1 != -1)
        row0b = rows[previousCode1];
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
            if (markerMoved) {
                if (!i) {
                    cels[i + 1] = markerX;
                    cels[i + 2] = markerY;
                } else if (!markerStep) {
                    if (netLevel < 2)
                        cels[i + 1] = markerX + 21;
                    else
                        cels[i + 1] = markerX + 3;
                    cels[i + 2] = markerY + 7;
                } else {
                    cels[i + 1] = markerX + 4;
                    cels[i + 2] = markerY + 3;
                }
            }
        }
        i += 3;
    }
}

/*
 * The notify of the Zoombinis crossing (movingSnoid the one moving): 0 flips
 * which way it faces and turns it the way netFacing says (then clears it); 2 runs
 * flyMarker; 4 starts the step onto the net (script 14000 on, by the place
 * currentNetPlace) and puts the marker and the three places' views in order;
 * 20 takes the next Zoombini from place currentNetPlace (a walk by its feet,
 * 13016 on); 30 starts its crossing (13031 on) and stacks it on the one
 * before (lastCrossed); 240-243 note a turn to make (netFacing), 250-253 turn
 * it. When a script ends: after a crossing, it walks on to the next spot
 * of acrossSpots, with sounds when the last one's across; else the Zoombini
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
        netFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (netFacing) {
            setSnoidFacing(snoid, netFacing - 1);
            netFacing = 0;
        }
        break;
    case 2:
        flyMarker(markerPlace);
        break;
    case 4:
        view = findView(movingSnoid);
        anchor = onto[currentNetPlace];
        startSnoidScript((Snoid *)&view->body, currentNetPlace + 14000, &anchor, 0);
        view->notifyEnd = 0;
        view->notify = crossingNotify;
        moveView(standingViews[0], 1, markerViews[markerCount]);
        moveView(standingViews[1], 1, standingViews[0]);
        moveView(standingViews[2], 1, standingViews[1]);
        moveView(movingSnoid, 1, standingViews[2]);
        break;
    case 20:
        movingSnoid = netPlaces[currentNetPlace];
        view = findView(movingSnoid);
        script = ((Snoid *)&view->body)->features[3] - 1;
        script = 2 - currentNetPlace + script * 3 + 13016;
        startSnoidScript((Snoid *)&view->body, script, 0, 0);
        ((Snoid *)&view->body)->chosen = 1;
        view->notifyEnd = 0;
        view->notify = crossingNotify;
        sentIndex = nextToSend;
        movingSnoid = netPlaces[currentNetPlace];
        netPlaces[currentNetPlace] = 0;
        if (!emptyNetPlaces && nextToSend < netPartySize)
            sendsLeft++;
        break;
    case 30:
        anchor = across[currentNetPlace];
        view = findView(movingSnoid);
        script = ((Snoid *)&view->body)->features[3] - 1;
        script = script * 3 + currentNetPlace + 13031;
        startSnoidScript((Snoid *)&view->body, script, &anchor, 0);
        view->notifyEnd = 1;
        view->notify = crossingNotify;
        crossingStarted++;
        if (lastCrossed)
            moveView(movingSnoid, 0, lastCrossed);
        lastCrossed = movingSnoid;
        break;
    case -1:
        if (!crossingStarted) {
            for (i = 2; i >= 0; i--)
                if (waitingOnNet[i] == view->id) {
                    View *waiting = findView(waitingOnNet[i]);

                    waitingOnNet[i] = 0;
                    if (waiting) {
                        setSnoidAction((Snoid *)&waiting->body, 2, 0);
                        if (!waitingOnNet[0] && !waitingOnNet[1] && !waitingOnNet[2]) {
                            startView(promptView, 10018, 0, 0);
                            promptHeld = promptHeld2 = 0;
                        }
                        return;
                    }
                }
            if (!sendsLeft && !netPlaces[0])
                promptHeld = promptHeld2 = 0;
        } else {
            anchor = acrossSpots[acrossCount];
            acrossCount++;
            view = findView(movingSnoid);
            setSnoidAction((Snoid *)&view->body, 7, 0);
            *(Point *)&((Snoid *)&view->body)->targetX = anchor;
            ((Snoid *)&view->body)->chosen = 1;
            view->notifyEnd = 0;
            view->notify = crossingNotify;
            lastAcross = movingSnoid;
            netGoAllowed = 1;
            crossingStarted = 0;
            if (snoidsFound >= netPartySize || emptyNetPlaces)
                promptHeld = promptHeld2 = 0;
            if (!sendsLeft && !netPlaces[0] && !netPlaces[1] && !netPlaces[2]) {
                if (snoidsFound >= netPartySize)
                    queueViewSound(randomBetween(20055, 20063), 0);
                allAcross++;
                netFidgetsOn++;
                netFidgetsAllowed = netPartySize;
            }
            if (remarkAfterCross) {
                remarkAfterCross = 0;
                if (snoidsFound >= 1 && snoidsFound < netPartySize)
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
 * entries of placeGroups, adds a view for each (script 9000 on, or 9025 on),
 * and picks the order of the codes (codeOrder1-codeOrder3, distinct from
 * level 3) and, from level 3, codeOrderHigh.
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

    fillMemory(placeGroups, 0, 250);
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
            codeRowsCopy[j] = codeRows[j];
        for (k = 1; k < 5; k++) {
            for (i = 0; i < shifts; i++) {
                saved = codeRowsCopy[4];
                for (j = 4; j > 0; j--)
                    codeRowsCopy[j] = codeRowsCopy[j - 1];
                codeRowsCopy[0] = saved;
            }
            for (j = 0; j < 5; j++)
                codeRows[k * 5 + j] = codeRowsCopy[j];
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
                        codeRowsCopy[j] = codeColumns[k + i * 5 + j * 25 - 1];
                    for (m = 0; m < shifts; m++) {
                        saved = codeRowsCopy[4];
                        for (j = 4; j > 0; j--)
                            codeRowsCopy[j] = codeRowsCopy[j - 1];
                        codeRowsCopy[0] = saved;
                    }
                    for (j = 0; j < 5; j++)
                        codeColumns[i * 5 + k + j * 25] = codeRowsCopy[j];
                }
        } else {
            for (k = 5; k < codeEntryCount; k += 5) {
                for (j = 0; j < 5; j++)
                    codeRowsCopy[j] = codeLayers[k + j - 5];
                for (i = 0; i < shifts; i++) {
                    saved = codeRowsCopy[4];
                    for (j = 4; j > 0; j--)
                        codeRowsCopy[j] = codeRowsCopy[j - 1];
                    codeRowsCopy[0] = saved;
                }
                for (j = 0; j < 5; j++)
                    codeLayers[k + j] = codeRowsCopy[j];
            }
        }
        break;
    }
    scriptBase = 0;
    if (netLevel > 1)
        scriptBase = 25;
    for (i = 0; i < netGroupCount; i++) {
        do {
            if (netLevel < 2)
                k = randomUpTo(24);
            else
                k = randomUpTo(124);
        } while (placeGroups[k]);
        placeGroups[k] = netGroups[i];
    }
    for (i = 0; i < codeEntryCount; i++) {
        placeGroupViews[i] = 0;
        if (placeGroups[i]) {
            placeGroupViews[i] = addView(0x4188000, drawCels, runViewScript, scriptBase + i + 9000, 6, 0, 0, 0);
            image = placeGroups[i] + 150;
            if (netLevel > 1)
                image += 3;
            View *view = findView(placeGroupViews[i]);

            if (view) {
                view->placed = placeSecondCel;
                short *parts = (short *)&view->body;

                parts[20] = image;
            }
            moveView(placeGroupViews[i], 0, standingViews[0]);
        }
    }
    if (netLevel <= 2) {
        if (randomUpTo(1)) {
            codeOrder1 = 2;
            codeOrder2 = 1;
            codeOrder3 = 0;
        } else {
            codeOrder1 = 1;
            codeOrder2 = 2;
            codeOrder3 = 0;
        }
    } else {
        distinct = 0;
        do {
            codeOrder1 = randomUpTo(2);
            codeOrder2 = randomUpTo(2);
            codeOrder3 = randomUpTo(2);
            if (codeOrder1 != codeOrder2 && codeOrder2 != codeOrder3 && codeOrder3 != codeOrder1)
                distinct++;
        } while (!distinct);
    }
    if (netLevel >= 3)
        codeOrderHigh = randomUpTo(5);
}

/*
 * A code chosen: `which` (1-3) sets the first, second or third code to
 * `value` (keeping the previous ones in previousCode1-previousCode3) and shows it
 * (script 10002, 10007 or 10012 on), and once all the codes the level
 * needs are set, shows the marker (7026, 7027 or 7019) and the net's view
 * (10018). `which` 0 sends the marker off (10001) to the entry for the
 * codes (markerPlace; script 7028 on by its column) with crossingNotify.
 */
/* @zoombi32 0x0043d0b4 */
void chooseCode(short which, short value)
{
    View *view;
    short ready = 0;
    short column;

    if ((netLevel <= 1 && chosenCode2 >= 0 && chosenCode3 >= 0) || (chosenCode1 >= 0 && chosenCode2 >= 0 && chosenCode3 >= 0))
        ready = 1;
    switch (which) {
    case 1:
        if (netLevel >= 2) {
            previousCode2 = chosenCode2;
            previousCode1 = chosenCode1;
            chosenCode1 = value;
            if (previousCode1 != chosenCode1) {
                startView(code1View, value + 10002, 0, 0);
                if (!netTriesOver && ready) {
                    view = startView(netMarkerView, 7027, 0, 0);
                    if (view) {
                        markerMoved = 0;
                        view->placed = markerPlaced;
                    }
                    if (!promptHeld)
                        startView(promptView, 10018, 0, 0);
                }
            }
        }
        break;
    case 2:
        previousCode2 = chosenCode2;
        chosenCode2 = value;
        previousCode1 = chosenCode1;
        if (previousCode2 != chosenCode2) {
            startView(code2View, value + 10007, 0, 0);
            if (!netTriesOver && ready) {
                view = findView(netMarkerView);
                if (view) {
                    setViewScript(view, 7019, 1);
                    view->interval = 3;
                } else {
                    netMarkerView = addView(0x4108000, drawCels, runViewScript, 7019, 3, 0, 0, 0);
                    view = findView(netMarkerView);
                }
                if (view) {
                    markerMoved = 0;
                    view->placed = markerPlaced;
                }
                if (!promptHeld)
                    startView(promptView, 10018, 0, 0);
            }
        }
        break;
    case 3:
        previousCode3 = chosenCode3;
        previousCode2 = chosenCode2;
        previousCode1 = chosenCode1;
        chosenCode3 = value;
        if (previousCode3 != chosenCode3) {
            startView(code3View, value + 10012, 0, 0);
            if (!netTriesOver && ready) {
                view = startView(netMarkerView, 7026, 0, 0);
                if (view) {
                    markerMoved = 0;
                    view->placed = markerPlaced;
                }
                if (!promptHeld)
                    startView(promptView, 10018, 0, 0);
            }
        }
        break;
    case 0:
        previousCode3 = chosenCode3;
        previousCode2 = chosenCode2;
        previousCode1 = chosenCode1;
        if ((netLevel <= 1 && chosenCode2 >= 0 && chosenCode3 >= 0)
            || (chosenCode1 >= 0 && chosenCode2 >= 0 && chosenCode3 >= 0)) {
            startView(promptView, 10001, 0, 0);
            codesShown = 0;
            markerPlace = findCodeEntry();
            if (netLevel < 2)
                column = markerPlace % 5;
            else
                column = markerPlace % 25 / 5;
            if (!column)
                markerColumnKind = 1;
            else if (column >= 1 && column < 4)
                markerColumnKind = 0;
            else
                markerColumnKind = 2;
            view = startView(netMarkerView, markerColumnKind + 7028, crossingNotify, 0);
            if (view) {
                markerMoved = 0;
                view->placed = markerPlaced;
                view->interval = 6;
                markerGroup = groupViews(netMarkerView, netMarkerView, 0, 0, 0, 0);
            }
        }
        break;
    }
    if (!codesShown && !netTriesOver) {
        codesDue++;
        codesShown++;
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

    if (inNetFrame || !netOpen)
        return;
    inNetFrame = netFrameEntered = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            inNetFrame = 0;
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
                inNetFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (revealing && revealGroup && !groupLeader[revealGroup]) {
        if (++revealStep < revealSteps) {
            startView(revealView, revealStep + 7000, 0, 0);
            revealGroup = groupViews(revealView, revealView, 0, 0, 0, 0);
        } else {
            revealing = revealGroup = 0;
            sendsLeft = 3;
            sendAllowed++;
            sendNextToNet();
            codesDue = 1;
            codesShown = 1;
        }
    }
    if (guideGroup && !groupLeader[guideGroup]) {
        guideGroup = 0;
        setUpCodes();
    }
    if (markerGroup) {
        if (!groupLeader[markerGroup]) {
            markerGroup = 0;
            if (--revealSteps < 0) {
                netTriesOver++;
                if (randomBetween(0, 4) > netLevel || (*(short *)(gameState + 0x3c) & 0xfff) <= 3) {
                    if (groupToCross < 1) {
                        if (snoidsFound >= 1 && snoidsFound < netPartySize)
                            queueViewSound(randomBetween(20045, 20048), 0);
                    } else if (nextToSend < netPartySize) {
                        remarkAfterCross++;
                    }
                }
            }
            if (groupToCross >= 1)
                crossDue++;
            else
                groupToCross = 0;
        }
    } else if (codesDue && !netTriesOver) {
        if ((netLevel <= 1 && chosenCode2 >= 0 && chosenCode3 >= 0) || (chosenCode1 >= 0 && chosenCode2 >= 0 && chosenCode3 >= 0)) {
            codesDue = 0;
            startView(revealView, revealScript + 7031, 0, 0);
            markerStep1Group = groupViews(revealView, revealView, 0, 0, 0, 0);
            if (++revealScript > 16)
                revealScript = 16;
        }
    } else if (markerStep1Group) {
        markerStep1Group = 0;
        previousCode2 = chosenCode2;
        previousCode3 = -1;
        previousCode1 = -1;
        if (netLevel <= 1)
            chosenCode1 = -1;
        if (!netTriesOver) {
            view = startView(netMarkerView, 7018, 0, 0);
            if (view) {
                markerMoved = 0;
                view->interval = 3;
                view->placed = markerPlaced;
                markerStep2Group = groupViews(netMarkerView, netMarkerView, 0, 0, 0, 0);
            }
        }
    } else if (markerStep2Group) {
        if (!groupLeader[markerStep2Group]) {
            markerStep2Group = 0;
            sendNextToNet();
            view = startView(netMarkerView, 7025, 0, 0);
            if (view) {
                view->interval = 2;
                markerStep3Group = groupViews(netMarkerView, netMarkerView, 0, 0, 0, 0);
            }
        }
    } else if (markerStep3Group) {
        if (!groupLeader[markerStep3Group]) {
            markerStep3Group = 0;
            view = startView(netMarkerView, 7026, 0, 0);
            if (view) {
                markerMoved = 0;
                view->placed = markerPlaced;
                view->interval = 6;
                markerStep4Group = groupViews(netMarkerView, netMarkerView, 0, 0, 0, 0);
            }
        }
    } else if (markerStep4Group) {
        if (!groupLeader[markerStep4Group]) {
            markerStep4Group = codesLocked = 0;
            if (netLevel <= 1) {
                markerStep5Group = 0;
                if (firstPrompt) {
                    startView(promptView, 10017, 0, 0);
                    firstPrompt = 0;
                    lastPromptTime = clockTime();
                } else if (markerMissed || snoidsFound >= netPartySize) {
                    promptHeld2 = markerMissed = 0;
                    startView(promptView, 10018, 0, 0);
                } else {
                    promptHeld2 = markerMissed = 0;
                }
            } else {
                previousCode1 = -1;
                view = startView(netMarkerView, 7027, 0, 0);
                if (view) {
                    markerStep5Group = groupViews(netMarkerView, netMarkerView, 0, 0, 0, 0);
                    markerMoved = 0;
                    view->placed = markerPlaced;
                }
            }
        }
    } else if (markerStep5Group) {
        if (!groupLeader[markerStep5Group]) {
            markerStep5Group = 0;
            if (firstPrompt) {
                startView(promptView, 10017, 0, 0);
                firstPrompt = 0;
                lastPromptTime = clockTime();
            } else if (markerMissed || snoidsFound >= netPartySize) {
                promptHeld2 = markerMissed = 0;
                startView(promptView, 10018, 0, 0);
            } else {
                promptHeld2 = markerMissed = 0;
            }
        }
    }
    if (crossDue && groupToCross) {
        crossDue = 0;
        if (nextToSend >= netPartySize)
            sendsLeft = 0;
        if (--groupToCross <= 0) {
            groupToCross = 0;
            if (sentIndex < 0 && sendsLeft)
                sendNextToNet();
        }
        if (netLevel < 2)
            column = markerPlace % 5;
        else
            column = markerPlace % 25 / 5;
        startView(standingViews[column], column + 8000, 0, 0);
        if (markerCount)
            moveView(standingViews[column], 1, markerViews[markerCount]);
        standingGroup = groupViews(standingViews[column], standingViews[column], 0, 0, 0, 0);
        promptHeld++;
        lastPromptTime = clockTime();
    } else if (standingGroup) {
        if (!groupLeader[standingGroup]) {
            standingGroup = 0;
            currentNetPlace = 0;
            for (i = 0; i < 3; i++)
                if (netPlaces[i]) {
                    currentNetPlace = i;
                    break;
                }
            if (netPlaces[currentNetPlace]) {
                view = startView(stepView, currentNetPlace + 8005, crossingNotify, 0);
                if (view)
                    stepGroup = groupViews(stepView, stepView, 0, 0, 0, 0);
            }
        }
    } else if (stepGroup) {
        if (!groupLeader[stepGroup]) {
            stepGroup = 0;
            if (groupToCross)
                crossDue++;
            else
                sendAllowed++;
            if (snoidsFound >= netPartySize && !groupToCross)
                groupToCross = crossDue = 0;
        }
    } else if (sentIndex >= 0 && !netTriesOver && sentIndex < netPartySize) {
        view = idleSnoidView(partyViews[sentIndex]);
        if (view) {
            script = ((Snoid *)&view->body)->features[3] - 1;
            script = script + (2 - currentNetPlace) * 5 + 13001;
            startSnoidScript((Snoid *)&view->body, script, 0, 0);
            view->notify = crossingNotify;
            view->notifyEnd = 1;
            sentCount++;
            waitingOnNet[currentNetPlace] = netPlaces[currentNetPlace];
            sentIndex = -1;
            sendUnderway = 0;
        }
    }
    if (sentIndex < 0 && sendsLeft)
        sendNextToNet();
    if (netFidgetsOn && netFidgets < netFidgetsAllowed) {
        if (clockTime() - lastNetFidgetTime > 30) {
            done = 0;
            tries = 0;
            lastNetFidgetTime = clockTime();
            do {
                i = allocateSlot(&netFidgetersUsed, netPartySize, 0);
                if (partyViews[i] != netPlaces[0] && partyViews[i] != netPlaces[1] && partyViews[i] != netPlaces[2]) {
                    netFidgeter = idleSnoidView(partyViews[i]);
                    if (netFidgeter && netFidgeter->body.running && netFidgeter->flags == 1) {
                        if (!((Snoid *)&netFidgeter->body)->chosen || allAcross) {
                            fidget = ((Snoid *)&netFidgeter->body)->features[3] - 1;
                            fidget += 13046;
                            startSnoidScript((Snoid *)&netFidgeter->body, fidget, 0, 0);
                            netFidgets++;
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
    } else if (netFidgets >= netFidgetsAllowed) {
        netFidgets = netFidgetsOn = lastNetFidgetTime = netFidgetersUsed = 0;
    }
    if (promptHeld) {
        if (clockTime() - lastPromptTime > 720) {
            promptHeld = promptHeld2 = 0;
            startView(promptView, 10018, 0, 0);
            lastPromptTime = clockTime();
        }
    } else if (clockTime() - lastPromptTime > 7200) {
        promptHeld = promptHeld2 = lastPromptTime = 0;
        startView(promptView, 10018, 0, 0);
        lastPromptTime = clockTime();
    }
    playAmbientSound();
    inNetFrame = 0;
}

/*
 * Scene 15's buttons: leaves at once if asked to; unless codesLocked, 3 sends
 * the marker off with the codes chosen, and 4-8, 9-13 and 14-18 choose the
 * first (from level 2), second and third code. 1 asks to leave for the map
 * (999, keeping the party); 2, once allowed (netGoAllowed), sends the
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
    if (!codesLocked) {
        switch (button) {
        case 3:
            if (!promptHeld && !promptHeld2 && !netTriesOver) {
                promptHeld++;
                promptHeld2++;
                lastPromptTime = clockTime();
                code = 0;
                chooseCode(0, code);
            }
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            if (netLevel > 1 && (!promptHeld2 || netTriesOver)) {
                code = button - 4;
                chooseCode(1, code);
            }
            break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            if (!promptHeld2 || netTriesOver) {
                code = button - 9;
                chooseCode(2, code);
            }
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            if (!promptHeld2 || netTriesOver) {
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
        if (netGoAllowed) {
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
