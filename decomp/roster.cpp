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

/* Shows frame n (up to rosterButtons[0].rect.top) of the view g_4ab9f8
   (script rosterButtons[0].rect.left on), if it's not running, with notify
   fn_41d30b. */
/* @zoombi32 0x0041dd37 */
void fn_41dd37(volatile short n)
{
    View *view = findView(g_4ab9f8);

    if (view && !view->body.running && n <= rosterButtons[0].rect.top) {
        setViewScript(view, rosterButtons[0].rect.left + n, 1);
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

/* A view's update: redraws button 2 when g_4a0fe8 changes, and button 1
   once. */
/* @zoombi32 0x0041d972 */
void fn_41d972(View *, short region)
{
    if (g_4a0fe8) {
        if (!g_4a120a) {
            g_4a120a = 1;
            unionRgnRect(region, &rosterButtons[2].rect);
        }
    } else if (g_4a120a) {
        g_4a120a = 0;
        unionRgnRect(region, &rosterButtons[2].rect);
    }
    if (!g_4a120c) {
        g_4a120c = 1;
        unionRgnRect(region, &rosterButtons[1].rect);
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

/* Draws button `which` (1: 5, 2: 2, or 1 if g_4a0fe8 isn't set; the next
   image if lit) from the bank g_4a1020, showing it if `show`. */
/* @zoombi32 0x0041d8bc */
void drawRosterButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4a0fe8) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a1020->offsets[image] + (char *)g_4a1020), rosterButtons[which].rect.left,
                      rosterButtons[which].rect.top, 8);
        if (show)
            showRect(&rosterButtons[which].rect);
    }
}

/* Draws feature n of kind `kind` (1-4: a table each; kind 1 five pixels
   further up and left) from the Zoombini images at `rect`, and shows it. */
/* @zoombi32 0x0041ed59 */
void fn_41ed59(short kind, short n, ShortRect rect)
{
    short handle;
    short image;
    ImageBank *bank;

    switch (kind) {
    case 1:
        rect.left += -5;
        rect.right += -5;
        image = g_4a12a6[n];
        break;
    case 2:
        image = g_4a129a[n];
        break;
    case 3:
        image = g_4a128e[n];
        break;
    case 4:
        image = g_4a1282[n];
        break;
    }
    handle = fn_46beac(snoidImagesResource);
    lockHandle(handle);
    bank = (ImageBank *)handleData(handle);
    drawImageData((unsigned short *)((char *)bank + bank->offsets[image]), rect.left, rect.top, 8);
    unlockHandle(handle);
    showRect(&rect);
}

/* Sends up to three of the placed Zoombinis (g_4ab8ec, from the 20th)
   that are ready off to (x, y), `interval` apart. */
/* @zoombi32 0x0041d80e */
void fn_41d80e(short x, short y, long interval)
{
    unsigned long when;
    short count;
    short i;
    View *view;

    count = 0;
    when = clockTime();
    for (i = 20; i > 0 && count < 3; i--) {
        view = findView(g_4ab8ec[i]);
        if (view) {
            view->flags = 1;
            Snoid *snoid = (Snoid *)&view->body;

            if (snoid->unknownF7) {
                *(Point *)&snoid->body.unknownAa = *(Point *)&snoid->body.x;
                snoid->targetX = x;
                snoid->targetY = y;
                setSnoidAction(snoid, 10, 0);
                view->nextUpdate = when;
                when += interval;
                count++;
            }
        }
    }
    sortViews();
    g_4b755a = 1;
    g_4b755c = 0;
}

/* The view drawing the two buttons. */
/* @zoombi32 0x0041d955 */
void drawRosterButtonsView(View *)
{
    drawRosterButton(1, 0, 0);
    drawRosterButton(2, 0, 0);
}

/* Closes the roster screen. */
/* @zoombi32 0x0041c9ed */
void closeRoster()
{
    if (g_4a0fec) {
        g_4a0fec = 0;
        short saved = fn_46bee9(1);

        clearViews();
        fn_41dccb();
        fn_41dce6();
        fn_46c602(&g_4a0fd0);
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4ab83c);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Counts the chosen Zoombinis (g_4a1014 of them) by feature g_4ab87a, and
   when there's a second (g_4ab878 above 2), by both it and g_4ab87c, into
   g_4ab892. */
/* @zoombi32 0x0041e273 */
void fn_41e273()
{
    short second;
    ChosenSnoids *chosen = listChosenSnoids();
    short i;
    short j;
    short first;

    if (g_4ab878 > 2)
        g_4a0ff4 = 2;
    else
        g_4a0ff4 = 1;
    for (i = 0; i < 6; i++)
        for (j = 0; j < 6; j++)
            g_4ab892[i][j] = 0;
    for (j = 0; j < g_4a1014; j++) {
        first = chosen->features[j][g_4ab87a];
        g_4ab892[0][first]++;
        if (g_4a0ff4 > 1) {
            second = chosen->features[j][g_4ab87c];
            g_4ab892[second][first]++;
        }
    }
}

/* The notify of the view g_4ab9f8's frames (fn_41dd37): 10 sets g_4ab872
   to 2; 20 notes the view done and, on its last frame, remarks (now and
   then) if not all are chosen, and sets g_4a0ff0; 21 likewise, ending the
   walk when g_4b754a. */
/* @zoombi32 0x0041d30b */
void fn_41d30b(View *, short event)
{
    switch (event) {
    case 10:
        g_4ab872 = 2;
        break;
    case 20:
        g_4ab874 = 0;
        if (rosterButtons[0].rect.right == rosterButtons[0].rect.top) {
            if (countChosenSnoids() < g_4a1014) {
                if (randomBetween(0, 4) > g_4ab878 - 1 || (*(short *)(g_4a4ba0 + 0x40) & 0xfff) <= 3)
                    queueViewSound(randomBetween(20045, 20048), 0);
            }
            g_4a0ff0 = 1;
        } else {
            g_4a0ff0 = 0;
        }
        break;
    case 21:
        g_4ab874 = 0;
        g_4a0ff0 = 1;
        if (g_4b754a) {
            g_4b755a = 0;
            g_4b755c = 1;
        }
        break;
    }
}

/* Draws image `image` (from the resource g_4a0fd4; big-endian offsets
   and sizes) at place `which` (1-10), centred across it and raised by
   g_4aba6c. */
/* @zoombi32 0x0041d9f2 */
void fn_41d9f2(short which, short image, long)
{
    short xs[11] = {0, 326, 348, 375, 397, 423, 324, 347, 373, 395, 422};
    short ys[11] = {0, 36, 39, 42, 44, 46, 77, 80, 83, 86, 90};
    short handle;
    ImageBank *bank;
    unsigned short *data;
    short x;
    short y;

    handle = fn_46beac(g_4a0fd4);
    lockHandle(handle);
    bank = (ImageBank *)handleData(handle);
    data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);
    x = xs[which] - swapShort(data[0]) / 2;
    y = ys[which] - g_4aba6c[which];
    drawImageData(data, x, y, 8);
    unlockHandle(handle);
}
