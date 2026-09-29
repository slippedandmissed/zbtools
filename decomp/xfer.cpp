/*
 * xfer (0x4696f0-0x46be28): 'xfer.MHK'
 */

#include <stdio.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "e2memory.h"
#include "features.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"
#include "xfer.h"

/* Resets scene 2's state. */
/* @zoombi32 0x004696f0 */
void resetScene2()
{
    short i;

    g_4b0d52 = g_4b9914 = 0;
    g_4b9904 = g_4b9906 = g_4b9908 = g_4b990a = 0;
    g_4b9922 = g_4b9924 = g_4b9926 = g_4b9920 = g_4b9928 = 0;
    g_4b9912 = 0;
    g_4b992c = 0;
    for (i = 0; i < 3; i++)
        g_4b990c[i] = 1;
    for (i = 0; i < 4; i++)
        g_4b98f8[i] = 0;
    for (i = 0; i < 2; i++)
        g_4b9900[i] = 0;
    for (i = 0; i < 17; i++)
        g_4b98e4[i] = 0;
    g_4b98f6 = g_4b991c = g_4b991c = 0;
    g_4a4b98 = 0;
    g_4b9916 = 0;
    g_4b98da = population();
    g_4b98dc = -1;
    g_4b9918 = g_4b991a = 0;
    g_4b98e0 = 0;
}

/* @zoombi32 0x0046b07b */
long fn_46b07b(long)
{
    return 0;
}

/* Draws the population sign's view while it runs (then stops it): its
   cels and "zoombiniville population N", straight to the screen. */
/* @zoombi32 0x0046b761 */
void fn_46b761(View *view)
{
    Color saved;
    ShortRect rect;
    char text[32];

    if (view->body.running) {
        drawCels(view);
        saved = setForeColor(Color(45));
        rect = view->body.bounds;
        rect.left += 16;
        rect.top += 8;
        sprintf(text, "%s%d", levelTexts[10], g_4b98da);
        drawOutlinedText(0x70, 0xd1, rect, 1, text);
        copyPortBits(viewPort, workPort, view->body.bounds, view->body.bounds, 0);
        view->body.running = 0;
        setForeColor(saved);
    }
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

/* Sets up the grid (`rows` of `columns` cells, `stride` apart): cells
   `from1` become `to1` and `from2` `to2`, counted in g_4b9930 (and
   g_4b9934); fn_46bb0c then marks those taken as `taken1` and `taken2`.
   The first of g_4b9944 is `start`, kept inside the grid. */
/* @zoombi32 0x0046b872 */
void fn_46b872(char *grid, unsigned long stride, unsigned long rows, unsigned long columns,
               unsigned char from1, unsigned char from2, char to1, char to2, char taken1,
               char taken2, Point &start)
{
    unsigned long i;
    char *row;
    unsigned long y;
    char *cell;
    unsigned long x;

    for (i = 0; i < 24; i++) {
        g_4b99a4[i] = 0;
        g_4b9944[i].x = 0;
        g_4b9944[i].y = 0;
    }
    g_4b9930 = 0;
    row = grid;
    for (y = 0; y < rows; y++) {
        cell = row;
        for (x = 0; x < columns; x++) {
            char c = *cell;

            if (c == from1) {
                g_4b9930++;
                *cell = to1;
            }
            if (c == from2) {
                g_4b9930++;
                *cell = to2;
            }
            cell++;
        }
        row += stride;
    }
    g_4b99c0 = to1;
    g_4b99c2 = to2;
    g_4b99c1 = taken1;
    g_4b99c3 = taken2;
    g_4b99bc = grid;
    g_4b9938 = stride;
    g_4b993c = rows;
    g_4b9940 = columns;
    g_4b9934 = g_4b9930;
    if (1) {
        x = start.x;
        y = start.y;
    } else {
        x = 0;
        y = 0;
    }
    if (x > columns)
        x = columns - 1;
    if (y > rows)
        y = rows - 1;
    g_4b99a4[0] = 1;
    g_4b9944[0].x = x;
    g_4b9944[0].y = y;
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
