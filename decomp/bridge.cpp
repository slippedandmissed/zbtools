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
