/*
 * tunnels (0x45e2d8-0x4623b8): Stone Cold Caves (scene 8), 'Tunnels.MHK'
 */

#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "loading.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "tunnels.h"
#include "view.h"

/* Resets scene 8's state (the rules, the entries, the counts; the pace
   g_4b80a0 by g_4b2b00) and picks g_4b7fbc at random. */
/* @zoombi32 0x0045e2d8 */
void resetTunnels()
{
    short i;

    g_4b7fee = g_4b8088 = g_4b808a = 0;
    g_4b7fe4 = g_4b8092 = g_4b8096 = 0;
    g_4b755e = 40;
    sceneDue = g_4b7ff0.count = 0;
    g_4b7fd4 = hintSound = g_4b8094 = 0;
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
    if (practiceLevel) {
        lastRuleCount = 0;
        lastRuleMask = 0;
    }
    g_4b7fbc = randomBetween(0, 1);
}

/* Closes scene 8. */
/* @zoombi32 0x0045ea2b */
void closeTunnels()
{
    if (g_4b7fb8) {
        g_4b7fb8 = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&g_4a7708);
        g_4b7564 = 0;
        setFreeAtOnce(saved);
        closeGameFile(&g_4b7fb4);
        fadeOutViews();
        showBusyCursor();
    }
}

/* A notify: at the end (-1), clears g_4b7fd2 and moves g_4b7fee on from
   1 to 2. */
/* @zoombi32 0x0045fa56 */
void remarkEndNotify(View *, short event)
{
    switch (event) {
    case -1:
        g_4b7fd2 = 0;
        if (g_4b7fee == 1)
            g_4b7fee++;
        break;
    }
}

/* A notify for the first entry's line: at the end (-1), requestViewSort, and if
   there's a line to follow, sets it up to say next (g_4b7fd4 and g_4b7fd6,
   clearing g_4b7fd8). */
/* @zoombi32 0x0045fb10 */
void firstLineNotify(View *, short event)
{
    switch (event) {
    case -1:
        requestViewSort();
        if (g_4b7ff0.entries[0].lineThen) {
            g_4b7fd4 = g_4b7ff0.entries[0].speaker;
            g_4b7fd6 = g_4b7ff0.entries[0].lineThen;
            g_4b7fd8 = 0;
        }
        break;
    }
}

/* Adds `entry` to `list` if it has room (five). */
/* @zoombi32 0x00460527 */
void addTunnelEntry(TunnelList *list, TunnelEntry entry)
{
    if (list->count < 5) {
        list->entries[list->count] = entry;
        list->count++;
    }
}

/* A notify: at the end (-1), requestViewSort and g_4b7fd0; now and then (more
   often at higher levels, g_4b7fbe, or early in the roster), a remark if
   some but not all of the Zoombinis have been chosen. */
/* @zoombi32 0x0045faa3 */
void tunnelRemarkNotify(View *, short event)
{
    short chosen;

    switch (event) {
    case -1:
        requestViewSort();
        g_4b7fd0 = 1;
        if (randomBetween(0, 4) > g_4b7fbe || (*(short *)(gameState + 0x2c) & 0xfff) <= 3) {
            chosen = countChosenSnoids();
            if (chosen < g_4b8094 && chosen)
                queueViewSound(randomBetween(20045, 20048), 1);
        }
        break;
    }
}

/* The buttons' view update: redraws button 2 when tunnelsGoReady changes, and
   button 1 the first time. */
/* @zoombi32 0x0045e9b9 */
void updateTunnelsButtons(View *, short region)
{
    if (tunnelsGoReady) {
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

/* Draws button `which` (1 or 2; 2 is dim unless tunnelsGoReady), lit or not,
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
        if (!tunnelsGoReady) {
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
void unghostDoorView()
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

/* Scene 8's keys (with debugging on, debugMessagesOn, or else only 0x16f): t, T,
   w (W also sets g_4b807e) and e queue a remark (queueRemark), a shows the rules,
   C, F, I and O step the views tunnelsSpeakers through their scripts, H
   adds 4 to g_4b8098. Returns whether the key was used. */
/* @zoombi32 0x0045f63a */
short tunnelsKey(unsigned short key)
{
    short used = 0;
    ShortRect area = {0, 0, 350, 100};
    ShortRect spot = {225, 0, 350, 100};
    short first;
    short tops[2] = {20, 70};
    short n;
    short i;
    short script;

    if (!debugMessagesOn && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        replayHint();
        used = 1;
        break;
    case 'H':
        g_4b8098 += 4;
        break;
    case 'a':
        unionRgnRect(removedRgn, &area);
        updateViews();
        if (g_4b7f18.count == 1) {
            if (g_4b7f18.rules[0].side)
                n = 5;
            else
                n = 4;
        } else if (g_4b7f18.rules[0].side) {
            if (g_4b7f18.rules[1].side)
                n = 1;
            else
                n = 0;
        } else if (g_4b7f18.rules[1].side) {
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
            n = tunnelsSpeakers[0];
            first = 4000;
            i = 4038;
            break;
        case 'F':
            n = tunnelsSpeakers[2];
            first = 4200;
            i = 4226;
            break;
        case 'I':
            n = tunnelsSpeakers[3];
            first = 4400;
            i = 4423;
            break;
        case 'O':
            n = tunnelsSpeakers[1];
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
        queueRemark(1);
        break;
    case 't':
        queueRemark(0);
        break;
    case 'T':
        queueRemark(2);
        break;
    case 'e':
        queueRemark(3);
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
                list->entries[i].back = list->entries[i + 1].back;
                list->entries[i].step = list->entries[i + 1].step;
                list->entries[i].from = list->entries[i + 1].from;
                list->entries[i].script = list->entries[i + 1].script;
                list->entries[i].backScript = list->entries[i + 1].backScript;
                list->entries[i].speaker = list->entries[i + 1].speaker;
                list->entries[i].line = list->entries[i + 1].line;
                list->entries[i].lineThen = list->entries[i + 1].lineThen;
                list->entries[i].replier = list->entries[i + 1].replier;
                list->entries[i].reply = list->entries[i + 1].reply;
                list->entries[i].replyThen = list->entries[i + 1].replyThen;
                list->entries[i].kind = list->entries[i + 1].kind;
            }
            list->count--;
            return kind;
        }
    return 0;
}

/* Whether `snoid` is turned back at door `door` (1-4; others count as 1)
   under `rules`: whether it matches the first rule (any of its features'
   values; inverted unless its side) and the second (when there are two;
   inverted unless its side), combined by the door. The
   first rule's result (for doors 3 and 4, inverted) goes in *first. */
/* Not exact: register allocation (the original keeps `door`, `a`, `b` and
   `passes` on the stack and `snoid` in esi, using ebx and ecx as scratch). */
/* @zoombi32 0x00460c41 */
short turnedBackAtDoor(FeatureRules *rules, short door, Snoid *snoid, unsigned short *first)
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
    for (i = 0; rules->count == 2 && i < rules->rules[1].count; i++)
        if (snoid->features[rules->rules[1].features[i] - 1] == rules->rules[1].values[i])
            b = 1;
    if (!rules->rules[0].side)
        a = !a;
    if (!rules->rules[1].side)
        b = !b;
    switch (door) {
    case 1:
        *first = a;
        if (rules->count == 1)
            passes = a;
        else
            passes = a && b;
        break;
    case 2:
        *first = a;
        if (rules->count == 1)
            passes = a;
        else
            passes = a && !b;
        break;
    case 3:
        *first = !a;
        if (rules->count == 1)
            passes = !a;
        else
            passes = !a && !b;
        break;
    case 4:
        *first = !a;
        if (rules->count == 1)
            passes = !a;
        else
            passes = !a && b;
        break;
    }
    return !passes;
}

/* Makes a one-feature rule (level 1): counts the chosen Zoombinis having
   each of the 20 feature values (a nibble each of a long, big-endian like
   the features), leaves out (lastRuleMask) the count lastRuleCount if others remain,
   looks around half the party's size for a count some values have, picks
   one of those values at random as the rule, which the door accepts or
   refuses at random (back). */
/* @zoombi32 0x00460e3d */
void makeOneFeatureRule()
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
    if (lastRuleMask && lastRuleCount) {
        found = 0;
        for (i = 0; !found && i < n; i++)
            if (counts[i] && counts[i] != lastRuleCount)
                found = 1;
        if (found)
            for (i = 0; i < n; i++)
                if (counts[i] == lastRuleCount)
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
    g_4b7f18.count = 1;
    g_4b7f18.rules[0].side = randomBetween(0, 1);
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
void findWaitingPlace(short *spot, short side)
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
void pickBestMaskPair(ChosenSnoids *chosen, unsigned long *masks, unsigned long *pair, short pairs, short n)
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

/* Queues a remark of the given kind for the cave's four characters (the
   views tunnelsSpeakers) to say: one speaker's script (and one to follow
   it), then perhaps another speaker's reply (and its follow-up), picked with
   allocateSlot so that each comes round before any repeats. Kind 1 has a
   shorter set once W has been pressed (g_4b807e); kind 3 depends on
   g_4b7fd0 and on whether every chosen Zoombini is on screen. */
/* @zoombi32 0x00460642 */
void queueRemark(short kind)
{
    short speaker;
    short line;
    short replier;
    short lineThen;
    short reply;
    short replyThen;
    TunnelEntry entry;

    entry.view = 0;
    entry.step = 0;
    speaker = replier = line = lineThen = reply = replyThen = 0;
    switch (kind) {
    case 0:
        switch (allocateSlot(&g_4a78c4, 10, 0)) {
                case 0:
                    speaker = tunnelsSpeakers[1];
                    line = 0x1202;
                    replier = tunnelsSpeakers[2];
                    reply = 0x1078;
                    break;
                case 1:
                    speaker = tunnelsSpeakers[2];
                    line = 0x1079;
                    break;
                case 2:
                    speaker = tunnelsSpeakers[2];
                    line = 0x107a;
                    break;
                case 3:
                    speaker = tunnelsSpeakers[2];
                    line = 0x107b;
                    break;
                case 4:
                    speaker = tunnelsSpeakers[2];
                    line = 0x107c;
                    break;
                case 5:
                    speaker = tunnelsSpeakers[0];
                    line = 0xfb5;
                    replier = tunnelsSpeakers[3];
                    reply = 0x1138;
                    break;
                case 6:
                    speaker = tunnelsSpeakers[0];
                    line = 0xfb6;
                    replier = tunnelsSpeakers[3];
                    reply = 0x1138;
                    break;
                case 7:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1132;
                    replier = tunnelsSpeakers[0];
                    reply = 0xfb7;
                    replyThen = 0xfbd;
                    break;
                case 8:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1132;
                    replier = tunnelsSpeakers[0];
                    reply = 0xfb7;
                    replyThen = 0xfbe;
                    break;
                case 9:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1143;
        }
        break;
    case 1:
        if (g_4b807e != 1) {
            switch (allocateSlot(&g_4a78c8, 8, 0)) {
                    case 0:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1203;
                        replier = tunnelsSpeakers[0];
                        reply = 0xfba;
                        break;
                    case 1:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1203;
                        replier = tunnelsSpeakers[2];
                        reply = 0x1080;
                        break;
                    case 2:
                        speaker = tunnelsSpeakers[3];
                        line = 0x113b;
                        replier = tunnelsSpeakers[1];
                        reply = 0x1204;
                        break;
                    case 3:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1205;
                        replier = tunnelsSpeakers[2];
                        reply = 0x107f;
                        break;
                    case 4:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1205;
                        replier = tunnelsSpeakers[2];
                        reply = 0x107e;
                        break;
                    case 5:
                        speaker = tunnelsSpeakers[3];
                        line = 0x113a;
                        replier = tunnelsSpeakers[2];
                        reply = 0x107e;
                        break;
                    case 6:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfbb;
                        replier = tunnelsSpeakers[3];
                        reply = 0x113c;
                        break;
                    case 7:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfbc;
                        replier = tunnelsSpeakers[2];
                        reply = 0x107f;
            }
        } else {
            switch (allocateSlot(&g_4a78c8, 4, 0)) {
                    case 0:
                        speaker = tunnelsSpeakers[2];
                        line = 0x107d;
                        break;
                    case 1:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfb8;
                        lineThen = 0xfb9;
                        break;
                    case 2:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1206;
                        break;
                    case 3:
                        speaker = tunnelsSpeakers[3];
                        line = 0x1139;
            }
        }
        break;
    case 2:
        switch (allocateSlot(&g_4a78d0, 3, 0)) {
                case 0:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1144;
                    break;
                case 1:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1145;
                    break;
                case 2:
                    speaker = tunnelsSpeakers[3];
                    line = 0x1146;
        }
        break;
    case 3:
        if (g_4b7fd0) {
            switch (allocateSlot(&g_4a78dc, 7, 0)) {
                    case 0:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc3;
                        break;
                    case 1:
                        speaker = tunnelsSpeakers[3];
                        line = 0x1147;
                        break;
                    case 2:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc2;
                        break;
                    case 3:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc4;
                        break;
                    case 4:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc5;
                        break;
                    case 5:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc0;
                        break;
                    case 6:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc1;
            }
        } else if (countSnoidViews() == countChosenSnoids()) {
            switch (allocateSlot(&g_4a78d4, 8, 0)) {
                    case 0:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfbf;
                        break;
                    case 1:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1209;
                        break;
                    case 2:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc6;
                        break;
                    case 3:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc2;
                        break;
                    case 4:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc4;
                        break;
                    case 5:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc5;
                        break;
                    case 6:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc0;
                        break;
                    case 7:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc1;
            }
        } else {
            switch (allocateSlot(&g_4a78d8, 9, 0)) {
                    case 0:
                        speaker = tunnelsSpeakers[2];
                        line = 0x1081;
                        break;
                    case 1:
                        speaker = tunnelsSpeakers[2];
                        line = 0x1082;
                        break;
                    case 2:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1207;
                        break;
                    case 3:
                        speaker = tunnelsSpeakers[1];
                        line = 0x1208;
                        break;
                    case 4:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc2;
                        break;
                    case 5:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc4;
                        break;
                    case 6:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc5;
                        break;
                    case 7:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc0;
                        break;
                    case 8:
                        speaker = tunnelsSpeakers[0];
                        line = 0xfc1;
            }
        }
        break;
    }
    entry.speaker = speaker;
    entry.line = line;
    entry.lineThen = lineThen;
    entry.replier = replier;
    entry.reply = reply;
    entry.replyThen = replyThen;
    addTunnelEntry(&g_4b7ff0, entry);
}

/* Makes a two-rule set (two feature values each): builds the 40 masks of
   two values of one feature (a byte of a long, a value per nibble, big-endian
   like the Zoombinis' features), then as pickBestMaskPair picks the pair of masks
   that best splits the chosen Zoombinis four ways, and makes each a rule
   with a random side. */
/* @zoombi32 0x004612b1 */
void makeTwoValueRules()
{
    char *both;
    char *firstOnly;
    char *secondOnly;
    char *neither;
    char *score;
    ChosenSnoids *chosen;
    short pairs;
    unsigned long pair[2];
    short j;
    short best;
    short ties;
    unsigned long masks[40];
    unsigned long base[10] = {0x12, 0x13, 0x14, 0x15, 0x23, 0x24, 0x25, 0x34, 0x35, 0x45};
    short i;
    unsigned long features;
    short row;
    short col;
    short most;
    short count;
    int shift;
    short k;
    short pick;

    both = (char *)newPtr(1600);
    if (!both)
        fatalError(msgOutOfMemory);
    firstOnly = (char *)newPtr(1600);
    if (!firstOnly) {
        disposePtr(both);
        fatalError(msgOutOfMemory);
    }
    secondOnly = (char *)newPtr(1600);
    if (!secondOnly) {
        disposePtr(both);
        disposePtr(firstOnly);
        fatalError(msgOutOfMemory);
    }
    neither = (char *)newPtr(1600);
    if (!neither) {
        disposePtr(both);
        disposePtr(firstOnly);
        disposePtr(secondOnly);
        fatalError(msgOutOfMemory);
    }
    score = (char *)newPtr(1600);
    if (!score) {
        disposePtr(both);
        disposePtr(firstOnly);
        disposePtr(secondOnly);
        disposePtr(neither);
        fatalError(msgOutOfMemory);
    }
    chosen = listChosenSnoids();
    for (i = 0; i < 40; i++)
        masks[i] = 0;
    for (i = 0; i < 1600; i++) {
        both[i] = 0;
        firstOnly[i] = 0;
        secondOnly[i] = 0;
        neither[i] = 0;
    }
    shift = 0;
    k = 0;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 10; i++) {
            masks[k] = base[i] << shift;
            k++;
        }
        shift += 8;
    }
    pairs = 1600;
    for (j = 0; j < chosen->count; j++) {
        features = swapLong(*(unsigned long *)chosen->features[j]);
        for (i = 0; i < pairs; i++) {
            row = i / 40;
            col = i % 40;
            if (row == col)
                continue;
            if ((features & 0xf) == (masks[row] & 0xf) || (features & 0xf00) == (masks[row] & 0xf00) || (features & 0xf0000) == (masks[row] & 0xf0000) || (features & 0xf000000) == (masks[row] & 0xf000000) || (features & 0xf) == (masks[row] & 0xf0) >> 4 || (features & 0xf00) == (masks[row] & 0xf000) >> 4 || (features & 0xf0000) == (masks[row] & 0xf00000) >> 4 || (features & 0xf000000) == (masks[row] & 0xf0000000) >> 4) {
                if ((features & 0xf) == (masks[col] & 0xf) || (features & 0xf00) == (masks[col] & 0xf00) || (features & 0xf0000) == (masks[col] & 0xf0000) || (features & 0xf000000) == (masks[col] & 0xf000000) || (features & 0xf) == (masks[col] & 0xf0) >> 4 || (features & 0xf00) == (masks[col] & 0xf000) >> 4 || (features & 0xf0000) == (masks[col] & 0xf00000) >> 4 || (features & 0xf000000) == (masks[col] & 0xf0000000) >> 4)
                    both[i]++;
                else
                    firstOnly[i]++;
            } else if ((features & 0xf) == (masks[col] & 0xf) || (features & 0xf00) == (masks[col] & 0xf00) || (features & 0xf0000) == (masks[col] & 0xf0000) || (features & 0xf000000) == (masks[col] & 0xf000000) || (features & 0xf) == (masks[col] & 0xf0) >> 4 || (features & 0xf00) == (masks[col] & 0xf000) >> 4 || (features & 0xf0000) == (masks[col] & 0xf00000) >> 4 || (features & 0xf000000) == (masks[col] & 0xf0000000) >> 4)
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
    best = 9999;
    for (i = 0; i < pairs; i++) {
        score[i] = -1;
        if (both[i] == -1)
            continue;
        score[i] = abs(both[i] - firstOnly[i]) + abs(both[i] - secondOnly[i]) + abs(both[i] - neither[i])
                   + abs(firstOnly[i] - secondOnly[i]) + abs(firstOnly[i] - neither[i])
                   + abs(secondOnly[i] - neither[i]);
        if (score[i] < best)
            best = score[i];
    }
    for (i = 0; i < pairs; i++)
        if (score[i] == best)
            ties++;
    pick = randomBetween(1, ties);
    for (i = 0; i < pairs; i++)
        if (score[i] == best) {
            pick--;
            if (!pick) {
                row = i / 40;
                col = i % 40;
                pair[0] = masks[row];
                pair[1] = masks[col];
                i = pairs;
            }
        }
    g_4b7f18.count = 2;
    for (i = 0; i < 2; i++) {
        g_4b7f18.rules[i].side = randomBetween(0, 1);
        g_4b7f18.rules[i].count = 2;
        if (pair[i] & 0xff) {
            g_4b7f18.rules[i].features[0] = 4;
            g_4b7f18.rules[i].values[0] = pair[i] & 0xf;
            g_4b7f18.rules[i].features[1] = 4;
            g_4b7f18.rules[i].values[1] = (pair[i] & 0xf0) >> 4;
        } else if (pair[i] & 0xff00) {
            g_4b7f18.rules[i].features[0] = 3;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 8) & 0xf;
            g_4b7f18.rules[i].features[1] = 3;
            g_4b7f18.rules[i].values[1] = (pair[i] >> 12) & 0xf;
        } else if (pair[i] & 0xff0000) {
            g_4b7f18.rules[i].features[0] = 2;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 16) & 0xf;
            g_4b7f18.rules[i].features[1] = 2;
            g_4b7f18.rules[i].values[1] = (pair[i] >> 20) & 0xf;
        } else if (pair[i] & 0xff000000) {
            g_4b7f18.rules[i].features[0] = 1;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 24) & 0xf;
            g_4b7f18.rules[i].features[1] = 1;
            g_4b7f18.rules[i].values[1] = (pair[i] >> 28) & 0xf;
        }
    }
    disposePtr(both);
    disposePtr(firstOnly);
    disposePtr(secondOnly);
    disposePtr(neither);
    disposePtr(score);
}

/* Makes a two-rule set of one feature value each: the 20 masks of one
   value, and the pair of them that best splits the chosen Zoombinis
   (pickBestMaskPair), each made a rule with a random side. */
/* @zoombi32 0x00461135 */
void makeOneValueRules()
{
    ChosenSnoids *chosen;
    unsigned long pair[2];
    unsigned long masks[20];
    short i;
    unsigned long value;
    unsigned long step;

    chosen = listChosenSnoids();
    for (i = 0; i < 20; i++)
        masks[i] = 0;
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
    pickBestMaskPair(chosen, masks, pair, 400, 20);
    g_4b7f18.count = 2;
    for (i = 0; i < 2; i++) {
        g_4b7f18.rules[i].side = randomBetween(0, 1);
        g_4b7f18.rules[i].count = 1;
        if (pair[i] & 0xff) {
            g_4b7f18.rules[i].features[0] = 4;
            g_4b7f18.rules[i].values[0] = pair[i] & 0xf;
        } else if (pair[i] & 0xff00) {
            g_4b7f18.rules[i].features[0] = 3;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 8) & 0xf;
        } else if (pair[i] & 0xff0000) {
            g_4b7f18.rules[i].features[0] = 2;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 16) & 0xf;
        } else if (pair[i] & 0xff000000) {
            g_4b7f18.rules[i].features[0] = 1;
            g_4b7f18.rules[i].values[0] = (pair[i] >> 24) & 0xf;
        }
    }
}

/* Makes a two-rule set whose rules each name values of two features: the
   150 masks of a value of each of two features (six pairs of features,
   `low` and `high` giving each one's place), the pair of them that best
   splits the chosen Zoombinis (pickBestMaskPair), each made a rule with a random
   side. */
/* @zoombi32 0x00461bec */
void makeTwoFeatureRules()
{
    ChosenSnoids *chosen;
    unsigned long pair[2];
    short i;
    unsigned long masks[150];
    unsigned long low[6] = {0x1, 0x1, 0x1, 0x100, 0x100, 0x10000};
    unsigned long high[6] = {0x100, 0x10000, 0x1000000, 0x10000, 0x1000000, 0x1000000};
    short n;
    short k;
    short f;
    unsigned long value;

    chosen = listChosenSnoids();
    for (i = 0; i < 150; i++)
        masks[i] = 0;
    k = 0;
    for (f = 0; f < 6; f++)
        for (i = 1; i <= 5; i++) {
            value = high[f] * i + low[f];
            for (n = 1; n <= 5; n++) {
                masks[k] = value;
                k++;
                value += low[f];
            }
        }
    pickBestMaskPair(chosen, masks, pair, 22500, 150);
    g_4b7f18.count = 2;
    for (i = 0; i < 2; i++) {
        f = 0;
        g_4b7f18.rules[i].side = randomBetween(0, 1);
        g_4b7f18.rules[i].count = 2;
        if ((pair[i] & 0xff) && f < 2) {
            g_4b7f18.rules[i].features[f] = 4;
            g_4b7f18.rules[i].values[f] = pair[i] & 0xf;
            f++;
        }
        if ((pair[i] & 0xff00) && f < 2) {
            g_4b7f18.rules[i].features[f] = 3;
            g_4b7f18.rules[i].values[f] = (pair[i] >> 8) & 0xf;
            f++;
        }
        if ((pair[i] & 0xff0000) && f < 2) {
            g_4b7f18.rules[i].features[f] = 2;
            g_4b7f18.rules[i].values[f] = (pair[i] >> 16) & 0xf;
            f++;
        }
        if ((pair[i] & 0xff000000) && f < 2) {
            g_4b7f18.rules[i].features[f] = 1;
            g_4b7f18.rules[i].values[f] = (pair[i] >> 24) & 0xf;
        }
    }
}

/* Drops the first entry of `list`, if any. */
/* @zoombi32 0x00460556 */
void dropFirstTunnelEntry(TunnelList *list)
{
    if (list->count)
        removeTunnelEntry(list, list->entries[0].view);
}

/* Says the next part of the remark first in g_4b7ff0 (its step: the
   speaker's line, the reply, the speaker's second line, the second reply;
   a part with no view or script is skipped), ending with remarkEndNotify; when
   there's nothing left to say, drops the remark. */
/* @zoombi32 0x00460571 */
void sayTunnelRemark()
{
    short view = 0;
    short script;

    g_4b7ff0.entries[0].step++;
    switch (g_4b7ff0.entries[0].step) {
    case 1:
        view = g_4b7ff0.entries[0].speaker;
        script = g_4b7ff0.entries[0].line;
        break;
    case 2:
        view = g_4b7ff0.entries[0].replier;
        script = g_4b7ff0.entries[0].reply;
        if (!view || !script) {
            g_4b7ff0.entries[0].step++;
            view = g_4b7ff0.entries[0].speaker;
            script = g_4b7ff0.entries[0].lineThen;
        }
        break;
    case 3:
        view = g_4b7ff0.entries[0].speaker;
        script = g_4b7ff0.entries[0].lineThen;
        if (!view || !script) {
            g_4b7ff0.entries[0].step++;
            view = g_4b7ff0.entries[0].replier;
            script = g_4b7ff0.entries[0].replyThen;
        }
        break;
    case 4:
        view = g_4b7ff0.entries[0].replier;
        script = g_4b7ff0.entries[0].replyThen;
        break;
    }
    if (view && script) {
        startView(view, script, remarkEndNotify, 1);
        loadViewSounds(view, 1);
        g_4b7fd2 = 1;
    } else {
        dropFirstTunnelEntry(&g_4b7ff0);
    }
    resetViewClock();
}

/* A notify: at the end (-1), drops the remark said and clears g_4b7fd2. */
/* @zoombi32 0x0045fa80 */
void dropRemarkNotify(View *, short event)
{
    switch (event) {
    case -1:
        dropFirstTunnelEntry(&g_4b7ff0);
        g_4b7fd2 = 0;
        break;
    }
}

/* Sends up to four waiting Zoombinis (the first entries of g_4b7ff0) off
   through their doors (the entry's kind, 1-4), freeing the four places by
   the doors. */
/* @zoombi32 0x0045f9c9 */
void sendThroughDoors()
{
    short doorX[4] = {141, 198, 426, 479};
    short i;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 4; i++) {
        claimPlacedView(i + 1, 0);
        if (g_4b7ff0.count) {
            view = findView(g_4b7ff0.entries[0].view);
            if (view) {
                snoid = (Snoid *)&view->body;
                snoid->targetX = doorX[g_4b7ff0.entries[0].kind - 1];
                snoid->targetY = 460;
                snoid->unknownF7 = 0;
                view->flags &= ~0x4000000;
                setSnoidAction(snoid, 7, 0);
            }
            dropFirstTunnelEntry(&g_4b7ff0);
        }
    }
}

/* The Zoombinis' notify in the caves: 240-243 set the facing to use when
   the next turn (0) ends, 250-253 face that way at once, 10 starts the
   script in the first entry (backScript) at one of four anchors, 13 has the
   first entry's speaker say its line and stops the last view sound. At the
   end of a script (-1): a Zoombini that is first in the queue, not turned
   back and with a line to follow, goes through its door (the entry's
   kind) to the next place there, counts toward g_4b8098 (more as the
   chosen Zoombinis run out) and sets off the line; one turned back gets
   a remark (the first time) and walks to a free waiting place
   (findWaitingPlace) on its door's side. Either way the entry is dropped unless its line is
   being said. */
/* @zoombi32 0x0045fb50 */
void tunnelsSnoidNotify(View *view, short event)
{
    short spot;
    Snoid *snoid;
    View *speaker;
    short side;
    short anchor;
    short after;
    short count;

    snoid = viewSnoid(view);
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
        g_4b7fe4 = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b7fe4) {
            setSnoidFacing(snoid, g_4b7fe4 - 1);
            g_4b7fe4 = 0;
        }
        break;
    case 10:
        side = ((g_4b7ff0.entries[0].backScript - 8000) / 2) & 3;
        unghostDoorView();
        startSnoidScript(snoid, g_4b7ff0.entries[0].backScript, &g_4a78a6[side], 0);
        view->notify = tunnelsSnoidNotify;
        break;
    case 13:
        speaker = 0;
        if (g_4b7ff0.entries[0].line) {
            startView(g_4b7ff0.entries[0].speaker, g_4b7ff0.entries[0].line, firstLineNotify, 1);
            loadViewSounds(g_4b7ff0.entries[0].speaker, 1);
            speaker = findView(g_4b7ff0.entries[0].speaker);
        }
        if (speaker) {
            runViewScript(speaker, removedRgn);
            speaker->body.group = view->body.group;
            setViewsLocked(0);
            if (isSoundPlaying(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
        }
        break;
    case -1:
        g_4b7fd4 = 0;
        g_4b7fd8 = 1;
        if (!g_4b7ff0.entries[0].back && g_4b7ff0.entries[0].line
            && g_4b7ff0.entries[0].view == view->id) {
            if (g_4b7fe6[g_4b7ff0.entries[0].kind - 1] < 2) {
                g_4b7fd4 = g_4b7ff0.entries[0].speaker;
                g_4b7fd6 = g_4b7ff0.entries[0].line;
            }
            if (g_4b7ff0.entries[0].kind) {
                anchor = g_4b808c;
                after = 0;
                view->flags |= 0x4008000;
                switch (g_4b7ff0.entries[0].kind) {
                case 1:
                    if (g_4b8080) {
                        anchor = g_4b7f34[g_4b8080 - 1];
                        after = 1;
                    }
                    g_4b7f34[g_4b8080] = view->id;
                    *(Point *)&viewSnoid(view)->targetX = g_4a7770[g_4b8080];
                    g_4b8080++;
                    break;
                case 2:
                    if (g_4b8084) {
                        anchor = g_4b7f74[g_4b8084 - 1];
                        after = 1;
                    }
                    g_4b7f74[g_4b8084] = view->id;
                    *(Point *)&viewSnoid(view)->targetX = g_4a77f0[g_4b8084];
                    g_4b8084++;
                    break;
                case 3:
                    if (g_4b8086) {
                        anchor = g_4b7f94[g_4b8086 - 1];
                        after = 1;
                    }
                    g_4b7f94[g_4b8086] = view->id;
                    *(Point *)&viewSnoid(view)->targetX = g_4a7830[g_4b8086];
                    g_4b8086++;
                    break;
                default:
                    if (g_4b8082) {
                        anchor = g_4b7f54[g_4b8082 - 1];
                        after = 1;
                    }
                    g_4b7f54[g_4b8082] = view->id;
                    *(Point *)&viewSnoid(view)->targetX = g_4a77b0[g_4b8082];
                    g_4b8082++;
                    break;
                }
                viewSnoid(view)->unknownF7 = 1;
                moveView(view->id, after, anchor);
                setSnoidAction(viewSnoid(view), 10, 0);
                count = countChosenSnoids();
                if (count == g_4b8094) {
                    g_4b8098 += 2;
                    g_4b8096 = randomBetween(20055, 20063);
                } else {
                    switch (count) {
                    case 10:
                        g_4b8098++;
                        break;
                    case 12:
                        g_4b8098++;
                        break;
                    case 14:
                        g_4b8098 += 2;
                        break;
                    }
                }
            }
            if (!tunnelsGoReady)
                tunnelsGoReady = countChosenSnoids();
        } else if (g_4b7ff0.entries[0].back) {
            if (!g_4b808e && g_4b8090) {
                g_4b808e = 1;
                queueViewSound(g_4b8090, 1);
                startView(g_4b8092, randomBetween(0, 3) + 7001, 0, 0);
            }
            switch (g_4b7ff0.entries[0].kind) {
            case 1:
            case 2:
                side = 1;
                break;
            case 3:
            case 4:
                side = 0;
                break;
            }
            findWaitingPlace(&spot, side);
            view->flags = 1;
            *(Point *)&viewSnoid(view)->targetX = tunnelPlaces[spot];
            setSnoidAction(viewSnoid(view), 7, 0);
            unghostDoorView();
        }
        if (!g_4b7fd4) {
            dropFirstTunnelEntry(&g_4b7ff0);
            g_4b7fd2 = 0;
        }
        break;
    }
}

/* Scene 8's frame. Leaves the scene when asked (sceneDue) unless a
   character is talking; ends the warning (g_4b8090) when its sound has;
   says the line set up to follow (g_4b7fd4) or queues the pending sound
   (g_4b8096) when nobody's talking. While turn-backs are left
   (turnBacksLeft) it starts the first entry of g_4b7ff0: a Zoombini walks
   to its door with its script (and the entry's reply is said); one turned
   back uses up a turn-back, with a warning (4700-4703) for the last four;
   otherwise the entry is a remark to say. With none left, sends the
   waiting Zoombinis off (sendThroughDoors) and queues a closing remark. Also makes an idle remark now and then (every 5400-10800 view
   ticks), starts g_4b7fc2's script once (g_4b7fee), and every so often
   has an idle Zoombini fidget (8559 on), up to g_4b8098 times. */
/* @zoombi32 0x0045ea81 */
void tunnelsFrame()
{
    short talking = 0;
    short aside = 0;
    short id;
    View *view;
    Snoid *snoid;

    if (g_4a7888 || !g_4b7fb8)
        return;
    g_4a7888 = 1;
    if (!turnBacksLeft)
        g_4b754c = 1;
    updateViews();
    if (lastViewSound >= 4000 && lastViewSound <= 4699) {
        if (isSoundPlaying(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D')))
            talking = 1;
    } else if (lastViewSound >= 7000 && lastViewSound <= 7099) {
        if (isSoundPlaying(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D')))
            aside = 1;
    }
    if (sceneDue) {
        if (talking) {
            g_4a7888 = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeTunnels();
                g_4a7888 = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (g_4b8090 && g_4b808e && !isSoundPlaying(g_4b8090, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        g_4b8090 = 0;
        g_4b808e = 0;
    }
    if (g_4b7fd4) {
        if (!talking && !aside) {
            id = g_4b7fd4;
            g_4b7fd4 = 0;
            if (g_4b8096) {
                if (g_4b7fd8)
                    dropRemarkNotify(0, -1);
            } else {
                if (g_4b7fd8)
                    startView(id, g_4b7fd6, dropRemarkNotify, 1);
                else
                    startView(id, g_4b7fd6, 0, 1);
                loadViewSounds(id, 1);
            }
        }
    } else if (g_4b8096 && !talking && !aside) {
        queueViewSound(g_4b8096, 1);
        g_4b8096 = 0;
    }
    if (turnBacksLeft) {
        if (!g_4b808e && g_4b7ff0.count && !g_4b7fd2) {
            if (g_4b7ff0.entries[0].view) {
                if (!g_4b7ff0.entries[0].step && !talking && !aside) {
                    view = idleSnoidView(g_4b7ff0.entries[0].view);
                    if (view) {
                        g_4b7ff0.entries[0].step = 1;
                        if (g_4b7ff0.entries[0].reply && g_4b7fe6[g_4b7ff0.entries[0].kind - 1] < 2) {
                            startView(g_4b7ff0.entries[0].replier, g_4b7ff0.entries[0].reply, 0, 0);
                            loadViewSounds(g_4b7ff0.entries[0].replier, 1);
                        }
                        claimPlacedView(g_4b7ff0.entries[0].kind, 0);
                        snoid = viewSnoid(view);
                        view->flags &= ~0x4000000;
                        if (view->body.cels[20].image < 4) /* +0xa8: not a cel here? */
                            snoid->unknownF2 = 1;
                        startSnoidScript(snoid, g_4b7ff0.entries[0].script, 0, 0);
                        view->notify = tunnelsSnoidNotify;
                        view->notifyEnd = 1;
                        groupViews(id, id, 0, 0, 0, 0);
                        if (g_4b7ff0.entries[0].back) {
                            turnBacksLeft--;
                            switch (turnBacksLeft) {
                            case 1:
                                g_4b8090 = 4703;
                                break;
                            case 2:
                                g_4b8090 = 4702;
                                break;
                            case 3:
                                g_4b8090 = 4701;
                                break;
                            case 4:
                                g_4b8090 = 4700;
                                break;
                            }
                        } else {
                            view->interval = 4;
                        }
                        g_4b7fd2 = 1;
                    }
                    resetViewClock();
                }
            } else {
                sayTunnelRemark();
            }
        }
    } else if (!g_4b7fce && !g_4b7fd2) {
        if (!talking) {
            g_4b7fce = 1;
            sendThroughDoors();
            queueRemark(2);
            g_4b7fee = 1;
        }
    } else if (g_4b7ff0.count && !g_4b7fd2 && !g_4b7ff0.entries[0].view && !talking) {
        sayTunnelRemark();
    }
    if (!g_4b7fce && viewClock() > g_4b7fe0) {
        resetViewClock();
        queueRemark(0);
        g_4b7fe0 = randomBetween(5400, 10800);
    }
    if (g_4b7fee == 2) {
        g_4b7fee++;
        view = findView(g_4b7fc2);
        if (view) {
            setViewScript(view, 0, 1);
            view->flags &= ~0x1000000;
            view->notifyEnd = 1;
            view->notify = tunnelRemarkNotify;
            loadViewSounds(g_4b7fc2, 1);
            setViewsLocked(0);
        }
    }
    playAmbientSound();
    if (g_4b809a < g_4b8098 && clockTime() - g_4b809c > g_4b80a0) {
        id = 0;
        g_4b809c = clockTime();
        aside = 0; /* now the tries */
        do {
            aside++;
            view = idleSnoidView(partyViews[allocateSlot(&g_4b80a4, g_4b8094, 0)]);
            if (view && viewSnoid(view)->unknownF7 && (view->flags & 1)) {
                id = viewSnoid(view)->features[3];
                id += 8559;
                startSnoidScript(viewSnoid(view), id, 0, 0);
                g_4b809a++;
                id = 1;
            }
        } while (!id && aside < 16);
    }
    g_4a7888 = 0;
}

/* Opens scene 8: its level's number of turn-backs allowed (16-22)
   and rules (makeOneFeatureRule, makeOneValueRules, makeTwoValueRules, makeTwoFeatureRules), Tunnels.MHK,
   the backdrop, images and scripts, the views (the four placed at the
   doors, the characters, the buttons), the party, and a first remark. */
/* @zoombi32 0x0045e441 */
void openTunnels()
{
    Point places[4] = {{98, 424}, {178, 415}, {453, 421}, {533, 430}};
    short i;

    g_4b7fb8 = tunnelsGoReady = 0;
    resetTunnels();
    g_4b807e++;
    g_4b7fbe = sceneLevel();
    switch (g_4b7fbe) {
    case 0:
        turnBacksLeft = 16;
        break;
    case 1:
        turnBacksLeft = 18;
        break;
    case 2:
        turnBacksLeft = 20;
        break;
    case 3:
        turnBacksLeft = 22;
        break;
    }
    addSoundRange(20000, 29999, 1);
    addSoundRange(4000, 4699, 1);
    addSoundRange(7000, 7099, 1);
    addSoundRange(425, 499, 0);
    addSoundRange(4700, 4799, 1);
    addSoundRange(6000, 6099, 1);
    addSoundRange(175, 199, 0);
    addSoundRange(99, 99, 0);
    addSoundRange(8500, 8599, 0);
    openGameFile(&g_4b7fb4, "Tunnels.MHK");
    setCurrentMap(g_4b7fb4);
    loadPaths(1000);
    loadTerrain(100);
    drawBackdrop(300);
    g_4a770c = loadImageBank(400, &g_4a7708);
    loadFeatureGroup(5000, 0, 0);
    loadFeatureGroup(6000, 1, 0);
    loadFeatureGroup(7000, 2, 0);
    loadFeatureGroup(9000, 3, 0);
    loadFeatureGroup(4000, 4, 0);
    loadFeatureGroup(4200, 5, 0);
    loadFeatureGroup(4400, 6, 0);
    loadFeatureGroup(4600, 7, 0);
    loadScripts(5000, 4);
    addScripts(6000, 12, 0);
    addScripts(7000, 5, 0);
    addScripts(9000, 7, 0);
    loadSnoidScripts(8000, 8, 0);
    addSnoidScripts(8500, 65, 5);
    addScripts(4000, 39, 2);
    addScripts(4200, 27, 2);
    addScripts(4400, 24, 2);
    addScripts(4600, 18, 2);
    g_4b808c = addView(0x8000, drawCels, runViewScript, 9000, 0, 0, 0, 0);
    for (i = 0; i < 4; i++)
        placedViews[i] = addView(0x108a000, drawCels, runViewScript, i + 5000, 6, &places[i], 0, 0);
    g_4b8092 = addView(0xc180000, drawCels, runViewScript, 7001, 6, 0, 0, 0);
    {
        short order[4] = {1, 2, 0, 3};

        for (i = 0; i < 4; i++)
            tunnelsSpeakers[order[i]] = addView(0xc180000, drawCels, runViewScript, order[i] + 6000, 6, 0, 0, 0);
    }
    for (i = 9001; i <= 9006; i++)
        addView(0, drawCels, runViewScript, i, 6, 0, 0, 0);
    g_4b7fc2 = addView(0xd181000, drawCels, runViewScript, 7000, 6, 0, 0, 0);
    copyPaletteRange(10, 236);
    g_4b7fcc = addView(0x1000, drawTunnelsButtons, updateTunnelsButtons, 0, 0, 0, 0, 0);
    moveView(g_4b7fc2, 0, g_4b7fcc);
    setViewPlaces(16, tunnelPlaces, 1);
    makePartySnoids(0);
    enterSnoids(100);
    updateViews();
    staggerSnoids(45, 30);
    switch (g_4b7fbe) {
    case 0:
        makeOneFeatureRule();
        break;
    case 1:
        makeOneValueRules();
        break;
    case 2:
        makeTwoValueRules();
        break;
    case 3:
        makeTwoFeatureRules();
        break;
    }
    setGroupLists(tunnelsGroups, 1, (short)0xc000);
    drawTunnelsButton(1, 0, 0);
    drawTunnelsButton(2, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    g_4b7fb8 = 1;
    g_4b8094 = countSnoidViews();
    campHint((short *)(gameState + 0x2c));
    hintSound = randomBetween(20069, 20070);
    queueRemark(1);
    g_4b7fe0 = randomBetween(5400, 10800);
}

/* Scene 8's clicks: 1 leaves (asking whether to keep the party), 2 sends
   the Zoombinis on (once all have gone through or given up) with a closing
   remark, 3 drags a Zoombini. Dropped at a door (heldPlaceNumber), it
   queues an entry in g_4b7ff0 for it to go through or be turned back
   (turnedBackAtDoor, under the rules; on level 0 the first rule's side decides a
   door's result), with what the characters say about it; a Zoombini taken
   off the queue goes back to a free waiting place. */
/* Not exact: register allocation (the original keeps `view` in esi and
   `remark` in edi, the other way round, whatever the declaration order),
   and it adds 8000 to `remark` as 32 bits (`add edi, 0x1f40`). */
/* @zoombi32 0x0045eff0 */
void tunnelsClicked(short which)
{
    short remark;
    View *view;
    Point where;
    volatile long home; /* where it stood (unused) */
    Snoid *snoid;
    short first;
    short back;
    short removed;
    short speaker;
    short line;
    short lineThen;
    short replier;
    short reply;
    short replyThen;
    short script;
    TunnelEntry entry;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeTunnels();
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawTunnelsButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawTunnelsButton(which, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (!tunnelsGoReady)
            break;
        if (g_4b7fce && !g_4b7fd0)
            break;
        g_4b7564 = 0;
        drawTunnelsButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawTunnelsButton(which, 0, 1);
        sendSnoids(670, 30, 45);
        sceneDue = 9;
        if (lastViewSound && isSoundPlaying(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            lastViewSound = 0;
        }
        queueRemark(3);
        break;
    case 3:
        if (!g_4b7fce && g_4b7ff0.count && !g_4b7ff0.entries[0].view) {
            g_4b7ff0.count = 0;
            g_4b7fd2 = 0;
        }
        if (snoidsOnTheirWay > 0 || g_4b7fce)
            break;
        removed = 0;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (!view)
            view = viewAt(where, 0x4008001, 1);
        if (!view || g_4b754c)
            break;
        snoid = viewSnoid(view);
        which = snoid->unknownF7;
        if (snoid->unknownF4 && snoid->unknownF4 != 6)
            break;
        home = *(long *)&view->body.x;
        if (which)
            g_4b7556 = 1;
        dragSnoid(view, where, 0, 0);
        if (which)
            break;
        which = removeTunnelEntry(&g_4b7ff0, view->id);
        if (which) {
            claimPlacedView(which, 0);
            removed = 1;
            view->flags &= ~0x4000000;
        }
        which = heldPlaceNumber();
        if (which) {
            line = lineThen = 0;
            replier = reply = replyThen = 0;
            view->flags |= 0x4000000;
            back = turnedBackAtDoor(&g_4b7f18, which, viewSnoid(view), (unsigned short *)&first);
            switch (which) {
            case 1:
                if (!first)
                    remark = 0;
                else
                    remark = 1;
                break;
            case 2:
                if (!first)
                    remark = 2;
                else
                    remark = 3;
                break;
            case 3:
                if (first)
                    remark = 4;
                else
                    remark = 5;
                break;
            case 4:
                if (first)
                    remark = 6;
                else
                    remark = 7;
                break;
            }
            if (!back && !g_4b7fbe) {
                if (g_4b7fbc) {
                    if (which == 1 || which == 4)
                        back = 1;
                } else if (which == 2 || which == 3) {
                    back = 1;
                }
            }
            view->body.cels[20].image = remark;
            speaker = tunnelsSpeakers[doorSpeakers[remark]];
            if (first) {
                if (remark < 4) {
                    replier = tunnelsSpeakers[0];
                    reply = g_4a75e8[allocateSlot(&g_4a7600, 11, 0)];
                } else {
                    replier = tunnelsSpeakers[3];
                    reply = g_4a7640[allocateSlot(&g_4a764c, 6, 0)];
                }
            }
            if (back) {
                g_4b7fe6[which - 1] = 0;
                line = remark + 6004;
                switch (doorSpeakers[remark]) {
                case 0:
                    g_4b8088++;
                    do
                        lineThen = g_4a75d0[allocateSlot(&g_4a75e4, 10, 0)];
                    while (lineThen == 4005 && g_4b8088 < 3);
                    break;
                case 1:
                    lineThen = g_4a7650[allocateSlot(&g_4a7658, 4, 0)];
                    break;
                case 2:
                    lineThen = g_4a7604[allocateSlot(&g_4a7614, 8, 0)];
                    break;
                case 3:
                    g_4b808a++;
                    do
                        lineThen = g_4a762c[allocateSlot(&g_4a763c, 7, 0)];
                    while (lineThen == 4416 && g_4b808a < 3);
                    break;
                }
                script = remark * 5 + snoid->features[3] + 8519;
                remark += 8000;
            } else {
                g_4b808a = g_4b8088 = 0;
                g_4b7fe6[which - 1]++;
                switch (remark) {
                case 1:
                case 6:
                    line = g_4a765c[allocateSlot(&g_4a7668, 6, 0)];
                    break;
                case 3:
                case 4:
                    line = g_4a7618[allocateSlot(&g_4a7628, 8, 0)];
                    break;
                }
                script = (snoid->features[3] - 1) * 4 + remark / 2 + 8500;
                remark = 0;
                if (countChosenSnoids() + 1 >= g_4b8094)
                    replier = reply = replyThen = 0;
            }
            entry.view = view->id;
            entry.back = back;
            entry.step = 0;
            entry.from = *(long *)&snoid->body.x;
            entry.script = script;
            entry.backScript = remark;
            entry.speaker = speaker;
            entry.line = line;
            entry.lineThen = lineThen;
            entry.replier = replier;
            entry.reply = reply;
            entry.replyThen = replyThen;
            entry.kind = which;
            addTunnelEntry(&g_4b7ff0, entry);
        } else if (removed) {
            if (snoid->body.x != snoid->targetX || snoid->body.y != snoid->targetY)
                pickFreePlace((Point *)&snoid->targetX, tunnelPlaces, 16, 500);
        }
        break;
    }
}
