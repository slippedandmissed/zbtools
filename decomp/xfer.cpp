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

/* Fills `levels` (17) with the highest level (1-4) each place has
   reached, by the game state's bits (or all g_4b754a); then notes the
   place of scene g_4b0d54 in g_4b991c, and its level in g_4b991e. */
/* @zoombi32 0x0046b084 */
void fn_46b084(char *levels)
{
    short i;
    short bits;
    short value;
    short saved;

    for (i = 0; i <= 16; i++) {
        if (g_4b754a)
            value = g_4b754a;
        else {
            value = bits = 0;
            switch (i) {
            case 0:
                bits = 1;
                break;
            case 1:
            case 2:
            case 3:
                bits = g_4a4ba0[0x55 + i] & 0xf;
                break;
            case 4:
                bits = g_4a4ba0[0x50] & 0xf;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                bits = g_4a4ba0[0x54 + i] & 0xf;
                break;
            case 11:
                bits = *(short *)(g_4a4ba0 + 0x52) & 0xf;
                break;
            case 12:
            case 13:
            case 14:
                bits = g_4a4ba0[0x53 + i] & 0xf;
                break;
            case 15:
                bits = g_4a4ba0[0x51] & 0xf;
                break;
            case 16:
                bits = *(short *)(g_4a4ba0 + 0x52) & 0xf0;
                bits = bits >> 4;
                break;
            }
            if (bits & 1)
                value = 1;
            if (bits & 2)
                value = 2;
            if (bits & 4)
                value = 3;
            if (bits & 8)
                value = 4;
        }
        levels[i] = value;
    }
    g_4b991c = i = 0;
    saved = currentScene;
    currentScene = g_4b0d54;
    value = sceneLevel() + 1;
    currentScene = saved;
    switch (g_4b0d54) {
    case 7:
        i = 1;
        bits = value;
        break;
    case 8:
        i = 2;
        bits = levels[1];
        break;
    case 9:
        i = 3;
        bits = levels[2];
        break;
    case 4:
        i = 4;
        bits = levels[3];
        break;
    case 10:
        i = 5;
        bits = value;
        break;
    case 11:
        i = 6;
        bits = levels[5];
        break;
    case 12:
        i = 7;
        bits = levels[6];
        break;
    case 5:
        if (g_4b0d56 == 12) {
            i = 11;
            bits = levels[7];
        } else {
            i = 16;
            bits = levels[10];
        }
        break;
    case 13:
        i = 8;
        bits = value;
        break;
    case 14:
        i = 9;
        bits = levels[8];
        break;
    case 15:
        i = 10;
        bits = levels[9];
        break;
    case 16:
        i = 12;
        bits = value;
        break;
    case 17:
        i = 13;
        bits = levels[12];
        break;
    case 18:
        i = 14;
        bits = levels[13];
        break;
    case 6:
        i = 15;
        bits = levels[14];
        break;
    }
    if (i) {
        g_4b991c = i;
        g_4b991e = bits;
        levels[i] = bits - 1;
        if (levels[i] < 1) {
            if (g_4b98dc < 0)
                g_4b98dc = bits - 1;
            levels[i] = -1;
        }
    }
}

/* The map view's placed callback (map g_4b9916, 1-4): picks each cel's
   image by the places' levels (g_4b98e4): the first ones show the places
   reached, the rest each place's level. Notes the image of place g_4b991c
   in g_4b9928 (1-4). */
/* @zoombi32 0x0046b326 */
void fn_46b326(View *view)
{
    short images[10];
    short last;
    short base;
    short found = 0;
    short count;
    short i;
    short place;
    ViewCel *cel;

    switch (g_4b9916) {
    case 1:
        base = 0;
        count = 5;
        last = 8;
        break;
    case 2:
        base = 5;
        count = 6;
        last = 9;
        break;
    case 3:
        base = 10;
        count = 6;
        last = 9;
        break;
    case 4:
        base = 15;
        count = 6;
        last = 9;
        break;
    default:
        return;
    }
    images[0] = 0;
    for (i = 1; i <= last; i++) {
        images[i] = 0;
        if (i >= count) {
            place = g_4a7ee0[0][base + i - count + 1];
            if (g_4b991c && place == g_4b991c) {
                g_4b991c = 0;
                found = i;
                switch (g_4b9916) {
                case 1:
                    switch (found) {
                    case 5:
                        g_4b9928 = 1;
                        break;
                    case 6:
                        g_4b9928 = 2;
                        break;
                    case 7:
                        g_4b9928 = 3;
                        break;
                    case 8:
                        g_4b9928 = 4;
                        break;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                    switch (found) {
                    case 6:
                        g_4b9928 = 1;
                        break;
                    case 7:
                        g_4b9928 = 2;
                        break;
                    case 8:
                        g_4b9928 = 3;
                        break;
                    case 9:
                        g_4b9928 = 4;
                        break;
                    }
                    break;
                }
            }
            place = g_4b98e4[place];
            if (place > 0)
                images[i] = i + place * 4;
            else {
                if (g_4b98dc < 0)
                    g_4b98dc = 0;
                if (i == count)
                    images[i] = g_4b98dc * 4 + i;
                else if (i > count && place == -1 && images[i - 1] > last)
                    images[i] = g_4b98dc * 4 + i;
            }
        } else if (!base) {
            if (g_4b98e4[i])
                images[i] = i;
        } else {
            place = g_4a7ee0[0][base + i - 1];
            if (g_4b98e4[place])
                images[i] = i;
            else if (place == 11) {
                if (g_4b98e4[16])
                    images[i] = i;
            } else if (place == 16) {
                if (g_4b98e4[11])
                    images[i] = i;
            }
        }
    }
    cel = view->body.cels;
    while (cel->image) {
        if (found && found == cel->image)
            found = 0;
        if (images[cel->image]) {
            cel->image = images[cel->image];
            cel++;
        } else
            removeFirstCel(cel);
    }
}

/* A Zoombini's view's script events: 250-253 face it that way; 240-243
   note a way to face (g_4b9904, then 1-4) when it next turns round (0);
   26 faces it left and moves it after g_4b9906; 10-11 start the view of
   g_4b9900 for g_4b990c; 50 counts one more in town and starts
   g_4b9926's view. */
/* @zoombi32 0x0046b5ce */
void fn_46b5ce(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);
    View *started;

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
        g_4b9904 = event - 239;
        break;
    case 26:
        setSnoidFacing(snoid, 0);
        moveView(view->id, 0, g_4b9906);
        if (g_4b9908 >= 0)
            g_4b9908++;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b9904) {
            setSnoidFacing(snoid, g_4b9904 - 1);
            g_4b9904 = 0;
        }
        snoid->unknownF0++;
        if (!g_4b9916 && snoid->unknownF0 == 2)
            moveView(view->id, 1, g_4b9906);
        break;
    case 10:
    case 11:
        if (g_4b990c[event - 10]) {
            g_4b990c[event - 10] = 0;
            started = findView(g_4b9900[event - 10]);
            if (started) {
                started->flags = 0x188000;
                setViewScript(started, 0, 1);
            }
        }
        break;
    case 50:
        g_4b98da++;
        startView(g_4b9926, 0, 0, 0);
        break;
    case -1:
        break;
    }
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
