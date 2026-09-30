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
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Draws button `which` (1 or 2; 2 is dim unless g_4abaae), lit or not,
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
        if (!g_4abaae) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a147c->offsets[image] + (char *)g_4a147c), ferryButtons[which - 1].rect.left,
                      ferryButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&ferryButtons[which - 1].rect);
    }
}

/* The buttons' view update: redraws button 2 as g_4abaae changes, and
   button 1 once. */
/* @zoombi32 0x0041fea4 */
void updateFerryButtons(View *, short region)
{
    if (g_4abaae) {
        if (!g_4a1570) {
            g_4a1570 = 1;
            unionRgnRect(region, &ferryButtons[1].rect);
        }
    } else if (g_4a1570) {
        g_4a1570 = 0;
        unionRgnRect(region, &ferryButtons[1].rect);
    }
    if (!g_4a1572) {
        g_4a1572 = 1;
        unionRgnRect(region, &ferryButtons[0].rect);
    }
}

/* Closes scene 10. */
/* @zoombi32 0x0041ff16 */
void closeScene10()
{
    if (g_4abaac) {
        g_4abaac = g_4abaa2 = g_4abaa4 = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        freeResource(&g_4a151c);
        unloadSounds();
        if (ferryLinks) {
            disposePtr(ferryLinks);
            ferryLinks = 0;
        }
        setFreeAtOnce(saved);
        closeGameFile(&g_4abaa8);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Moves g_4abac0 to g_4abac2 and starts its view on g_4a1440's script
   for g_4abaee. */
/* @zoombi32 0x004209b8 */
void moveFerryOn()
{
    View *view;

    g_4abac2 = g_4abac0;
    g_4abac0 = 0;
    view = findView(g_4abac2);
    if (view) {
        view->flags &= ~0x800000;
        setViewScript(view, g_4a1440[g_4abaee], 1);
        view->flags |= 0x800000;
    }
}

/* Starts g_4abaf2's Zoombini on `script` (anchored at g_4aba98) in
   `group`, with `notify` if given. */
/* @zoombi32 0x00420a08 */
void startCrosserScript(short group, short script, ViewNotify notify, char unknownF8)
{
    View *view = findView(g_4abaf2);

    if (view) {
        startSnoidScript(viewSnoid(view), script, g_4aba98, unknownF8);
        view->body.group = group;
        loadViewSounds(g_4abaf2, 0);
        if (notify)
            view->notify = notify;
    }
}

/* A notify: 6 starts g_4ababe's script in this view's group; other events
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
        other = findView(g_4ababe);
        if (other) {
            setViewScript(other, 0, 1);
            other->body.group = view->body.group;
        }
        break;
    }
}

/* Moves the placed Zoombinis (flag 1 and unknownF7) and two kinds of
   view (flags 0x748c2000, 0x74980000) `dx` along, cels and all. */
/* @zoombi32 0x0042113f */
void slideFerryViews(View *, short dx)
{
    View *view;
    ViewCel *cel;

    for (view = viewListEnd(1)->next; view; view = view->next)
        if (view->flags == 1 && viewSnoid(view)->unknownF7 || view->flags == 0x748c2000 || view->flags == 0x74980000) {
            view->body.group = 0;
            view->body.x += dx;
            for (cel = view->body.cels; cel->image; cel++)
                cel->x += dx;
        }
}

/* Resets scene 10's state. */
/* @zoombi32 0x0041f8cc */
void resetScene10()
{
    g_4aba88 = 0;
    g_4b966e = 0;
    g_4abb10 = g_4abaf4 = 0;
    g_4abb08 = g_4abb0a = 0;
    g_4abb0c = 1;
    g_4aba90 = 0;
    g_4abaee = g_4abaf0 = g_4abaf2 = g_4abaa6 = 0;
    g_4aba98 = 0;
    g_4abab0 = g_4abaa2 = g_4abaa4 = 0;
    g_4abb06 = g_4abb04 = g_4abaa0 = g_4b0d52 = 0;
    g_4abab6 = g_4abab8 = g_4ababa = g_4ababc = g_4ababe = 0;
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

/* Starts the next Zoombini (g_4abaf0) on its way by `n` (g_4abaee; 0 or
   over 9: 0): Captain Cajun's view (g_4abab4) plays 1604-1607, and for 7-9
   the views g_4ababc and g_4ababe are made anew about g_4aba8c. */
/* @zoombi32 0x00420f85 */
void startNextCrosser(short n)
{
    View *view;
    Point at;

    g_4abaee = n;
    if (!g_4abaf0)
        return;
    g_4abaf2 = g_4abaf0;
    g_4abaf0 = 0;
    if (n >= 10)
        n = 0;
    view = findView(g_4abab4);
    if (!view)
        return;
    switch (n) {
    case 0:
        g_4abb16 = 1605;
        g_4aba92.x = 122;
        g_4aba92.y = 164;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        if (randomBetween(1, 100) <= 50)
            g_4abb16 = 1604;
        else
            g_4abb16 = 1606;
        g_4aba92 = g_4aba8c;
        break;
    case 7:
    case 8:
    case 9:
        g_4abb16 = 1607;
        deleteView(g_4ababc);
        deleteView(g_4ababe);
        g_4ababc = addView(0x1180000, drawCels, runViewScript, 1705, 6, 0, 0, -1);
        g_4aba92.x = 236;
        g_4aba92.y = 474;
        g_4aba9c = g_4aba8c;
        at.x = g_4aba9c.x - 14;
        at.y = g_4aba9c.y - 14;
        g_4ababe = addView(0x1980000, drawCels, runViewScript, 1706, 6, &at, 0, -1);
        break;
    }
    setViewScript(view, g_4abb16, 1);
    loadViewSounds(g_4abab4, 1);
    g_4aba98 = 0;
    view->notify = crosserNotify;
    groupViews(view->id, g_4abac0, g_4abaf2, 0, 0, 0);
    setViewsLocked(0);
}

/* A crossing Zoombini's notify (g_4abaf2, by g_4abaee): 1 walks it on
   (g_4a1454), 2 and 3 on again (g_4a1468; 2 also lets the views go, or
   puts it behind g_4ababc when that script is 1907), 4 sets it down facing
   left at (93, 408), 5 walks it off to g_4aba9c; 6 picks a sound
   (g_4a1424) if none is due. */
/* @zoombi32 0x00420a60 */
void crosserNotify(View *view, short event)
{
    View *other;

    switch (event) {
    case 1:
        moveFerryOn();
        startCrosserScript(view->body.group, g_4a1454[g_4abaee], crosserNotify, 1);
        break;
    case 2:
        if (g_4a1468[g_4abaee] == 1907) {
            moveView(g_4abaf2, 0, g_4ababc);
            g_4abaa4 = 1;
        } else {
            g_4aba98 = &g_4aba92;
            startCrosserScript(view->body.group, g_4a1468[g_4abaee], 0, 0);
            g_4aba98 = 0;
            g_4aba90 = 0;
            requestViewSort();
        }
        break;
    case 3:
        g_4aba98 = &g_4aba92;
        startCrosserScript(view->body.group, g_4a1468[g_4abaee], 0, 1);
        g_4aba98 = 0;
        break;
    case 6:
        if (!g_4abaa0)
            g_4abaa0 = g_4a1424[allocateSlot(&g_4a1430, 5, 0)];
        break;
    case 4:
        other = findView(g_4abaf2);
        if (other) {
            viewSnoid(other)->unknownF2 = 1;
            other->body.x = 93;
            other->body.y = 408;
            startSnoidScript(viewSnoid(other), viewSnoid(other)->features[3] * 2 + 998, 0, 0);
            other->notify = crosserNotify;
            other->body.group = view->body.group;
            requestViewSort();
        }
        break;
    case 5:
        other = findView(g_4abaf2);
        if (other) {
            viewSnoid(other)->unknownF2 = 0;
            startSnoidScript(viewSnoid(other), viewSnoid(other)->features[3] * 2 + 999, &g_4aba9c, 0);
            other->body.group = view->body.group;
            other->notify = ferryHelperNotify;
        }
        g_4abaf2 = 0;
        g_4aba90 = 0;
        break;
    }
}

/* Works out which of the placed views (g_4abac6) touch: for each, the
   others meeting its bounds grown or shrunk by half its height less 2 (and
   from level 3, g_4aba8a, widened), up to 8, into ferryLinks (from 1); with
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
        view = findView(g_4abac6[i]);
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
            if (!met && g_4aba8a >= 3) {
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

/* Scene 10's keys (with debugging on, g_4b8803, or else only 0x16f; case
   ignored): 0x16f fn_466b93; A draws the links between the places; L
   reports the level (from 1); F plays Captain Cajun's script g_4abb14
   (1800-1832; else his current one). Returns whether the key was used. */
/* @zoombi32 0x004208a3 */
short scene10Key(unsigned short key)
{
    short used = 0;
    ShortRect unused = {0, 0, 225, 18};
    View *view;

    if (!g_4b8803 && key != 0x16f)
        return 0;
    if (key >= 'a' && key <= 'z')
        key -= 32;
    switch (key) {
    case 0x16f:
        fn_466b93();
        used = 1;
        break;
    case 'A':
        linkFerryPlaces(1);
        used = 1;
        break;
    case 'L':
        debugMessage(g_4aba8a + 1, 0, 0, 0, 0);
        used = 1;
        break;
    case 'F':
        if (g_4abab2) {
            view = findView(g_4abab4);
            if (view) {
                if (g_4abb14 > 1832 || g_4abb14 < 1800)
                    g_4abb14 = view->kind;
                setViewScript(view, g_4abb14, 1);
                loadViewSounds(g_4abab4, 1);
                used = 1;
                debugMessage(g_4abb14, "Play FrogMan SCRB id:", 0, 0, 0);
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
 * g_4abac4 on), 4-10 scenery (unless g_4a4ba0's +0x20).
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
    after = g_4abac4;
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
            g_4abac6[count] = after;
            placedViewPoints[count].x = at.x + 22;
            placedViewPoints[count].y = at.y - 7;
            placedViews[count] = g_4abac6[count];
            g_4b83e4[count] = 0;
            count++;
        } else if (word >= 4 && word <= 10 && !*(short *)(g_4a4ba0 + 0x20)) {
            after = addView(0x74980000, drawCels, runViewScript, word + 1499, 6, &at, 1, after);
        }
    }
    placedViewCount = count;
    freeResource(&resource);
}

/* Lays out the places for the level (g_4aba8a, 0-4) and the number of
   Zoombinis (16-20, or g_4aba88): scripts 1510-1529. */
/* @zoombi32 0x004211a3 */
void layOutFerryLevel()
{
    short n;

    if (g_4aba8a < 0 || g_4aba8a > 4)
        g_4aba8a = 0;
    n = countChosenSnoids();
    if (g_4aba88)
        n = g_4aba88;
    if (n < 16 || n > 20)
        n = 16;
    n -= 16;
    switch (g_4aba8a) {
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
   Cajun (the first time 1803, then one of g_4a13e4), the views, the
   places for the level (layOutFerryLevel) and the party, and a hint or
   greeting. */
/* @zoombi32 0x0041f97c */
void openScene10()
{
    short i;

    g_4abaac = g_4abaae = 0;
    resetScene10();
    g_4abafc++;
    g_4aba8a = sceneLevel();
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
    openGameFile(&g_4abaa8, "Ferry.MHK");
    setCurrentMap(g_4abaa8);
    drawBackdrop(1300);
    g_4a147c = loadImageBank(1400, &g_4a151c);
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
    g_4abab2 = addView(0xc000, drawCels, runViewScript, 1601, 6, 0, 0, 0);
    if (g_4abafc == 1)
        g_4abab4 = 1803;
    else
        g_4abab4 = g_4a13e4[allocateSlot(&g_4a13ec, 4, 0)];
    g_4abab4 = addView(0x188000, drawCels, runViewScript, g_4abab4, 6, 0, 0, 0);
    g_4abaf2 = g_4abab4;
    if (!*(short *)(g_4a4ba0 + 0x20)) {
        g_4abab6 = addView(0x8000, drawCels, runViewScript, 1602, 6, 0, 0, 0);
        g_4abab8 = addView(0x8000, drawCels, runViewScript, 1603, 6, 0, 0, 0);
        pairViews(g_4abab6, g_4abab8);
    }
    g_4ababa = addView(0x1188000, drawCels, runViewScript, 1704, 6, 0, 0, 0);
    addView(0, drawCels, runViewScript, 1600, 6, 0, 0, 0);
    for (i = 0; i < 3; i++)
        g_4abac4 = addView(0x4000000, drawCels, runViewScript, i + 1450, 0, 0, 0, 0);
    g_4abac0 = g_4abac2 = 0;
    addView(0x1000, drawFerryButtons, updateFerryButtons, 0, 0, 0, 0, 0);
    setViewPlaces(20, ferryPlaces, 1);
    setViewsLocked(0);
    fn_4148da(10, 236);
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
    showRect(&g_4aa7b8);
    fadeInViews();
    queueViewSound(997, 0);
    chooseSnoids(0, 0);
    g_4abb0e = countSnoidViews();
    resetViewClock();
    g_4aba84 = randomBetween(5400, 10800);
    g_4abaac = 1;
    switch (campHint((short *)(g_4a4ba0 + 0x32))) {
    case 2:
        g_4b966e = 20074;
        break;
    default:
        if (g_4aba8a)
            g_4b966e = randomBetween(20073, 20074);
        else
            g_4b966e = 20073;
        break;
    }
}

/* Scene 10's frame: once everyone has crossed (g_4abaf4), Captain Cajun
   leaves (1608-1609) and so does the scene; leaves when asked; plays a
   remark due (g_4abaa0) or picks one now and then (g_4a13f0); starts the
   ferry's two views (g_4abaa4); sends the next Zoombini to a free place
   (g_4abaa2, startNextCrosser, some routes needing places free on the right);
   and greets once the sound 997 ends. */
/* @zoombi32 0x0041ff89 */
void scene10Frame()
{
    short spot;
    short again;
    short i;
    View *view;

    if (g_4a1574 || !g_4abaac)
        return;
    g_4a1574 = 1;
    if (g_4abaf4 && !g_4abaa2 && !g_4aba90) {
        g_4abaf4 = 0;
        deleteView(g_4abab2);
        g_4abab2 = 0;
        deleteView(g_4abab6);
        deleteView(g_4abab8);
        startView(g_4abab4, randomBetween(1608, 1609), slideFerryViews, 0);
        loadViewSounds(g_4abab4, 1);
        g_4abaa6 = groupViews(g_4abab4, g_4abab4, 0, 0, 0, 0);
        g_4b0d52 = 11;
    }
    updateViews();
    if (g_4b0d52) {
        if (!g_4b9688 || g_4b9688 == 3) {
            if (g_4b9688 == 3 && !g_4b754a)
                chooseSnoids(0, 0);
            if (viewsLocked || !groupLeader[g_4abaa6]) {
                g_4b0d50 = g_4b0d52;
                g_4b0d52 = 0;
                setCurrentMap(0);
                closeScene10();
                g_4a1574 = 0;
                return;
            }
        } else if (g_4b9688 == 2) {
            g_4b9688 = 0;
            g_4b0d52 = 0;
        }
    }
    if (g_4abaa0) {
        i = g_4abaa0;
        g_4abaa0 = 0;
        if (g_4abab2) {
            startView(g_4abab4, i, 0, 0);
            loadViewSounds(g_4abab4, 1);
            g_4abb12 = groupViews(g_4abab4, g_4abab4, 0, 0, 0, 0);
        }
    } else if (g_4abaa4) {
        g_4abaa4 = 0;
        startView(g_4ababa, 0, crosserNotify, 0);
        startView(g_4ababc, 0, crosserNotify, 0);
        groupViews(g_4ababa, g_4ababc, 0, 0, 0, 0);
    } else if (g_4abaa2) {
        if (!groupLeader[g_4abb12]) {
            g_4abaa2 = 0;
            findFerryPlace(&spot);
            for (again = 1; again;) {
                again = 0;
                g_4abaee = allocateSlot(&g_4abb00, 10, 0);
                switch (g_4abaee) {
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
            g_4aba8c = ferryPlaces[spot];
            startNextCrosser(g_4abaee);
        }
    } else if (viewClock() > g_4aba84) {
        resetViewClock();
        g_4abaa0 = g_4a13f0[allocateSlot(&g_4a13fc, 5, 0)];
        g_4aba84 = randomBetween(5400, 10800);
    }
    if (g_4abb06) {
        view = idleSnoidView(g_4abb06);
        if (view) {
            viewSnoid(view)->unknownF5 = g_4abb04;
            g_4abb04 = 0;
            g_4abb06 = 0;
        }
    }
    if (!g_4abab0 && !g_4b755a && !isSoundPlaying(997, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        g_4abab0 = 1;
        startView(g_4abab4, 0, 0, 0);
        loadViewSounds(g_4abab4, 1);
    }
    playAmbientSound();
    g_4a1574 = 0;
}

/* Scene 10's clicks: 1 leaves (asking whether to keep the party), 2 sets
   the ferry off (once there's a Zoombini aboard, g_4abaae), 3 drags a
   Zoombini. Put at a place, it must share a feature with every Zoombini
   at a place linked to it (ferryLinks; the shared features go in
   g_4abb04): if so it stays (with a remark now and then), else it's sent
   back to cross (g_4abaa2) with a remark. Dropped elsewhere, it goes back
   where it was if that was in the waiting area, else to a free waiting
   place. */
/* @zoombi32 0x004203b3 */
void scene10Clicked(short which)
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

    if (g_4b0d52 || g_4abaf4) {
        if (g_4abaf4)
            g_4b0d52 = 11;
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        setCurrentMap(0);
        closeScene10();
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawFerryButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFerryButton(which, 0, 1);
        g_4b0d52 = 1;
        askKeepParty();
        break;
    case 2:
        if (!g_4abaae)
            break;
        queueViewSound(999, 0);
        if (g_4abaa2)
            g_4aba90 = 1;
        drawFerryButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFerryButton(which, 0, 1);
        g_4abaf4 = 1;
        break;
    case 3:
        if (g_4b755a > 0 || g_4aba90)
            break;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (!view || g_4abaa6 || g_4abaf4)
            break;
        placed = viewSnoid(view)->unknownF7;
        viewSnoid(view)->unknownF7 = 0;
        g_4aba8c = *(Point *)&view->body.x;
        dragSnoid(view, where, 0, 0);
        unloadSounds();
        bounds = view->body.bounds;
        g_4abaf0 = 0;
        g_4abac0 = heldPlaceNumber();
        if (g_4abac0) {
            ok = 1;
            g_4abb04 = 0;
            for (k = 0; ok && k < 8; k++)
                if (ferryLinks[g_4abac0 - 1][k]) {
                    other = findView(g_4b83e4[ferryLinks[g_4abac0 - 1][k] - 1]);
                    if (other) {
                        ok = 0;
                        for (f = 0; f < 4; f++)
                            if (viewSnoid(other)->features[f] == viewSnoid(view)->features[f]) {
                                switch (f) {
                                case 0:
                                    g_4abb04 |= 1;
                                    break;
                                case 1:
                                    g_4abb04 |= 2;
                                    break;
                                case 2:
                                    g_4abb04 |= 4;
                                    break;
                                case 3:
                                    g_4abb04 |= 8;
                                    break;
                                }
                                ok = 1;
                            }
                    }
                }
            if (ok) {
                g_4abb0a = 0;
                if (!placed)
                    g_4abb08++;
                if (countChosenSnoids() + 1 == g_4abb0e || g_4abb08 == g_4abb0c) {
                    g_4abb0c += randomBetween(3, 5);
                    if (!g_4abb10) {
                        g_4abb10 = 1;
                        g_4abaa0 = 1816;
                    } else {
                        g_4abaa0 = g_4a1400[allocateSlot(&g_4a1404, 2, 0)];
                    }
                }
                viewSnoid(view)->unknownF7 = 1;
                if (g_4abb04 && g_4b754a)
                    g_4abb06 = view->id;
            } else {
                g_4abb0a++;
                g_4abb08 = 0;
                g_4abb0c = 1;
                g_4aba90 = 1;
                releaseHeldPlace();
                viewSnoid(view)->unknownF8 = 1;
                g_4abaf0 = view->id;
                g_4abac0 = g_4abac6[g_4abac0 - 1];
                if (randomBetween(3, 5) == g_4abb0a) {
                    g_4abaa0 = 1815;
                    g_4abb0a = 5;
                } else {
                    g_4abaa0 = g_4a1408[allocateSlot(&g_4a1420, 11, 0)];
                }
                g_4abaa2 = 1;
            }
        } else if (viewSnoid(view)->unknownF4 == 4) {
            target = *(Point *)&viewSnoid(view)->targetX;
            from = *(Point *)&view->body.x;
            if (target.x != from.x || target.y != from.y) {
                ShortRect area = {0, 130, 469, 240};

                if (ptInRect(&area, g_4aba8c)) {
                    *(Point *)&viewSnoid(view)->targetX = g_4aba8c;
                } else {
                    findFerryPlace(&spot);
                    *(Point *)&viewSnoid(view)->targetX = ferryPlaces[spot];
                    placed = 0;
                }
                if (!g_4abaa0 && nearPlacedView(from))
                    g_4abaa0 = g_4a1434[allocateSlot(&g_4a143c, 3, 0)];
            } else {
                placed = 0;
            }
            viewSnoid(view)->unknownF7 = placed;
        }
        break;
    }
    g_4abaae = countChosenSnoids();
}
