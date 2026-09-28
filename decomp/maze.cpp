/*
 * maze (0x433510-0x439560): the scene in Maze2.MHK
 */

#include "zoombinis.h"

/* @zoombi32 0x0043595f */
void fn_43595f(long)
{
}

/* @zoombi32 0x00435966 */
void fn_435966(long, long)
{
}

/*
 * Only an unsigned constant (or `-=` on a local) gives the original's
 * `sub eax, 50`; a signed one becomes `add eax, -50`. Perhaps a sizeof or an
 * unsigned #define.
 */
/* @zoombi32 0x0043691d */
int fn_43691d(long, short value)
{
    return value - 50u;
}

/* The index (1-20) of the largest value, ignoring `exclude`. */
/* @zoombi32 0x00437390 */
short indexOfLargestExcept(short exclude)
{
    short best, bestValue, i;
    for (i = 1, best = 0, bestValue = 0; i < 0x15; i++) {
        if (g_4aff9a[i] > bestValue && exclude != i) {
            bestValue = g_4aff9a[i];
            best = i;
        }
    }
    return best;
}

/* @zoombi32 0x00437acb */
short fn_437acb(short i)
{
    return g_4aff9a[i];
}

/* How many of g_4aff9a[1..20] are non-zero. */
/* @zoombi32 0x004381bb */
short fn_4381bb()
{
    short i, count;
    for (i = 1, count = 0; i < 0x15; i++)
        if (g_4aff9a[i])
            count++;
    return count;
}

/* Loads a 'REGS' table (swapping its words) and keeps it locked (as
   lilly's loadLockedTable). */
/* @zoombi32 0x0043462b */
void loadMazeTable(long *resource, short *handle, short id, short **locked)
{
    short *at;

    *resource = 0;
    fn_46c4fe(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    *handle = fn_46beac(*resource);
    *locked = (short *)lockHandle(*handle);
    at = (short *)handleData(*handle);
    for (unsigned long size = handleSize(*handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
}

/* Frees a table loaded by loadMazeTable. */
/* @zoombi32 0x004346ad */
void freeMazeTable(long *resource, short *handle)
{
    if (*handle) {
        unlockHandle(*handle);
        fn_46c602(resource);
        *handle = 0;
        *resource = 0;
    }
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   g_4afc6a), lit or not, and with `show` shows it. */
/* @zoombi32 0x004346dc */
void drawMazeButton(short which, short lit, short show)
{
    short image = 0;
    short handle;
    ImageBank *bank;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4afc6a) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        handle = fn_46beac(g_4a21b4);
        lockHandle(handle);
        bank = (ImageBank *)handleData(handle);
        unsigned short *data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);

        drawImageData(data, g_4a20f4[which].rect.left, g_4a20f4[which].rect.top, 8);
        unlockHandle(handle);
        if (show)
            showRect(&g_4a20f4[which].rect);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x004347d9 */
void drawMazeButtons(View *)
{
    drawMazeButton(1, 0, 0);
    drawMazeButton(2, 0, 0);
}

/* A view's update: adds buttons 2 (when g_4afc6a changes) and 1 (the
   first time) to the region to redraw. */
/* @zoombi32 0x004347f6 */
void updateMazeButtons(View *, short region)
{
    if (g_4afc6a) {
        if (!g_4a25c4) {
            g_4a25c4 = 1;
            unionRgnRect(region, &g_4a20f4[2].rect);
        }
    } else if (g_4a25c4) {
        g_4a25c4 = 0;
        unionRgnRect(region, &g_4a20f4[2].rect);
    }
    if (!g_4a25c6) {
        g_4a25c6 = 1;
        unionRgnRect(region, &g_4a20f4[1].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x00434868 */
void closeMaze()
{
    if (g_4afc68) {
        g_4afc68 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        freeMazeTable(&g_4afc18, &g_4afc20);
        fn_46c602(&g_4a21b4);
        fn_46c602(&g_4afbd8);
        fn_46c602(&g_4afbdc);
        fn_46c602(&g_4afbe0);
        fn_46c602(&g_4afbe4);
        fn_46c602(&g_4afbc4);
        fn_46c602(&g_4afbc8);
        fn_46c602(&g_4afbcc);
        fn_46bee9(saved);
        fn_46ca9c(&g_4afc64);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The scene's keys: 0x16f calls fn_466b93 (the only one; the check of
   g_4b8803 for other keys is left from the other scenes' cheat keys).
   Returns whether the key was used. */
/* @zoombi32 0x0043570e */
short mazeKey(unsigned short key)
{
    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        fn_466b93();
        return 1;
    }
    return 0;
}

/* Starts the script of the Zoombini paired with `view` (its body's word 50
   is the Zoombini's view): the entry of its own table (words 37 on) at its
   index (word 36), moving it into `group`. */
/* @zoombi32 0x0043583c */
void fn_43583c(View *view, short group, short, char unknownF8)
{
    short *parts = (short *)&view->body;
    View *other = findView(parts[50]);

    if (other) {
        short *its = (short *)&other->body;
        short script = its[37 + its[36]];

        startSnoidScript((Snoid *)&other->body, script, 0, unknownF8);
        other->body.group = group;
    }
}

/* The same, nudging the Zoombini by its pose (word 20) first and starting
   script 14000 on by its index. */
/* @zoombi32 0x00435882 */
void fn_435882(View *view, short group, short, char unknownF8)
{
    short *parts = (short *)&view->body;
    View *other = findView(parts[50]);

    if (other) {
        ViewBody *body = &other->body;

        parts = (short *)body;
        switch (parts[20]) {
        case 0:
            body->x += 3;
            body->y += 6;
            break;
        case 1:
            body->x += 7;
            body->y += 19;
            break;
        case 3:
            body->x += 12;
            body->y += 21;
            break;
        default:
            body->x += 3;
            body->y += 6;
            break;
        }
        short script = parts[36] + 14000;

        startSnoidScript((Snoid *)&other->body, script, 0, unknownF8);
        other->body.group = group;
    }
}

/* The same with script 14003, the paired view assumed to exist. */
/* @zoombi32 0x00435925 */
void fn_435925(View *view, short group, short, char unknownF8)
{
    short *parts = (short *)&view->body;
    View *other = findView(parts[50]);

    startSnoidScript((Snoid *)&other->body, 14003, 0, unknownF8);
    other->body.group = group;
}

/* A view's placing: its first cel by the hot spots in g_4afbe8/g_4afbec,
   35 below. */
/* @zoombi32 0x00436321 */
void fn_436321(View *view)
{
    short *cel = (short *)&view->body;

    cel[1] -= g_4afbe8[cel[0]];
    cel[2] += 35 - g_4afbec[cel[0]];
}

/* The same, 25 below. */
/* @zoombi32 0x00436356 */
void fn_436356(View *view)
{
    short *cel = (short *)&view->body;

    cel[1] -= g_4afbe8[cel[0]];
    cel[2] += 25 - g_4afbec[cel[0]];
}

/* The arrival hook (setArrivalHook): a Zoombini arriving in pose 1 or 3
   sets g_4b0a0a or g_4b0a0c. */
/* @zoombi32 0x00435f03 */
void fn_435f03(short id)
{
    View *view = findView(id);

    if (view) {
        short *parts = (short *)&view->body;

        switch (parts[35]) {
        case 1:
            g_4b0a0a = 1;
            break;
        case 3:
            g_4b0a0c = 1;
            break;
        }
    }
}

/* Lists (by index, in place) the entries of g_4aff9a that are empty. */
/* @zoombi32 0x0043824f */
void fn_43824f()
{
    short i;

    fillMemory(g_4affc4, 0, 42);
    for (i = 0; i < 21; i++)
        if (!g_4aff9a[i])
            g_4affc4[i] = i;
}

/* Packs the list made by fn_43824f into g_4b0018 (from 1), counting them
   in g_4b00c6 (g_4b00c8: any); returns the count. */
/* @zoombi32 0x00438280 */
short fn_438280()
{
    short i;

    g_4b00c8 = 0;
    fillMemory(g_4b0018, 0, 42);
    for (i = 0, g_4b00c6 = 0; i < 21; i++)
        if (g_4affc4[i]) {
            g_4b00c8 = 1;
            g_4b00c6++;
            g_4b0018[g_4b00c6] = g_4affc4[i];
        }
    return g_4b00c6;
}

/* The same as fn_43824f into g_4affee. */
/* @zoombi32 0x004382df */
void fn_4382df()
{
    short i;

    fillMemory(g_4affee, 0, 42);
    for (i = 0; i < 21; i++)
        if (!g_4aff9a[i])
            g_4affee[i] = i;
}

/* The same as fn_438280 from g_4affee into g_4b0042 (g_4b00ca, g_4b00cc). */
/* @zoombi32 0x00438310 */
short fn_438310()
{
    short i;

    g_4b00cc = 0;
    fillMemory(g_4b0042, 0, 42);
    for (i = 0, g_4b00ca = 0; i < 21; i++)
        if (g_4affee[i]) {
            g_4b00cc = 1;
            g_4b00ca++;
            g_4b0042[g_4b00ca] = g_4affee[i];
        }
    return g_4b00ca;
}

/* Lists all of 0-20 in g_4b0018 (count 20). */
/* @zoombi32 0x0043836f */
void fn_43836f()
{
    short i;

    for (i = 0; i < 21; i++)
        g_4b0018[i] = i;
    g_4b00c6 = 20;
    g_4b00c8 = 1;
}
