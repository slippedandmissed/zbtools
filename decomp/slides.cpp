/*
 * slides (0x446bf8-0x44b550): Stone Rise (scene 12), 'Slides.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "random.h"
#include "slides.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Closes scene 12. */
/* @zoombi32 0x00447124 */
void closeScene12()
{
    if (g_4b1930) {
        g_4b1930 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a3fc8);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b1928);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The buttons' view update: redraws button 2 when g_4b1932 changes, and
   button 1 the first time. */
/* @zoombi32 0x004470b2 */
void fn_4470b2(View *, short region)
{
    if (g_4b1932) {
        if (!g_4a41e0) {
            g_4a41e0 = 1;
            unionRgnRect(region, &slidesButtons[1].rect);
        }
    } else if (g_4a41e0) {
        g_4a41e0 = 0;
        unionRgnRect(region, &slidesButtons[1].rect);
    }
    if (!g_4a41e2) {
        g_4a41e2 = 1;
        unionRgnRect(region, &slidesButtons[0].rect);
    }
}

/* Marks the Zoombini on each cell in state 508 (unknownF7). */
/* @zoombi32 0x0044943b */
void fn_44943b()
{
    short i;

    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508)
            ((Snoid *)&findView(g_4b1aea[i].snoid)->body)->unknownF7 = 1;
}

/* Sets g_4b1932 if any of the cells g_4b1ab4 lists (1 to g_4b240e) is in
   state 508. */
/* @zoombi32 0x00449475 */
void fn_449475()
{
    short i;

    g_4b1932 = 0;
    for (i = 1; i <= g_4b240e; i++)
        if (g_4b1aea[g_4b1ab4[i]].state == 508) {
            g_4b1932 = 1;
            return;
        }
}

/* Counts the cells in state 502 or 508, and sums their numbers into
   g_4b1a44. */
/* @zoombi32 0x0044b261 */
short fn_44b261()
{
    short n;
    short i;

    n = g_4b1a44 = 0;
    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 502 || g_4b1aea[i].state == 508) {
            n++;
            g_4b1a44 += i;
        }
    return n;
}

/* Remarks on the count of cells in state 502 or 508 against the last
   (g_4b1a42): up (more than four: 8505, else 8504, and sets g_4b1932), down
   (8501 or 8500), or the same count on other cells (8502). A long remark
   already playing is stopped first. */
/* @zoombi32 0x0044b2a4 */
void fn_44b2a4()
{
    short n = fn_44b261();

    if (n > g_4b1a42) {
        g_4b1932 = 1;
        if (n - g_4b1a42 > 4) {
            if (g_4b87fe && lastViewSound == 8505) {
                stopSounds(8505, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8505, 0);
        } else {
            if (g_4b87fe && lastViewSound == 8504) {
                stopSounds(8504, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8504, 0);
        }
    } else if (n < g_4b1a42) {
        if (g_4b1a42 - n > 4) {
            if (g_4b87fe && lastViewSound == 8501) {
                stopSounds(8501, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8501, 0);
        } else {
            if (g_4b87fe && lastViewSound == 8500) {
                stopSounds(8500, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8500, 0);
        }
    } else if (g_4b1a44 != g_4b1a46) {
        queueViewSound(8502, 0);
    }
}

/* Draws button `which` (1 or 2; 2 is dim unless g_4b1932), lit or not,
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
        if (!g_4b1932) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a3fc4->offsets[image] + (char *)g_4a3fc4), slidesButtons[which - 1].rect.left,
                      slidesButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&slidesButtons[which - 1].rect);
    }
}

/* Counts, for each feature, how many of its values the chosen Zoombinis
   (g_4b192c, g_4b2414 of them) show, into g_4b251c. */
/* @zoombi32 0x004488e8 */
void fn_4488e8()
{
    short counts[4][6];
    short i;
    short j;

    g_4b251c[0] = 0;
    g_4b251c[1] = 0;
    g_4b251c[2] = 0;
    g_4b251c[3] = 0;
    g_4b192c = listChosenSnoids();
    g_4b2414 = g_4b192c->count;
    fillMemory(counts, 0, sizeof counts);
    for (i = 0; i < g_4b2414; i++)
        for (j = 0; j < 4; j++)
            counts[j][g_4b192c->features[i][j]]++;
    for (i = 0; i < 4; i++)
        for (j = 1; j < 6; j++)
            if (counts[i][j])
                g_4b251c[i]++;
}

/* Cycles palette colours 19-21 by one (the last to the first). */
/* @zoombi32 0x00448bf5 */
void fn_448bf5()
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

/* Clears the party's features (partyHair to g_4b24f2, g_4b2452, g_4b2430) and
   reads each Zoombini's (g_4b2414 of them, partyViews) into partyHair to partyFeet. */
/* @zoombi32 0x00449b40 */
void fn_449b40()
{
    Snoid *snoid;
    short i;

    fillMemory(partyHair, 0, 32);
    fillMemory(partyEyes, 0, 32);
    fillMemory(partyNoses, 0, 32);
    fillMemory(partyFeet, 0, 32);
    fillMemory(g_4b24f2, 0, 32);
    fillMemory(g_4b2452, 0, 32);
    fillMemory(g_4b2430, 0, 32);
    g_4b2450 = 0;
    for (i = 0; i < g_4b2414; i++) {
        snoid = (Snoid *)&findView(partyViews[i])->body;
        partyHair[i] = snoid->features[0];
        partyEyes[i] = snoid->features[1];
        partyNoses[i] = snoid->features[2];
        partyFeet[i] = snoid->features[3];
    }
}

/* Orders the party (into g_4b2430) by how many others share a feature with
   each, most first. */
/* @zoombi32 0x00449c18 */
void fn_449c18()
{
    short alike[16];
    short i;
    short j;
    short best;
    short most;

    fillMemory(alike, 0, sizeof alike);
    for (i = 0; i < g_4b2414; i++)
        for (j = 0; j < g_4b2414; j++)
            if (partyHair[i] == partyHair[j] || partyEyes[i] == partyEyes[j]
                || partyNoses[i] == partyNoses[j] || partyFeet[i] == partyFeet[j])
                alike[i]++;
    for (i = 0; i < g_4b2414; i++) {
        most = best = 0;
        for (j = 0; j < g_4b2414; j++)
            if (most < alike[j]) {
                most = alike[j];
                best = j;
            }
        g_4b2430[i] = best;
        alike[best] = -1;
    }
}

/* A cell's placed callback: moves its cels into place and drops the
   images 4-24 for the directions its cell (g_4b1ab4, by the view's place)
   has no link in (g_4b2324's bits). */
/* @zoombi32 0x00448c81 */
void fn_448c81(View *view)
{
    ViewCel *cel;
    short cell;
    short removed;

    cell = g_4b1ab4[(short)(view->id - placedViews[0]) + 1];
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
        cel->x += -22;
        cel->y += 6;
        switch (cel->image) {
        case 4:
            if (!(g_4b2324[cell] & 1)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 8:
            if (!(g_4b2324[cell] & 2)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 12:
            if (!(g_4b2324[cell] & 4)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 16:
            if (!(g_4b2324[cell] & 8)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 20:
            if (!(g_4b2324[cell] & 0x10)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 24:
            if (!(g_4b2324[cell] & 0x20)) {
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
void fn_44b3ee(Point *where)
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

/* Another Zoombini of the party (not `who`, and not marked in g_4b24f2)
   with the same value of a feature (g_4b2516, each in turn from a random
   one); -1 if none. */
/* @zoombi32 0x00449a21 */
short fn_449a21(short who)
{
    short tries;
    short i;

    g_4b2516 = randomUpTo(3);
    tries = 4;
    do {
        if (++g_4b2516 > 3)
            g_4b2516 = 0;
        for (i = 0; i < g_4b2414; i++) {
            if (i == who)
                continue;
            switch (g_4b2516) {
            case 0:
                if (!g_4b24f2[i] && partyHair[who] == partyHair[i])
                    return i;
                break;
            case 1:
                if (!g_4b24f2[i] && partyEyes[who] == partyEyes[i])
                    return i;
                break;
            case 2:
                if (!g_4b24f2[i] && partyNoses[who] == partyNoses[i])
                    return i;
                break;
            case 3:
                if (!g_4b24f2[i] && partyFeet[who] == partyFeet[i])
                    return i;
                break;
            }
        }
    } while (--tries);
    return -1;
}

/* Layers the Zoombinis' views: those on the listed cells (g_4b1ab4) in state
   507 go behind the next cell's view; the party's Zoombinis on none of them
   in state 507 or 508 go behind the first cell's. */
/* @zoombi32 0x0044aa79 */
void fn_44aa79()
{
    short placed[16];
    View *view;
    short i;
    short j;

    setViewsLocked(0);
    fillMemory(placed, 0, sizeof placed);
    for (i = 1; i <= g_4b240e; i++) {
        if (g_4b1aea[g_4b1ab4[i]].state == 507) {
            view = findView(g_4b1aea[g_4b1ab4[i]].snoid);
            if (view) {
                view->flags |= 0x4008000;
                moveView(g_4b1aea[g_4b1ab4[i]].snoid, 0, g_4b1aea[g_4b1ab4[i] + 1].view);
            }
            for (j = 0; j < 16; j++)
                if (partyViews[j] == g_4b1aea[g_4b1ab4[i]].snoid)
                    placed[j]++;
        } else if (g_4b1aea[g_4b1ab4[i]].state == 508) {
            for (j = 0; j < 16; j++)
                if (partyViews[j] == g_4b1aea[g_4b1ab4[i]].snoid)
                    placed[j]++;
        }
    }
    for (i = 0; i < 16; i++)
        if (!placed[i]) {
            view = findView(partyViews[i]);
            if (view) {
                view->flags |= 0x4008000;
                moveView(partyViews[i], 0, g_4b1aea[0].view);
            }
        }
}

/* A cell's placed callback: keeps image 109 only on cells in state 502, 504
   or 508 unless g_4b2512 is 505, and image 110 on cells in state 502, 505
   or 508 while g_4b2512 is 505; images 4, 8 and 24 only where the cell has
   that link (g_4b2324), moved on by g_4b2514. */
/* @zoombi32 0x00448d9d */
void fn_448d9d(View *view)
{
    ViewCel *cel;
    short removed;
    short cell;

    cell = view->id - g_4b1aea[0].view;
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
        switch (cel->image) {
        case 109:
            if (!((g_4b1aea[cell].state == 502 || g_4b1aea[cell].state == 504 || g_4b1aea[cell].state == 508)
                  && g_4b2512 != 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 110:
            if (!((g_4b1aea[cell].state == 502 || g_4b1aea[cell].state == 505 || g_4b1aea[cell].state == 508)
                  && g_4b2512 == 505)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 4:
            if (!(g_4b2324[cell] & 1)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += g_4b2514;
            }
            break;
        case 8:
            if (!(g_4b2324[cell] & 2)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += g_4b2514;
            }
            break;
        case 24:
            if (!(g_4b2324[cell] & 0x20)) {
                removeFirstCel(cel);
                removed++;
            } else {
                cel->image += g_4b2514;
            }
            break;
        }
        if (!removed)
            cel++;
    }
}
