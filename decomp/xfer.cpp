/*
 * xfer (0x4696f0-0x46be28): 'xfer.MHK'
 */

#include <stdio.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "loading.h"
#include "mainloop.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"
#include "xfer.h"

short journeyRoute = 0;
InputItem g_4a7e6a[1] = {{{0, 0, 640, 480}}};
Group g_4a7e8e[1] = {{g_4a0766, g_4a7e6a, 1, 0x2068}};
GroupList xferGroups[1] = {{g_4a7e8e, 1, 0, journeyClicked}};
Scene g_4a7eaa[1] = {{openJourney, closeJourney, journeyFrame, 0, journeyKey}};
ShortRect mapTitleRects[4] = {
    {43, 54, 226, 107}, {371, 33, 613, 65}, {127, 29, 299, 81}, {135, 29, 323, 82},
};
short inJourneyFrame = 0;
short mapPlaces[4][5] = {
    {0, 1, 2, 3, 4}, {4, 5, 6, 7, 11}, {4, 8, 9, 10, 16}, {11, 12, 13, 14, 15},
};
Point gridStarts[16] = {
    {3, 105}, {130, 146}, {1, 2}, {6, 60}, {42, 194}, {1, 106}, {1, 1}, {1, 4}, {1, 1}, {1, 1},
    {1, 53}, {102, 162}, {1, 12}, {57, 154}, {1, 1}, {2, 109},
};

long journeyFile;
short journeyOpen;
short shownPopulation;
short placeLevelShown;
long nextJourneyMoveTime;
char placeLevels[17];
short view5108;
short views5104[4];
short views5102[2];
short pendingJourneyFacing;
short journeyAnchorView;
short snoidsPastAnchor;
short view6108;
short views5102Due[3];
short journeyFirstMoveDone;
short xferSound;
short xferMap;
short nextWalker;
short journeyPartySize;
short destinationPlace;
short destinationLevel;
short gridView;
short view6106;
short view6107;
short populationSignView;
short destinationImage;
unsigned long gridProgress;
unsigned long gridCellsTotal;
unsigned long gridCellsLeft;
unsigned long gridStride;
unsigned long gridRows;
unsigned long gridColumns;
Point gridMarks[24];
char gridMarkUsed[24];
char *gridCells;
char gridFree1;
char gridTaken1;
char gridFree2;
char gridTaken2;

/* Resets scene 2's state. */
/* @zoombi32 0x004696f0 */
void resetJourney()
{
    short i;

    xferSound = sceneDue = 0;
    pendingJourneyFacing = journeyAnchorView = snoidsPastAnchor = view6108 = 0;
    view6106 = view6107 = populationSignView = gridView = destinationImage = 0;
    journeyFirstMoveDone = 0;
    gridProgress = 0;
    for (i = 0; i < 3; i++)
        views5102Due[i] = 1;
    for (i = 0; i < 4; i++)
        views5104[i] = 0;
    for (i = 0; i < 2; i++)
        views5102[i] = 0;
    for (i = 0; i < 17; i++)
        placeLevels[i] = 0;
    view5108 = destinationPlace = destinationPlace = 0;
    snoidIdleDelay = 0;
    xferMap = 0;
    shownPopulation = population();
    placeLevelShown = -1;
    nextWalker = journeyPartySize = 0;
    nextJourneyMoveTime = 0;
}

/* @zoombi32 0x0046b07b */
short journeyKey(unsigned short)
{
    return 0;
}

/* Fills `levels` (17) with the highest level (1-4) each place has
   reached, by the game state's bits (or all practiceLevel); then notes the
   place of scene journeyTo in destinationPlace, and its level in destinationLevel. */
/* @zoombi32 0x0046b084 */
void readPlaceLevels(char *levels)
{
    short i;
    short bits;
    short value;
    short saved;

    for (i = 0; i <= 16; i++) {
        if (practiceLevel)
            value = practiceLevel;
        else {
            value = bits = 0;
            switch (i) {
            case 0:
                bits = 1;
                break;
            case 1:
            case 2:
            case 3:
                bits = gameState[0x55 + i] & 0xf;
                break;
            case 4:
                bits = gameState[0x50] & 0xf;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                bits = gameState[0x54 + i] & 0xf;
                break;
            case 11:
                bits = *(short *)(gameState + 0x52) & 0xf;
                break;
            case 12:
            case 13:
            case 14:
                bits = gameState[0x53 + i] & 0xf;
                break;
            case 15:
                bits = gameState[0x51] & 0xf;
                break;
            case 16:
                bits = *(short *)(gameState + 0x52) & 0xf0;
                bits = bits >> 4;
                break;
            }
            if (bits & 1)
                value = 1;
            if (bits & 2)
                value = 2;
            if (bits & 4)
                value = 3;
            if (bits & 8)
                value = 4;
        }
        levels[i] = value;
    }
    destinationPlace = i = 0;
    saved = currentScene;
    currentScene = journeyTo;
    value = sceneLevel() + 1;
    currentScene = saved;
    switch (journeyTo) {
    case 7:
        i = 1;
        bits = value;
        break;
    case 8:
        i = 2;
        bits = levels[1];
        break;
    case 9:
        i = 3;
        bits = levels[2];
        break;
    case 4:
        i = 4;
        bits = levels[3];
        break;
    case 10:
        i = 5;
        bits = value;
        break;
    case 11:
        i = 6;
        bits = levels[5];
        break;
    case 12:
        i = 7;
        bits = levels[6];
        break;
    case 5:
        if (journeyFrom == 12) {
            i = 11;
            bits = levels[7];
        } else {
            i = 16;
            bits = levels[10];
        }
        break;
    case 13:
        i = 8;
        bits = value;
        break;
    case 14:
        i = 9;
        bits = levels[8];
        break;
    case 15:
        i = 10;
        bits = levels[9];
        break;
    case 16:
        i = 12;
        bits = value;
        break;
    case 17:
        i = 13;
        bits = levels[12];
        break;
    case 18:
        i = 14;
        bits = levels[13];
        break;
    case 6:
        i = 15;
        bits = levels[14];
        break;
    }
    if (i) {
        destinationPlace = i;
        destinationLevel = bits;
        levels[i] = bits - 1;
        if (levels[i] < 1) {
            if (placeLevelShown < 0)
                placeLevelShown = bits - 1;
            levels[i] = -1;
        }
    }
}

/* The map view's placed callback (map xferMap, 1-4): picks each cel's
   image by the places' levels (placeLevels): the first ones show the places
   reached, the rest each place's level. Notes the image of place destinationPlace
   in destinationImage (1-4). */
/* @zoombi32 0x0046b326 */
void placeMapImages(View *view)
{
    short images[10];
    short last;
    short base;
    short found = 0;
    short count;
    short i;
    short place;
    ViewCel *cel;

    switch (xferMap) {
    case 1:
        base = 0;
        count = 5;
        last = 8;
        break;
    case 2:
        base = 5;
        count = 6;
        last = 9;
        break;
    case 3:
        base = 10;
        count = 6;
        last = 9;
        break;
    case 4:
        base = 15;
        count = 6;
        last = 9;
        break;
    default:
        return;
    }
    images[0] = 0;
    for (i = 1; i <= last; i++) {
        images[i] = 0;
        if (i >= count) {
            place = mapPlaces[0][base + i - count + 1];
            if (destinationPlace && place == destinationPlace) {
                destinationPlace = 0;
                found = i;
                switch (xferMap) {
                case 1:
                    switch (found) {
                    case 5:
                        destinationImage = 1;
                        break;
                    case 6:
                        destinationImage = 2;
                        break;
                    case 7:
                        destinationImage = 3;
                        break;
                    case 8:
                        destinationImage = 4;
                        break;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                    switch (found) {
                    case 6:
                        destinationImage = 1;
                        break;
                    case 7:
                        destinationImage = 2;
                        break;
                    case 8:
                        destinationImage = 3;
                        break;
                    case 9:
                        destinationImage = 4;
                        break;
                    }
                    break;
                }
            }
            place = placeLevels[place];
            if (place > 0)
                images[i] = i + place * 4;
            else {
                if (placeLevelShown < 0)
                    placeLevelShown = 0;
                if (i == count)
                    images[i] = placeLevelShown * 4 + i;
                else if (i > count && place == -1 && images[i - 1] > last)
                    images[i] = placeLevelShown * 4 + i;
            }
        } else if (!base) {
            if (placeLevels[i])
                images[i] = i;
        } else {
            place = mapPlaces[0][base + i - 1];
            if (placeLevels[place])
                images[i] = i;
            else if (place == 11) {
                if (placeLevels[16])
                    images[i] = i;
            } else if (place == 16) {
                if (placeLevels[11])
                    images[i] = i;
            }
        }
    }
    cel = view->body.cels;
    while (cel->image) {
        if (found && found == cel->image)
            found = 0;
        if (images[cel->image]) {
            cel->image = images[cel->image];
            cel++;
        } else
            removeFirstCel(cel);
    }
}

/* A Zoombini's view's script events: 250-253 face it that way; 240-243
   note a way to face (pendingJourneyFacing, then 1-4) when it next turns round (0);
   26 faces it left and moves it after journeyAnchorView; 10-11 start the view of
   views5102 for views5102Due; 50 counts one more in town and starts
   populationSignView's view. */
/* @zoombi32 0x0046b5ce */
void xferSnoidNotify(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);
    View *started;

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
        pendingJourneyFacing = event - 239;
        break;
    case 26:
        setSnoidFacing(snoid, 0);
        moveView(view->id, 0, journeyAnchorView);
        if (snoidsPastAnchor >= 0)
            snoidsPastAnchor++;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingJourneyFacing) {
            setSnoidFacing(snoid, pendingJourneyFacing - 1);
            pendingJourneyFacing = 0;
        }
        snoid->pathDirection++;
        if (!xferMap && snoid->pathDirection == 2)
            moveView(view->id, 1, journeyAnchorView);
        break;
    case 10:
    case 11:
        if (views5102Due[event - 10]) {
            views5102Due[event - 10] = 0;
            started = findView(views5102[event - 10]);
            if (started) {
                started->flags = 0x188000;
                setViewScript(started, 0, 1);
            }
        }
        break;
    case 50:
        shownPopulation++;
        startView(populationSignView, 0, 0, 0);
        break;
    case -1:
        break;
    }
}

/* Draws the population sign's view while it runs (then stops it): its
   cels and "zoombiniville population N", straight to the screen. */
/* @zoombi32 0x0046b761 */
void drawPopulationSign(View *view)
{
    Color saved;
    ShortRect rect;
    char text[32];

    if (view->body.running) {
        drawCels(view);
        saved = setForeColor(Color(45));
        rect = view->body.bounds;
        rect.left += 16;
        rect.top += 8;
        sprintf(text, "%s%d", levelTexts[10], shownPopulation);
        drawOutlinedText(0x70, 0xd1, rect, 1, text);
        copyPortBits(viewPort, workPort, view->body.bounds, view->body.bounds, 0);
        view->body.running = 0;
        setForeColor(saved);
    }
}

/* Scene 2's frame: leaves for the scene due; else goes on to scene
   journeyTo after 300 ticks (and sound xferSound), and now and then starts
   something moving: the next of the party (with xferSnoidNotify), one of the
   views views5104 or view5108, or view6108's once snoidsPastAnchor passes 4. */
/* @zoombi32 0x0046ace4 */
void journeyFrame()
{
    View *view;
    Snoid *snoid;

    if (inJourneyFrame || !journeyOpen)
        return;
    inJourneyFrame = 1;
    updateViews();
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        if (journeyRoute) {
            journeyRoute = 0;
            pendingScene = 1;
        }
        setCurrentMap(0);
        closeJourney();
        inJourneyFrame = 0;
        return;
    }
    if (!dialogFlags) {
        if (soundOn && xferSound) {
            if (!isSoundPlaying(xferSound, RESOURCE_TYPE(0, 'S', 'N', 'D')) && viewClock() > 300)
                sceneDue = journeyTo;
        } else if (viewClock() > 300)
            sceneDue = journeyTo;
        if (!sceneDue && clockTime() > nextJourneyMoveTime) {
            if (snoidsPastAnchor > 4) {
                snoidsPastAnchor = -1;
                view = findView(view6108);
                if (view) {
                    setViewScript(view, 0, 1);
                    view->notify = xferEndNotify;
                }
            }
            if (!xferMap) {
                nextJourneyMoveTime = randomBetween(3, 6) * 30 + clockTime();
                if (randomBetween(1, 100) > 40 || !journeyFirstMoveDone) {
                    journeyFirstMoveDone = 1;
                    if (nextWalker < journeyPartySize) {
                        view = findView(partyViews[nextWalker]);
                        if (view) {
                            snoid = viewSnoid(view);
                            snoid->facingLeft = 0;
                            snoid->pathDirection = 0;
                            startSnoidScript(snoid, snoid->features[3] + 5199, 0, 1);
                            view->notify = xferSnoidNotify;
                            view->notifyEnd = 1;
                        }
                        nextWalker++;
                    }
                } else {
                    short n = randomBetween(0, 4);

                    switch (n) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        view = findView(views5104[n]);
                        if (view && !view->body.running)
                            setViewScript(view, 0, 1);
                        break;
                    case 4:
                        if (views5102Due[2]) {
                            views5102Due[2] = 0;
                            view = findView(view5108);
                            if (view && !view->body.running) {
                                view->flags = 0x188000;
                                setViewScript(view, 0, 1);
                            }
                        }
                        break;
                    }
                }
            } else if (xferMap == 5) {
                nextJourneyMoveTime = randomBetween(3, 6) * 40 + clockTime();
                if (nextWalker < journeyPartySize) {
                    view = findView(partyViews[nextWalker]);
                    if (view) {
                        snoid = viewSnoid(view);
                        snoid->facingLeft = 0;
                        startSnoidScript(snoid, snoid->features[3] + 6199, 0, 1);
                        view->notify = xferSnoidNotify;
                        view->notifyEnd = 1;
                    }
                    nextWalker++;
                }
            }
        }
    }
    inJourneyFrame = 0;
}

/* Scene 2's clicks: once a scene is due (sceneDue), leaves for it (for
   scene 1 if journeyRoute); 1 goes on to scene journeyTo. */
/* @zoombi32 0x0046b00e */
void journeyClicked(short which)
{
    if (journeyOpen) {
        if (sceneDue) {
            pendingScene = sceneDue;
            sceneDue = 0;
            if (journeyRoute) {
                journeyRoute = 0;
                pendingScene = 1;
            }
            setCurrentMap(0);
            closeJourney();
        } else
            switch (which) {
            case 1:
                sceneDue = journeyTo;
                break;
            }
    }
}

/* Spreads the marks in gridMarks over the grid (markGridCell on each one's
   neighbours) until `permille` thousandths of the cells counted by
   setUpGrid are taken, or a pass takes none; returns how many are left.
   Only the right and bottom edges are checked. */
/* @zoombi32 0x0046b9a2 */
unsigned long spreadGridMarks(long permille)
{
    char *cell;
    char *next;
    unsigned long before;
    unsigned long limit;
    unsigned long i;
    unsigned long x;
    unsigned long y;

    limit = gridCellsTotal - gridCellsTotal * permille / 1000;
    while (gridCellsLeft > limit) {
        before = gridCellsLeft;
        for (i = 0; i < 24; i++)
            if (gridMarkUsed[i]) {
                gridMarkUsed[i] = 0;
                x = gridMarks[i].x;
                y = gridMarks[i].y;
                cell = y * gridStride + x + gridCells;
                if (y + 1 < gridRows) {
                    next = cell + gridStride;
                    markGridCell(next, x, y + 1);
                    markGridCell(next - 1, x - 1, y + 1);
                    if (x + 1 < gridColumns)
                        markGridCell(next + 1, x + 1, y + 1);
                }
                if (y - 1 > 0) { /* unsigned: true for row 0 too */
                    next = cell - gridStride;
                    markGridCell(next, x, y - 1);
                    markGridCell(next - 1, x - 1, y - 1);
                    if (x + 1 < gridColumns)
                        markGridCell(next + 1, x + 1, y - 1);
                }
                markGridCell(cell - 1, x - 1, y);
                if (x + 1 < gridColumns)
                    markGridCell(cell + 1, ++x, y);
            }
        if (before == gridCellsLeft)
            gridCellsLeft = 0;
    }
    return gridCellsLeft;
}

/* @zoombi32 0x0046b747 */
void xferEndNotify(View *, short event)
{
    if (event == 30)
        sceneDue = journeyTo;
}

/* Opens scene 2, the journey on from a place (route journeyRoute, 1-16: from
   scene journeyFrom to journeyTo): the backdrop, views and sounds of the map
   the next place is on (xferMap: 0 Zoombini Isle, 1-4 the maps, 5 the
   town), a sound chosen by the camp's hint and the level, the party and,
   on the maps, the grid being filled in under the map's name. */
/* @zoombi32 0x004697f1 */
void openJourney()
{
    short mapView;
    short visits;
    short to;
    Point places[16];
    short from;
    short fromPlace;
    short toPlace;
    short scene;
    short level;
    short hint;
    short scripts;
    short backdrop;
    short group;
    short i;
    View *view;
    Font *font;

    journeyOpen = 0;
    busyCount++;
    resetJourney();
    addSoundRange(20000, 29999, 1);
    setViewsLocked(0);
    openGameFile(&journeyFile, "xfer.MHK");
    setCurrentMap(journeyFile);
    switch (journeyRoute) {
    case 1:
        from = 3;
        to = 7;
        fromPlace = 0;
        toPlace = 1;
        break;
    case 2:
        from = 7;
        to = 8;
        fromPlace = 1;
        toPlace = 2;
        break;
    case 3:
        from = 8;
        to = 9;
        fromPlace = 2;
        toPlace = 3;
        break;
    case 4:
        from = 9;
        to = 4;
        fromPlace = 3;
        toPlace = 4;
        break;
    case 5:
        from = 4;
        to = 10;
        fromPlace = 4;
        toPlace = 5;
        break;
    case 6:
        from = 10;
        to = 11;
        fromPlace = 5;
        toPlace = 6;
        break;
    case 7:
        from = 11;
        to = 12;
        fromPlace = 6;
        toPlace = 7;
        break;
    case 8:
        from = 12;
        to = 5;
        fromPlace = 7;
        toPlace = 11;
        break;
    case 9:
        from = 4;
        to = 13;
        fromPlace = 4;
        toPlace = 8;
        break;
    case 10:
        from = 13;
        to = 14;
        fromPlace = 8;
        toPlace = 9;
        break;
    case 11:
        from = 14;
        to = 15;
        fromPlace = 9;
        toPlace = 10;
        break;
    case 12:
        from = 15;
        to = 5;
        fromPlace = 10;
        toPlace = 16;
        break;
    case 13:
        from = 5;
        to = 16;
        fromPlace = 11;
        toPlace = 12;
        break;
    case 14:
        from = 16;
        to = 17;
        fromPlace = 12;
        toPlace = 13;
        break;
    case 15:
        from = 17;
        to = 18;
        fromPlace = 13;
        toPlace = 14;
        break;
    case 16:
        from = 18;
        to = 6;
        fromPlace = 14;
        toPlace = 15;
        break;
    default:
        from = 0;
        break;
    }
    if (from) {
        journeyFrom = from;
        journeyTo = to;
        readPlaceLevels(placeLevels);
        if (placeLevels[fromPlace] < 0)
            placeLevels[fromPlace] = 1;
        placeLevelShown = placeLevels[toPlace];
        placeLevels[toPlace] = -1;
    } else
        readPlaceLevels(placeLevels);
    switch (journeyTo) {
    case 5:
        xferMap = 2;
        if (journeyFrom == 15)
            xferMap = 3;
        visits = *(short *)(gameState + 0x3e);
        break;
    case 7:
        xferMap = 0;
        visits = *(short *)(gameState + 0x2a);
        break;
    case 8:
        xferMap = 1;
        visits = *(short *)(gameState + 0x2c);
        break;
    case 9:
        xferMap = 1;
        visits = *(short *)(gameState + 0x2e);
        break;
    case 4:
        xferMap = 1;
        visits = *(short *)(gameState + 0x30);
        break;
    case 10:
        xferMap = 2;
        visits = *(short *)(gameState + 0x32);
        break;
    case 11:
        xferMap = 2;
        visits = *(short *)(gameState + 0x34);
        break;
    case 12:
        xferMap = 2;
        visits = *(short *)(gameState + 0x36);
        break;
    case 13:
        xferMap = 3;
        visits = *(short *)(gameState + 0x38);
        break;
    case 14:
        xferMap = 3;
        visits = *(short *)(gameState + 0x3a);
        break;
    case 15:
        xferMap = 3;
        visits = *(short *)(gameState + 0x3c);
        break;
    case 16:
        xferMap = 4;
        visits = *(short *)(gameState + 0x40);
        break;
    case 17:
        xferMap = 4;
        visits = *(short *)(gameState + 0x42);
        break;
    case 18:
        xferMap = 4;
        visits = *(short *)(gameState + 0x44);
        break;
    case 6:
        visits = *(short *)(gameState + 0x46);
        xferMap = 5;
        break;
    }
    scene = currentScene;
    currentScene = journeyTo;
    level = sceneLevel() + 1;
    hint = campHint(&visits);
    currentScene = scene;
    scripts = 0;
    switch (xferMap) {
    case 0:
        switch (hint) {
        case 0:
            if (level >= 2 && level <= 3)
                switch (randomBetween(1, 6)) {
                case 1:
                    xferSound = 20094;
                    break;
                case 2:
                    xferSound = 20095;
                    break;
                case 3:
                    xferSound = 20096;
                    break;
                case 4:
                    xferSound = 20097;
                    break;
                case 5:
                    xferSound = 20098;
                    break;
                case 6:
                    xferSound = 20099;
                    break;
                }
            else
                switch (randomBetween(1, 5)) {
                case 1:
                    xferSound = 20094;
                    break;
                case 2:
                    xferSound = 20095;
                    break;
                case 3:
                    xferSound = 20096;
                    break;
                case 4:
                    xferSound = 20097;
                    break;
                case 5:
                    xferSound = 20099;
                    break;
                }
            break;
        case 1:
            xferSound = 20094;
            break;
        case 2:
        case 12:
            xferSound = 20098;
            break;
        case 5:
            if (level >= 2 && level <= 3)
                xferSound = 20098;
            else
                xferSound = 20094;
            break;
        }
        backdrop = 5000;
        scripts = 9;
        break;
    case 1:
        switch (journeyTo) {
        case 8:
            switch (hint) {
            default:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20007;
                    break;
                case 2:
                    xferSound = 20008;
                    break;
                case 3:
                    xferSound = 20009;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
                xferSound = 20008;
                break;
            }
            break;
        case 9:
            switch (hint) {
            default:
                if (level >= 2)
                    switch (randomBetween(1, 2)) {
                    case 1:
                        xferSound = 20011;
                        break;
                    case 2:
                        xferSound = 20012;
                        break;
                    }
                else
                    switch (randomBetween(1, 2)) {
                    case 1:
                        xferSound = 20010;
                        break;
                    case 2:
                        xferSound = 20012;
                        break;
                    }
                break;
            case 1:
                xferSound = 20010;
                break;
            case 2:
            case 12:
                xferSound = 20011;
                break;
            case 5:
                if (level >= 2)
                    xferSound = 20011;
                else
                    xferSound = 20010;
                break;
            }
            break;
        case 4:
            switch (randomBetween(1, 2)) {
            case 1:
                xferSound = 20009;
                break;
            case 2:
                xferSound = 20012;
                break;
            }
            break;
        }
        backdrop = 1000;
        scripts = 3;
        break;
    case 2:
        switch (journeyTo) {
        case 10:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        xferSound = 20013;
                        break;
                    case 2:
                        xferSound = 20014;
                        break;
                    case 3:
                        xferSound = 20015;
                        break;
                    case 4:
                        xferSound = 20016;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        xferSound = 20013;
                        break;
                    case 2:
                        xferSound = 20014;
                        break;
                    case 3:
                        xferSound = 20016;
                        break;
                    }
                break;
            case 1:
            case 5:
                xferSound = 20014;
                break;
            case 2:
            case 12:
                xferSound = 20015;
                break;
            }
            break;
        case 11:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        xferSound = 20017;
                        break;
                    case 2:
                        xferSound = 20018;
                        break;
                    case 3:
                        xferSound = 20019;
                        break;
                    case 4:
                        xferSound = 20020;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        xferSound = 20017;
                        break;
                    case 2:
                        xferSound = 20018;
                        break;
                    case 3:
                        xferSound = 20020;
                        break;
                    }
                break;
            case 1:
                xferSound = 20018;
                break;
            case 2:
            case 12:
                xferSound = 20019;
                break;
            case 5:
                if (level >= 2)
                    xferSound = 20019;
                else
                    xferSound = 20018;
                break;
            }
            break;
        case 12:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20021;
                    break;
                case 2:
                    xferSound = 20022;
                    break;
                case 3:
                    xferSound = 20024;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                xferSound = 20022;
                break;
            }
            break;
        case 5:
            switch (randomBetween(1, 3)) {
            case 1:
                xferSound = 20016;
                break;
            case 2:
                xferSound = 20020;
                break;
            case 3:
                xferSound = 20024;
                break;
            }
            break;
        }
        backdrop = 2000;
        scripts = 3;
        break;
    case 3:
        switch (journeyTo) {
        case 13:
            switch (hint) {
            case 0:
                if (level == 1 || level == 3)
                    switch (randomBetween(1, 3)) {
                    case 1:
                        xferSound = 20025;
                        break;
                    case 2:
                        xferSound = 20026;
                        break;
                    case 3:
                        xferSound = 20028;
                        break;
                    }
                else
                    switch (randomBetween(1, 4)) {
                    case 1:
                        xferSound = 20025;
                        break;
                    case 2:
                        xferSound = 20026;
                        break;
                    case 3:
                        xferSound = 20027;
                        break;
                    case 4:
                        xferSound = 20028;
                        break;
                    }
                break;
            case 1:
            case 5:
                xferSound = 20026;
                break;
            case 2:
            case 12:
                xferSound = 20026;
                break;
            }
            break;
        case 14:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20029;
                    break;
                case 2:
                    xferSound = 20030;
                    break;
                case 3:
                    xferSound = 20031;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                xferSound = 20030;
                break;
            }
            break;
        case 15:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20032;
                    break;
                case 2:
                    xferSound = 20033;
                    break;
                case 3:
                    xferSound = 20034;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                xferSound = 20033;
                break;
            }
            break;
        case 5:
            switch (randomBetween(1, 3)) {
            case 1:
                xferSound = 20028;
                break;
            case 2:
                xferSound = 20031;
                break;
            case 3:
                xferSound = 20034;
                break;
            }
            break;
        }
        backdrop = 3000;
        scripts = 3;
        break;
    case 4:
        switch (journeyTo) {
        case 16:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20035;
                    break;
                case 2:
                    xferSound = 20036;
                    break;
                case 3:
                    xferSound = 20037;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                xferSound = 20036;
                break;
            }
            break;
        case 17:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        xferSound = 20000;
                        break;
                    case 2:
                        xferSound = 20001;
                        break;
                    case 3:
                        xferSound = 20002;
                        break;
                    case 4:
                        xferSound = 20003;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        xferSound = 20000;
                        break;
                    case 2:
                        xferSound = 20001;
                        break;
                    case 3:
                        xferSound = 20003;
                        break;
                    }
                break;
            case 1:
                xferSound = 20002;
                break;
            case 2:
            case 5:
            case 12:
                if (level >= 2)
                    xferSound = 20002;
                else
                    xferSound = 20001;
                break;
            }
            break;
        case 18:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    xferSound = 20004;
                    break;
                case 2:
                    xferSound = 20005;
                    break;
                case 3:
                    xferSound = 20006;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                xferSound = 20005;
                break;
            }
            break;
        }
        backdrop = 4000;
        scripts = 3;
        break;
    case 5:
        switch (hint) {
        default:
            switch (randomBetween(1, 4)) {
            case 1:
                xferSound = 20100;
                break;
            case 2:
                xferSound = 20101;
                break;
            case 3:
                xferSound = 20102;
                break;
            case 4:
                xferSound = 20103;
                break;
            }
            /* falls through: always 20100 */
        case 1:
        case 5:
            xferSound = 20100;
            break;
        }
        backdrop = 6000;
        scripts = 9;
        break;
    }
    group = backdrop + 100;
    mapView = 0;
    drawBackdrop(backdrop);
    if (scripts) {
        loadFeatureGroup(group, 0, 0);
        loadScripts(group, scripts);
        if (backdrop >= 1000 && backdrop <= 4000) {
            gridView = backdrop + 200;
            loadFeatureGroup(gridView, 1, 0);
            addScripts(gridView, 1, 0);
        }
        copyPaletteRange(10, 236);
        if (!xferMap) {
            for (i = 5102; i <= 5103; i++)
                views5102[i - 5102] = addView(0x1188000, drawCels, runViewScript, i, 6, 0, 0, 0);
            for (i = 5104; i <= 5107; i++)
                views5104[i - 5104] = addView(0x1188000, drawCels, runViewScript, i, 6, 0, 0, 0);
            view5108 = addView(0x1188000, drawCels, runViewScript, 5108, 6, 0, 0, 0);
            useAltSnoids(0);
            for (i = 0; i < 16; i++) {
                places[i].x = 200;
                places[i].y = 235;
            }
            setViewPlaces(16, places, 1);
            makePartySnoids(0);
            journeyAnchorView = addView(0, drawCels, runViewScript, 5100, 0, 0, 0, 0);
            addView(0, drawCels, runViewScript, 5101, 0, 0, 0, 0);
            loadSnoidScripts(5199, 1, 0);
            addSnoidScripts(5200, 5, 0);
        } else if (xferMap < 5) {
            mapView = addView(0xc10c000, drawCels, runViewScript, group, 6, 0, 0, 0);
            view = findView(mapView);
            if (view)
                view->placed = placeMapImages;
        } else {
            view6108 = addView(0x1188000, drawCels, runViewScript, 6108, 6, 0, 0, 0);
            populationSignView = addView(0, drawPopulationSign, runViewScript, 6105, 0, 0, 0, 0);
            journeyAnchorView = addView(0, drawCels, runViewScript, 6104, 0, 0, 0, 0);
            for (i = 0; i < 16; i++) {
                places[i].x = -22;
                places[i].y = randomBetween(0, 3) * 6 + 282;
            }
            setViewPlaces(16, places, 1);
            makePartySnoids(0);
            for (i = 6100; i <= 6103; i++)
                addView(0, drawCels, runViewScript, i, 0, 0, 0, 0);
            loadSnoidScripts(5199, 1, 0);
            addSnoidScripts(6200, 5, 0);
            view6106 = addView(0x1180000, drawCels, runViewScript, 6106, 6, 0, 0, 0);
            view6107 = addView(0x1180000, drawCels, runViewScript, 6107, 6, 0, 0, 0);
        }
    } else
        copyPaletteRange(10, 236);
    if (xferMap >= 1 && xferMap <= 4) {
        gridView = addView(0x4000000, drawGridView, updateGridView, gridView, 4, 0, 0, 0);
        view = findView(gridView);
        if (view)
            view->placed = placeMapPlace;
        for (i = 0; i < 16; i++) {
            places[i].x = -22;
            places[i].y = 445;
        }
        setViewPlaces(16, places, 1);
        makePartySnoids(0);
        addView(0, drawCels, runViewScript, group + 1, 0, 0, 0, 0);
        addView(0, drawCels, runViewScript, group + 2, 0, 0, 0, 0);
    }
    startView(populationSignView, 0, 0, 0);
    updateViews();
    if (xferMap >= 1 && xferMap <= 4) {
        Color saved;

        font = setFont(fonts[2]);
        saved = setForeColor(Color(10));
        drawOutlinedText(45, 10, mapTitleRects[xferMap - 1], 0x22, levelTexts[xferMap + 5]);
        copyPortBits(viewPort, workPort, gameRect, gameRect, 0);
        setForeColor(saved);
        setFont(font);
        deleteView(mapView);
    }
    setGroupLists(xferGroups, 1, (short)0xc000);
    if (xferSound)
        queueViewSound(xferSound, 1);
    showRect(&shownGameRect);
    fadeInViews();
    resetViewClock();
    journeyPartySize = countChosenSnoids();
    journeyOpen = 1;
    startView(view6107, 0, 0, 0);
    startView(view6106, 0, 0, 0);
    if (xferMap >= 1 && xferMap <= 4)
        sendSnoids(670, 445, 90);
}

/* Closes scene 2. */
/* @zoombi32 0x0046ac6e */
void closeJourney()
{
    if (journeyOpen) {
        journeyOpen = 0;
        short saved = setFreeAtOnce(1);

        journeyRoute = 0;
        useAltSnoids(1);
        chooseSnoids(1, 1);
        clearViews();
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&journeyFile);
        fadeOutViews();
        showBusyCursor();
        snoidIdleDelay = 64;
        if (busyCount)
            busyCount--;
    }
}

/* Sets up the grid (`rows` of `columns` cells, `stride` apart): cells
   `from1` become `to1` and `from2` `to2`, counted in gridCellsTotal (and
   gridCellsLeft); markGridCell then marks those taken as `taken1` and `taken2`.
   The first of gridMarks is `start`, kept inside the grid. */
/* @zoombi32 0x0046b872 */
void setUpGrid(char *grid, unsigned long stride, unsigned long rows, unsigned long columns,
               unsigned char from1, unsigned char from2, char to1, char to2, char taken1,
               char taken2, Point &start)
{
    unsigned long i;
    char *row;
    unsigned long y;
    char *cell;
    unsigned long x;

    for (i = 0; i < 24; i++) {
        gridMarkUsed[i] = 0;
        gridMarks[i].x = 0;
        gridMarks[i].y = 0;
    }
    gridCellsTotal = 0;
    row = grid;
    for (y = 0; y < rows; y++) {
        cell = row;
        for (x = 0; x < columns; x++) {
            char c = *cell;

            if (c == from1) {
                gridCellsTotal++;
                *cell = to1;
            }
            if (c == from2) {
                gridCellsTotal++;
                *cell = to2;
            }
            cell++;
        }
        row += stride;
    }
    gridFree1 = to1;
    gridFree2 = to2;
    gridTaken1 = taken1;
    gridTaken2 = taken2;
    gridCells = grid;
    gridStride = stride;
    gridRows = rows;
    gridColumns = columns;
    gridCellsLeft = gridCellsTotal;
    if (1) {
        x = start.x;
        y = start.y;
    } else {
        x = 0;
        y = 0;
    }
    if (x > columns)
        x = columns - 1;
    if (y > rows)
        y = rows - 1;
    gridMarkUsed[0] = 1;
    gridMarks[0].x = x;
    gridMarks[0].y = y;
}

/* Marks the cell at `cell` taken (gridFree1 becomes gridTaken1, gridFree2
   becomes gridTaken2), noting the point in a free one of gridMarks. */
/* @zoombi32 0x0046bb0c */
void markGridCell(char *cell, long x, long y)
{
    short i;

    if (*cell == gridFree1)
        for (i = 0; i < 24; i++)
            if (!gridMarkUsed[i]) {
                gridMarks[i].x = x;
                gridMarks[i].y = y;
                gridMarkUsed[i] = 1;
                if (gridCellsLeft > 0)
                    gridCellsLeft--;
                *cell = gridTaken1;
                return;
            }
    if (*cell == gridFree2)
        for (i = 0; i < 24; i++)
            if (!gridMarkUsed[i]) {
                gridMarks[i].x = x;
                gridMarks[i].y = y;
                gridMarkUsed[i] = 1;
                if (gridCellsLeft > 0)
                    gridCellsLeft--;
                *cell = gridTaken2;
                return;
            }
}

/* A view's placed callback: keeps one of its first four cels by
   destinationImage (1-4) as the first, alone. */
/* @zoombi32 0x0046bbce */
void placeMapPlace(View *view)
{
    ViewCel *cels = view->body.cels;

    switch (destinationImage) {
    case 1:
    default:
        cels[1].image = 0;
        break;
    case 2:
        cels[0].image = cels[1].image;
        cels[0].x = cels[1].x;
        cels[0].y = cels[1].y;
        cels[1].image = 0;
        break;
    case 3:
        cels[0].image = cels[2].image;
        cels[0].x = cels[2].x;
        cels[0].y = cels[2].y;
        cels[1].image = 0;
        break;
    case 4:
        cels[0].image = cels[3].image;
        cels[0].x = cels[3].x;
        cels[0].y = cels[3].y;
        cels[1].image = 0;
        break;
    }
}

/* A view's draw callback: while it changes, fills in its first cel's
   image (a grid, the header's words big-endian) 7 thousandths more each
   time: first setting up the grid (setUpGrid) from where map xferMap's
   place destinationImage starts, with marks by destinationLevel's level, then spreading
   them (spreadGridMarks). */
/* @zoombi32 0x0046bc51 */
void drawGridView(View *view)
{
    char to1;
    char to2;
    char taken1;
    char taken2;
    ViewCel *cel;
    short start;
    unsigned short *header;
    ImageBank *bank;

    if (!view->changed) {
        drawCels(view);
        return;
    }
    if (!gridProgress) {
        cel = view->body.cels;
        bank = groupBanks[view->body.scriptGroup];
        if (cel->image) {
            start = (xferMap - 1) * 4 + destinationImage - 1;
            if (start < 0 || start > 15)
                start = 0;
            switch (destinationLevel) {
            default:
                to1 = '.';
                to2 = '/';
                taken1 = '0';
                taken2 = '1';
                break;
            case 2:
                to1 = '0';
                to2 = '1';
                taken1 = '2';
                taken2 = '3';
                break;
            case 3:
                to1 = '2';
                to2 = '3';
                taken1 = '4';
                taken2 = '5';
                break;
            case 4:
                to1 = '4';
                to2 = '5';
                taken1 = '6';
                taken2 = '7';
                break;
            }
            header = (unsigned short *)((char *)bank + bank->offsets[cel->image]);
            setUpGrid((char *)(header + 4), swapShort(header[2]), swapShort(header[1]),
                      swapShort(header[0]), 1, 2, to1, to2, taken1, taken2, gridStarts[start]);
        }
    } else
        spreadGridMarks(gridProgress);
    gridProgress += 7;
    if (gridProgress > 1000)
        gridProgress = 1000;
    drawCels(view);
}

/* A view update: runs the script, and when due redraws it. */
/* Not exact: the original keeps `region` in esi (loaded once); here it is
   read from the stack at each use, `register` or not. */
/* @zoombi32 0x0046bdde */
void updateGridView(View *view, short region)
{
    if (!dialogFlags) {
        runViewScript(view, region);
        if (view->nextUpdate <= updateTime) {
            view->changed = 1;
            view->nextUpdate = updateTime + view->interval;
            unionRgnRect(region, &view->body.bounds);
        }
    }
}
