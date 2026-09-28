/*
 * tunnels (0x45e2d8-0x4623b8): Stone Cold Caves (scene 8), 'Tunnels.MHK'
 */

#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "loading.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "tunnels.h"
#include "view.h"

/* Resets scene 8's state (the rules, the entries, the counts; the pace
   g_4b80a0 by g_4b2b00) and picks g_4b7fbc at random. */
/* @zoombi32 0x0045e2d8 */
void resetScene8()
{
    short i;

    g_4b7fee = g_4b8088 = g_4b808a = 0;
    g_4b7fe4 = g_4b8092 = g_4b8096 = 0;
    g_4b755e = 40;
    g_4b0d52 = g_4b7ff0.count = 0;
    g_4b7fd4 = g_4b966e = g_4b8094 = 0;
    g_4b7fd2 = g_4b7fd0 = g_4b7fce = 0;
    g_4b7fda = g_4b7fdc = g_4b808e = 0;
    g_4b8080 = g_4b8082 = g_4b8084 = g_4b8086 = g_4b808c = g_4b8090 = 0;
    for (i = 0; i < 4; i++)
        g_4b7fe6[i] = 0;
    g_4b7564 = 1;
    for (i = 0; i < 16; i++)
        g_4b7f34[i] = g_4b7f54[i] = g_4b7f74[i] = g_4b7f94[i] = 0;
    g_4b8098 = g_4b809a = 0;
    g_4b809c = g_4b80a4 = 0;
    if (g_4b2b00)
        g_4b80a0 = 120;
    else
        g_4b80a0 = 60;
    fillMemory(&g_4b7f18, 0, 28);
    if (g_4b754a) {
        g_4b7548 = 0;
        g_4b7544 = 0;
    }
    g_4b7fbc = randomBetween(0, 1);
}

/* Closes scene 8. */
/* @zoombi32 0x0045ea2b */
void closeScene8()
{
    if (g_4b7fb8) {
        g_4b7fb8 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a7708);
        g_4b7564 = 0;
        fn_46bee9(saved);
        fn_46ca9c(&g_4b7fb4);
        fadeOutViews();
        fn_4624fc();
    }
}

/* A notify: at the end (-1), clears g_4b7fd2 and moves g_4b7fee on from
   1 to 2. */
/* @zoombi32 0x0045fa56 */
void fn_45fa56(View *, short event)
{
    switch (event) {
    case -1:
        g_4b7fd2 = 0;
        if (g_4b7fee == 1)
            g_4b7fee++;
        break;
    }
}

/* A notify: at the end (-1), fn_465175, and if g_4b8004, copies g_4b8000
   and g_4b8004 into g_4b7fd4 and g_4b7fd6 and clears g_4b7fd8. */
/* @zoombi32 0x0045fb10 */
void fn_45fb10(View *, short event)
{
    switch (event) {
    case -1:
        fn_465175();
        if (g_4b8004) {
            g_4b7fd4 = g_4b8000;
            g_4b7fd6 = g_4b8004;
            g_4b7fd8 = 0;
        }
        break;
    }
}

/* Adds `entry` to `list` if it has room (five). */
/* @zoombi32 0x00460527 */
void fn_460527(TunnelList *list, TunnelEntry entry)
{
    if (list->count < 5) {
        list->entries[list->count] = entry;
        list->count++;
    }
}

/* A notify: at the end (-1), fn_465175 and g_4b7fd0; now and then (more
   often at higher levels, g_4b7fbe, or early in the roster), a remark if
   some but not all of the Zoombinis have been chosen. */
/* @zoombi32 0x0045faa3 */
void fn_45faa3(View *, short event)
{
    short chosen;

    switch (event) {
    case -1:
        fn_465175();
        g_4b7fd0 = 1;
        if (randomBetween(0, 4) > g_4b7fbe || (*(short *)(g_4a4ba0 + 0x2c) & 0xfff) <= 3) {
            chosen = countChosenSnoids();
            if (chosen < g_4b8094 && chosen)
                queueViewSound(randomBetween(20045, 20048), 1);
        }
        break;
    }
}

/* The buttons' view update: redraws button 2 when g_4b7fba changes, and
   button 1 the first time. */
/* @zoombi32 0x0045e9b9 */
void fn_45e9b9(View *, short region)
{
    if (g_4b7fba) {
        if (!g_4b7fda) {
            g_4b7fda = 1;
            unionRgnRect(region, &tunnelsButtons[1].rect);
        }
    } else if (g_4b7fda) {
        g_4b7fda = 0;
        unionRgnRect(region, &tunnelsButtons[1].rect);
    }
    if (!g_4b7fdc) {
        g_4b7fdc = 1;
        unionRgnRect(region, &tunnelsButtons[0].rect);
    }
}

/* Draws button `which` (1 or 2; 2 is dim unless g_4b7fba), lit or not,
   showing it on screen if `show`. */
/* @zoombi32 0x0045e903 */
void drawTunnelsButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b7fba) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a770c->offsets[image] + (char *)g_4a770c), tunnelsButtons[which - 1].rect.left,
                      tunnelsButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&tunnelsButtons[which - 1].rect);
    }
}

/* With the first entry of kind 1 (or 4), makes the view of the first other
   entry of kind 2 (or 3) solid again (clears flag 0x4000000). */
/* @zoombi32 0x004622f5 */
void fn_4622f5()
{
    View *view;
    short i;

    if (g_4b7ff0.entries[0].kind == 1) {
        for (i = 1; i < 5; i++)
            if (g_4b7ff0.entries[i].view && g_4b7ff0.entries[i].kind == 2) {
                view = findView(g_4b7ff0.entries[i].view);
                if (view) {
                    view->flags &= ~0x4000000;
                    i = 5;
                }
            }
    } else if (g_4b7ff0.entries[0].kind == 4) {
        for (i = 1; i < 5; i++)
            if (g_4b7ff0.entries[i].view && g_4b7ff0.entries[i].kind == 3) {
                view = findView(g_4b7ff0.entries[i].view);
                if (view) {
                    view->flags &= ~0x4000000;
                    i = 5;
                }
            }
    }
}

/* Scene 8's keys (with debugging on, g_4b8803, or else only 0x16f): t, T,
   w (W also sets g_4b807e) and e pick a rule (fn_460642), a shows the rules,
   C, F, I and O step the views g_4b7fc4-g_4b7fca through their scripts, H
   adds 4 to g_4b8098. Returns whether the key was used. */
/* @zoombi32 0x0045f63a */
short scene8Key(unsigned short key)
{
    short used = 0;
    ShortRect area = {0, 0, 350, 100};
    ShortRect spot = {225, 0, 350, 100};
    short first;
    short tops[2] = {20, 70};
    short n;
    short i;
    short script;

    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        fn_466b93();
        used = 1;
        break;
    case 'H':
        g_4b8098 += 4;
        break;
    case 'a':
        unionRgnRect(removedRgn, &area);
        updateViews();
        if (g_4b7f18.unknown0 == 1) {
            if (g_4b7f18.unknown2)
                n = 5;
            else
                n = 4;
        } else if (g_4b7f18.unknown2) {
            if (g_4b7f18.rules[0].unknownB)
                n = 1;
            else
                n = 0;
        } else if (g_4b7f18.rules[0].unknownB) {
            n = 3;
        } else {
            n = 2;
        }
        switch (n) {
        case 0:
            debugMessage(-1, "Left / Bottom accept:", 0, 0, 0);
            break;
        case 1:
            debugMessage(-1, "Left / Top accept:", 0, 0, 0);
            break;
        case 2:
            debugMessage(-1, "Right / Bottom accept:", 0, 0, 0);
            break;
        case 3:
            debugMessage(-1, "Right / Top accept:", 0, 0, 0);
            break;
        case 4:
            debugMessage(-1, "Right accepts:", 0, 0, 0);
            break;
        case 5:
            debugMessage(-1, "Left accepts:", 0, 0, 0);
            break;
        }
        setClipRect(gameRect);
        for (i = 0; i < 2; i++) {
            spot.left = 250;
            for (n = 0; n < g_4b7f18.rules[i].count; n++) {
                script = spot.left;
                spot.top = tops[i];
                drawFeature(g_4b7f18.rules[i].features[n], g_4b7f18.rules[i].values[n], &spot);
                spot.left = script + 30;
            }
        }
        break;
    case 'C':
    case 'F':
    case 'I':
    case 'O':
        switch (key) {
        case 'C':
            n = g_4b7fc4;
            first = 4000;
            i = 4038;
            break;
        case 'F':
            n = g_4b7fc8;
            first = 4200;
            i = 4226;
            break;
        case 'I':
            n = g_4b7fca;
            first = 4400;
            i = 4423;
            break;
        case 'O':
            n = g_4b7fc6;
            first = 4600;
            i = 4617;
            break;
        }
        {
            View *view = findView(n);

            if (view) {
                script = g_4b80a8 + 1;
                if (script < first || script > i)
                    script = first;
                setViewScript(view, script, 1);
                loadViewSounds(n, 1);
                g_4b80a8 = script;
                debugMessage(script, "SCRB n:", 0, 0, 0);
            }
        }
        break;
    case 'W':
        g_4b807e = 1;
    case 'w':
        fn_460642(1);
        break;
    case 't':
        fn_460642(0);
        break;
    case 'T':
        fn_460642(2);
        break;
    case 'e':
        fn_460642(3);
        break;
    }
    return used;
}

/* The buttons' view's draw callback: draws both buttons, dim. */
/* @zoombi32 0x0045e99c */
void drawTunnelsButtons(View *)
{
    drawTunnelsButton(1, 0, 0);
    drawTunnelsButton(2, 0, 0);
}

/* Takes the entry for `view` out of `list`, returning its kind (0 if
   there's none). */
/* Not exact: register allocation (the original keeps `kind` in ebx, `list`
   in eax and `i` in edx; here `kind` is in ecx and ebx is scratch). */
/* @zoombi32 0x0046033d */
short removeTunnelEntry(TunnelList *list, short view)
{
    short i;
    short kind;

    for (i = 0; i < list->count; i++)
        if (view == list->entries[i].view) {
            kind = list->entries[i].kind;
            for (; i < list->count - 1; i++) {
                list->entries[i].view = list->entries[i + 1].view;
                list->entries[i].unknown2 = list->entries[i + 1].unknown2;
                list->entries[i].unknown4 = list->entries[i + 1].unknown4;
                list->entries[i].unknown6 = list->entries[i + 1].unknown6;
                list->entries[i].unknown8 = list->entries[i + 1].unknown8;
                list->entries[i].unknownC[0] = list->entries[i + 1].unknownC[0];
                list->entries[i].unknownC[1] = list->entries[i + 1].unknownC[1];
                list->entries[i].unknownC[2] = list->entries[i + 1].unknownC[2];
                list->entries[i].unknownC[3] = list->entries[i + 1].unknownC[3];
                list->entries[i].unknownC[4] = list->entries[i + 1].unknownC[4];
                list->entries[i].unknownC[5] = list->entries[i + 1].unknownC[5];
                list->entries[i].unknownC[6] = list->entries[i + 1].unknownC[6];
                list->entries[i].kind = list->entries[i + 1].kind;
            }
            list->count--;
            return kind;
        }
    return 0;
}

/* Whether `snoid` is turned back at door `door` (1-4; others count as 1)
   under `rules`: whether it matches the first rule (any of its features'
   values; inverted unless unknown2) and the second (when unknown0 is 2;
   inverted unless the first rule's unknownB), combined by the door. The
   first rule's result (for doors 3 and 4, inverted) goes in *first. */
/* Not exact: register allocation (the original keeps `door`, `a`, `b` and
   `passes` on the stack and `snoid` in esi, using ebx and ecx as scratch). */
/* @zoombi32 0x00460c41 */
short fn_460c41(TunnelRules *rules, short door, Snoid *snoid, unsigned short *first)
{
    unsigned short passes;
    unsigned short a;
    unsigned short b;
    short i;

    if (door < 1 || door > 4)
        door = 1;
    a = b = 0;
    for (i = 0; i < rules->rules[0].count; i++)
        if (snoid->features[rules->rules[0].features[i] - 1] == rules->rules[0].values[i])
            a = 1;
    for (i = 0; rules->unknown0 == 2 && i < rules->rules[1].count; i++)
        if (snoid->features[rules->rules[1].features[i] - 1] == rules->rules[1].values[i])
            b = 1;
    if (!rules->unknown2)
        a = !a;
    if (!rules->rules[0].unknownB)
        b = !b;
    switch (door) {
    case 1:
        *first = a;
        if (rules->unknown0 == 1)
            passes = a;
        else
            passes = a && b;
        break;
    case 2:
        *first = a;
        if (rules->unknown0 == 1)
            passes = a;
        else
            passes = a && !b;
        break;
    case 3:
        *first = !a;
        if (rules->unknown0 == 1)
            passes = !a;
        else
            passes = !a && !b;
        break;
    case 4:
        *first = !a;
        if (rules->unknown0 == 1)
            passes = !a;
        else
            passes = !a && b;
        break;
    }
    return !passes;
}

/* Makes a one-feature rule (level 1): counts the chosen Zoombinis having
   each of the 20 feature values (a nibble each of a long, big-endian like
   the features), leaves out (g_4b7544) the count g_4b7548 if others remain,
   looks around half the party's size for a count some values have, picks
   one of those values at random as the rule, which the door accepts or
   refuses at random (unknown2). */
/* @zoombi32 0x00460e3d */
void fn_460e3d()
{
    ChosenSnoids *chosen;
    unsigned long picked;
    unsigned short j;
    unsigned short best;
    unsigned long masks[20];
    unsigned short counts[20];
    unsigned short n;
    unsigned short sign;
    short matches;
    unsigned short i;
    unsigned long value;
    unsigned long step;
    unsigned long features;
    unsigned short target;
    short pick;
    short found;

    chosen = listChosenSnoids();
    for (i = 0; i < 20; i++) {
        masks[i] = 0;
        counts[i] = 0;
    }
    value = 1;
    step = 1;
    for (i = 0; i < 20; i++) {
        masks[i] = value;
        switch (value) {
        case 5:
            value = step = 0x100;
            break;
        case 0x500:
            value = step = 0x10000;
            break;
        case 0x50000:
            value = step = 0x1000000;
            break;
        case 0x5000000:
            break;
        default:
            value += step;
            break;
        }
    }
    n = 20;
    for (j = 0; j < chosen->count; j++) {
        features = swapLong(*(unsigned long *)chosen->features[j]);
        for (i = 0; i < n; i++)
            if ((features & 0xf) == (masks[i] & 0xf) || (features & 0xf00) == (masks[i] & 0xf00)
                || (features & 0xf0000) == (masks[i] & 0xf0000)
                || (features & 0xf000000) == (masks[i] & 0xf000000))
                counts[i]++;
    }
    if (g_4b7544 && g_4b7548) {
        found = 0;
        for (i = 0; !found && i < n; i++)
            if (counts[i] && counts[i] != g_4b7548)
                found = 1;
        if (found)
            for (i = 0; i < n; i++)
                if (counts[i] == g_4b7548)
                    counts[i] = 0;
    }
    matches = 0;
    sign = 1;
    target = chosen->count / 2;
    while (!matches) {
        if (target > 0 && target < 16) {
            for (i = 0; i < n; i++)
                if (target == counts[i])
                    matches++;
            best = target;
        }
        target += sign;
        /* sign is unsigned, so the test is always false: the steps go +1,
           -2, +1, -2, ... (probably meant to widen either way). */
        if (sign < 0)
            sign--;
        sign++;
        sign = -sign;
    }
    pick = randomBetween(1, matches);
    for (i = 0; i < n; i++)
        if (counts[i] == best && !--pick) {
            picked = masks[i];
            i = n;
        }
    g_4b7f18.unknown0 = 1;
    g_4b7f18.unknown2 = randomBetween(0, 1);
    g_4b7f18.rules[0].count = 1;
    if (picked & 0xff) {
        g_4b7f18.rules[0].features[0] = 4;
        g_4b7f18.rules[0].values[0] = picked & 0xf;
    } else if (picked & 0xff00) {
        g_4b7f18.rules[0].features[0] = 3;
        g_4b7f18.rules[0].values[0] = (picked >> 8) & 0xf;
    } else if (picked & 0xff0000) {
        g_4b7f18.rules[0].features[0] = 2;
        g_4b7f18.rules[0].values[0] = (picked >> 16) & 0xf;
    } else if (picked & 0xff000000) {
        g_4b7f18.rules[0].features[0] = 1;
        g_4b7f18.rules[0].values[0] = (picked >> 24) & 0xf;
    }
}

/* Finds a free waiting place (of tunnelPlaces, noting which Zoombini is
   nearest each in sortedIds) for a Zoombini coming from `side`: one of the
   two by that side's door if free; otherwise moves Zoombinis along the
   row toward it to free one, and takes the first free place, into
   *spot. */
/* @zoombi32 0x00460021 */
void fn_460021(short *spot, short side)
{
    short nearRight[2] = {15, 10};
    short nearLeft[2] = {11, 6};
    short third;
    Point none = {0, 0};
    Snoid *snoid;
    short moving;
    short i;
    short k;
    short skip;
    short found;
    short j;
    short second;

    spotTaken(&none, 0, 500);
    for (k = 0; k < 16; k++) {
        skip = 0;
        found = spotNear(&tunnelPlaces[k], 500, skip);
        for (j = 0; found && j < k; j++)
            if (found == sortedIds[j]) {
                skip++;
                found = spotNear(&tunnelPlaces[k], 500, skip);
                j = 0;
            }
        sortedIds[k] = found;
    }
    found = -1;
    if (side) {
        for (j = 0; found == -1 && j < 2; j++)
            if (!sortedIds[nearRight[j]])
                found = nearRight[j];
    } else {
        for (j = 0; found == -1 && j < 2; j++)
            if (!sortedIds[nearLeft[j]])
                found = nearLeft[j];
    }
    if (found != -1) {
        *spot = found;
        return;
    }
    for (k = 0; k < 15; k++) {
        i = k;
        if (k >= 11 && !side)
            k = 26 - k;
        if (!sortedIds[k]) {
            skip = second = third = 0;
            if (k == 0) {
                skip = 6;
            } else if (k == 5 || k == 6) {
                skip = 5;
            } else if (k <= 4) {
                skip = 5;
                second = 6;
            } else if (k <= 10) {
                skip = 4;
                second = 5;
            } else if (side) {
                if (k < 15)
                    skip = 1;
                if (k < 14)
                    second = 2;
                if (k < 13)
                    third = 3;
            } else {
                if (k > 11)
                    skip = -1;
                if (k > 12)
                    second = -2;
                if (k > 13)
                    third = -3;
            }
            for (moving = 1; skip && moving;) {
                if (sortedIds[k + skip]) {
                    snoid = findSnoid(sortedIds[k + skip], 1);
                    if (snoid) {
                        *(Point *)&snoid->targetX = tunnelPlaces[k];
                        setSnoidAction(snoid, 7, 0);
                        sortedIds[k] = sortedIds[k + skip];
                        sortedIds[k + skip] = 0;
                        moving = 0;
                    }
                }
                skip = second;
                second = third;
                third = 0;
            }
        }
        k = i;
    }
    found = -1;
    if (side) {
        for (j = 0; found == -1 && j < 2; j++)
            if (!sortedIds[nearRight[j]])
                found = nearRight[j];
    } else {
        for (j = 0; found == -1 && j < 2; j++)
            if (!sortedIds[nearLeft[j]])
                found = nearLeft[j];
    }
    for (k = 0; found == -1 && k < 16; k++)
        if (!sortedIds[k])
            found = k;
    *spot = found;
}

/* Picks the pair of masks (of `n`; `pairs` is n * n) that best splits the
   chosen Zoombinis four ways: by whether each matches the first mask in some
   feature, and the second. Pairs leaving a way empty drop out; of the rest,
   those splitting most evenly tie, and one is picked at random. */
/* @zoombi32 0x00461e1a */
void fn_461e1a(ChosenSnoids *chosen, unsigned long *masks, unsigned long *pair, short pairs, short n)
{
    char *score;
    char *both;
    char *firstOnly;
    char *secondOnly;
    char *neither;
    unsigned long row;
    long ties;
    unsigned long j;
    long pick;
    unsigned long i;
    unsigned long features;
    unsigned long col;
    unsigned long most;
    unsigned long count;

    both = (char *)newPtr(pairs);
    if (!both)
        fatalError(msgOutOfMemory);
    firstOnly = (char *)newPtr(pairs);
    if (!firstOnly) {
        disposePtr(both);
        fatalError(msgOutOfMemory);
    }
    secondOnly = (char *)newPtr(pairs);
    if (!secondOnly) {
        disposePtr(both);
        disposePtr(firstOnly);
        fatalError(msgOutOfMemory);
    }
    neither = (char *)newPtr(pairs);
    if (!neither) {
        disposePtr(both);
        disposePtr(firstOnly);
        disposePtr(secondOnly);
        fatalError(msgOutOfMemory);
    }
    score = (char *)newPtr(pairs);
    if (!score) {
        disposePtr(both);
        disposePtr(firstOnly);
        disposePtr(secondOnly);
        disposePtr(neither);
        fatalError(msgOutOfMemory);
    }
    for (i = 0; i < pairs; i++) {
        both[i] = 0;
        firstOnly[i] = 0;
        secondOnly[i] = 0;
        neither[i] = 0;
    }
    for (j = 0; j < chosen->count; j++) {
        features = swapLong(*(unsigned long *)chosen->features[j]);
        for (i = 0; i < pairs; i++) {
            row = i / n;
            col = i % n;
            if (col == row)
                continue;
            if ((features & 0xf) == (masks[row] & 0xf) || (features & 0xf00) == (masks[row] & 0xf00)
                || (features & 0xf0000) == (masks[row] & 0xf0000)
                || (features & 0xf000000) == (masks[row] & 0xf000000)) {
                if ((features & 0xf) == (masks[col] & 0xf) || (features & 0xf00) == (masks[col] & 0xf00)
                    || (features & 0xf0000) == (masks[col] & 0xf0000)
                    || (features & 0xf000000) == (masks[col] & 0xf000000))
                    both[i]++;
                else
                    firstOnly[i]++;
            } else if ((features & 0xf) == (masks[col] & 0xf) || (features & 0xf00) == (masks[col] & 0xf00)
                       || (features & 0xf0000) == (masks[col] & 0xf0000)
                       || (features & 0xf000000) == (masks[col] & 0xf000000))
                secondOnly[i]++;
            else
                neither[i]++;
        }
    }
    most = 0;
    for (i = 0; i < pairs; i++) {
        count = 0;
        if (both[i])
            count++;
        if (firstOnly[i])
            count++;
        if (secondOnly[i])
            count++;
        if (neither[i])
            count++;
        score[i] = count;
        if (count > most)
            most = count;
    }
    for (i = 0; i < pairs; i++)
        if (score[i] < most)
            both[i] = -1;
    ties = 0;
    features = 32000; /* now the best score */
    for (i = 0; i < pairs; i++) {
        score[i] = -1;
        if (both[i] == -1)
            continue;
        score[i] = abs(both[i] - firstOnly[i]) + abs(both[i] - secondOnly[i]) + abs(both[i] - neither[i])
                   + abs(firstOnly[i] - secondOnly[i]) + abs(firstOnly[i] - neither[i])
                   + abs(secondOnly[i] - neither[i]);
        if (score[i] < features)
            features = score[i];
    }
    for (i = 0; i < pairs; i++)
        if (score[i] == features)
            ties++;
    pick = randomBetween(1, ties);
    for (i = 0; i < pairs; i++)
        if (score[i] == features) {
            pick--;
            if (!pick) {
                row = i / n;
                col = i % n;
                pair[0] = masks[row];
                pair[1] = masks[col];
                i = pairs;
            }
        }
    disposePtr(both);
    disposePtr(firstOnly);
    disposePtr(secondOnly);
    disposePtr(neither);
    disposePtr(score);
}
