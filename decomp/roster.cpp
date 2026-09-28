/*
 * roster (0x41c09c-0x41f8cc): Saved players: 'ZBUser', 'Could not Open/Create Roster file.', 'Zoombini.who'
 */

#include <stdio.h>

#include "zoombinis.h"

/*
 * The original adds one with `sub eax, -1`; BCC32 turns every way of writing
 * it tried so far (+ 1, - -1, enums, consts, unsigned, compound assignment,
 * locals, other -O options, and Borland C++ 4.52 as well as 4.5) into `inc eax`.
 */
/* @zoombi32 0x0041d3e6 */
int fn_41d3e6(long, short value)
{
    return value + 1;
}

/* @zoombi32 0x0041d9e4 */
void fn_41d9e4(long)
{
}

/* @zoombi32 0x0041d9eb */
void fn_41d9eb(long, long)
{
}

/* Frees the resource g_4a0fd4, if loaded. */
/* @zoombi32 0x0041dccb */
void fn_41dccb()
{
    if (g_4a0fd4) {
        fn_46c602(&g_4a0fd4);
        g_4a0fd4 = 0;
    }
}

/* A view's update: redraws g_4a11ac when the view asks to (reset). */
/* @zoombi32 0x0041dbab */
void fn_41dbab(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &g_4a11ac);
    }
}

/* Puts view `id` in placed spot n (1-20), noting the spot's point in
   g_4ab8e0. */
/* @zoombi32 0x0041e8f3 */
void fn_41e8f3(short id, short n)
{
    if (n > 0 && n < 21) {
        placedViewPoint(&g_4ab8e0, n);
        claimPlacedView(n, id);
    }
}

/* A new player's file name: "ZOOM" and the next number (*counter, four
   digits). */
/* @zoombi32 0x0041f514 */
void fn_41f514(long, char *name, short *counter)
{
    name[0] = 'Z';
    name[1] = 'O';
    name[2] = 'O';
    name[3] = 'M';
    sprintf(name + 4, "%04d", (*counter)++);
    name[8] = 0;
}

/* Reports a roster error. */
/* @zoombi32 0x0041f195 */
void reportRosterError(const char *message)
{
    char none[48] = "";

    joinText(&rosterError, message, none);
    reportJoinedError(rosterError);
    freeText((void **)&rosterError);
}

/* Starts the view g_4ab874's Snoid on script `script` (by g_4ab8e4,
   unknownF8 `f8`), in group `group`, with notify `notify` if given. */
/* @zoombi32 0x0041d167 */
void fn_41d167(short group, short script, ViewNotify notify, char f8)
{
    View *view = findView(g_4ab874);

    if (view) {
        startSnoidScript((Snoid *)&view->body, script, g_4ab8e4, f8);
        view->body.group = group;
        if (notify)
            view->notify = notify;
    }
}

/* Shows frame n (up to g_4a1002) of the view g_4ab9f8 (script g_4a1000
   on), if it's not running, with notify fn_41d30b. */
/* @zoombi32 0x0041dd37 */
void fn_41dd37(volatile short n)
{
    View *view = findView(g_4ab9f8);

    if (view && !view->body.running && n <= g_4a1002) {
        setViewScript(view, g_4a1000 + n, 1);
        view->notify = fn_41d30b;
    }
}

/* Releases the two locked handles (g_4aba78) and their resources
   (g_4aba70). */
/* @zoombi32 0x0041dce6 */
void fn_41dce6()
{
    short i;

    for (i = 0; i < 2; i++)
        if (g_4aba78[i]) {
            unlockHandle(g_4aba78[i]);
            fn_46c602(&g_4aba70[i]);
            g_4aba78[i] = 0;
            g_4aba70[i] = 0;
        }
}

/* A view's update: redraws g_4a1048 when g_4a0fe8 changes, and g_4a1024
   once. */
/* @zoombi32 0x0041d972 */
void fn_41d972(View *, short region)
{
    if (g_4a0fe8) {
        if (!g_4a120a) {
            g_4a120a = 1;
            unionRgnRect(region, &g_4a1048);
        }
    } else if (g_4a120a) {
        g_4a120a = 0;
        unionRgnRect(region, &g_4a1048);
    }
    if (!g_4a120c) {
        g_4a120c = 1;
        unionRgnRect(region, &g_4a1024);
    }
}

/* Applies the player's settings from the game state: click time (stored
   big-endian), sound and music, the drag options, debugging messages,
   and the scenes. */
/* @zoombi32 0x0041f668 */
void applyPlayerSettings()
{
    clickTime = swapShort(*(unsigned short *)(g_4a4ba0 + 2));
    g_4b87fe = g_4a4ba0[4];
    g_4b87ff = g_4a4ba0[5];
    clickToDragOption = g_4a4ba0[6];
    hideDragCursor = g_4a4ba0[7];
    g_4b8803 = g_4a4ba0[8];
    dragClicks = g_4a4ba0[9];
    g_4b0d4a = *(short *)(g_4a4ba0 + 0xa);
    g_4b0d52 = *(short *)(g_4a4ba0 + 0xcc);
    g_4b0d56 = *(short *)(g_4a4ba0 + 0xca);
}

/* Opens the roster file `path` (mode `mode`) as g_4aba7c, making its
   directory first if need be: 0 if it opened, 1 if it did after making
   the directory, 2 if it didn't. */
/* @zoombi32 0x0041f100 */
short openRosterFile(const char *path, short mode)
{
    fileSpec spec(path);
    short result = 0;

    g_4aba7c = openFile(&spec, mode);
    if (!g_4aba7c) {
        createPath(spec, 0);
        g_4aba7c = openFile(&spec, mode);
        if (!g_4aba7c)
            result = 2;
        else
            result = 1;
    }
    return result;
}
