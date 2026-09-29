/*
 * xfer (0x4696f0-0x46be28): 'xfer.MHK'
 */

#include "zoombinis.h"
#include "e2memory.h"
#include "features.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"
#include "xfer.h"

/* @zoombi32 0x0046b07b */
long fn_46b07b(long)
{
    return 0;
}

/* @zoombi32 0x0046b747 */
void fn_46b747(long, short id)
{
    if (id == 30)
        g_4b0d52 = g_4b0d54;
}

/* Closes scene 2. */
/* @zoombi32 0x0046ac6e */
void closeScene2()
{
    if (g_4b98d8) {
        g_4b98d8 = 0;
        short saved = fn_46bee9(1);

        g_4a7e68 = 0;
        useAltSnoids(1);
        chooseSnoids(1, 1);
        clearViews();
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4b98d4);
        fadeOutViews();
        fn_4624fc();
        g_4a4b98 = 64;
        if (g_4a7d3c)
            g_4a7d3c--;
    }
}

/* Marks the cell at `cell` taken (g_4b99c0 becomes g_4b99c1, g_4b99c2
   becomes g_4b99c3), noting the point in a free one of g_4b9944. */
/* @zoombi32 0x0046bb0c */
void fn_46bb0c(char *cell, short x, short y)
{
    short i;

    if (*cell == g_4b99c0)
        for (i = 0; i < 24; i++)
            if (!g_4b99a4[i]) {
                g_4b9944[i].x = x;
                g_4b9944[i].y = y;
                g_4b99a4[i] = 1;
                if (g_4b9934 > 0)
                    g_4b9934--;
                *cell = g_4b99c1;
                return;
            }
    if (*cell == g_4b99c2)
        for (i = 0; i < 24; i++)
            if (!g_4b99a4[i]) {
                g_4b9944[i].x = x;
                g_4b9944[i].y = y;
                g_4b99a4[i] = 1;
                if (g_4b9934 > 0)
                    g_4b9934--;
                *cell = g_4b99c3;
                return;
            }
}

/* A view's placed callback: keeps one of its first four cels by
   g_4b9928 (1-4) as the first, alone. */
/* @zoombi32 0x0046bbce */
void fn_46bbce(View *view)
{
    ViewCel *cels = view->body.cels;

    switch (g_4b9928) {
    case 1:
    default:
        cels[1].image = 0;
        break;
    case 2:
        cels[0].image = cels[1].image;
        cels[0].x = cels[1].x;
        cels[0].y = cels[1].y;
        cels[1].image = 0;
        break;
    case 3:
        cels[0].image = cels[2].image;
        cels[0].x = cels[2].x;
        cels[0].y = cels[2].y;
        cels[1].image = 0;
        break;
    case 4:
        cels[0].image = cels[3].image;
        cels[0].x = cels[3].x;
        cels[0].y = cels[3].y;
        cels[1].image = 0;
        break;
    }
}

/* A view update: runs the script, and when due redraws it. */
/* Not exact: the original keeps `region` in esi (loaded once); here it is
   read from the stack at each use, `register` or not. */
/* @zoombi32 0x0046bdde */
void fn_46bdde(View *view, short region)
{
    if (!g_4b9684) {
        runViewScript(view, region);
        if (view->nextUpdate <= updateTime) {
            view->changed = 1;
            view->nextUpdate = updateTime + view->interval;
            unionRgnRect(region, &view->body.bounds);
        }
    }
}
