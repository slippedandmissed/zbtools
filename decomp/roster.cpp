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
