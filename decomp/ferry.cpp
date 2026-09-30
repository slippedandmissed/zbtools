/*
 * ferry (0x41f8cc-0x42160c): Captain Cajun's ferry (scene 10), 'Ferry.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "ferry.h"
#include "focus.h"
#include "graphics.h"
#include "mainloop.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

short cajunGreetings[4] = {0x708, 0x709, 0x70a, 0x70b};
unsigned long cajunGreetingsUsed = 0;
short cajunIdleRemarks[5] = {0x71f, 0x720, 0x721, 0x722, 0x723};
unsigned long cajunIdleRemarksUsed = 0;
short goodPlacingRemarks[2] = {0x719, 0x71a};
unsigned long goodPlacingRemarksUsed = 0;
short badPlacingRemarks[11] = {
    0x70c, 0x70d, 0x70e, 0x70f, 0x710, 0x711, 0x712, 0x713, 0x714, 0x715, 0x716,
};
unsigned long badPlacingRemarksUsed = 0;
short returnSounds[5] = {0x724, 0x725, 0x726, 0x727, 0x728};
unsigned long returnSoundsUsed = 0;
short movedRemarks[3] = {0x71c, 0x71d, 0x71e};
unsigned long movedRemarksUsed = 0;
short placeViewScripts[10] = {
    0x6a5, 0x6a4, 0x6a4, 0x6a6, 0x6a6, 0x6a7, 0x6a7, 0x6a4, 0x6a6, 0x6a7,
};
short returnScripts[10] = {
    0x76e, 0x76c, 0x76c, 0x770, 0x770, 0x772, 0x772, 0x76c, 0x770, 0x772,
};
short returnNextScripts[10] = {
    0x76f, 0x76d, 0x771, 0x76d, 0x771, 0x76d, 0x771, 0x773, 0x773, 0x773,
};
ImageBank *ferryButtonImages = 0;
SceneButton ferryButtons[2] = {{{600, 403, 639, 440}}, {{600, 441, 639, 478}}};
long ferryButtonResource = 0;
Point ferryPlaces[20] = {
    {370, 160}, {395, 196}, {332, 156}, {348, 196}, {294, 168}, {316, 196}, {253, 166}, {276, 196},
    {214, 157}, {237, 196}, {175, 160}, {196, 190}, {135, 152}, {150, 191}, {94, 145}, {110, 186},
    {57, 146}, {71, 182}, {25, 145}, {27, 183},
};
short ferryButton2Lit = 0;
short ferryButton1Drawn = 0;
short inFerryFrame = 0;

unsigned long nextIdleRemarkTime;
short forcedFerryCount;
short ferryLevel;
Point returnPlace;
short returnUnderway;
Point returnTarget;
Point *returnAnchor;
Point returnLanding;
short cajunRemarkDue;
short returnDue;
short ferryHelpersDue;
short cajunLeavingGroup;
long ferryFile;
short ferryOpen;
short ferryHasPassengers;
short cajunGreeted;
short view1601;
short cajunView;
short view1602;
short view1603;
short view1704;
short view1705;
short view1706;
short returnPlaceView;
short movingPlaceView;
short lastSceneryView;
short ferryPlaceViews[20];
short returnRoute;
short nextReturner;
short returner;
short ferryLeaving;
char (*ferryLinks)[8];

short ferryVisits;
unsigned long returnRoutesUsed;
short sharedFeatureBits;
short sharedFeatureView;
short goodPlacings;
short badPlacings;
short nextPraiseAt;
short ferrySnoidCount;
short praisedOnce;
short cajunRemarkGroup;
short debugCajunScript;
short cajunScript;

/* Draws button `which` (1 or 2; 2 is dim unless ferryHasPassengers), lit or not,
   and shows it if asked. */
/* @zoombi32 0x0041fdee */
void drawFerryButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!ferryHasPassengers) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(ferryButtonImages->offsets[image] + (char *)ferryButtonImages), ferryButtons[which - 1].rect.left,
                      ferryButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&ferryButtons[which - 1].rect);
    }
}

/* The buttons' view update: redraws button 2 as ferryHasPassengers changes, and
   button 1 once. */
/* @zoombi32 0x0041fea4 */
void updateFerryButtons(View *, short region)
{
    if (ferryHasPassengers) {
        if (!ferryButton2Lit) {
            ferryButton2Lit = 1;
            unionRgnRect(region, &ferryButtons[1].rect);
        }
    } else if (ferryButton2Lit) {
        ferryButton2Lit = 0;
        unionRgnRect(region, &ferryButtons[1].rect);
    }
    if (!ferryButton1Drawn) {
        ferryButton1Drawn = 1;
        unionRgnRect(region, &ferryButtons[0].rect);
    }
}

/* Closes scene 10. */
/* @zoombi32 0x0041ff16 */
void closeFerry()
{
    if (ferryOpen) {
        ferryOpen = returnDue = ferryHelpersDue = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        freeResource(&ferryButtonResource);
        unloadSounds();
        if (ferryLinks) {
            disposePtr(ferryLinks);
            ferryLinks = 0;
        }
        setFreeAtOnce(saved);
        closeGameFile(&ferryFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Moves returnPlaceView to movingPlaceView and starts its view on placeViewScripts's script
   for returnRoute. */
/* @zoombi32 0x004209b8 */
void moveFerryOn()
{
    View *view;

    movingPlaceView = returnPlaceView;
    returnPlaceView = 0;
    view = findView(movingPlaceView);
    if (view) {
        view->flags &= ~0x800000;
        setViewScript(view, placeViewScripts[returnRoute], 1);
        view->flags |= 0x800000;
    }
}

/* Starts returner's Zoombini on `script` (anchored at returnAnchor) in
   `group`, with `notify` if given. */
/* @zoombi32 0x00420a08 */
void startCrosserScript(short group, short script, ViewNotify notify, char idleTicks)
{
    View *view = findView(returner);

    if (view) {
        startSnoidScript(viewSnoid(view), script, returnAnchor, idleTicks);
        view->body.group = group;
        loadViewSounds(returner, 0);
        if (notify)
            view->notify = notify;
    }
}

/* A notify: 6 starts view1706's script in this view's group; other events
   turn the Zoombini (turnSnoid). */
/* @zoombi32 0x00420c82 */
void ferryHelperNotify(View *view, short event)
{
    View *other;

    switch (event) {
    default:
        turnSnoid(view, event);
        break;
    case 6:
        other = findView(view1706);
        if (other) {
            setViewScript(other, 0, 1);
            other->body.group = view->body.group;
        }
        break;
    }
}

/* Moves the placed Zoombinis (flag 1 and chosen) and two kinds of
   view (flags 0x748c2000, 0x74980000) `dx` along, cels and all. */
/* @zoombi32 0x0042113f */
void slideFerryViews(View *, short dx)
{
    View *view;
    ViewCel *cel;

    for (view = viewListEnd(1)->next; view; view = view->next)
        if (view->flags == 1 && viewSnoid(view)->chosen || view->flags == 0x748c2000 || view->flags == 0x74980000) {
            view->body.group = 0;
            view->body.x += dx;
            for (cel = view->body.cels; cel->image; cel++)
                cel->x += dx;
        }
}

/* Resets scene 10's state. */
/* @zoombi32 0x0041f8cc */
void resetFerry()
{
    forcedFerryCount = 0;
    hintSound = 0;
    praisedOnce = ferryLeaving = 0;
    goodPlacings = badPlacings = 0;
    nextPraiseAt = 1;
    returnUnderway = 0;
    returnRoute = nextReturner = returner = cajunLeavingGroup = 0;
    returnAnchor = 0;
    cajunGreeted = returnDue = ferryHelpersDue = 0;
    sharedFeatureView = sharedFeatureBits = cajunRemarkDue = sceneDue = 0;
    view1602 = view1603 = view1704 = view1705 = view1706 = 0;
}

/* Finds a free waiting place (of ferryPlaces, noting which Zoombini is
   nearest each in sortedIds) into *spot: an even one (from either end at
   random), else an odd one, else 0. */
/* @zoombi32 0x004214ab */
void findFerryPlace(short *spot)
{
    Point none = {0, 0};
    short i;
    short found;
    short skip;
    short j;

    spotTaken(&none, 0, 500);
    for (i = 0; i < 20; i++) {
        skip = 0;
        found = spotNear(&ferryPlaces[i], 500, skip);
        for (j = 0; found && j < i; j++)
            if (found == sortedIds[j]) {
                skip++;
                found = spotNear(&ferryPlaces[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = found;
    }
    found = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 18; found == -1 && i >= 0; i -= 2)
            if (!sortedIds[i])
                found = i;
    } else {
        for (i = 0; found == -1 && i <= 18; i += 2)
            if (!sortedIds[i])
                found = i;
    }
    if (found == -1) {
        if (randomBetween(1, 100) <= 50) {
            for (i = 19; found == -1 && i >= 1; i -= 2)
                if (!sortedIds[i])
                    found = i;
        } else {
            for (i = 1; found == -1 && i <= 19; i += 2)
                if (!sortedIds[i])
                    found = i;
        }
    }
    if (found == -1)
        found = 0;
    *spot = found;
}

/* Starts the next Zoombini (nextReturner) on its way by `n` (returnRoute; 0 or
   over 9: 0): Captain Cajun's view (cajunView) plays 1604-1607, and for 7-9
   the views view1705 and view1706 are made anew about returnPlace. */
/* @zoombi32 0x00420f85 */
void startNextCrosser(short n)
{
    View *view;
    Point at;

    returnRoute = n;
    if (!nextReturner)
        return;
    returner = nextReturner;
    nextReturner = 0;
    if (n >= 10)
        n = 0;
    view = findView(cajunView);
    if (!view)
        return;
    switch (n) {
    case 0:
        cajunScript = 1605;
        returnTarget.x = 122;
        returnTarget.y = 164;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        if (randomBetween(1, 100) <= 50)
            cajunScript = 1604;
        else
            cajunScript = 1606;
        returnTarget = returnPlace;
        break;
    case 7:
    case 8:
    case 9:
        cajunScript = 1607;
        deleteView(view1705);
        deleteView(view1706);
        view1705 = addView(0x1180000, drawCels, runViewScript, 1705, 6, 0, 0, -1);
        returnTarget.x = 236;
        returnTarget.y = 474;
        returnLanding = returnPlace;
        at.x = returnLanding.x - 14;
        at.y = returnLanding.y - 14;
        view1706 = addView(0x1980000, drawCels, runViewScript, 1706, 6, &at, 0, -1);
        break;
    }
    setViewScript(view, cajunScript, 1);
    loadViewSounds(cajunView, 1);
    returnAnchor = 0;
    view->notify = crosserNotify;
    groupViews(view->id, returnPlaceView, returner, 0, 0, 0);
    setViewsLocked(0);
}

/* A crossing Zoombini's notify (returner, by returnRoute): 1 walks it on
   (returnScripts), 2 and 3 on again (returnNextScripts; 2 also lets the views go, or
   puts it behind view1705 when that script is 1907), 4 sets it down facing
   left at (93, 408), 5 walks it off to returnLanding; 6 picks a sound
   (returnSounds) if none is due. */
/* @zoombi32 0x00420a60 */
void crosserNotify(View *view, short event)
{
    View *other;

    switch (event) {
    case 1:
        moveFerryOn();
        startCrosserScript(view->body.group, returnScripts[returnRoute], crosserNotify, 1);
        break;
    case 2:
        if (returnNextScripts[returnRoute] == 1907) {
            moveView(returner, 0, view1705);
            ferryHelpersDue = 1;
        } else {
            returnAnchor = &returnTarget;
            startCrosserScript(view->body.group, returnNextScripts[returnRoute], 0, 0);
            returnAnchor = 0;
            returnUnderway = 0;
            requestViewSort();
        }
        break;
    case 3:
        returnAnchor = &returnTarget;
        startCrosserScript(view->body.group, returnNextScripts[returnRoute], 0, 1);
        returnAnchor = 0;
        break;
    case 6:
        if (!cajunRemarkDue)
            cajunRemarkDue = returnSounds[allocateSlot(&returnSoundsUsed, 5, 0)];
        break;
    case 4:
        other = findView(returner);
        if (other) {
            viewSnoid(other)->facingLeft = 1;
            other->body.x = 93;
            other->body.y = 408;
            startSnoidScript(viewSnoid(other), viewSnoid(other)->features[3] * 2 + 998, 0, 0);
            other->notify = crosserNotify;
            other->body.group = view->body.group;
            requestViewSort();
        }
        break;
    case 5:
        other = findView(returner);
        if (other) {
            viewSnoid(other)->facingLeft = 0;
            startSnoidScript(viewSnoid(other), viewSnoid(other)->features[3] * 2 + 999, &returnLanding, 0);
            other->body.group = view->body.group;
            other->notify = ferryHelperNotify;
        }
        returner = 0;
        returnUnderway = 0;
        break;
    }
}

/* Works out which of the placed views (ferryPlaceViews) touch: for each, the
   others meeting its bounds grown or shrunk by half its height less 2 (and
   from level 3, ferryLevel, widened), up to 8, into ferryLinks (from 1); with
   `draw`, draws each link as a line between the centres. */
/* @zoombi32 0x0042121c */
void linkFerryPlaces(short draw)
{
    short count;
    ShortRect rect;
    ShortRect other;
    ShortRect probe;
    ShortRect rects[20];
    short i;
    short j;
    short d;
    short met;
    View *view;

    for (i = 0; i < 20; i++)
        for (j = 0; j < 8; j++)
            ferryLinks[i][j] = 0;
    for (i = 0; i < placedViewCount; i++) {
        view = findView(ferryPlaceViews[i]);
        if (view)
            rects[i] = view->body.bounds;
    }
    for (i = 0; i < placedViewCount; i++) {
        rect = rects[i];
        count = 0;
        for (j = 0; j < placedViewCount; j++) {
            if (i == j)
                continue;
            other = rects[j];
            d = (rect.bottom - rect.top) / 2 - 2;
            probe.top = rect.top - d;
            probe.bottom = rect.bottom + d;
            probe.right = rect.right - d;
            probe.left = rect.left + d;
            met = sectRect(&probe, &other);
            if (!met) {
                probe.top = rect.top + d;
                probe.bottom = rect.bottom - d;
                probe.right = rect.right + d;
                probe.left = rect.left - d;
                met = sectRect(&probe, &other);
            }
            if (!met && ferryLevel >= 3) {
                probe.top = rect.top - d;
                probe.bottom = d += rect.bottom;
                probe.right = rect.right;
                probe.left = rect.left;
                met = sectRect(&probe, &other);
            }
            if (met && count < 8) {
                if (draw) {
                    Color saved;

                    saved = setForeColor(Color(11));
                    moveTo((rect.right + rect.left) / 2, (rect.bottom + rect.top) / 2);
                    lineTo((other.right + other.left) / 2, (other.bottom + other.top) / 2);
                    setForeColor(saved);
                }
                ferryLinks[i][count] = j + 1;
                count++;
            }
        }
    }
    if (draw)
        showRect(&gameRect);
}

/* A view draw: draws both buttons, unlit. */
/* @zoombi32 0x0041fe87 */
void drawFerryButtons(View *)
{
    drawFerryButton(1, 0, 0);
    drawFerryButton(2, 0, 0);
}

/* Scene 10's keys (with debugging on, debugMessagesOn, or else only 0x16f; case
   ignored): 0x16f replayHint; A draws the links between the places; L
   reports the level (from 1); F plays Captain Cajun's script debugCajunScript
   (1800-1832; else his current one). Returns whether the key was used. */
/* @zoombi32 0x004208a3 */
short ferryKey(unsigned short key)
{
    short used = 0;
    ShortRect unused = {0, 0, 225, 18};
    View *view;

    if (!debugMessagesOn && key != 0x16f)
        return 0;
    if (key >= 'a' && key <= 'z')
        key -= 32;
    switch (key) {
    case 0x16f:
        replayHint();
        used = 1;
        break;
    case 'A':
        linkFerryPlaces(1);
        used = 1;
        break;
    case 'L':
        debugMessage(ferryLevel + 1, 0, 0, 0, 0);
        used = 1;
        break;
    case 'F':
        if (view1601) {
            view = findView(cajunView);
            if (view) {
                if (debugCajunScript > 1832 || debugCajunScript < 1800)
                    debugCajunScript = view->kind;
                setViewScript(view, debugCajunScript, 1);
                loadViewSounds(cajunView, 1);
                used = 1;
                debugMessage(debugCajunScript, "Play FrogMan SCRB id:", 0, 0, 0);
            }
        }
        break;
    }
    return used;
}

/*
 * Lays out the places from 'SCRB' script `id`: its first two frames are
 * two lists of parts, taken in turn, a list at a time; each part is a view
 * (script part + 1499) at a point: 1-3 places to stand (placed views, from
 * lastSceneryView on), 4-10 scenery (unless gameState's +0x20).
 */
/* @zoombi32 0x00420cce */
void layOutFerry(short id)
{
    long resource;
    short count;
    short frame;
    short *first;
    short *second;
    Point at;
    short useFirst;
    short after;
    short *data;
    short word;
    int i;
    int j;
    short done;

    resource = 0;
    count = 0;
    after = lastSceneryView;
    data = loadSwappedResource(&resource, id, RESOURCE_TYPE('S', 'C', 'R', 'B'));
    frame = 0;
    first = data + scriptFrameOffset(data, &frame, 0);
    frame = 1;
    data += scriptFrameOffset(data, &frame, 0);
    second = data;
    useFirst = 1;
    i = j = 0;
    word = second[j];
    if (!word) {
        j += 3;
        for (done = 0; !done;) {
            word = second[j++];
            if (!word) {
                j += 2;
            } else {
                if (word < 0)
                    j = -1;
                else
                    j--;
                done = 1;
            }
        }
    }
    while (i >= 0 || j >= 0) {
        if (useFirst) {
            if (i >= 0) {
                word = first[i++];
                if (!word) {
                    i += 2;
                    for (done = 0; !done;) {
                        word = first[i++];
                        if (!word) {
                            i += 2;
                        } else {
                            if (word < 0)
                                i = -1;
                            else
                                i--;
                            done = 1;
                        }
                    }
                    word = 0;
                    useFirst = 0;
                } else if (word > 0) {
                    at.x = first[i];
                    i++;
                    at.y = first[i];
                    i++;
                } else {
                    i = -1;
                }
            } else {
                useFirst = 0;
            }
        } else if (j >= 0) {
            word = second[j++];
            if (!word) {
                j += 2;
                for (done = 0; !done;) {
                    word = second[j++];
                    if (!word) {
                        j += 2;
                    } else {
                        if (word < 0)
                            j = -1;
                        else
                            j--;
                        done = 1;
                    }
                }
                word = 0;
                useFirst = 1;
            } else if (word > 0) {
                at.x = second[j];
                j++;
                at.y = second[j];
                j++;
            } else {
                j = -1;
            }
        } else {
            useFirst = 1;
        }
        if (word >= 1 && word <= 3) {
            after = addView(0x748c2000, drawCels, runViewScript, word + 1499, 6, &at, 1, after);
            ferryPlaceViews[count] = after;
            placedViewPoints[count].x = at.x + 22;
            placedViewPoints[count].y = at.y - 7;
            placedViews[count] = ferryPlaceViews[count];
            placeClaims[count] = 0;
            count++;
        } else if (word >= 4 && word <= 10 && !*(short *)(gameState + 0x20)) {
            after = addView(0x74980000, drawCels, runViewScript, word + 1499, 6, &at, 1, after);
        }
    }
    placedViewCount = count;
    freeResource(&resource);
}

/* Lays out the places for the level (ferryLevel, 0-4) and the number of
   Zoombinis (16-20, or forcedFerryCount): scripts 1510-1529. */
/* @zoombi32 0x004211a3 */
void layOutFerryLevel()
{
    short n;

    if (ferryLevel < 0 || ferryLevel > 4)
        ferryLevel = 0;
    n = countChosenSnoids();
    if (forcedFerryCount)
        n = forcedFerryCount;
    if (n < 16 || n > 20)
        n = 16;
    n -= 16;
    switch (ferryLevel) {
    case 0:
        n += 1510;
        break;
    case 1:
        n += 1515;
        break;
    case 2:
        n += 1520;
        break;
    case 3:
        n += 1525;
        break;
    }
    layOutFerry(n);
}

/* Opens scene 10: Ferry.MHK, its sounds, images and scripts, Captain
   Cajun (the first time 1803, then one of cajunGreetings), the views, the
   places for the level (layOutFerryLevel) and the party, and a hint or
   greeting. */
/* @zoombi32 0x0041f97c */
void openFerry()
{
    short i;

    ferryOpen = ferryHasPassengers = 0;
    resetFerry();
    ferryVisits++;
    ferryLevel = sceneLevel();
    ferryLinks = (char (*)[8])newPtr(160);
    soundRanges = 0;
    addSoundRange(1606, 1607, 1);
    addSoundRange(20000, 29999, 1);
    addSoundRange(1800, 1899, 1);
    addSoundRange(996, 997, 0);
    addSoundRange(1704, 1705, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(1600, 1699, 0);
    addSoundRange(1900, 1999, 0);
    addSoundRange(1700, 1799, 0);
    openGameFile(&ferryFile, "Ferry.MHK");
    setCurrentMap(ferryFile);
    drawBackdrop(1300);
    ferryButtonImages = loadImageBank(1400, &ferryButtonResource);
    loadFeatureGroup(1500, 0, 0);
    loadFeatureGroup(1600, 1, 0);
    loadFeatureGroup(1700, 2, 0);
    loadFeatureGroup(1800, 3, 0);
    loadFeatureGroup(1450, 4, 0);
    loadScripts(1500, 10);
    addScripts(1600, 10, 0);
    addScripts(1700, 7, 0);
    addScripts(1800, 33, 5);
    addScripts(1450, 3, 0);
    loadTerrain(100);
    loadSnoidScripts(1900, 8, 0);
    addSnoidScripts(1000, 10, 1);
    view1601 = addView(0xc000, drawCels, runViewScript, 1601, 6, 0, 0, 0);
    if (ferryVisits == 1)
        cajunView = 1803;
    else
        cajunView = cajunGreetings[allocateSlot(&cajunGreetingsUsed, 4, 0)];
    cajunView = addView(0x188000, drawCels, runViewScript, cajunView, 6, 0, 0, 0);
    returner = cajunView;
    if (!*(short *)(gameState + 0x20)) {
        view1602 = addView(0x8000, drawCels, runViewScript, 1602, 6, 0, 0, 0);
        view1603 = addView(0x8000, drawCels, runViewScript, 1603, 6, 0, 0, 0);
        pairViews(view1602, view1603);
    }
    view1704 = addView(0x1188000, drawCels, runViewScript, 1704, 6, 0, 0, 0);
    addView(0, drawCels, runViewScript, 1600, 6, 0, 0, 0);
    for (i = 0; i < 3; i++)
        lastSceneryView = addView(0x4000000, drawCels, runViewScript, i + 1450, 0, 0, 0, 0);
    returnPlaceView = movingPlaceView = 0;
    addView(0x1000, drawFerryButtons, updateFerryButtons, 0, 0, 0, 0, 0);
    setViewPlaces(20, ferryPlaces, 1);
    setViewsLocked(0);
    copyPaletteRange(10, 236);
    makePartySnoids(0);
    layOutFerryLevel();
    enterSnoids(0);
    updateViews();
    linkFerryPlaces(0);
    staggerSnoids(45, 30);
    requestViewSort();
    setGroupLists(ferryGroups, 1, (short)0xc000);
    drawFerryButton(1, 0, 0);
    drawFerryButton(2, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    queueViewSound(997, 0);
    chooseSnoids(0, 0);
    ferrySnoidCount = countSnoidViews();
    resetViewClock();
    nextIdleRemarkTime = randomBetween(5400, 10800);
    ferryOpen = 1;
    switch (campHint((short *)(gameState + 0x32))) {
    case 2:
        hintSound = 20074;
        break;
    default:
        if (ferryLevel)
            hintSound = randomBetween(20073, 20074);
        else
            hintSound = 20073;
        break;
    }
}

/* Scene 10's frame: once everyone has crossed (ferryLeaving), Captain Cajun
   leaves (1608-1609) and so does the scene; leaves when asked; plays a
   remark due (cajunRemarkDue) or picks one now and then (cajunIdleRemarks); starts the
   ferry's two views (ferryHelpersDue); sends the next Zoombini to a free place
   (returnDue, startNextCrosser, some routes needing places free on the right);
   and greets once the sound 997 ends. */
/* @zoombi32 0x0041ff89 */
void ferryFrame()
{
    short spot;
    short again;
    short i;
    View *view;

    if (inFerryFrame || !ferryOpen)
        return;
    inFerryFrame = 1;
    if (ferryLeaving && !returnDue && !returnUnderway) {
        ferryLeaving = 0;
        deleteView(view1601);
        view1601 = 0;
        deleteView(view1602);
        deleteView(view1603);
        startView(cajunView, randomBetween(1608, 1609), slideFerryViews, 0);
        loadViewSounds(cajunView, 1);
        cajunLeavingGroup = groupViews(cajunView, cajunView, 0, 0, 0, 0);
        sceneDue = 11;
    }
    updateViews();
    if (sceneDue) {
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3 && !practiceLevel)
                chooseSnoids(0, 0);
            if (viewsLocked || !groupLeader[cajunLeavingGroup]) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeFerry();
                inFerryFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (cajunRemarkDue) {
        i = cajunRemarkDue;
        cajunRemarkDue = 0;
        if (view1601) {
            startView(cajunView, i, 0, 0);
            loadViewSounds(cajunView, 1);
            cajunRemarkGroup = groupViews(cajunView, cajunView, 0, 0, 0, 0);
        }
    } else if (ferryHelpersDue) {
        ferryHelpersDue = 0;
        startView(view1704, 0, crosserNotify, 0);
        startView(view1705, 0, crosserNotify, 0);
        groupViews(view1704, view1705, 0, 0, 0, 0);
    } else if (returnDue) {
        if (!groupLeader[cajunRemarkGroup]) {
            returnDue = 0;
            findFerryPlace(&spot);
            for (again = 1; again;) {
                again = 0;
                returnRoute = allocateSlot(&returnRoutesUsed, 10, 0);
                switch (returnRoute) {
                case 0:
                    if (sortedIds[12] || sortedIds[14])
                        again = 1;
                    break;
                case 7:
                case 8:
                case 9:
                    again = 1;
                    for (i = 19; again && i >= 11; i -= 2)
                        if (!sortedIds[i]) {
                            if (!sortedIds[i - 1] && !sortedIds[i - 3]) {
                                again = 0;
                                spot = i;
                            }
                        } else {
                            i = 0;
                        }
                    break;
                }
            }
            returnPlace = ferryPlaces[spot];
            startNextCrosser(returnRoute);
        }
    } else if (viewClock() > nextIdleRemarkTime) {
        resetViewClock();
        cajunRemarkDue = cajunIdleRemarks[allocateSlot(&cajunIdleRemarksUsed, 5, 0)];
        nextIdleRemarkTime = randomBetween(5400, 10800);
    }
    if (sharedFeatureView) {
        view = idleSnoidView(sharedFeatureView);
        if (view) {
            viewSnoid(view)->pose = sharedFeatureBits;
            sharedFeatureBits = 0;
            sharedFeatureView = 0;
        }
    }
    if (!cajunGreeted && !snoidsOnTheirWay && !isSoundPlaying(997, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        cajunGreeted = 1;
        startView(cajunView, 0, 0, 0);
        loadViewSounds(cajunView, 1);
    }
    playAmbientSound();
    inFerryFrame = 0;
}

/* Scene 10's clicks: 1 leaves (asking whether to keep the party), 2 sets
   the ferry off (once there's a Zoombini aboard, ferryHasPassengers), 3 drags a
   Zoombini. Put at a place, it must share a feature with every Zoombini
   at a place linked to it (ferryLinks; the shared features go in
   sharedFeatureBits): if so it stays (with a remark now and then), else it's sent
   back to cross (returnDue) with a remark. Dropped elsewhere, it goes back
   where it was if that was in the waiting area, else to a free waiting
   place. */
/* @zoombi32 0x004203b3 */
void ferryClicked(short which)
{
    View *other;
    ShortRect bounds;
    Point where;
    ShortRect unused = {203, 261, 639, 408};
    short spot;
    short placed;
    Point target;
    Point from;
    short ok;
    short k;
    short f;
    View *view;

    if (sceneDue || ferryLeaving) {
        if (ferryLeaving)
            sceneDue = 11;
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeFerry();
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawFerryButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFerryButton(which, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (!ferryHasPassengers)
            break;
        queueViewSound(999, 0);
        if (returnDue)
            returnUnderway = 1;
        drawFerryButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFerryButton(which, 0, 1);
        ferryLeaving = 1;
        break;
    case 3:
        if (snoidsOnTheirWay > 0 || returnUnderway)
            break;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (!view || cajunLeavingGroup || ferryLeaving)
            break;
        placed = viewSnoid(view)->chosen;
        viewSnoid(view)->chosen = 0;
        returnPlace = *(Point *)&view->body.x;
        dragSnoid(view, where, 0, 0);
        unloadSounds();
        bounds = view->body.bounds;
        nextReturner = 0;
        returnPlaceView = heldPlaceNumber();
        if (returnPlaceView) {
            ok = 1;
            sharedFeatureBits = 0;
            for (k = 0; ok && k < 8; k++)
                if (ferryLinks[returnPlaceView - 1][k]) {
                    other = findView(placeClaims[ferryLinks[returnPlaceView - 1][k] - 1]);
                    if (other) {
                        ok = 0;
                        for (f = 0; f < 4; f++)
                            if (viewSnoid(other)->features[f] == viewSnoid(view)->features[f]) {
                                switch (f) {
                                case 0:
                                    sharedFeatureBits |= 1;
                                    break;
                                case 1:
                                    sharedFeatureBits |= 2;
                                    break;
                                case 2:
                                    sharedFeatureBits |= 4;
                                    break;
                                case 3:
                                    sharedFeatureBits |= 8;
                                    break;
                                }
                                ok = 1;
                            }
                    }
                }
            if (ok) {
                badPlacings = 0;
                if (!placed)
                    goodPlacings++;
                if (countChosenSnoids() + 1 == ferrySnoidCount || goodPlacings == nextPraiseAt) {
                    nextPraiseAt += randomBetween(3, 5);
                    if (!praisedOnce) {
                        praisedOnce = 1;
                        cajunRemarkDue = 1816;
                    } else {
                        cajunRemarkDue = goodPlacingRemarks[allocateSlot(&goodPlacingRemarksUsed, 2, 0)];
                    }
                }
                viewSnoid(view)->chosen = 1;
                if (sharedFeatureBits && practiceLevel)
                    sharedFeatureView = view->id;
            } else {
                badPlacings++;
                goodPlacings = 0;
                nextPraiseAt = 1;
                returnUnderway = 1;
                releaseHeldPlace();
                viewSnoid(view)->idleTicks = 1;
                nextReturner = view->id;
                returnPlaceView = ferryPlaceViews[returnPlaceView - 1];
                if (randomBetween(3, 5) == badPlacings) {
                    cajunRemarkDue = 1815;
                    badPlacings = 5;
                } else {
                    cajunRemarkDue = badPlacingRemarks[allocateSlot(&badPlacingRemarksUsed, 11, 0)];
                }
                returnDue = 1;
            }
        } else if (viewSnoid(view)->action == 4) {
            target = *(Point *)&viewSnoid(view)->targetX;
            from = *(Point *)&view->body.x;
            if (target.x != from.x || target.y != from.y) {
                ShortRect area = {0, 130, 469, 240};

                if (ptInRect(&area, returnPlace)) {
                    *(Point *)&viewSnoid(view)->targetX = returnPlace;
                } else {
                    findFerryPlace(&spot);
                    *(Point *)&viewSnoid(view)->targetX = ferryPlaces[spot];
                    placed = 0;
                }
                if (!cajunRemarkDue && nearPlacedView(from))
                    cajunRemarkDue = movedRemarks[allocateSlot(&movedRemarksUsed, 3, 0)];
            } else {
                placed = 0;
            }
            viewSnoid(view)->chosen = placed;
        }
        break;
    }
    ferryHasPassengers = countChosenSnoids();
}
