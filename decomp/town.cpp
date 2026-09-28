/*
 * town (0x45c0f4-0x45e2d8): the town (scene 0), 'Town.MHK'
 */

#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "town.h"
#include "view.h"

/* Opens scene 0. */
/* @zoombi32 0x0045c12e */
void openScene0()
{
    g_4b7cf4 = g_4b7cf8 = g_4b7cf6 = 0;
    g_4b0d52 = g_4b7cec = 0;
    g_4b7cf0 = 1;
    setGroupLists(townGroups, 1, (short)0xc000);
    g_4b7cf4 = 1;
}

/* Scene 0's clicks: moves g_4a7410 on from 1 to 2 (always, with
   g_4b7cf8); 1 or -1 goes back to the scene fn_454c10 picks. */
/* @zoombi32 0x0045c391 */
void scene0Clicked(short which)
{
    if (g_4b7cf8)
        g_4a7410 = 2;
    if (which > 0 && g_4a7410 == 1)
        g_4a7410 = 2;
    if (abs(which) == 1) {
        g_4b7cf0 = g_4b7cf6 = 0;
        g_4b0d52 = fn_454c10();
    }
}

/* Every 1800 ticks, reads the time for the clock: its minute hand
   (0-11, in fives) and hour hand (0-11). */
/* @zoombi32 0x0045c4c9 */
void fn_45c4c9()
{
    unsigned long now;
    short year;
    char ignored;

    now = clockTime();
    if (now > g_4b7efc + 1800) {
        g_4b7efc = now;
        getDateTime(&year, &ignored, &ignored, (char *)&clockHour, (char *)&clockMinute);
        clockMinute = clockMinute / 5;
        clockHour = clockHour % 12;
    }
}

/* Sets whether the views g_4b7ece and the first g_4b7f02 party views run
   their scripts. */
/* @zoombi32 0x0045ccca */
void fn_45ccca(short running)
{
    short i;
    View *view;

    for (i = 0; i <= 19; i++) {
        view = findView(g_4b7ece[i]);
        if (view)
            view->body.running = running;
    }
    for (i = 0; i < g_4b7f02; i++) {
        view = findView(partyViews[i]);
        if (view)
            view->body.running = running;
    }
}

/* A view update: once reset, adds g_4a7428 to `region`. */
/* @zoombi32 0x0045cf8b */
void fn_45cf8b(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &g_4a7428);
    }
}

/* A script from 3000-3002 by g_4a4ba0's +0x46. */
/* @zoombi32 0x0045d04c */
short fn_45d04c()
{
    short script;

    script = ((*(short *)(g_4a4ba0 + 0x46) - 1) & 0xfff) % 3 + 3000;
    if (script < 3000)
        script = 3000;
    if (script >= 3003)
        script = 3002;
    return script;
}

/* A view's placed callback: drops its cels whose image is past g_4b7e10. */
/* @zoombi32 0x0045daf7 */
void fn_45daf7(View *view)
{
    ViewCel *cel;

    for (cel = view->body.cels; cel->image;)
        if (cel->image > g_4b7e10)
            removeFirstCel(cel);
        else
            cel++;
}

/* A notify: negates the view's entry in g_4b7ece (the first 19) and counts
   it in g_4b7f10. */
/* @zoombi32 0x0045e29e */
void fn_45e29e(View *view, short)
{
    short id = view->id;
    short i;

    for (i = 0; i < 19; i++)
        if (id == g_4b7ece[i]) {
            g_4b7ece[i] = -g_4b7ece[i];
            g_4b7f10++;
            break;
        }
}
