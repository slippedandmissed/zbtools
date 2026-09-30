/*
 * slides (0x446bf8-0x44b550): Stone Rise (scene 12), 'Slides.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "mainloop.h"
#include "net.h"
#include "platform.h"
#include "random.h"
#include "slides.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Opens scene 12 (Stone Rise): Slides.MHK, the board (all cells empty,
   500, until layOutGrid lays it out for the level), the cells' views, the
   buttons and the party. */
/* @zoombi32 0x00446bf8 */
void openStoneRise()
{
    short i;

    hintSound = 0;
    snoidsOnTheirWay = snoidsArrived = 0;
    sceneDue = listedCount = pathStart = 0;
    stoneRiseOpen = slidesGoReady = 0;
    alikeTarget = markerView = cellMarked = slidesDragLocked = 0;
    slidesMoves = slidesFidgetStarted = cyclingColours = 0;
    savedPartyFlags = -1;
    openGameFile(&slidesFile, "Slides.MHK");
    setCurrentMap(slidesFile);
    fillMemory(slidesRowViews, 0, 20);
    fillMemory(hexCells, 0, sizeof hexCells);
    fillMemory(cellLinkBits, 0, sizeof cellLinkBits);
    fillMemory(unusedCellTable, 0, sizeof unusedCellTable);
    fillMemory(startCells, 0, 20);
    for (i = 0; i < 117; i++)
        hexCells[i].state = 500;
    startCellCount = startCellsGroup = finishGroup = slidesGoPressed = solveTyped = 0;
    stoneRiseLevel = sceneLevel();
    if (stoneRiseLevel == 3)
        loadPaths(1000);
    loadTerrain(100);
    drawBackdrop(5000);
    slidesButtonImages = loadImageBank(6000, &slidesButtonResource);
    loadFeatureGroup(7000, 0, 0);
    loadFeatureGroup(8000, 1, 0);
    loadScripts(7000, 14);
    addScripts(8000, 3, 0);
    addView(0x1000, drawSlidesButtons, updateSlidesButtons, 0, 0, 0, 0, 0);
    loadSnoidScripts(14000, 4, 0);
    addSnoidScripts(13000, 6, 0);
    setViewPlaces(16, slidesPlaces, 1);
    copyPaletteRange(10, 236);
    makePartySnoids(0);
    enterSnoids(0);
    slidesChosen = listChosenSnoids();
    slidesFidgetsAllowed = partySize = slidesChosen->count;
    layOutGrid();
    moveView(slidesRowViews[1], 0, hexCells[9].view);
    moveView(slidesRowViews[2], 0, hexCells[27].view);
    moveView(slidesRowViews[3], 0, hexCells[45].view);
    moveView(slidesRowViews[4], 0, hexCells[63].view);
    moveView(slidesRowViews[5], 0, hexCells[81].view);
    moveView(slidesRowViews[6], 0, hexCells[99].view);
    for (i = 0; i < listedCount; i++) {
        placedViews[i] = addView(0x188a000, drawCels, runViewScript, 7013, 7, &listedCellPlaces[i], 0, 0);
        findView(placedViews[i])->placed = placeListedCell;
    }
    moveView(slidesRowViews[7], 1, placedViews[listedCount - 1]);
    updateViews();
    staggerSnoids(45, 30);
    chooseSnoids(0, 0);
    setGroupLists(slidesGroups, 1, (short)0xc000);
    drawSlidesButton(1, 0, 0);
    drawSlidesButton(2, 0, 0);
    addSoundRange(7001, 7001, 0);
    addSoundRange(7000, 7000, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(8000, 8000, 0);
    addSoundRange(8500, 8599, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(7002, 7002, 0);
    showRect(&shownGameRect);
    fadeInViews();
    unloadSounds();
    queueViewSound(997, 0);
    stoneRiseOpen = 1;
    campHint((short *)(gameState + 0x36));
    hintSound = 20078;
}

/* Closes scene 12. */
/* @zoombi32 0x00447124 */
void closeStoneRise()
{
    if (stoneRiseOpen) {
        stoneRiseOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&slidesButtonResource);
        setFreeAtOnce(saved);
        closeGameFile(&slidesFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* The buttons' view update: redraws button 2 when slidesGoReady changes, and
   button 1 the first time. */
/* @zoombi32 0x004470b2 */
void updateSlidesButtons(View *, short region)
{
    if (slidesGoReady) {
        if (!slidesButton2Lit) {
            slidesButton2Lit = 1;
            unionRgnRect(region, &slidesButtons[1].rect);
        }
    } else if (slidesButton2Lit) {
        slidesButton2Lit = 0;
        unionRgnRect(region, &slidesButtons[1].rect);
    }
    if (!slidesButton1Drawn) {
        slidesButton1Drawn = 1;
        unionRgnRect(region, &slidesButtons[0].rect);
    }
}

/* Marks the Zoombini on each cell in state 508 (chosen). */
/* @zoombi32 0x0044943b */
void markLitSnoids()
{
    short i;

    for (i = 0; i < 117; i++)
        if (hexCells[i].state == 508)
            ((Snoid *)&findView(hexCells[i].snoid)->body)->chosen = 1;
}

/* Sets slidesGoReady if any of the cells listedCells lists (1 to listedCount) is in
   state 508. */
/* @zoombi32 0x00449475 */
void noteAnyLit()
{
    short i;

    slidesGoReady = 0;
    for (i = 1; i <= listedCount; i++)
        if (hexCells[listedCells[i]].state == 508) {
            slidesGoReady = 1;
            return;
        }
}

/* Counts the cells in state 502 or 508, and sums their numbers into
   litSum. */
/* @zoombi32 0x0044b261 */
short countLitCells()
{
    short n;
    short i;

    n = litSum = 0;
    for (i = 0; i < 117; i++)
        if (hexCells[i].state == 502 || hexCells[i].state == 508) {
            n++;
            litSum += i;
        }
    return n;
}

/* Remarks on the count of cells in state 502 or 508 against the last
   (lastLitCount): up (more than four: 8505, else 8504, and sets slidesGoReady), down
   (8501 or 8500), or the same count on other cells (8502). A long remark
   already playing is stopped first. */
/* @zoombi32 0x0044b2a4 */
void remarkOnLit()
{
    short n = countLitCells();

    if (n > lastLitCount) {
        slidesGoReady = 1;
        if (n - lastLitCount > 4) {
            if (soundOn && lastViewSound == 8505) {
                stopSounds(8505, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8505, 0);
        } else {
            if (soundOn && lastViewSound == 8504) {
                stopSounds(8504, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8504, 0);
        }
    } else if (n < lastLitCount) {
        if (lastLitCount - n > 4) {
            if (soundOn && lastViewSound == 8501) {
                stopSounds(8501, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8501, 0);
        } else {
            if (soundOn && lastViewSound == 8500) {
                stopSounds(8500, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8500, 0);
        }
    } else if (litSum != lastLitSum) {
        queueViewSound(8502, 0);
    }
}

/* Draws button `which` (1 or 2; 2 is dim unless slidesGoReady), lit or not,
   showing it on screen if `show`. */
/* @zoombi32 0x00446ffc */
void drawSlidesButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!slidesGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(slidesButtonImages->offsets[image] + (char *)slidesButtonImages), slidesButtons[which - 1].rect.left,
                      slidesButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&slidesButtons[which - 1].rect);
    }
}

/* Counts, for each feature, how many of its values the chosen Zoombinis
   (slidesChosen, partySize of them) show, into featureValueCounts. */
/* @zoombi32 0x004488e8 */
void countPartyValues()
{
    short counts[4][6];
    short i;
    short j;

    featureValueCounts[0] = 0;
    featureValueCounts[1] = 0;
    featureValueCounts[2] = 0;
    featureValueCounts[3] = 0;
    slidesChosen = listChosenSnoids();
    partySize = slidesChosen->count;
    fillMemory(counts, 0, sizeof counts);
    for (i = 0; i < partySize; i++)
        for (j = 0; j < 4; j++)
            counts[j][slidesChosen->features[i][j]]++;
    for (i = 0; i < 4; i++)
        for (j = 1; j < 6; j++)
            if (counts[i][j])
                featureValueCounts[i]++;
}

/* Cycles palette colours 19-21 by one (the last to the first). */
/* @zoombi32 0x00448bf5 */
void cycleColors()
{
    PALETTEENTRY last;
    PALETTEENTRY colors[256];
    short first;
    short count;

    first = 19;
    count = 3;
    getColors(&colors[10], 10, 236);
    last = colors[first + count - 1];
    memmove(&colors[first + 1], &colors[first], (count - 1) * sizeof(PALETTEENTRY));
    colors[first] = last;
    setColors(&colors[10], 10, 236);
}

/* Clears the party's features (partyHair to partyTaken, pairFeatures, waitingSnoids) and
   reads each Zoombini's (partySize of them, partyViews) into partyHair to partyFeet. */
/* @zoombi32 0x00449b40 */
void readPartyFeatures()
{
    Snoid *snoid;
    short i;

    fillMemory(partyHair, 0, 32);
    fillMemory(partyEyes, 0, 32);
    fillMemory(partyNoses, 0, 32);
    fillMemory(partyFeet, 0, 32);
    fillMemory(partyTaken, 0, 32);
    fillMemory(pairFeatures, 0, 32);
    fillMemory(waitingSnoids, 0, 32);
    groupCount = 0;
    for (i = 0; i < partySize; i++) {
        snoid = (Snoid *)&findView(partyViews[i])->body;
        partyHair[i] = snoid->features[0];
        partyEyes[i] = snoid->features[1];
        partyNoses[i] = snoid->features[2];
        partyFeet[i] = snoid->features[3];
    }
}

/* Orders the party (into waitingSnoids) by how many others share a feature with
   each, most first. */
/* @zoombi32 0x00449c18 */
void orderPartyByAlike()
{
    short alike[16];
    short i;
    short j;
    short best;
    short most;

    fillMemory(alike, 0, sizeof alike);
    for (i = 0; i < partySize; i++)
        for (j = 0; j < partySize; j++)
            if (partyHair[i] == partyHair[j] || partyEyes[i] == partyEyes[j]
                || partyNoses[i] == partyNoses[j] || partyFeet[i] == partyFeet[j])
                alike[i]++;
    for (i = 0; i < partySize; i++) {
        most = best = 0;
        for (j = 0; j < partySize; j++)
            if (most < alike[j]) {
                most = alike[j];
                best = j;
            }
        waitingSnoids[i] = best;
        alike[best] = -1;
    }
}

/* A cell's placed callback: moves its cels into place and drops the
   images 4-24 for the directions its cell (listedCells, by the view's place)
   has no link in (cellLinkBits's bits). */
/* @zoombi32 0x00448c81 */
void placeListedCell(View *view)
{
    ViewCel *cel;
    short cell;
    short removed;

    cell = listedCells[(short)(view->id - placedViews[0]) + 1];
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
        cel->x += -22;
        cel->y += 6;
        switch (cel->image) {
        case 4:
            if (!(cellLinkBits[cell] & 1)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 8:
            if (!(cellLinkBits[cell] & 2)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 12:
            if (!(cellLinkBits[cell] & 4)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 16:
            if (!(cellLinkBits[cell] & 8)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 20:
            if (!(cellLinkBits[cell] & 0x10)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 24:
            if (!(cellLinkBits[cell] & 0x20)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        }
        if (!removed)
            cel++;
    }
}

/* Picks a free place for a Zoombini among slidesPlaces (the one nearest
   each place that's free and not already picked, noted in sortedIds), from
   one end or the other at random, and gives its point in `where`. */
/* @zoombi32 0x0044b3ee */
void pickSlidesPlace(Point *where)
{
    Point spot = {0, 0};
    short i;
    short skip;
    short id;
    short j;

    spotTaken(&spot, 0, 500);
    for (i = 0; i < 16; i++) {
        skip = 0;
        id = spotNear(&slidesPlaces[i], 500, skip);
        for (j = 0; id && j < i; j++)
            if (id == sortedIds[j]) {
                skip++;
                id = spotNear(&slidesPlaces[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = id;
    }
    id = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 15; id == -1 && i >= 0; i--)
            if (!sortedIds[i])
                id = i;
    } else {
        for (i = 0; id == -1 && i < 16; i++)
            if (!sortedIds[i])
                id = i;
    }
    if (id == -1)
        id = 0;
    *where = slidesPlaces[id];
}

/* Another Zoombini of the party (not `who`, and not marked in partyTaken)
   with the same value of a feature (sharedFeature, each in turn from a random
   one); -1 if none. */
/* @zoombi32 0x00449a21 */
short findAlike(short who)
{
    short tries;
    short i;

    sharedFeature = randomUpTo(3);
    tries = 4;
    do {
        if (++sharedFeature > 3)
            sharedFeature = 0;
        for (i = 0; i < partySize; i++) {
            if (i == who)
                continue;
            switch (sharedFeature) {
            case 0:
                if (!partyTaken[i] && partyHair[who] == partyHair[i])
                    return i;
                break;
            case 1:
                if (!partyTaken[i] && partyEyes[who] == partyEyes[i])
                    return i;
                break;
            case 2:
                if (!partyTaken[i] && partyNoses[who] == partyNoses[i])
                    return i;
                break;
            case 3:
                if (!partyTaken[i] && partyFeet[who] == partyFeet[i])
                    return i;
                break;
            }
        }
    } while (--tries);
    return -1;
}

/* Layers the Zoombinis' views: those on the listed cells (listedCells) in state
   507 go behind the next cell's view; the party's Zoombinis on none of them
   in state 507 or 508 go behind the first cell's. */
/* @zoombi32 0x0044aa79 */
void layerSnoidViews()
{
    short placed[16];
    View *view;
    short i;
    short j;

    setViewsLocked(0);
    fillMemory(placed, 0, sizeof placed);
    for (i = 1; i <= listedCount; i++) {
        if (hexCells[listedCells[i]].state == 507) {
            view = findView(hexCells[listedCells[i]].snoid);
            if (view) {
                view->flags |= 0x4008000;
                moveView(hexCells[listedCells[i]].snoid, 0, hexCells[listedCells[i] + 1].view);
            }
            for (j = 0; j < 16; j++)
                if (partyViews[j] == hexCells[listedCells[i]].snoid)
                    placed[j]++;
        } else if (hexCells[listedCells[i]].state == 508) {
            for (j = 0; j < 16; j++)
                if (partyViews[j] == hexCells[listedCells[i]].snoid)
                    placed[j]++;
        }
    }
    for (i = 0; i < 16; i++)
        if (!placed[i]) {
            view = findView(partyViews[i]);
            if (view) {
                view->flags |= 0x4008000;
                moveView(partyViews[i], 0, hexCells[0].view);
            }
        }
}

/* A cell's placed callback: keeps image 109 only on cells in state 502, 504
   or 508 unless startState is 505, and image 110 on cells in state 502, 505
   or 508 while startState is 505; images 4, 8 and 24 only where the cell has
   that link (cellLinkBits), moved on by linkImageOffset. */
/* @zoombi32 0x00448d9d */
void placeCellImages(View *view)
{
    ViewCel *cel;
    short removed;
    short cell;

    cell = view->id - hexCells[0].view;
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
        switch (cel->image) {
        case 109:
            if (!((hexCells[cell].state == 502 || hexCells[cell].state == 504 || hexCells[cell].state == 508)
                  && startState != 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 110:
            if (!((hexCells[cell].state == 502 || hexCells[cell].state == 505 || hexCells[cell].state == 508)
                  && startState == 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 4:
            if (!(cellLinkBits[cell] & 1)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += linkImageOffset;
            }
            break;
        case 8:
            if (!(cellLinkBits[cell] & 2)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += linkImageOffset;
            }
            break;
        case 24:
            if (!(cellLinkBits[cell] & 0x20)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += linkImageOffset;
            }
            break;
        }
        if (!removed)
            cel++;
    }
}

/* The buttons' view's draw callback: draws both buttons, dim. */
/* @zoombi32 0x00447095 */
void drawSlidesButtons(View *)
{
    drawSlidesButton(1, 0, 0);
    drawSlidesButton(2, 0, 0);
}

/* A view's update cycling colours 19-21 at its interval. */
/* @zoombi32 0x004489a8 */
void cycleColorsUpdate(View *view, short)
{
    if (view->nextUpdate <= updateTime) {
        view->nextUpdate = updateTime + view->interval;
        cycleColors();
    }
}

/* Marks the cell at (x, y) (within 45 by 22 of its point in cellPoints),
   unless one is already (cellMarked): notes it in markedCell and shows the
   marker view markerView there (script 8000-8002 by row, notify walkToMarkNotify),
   layered with the row. */
/* Not exact: register allocation in the loop's tests (the original keeps
   `y` in ecx and uses esi for scratch; here `y` shares esi with `kind`). */
/* @zoombi32 0x0044b0fc */
void markCellAt(short x, short y)
{
    Point at;
    Point *points = cellPoints;
    short i;
    short kind;

    if (!cellMarked) {
        for (i = 0; i < 117; i++) {
            if (y >= points[i].y - 22 && y <= points[i].y && x >= points[i].x
                && x <= points[i].x + 45) {
                cellMarked = 1;
                kind = 0;
                markedCell = i;
                if (i < 36)
                    kind = 2;
                else if (i < 89)
                    kind = 1;
                at.x = points[i].x + 19;
                at.y = points[i].y + 24;
                if (markerView)
                    deleteView(markerView);
                markerView = addView(0x908000, drawCels, runViewScript, kind + 8000, 6, &at, 0, 0);
                findView(markerView)->notify = walkToMarkNotify;
                if (i % 18)
                    moveView(markerView, 0, slidesRowViews[7]);
                else {
                    short row = i / 18 + 1;

                    moveView(markerView, 0, slidesRowViews[row]);
                }
                return;
            }
        }
    }
}

/* The notify of the Zoombini walking to the marked cell (the view
   markWalker): 90-92 walk it on (scripts 14000-14002) toward its own place
   raised 50, 93 off to a random spot (14003, unmarking the cell); 240-243
   note a facing to take at the end (pendingMarkFacing), 250-253 face it at once; the
   end (0) flips it and takes the noted facing. */
/* @zoombi32 0x0044af15 */
void walkToMarkNotify(View *view, short event)
{
    Point at;
    View *walker;
    Snoid *snoid;

    snoid = viewSnoid(view);
    walker = findView(markWalker);
    at.x = walker->body.x;
    at.y = -50;
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
        pendingMarkFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingMarkFacing) {
            setSnoidFacing(snoid, pendingMarkFacing - 1);
            pendingMarkFacing = 0;
        }
        break;
    case 90:
        walker = findView(markWalker);
        startSnoidScript(viewSnoid(walker), 14000, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = walkToMarkNotify;
        cellMarked = 1;
        break;
    case 91:
        walker = findView(markWalker);
        startSnoidScript(viewSnoid(walker), 14001, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = walkToMarkNotify;
        cellMarked = 1;
        break;
    case 92:
        walker = findView(markWalker);
        startSnoidScript(viewSnoid(walker), 14002, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = walkToMarkNotify;
        cellMarked = 1;
        break;
    case 93:
        walker = findView(markWalker);
        at.x = randomUpTo(42) + 70;
        at.y = randomUpTo(200) + 152;
        startSnoidScript(viewSnoid(walker), 14003, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = walkToMarkNotify;
        cellMarked = 0;
        break;
    }
}

/* Groups the party in threes (groupCount groups; partyTaken marks those
   taken): each next Zoombini shares a feature with the last where one can
   (noted in pairFeatures as 510-513 by feature, else 501). */
/* @zoombi32 0x0044986f */
void groupInThrees()
{
    short group;
    short n;
    short who;
    short other;
    short i;

    who = n = 0;
    fillMemory(pairFeatures, 0, 32);
    fillMemory(partyTaken, 0, 32);
    groupCount = partySize / 3;
    if (partySize % 3)
        groupCount++;
    for (group = 0; group < groupCount; group++) {
        partyTaken[who] = 1;
        other = findAlike(who);
        if (other == -1) {
            pairFeatures[n] = 501;
            n++;
            for (i = 1; i < partySize; i++)
                if (!partyTaken[i]) {
                    other = i;
                    partyTaken[other] = 1;
                    break;
                }
        } else {
            partyTaken[other] = 1;
            pairFeatures[n] = sharedFeature + 510;
            n++;
        }
        who = other;
        other = -1;
        for (i = 1; i < partySize; i++)
            if (!partyTaken[i])
                other = i;
        if (other == -1)
            return;
        other = findAlike(who);
        if (other == -1) {
            pairFeatures[n] = 501;
            n++;
            for (i = 1; i < partySize; i++)
                if (!partyTaken[i]) {
                    other = i;
                    who = i;
                }
        } else {
            partyTaken[other] = 1;
            pairFeatures[n] = sharedFeature + 510;
            n++;
        }
        partyTaken[other] = 1;
        other = -1;
        for (i = 1; i < partySize; i++)
            if (!partyTaken[i]) {
                other = i;
                who = i;
            }
        if (other == -1)
            return;
    }
}

/* Whether the Zoombinis on cells `a` and `b` share a feature, checking the
   features from a random one on: 510-513 for the first shared (hair, eyes,
   nose, feet); 0 if none, or if a cell is empty. */
/* Not exact: register allocation (the original keeps `a` in eax; here it
   shares ebx with `feet`), as in markCellAt. */
/* @zoombi32 0x00449f96 */
short sharedStone(short a, short b)
{
    Snoid *snoid;
    short feet;
    short hair;
    short eyes;
    short nose;
    short otherFeet;
    short otherHair;
    short otherEyes;
    short otherNose;
    short r;

    if (hexCells[a].state == 500 || hexCells[b].state == 500)
        return 0;
    if (!hexCells[a].snoid || !hexCells[b].snoid)
        return 0;
    snoid = (Snoid *)&findView(hexCells[a].snoid)->body;
    hair = snoid->features[0];
    eyes = snoid->features[1];
    nose = snoid->features[2];
    feet = snoid->features[3];
    snoid = (Snoid *)&findView(hexCells[b].snoid)->body;
    otherHair = snoid->features[0];
    otherEyes = snoid->features[1];
    otherNose = snoid->features[2];
    otherFeet = snoid->features[3];
    r = randomUpTo(1000);
    if (r < 250) {
        if (hair == otherHair)
            return 510;
        if (eyes == otherEyes)
            return 511;
        if (nose == otherNose)
            return 512;
        if (feet == otherFeet)
            return 513;
    } else if (r < 500) {
        if (eyes == otherEyes)
            return 511;
        if (nose == otherNose)
            return 512;
        if (feet == otherFeet)
            return 513;
        if (hair == otherHair)
            return 510;
    } else if (r < 750) {
        if (nose == otherNose)
            return 512;
        if (feet == otherFeet)
            return 513;
        if (hair == otherHair)
            return 510;
        if (eyes == otherEyes)
            return 511;
    } else {
        if (feet == otherFeet)
            return 513;
        if (hair == otherHair)
            return 510;
        if (eyes == otherEyes)
            return 511;
        if (nose == otherNose)
            return 512;
    }
    return 0;
}

/* A cell's placed callback: keeps images 4, 8 and 24 only where the cell
   has that link (cellLinkBits), 73-76 only on a cell whose Zoombini field holds
   the matching shared feature (513, 510, 512, 511), 103 on cells in state
   506, and 109 and 110 as placeCellImages does. */
/* @zoombi32 0x004489ce */
void placeCellViewImages(View *view)
{
    ViewCel *cel;
    short removed;
    short cell;

    cell = view->id - hexCells[0].view;
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
        switch (cel->image) {
        case 4:
            if (!(cellLinkBits[cell] & 1)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 8:
            if (!(cellLinkBits[cell] & 2)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 24:
            if (!(cellLinkBits[cell] & 0x20)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 74:
            if (hexCells[cell].snoid != 510) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 76:
            if (hexCells[cell].snoid != 511) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 75:
            if (hexCells[cell].snoid != 512) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 73:
            if (hexCells[cell].snoid != 513) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 109:
            if (!((hexCells[cell].state == 502 || hexCells[cell].state == 504 || hexCells[cell].state == 508)
                  && startState != 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 110:
            if (!((hexCells[cell].state == 502 || hexCells[cell].state == 505 || hexCells[cell].state == 508)
                  && startState == 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 103:
            if (hexCells[cell].state != 506) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        }
        if (!removed)
            cel++;
    }
}

/* Puts the next Zoombini of the party (from the end of waitingSnoids) two cells
   from `cell` in direction `dir` (0-5 straight on, 6-9 turning), if it
   shares no feature with the Zoombini on `cell` among the features tried
   (from a random one): the cell between records the feature it was tried
   on (state 501, 510-513) and the far cell takes it (state 507). Returns
   its place in waitingSnoids, or -1. */
/* @zoombi32 0x00449cfc */
short placeUnalike(short cell, short dir)
{
    short via;
    short to;
    short feet;
    short hair;
    short eyes;
    short nose;
    short otherFeet;
    short otherEyes;
    short otherNose;
    Snoid *snoid;
    short otherHair;
    short feature;
    short different;
    short tries;

    feature = randomUpTo(3);
    if (dir <= 5) {
        via = hexCells[cell].links[dir];
        to = hexCells[via].links[dir];
    } else {
        switch (dir) {
        case 6:
            via = hexCells[cell].links[0];
            to = hexCells[via].links[1];
            break;
        case 7:
            via = hexCells[cell].links[2];
            to = hexCells[via].links[1];
            break;
        case 8:
            via = hexCells[cell].links[5];
            to = hexCells[via].links[4];
            break;
        case 9:
            via = hexCells[cell].links[3];
            to = hexCells[via].links[4];
            break;
        }
    }
    snoid = (Snoid *)&findView(hexCells[cell].snoid)->body;
    hair = snoid->features[0];
    eyes = snoid->features[1];
    nose = snoid->features[2];
    feet = snoid->features[3];
    different = 1;
    for (dir = partySize - 1; dir >= 0; dir--) {
        if (waitingSnoids[dir] == -1)
            continue;
        snoid = (Snoid *)&findView(partyViews[waitingSnoids[dir]])->body;
        otherHair = snoid->features[0];
        otherEyes = snoid->features[1];
        otherNose = snoid->features[2];
        otherFeet = snoid->features[3];
        tries = 4;
        do {
            if (feature == 0 && otherHair == hair)
                different = 0;
            if (feature == 1 && eyes == otherEyes)
                different = 0;
            if (feature == 2 && nose == otherNose)
                different = 0;
            if (feature == 3 && feet == otherFeet)
                different = 0;
            if (different) {
                tries--;
                if (++feature > 3)
                    feature = 0;
            }
        } while (different && tries);
        if (!different) {
            hexCells[to].state = 507;
            hexCells[to].snoid = partyViews[waitingSnoids[dir]];
            hexCells[via].state = 501;
            hexCells[via].snoid = feature + 510;
            waitingSnoids[dir] = -1;
            return dir;
        }
    }
    return -1;
}

/* Scene 12's frame: leaves after a choice (sceneDue) once the sound and the
   Zoombinis are done; cycles colours (cyclingColours) every 6 ticks; when the
   group finishGroup has arrived, sends the Zoombinis on the finished cells
   off (by the level, stoneRiseLevel) and ends; and has an idle Zoombini fidget
   now and then while slidesMoves. */
/* @zoombi32 0x00447171 */
void stoneRiseFrame()
{
    View *view;
    short done;
    short tries;

    if (!inStoneRiseFrame && stoneRiseOpen) {
        inStoneRiseFrame = 1;
        updateViews();
        if (sceneDue) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                inStoneRiseFrame = 0;
                return;
            }
            if (!dialogQuestion || dialogQuestion == 3) {
                if (dialogQuestion == 3)
                    chooseSnoids(0, 0);
                if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                    pendingScene = sceneDue;
                    sceneDue = 0;
                    setCurrentMap(0);
                    closeStoneRise();
                    inStoneRiseFrame = 0;
                    return;
                }
            } else if (dialogQuestion == 2) {
                dialogQuestion = 0;
                sceneDue = 0;
            }
        }
        if (cyclingColours && clockTime() - lastColourCycle > 6) {
            cycleColors();
            lastColourCycle = clockTime();
        }
        if (finishGroup && !groupLeader[finishGroup]) {
            queueViewSound(7001, 0);
            updateViews();
            waitForEventFor(0, 60, 0, 1);
            queueViewSound(996, 0);
            finishGroup = 0;
            if (stoneRiseLevel == 3) {
                chooseSnoids(0, 0);
                if (hexCells[55].state == 508)
                    ((Snoid *)&findView(hexCells[55].snoid)->body)->chosen = 1;
                if (hexCells[38].state == 508 && hexCells[46].state == 502)
                    ((Snoid *)&findView(hexCells[38].snoid)->body)->chosen = 1;
                if (hexCells[74].state == 508 && hexCells[64].state == 502)
                    ((Snoid *)&findView(hexCells[74].snoid)->body)->chosen = 1;
                sendSnoids(800, 200, 45);
                markLitSnoids();
            } else if (stoneRiseLevel <= 1) {
                sendSnoids(1280, 240, 45);
            } else {
                chooseSnoids(0, 0);
                if (hexCells[19].state == 508)
                    ((Snoid *)&findView(hexCells[19].snoid)->body)->chosen = 1;
                if (hexCells[55].state == 508)
                    ((Snoid *)&findView(hexCells[55].snoid)->body)->chosen = 1;
                if (hexCells[91].state == 508)
                    ((Snoid *)&findView(hexCells[91].snoid)->body)->chosen = 1;
                sendSnoids(800, 200, 45);
                markLitSnoids();
            }
            sceneDue = 5;
        }
        if (!slidesFidgetStarted && slidesMoves && slidesFidgets < slidesFidgetsAllowed) {
            slidesFidgetStarted++;
            if (clockTime() - lastSlidesFidgetTime > 30) {
                done = 0;
                tries = 0;
                lastSlidesFidgetTime = clockTime();
                do {
                    view = idleSnoidView(partyViews[allocateSlot(&slidesFidgetersUsed, partySize, 0)]);
                    if (view && view->body.running && view->flags == 1) {
                        int script = ((Snoid *)&view->body)->features[3] - 1;

                        script += 13001;
                        startSnoidScript((Snoid *)&view->body, script, 0, 0);
                        slidesFidgets++;
                        done = 1;
                    } else if (++tries > 20) {
                        done = 1;
                    }
                } while (!done);
            }
        } else if (slidesFidgets >= slidesFidgetsAllowed) {
            slidesFidgets = slidesMoves = lastSlidesFidgetTime = slidesFidgetersUsed = 0;
        }
        playAmbientSound();
        inStoneRiseFrame = 0;
    }
}

/* Pairs up the party by shared features (partyTaken marks those paired, 99
   those left alone; pairFeatures notes each pair's feature, 510-513, or 501):
   each tries the features in turn from a random one. Unless all are paired
   (or one, with an odd party), the lonely ones are swapped with the first
   and it tries again, up to ten rounds. */
/* @zoombi32 0x004494b3 */
void pairByFeatures()
{
    short feature;
    short lonely;
    short done;
    short rounds;
    short i;
    short j;
    short tries;
    short swap;
    short *paired = partyTaken;

    feature = randomUpTo(3);
    lonely = rounds = done = 0;
    do {
        fillMemory(pairFeatures, 0, 32);
        fillMemory(paired, 0, 32);
        groupCount = 0;
        for (i = 0; i < partySize; i++) {
            tries = 4;
            if (paired[i])
                continue;
            do {
                if (++feature > 3)
                    feature = 0;
                for (j = i + 1; j < partySize; j++) {
                    switch (feature) {
                    case 0:
                        if (!paired[j] && partyHair[i] == partyHair[j]) {
                            pairFeatures[groupCount] = 510;
                            groupCount++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 1:
                        if (!paired[j] && partyEyes[i] == partyEyes[j]) {
                            pairFeatures[groupCount] = 511;
                            groupCount++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 2:
                        if (!paired[j] && partyNoses[i] == partyNoses[j]) {
                            pairFeatures[groupCount] = 512;
                            groupCount++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 3:
                        if (!paired[j] && partyFeet[i] == partyFeet[j]) {
                            pairFeatures[groupCount] = 513;
                            groupCount++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    }
                }
                tries--;
                if (!tries && !paired[i]) {
                    paired[i] = 99;
                    pairFeatures[groupCount] = 501;
                    groupCount++;
                    lonely++;
                }
            } while (tries && !paired[i]);
        }
        if (!lonely) {
            done++;
        } else if (lonely == 1 && partySize % 2) {
            done++;
        } else {
            for (i = partySize - 1; i >= 0; i--) {
                if (paired[i] != 99)
                    continue;
                for (j = 0; j < partySize; j++) {
                    if (paired[j] == 99)
                        break;
                    swap = partyHair[i];
                    partyHair[i] = partyHair[j];
                    partyHair[j] = swap;
                    swap = partyEyes[i];
                    partyEyes[i] = partyEyes[j];
                    partyEyes[j] = swap;
                    swap = partyNoses[i];
                    partyNoses[i] = partyNoses[j];
                    partyNoses[j] = swap;
                    swap = partyFeet[i];
                    partyFeet[i] = partyFeet[j];
                    partyFeet[j] = swap;
                    break;
                }
            }
        }
        if (++rounds >= 10)
            done++;
    } while (!done);
}

/* A Zoombini's move from cell `from` over `via` to `to`: a plain stone
   between (state 501, below 510) lights (502) when the Zoombini stands on
   `from`, and so does `to` (502, or 508 with a Zoombini on it); a feature
   stone (510-513) lights, with `to` (508), when the Zoombinis on `from` and
   `to` share that feature; otherwise `to` lights alone (502). */
/* Not exact: register allocation (the original keeps `from` in eax, as in
   markCellAt; here it takes esi, which shifts the other variables' registers
   and stack slots). */
/* @zoombi32 0x0044a674 */
void tryMove(short from, short via, short to)
{
    short feet;
    short hair;
    short eyes;
    short nose;
    short code;
    short otherNose;
    View *view;
    short otherHair;
    short otherEyes;
    short otherFeet;
    Snoid *snoid;

    if (via == -1)
        return;
    if (hexCells[via].state != 501)
        return;
    code = hexCells[via].snoid;
    if (code < 510) {
        if (hexCells[from].state != 507 && hexCells[from].state != 508 && hexCells[from].state != 502)
            return;
        hexCells[via].state = 502;
        view = findView(hexCells[via].view);
        setViewScript(view, 7000, 1);
        view->placed = placeCellViewImages;
        if (to == -1)
            return;
        if (hexCells[to].state == 507 || hexCells[to].state == 508) {
            hexCells[to].state = 508;
            view = findView(hexCells[to].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
            return;
        }
        if (hexCells[to].state == 501) {
            hexCells[to].state = 502;
            view = findView(hexCells[to].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        }
        return;
    }
    if (to != -1 && (hexCells[from].state == 507 || hexCells[from].state == 508)
        && (hexCells[to].state == 507 || hexCells[to].state == 508)) {
        snoid = (Snoid *)&findView(hexCells[from].snoid)->body;
        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
        snoid = (Snoid *)&findView(hexCells[to].snoid)->body;
        otherHair = snoid->features[0];
        otherEyes = snoid->features[1];
        otherNose = snoid->features[2];
        otherFeet = snoid->features[3];
        if (code < 510 || hexCells[via].state == 502) {
            hexCells[to].state = 508;
            view = findView(hexCells[to].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
            hexCells[via].state = 502;
            view = findView(hexCells[via].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        } else if ((code == 510 && otherHair == hair) || (code == 511 && otherEyes == eyes)
                   || (code == 512 && nose == otherNose) || (code == 513 && otherFeet == feet)) {
            hexCells[to].state = 508;
            view = findView(hexCells[to].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
            hexCells[via].state = 502;
            view = findView(hexCells[via].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        }
    } else if (to != -1 && (hexCells[to].state == 502 || hexCells[to].state == 501)) {
        hexCells[to].state = 502;
        view = findView(hexCells[to].view);
        setViewScript(view, 7000, 1);
        view->placed = placeCellViewImages;
        if (code < 510 && code != 500) {
            hexCells[via].state = 502;
            view = findView(hexCells[via].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        }
    }
}

/* Seats the next two Zoombinis of the party (waitingSnoids) on `cell` and the
   cell five on (state 507), then from each places more two cells on
   (placeUnalike), turning (directions 8 and 9, 6 and 7). */
/* @zoombi32 0x0044abce */
void seatPair(short cell)
{
    short i;

    for (i = 0; i < partySize; i++)
        if (waitingSnoids[i] != -1) {
            hexCells[cell].state = 507;
            hexCells[cell].snoid = partyViews[waitingSnoids[i]];
            waitingSnoids[i] = -1;
            break;
        }
    for (i = 0; i < partySize; i++)
        if (waitingSnoids[i] != -1) {
            hexCells[cell + 5].state = 507;
            hexCells[cell + 5].snoid = partyViews[waitingSnoids[i]];
            waitingSnoids[i] = -1;
            break;
        }
    if (hexCells[cell].state == 507 && placeUnalike(cell, 8) != -1)
        placeUnalike(cell, 9);
    if (hexCells[cell + 5].state == 507 && placeUnalike(cell + 5, 6) != -1)
        placeUnalike(cell + 5, 7);
}

/* Lights the path: from the listed cell listedCells[pathStart] (taken, 507),
   finds the cell before the first in state startState, then walks back along
   the links: Zoombinis' cells go to 508; a feature stone (510-513) lights
   when the Zoombinis on either side share its feature; a plain stone
   lights on level 1 after a Zoombini's cell. The walk ends at an empty or
   blocked cell, or a stone that doesn't light. */
/* Not exact: register allocation (the original keeps `view` in esi and the
   board's address in edi, the other way round, and keeps the first
   findView's result). */
/* @zoombi32 0x00448f02 */
void lightPath()
{
    short done;
    short feet;
    short hair;
    short eyes;
    short nose;
    short otherNose;
    short back;
    short ahead;
    View *view;
    Snoid *snoid;
    short cell;
    short found;
    short start;
    short otherHair;
    short otherEyes;
    short otherFeet;

    cell = listedCells[pathStart];
    view = findView(hexCells[cell].snoid);
    hexCells[cell].state = 507;
    view = findView(hexCells[cell].view);
    setViewScript(view, 7000, 1);
    view->placed = placeCellViewImages;
    found = 0;
    do {
        start = cell;
        if (hexCells[cell].links[0] != -1)
            cell = hexCells[cell].links[0];
        else if (hexCells[cell].links[1] != -1)
            cell = hexCells[cell].links[1];
        else if (hexCells[cell].links[2] != -1)
            cell = hexCells[cell].links[2];
        if (hexCells[cell].state == startState) {
            found++;
            cell = start;
        }
    } while (!found);
    done = 0;
    do {
        switch (hexCells[cell].state) {
        case 500:
        case 506:
            done++;
            break;
        case 507:
            hexCells[cell].state = 508;
            view = findView(hexCells[cell].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
            break;
        case 501:
            if (hexCells[cell].snoid >= 510 && hexCells[cell].snoid <= 513) {
                if (hexCells[cell].links[5] != -1)
                    back = hexCells[cell].links[5];
                else if (hexCells[cell].links[4] != -1)
                    back = hexCells[cell].links[4];
                else if (hexCells[cell].links[3] != -1)
                    back = hexCells[cell].links[3];
                if (hexCells[back].state != 507 && hexCells[back].state != 508) {
                    done++;
                    break;
                }
                if (hexCells[cell].links[0] != -1)
                    ahead = hexCells[cell].links[0];
                else if (hexCells[cell].links[1] != -1)
                    ahead = hexCells[cell].links[1];
                else if (hexCells[cell].links[2] != -1)
                    ahead = hexCells[cell].links[2];
                if (hexCells[ahead].state != 507 && hexCells[ahead].state != 508) {
                    done++;
                    break;
                }
                view = findView(hexCells[ahead].snoid);
                snoid = (Snoid *)&view->body;
                hair = snoid->features[0];
                eyes = snoid->features[1];
                nose = snoid->features[2];
                feet = snoid->features[3];
                view = findView(hexCells[back].snoid);
                snoid = (Snoid *)&view->body;
                otherHair = snoid->features[0];
                otherEyes = snoid->features[1];
                otherNose = snoid->features[2];
                otherFeet = snoid->features[3];
                if (hexCells[cell].snoid == 510) {
                    if (otherHair == hair) {
                        hexCells[cell].state = 502;
                        view = findView(hexCells[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = placeCellViewImages;
                    } else {
                        done++;
                    }
                } else if (hexCells[cell].snoid == 511) {
                    if (otherEyes == eyes) {
                        hexCells[cell].state = 502;
                        view = findView(hexCells[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = placeCellViewImages;
                    } else {
                        done++;
                    }
                } else if (hexCells[cell].snoid == 512) {
                    if (nose == otherNose) {
                        hexCells[cell].state = 502;
                        view = findView(hexCells[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = placeCellViewImages;
                    } else {
                        done++;
                    }
                } else if (hexCells[cell].snoid == 513) {
                    if (otherFeet == feet) {
                        hexCells[cell].state = 502;
                        view = findView(hexCells[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = placeCellViewImages;
                    } else {
                        done++;
                    }
                }
                if (!done) {
                    hexCells[cell].state = 502;
                    view = findView(hexCells[cell].view);
                    setViewScript(view, 7000, 1);
                    view->placed = placeCellViewImages;
                }
            } else if (stoneRiseLevel == 1 && !hexCells[cell].snoid && hexCells[cell - 1].state == 508) {
                hexCells[cell].state = 502;
                view = findView(hexCells[cell].view);
                setViewScript(view, 7000, 1);
                view->placed = placeCellViewImages;
            }
            break;
        }
        if (!done) {
            if (hexCells[cell].links[5] != -1)
                cell = hexCells[cell].links[5];
            else if (hexCells[cell].links[4] != -1)
                cell = hexCells[cell].links[4];
            else if (hexCells[cell].links[3] != -1)
                cell = hexCells[cell].links[3];
            else
                done++;
        }
    } while (!done);
}

/* Links the board's cells to their neighbours (links[0-5], -1 for none),
   where each cell's bits in cellLinkBits allow: the board is 13 rows of 9,
   odd rows set half a cell right, so a neighbour's number depends on the
   row and on the edges. Empty cells (state 0) get no links. */
/* @zoombi32 0x0044833d */
void linkCells()
{
    short top;
    short bottom;
    short leftEven;
    short rightEven;
    short leftOdd;
    short i;
    short rightOdd;
    short cell;

    for (cell = 0; cell < 117; cell++) {
        for (i = 0; i < 6; i++)
            hexCells[cell].links[i] = -1;
        top = bottom = leftEven = rightOdd = leftOdd = rightEven = 0;
        if (cell <= 8)
            top++;
        else if (cell >= 108)
            bottom++;
        else if (cell % 9 == 0) {
            if (cell % 18 == 0)
                leftEven++;
            else
                leftOdd++;
        } else if (cell % 9 == 8) {
            if (cell % 18 == 8)
                rightEven++;
            else
                rightOdd++;
        }
        if (!hexCells[cell].state)
            continue;
        if (top) {
            if (cell == 0) {
                if (cellLinkBits[cell] & 0x10)
                    hexCells[cell].links[4] = 1;
                if (cellLinkBits[cell] & 8)
                    hexCells[cell].links[3] = 9;
            }
            if (cell == 8) {
                if (cellLinkBits[cell] & 2)
                    hexCells[cell].links[1] = 7;
                if (cellLinkBits[cell] & 4)
                    hexCells[cell].links[2] = 16;
                if (cellLinkBits[cell] & 8)
                    hexCells[cell].links[3] = 17;
            } else {
                if (cellLinkBits[cell] & 2)
                    hexCells[cell].links[1] = cell - 1;
                if (cellLinkBits[cell] & 4)
                    hexCells[cell].links[2] = cell + 8;
                if (cellLinkBits[cell] & 8)
                    hexCells[cell].links[3] = cell + 9;
                if (cellLinkBits[cell] & 0x10)
                    hexCells[cell].links[4] = cell + 1;
            }
        } else if (bottom) {
            if (cell == 108) {
                if (cellLinkBits[cell] & 0x10)
                    hexCells[cell].links[4] = 109;
                if (cellLinkBits[cell] & 0x20)
                    hexCells[cell].links[5] = 99;
            } else if (cell == 116) {
                if (cellLinkBits[cell] & 1)
                    hexCells[cell].links[0] = 106;
                if (cellLinkBits[cell] & 2)
                    hexCells[cell].links[1] = 115;
                if (cellLinkBits[cell] & 0x20)
                    hexCells[cell].links[5] = 107;
            } else {
                if (cellLinkBits[cell] & 1)
                    hexCells[cell].links[0] = cell - 10;
                if (cellLinkBits[cell] & 2)
                    hexCells[cell].links[1] = cell - 1;
                if (cellLinkBits[cell] & 0x10)
                    hexCells[cell].links[4] = cell + 1;
                if (cellLinkBits[cell] & 0x20)
                    hexCells[cell].links[5] = cell - 9;
            }
        } else if (rightOdd) {
            if (cellLinkBits[cell] & 1)
                hexCells[cell].links[0] = cell - 9;
            if (cellLinkBits[cell] & 2)
                hexCells[cell].links[1] = cell - 1;
            if (cellLinkBits[cell] & 4)
                hexCells[cell].links[2] = cell + 9;
        } else if (rightEven) {
            if (cellLinkBits[cell] & 1)
                hexCells[cell].links[0] = cell - 10;
            if (cellLinkBits[cell] & 2)
                hexCells[cell].links[1] = cell - 1;
            if (cellLinkBits[cell] & 4)
                hexCells[cell].links[2] = cell + 8;
            if (cellLinkBits[cell] & 8)
                hexCells[cell].links[3] = cell + 9;
            if (cellLinkBits[cell] & 0x20)
                hexCells[cell].links[5] = cell - 9;
        } else if (leftEven) {
            if (cellLinkBits[cell] & 8)
                hexCells[cell].links[3] = cell + 9;
            if (cellLinkBits[cell] & 0x10)
                hexCells[cell].links[4] = cell + 1;
            if (cellLinkBits[cell] & 0x20)
                hexCells[cell].links[5] = cell - 9;
        } else if (leftOdd) {
            if (cellLinkBits[cell] & 1)
                hexCells[cell].links[0] = cell - 9;
            if (cellLinkBits[cell] & 4)
                hexCells[cell].links[2] = cell + 9;
            if (cellLinkBits[cell] & 8)
                hexCells[cell].links[3] = cell + 10;
            if (cellLinkBits[cell] & 0x10)
                hexCells[cell].links[4] = cell + 1;
            if (cellLinkBits[cell] & 0x20)
                hexCells[cell].links[5] = cell - 8;
        } else if (cell % 18 <= 8) {
            if (cellLinkBits[cell] & 1)
                hexCells[cell].links[0] = cell - 10;
            if (cellLinkBits[cell] & 2)
                hexCells[cell].links[1] = cell - 1;
            if (cellLinkBits[cell] & 4)
                hexCells[cell].links[2] = cell + 8;
            if (cellLinkBits[cell] & 8)
                hexCells[cell].links[3] = cell + 9;
            if (cellLinkBits[cell] & 0x10)
                hexCells[cell].links[4] = cell + 1;
            if (cellLinkBits[cell] & 0x20)
                hexCells[cell].links[5] = cell - 9;
        } else {
            if (cellLinkBits[cell] & 1)
                hexCells[cell].links[0] = cell - 9;
            if (cellLinkBits[cell] & 2)
                hexCells[cell].links[1] = cell - 1;
            if (cellLinkBits[cell] & 4)
                hexCells[cell].links[2] = cell + 9;
            if (cellLinkBits[cell] & 8)
                hexCells[cell].links[3] = cell + 10;
            if (cellLinkBits[cell] & 0x10)
                hexCells[cell].links[4] = cell + 1;
            if (cellLinkBits[cell] & 0x20)
                hexCells[cell].links[5] = cell - 8;
        }
    }
}

/* Whether party members `a` and `b` share a feature (sharedStone), by seating
   them on cells 1 and 3 for the moment. */
/* @zoombi32 0x0044b4ec */
short membersShare(short a, short b)
{
    short first;
    short third;
    short shared;

    first = hexCells[1].state;
    third = hexCells[3].state;
    hexCells[1].snoid = partyViews[a];
    hexCells[3].snoid = partyViews[b];
    hexCells[1].state = 506;
    hexCells[3].state = 506;
    shared = sharedStone(1, 3);
    hexCells[1].snoid = 0;
    hexCells[3].snoid = 0;
    hexCells[1].state = first;
    hexCells[3].state = third;
    return shared;
}

/* Tries the moves from `cell` over each neighbour (tryMove) in
   directions 4, 1 and 5, then from a lit cell reached in direction 5
   directions 3 and 0; then 3, and from there 5 and 2; then 0 and 2. */
/* @zoombi32 0x0044a4d9 */
void tryMovesAround(short cell)
{
    short middle;
    short via;
    short to;

    via = hexCells[cell].links[4];
    to = hexCells[via].links[4];
    tryMove(cell, via, to);
    via = hexCells[cell].links[1];
    to = hexCells[via].links[1];
    tryMove(cell, via, to);
    via = hexCells[cell].links[5];
    to = hexCells[via].links[5];
    tryMove(cell, via, to);
    if (hexCells[to].state == 508 || hexCells[to].state == 502) {
        middle = to;
        via = hexCells[middle].links[3];
        to = hexCells[via].links[3];
        tryMove(middle, via, to);
        via = hexCells[middle].links[0];
        to = hexCells[via].links[0];
        tryMove(middle, via, to);
    }
    via = hexCells[cell].links[3];
    to = hexCells[via].links[3];
    tryMove(cell, via, to);
    if (hexCells[to].state == 508 || hexCells[to].state == 502) {
        middle = to;
        via = hexCells[middle].links[5];
        to = hexCells[via].links[5];
        tryMove(middle, via, to);
        via = hexCells[middle].links[2];
        to = hexCells[via].links[2];
        tryMove(middle, via, to);
    }
    via = hexCells[cell].links[0];
    to = hexCells[via].links[0];
    tryMove(cell, via, to);
    via = hexCells[cell].links[2];
    to = hexCells[via].links[2];
    tryMove(cell, via, to);
}

/* Follows the moves (tryMove) from `cell` along two winding routes (from
   directions 5 and 3), each step taken only while the last landed on a lit
   cell (508 or 502). */
/* @zoombi32 0x0044accc */
void followRoutes(short cell)
{
    short middle;
    short via;
    short to;

    via = hexCells[cell].links[5];
    to = hexCells[via].links[4];
    tryMove(cell, via, to);
    if (hexCells[to].state == 508 || hexCells[to].state == 502) {
        middle = to;
        via = hexCells[middle].links[4];
        to = hexCells[via].links[4];
        tryMove(middle, via, to);
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[4];
            to = hexCells[via].links[3];
            tryMove(middle, via, to);
        }
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[2];
            to = hexCells[via].links[1];
            tryMove(middle, via, to);
        }
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[1];
            to = hexCells[via].links[1];
            tryMove(middle, via, to);
        }
    }
    via = hexCells[cell].links[3];
    to = hexCells[via].links[4];
    tryMove(cell, via, to);
    if (hexCells[to].state == 508 || hexCells[to].state == 502) {
        middle = to;
        via = hexCells[middle].links[4];
        to = hexCells[via].links[4];
        tryMove(middle, via, to);
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[4];
            to = hexCells[via].links[5];
            tryMove(middle, via, to);
        }
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[0];
            to = hexCells[via].links[1];
            tryMove(middle, via, to);
        }
        if (hexCells[to].state == 508 || hexCells[to].state == 502) {
            middle = to;
            via = hexCells[middle].links[1];
            to = hexCells[via].links[1];
            tryMove(middle, via, to);
        }
    }
}

/* Lights the three starting cells (19, 55, 91) where a Zoombini waits (507
   to 508) and follows the moves from each (followRoutes), then checkAllFilled. */
/* @zoombi32 0x0044a359 */
void lightFromStarts()
{
    View *view;

    if (hexCells[19].state == 507) {
        hexCells[19].state = 508;
        view = findView(hexCells[19].view);
        setViewScript(view, 7000, 1);
        view->placed = placeCellViewImages;
        followRoutes(19);
    }
    if (hexCells[55].state == 507) {
        hexCells[55].state = 508;
        view = findView(hexCells[55].view);
        setViewScript(view, 7000, 1);
        view->placed = placeCellViewImages;
        followRoutes(55);
    }
    if (hexCells[91].state == 507) {
        hexCells[91].state = 508;
        view = findView(hexCells[91].view);
        setViewScript(view, 7000, 1);
        view->placed = placeCellViewImages;
        followRoutes(91);
    }
    checkAllFilled();
}

/* Lights the cell after `cell` if a Zoombini waits there, then every cell
   of lightCells with a Zoombini next to a lit stone, and follows the moves
   around each lit one (tryMovesAround); on level 3, with cells 57, 59 and 61
   lit and four Zoombinis on the listed cells, the colours start cycling
   (cyclingColours) and showZoneMessage follows. */
/* @zoombi32 0x0044a180 */
void lightFrom(short cell)
{
    View *view;
    short i;
    short j;
    short next;
    short after;

    after = cell + 1;
    if (hexCells[after].state != 507)
        return;
    hexCells[after].state = 508;
    view = findView(hexCells[after].view);
    setViewScript(view, 7000, 1);
    view->placed = placeCellViewImages;
    for (i = 0; i < lightCellCount; i++) {
        if (hexCells[lightCells[i]].state == 507) {
            for (j = 0; j <= 5; j++) {
                next = hexCells[lightCells[i]].links[j];
                if (next != -1 && hexCells[next].state == 502) {
                    hexCells[lightCells[i]].state = 508;
                    break;
                }
            }
        }
        if (hexCells[lightCells[i]].state == 502 || hexCells[lightCells[i]].state == 508)
            tryMovesAround(lightCells[i]);
    }
    checkAllFilled();
    if (!cyclingColours && stoneRiseLevel == 3 && hexCells[57].state == 508 && hexCells[59].state == 508
        && hexCells[61].state == 508) {
        cyclingColours = 0;
        for (i = 1; i <= listedCount; i++)
            if (hexCells[listedCells[i]].state == 507 || hexCells[listedCells[i]].state == 508)
                cyclingColours++;
        if (cyclingColours == 4) {
            cyclingColours = 1;
            lastColourCycle = clockTime();
            showZoneMessage();
        } else {
            cyclingColours = 0;
        }
    }
}

/* Turns every lit cell back (508 to 507, 502 to 501) and lights the path
   again from the start (cell 54 on level 3, lightFrom; else lightFromStarts). */
/* @zoombi32 0x0044a422 */
void relightPath()
{
    View *view;
    short i;

    for (i = 0; i < 117; i++) {
        if (hexCells[i].state == 508) {
            hexCells[i].state = 507;
            view = findView(hexCells[i].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        } else if (hexCells[i].state == 502) {
            hexCells[i].state = 501;
            view = findView(hexCells[i].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        }
    }
    if (stoneRiseLevel == 3)
        lightFrom(54);
    else
        lightFromStarts();
}

/* Scene 12's keys (with debugging on, debugMessagesOn, or else only 0x16f): typing
   "solve" (solveTyped counts the letters) solves level 3. */
/* @zoombi32 0x00448231 */
short stoneRiseKey(unsigned short key)
{
    if (!debugMessagesOn && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        replayHint();
        return 1;
    case 's':
        if (!solveTyped)
            solveTyped = 1;
        return 1;
    case 'o':
        if (solveTyped == 1)
            solveTyped = 2;
        return 1;
    case 'v':
        if (solveTyped == 2)
            solveTyped = 3;
        return 1;
    case 'l':
        if (solveTyped == 3)
            solveTyped = 4;
        return 1;
    case 'e':
        if (solveTyped == 4) {
            solveTyped = 5;
            if (stoneRiseLevel == 3) {
                standPlacedSnoids();
                relightPath();
                slidesGoReady = 1;
                slidesMoves++;
            }
            return 1;
        }
        return 1;
    }
    return 0;
}

/* Scene 12's clicks: button 1 asks whether to keep the party; button 2
   (once there's something to check) shows the answer: the cells lit and
   the Zoombinis on them cheer, and the group starts across; otherwise a
   Zoombini is dragged onto a listed cell (lighting the path, lightPath) or
   off it (unlighting what it lit), by the level's rules, and a Zoombini
   dropped elsewhere walks to the marked spot (markCellAt). */
/* @zoombi32 0x00447528 */
void stoneRiseClicked(short which)
{
    View *view;
    ShortRect unused1; /* never used: the original's frame has room for two */
    ShortRect unused2;
    Point where;
    short i;
    short first;
    View *cellView;
    Snoid *snoid;
    short dropped;
    short place;
    short moved;
    short x;
    short y;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeStoneRise();
        return;
    }
    if (savedPartyFlags != -1) {
        for (i = 0; i < partySize; i++) {
            cellView = findView(partyViews[i]);
            cellView->flags = savedPartyFlags;
        }
        savedPartyFlags = -1;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawSlidesButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawSlidesButton(which, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (!slidesGoReady || slidesGoPressed)
            break;
        if (slidesMoves)
            standFilledCells();
        drawSlidesButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawSlidesButton(which, 0, 1);
        slidesGoPressed++;
        first = 1;
        markLitSnoids();
        updateCellLinks();
        for (i = 0; i < 117; i++) {
            if (hexCells[i].state == 502 || hexCells[i].state == 508 || hexCells[i].state == startState) {
                cellView = findView(hexCells[i].view);
                setViewScript(cellView, 7002, 1);
                cellView->placed = placeCellImages;
                if (first) {
                    finishGroup = groupViews(hexCells[i].view, hexCells[i].view, 0, 0, 0, 0);
                    first = 0;
                }
            }
            if (hexCells[i].state == 508) {
                cellView = findView(hexCells[i].snoid);
                startSnoidScript((Snoid *)&cellView->body, 13000, 0, 0);
                groupViews(hexCells[i].snoid, hexCells[i].view, 0, 0, 0, 0);
            }
        }
        slidesDragLocked++;
        queueViewSound(7000, 0);
        moveView(slidesRowViews[1], 0, hexCells[9].view);
        moveView(slidesRowViews[2], 0, hexCells[27].view);
        moveView(slidesRowViews[3], 0, hexCells[45].view);
        moveView(slidesRowViews[4], 0, hexCells[63].view);
        moveView(slidesRowViews[5], 0, hexCells[81].view);
        moveView(slidesRowViews[6], 0, hexCells[99].view);
        layerSnoidViews();
        break;
    case 3:
        if (slidesDragLocked)
            break;
        if (slidesMoves) {
            standFilledCells();
            break;
        }
        if (snoidsOnTheirWay > 0 || cellMarked || finishGroup)
            break;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (view)
            view = idleSnoidView(view->id);
        lastLitCount = countLitCells();
        lastLitSum = litSum;
        if (stoneRiseLevel <= 1) {
            if (!view)
                break;
            dropped = dragSnoid(view, where, 0, 0);
            place = heldPlaceNumber();
            snoid = (Snoid *)&view->body;
            moved = snoid->targetX != snoid->body.x || snoid->targetY != snoid->body.y;
            if (!place && dropped && moved)
                pickSlidesPlace((Point *)&snoid->targetX);
            pathStart = place;
            if (pathStart) {
                for (i = 1; i <= listedCount; i++) {
                    if (hexCells[listedCells[i]].snoid == view->id && i != pathStart) {
                        hexCells[listedCells[i]].snoid = 0;
                        hexCells[listedCells[i]].state = 506;
                        cellView = findView(hexCells[listedCells[i]].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                        if (hexCells[listedCells[i] - 1].state == 502) {
                            hexCells[listedCells[i] - 1].state = 501;
                            cellView = findView(hexCells[listedCells[i] - 1].view);
                            setViewScript(cellView, 7000, 1);
                            cellView->placed = placeCellViewImages;
                        }
                        for (dropped = 1; dropped < 6; dropped++) {
                            if (hexCells[listedCells[i] + dropped].state == 500)
                                break;
                            if (hexCells[listedCells[i] + dropped].state == 502
                                || hexCells[listedCells[i] + dropped].state == 508) {
                                if (hexCells[listedCells[i] + dropped].state == 508)
                                    hexCells[listedCells[i] + dropped].state = 507;
                                else
                                    hexCells[listedCells[i] + dropped].state = 501;
                                cellView = findView(hexCells[listedCells[i] + dropped].view);
                                setViewScript(cellView, 7000, 1);
                                cellView->placed = placeCellViewImages;
                            }
                        }
                        break;
                    }
                }
                hexCells[listedCells[pathStart]].snoid = view->id;
                lightPath();
                remarkOnLit();
                checkAllFilled();
            } else {
                for (i = 1; i <= listedCount; i++) {
                    if (hexCells[listedCells[i]].snoid == view->id) {
                        hexCells[listedCells[i]].snoid = 0;
                        hexCells[listedCells[i]].state = 506;
                        cellView = findView(hexCells[listedCells[i]].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                        if (hexCells[listedCells[i] - 1].state == 502) {
                            hexCells[listedCells[i] - 1].state = 501;
                            cellView = findView(hexCells[listedCells[i] - 1].view);
                            setViewScript(cellView, 7000, 1);
                            cellView->placed = placeCellViewImages;
                        }
                        for (dropped = 1; dropped < 6; dropped++) {
                            if (hexCells[listedCells[i] + dropped].state == 500)
                                break;
                            if (hexCells[listedCells[i] + dropped].state == 502
                                || hexCells[listedCells[i] + dropped].state == 508) {
                                if (hexCells[listedCells[i] + dropped].state == 508)
                                    hexCells[listedCells[i] + dropped].state = 507;
                                else
                                    hexCells[listedCells[i] + dropped].state = 501;
                                cellView = findView(hexCells[listedCells[i] + dropped].view);
                                setViewScript(cellView, 7000, 1);
                                cellView->placed = placeCellViewImages;
                            }
                        }
                    }
                }
                x = view->body.x;
                y = view->body.y;
                markWalker = view->id;
                markCellAt(x, y);
                remarkOnLit();
            }
        } else if (stoneRiseLevel == 2) {
            if (!view)
                break;
            dropped = dragSnoid(view, where, 0, 0);
            place = heldPlaceNumber();
            snoid = (Snoid *)&view->body;
            moved = snoid->targetX != snoid->body.x || snoid->targetY != snoid->body.y;
            if (!place && dropped && moved)
                pickSlidesPlace((Point *)&snoid->targetX);
            pathStart = place;
            if (pathStart) {
                for (i = 1; i <= listedCount; i++)
                    if (hexCells[listedCells[i]].snoid == view->id && i != pathStart) {
                        hexCells[listedCells[i]].snoid = 0;
                        hexCells[listedCells[i]].state = 506;
                        cellView = findView(hexCells[listedCells[i]].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                    }
                hexCells[listedCells[pathStart]].snoid = view->id;
                hexCells[listedCells[pathStart]].state = 507;
                relightPath();
                remarkOnLit();
            } else {
                for (i = 1; i < 117; i++)
                    if ((hexCells[i].state == 507 || hexCells[i].state == 508) && hexCells[i].snoid == view->id) {
                        hexCells[i].state = 506;
                        hexCells[i].snoid = -1;
                        cellView = findView(hexCells[i].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                        relightPath();
                        break;
                    }
                x = view->body.x;
                y = view->body.y;
                markWalker = view->id;
                markCellAt(x, y);
                remarkOnLit();
            }
        } else {
            if (!view)
                break;
            dropped = dragSnoid(view, where, 0, 0);
            place = heldPlaceNumber();
            snoid = (Snoid *)&view->body;
            moved = snoid->targetX != snoid->body.x || snoid->targetY != snoid->body.y;
            if (!place && dropped && moved)
                pickSlidesPlace((Point *)&snoid->targetX);
            pathStart = place;
            if (pathStart) {
                for (i = 1; i <= listedCount; i++)
                    if (hexCells[listedCells[i]].snoid == view->id && i != pathStart) {
                        hexCells[listedCells[i]].snoid = 0;
                        hexCells[listedCells[i]].state = 506;
                        cellView = findView(hexCells[listedCells[i]].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                    }
                hexCells[listedCells[pathStart]].snoid = view->id;
                hexCells[listedCells[pathStart]].state = 507;
                relightPath();
                remarkOnLit();
            } else {
                for (i = 1; i < 117; i++)
                    if ((hexCells[i].state == 507 || hexCells[i].state == 508) && hexCells[i].snoid == view->id) {
                        hexCells[i].state = 506;
                        hexCells[i].snoid = -1;
                        cellView = findView(hexCells[i].view);
                        setViewScript(cellView, 7000, 1);
                        cellView->placed = placeCellViewImages;
                        relightPath();
                        break;
                    }
                x = view->body.x;
                y = view->body.y;
                markWalker = view->id;
                markCellAt(x, y);
                remarkOnLit();
            }
        }
        break;
    }
    noteAnyLit();
}
