/*
 * bridge (0x41a404-0x41c09c): the Allergic Cliffs (scene 7), 'bridge.mhk'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "bridge.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "loading.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "tunnels.h"
#include "view.h"

/* Starts the timer. */
/* @zoombi32 0x0041a404 */
void startBridgeTimer()
{
    g_4ab7d4 = clockTime();
}

/* The ticks since startBridgeTimer. */
/* @zoombi32 0x0041a40f */
unsigned long bridgeTimer()
{
    return clockTime() - g_4ab7d4;
}

/* Resets scene 7's state (the rules too); the pace g_4ab834 by
   g_4b2b00. */
/* @zoombi32 0x0041a41b */
void resetScene7()
{
    short i;

    g_4ab826 = g_4ab828 = -1;
    g_4ab7f0 = 0;
    g_4b755e = 55;
    g_4ab7ea = g_4ab7e6 = 0;
    g_4ab7ee = g_4b0d52 = g_4ab800 = 0;
    g_4ab7f2 = g_4b966e = 0;
    g_4ab78c = g_4ab78e = 0;
    for (i = 0; i < 16; i++)
        g_4ab792[i] = g_4ab7b2[i] = 0;
    g_4b755a = g_4b755c = g_4ab802 = 0;
    g_4ab7d8 = g_4ab824 = 0;
    g_4ab82a = g_4ab82c = 0;
    g_4ab830 = g_4ab838 = 0;
    if (g_4b2b00)
        g_4ab834 = 120;
    else
        g_4ab834 = 60;
    fillMemory(&bridgeRules, 0, 28);
}

/* Draws button `which` (1 or 2; 2 is dim unless g_4ab78a), lit or not,
   and shows it if asked. */
/* @zoombi32 0x0041a8af */
void drawBridgeButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4ab78a) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4ab820->offsets[image] + (char *)g_4ab820), bridgeButtons[which - 1].rect.left,
                      bridgeButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&bridgeButtons[which - 1].rect);
    }
}

/* The buttons' view update: redraws button 2 as g_4ab78a changes, and
   button 1 once. */
/* @zoombi32 0x0041a965 */
void fn_41a965(View *, short region)
{
    if (g_4ab78a) {
        if (!g_4a0f08) {
            g_4a0f08 = 1;
            unionRgnRect(region, &bridgeButtons[1].rect);
        }
    } else if (g_4a0f08) {
        g_4a0f08 = 0;
        unionRgnRect(region, &bridgeButtons[1].rect);
    }
    if (!g_4a0f0a) {
        g_4a0f0a = 1;
        unionRgnRect(region, &bridgeButtons[0].rect);
    }
}

/* Closes scene 7. */
/* @zoombi32 0x0041a9d7 */
void closeScene7()
{
    if (g_4ab788) {
        g_4ab788 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a0e24);
        fn_46bee9(saved);
        fn_46ca9c(&g_4ab784);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Whether `snoid` is turned back at `edge` (1 or 2; others count as 1)
   under the first rule: whether it matches (any of its features' values),
   inverted unless the rule's side, and again for edge 2. */
/* Not exact: register allocation (the original keeps `snoid` on the stack
   and `passes` in ebx, with ecx and esi as scratch; here `snoid` takes esi),
   as in the tunnels' fn_460c41. */
/* @zoombi32 0x0041c00c */
short turnedBack(FeatureRules *rules, short edge, Snoid *snoid)
{
    short passes;
    short i;

    if (edge < 1 || edge > 2)
        edge = 1;
    passes = 0;
    for (i = 0; i < rules->rules[0].count; i++)
        if (snoid->features[(short)rules->rules[0].features[i] - 1] == rules->rules[0].values[i])
            passes = 1;
    if (passes)
        passes = rules->rules[0].side;
    else
        passes = !rules->rules[0].side;
    if (edge == 2)
        passes = !passes;
    return !passes;
}

/* A notify: 0 sets g_4ab7e6, 1-6 note the event in g_4ab802, 100 and 101
   start g_4ab7e4's script (1236 in this view's group, or 1103); at the end
   (-1), with fewer chosen than g_4ab82e, now and then a remark
   (20045-20048). */
/* @zoombi32 0x0041b357 */
void fn_41b357(View *view, short event)
{
    View *other;

    switch (event) {
    case 0:
        g_4ab7e6 = 1;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        g_4ab802 = event;
        break;
    case 10:
        break;
    case 100:
    case 101:
        other = findView(g_4ab7e4);
        if (other) {
            if (event == 100) {
                setViewScript(other, 1236, 1);
                other->body.group = view->body.group;
            } else {
                setViewScript(other, 1103, 1);
            }
        }
        break;
    case -1:
        if (countChosenSnoids() < g_4ab82e
            && (randomBetween(0, 4) > g_4ab790 || (*(short *)(g_4a4ba0 + 0x2a) & 0xfff) <= 3)
            && countChosenSnoids())
            queueViewSound(randomBetween(20045, 20048), 0);
        break;
    }
}

/* Scene 7's keys (with debugging on, g_4b8803, or else only 0x16f; case
   ignored; only while the scene is open and nobody's moving): 0x16f
   fn_466b93; R reports the script and type g_4ab826/g_4ab828; A shows the
   rule. Returns whether the key was used. */
/* @zoombi32 0x0041b203 */
short scene7Key(unsigned short key)
{
    short used = 0;
    ShortRect area = {225, 0, 350, 70};
    short x;

    if (!g_4b8803 && key != 0x16f)
        return 0;
    if (key >= 'a' && key <= 'z')
        key -= 32;
    if (!g_4ab788 || g_4b755a > 0)
        return 0;
    switch (key) {
    case 0x16f:
        fn_466b93();
        used = 1;
        break;
    case 'R':
        debugMessage(g_4ab826, "Snoid Script:", &g_4ab828, " Type:", 1);
        break;
    case 'A':
        unionRgnRect(removedRgn, &area);
        unionRgnRect(removedRgn, &debugRect);
        updateViews();
        if (bridgeRules.rules[0].side)
            debugMessage(-1, "Upper bridge accepts:", 0, 0, 0);
        else
            debugMessage(-1, "Lower bridge accepts:", 0, 0, 0);
        area.left = 250;
        for (used = 0; used < bridgeRules.rules[0].count; used++) {
            x = area.left;
            area.top = 20;
            drawFeature(bridgeRules.rules[0].features[used], bridgeRules.rules[0].values[used], &area);
            area.left = x + 30;
        }
        used = 1;
        break;
    }
    return used;
}

/*
 * A Zoombini's notify at the cliffs: 10 starts its crossing script by how
 * it's going (g_4ab802: 1000-1016, at the upper or lower bridge by
 * g_4ab7ec); 1-2 and 4-5 set g_4ab7f2; 3 (lower) and 6 (upper) mean it
 * got across: it walks to the next place on that side (g_4ab7b2/g_4ab792),
 * stacked among the others, counts toward g_4ab82a (more as the chosen run
 * out) and a cheer when all are over; 20 means it was sent back (g_4ab7da,
 * up to 6); at the end (-1) it finds a spot back by its bridge.
 */
/* @zoombi32 0x0041b453 */
void fn_41b453(View *view, short event)
{
    Point anchor;
    View *other;
    short script;
    short after;
    short n;

    switch (event) {
    case 10:
        other = findView(view->id);
        if (!other || !g_4ab802)
            break;
        switch (g_4ab802) {
        case 0:
        case 1:
        case 6:
        default:
            script = 1016;
            break;
        case 2:
            script = 1012;
            break;
        case 3:
            script = 1008;
            break;
        case 4:
            script = 1000;
            break;
        case 5:
            script = 1004;
            break;
        }
        switch (g_4ab7ec) {
        case 1:
            script += 2;
            anchor.x = 38;
            anchor.y = 106;
            break;
        default:
            anchor.x = 56;
            anchor.y = 205;
            break;
        }
        script += randomBetween(0, 1);
        unionRgnRect(currentViewRgn, &other->body.bounds);
        startSnoidScript(viewSnoid(other), script, &anchor, 0);
        g_4ab826 = script;
        g_4ab828 = g_4ab802;
        other->body.group = g_4ab7f0;
        loadViewSounds(view->id, 1);
        g_4ab802 = 0;
        break;
    case 2:
    case 5:
        g_4ab7f2 = g_4ab7dc;
        break;
    case 1:
    case 4:
        g_4ab7f2 = g_4ab7e0;
        break;
    case 3:
    case 6:
        g_4ab7ee--;
        view->notifyEnd = 0;
        setSnoidAction(viewSnoid(view), 7, 0);
        view->flags |= 0x4008000;
        if (event == 6) {
            g_4ab792[g_4ab78c] = view->id;
            *(Point *)&viewSnoid(view)->targetX = upperPlaces[g_4ab78c];
            switch (g_4ab78c) {
            case 0:
                after = 0;
                break;
            case 5:
            case 6:
                after = g_4ab792[g_4ab78c - 1];
                script = 1;
                break;
            default:
                after = g_4ab792[g_4ab78c - 1];
                script = 0;
                break;
            case 7:
                after = g_4ab792[0];
                script = 1;
                break;
            case 10:
                after = g_4ab792[7];
                script = 1;
                break;
            case 15:
                after = g_4ab792[9];
                script = 0;
                break;
            }
            g_4ab78c++;
        } else {
            g_4ab7b2[g_4ab78e] = view->id;
            *(Point *)&viewSnoid(view)->targetX = lowerPlaces[g_4ab78e];
            switch (g_4ab78e) {
            case 0:
                after = 0;
                break;
            default:
                after = g_4ab7b2[g_4ab78e - 1];
                script = 0;
                break;
            }
            g_4ab78e++;
        }
        if (after)
            moveView(view->id, script, after);
        n = countChosenSnoids();
        if (!g_4ab78a)
            g_4ab78a = n;
        switch (n) {
        case 10:
            g_4ab82a++;
            break;
        case 12:
            g_4ab82a++;
            break;
        case 14:
            g_4ab82a += 2;
            break;
        }
        if (n == g_4ab82e)
            g_4ab82a += 2;
        g_4b7566 = 1;
        viewSnoid(view)->unknownF7 = 2;
        if (n == g_4ab82e && !g_4ab7ee)
            queueViewSound(randomBetween(20055, 20063), 0);
        break;
    case 20:
        g_4ab7ee--;
        if (g_4ab7da < 6)
            g_4ab7da++;
        g_4ab824 = 1;
        break;
    case -1:
        view->notifyEnd = 0;
        g_4ab824 = 0;
        if (g_4ab7ea)
            g_4ab7ea = 0;
        {
            ShortRect *area = g_4ab7ec == 1 ? &g_4a0ea8 : &g_4a0eb0;

            findSpot(view, area, 1, 36);
        }
        break;
    }
}

/*
 * Makes the cliffs' rule for the level (g_4ab790): builds the masks of
 * feature values it may use (0: one value; 1: either of two values of one
 * feature; 2: a value of each of two features; 3: 500 combinations), counts
 * the chosen Zoombinis matching each, and picks at random among those
 * matching as near half as possible (skipping, at level 0, the count of the
 * last rule, g_4b7548). The rule's side is random.
 */
/* Not exact: register allocation (the original keeps `masks` in ebx and
   `value` in esi, saving esi around the arrays' copies; here they're the
   other way round, whatever the declaration order or scope). */
/* @zoombi32 0x0041b812 */
void makeBridgeRule()
{
    unsigned short *counts;
    ChosenSnoids *chosen;
    unsigned long n;
    unsigned long picked;
    unsigned long j;
    unsigned long best;
    unsigned long shift;
    unsigned long shift2;
    unsigned long step;
    unsigned long step2;
    unsigned long step3;
    unsigned long mask1;
    unsigned long mask2;
    unsigned long base[10] = {0x12, 0x13, 0x14, 0x15, 0x23, 0x24, 0x25, 0x34, 0x35, 0x45};
    unsigned long low[6] = {0x1, 0x1, 0x1, 0x100, 0x100, 0x10000};
    unsigned long high[6] = {0x100, 0x10000, 0x1000000, 0x10000, 0x1000000, 0x1000000};
    unsigned long value;
    unsigned long *masks;
    unsigned long i;
    unsigned long k;
    unsigned long m;
    long matches;
    unsigned long target;
    long pick;

    masks = (unsigned long *)newPtr(2000);
    if (!masks)
        fatalError(msgOutOfMemory);
    counts = (unsigned short *)newPtr(1000);
    if (!counts)
        fatalError(msgOutOfMemory);
    chosen = listChosenSnoids();
    for (i = 0; i < 500; i++) {
        masks[i] = 0;
        counts[i] = 0;
    }
    switch (g_4ab790) {
    case 0:
        n = 20;
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
        break;
    case 1:
        n = 40;
        shift = 0;
        k = 0;
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 10; i++) {
                masks[k] = base[i] << shift;
                k++;
            }
            shift += 8;
        }
        for (j = 0; j < chosen->count; j++) {
            value = swapLong(*(unsigned long *)chosen->features[j]);
            for (i = 0; i < n; i++)
                if ((value & 0xf) == (masks[i] & 0xf) || (value & 0xf00) == (masks[i] & 0xf00)
                    || (value & 0xf0000) == (masks[i] & 0xf0000)
                    || (value & 0xf000000) == (masks[i] & 0xf000000)
                    || (value & 0xf) == (masks[i] & 0xf0) >> 4
                    || (value & 0xf00) == (masks[i] & 0xf000) >> 4
                    || (value & 0xf0000) == (masks[i] & 0xf00000) >> 4
                    || (value & 0xf000000) == (masks[i] & 0xf0000000) >> 4)
                    counts[i]++;
        }
        break;
    case 2:
        n = 150;
        k = 0;
        for (j = 0; j < 6; j++)
            for (i = 1; i <= 5; i++) {
                step = high[j] * i + low[j];
                for (m = 1; m <= 5; m++) {
                    masks[k] = step;
                    k++;
                    step += low[j];
                }
            }
        break;
    case 3:
        n = 500;
        k = 0;
        for (j = 0; j < 4; j++) {
            switch (j) {
            case 0:
                value = 0x10101;
                step = 1;
                mask1 = 0xf0f00;
                step2 = 0x100;
                mask2 = 0xf000f;
                step3 = 0x10000;
                shift = 0;
                shift2 = 8;
                break;
            case 1:
                value = 0x1000101;
                step = 1;
                mask1 = 0xf000f00;
                step2 = 0x100;
                mask2 = 0xf00000f;
                step3 = 0x1000000;
                shift = 0;
                shift2 = 8;
                break;
            case 2:
                value = 0x1010001;
                step = 1;
                mask1 = 0xf0f0000;
                step2 = 0x10000;
                mask2 = 0xf00000f;
                step3 = 0x1000000;
                shift = 0;
                shift2 = 16;
                break;
            case 3:
                value = 0x1010100;
                step = 0x100;
                mask1 = 0xf0f0000;
                step2 = 0x10000;
                mask2 = 0xf000f00;
                step3 = 0x1000000;
                shift = 8;
                shift2 = 16;
                break;
            }
            for (i = 0; i < 125; i++) {
                masks[i + k] = value;
                value += step;
                if (((value >> shift) & 0xf) == 6) {
                    value &= mask1;
                    value += step + step2;
                    if (((value >> shift2) & 0xf) == 6) {
                        value &= mask2;
                        value += step2 + step3;
                    }
                }
            }
            k += 125;
        }
        break;
    }
    if (g_4ab790 != 1)
        for (j = 0; j < chosen->count; j++) {
            value = swapLong(*(unsigned long *)chosen->features[j]);
            for (i = 0; i < n; i++)
                if ((value & 0xf) == (masks[i] & 0xf) || (value & 0xf00) == (masks[i] & 0xf00)
                    || (value & 0xf0000) == (masks[i] & 0xf0000)
                    || (value & 0xf000000) == (masks[i] & 0xf000000))
                    counts[i]++;
        }
    matches = 0;
    value = 1;
    target = chosen->count / 2;
    while (!matches) {
        if (target > 0 && target < 16) {
            for (i = 0; i < n; i++)
                if (target == counts[i])
                    matches++;
            best = target;
        }
        target += value;
        /* value (the step here) is unsigned, so the test is always false (as in the tunnels'
           fn_460e3d). */
        if (value < 0)
            value--;
        value++;
        value = -value;
    }
    pick = randomBetween(1, matches);
    for (i = 0; i < n; i++)
        if (counts[i] == best && !--pick) {
            picked = masks[i];
            i = n;
        }
    g_4b7548 = 0;
    g_4b7544 = 0;
    if (!g_4ab790 && matches == 1) {
        g_4b7548 = best;
        g_4b7544 = picked;
    }
    bridgeRules.count = 1;
    bridgeRules.rules[0].side = randomBetween(0, 1);
    switch (g_4ab790) {
    case 0:
        bridgeRules.rules[0].count = 1;
        if (picked & 0xff) {
            bridgeRules.rules[0].features[0] = 4;
            bridgeRules.rules[0].values[0] = picked & 0xf;
        } else if (picked & 0xff00) {
            bridgeRules.rules[0].features[0] = 3;
            bridgeRules.rules[0].values[0] = (picked >> 8) & 0xf;
        } else if (picked & 0xff0000) {
            bridgeRules.rules[0].features[0] = 2;
            bridgeRules.rules[0].values[0] = (picked >> 16) & 0xf;
        } else if (picked & 0xff000000) {
            bridgeRules.rules[0].features[0] = 1;
            bridgeRules.rules[0].values[0] = (picked >> 24) & 0xf;
        }
        break;
    case 1:
        bridgeRules.rules[0].count = 2;
        if (picked & 0xff) {
            bridgeRules.rules[0].features[0] = 4;
            bridgeRules.rules[0].values[0] = picked & 0xf;
            bridgeRules.rules[0].features[1] = 4;
            bridgeRules.rules[0].values[1] = (picked & 0xf0) >> 4;
        } else if (picked & 0xff00) {
            bridgeRules.rules[0].features[0] = 3;
            bridgeRules.rules[0].values[0] = (picked >> 8) & 0xf;
            bridgeRules.rules[0].features[1] = 3;
            bridgeRules.rules[0].values[1] = (picked >> 12) & 0xf;
        } else if (picked & 0xff0000) {
            bridgeRules.rules[0].features[0] = 2;
            bridgeRules.rules[0].values[0] = (picked >> 16) & 0xf;
            bridgeRules.rules[0].features[1] = 2;
            bridgeRules.rules[0].values[1] = (picked >> 20) & 0xf;
        } else if (picked & 0xff000000) {
            bridgeRules.rules[0].features[0] = 1;
            bridgeRules.rules[0].values[0] = (picked >> 24) & 0xf;
            bridgeRules.rules[0].features[1] = 1;
            bridgeRules.rules[0].values[1] = (picked >> 28) & 0xf;
        }
        break;
    case 2:
    case 3:
        i = 0;
        bridgeRules.rules[0].count = 0;
        if (picked & 0xff) {
            bridgeRules.rules[0].features[i] = 4;
            bridgeRules.rules[0].values[i] = picked & 0xf;
            i++;
            bridgeRules.rules[0].count++;
        }
        if ((picked & 0xff00) && i < g_4ab790) {
            bridgeRules.rules[0].features[i] = 3;
            bridgeRules.rules[0].values[i] = (picked >> 8) & 0xf;
            i++;
            bridgeRules.rules[0].count++;
        }
        if ((picked & 0xff0000) && i < g_4ab790) {
            bridgeRules.rules[0].features[i] = 2;
            bridgeRules.rules[0].values[i] = (picked >> 16) & 0xf;
            i++;
            bridgeRules.rules[0].count++;
        }
        if ((picked & 0xff000000) && i < g_4ab790) {
            bridgeRules.rules[0].features[i] = 1;
            bridgeRules.rules[0].values[i] = (picked >> 24) & 0xf;
            bridgeRules.rules[0].count++;
        }
        break;
    }
    disposePtr(masks);
    disposePtr(counts);
}
