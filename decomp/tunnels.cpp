/*
 * tunnels (0x45e2d8-0x4623b8): Stone Cold Caves (scene 8), 'Tunnels.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "tunnels.h"
#include "view.h"

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
