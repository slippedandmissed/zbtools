/*
 * slides (0x446bf8-0x44b550): Stone Rise (scene 12), 'Slides.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "net.h"
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

/* The buttons' view's draw callback: draws both buttons, dim. */
/* @zoombi32 0x00447095 */
void drawSlidesButtons(View *)
{
    drawSlidesButton(1, 0, 0);
    drawSlidesButton(2, 0, 0);
}

/* A view's update cycling colours 19-21 at its interval. */
/* @zoombi32 0x004489a8 */
void fn_4489a8(View *view, short)
{
    if (view->nextUpdate <= updateTime) {
        view->nextUpdate = updateTime + view->interval;
        fn_448bf5();
    }
}

/* Marks the cell at (x, y) (within 45 by 22 of its point in cellPoints),
   unless one is already (g_4b1a3e): notes it in g_4b1a36 and shows the
   marker view g_4b1a34 there (script 8000-8002 by row, notify fn_44af15),
   layered with the row. */
/* Not exact: register allocation in the loop's tests (the original keeps
   `y` in ecx and uses esi for scratch; here `y` shares esi with `kind`). */
/* @zoombi32 0x0044b0fc */
void fn_44b0fc(short x, short y)
{
    Point at;
    Point *points = cellPoints;
    short i;
    short kind;

    if (!g_4b1a3e) {
        for (i = 0; i < 117; i++) {
            if (y >= points[i].y - 22 && y <= points[i].y && x >= points[i].x
                && x <= points[i].x + 45) {
                g_4b1a3e = 1;
                kind = 0;
                g_4b1a36 = i;
                if (i < 36)
                    kind = 2;
                else if (i < 89)
                    kind = 1;
                at.x = points[i].x + 19;
                at.y = points[i].y + 24;
                if (g_4b1a34)
                    deleteView(g_4b1a34);
                g_4b1a34 = addView(0x908000, drawCels, runViewScript, kind + 8000, 6, &at, 0, 0);
                findView(g_4b1a34)->notify = fn_44af15;
                if (i % 18)
                    moveView(g_4b1a34, 0, g_4b1936[7]);
                else {
                    short row = i / 18 + 1;

                    moveView(g_4b1a34, 0, g_4b1936[row]);
                }
                return;
            }
        }
    }
}

/* The notify of the Zoombini walking to the marked cell (the view
   g_4b1a38): 90-92 walk it on (scripts 14000-14002) toward its own place
   raised 50, 93 off to a random spot (14003, unmarking the cell); 240-243
   note a facing to take at the end (g_4b1a3a), 250-253 face it at once; the
   end (0) flips it and takes the noted facing. */
/* @zoombi32 0x0044af15 */
void fn_44af15(View *view, short event)
{
    Point at;
    View *walker;
    Snoid *snoid;

    snoid = viewSnoid(view);
    walker = findView(g_4b1a38);
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
        g_4b1a3a = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b1a3a) {
            setSnoidFacing(snoid, g_4b1a3a - 1);
            g_4b1a3a = 0;
        }
        break;
    case 90:
        walker = findView(g_4b1a38);
        startSnoidScript(viewSnoid(walker), 14000, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = fn_44af15;
        g_4b1a3e = 1;
        break;
    case 91:
        walker = findView(g_4b1a38);
        startSnoidScript(viewSnoid(walker), 14001, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = fn_44af15;
        g_4b1a3e = 1;
        break;
    case 92:
        walker = findView(g_4b1a38);
        startSnoidScript(viewSnoid(walker), 14002, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = fn_44af15;
        g_4b1a3e = 1;
        break;
    case 93:
        walker = findView(g_4b1a38);
        at.x = randomUpTo(42) + 70;
        at.y = randomUpTo(200) + 152;
        startSnoidScript(viewSnoid(walker), 14003, &at, 0);
        walker->notifyEnd = 0;
        walker->notify = fn_44af15;
        g_4b1a3e = 0;
        break;
    }
}

/* Groups the party in threes (g_4b2450 groups; g_4b24f2 marks those
   taken): each next Zoombini shares a feature with the last where one can
   (noted in g_4b2452 as 510-513 by feature, else 501). */
/* @zoombi32 0x0044986f */
void fn_44986f()
{
    short group;
    short n;
    short who;
    short other;
    short i;

    who = n = 0;
    fillMemory(g_4b2452, 0, 32);
    fillMemory(g_4b24f2, 0, 32);
    g_4b2450 = g_4b2414 / 3;
    if (g_4b2414 % 3)
        g_4b2450++;
    for (group = 0; group < g_4b2450; group++) {
        g_4b24f2[who] = 1;
        other = fn_449a21(who);
        if (other == -1) {
            g_4b2452[n] = 501;
            n++;
            for (i = 1; i < g_4b2414; i++)
                if (!g_4b24f2[i]) {
                    other = i;
                    g_4b24f2[other] = 1;
                    break;
                }
        } else {
            g_4b24f2[other] = 1;
            g_4b2452[n] = g_4b2516 + 510;
            n++;
        }
        who = other;
        other = -1;
        for (i = 1; i < g_4b2414; i++)
            if (!g_4b24f2[i])
                other = i;
        if (other == -1)
            return;
        other = fn_449a21(who);
        if (other == -1) {
            g_4b2452[n] = 501;
            n++;
            for (i = 1; i < g_4b2414; i++)
                if (!g_4b24f2[i]) {
                    other = i;
                    who = i;
                }
        } else {
            g_4b24f2[other] = 1;
            g_4b2452[n] = g_4b2516 + 510;
            n++;
        }
        g_4b24f2[other] = 1;
        other = -1;
        for (i = 1; i < g_4b2414; i++)
            if (!g_4b24f2[i]) {
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
   shares ebx with `feet`), as in fn_44b0fc. */
/* @zoombi32 0x00449f96 */
short fn_449f96(short a, short b)
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

    if (g_4b1aea[a].state == 500 || g_4b1aea[b].state == 500)
        return 0;
    if (!g_4b1aea[a].snoid || !g_4b1aea[b].snoid)
        return 0;
    snoid = (Snoid *)&findView(g_4b1aea[a].snoid)->body;
    hair = snoid->features[0];
    eyes = snoid->features[1];
    nose = snoid->features[2];
    feet = snoid->features[3];
    snoid = (Snoid *)&findView(g_4b1aea[b].snoid)->body;
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
   has that link (g_4b2324), 73-76 only on a cell whose Zoombini field holds
   the matching shared feature (513, 510, 512, 511), 103 on cells in state
   506, and 109 and 110 as fn_448d9d does. */
/* @zoombi32 0x004489ce */
void fn_4489ce(View *view)
{
    ViewCel *cel;
    short removed;
    short cell;

    cell = view->id - g_4b1aea[0].view;
    cel = view->body.cels;
    while (cel->image) {
        removed = 0;
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
        case 24:
            if (!(g_4b2324[cell] & 0x20)) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 74:
            if (g_4b1aea[cell].snoid != 510) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 76:
            if (g_4b1aea[cell].snoid != 511) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 75:
            if (g_4b1aea[cell].snoid != 512) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        case 73:
            if (g_4b1aea[cell].snoid != 513) {
                removeFirstCel(cel);
                removed++;
            }
            break;
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
        case 103:
            if (g_4b1aea[cell].state != 506) {
                removeFirstCel(cel);
                removed++;
            }
            break;
        }
        if (!removed)
            cel++;
    }
}

/* Puts the next Zoombini of the party (from the end of g_4b2430) two cells
   from `cell` in direction `dir` (0-5 straight on, 6-9 turning), if it
   shares no feature with the Zoombini on `cell` among the features tried
   (from a random one): the cell between records the feature it was tried
   on (state 501, 510-513) and the far cell takes it (state 507). Returns
   its place in g_4b2430, or -1. */
/* @zoombi32 0x00449cfc */
short fn_449cfc(short cell, short dir)
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
        via = g_4b1aea[cell].links[dir];
        to = g_4b1aea[via].links[dir];
    } else {
        switch (dir) {
        case 6:
            via = g_4b1aea[cell].links[0];
            to = g_4b1aea[via].links[1];
            break;
        case 7:
            via = g_4b1aea[cell].links[2];
            to = g_4b1aea[via].links[1];
            break;
        case 8:
            via = g_4b1aea[cell].links[5];
            to = g_4b1aea[via].links[4];
            break;
        case 9:
            via = g_4b1aea[cell].links[3];
            to = g_4b1aea[via].links[4];
            break;
        }
    }
    snoid = (Snoid *)&findView(g_4b1aea[cell].snoid)->body;
    hair = snoid->features[0];
    eyes = snoid->features[1];
    nose = snoid->features[2];
    feet = snoid->features[3];
    different = 1;
    for (dir = g_4b2414 - 1; dir >= 0; dir--) {
        if (g_4b2430[dir] == -1)
            continue;
        snoid = (Snoid *)&findView(partyViews[g_4b2430[dir]])->body;
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
            g_4b1aea[to].state = 507;
            g_4b1aea[to].snoid = partyViews[g_4b2430[dir]];
            g_4b1aea[via].state = 501;
            g_4b1aea[via].snoid = feature + 510;
            g_4b2430[dir] = -1;
            return dir;
        }
    }
    return -1;
}

/* Scene 12's frame: leaves after a choice (g_4b0d52) once the sound and the
   Zoombinis are done; cycles colours (g_4b1a3c) every 6 ticks; when the
   group g_4b251a has arrived, sends the Zoombinis on the finished cells
   off (by the level, g_4b1934) and ends; and has an idle Zoombini fidget
   now and then while g_4b2540. */
/* @zoombi32 0x00447171 */
void scene12Frame()
{
    View *view;
    short done;
    short tries;

    if (!g_4a41e4 && g_4b1930) {
        g_4a41e4 = 1;
        updateViews();
        if (g_4b0d52) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                g_4a41e4 = 0;
                return;
            }
            if (!g_4b9688 || g_4b9688 == 3) {
                if (g_4b9688 == 3)
                    chooseSnoids(0, 0);
                if (viewsLocked || !g_4b755a || g_4b755c >= 1) {
                    g_4b0d50 = g_4b0d52;
                    g_4b0d52 = 0;
                    fn_46be2e(0);
                    closeScene12();
                    g_4a41e4 = 0;
                    return;
                }
            } else if (g_4b9688 == 2) {
                g_4b9688 = 0;
                g_4b0d52 = 0;
            }
        }
        if (g_4b1a3c && clockTime() - g_4b2534 > 6) {
            fn_448bf5();
            g_4b2534 = clockTime();
        }
        if (g_4b251a && !groupLeader[g_4b251a]) {
            queueViewSound(7001, 0);
            updateViews();
            waitForEventFor(0, 60, 0, 1);
            queueViewSound(996, 0);
            g_4b251a = 0;
            if (g_4b1934 == 3) {
                chooseSnoids(0, 0);
                if (g_4b1aea[55].state == 508)
                    ((Snoid *)&findView(g_4b1aea[55].snoid)->body)->unknownF7 = 1;
                if (g_4b1aea[38].state == 508 && g_4b1aea[46].state == 502)
                    ((Snoid *)&findView(g_4b1aea[38].snoid)->body)->unknownF7 = 1;
                if (g_4b1aea[74].state == 508 && g_4b1aea[64].state == 502)
                    ((Snoid *)&findView(g_4b1aea[74].snoid)->body)->unknownF7 = 1;
                sendSnoids(800, 200, 45);
                fn_44943b();
            } else if (g_4b1934 <= 1) {
                sendSnoids(1280, 240, 45);
            } else {
                chooseSnoids(0, 0);
                if (g_4b1aea[19].state == 508)
                    ((Snoid *)&findView(g_4b1aea[19].snoid)->body)->unknownF7 = 1;
                if (g_4b1aea[55].state == 508)
                    ((Snoid *)&findView(g_4b1aea[55].snoid)->body)->unknownF7 = 1;
                if (g_4b1aea[91].state == 508)
                    ((Snoid *)&findView(g_4b1aea[91].snoid)->body)->unknownF7 = 1;
                sendSnoids(800, 200, 45);
                fn_44943b();
            }
            g_4b0d52 = 5;
        }
        if (!g_4b2542 && g_4b2540 && g_4b253e < g_4b253c) {
            g_4b2542++;
            if (clockTime() - g_4b252c > 30) {
                done = 0;
                tries = 0;
                g_4b252c = clockTime();
                do {
                    view = idleSnoidView(partyViews[allocateSlot(&g_4b2538, g_4b2414, 0)]);
                    if (view && view->body.running && view->flags == 1) {
                        int script = ((Snoid *)&view->body)->features[3] - 1;

                        script += 13001;
                        startSnoidScript((Snoid *)&view->body, script, 0, 0);
                        g_4b253e++;
                        done = 1;
                    } else if (++tries > 20) {
                        done = 1;
                    }
                } while (!done);
            }
        } else if (g_4b253e >= g_4b253c) {
            g_4b253e = g_4b2540 = g_4b252c = g_4b2538 = 0;
        }
        playAmbientSound();
        g_4a41e4 = 0;
    }
}

/* Pairs up the party by shared features (g_4b24f2 marks those paired, 99
   those left alone; g_4b2452 notes each pair's feature, 510-513, or 501):
   each tries the features in turn from a random one. Unless all are paired
   (or one, with an odd party), the lonely ones are swapped with the first
   and it tries again, up to ten rounds. */
/* @zoombi32 0x004494b3 */
void fn_4494b3()
{
    short feature;
    short lonely;
    short done;
    short rounds;
    short i;
    short j;
    short tries;
    short swap;
    short *paired = g_4b24f2;

    feature = randomUpTo(3);
    lonely = rounds = done = 0;
    do {
        fillMemory(g_4b2452, 0, 32);
        fillMemory(paired, 0, 32);
        g_4b2450 = 0;
        for (i = 0; i < g_4b2414; i++) {
            tries = 4;
            if (paired[i])
                continue;
            do {
                if (++feature > 3)
                    feature = 0;
                for (j = i + 1; j < g_4b2414; j++) {
                    switch (feature) {
                    case 0:
                        if (!paired[j] && partyHair[i] == partyHair[j]) {
                            g_4b2452[g_4b2450] = 510;
                            g_4b2450++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 1:
                        if (!paired[j] && partyEyes[i] == partyEyes[j]) {
                            g_4b2452[g_4b2450] = 511;
                            g_4b2450++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 2:
                        if (!paired[j] && partyNoses[i] == partyNoses[j]) {
                            g_4b2452[g_4b2450] = 512;
                            g_4b2450++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    case 3:
                        if (!paired[j] && partyFeet[i] == partyFeet[j]) {
                            g_4b2452[g_4b2450] = 513;
                            g_4b2450++;
                            paired[i] = paired[j] = 1;
                            j = 20;
                        }
                        break;
                    }
                }
                tries--;
                if (!tries && !paired[i]) {
                    paired[i] = 99;
                    g_4b2452[g_4b2450] = 501;
                    g_4b2450++;
                    lonely++;
                }
            } while (tries && !paired[i]);
        }
        if (!lonely) {
            done++;
        } else if (lonely == 1 && g_4b2414 % 2) {
            done++;
        } else {
            for (i = g_4b2414 - 1; i >= 0; i--) {
                if (paired[i] != 99)
                    continue;
                for (j = 0; j < g_4b2414; j++) {
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
   fn_44b0fc; here it takes esi, which shifts the other variables' registers
   and stack slots). */
/* @zoombi32 0x0044a674 */
void fn_44a674(short from, short via, short to)
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
    if (g_4b1aea[via].state != 501)
        return;
    code = g_4b1aea[via].snoid;
    if (code < 510) {
        if (g_4b1aea[from].state != 507 && g_4b1aea[from].state != 508 && g_4b1aea[from].state != 502)
            return;
        g_4b1aea[via].state = 502;
        view = findView(g_4b1aea[via].view);
        setViewScript(view, 7000, 1);
        view->placed = fn_4489ce;
        if (to == -1)
            return;
        if (g_4b1aea[to].state == 507 || g_4b1aea[to].state == 508) {
            g_4b1aea[to].state = 508;
            view = findView(g_4b1aea[to].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
            return;
        }
        if (g_4b1aea[to].state == 501) {
            g_4b1aea[to].state = 502;
            view = findView(g_4b1aea[to].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
        }
        return;
    }
    if (to != -1 && (g_4b1aea[from].state == 507 || g_4b1aea[from].state == 508)
        && (g_4b1aea[to].state == 507 || g_4b1aea[to].state == 508)) {
        snoid = (Snoid *)&findView(g_4b1aea[from].snoid)->body;
        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
        snoid = (Snoid *)&findView(g_4b1aea[to].snoid)->body;
        otherHair = snoid->features[0];
        otherEyes = snoid->features[1];
        otherNose = snoid->features[2];
        otherFeet = snoid->features[3];
        if (code < 510 || g_4b1aea[via].state == 502) {
            g_4b1aea[to].state = 508;
            view = findView(g_4b1aea[to].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
            g_4b1aea[via].state = 502;
            view = findView(g_4b1aea[via].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
        } else if ((code == 510 && otherHair == hair) || (code == 511 && otherEyes == eyes)
                   || (code == 512 && nose == otherNose) || (code == 513 && otherFeet == feet)) {
            g_4b1aea[to].state = 508;
            view = findView(g_4b1aea[to].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
            g_4b1aea[via].state = 502;
            view = findView(g_4b1aea[via].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
        }
    } else if (to != -1 && (g_4b1aea[to].state == 502 || g_4b1aea[to].state == 501)) {
        g_4b1aea[to].state = 502;
        view = findView(g_4b1aea[to].view);
        setViewScript(view, 7000, 1);
        view->placed = fn_4489ce;
        if (code < 510 && code != 500) {
            g_4b1aea[via].state = 502;
            view = findView(g_4b1aea[via].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
        }
    }
}

/* Seats the next two Zoombinis of the party (g_4b2430) on `cell` and the
   cell five on (state 507), then from each places more two cells on
   (fn_449cfc), turning (directions 8 and 9, 6 and 7). */
/* @zoombi32 0x0044abce */
void fn_44abce(short cell)
{
    short i;

    for (i = 0; i < g_4b2414; i++)
        if (g_4b2430[i] != -1) {
            g_4b1aea[cell].state = 507;
            g_4b1aea[cell].snoid = partyViews[g_4b2430[i]];
            g_4b2430[i] = -1;
            break;
        }
    for (i = 0; i < g_4b2414; i++)
        if (g_4b2430[i] != -1) {
            g_4b1aea[cell + 5].state = 507;
            g_4b1aea[cell + 5].snoid = partyViews[g_4b2430[i]];
            g_4b2430[i] = -1;
            break;
        }
    if (g_4b1aea[cell].state == 507 && fn_449cfc(cell, 8) != -1)
        fn_449cfc(cell, 9);
    if (g_4b1aea[cell + 5].state == 507 && fn_449cfc(cell + 5, 6) != -1)
        fn_449cfc(cell + 5, 7);
}

/* Lights the path: from the listed cell g_4b1ab4[g_4b2410] (taken, 507),
   finds the cell before the first in state g_4b2512, then walks back along
   the links: Zoombinis' cells go to 508; a feature stone (510-513) lights
   when the Zoombinis on either side share its feature; a plain stone
   lights on level 1 after a Zoombini's cell. The walk ends at an empty or
   blocked cell, or a stone that doesn't light. */
/* Not exact: register allocation (the original keeps `view` in esi and the
   board's address in edi, the other way round, and keeps the first
   findView's result). */
/* @zoombi32 0x00448f02 */
void fn_448f02()
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

    cell = g_4b1ab4[g_4b2410];
    view = findView(g_4b1aea[cell].snoid);
    g_4b1aea[cell].state = 507;
    view = findView(g_4b1aea[cell].view);
    setViewScript(view, 7000, 1);
    view->placed = fn_4489ce;
    found = 0;
    do {
        start = cell;
        if (g_4b1aea[cell].links[0] != -1)
            cell = g_4b1aea[cell].links[0];
        else if (g_4b1aea[cell].links[1] != -1)
            cell = g_4b1aea[cell].links[1];
        else if (g_4b1aea[cell].links[2] != -1)
            cell = g_4b1aea[cell].links[2];
        if (g_4b1aea[cell].state == g_4b2512) {
            found++;
            cell = start;
        }
    } while (!found);
    done = 0;
    do {
        switch (g_4b1aea[cell].state) {
        case 500:
        case 506:
            done++;
            break;
        case 507:
            g_4b1aea[cell].state = 508;
            view = findView(g_4b1aea[cell].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
            break;
        case 501:
            if (g_4b1aea[cell].snoid >= 510 && g_4b1aea[cell].snoid <= 513) {
                if (g_4b1aea[cell].links[5] != -1)
                    back = g_4b1aea[cell].links[5];
                else if (g_4b1aea[cell].links[4] != -1)
                    back = g_4b1aea[cell].links[4];
                else if (g_4b1aea[cell].links[3] != -1)
                    back = g_4b1aea[cell].links[3];
                if (g_4b1aea[back].state != 507 && g_4b1aea[back].state != 508) {
                    done++;
                    break;
                }
                if (g_4b1aea[cell].links[0] != -1)
                    ahead = g_4b1aea[cell].links[0];
                else if (g_4b1aea[cell].links[1] != -1)
                    ahead = g_4b1aea[cell].links[1];
                else if (g_4b1aea[cell].links[2] != -1)
                    ahead = g_4b1aea[cell].links[2];
                if (g_4b1aea[ahead].state != 507 && g_4b1aea[ahead].state != 508) {
                    done++;
                    break;
                }
                view = findView(g_4b1aea[ahead].snoid);
                snoid = (Snoid *)&view->body;
                hair = snoid->features[0];
                eyes = snoid->features[1];
                nose = snoid->features[2];
                feet = snoid->features[3];
                view = findView(g_4b1aea[back].snoid);
                snoid = (Snoid *)&view->body;
                otherHair = snoid->features[0];
                otherEyes = snoid->features[1];
                otherNose = snoid->features[2];
                otherFeet = snoid->features[3];
                if (g_4b1aea[cell].snoid == 510) {
                    if (otherHair == hair) {
                        g_4b1aea[cell].state = 502;
                        view = findView(g_4b1aea[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = fn_4489ce;
                    } else {
                        done++;
                    }
                } else if (g_4b1aea[cell].snoid == 511) {
                    if (otherEyes == eyes) {
                        g_4b1aea[cell].state = 502;
                        view = findView(g_4b1aea[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = fn_4489ce;
                    } else {
                        done++;
                    }
                } else if (g_4b1aea[cell].snoid == 512) {
                    if (nose == otherNose) {
                        g_4b1aea[cell].state = 502;
                        view = findView(g_4b1aea[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = fn_4489ce;
                    } else {
                        done++;
                    }
                } else if (g_4b1aea[cell].snoid == 513) {
                    if (otherFeet == feet) {
                        g_4b1aea[cell].state = 502;
                        view = findView(g_4b1aea[cell].view);
                        setViewScript(view, 7000, 1);
                        view->placed = fn_4489ce;
                    } else {
                        done++;
                    }
                }
                if (!done) {
                    g_4b1aea[cell].state = 502;
                    view = findView(g_4b1aea[cell].view);
                    setViewScript(view, 7000, 1);
                    view->placed = fn_4489ce;
                }
            } else if (g_4b1934 == 1 && !g_4b1aea[cell].snoid && g_4b1aea[cell - 1].state == 508) {
                g_4b1aea[cell].state = 502;
                view = findView(g_4b1aea[cell].view);
                setViewScript(view, 7000, 1);
                view->placed = fn_4489ce;
            }
            break;
        }
        if (!done) {
            if (g_4b1aea[cell].links[5] != -1)
                cell = g_4b1aea[cell].links[5];
            else if (g_4b1aea[cell].links[4] != -1)
                cell = g_4b1aea[cell].links[4];
            else if (g_4b1aea[cell].links[3] != -1)
                cell = g_4b1aea[cell].links[3];
            else
                done++;
        }
    } while (!done);
}
