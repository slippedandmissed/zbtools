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
void fn_43583c(View *view, short group, ViewNotify, char unknownF8)
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
void fn_435882(View *view, short group, ViewNotify, char unknownF8)
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
void fn_435925(View *view, short group, ViewNotify, char unknownF8)
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

/* The index (1-19) of the largest value in g_4aff9a between `low` and
   `high` whose kind (g_4a263c) is `kind`'s. */
/* Not exact: BCC caches g_4aff9a's address in a register here, where the
   original keeps the parameters in registers instead (see findings.md on
   address caching). */
/* @zoombi32 0x004381da */
short fn_4381da(short kind, short low, short high)
{
    short i, best, value;

    for (i = 1, value = 0, best = 0; i < 20; i++)
        if (g_4a263c[i] == g_4a263c[kind] && g_4aff9a[i] >= low && g_4aff9a[i] <= high && value < g_4aff9a[i]) {
            value = g_4aff9a[i];
            best = i;
        }
    return best;
}

/* The index (1-20) of the smallest value in g_4aff9a from `least` on. */
/* @zoombi32 0x00437ade */
short fn_437ade(short least)
{
    short i, best, value;

    for (i = 0, best = 0, value = 21; i < 20; i++)
        if (value > g_4aff9a[i + 1] && least <= g_4aff9a[i + 1]) {
            value = g_4aff9a[i + 1];
            best = i + 1;
        }
    return best;
}

/* The index (1-20) of the largest value in g_4aff9a between `low` and
   `high`. */
/* Not exact: BCC caches g_4aff9a's address in a register here, where the
   original keeps the parameters in registers instead (see findings.md on
   address caching). */
/* @zoombi32 0x00437b23 */
short fn_437b23(short low, short high)
{
    short i, best, value;

    for (i = 1, best = 0, value = 0; i < 21; i++)
        if (low <= g_4aff9a[i] && high >= g_4aff9a[i] && value < g_4aff9a[i]) {
            value = g_4aff9a[i];
            best = i;
        }
    return best;
}

/* The index (1-20) of the smallest positive value in g_4aff9a, ignoring
   `exclude`. */
/* @zoombi32 0x004373cd */
short fn_4373cd(short exclude)
{
    short i, best, value;

    for (i = 1, best = 0, value = 20; i < 21; i++)
        if (value > g_4aff9a[i] && g_4aff9a[i] > 0 && exclude != i) {
            value = g_4aff9a[i];
            best = i;
        }
    return best;
}

/* The first entry set in the rows of g_4afe5a (g_4afc36 of them) other
   than in column `which`, plus that column's offset (g_4a2634); 0 if
   none. */
/* @zoombi32 0x00437331 */
short fn_437331(short which)
{
    short row, column;

    for (row = 0; row < g_4afc36; row++)
        for (column = 0; column < 4; column++)
            if (column != which && g_4afe5a[row][column])
                return g_4afe5a[row][column] + g_4a2634[column];
    return 0;
}

/* A view's notify: when its script ends (-1) with g_4b0d3a up to
   g_4b0d38, clears g_4b0d3c. */
/* @zoombi32 0x00436045 */
void fn_436045(View *, short event)
{
    switch (event) {
    case -1:
        if (g_4b0d3a >= g_4b0d38)
            g_4b0d3c = 0;
        break;
    }
}

/* A view's notify: event 10 sets g_4afc2e (151-156 do nothing). */
/* @zoombi32 0x0043606d */
void fn_43606d(View *, short event)
{
    switch (event) {
    case 10:
        g_4afc2e = 1;
        break;
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
        break;
    }
}

/* A view's drawing: its cels from the bank g_4afbc0, while it runs and
   stands in the game's area. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x0043692b */
void fn_43692b(View *view)
{
    if (view->body.running && ptInRect(&gameRect, *(Point *)&view->body.x)) {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = g_4afbc0;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
}

/* The scene's Zoombini views' update: lays the Zoombini out again
   (fn_43a7a6) unless it's in state 1. */
/* @zoombi32 0x00436994 */
void fn_436994(View *view, short region)
{
    Snoid *snoid;
    short changed = 0;

    if (!g_4b9684 && view->body.running && clockTime() >= view->nextUpdate) {
        view->nextUpdate = clockTime() + view->interval;
        snoid = (Snoid *)&view->body;
        switch (snoid->unknownF4) {
        case 1:
            break;
        default:
            changed = 1;
            break;
        }
        if (changed) {
            unionRgnRect(region, &view->body.bounds);
            fn_43a7a6(snoid);
            view->changed = 1;
        }
    }
}

/* Copies the chosen Zoombinis' features into the rows of g_4afe5a. */
/* Not exact: in the second loop the original computes the row's address
   before the column's index; BCC does it the other way round however the
   element is written. */
/* @zoombi32 0x00437089 */
void fn_437089()
{
    short row;
    short column;
    ChosenSnoids *chosen;

    for (row = 0; row < g_4afc36; row++)
        for (column = 0; column < 4; column++)
            g_4afe5a[row][column] = 0;
    chosen = listChosenSnoids();
    for (row = 0; row < g_4afc36; row++)
        for (column = 0; column < 4; column++)
            g_4afe5a[row][column] = chosen->features[row][column];
}

/* Copies into g_4afe5a only the chosen Zoombinis with a feature that is
   `id` (with g_4a2634's offsets); returns how many. */
/* @zoombi32 0x004370f8 */
short fn_4370f8(short id)
{
    short count;
    short row;
    short column;
    short found;
    ChosenSnoids *chosen;

    for (row = 0; row < g_4afc36; row++)
        for (column = 0; column < 4; column++)
            g_4afe5a[row][column] = 0;
    count = 0;
    chosen = listChosenSnoids();
    for (row = 0; row < g_4afc36; row++) {
        found = 0;
        for (column = 0; column < 4; column++)
            if (chosen->features[row][column] + g_4a2634[column] == id)
                found = 1;
        if (found) {
            for (column = 0; column < 4; column++)
                g_4afe5a[row][column] = chosen->features[row][column];
            count++;
        }
    }
    return count;
}

/* The first entry of column `which` in the rows of g_4afe5a that is set
   and isn't `ignore`, plus the column's offset (g_4a2634); 0 if none. */
/* @zoombi32 0x004372bf */
short fn_4372bf(short which, short ignore)
{
    short row, column;

    for (row = 0; row < g_4afc36; row++)
        for (column = 0; column < 4; column++)
            if (column == which && g_4afe5a[row][column] != ignore && g_4afe5a[row][column])
                return g_4afe5a[row][column] + g_4a2634[column];
    return 0;
}

/* A view's notify: 61 starts its paired Zoombini's script 14004 in its
   group (then told fn_435f3d); 63 lists the view in g_4b0908. */
/* @zoombi32 0x00435e8a */
void fn_435e8a(View *view, short event)
{
    switch (event) {
    case 61: {
        short *parts = (short *)&view->body;
        View *other = findView(parts[50]);

        if (other) {
            startSnoidScript((Snoid *)&other->body, 14004, 0, 1);
            other->body.group = view->body.group;
            other->notify = fn_435f3d;
        }
        break;
    }
    case 62:
        break;
    case 63:
        g_4b0908[g_4b09fc] = view->id;
        g_4b09fc++;
        break;
    }
}

/* Loads the hot-spot table for one of five sets (the first four by their
   entries in g_4a210e, recorded in g_4a210c). */
/* @zoombi32 0x00436a00 */
short *fn_436a00(short which)
{
    short id;

    switch (which) {
    case 0:
        id = g_4a210e[0] + 16600;
        g_4a210c = g_4a210e[0];
        break;
    case 1:
        id = g_4a210e[1] + 16602;
        g_4a210c = g_4a210e[1];
        break;
    case 2:
        id = g_4a210e[2] + 16604;
        g_4a210c = g_4a210e[2];
        break;
    case 3:
        id = g_4a210e[3] + 16606;
        g_4a210c = g_4a210e[3];
        break;
    case 4:
        id = 16609;
        g_4a210c = 0;
        break;
    }
    g_4afbdc = 0;
    g_4b076c = 0;
    return loadShortTable(id, &g_4afbdc);
}

/* A view's notify: 0 turns it round (the flags test is always true: `==`
   binds before `|`, as in the original), 61 starts its paired Zoombini's
   script 14004 (then told fn_435c57), 63 lists the view in g_4b0908. */
/* @zoombi32 0x00435b9e */
void fn_435b9e(View *view, short event)
{
    switch (event) {
    case 0:
        if (view->flags == 0x8000 | 0x4000001) {
            Snoid *snoid = (Snoid *)&view->body;

            snoid->unknownF2 = !snoid->unknownF2;
        }
        break;
    case 61: {
        short *parts = (short *)&view->body;
        View *other = findView(parts[50]);

        if (other) {
            startSnoidScript((Snoid *)&other->body, 14004, 0, 1);
            other->body.group = view->body.group;
            other->notify = fn_435c57;
        }
        break;
    }
    case 62:
        break;
    case 63:
        g_4b0908[g_4b09fc] = view->id;
        g_4b09fc++;
        break;
    }
}

/* A view's notify: 71 lists its paired view in g_4b0908 and clears its
   square (g_4b00d2, by its words 33 and 34) if the square is still its;
   120 starts its paired Zoombini's script 14007. */
/* @zoombi32 0x00435da5 */
void fn_435da5(View *view, short event)
{
    short *parts;
    View *other;

    switch (event) {
    case 71:
        parts = (short *)&view->body;
        other = findView(parts[50]);
        if (other) {
            g_4b0908[g_4b09fc] = other->id;
            g_4b09fc++;
        }
        if (g_4b00d2[parts[33]][parts[34]][1] == view->id) {
            g_4b00d2[parts[33]][parts[34]][0] = 0;
            g_4b00d2[parts[33]][parts[34]][1] = 0;
        }
        break;
    case 120:
        parts = (short *)&view->body;
        other = findView(parts[50]);
        if (other) {
            startSnoidScript((Snoid *)&other->body, 14007, 0, 0);
            other->notify = fn_435c57;
            other->body.group = view->body.group;
        }
        break;
    }
}

/* Starts view g_4afd2c[n]'s script (g_4a2308[n], then told fn_436092),
   with its second view's (g_4afd48[n], if g_4a22d0[n]), grouping them with
   its paired Zoombini's view. */
/* @zoombi32 0x0043573e */
void fn_43573e(short n)
{
    short *parts;
    View *view;
    View *second;
    View *other;

    second = 0;
    view = findView(g_4afd2c[n]);
    if (view) {
        setViewScript(view, g_4a2308[n], 1);
        view->notify = fn_436092;
        parts = (short *)&view->body;
        if (g_4a22d0[n]) {
            second = findView(g_4afd48[n]);
            if (second)
                setViewScript(second, g_4a2308[n] + 1, 1);
        }
        other = findView(parts[50]);
        if (other) {
            moveView(other->id, 1, view->id);
            if (second)
                groupViews(view->id, other->id, second->id, 0, 0, 0);
            else
                groupViews(view->id, other->id, 0, 0, 0, 0);
        } else {
            groupViews(view->id, view->id, 0, 0, 0, 0);
        }
    }
}

/* A view's notify: 92 deletes its two helper views (words 41 and 42),
   takes it out of g_4afc4a and clears its square; when its script ends
   (-1) it's listed in g_4b0958 (and g_4b0d3c cleared once g_4b0d3a
   reaches g_4b0d38). */
/* @zoombi32 0x00435f3d */
void fn_435f3d(View *view, short event)
{
    short *parts = (short *)&view->body;
    short i;

    switch (event) {
    case 92:
        deleteView(parts[41]);
        deleteView(parts[42]);
        for (i = 0; i < 11; i++)
            if (g_4afc4a[i] == view->id) {
                for (; g_4afc4a[i]; i++)
                    g_4afc4a[i] = g_4afc4a[i + 1];
                i = 11;
                g_4afc60--;
            }
        if (g_4b00d2[parts[33]][parts[34]][1] == view->id) {
            g_4b00d2[parts[33]][parts[34]][0] = 0;
            g_4b00d2[parts[33]][parts[34]][1] = 0;
        }
        break;
    case -1:
        g_4b0958[g_4b0a00] = view->id;
        g_4b0a00++;
        if (g_4b0d3a >= g_4b0d38)
            g_4b0d3c = 0;
        break;
    }
}

/* A view's notify: 91 lists a view in pose 3 in g_4b09d0, then (as 92)
   deletes its helper views, takes it out of g_4afc4a, clears its square
   and lists it in g_4b0958 unless in pose 3; -1 as fn_435f3d's. */
/* @zoombi32 0x00435c57 */
void fn_435c57(View *view, short event)
{
    short *parts = (short *)&view->body;
    short i;

    switch (event) {
    case 91:
        if (parts[35] == 3) {
            g_4b09d0[g_4b0a06] = view->id;
            g_4b0a06++;
        }
    case 92:
        deleteView(parts[41]);
        deleteView(parts[42]);
        for (i = 0; i < 11; i++)
            if (g_4afc4a[i] == view->id) {
                for (; g_4afc4a[i]; i++)
                    g_4afc4a[i] = g_4afc4a[i + 1];
                i = 11;
                g_4afc60--;
            }
        if (g_4b00d2[parts[33]][parts[34]][1] == view->id) {
            g_4b00d2[parts[33]][parts[34]][0] = 0;
            g_4b00d2[parts[33]][parts[34]][1] = 0;
        }
        if (parts[35] != 3) {
            g_4b0958[g_4b0a00] = view->id;
            g_4b0a00++;
        }
        break;
    case -1:
        g_4b0958[g_4b0a00] = view->id;
        g_4b0a00++;
        if (g_4b0d3a >= g_4b0d38)
            g_4b0d3c = 0;
        break;
    }
}

/* Clears the rows of g_4afe5a with a feature that is `id`, counts the
   features of the complete rows left in g_4aff9a (by value, with
   g_4a2634's offsets), and returns how many complete rows there are. */
/* @zoombi32 0x004371b3 */
short fn_4371b3(short id)
{
    short count;
    short keep;
    short i;
    short row;

    count = 0;
    for (i = 0; i < 21; i++)
        g_4aff9a[i] = 0;
    for (row = 0; row < g_4afc36; row++) {
        keep = 1;
        for (i = 0; i < 4; i++)
            if (g_4afe5a[row][i] && g_4afe5a[row][i] + g_4a2634[i] == id && id) {
                keep = 0;
                for (i = 0; i < 4; i++)
                    g_4afe5a[row][i] = 0;
                i = 4;
            }
        if (keep)
            for (i = 0; i < 4; i++)
                if (g_4afe5a[row][i]) {
                    g_4aff9a[g_4afe5a[row][i] + g_4a2634[i]]++;
                } else {
                    keep = 0;
                    i = 4;
                }
        if (keep)
            count++;
    }
    return count;
}

/* Puts a Zoombini in the maze in pose `pose` - 20 on its square (from
   g_4afbf0), with its helper view (word 41: script 10040) and a shadow
   view it adds (word 42: script 10041), grouped; the first to reach pose
   3 turns the go button on. */
/* @zoombi32 0x004350be */
void fn_4350be(View *view, short pose)
{
    Snoid *snoid = (Snoid *)&view->body;
    short *parts = (short *)&view->body;
    View *helper;
    View *shadow;
    Point where;

    if ((parts[35] = pose - 20) == 3) {
        snoid->unknownF7 = 1;
        if (++g_4b0d26 == 1) {
            g_4afc6a = 1;
            unionRgnRect(removedRgn, &g_4a20f4[2].rect);
        }
    }
    *(Point *)&view->body.x = (g_4afbf0 + parts[34])[parts[33] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, 10040, 1);
        *(Point *)&helper->body.x = *(Point *)&view->body.x;
        helper->placed = fn_436356;
        helper->notify = fn_435da5;
        where = *(Point *)&helper->body.x;
        parts[42] = addView(0x900000, drawCels, runViewScript, 10041, 7, &where, 0, 0);
        shadow = findView(parts[42]);
        if (shadow) {
            shadow->placed = fn_436321;
            runViewScript(shadow, removedRgn);
        }
    }
    short script = snoid->features[3] + 15075;

    startSnoidScript((Snoid *)&view->body, script, 0, 0);
    moveView(parts[41], 1, view->id);
    moveView(parts[42], 0, view->id);
    if (shadow)
        groupViews(parts[41], view->id, parts[42], 0, 0, 0);
    else if (helper)
        groupViews(parts[41], view->id, 0, 0, 0, 0);
}

/* The maze views' notify: 50 starts the view's second view (g_4afd26, by
   its word 45); 61, 62, 71, 72, 81 and 82 start its paired Zoombini's
   scripts; 65, 75 and 85 fn_43596d; 64, 74 and 84 list the Zoombini's view
   in g_4b08b8 (74 also forgets it); 66, 76 and 86 free its place. */
/* @zoombi32 0x00436092 */
void fn_436092(View *view, short event)
{
    short *parts;
    View *other;

    switch (event) {
    case 50:
        parts = (short *)&view->body;
        other = findView(g_4afd26[parts[45]]);
        if (other) {
            setViewScript(other, g_4a2324[parts[45]], 1);
            groupViews(other->id, other->id, 0, 0, 0, 0);
        }
        break;
    case 61:
        fn_43583c(view, view->body.group, fn_436092, 0);
        break;
    case 62:
        fn_435882(view, view->body.group, fn_436092, 0);
        break;
    case 64:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        break;
    case 65:
        fn_43596d(view, view->body.group, fn_436092, 1);
        break;
    case 66:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    case 71:
        fn_43583c(view, view->body.group, fn_436092, 0);
        break;
    case 72:
        fn_435882(view, view->body.group, fn_436092, 1);
        break;
    case 74:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        parts[50] = 0;
        break;
    case 75:
        fn_43596d(view, view->body.group, fn_436092, 0);
        break;
    case 76:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    case 81:
        fn_43583c(view, view->body.group, fn_436092, 0);
        break;
    case 82:
        fn_435882(view, view->body.group, fn_436092, 1);
        break;
    case 84:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        break;
    case 85:
        fn_43596d(view, view->body.group, fn_436092, 0);
        break;
    case 86:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    }
}

/* Moves the Zoombini paired with `view` onto its square (words 31 and
   32, g_4afbf0) by its pose (word 20), adds a helper view there for the
   pose (paired back with the Zoombini), and starts the Zoombini's script
   for the pose (then told fn_43638b), in `group`. */
/* @zoombi32 0x0043596d */
void fn_43596d(View *view, short group, ViewNotify, char unknownF8)
{
    short script;
    short helperScript;
    short pose;
    short column;
    Point where;
    short *parts;
    View *other;
    View *helper;

    parts = (short *)&view->body;
    other = findView(parts[50]);
    if (other) {
        Snoid *snoid = (Snoid *)&other->body;

        parts = (short *)snoid;
        column = parts[32];
        *(Point *)&snoid->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
        pose = parts[20];
        switch (pose) {
        case 0:
            snoid->body.x += 8;
            snoid->body.y += -33;
            helperScript = parts[20] + 10000;
            script = parts[21 + parts[20]];
            break;
        case 1:
            snoid->body.x += 4;
            snoid->body.y += -38;
            helperScript = 10042;
            script = snoid->features[3] + 15080;
            break;
        case 3:
            snoid->body.x += 4;
            snoid->body.y += -33;
            helperScript = 10043;
            script = snoid->features[3] + 15085;
            break;
        default:
            snoid->body.x += 4;
            snoid->body.y += -38;
            helperScript = parts[20] + 10042;
            script = snoid->features[3] + 15085;
            break;
        }
        where = *(Point *)&snoid->body.x;
        parts[41] = addView(0x4908000, drawCels, runViewScript, helperScript, 7, &where, 0, 0);
        helper = findView(parts[41]);
        if (helper) {
            setViewScript(helper, helperScript, 1);
            unionRgnRect(removedRgn, &helper->body.bounds);
            helper->placed = fn_436321;
            helper->body.group = group;
            parts = (short *)&helper->body;
            parts[50] = other->id;
            runViewScript(helper, removedRgn);
        }
        startSnoidScript((Snoid *)&other->body, script, 0, unknownF8);
        other->notify = fn_43638b;
        other->body.group = group;
        switch (pose) {
        case 0:
            moveView(other->id, 0, g_4afd8c[column]);
            moveView(helper->id, 1, other->id);
            break;
        case 1:
        case 3:
            moveView(other->id, 0, g_4afd8c[column]);
            moveView(helper->id, 1, other->id);
            break;
        }
    }
}

/*
 * Picks the value (1-20, not `exclude`) with the fewest (non-zero) counts
 * in g_4aff9a for which some row of g_4afe5a has that feature in a column
 * where no row already taken (g_4b0770) has the same one; takes that row
 * (into g_4b0770, up to 20), clears the rows with the value, recounts
 * g_4aff9a, and returns the value (0: none).
 */
/* Not exact: the original tests the outer loop's condition before its
   first pass (BCC drops that test for a constant start however the loop is
   written), and copies `v` to `best` through dx rather than ax. */
/* @zoombi32 0x0043780d */
short fn_43780d(short exclude)
{
    short v;
    short best;
    short least;
    short fresh;
    short bestRow;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, bestRow = -1, least = 21; v < 21; v++) {
        if (g_4aff9a[v] > 0 && g_4aff9a[v] <= least && exclude != v)
            for (row = 0; row < g_4afc36; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (g_4afe5a[row][column] > 0 && g_4afe5a[row][column] + g_4a2634[column] == v) {
                        for (k = 0; k < 20 && g_4b00d0 < 4; k++)
                            if (g_4b0770[k][column] > 0 && g_4b0770[k][column] == g_4afe5a[row][column])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            bestRow = row;
                            least = g_4aff9a[v];
                            row = g_4afc36;
                        }
                        column = 4;
                    }
                }
    }
    if (best) {
        fillMemory(g_4aff9a, 0, 42);
        if (g_4b00d0 < 20) {
            g_4b0770[g_4b00d0][0] = g_4afe5a[bestRow][0];
            g_4b0770[g_4b00d0][1] = g_4afe5a[bestRow][1];
            g_4b0770[g_4b00d0][2] = g_4afe5a[bestRow][2];
            g_4b0770[g_4b00d0][3] = g_4afe5a[bestRow][3];
            g_4b00d0++;
        }
        for (row = 0; row < g_4afc36; row++) {
            for (column = 0; column < 4; column++)
                if (g_4afe5a[row][column] && g_4afe5a[row][column] + g_4a2634[column] == best) {
                    column = 4;
                    for (v = 0; v < 4; v++)
                        g_4afe5a[row][v] = 0;
                }
            if (g_4afe5a[row][0] > 0)
                for (column = 0; column < 4; column++)
                    g_4aff9a[g_4afe5a[row][column] + g_4a2634[column]]++;
        }
    }
    return best;
}

/*
 * Picks the value (1-20) with the most counts in g_4aff9a, between `low`
 * and `high`, for which some row of g_4afe5a has that feature in a column
 * where no row already taken (g_4b0770) has the same one; takes the rows
 * with the value (into g_4b0770 and the copy g_4b0810), clears them,
 * recounts g_4aff9a, and returns the value (0: none).
 */
/* Not exact: the original copies `v` to `best` through dx; this uses ax
   (as in fn_43780d). */
/* @zoombi32 0x00437b7b */
short fn_437b7b(short low, short high)
{
    short v;
    short best;
    short most;
    short fresh;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, most = 0; v < 21; v++)
        if (g_4aff9a[v] > most && g_4aff9a[v] >= low && g_4aff9a[v] <= high)
            for (row = 0; row < g_4afc36; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (g_4afe5a[row][column] > 0 && g_4afe5a[row][column] + g_4a2634[column] == v) {
                        for (k = 0; k < 20; k++)
                            if (g_4b0770[k][column] > 0 && g_4b0770[k][column] == g_4afe5a[row][column])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            most = g_4aff9a[v];
                            row = g_4afc36;
                        }
                        column = 4;
                    }
                }
    if (best) {
        fillMemory(g_4aff9a, 0, 42);
        for (row = 0; row < g_4afc36; row++) {
            for (column = 0; column < 4; column++)
                if (g_4afe5a[row][column] && g_4afe5a[row][column] + g_4a2634[column] == best) {
                    if (g_4b00d0 < 20) {
                        g_4b0810[g_4b00d0][0] = g_4afe5a[row][0];
                        g_4b0810[g_4b00d0][1] = g_4afe5a[row][1];
                        g_4b0810[g_4b00d0][2] = g_4afe5a[row][2];
                        g_4b0810[g_4b00d0][3] = g_4afe5a[row][3];
                        g_4b0770[g_4b00d0][0] = g_4afe5a[row][0];
                        g_4b0770[g_4b00d0][1] = g_4afe5a[row][1];
                        g_4b0770[g_4b00d0][2] = g_4afe5a[row][2];
                        g_4b0770[g_4b00d0][3] = g_4afe5a[row][3];
                        g_4b00d0++;
                    }
                    g_4afe5a[row][0] = 0;
                    g_4afe5a[row][1] = 0;
                    g_4afe5a[row][2] = 0;
                    g_4afe5a[row][3] = 0;
                    column = 4;
                }
            if (g_4afe5a[row][0] > 0)
                for (column = 0; column < 4; column++)
                    g_4aff9a[g_4afe5a[row][column] + g_4a2634[column]]++;
        }
    }
    return best;
}

/* As fn_437b7b, checking a Zoombini's features against the rows already
   taken only in the copy (g_4b0810), and taking the rows into the
   copy alone. */
/* Not exact: BCC caches g_4afe5a's address in esi, where the original
   keeps `column` there (see findings.md on address caching). */
/* @zoombi32 0x00437ea2 */
short fn_437ea2(short low, short high)
{
    short (*copies)[4] = g_4b0810;
    short v;
    short best;
    short most;
    short fresh;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, most = 0; v < 21; v++)
        if (g_4aff9a[v] > most && g_4aff9a[v] >= low && g_4aff9a[v] <= high)
            for (row = 0; row < g_4afc36; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (g_4afe5a[row][column] > 0 && g_4afe5a[row][column] + g_4a2634[column] == v) {
                        for (k = 0; k < 20; k++)
                            if (copies[k][0] > 0 && copies[k][0] == g_4afe5a[row][0])
                                fresh = 0;
                            else if (copies[k][1] > 0 && copies[k][1] == g_4afe5a[row][1])
                                fresh = 0;
                            else if (copies[k][2] > 0 && copies[k][2] == g_4afe5a[row][2])
                                fresh = 0;
                            else if (copies[k][3] > 0 && copies[k][3] == g_4afe5a[row][3])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            most = g_4aff9a[v];
                            row = g_4afc36;
                        }
                        column = 4;
                    }
                }
    if (best) {
        fillMemory(g_4aff9a, 0, 42);
        for (row = 0; row < g_4afc36; row++) {
            for (column = 0; column < 4; column++)
                if (g_4afe5a[row][column] && g_4afe5a[row][column] + g_4a2634[column] == best) {
                    if (g_4b00d0 < 20) {
                        copies[g_4b00d0][0] = g_4afe5a[row][0];
                        copies[g_4b00d0][1] = g_4afe5a[row][1];
                        copies[g_4b00d0][2] = g_4afe5a[row][2];
                        copies[g_4b00d0][3] = g_4afe5a[row][3];
                        g_4b00d0++;
                    }
                    g_4afe5a[row][0] = 0;
                    g_4afe5a[row][1] = 0;
                    g_4afe5a[row][2] = 0;
                    g_4afe5a[row][3] = 0;
                    column = 4;
                }
            if (g_4afe5a[row][0] > 0)
                for (column = 0; column < 4; column++)
                    g_4aff9a[g_4afe5a[row][column] + g_4a2634[column]]++;
        }
    }
    return best;
}

/* Adds a view for a Zoombini in the maze (drawn by fn_43692b, updated by
   fn_436994) from `snoid`: gives it the next ten words of g_4b076c (its
   kind, square, line...), records it on its square (g_4b04c8, g_4b061a)
   and in its line's list, and lays it out. */
/* @zoombi32 0x00436d39 */
void fn_436d39(Snoid *snoid)
{
    View *view;
    short id;
    Snoid *made;
    short *parts;
    short i;

    id = addView(1, fn_43692b, fn_436994, 0, randomBetween(20, 25), snoid, 0, 0);
    if (g_4afc44)
        moveView(id, 1, g_4afc44);
    else if (g_4afc2a)
        moveView(id, 1, g_4afc2a);
    if ((view = findView(id)) != 0) {
        made = (Snoid *)&view->body;
        parts = (short *)made;
        for (i = 0; i < 10; i++)
            parts[30 + i] = (g_4b076c + i)[g_4b08b4 * 10];
        g_4b08b4++;
        g_4b04c8[parts[31]][parts[32]] = view->id;
        g_4b061a[parts[31]][parts[32]] = parts[30];
        *(Point *)&made->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
        switch (parts[33]) {
        case 1:
            g_4b0a10[g_4b0cfe[0]] = id;
            g_4b0cfe[0]++;
            parts[40] = g_4b0d12[0];
            break;
        case 2:
            g_4b0b6e[g_4b0cfe[1]] = id;
            g_4b0cfe[1]++;
            parts[40] = g_4b0d12[1];
            break;
        case 3:
            g_4b0ba0[g_4b0cfe[2]] = id;
            g_4b0cfe[2]++;
            parts[40] = g_4b0d12[2];
            break;
        case 4:
            g_4b0bd2[g_4b0cfe[3]] = id;
            g_4b0cfe[3]++;
            parts[40] = g_4b0d12[3];
            break;
        case 5:
            g_4b0c04[g_4b0cfe[4]] = id;
            g_4b0cfe[4]++;
            parts[40] = g_4b0d12[4];
            break;
        case 6:
            g_4b0c36[g_4b0cfe[5]] = id;
            g_4b0cfe[5]++;
            parts[40] = g_4b0d12[5];
            break;
        case 7:
            g_4b0c68[g_4b0cfe[6]] = id;
            g_4b0cfe[6]++;
            parts[40] = g_4b0d12[6];
            break;
        case 8:
            g_4b0c9a[g_4b0cfe[7]] = id;
            g_4b0cfe[7]++;
            parts[40] = g_4b0d12[7];
            break;
        default:
            g_4b0a10[g_4b0cfe[0]] = id;
            g_4b0cfe[0]++;
            parts[40] = g_4b0d12[0];
            break;
        }
        switch (parts[30]) {
        case 1:
        case 5:
            made->unknownF4 = 2;
            break;
        case 2:
            parts[41] = g_4a25e0[g_4b0096[g_4b00c2]][0] + 1;
            parts[42] = g_4a25e0[g_4b0096[g_4b00c2]][1];
            g_4b00c2++;
            made->unknownF4 = 3;
            break;
        default:
            parts[41] = 0;
            parts[42] = 0;
            made->unknownF4 = 3;
            break;
        }
        parts[43] = 0;
        fn_43a7a6(made);
        view->flags = 0x4188000;
        view->nextUpdate = 0;
    }
}

/*
 * As fn_43780d (up to three rows taken, not `exclude`), remembering the
 * column too: with `whole` 0 it takes just that feature (and the whole row
 * into the copy), else the whole row into both; then clears the row and
 * recounts g_4aff9a. Returns the value (0: none).
 */
/* @zoombi32 0x00437416 */
short fn_437416(short exclude, short whole)
{
    short v;
    short best;
    short least;
    short fresh;
    short bestRow;
    short bestColumn;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, bestRow = -1, bestColumn = 0, least = 21; v < 21; v++)
        if (g_4aff9a[v] > 0 && g_4aff9a[v] <= least && exclude != v)
            for (row = 0; row < g_4afc36; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (g_4afe5a[row][column] > 0 && g_4afe5a[row][column] + g_4a2634[column] == v) {
                        for (k = 0; k < 20 && g_4b00d0 < 3; k++)
                            if (g_4b0770[k][0] > 0 && g_4b0770[k][0] == g_4afe5a[row][0])
                                fresh = 0;
                            else if (g_4b0770[k][1] > 0 && g_4b0770[k][1] == g_4afe5a[row][1])
                                fresh = 0;
                            else if (g_4b0770[k][2] > 0 && g_4b0770[k][2] == g_4afe5a[row][2])
                                fresh = 0;
                            else if (g_4b0770[k][3] > 0 && g_4b0770[k][3] == g_4afe5a[row][3])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            bestRow = row;
                            bestColumn = column;
                            least = g_4aff9a[v];
                            row = g_4afc36;
                        }
                        column = 4;
                    }
                }
    if (best) {
        if (g_4b00d0 < 4) {
            if (!whole) {
                g_4b0770[g_4b00d0][bestColumn] = g_4afe5a[bestRow][bestColumn];
                g_4b0810[g_4b00d0][0] = g_4afe5a[bestRow][0];
                g_4b0810[g_4b00d0][1] = g_4afe5a[bestRow][1];
                g_4b0810[g_4b00d0][2] = g_4afe5a[bestRow][2];
                g_4b0810[g_4b00d0][3] = g_4afe5a[bestRow][3];
                g_4b00d0++;
            } else {
                g_4b0810[g_4b00d0][0] = g_4afe5a[bestRow][0];
                g_4b0810[g_4b00d0][1] = g_4afe5a[bestRow][1];
                g_4b0810[g_4b00d0][2] = g_4afe5a[bestRow][2];
                g_4b0810[g_4b00d0][3] = g_4afe5a[bestRow][3];
                g_4b0770[g_4b00d0][0] = g_4afe5a[bestRow][0];
                g_4b0770[g_4b00d0][1] = g_4afe5a[bestRow][1];
                g_4b0770[g_4b00d0][2] = g_4afe5a[bestRow][2];
                g_4b0770[g_4b00d0][3] = g_4afe5a[bestRow][3];
                g_4b00d0++;
            }
        }
        g_4afe5a[bestRow][0] = 0;
        g_4afe5a[bestRow][1] = 0;
        g_4afe5a[bestRow][2] = 0;
        g_4afe5a[bestRow][3] = 0;
        fillMemory(g_4aff9a, 0, 42);
        for (row = 0; row < g_4afc36; row++)
            if (g_4afe5a[row][0])
                for (column = 0; column < 4; column++)
                    g_4aff9a[g_4afe5a[row][column] + g_4a2634[column]]++;
    }
    return best;
}

/* The maze Zoombinis' notify: at frame 3 of events 20, 30, 40 and 50 it
   counts the Zoombini onto its square (g_4b00d2): the first is recorded,
   a second meeting it lists both in g_4b0980 and clears the square (as
   more do); 21, 31, 41, 51 and 61 list it in g_4b08e0. */
/* @zoombi32 0x0043638b */
void fn_43638b(View *view, short event)
{
    short *parts;
    short count;

    switch (event) {
    case 20:
        if (view->body.frame == 3) {
            parts = (short *)&view->body;
            count = ++g_4b00d2[parts[33]][parts[34]][0];
            if (count == 1) {
                g_4b00d2[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = g_4b00d2[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            } else if (count > 2) {
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
        }
        break;
    case 21:
        g_4b08e0[g_4b09f8] = view->id;
        g_4b09f8++;
        break;
    case 30:
        if (view->body.frame == 3) {
            parts = (short *)&view->body;
            count = ++g_4b00d2[parts[33]][parts[34]][0];
            if (count == 1) {
                g_4b00d2[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = g_4b00d2[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
        }
        break;
    case 31:
        g_4b08e0[g_4b09f8] = view->id;
        g_4b09f8++;
        break;
    case 40:
        if (view->body.frame == 3) {
            parts = (short *)&view->body;
            count = ++g_4b00d2[parts[33]][parts[34]][0];
            if (count == 1) {
                g_4b00d2[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = g_4b00d2[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
        }
        break;
    case 41:
        g_4b08e0[g_4b09f8] = view->id;
        g_4b09f8++;
        break;
    case 50:
        if (view->body.frame == 3) {
            parts = (short *)&view->body;
            count = ++g_4b00d2[parts[33]][parts[34]][0];
            if (count == 1) {
                g_4b00d2[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = g_4b00d2[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                g_4b00d2[parts[33]][parts[34]][1] = 0;
                g_4b00d2[parts[33]][parts[34]][0] = 0;
            }
        }
        break;
    case 51:
        g_4b08e0[g_4b09f8] = view->id;
        g_4b09f8++;
        break;
    case 61:
        g_4b08e0[g_4b09f8] = view->id;
        g_4b09f8++;
        break;
    }
}

/* Adds `count` Zoombini views to the maze (fn_436d39), from a blank
   Zoombini. */
/* @zoombi32 0x00436c71 */
void fn_436c71(short count)
{
    Snoid snoid;
    Snoid *made = &snoid;
    short i;

    for (i = 0; i < count; i++) {
        made->body.x = 0;
        made->body.y = 0;
        made->unknownC2[0] = 0;
        made->unknownC2[1] = 0;
        made->unknownC2[2] = 0;
        made->unknownF2 = 0;
        made->name[0] = 0;
        made->home = *(Point *)&made->body.x;
        *(Point *)&made->body.unknownAa = *(Point *)&made->body.x;
        *(Point *)&made->targetX = *(Point *)&made->body.x;
        made->unknownEa = 0;
        made->unknownEb = 0;
        made->unknownEc = 0;
        made->unknownEe = 0;
        made->unknownF0 = 0;
        made->unknownF7 = 0;
        fn_436d39(made);
    }
}

/* One way of choosing the maze's sequence of values (g_4b0096, g_4b00be
   of them) from the chosen Zoombinis' features. */
/* @zoombi32 0x00438396 */
void fn_438396()
{
    short value;
    short rows;
    short other;

    g_4b00be = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    fn_437089();
    rows = fn_4371b3(0);
    fn_43824f();
    if (!fn_438280())
        fn_43836f();
    if (rows >= 3) {
        g_4b0096[g_4b00be] = fn_437b23(2, 5);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = fn_437b23(6, 9);
            if (!g_4b0096[g_4b00be]) {
                g_4b0096[g_4b00be] = fn_437b23(10, 16);
                if (!g_4b0096[g_4b00be])
                    g_4b0096[g_4b00be] = fn_437b23(1, 16);
            }
        }
    } else {
        g_4b0096[g_4b00be] = fn_437b23(1, 2);
    }
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
    g_4b00be++;
    if (g_4a210c == 2) {
        g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
        g_4b00be++;
    }
    value = fn_4371b3(g_4b0096[g_4b00be]);
    g_4b00be++;
    if (fn_4381bb() > 4) {
        g_4b0096[g_4b00be] = fn_437b23(1, g_4a2666[value]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            rows = 0;
        } else {
            rows = fn_437acb(g_4b0096[g_4b00be]);
        }
        fn_4371b3(g_4b0096[g_4b00be]);
        g_4b00be++;
        g_4b0096[g_4b00be] = fn_4381da(g_4b0096[g_4b00be - 1], 1, g_4a2666[value]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            other = 0;
        } else {
            other = fn_437acb(g_4b0096[g_4b00be]);
        }
    } else {
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        rows = 0;
        g_4b00be++;
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        other = 0;
    }
    if (rows > other) {
        rows = g_4b0096[g_4b00be - 1];
        g_4b0096[g_4b00be - 1] = g_4b0096[g_4b00be];
        g_4b0096[g_4b00be] = rows;
    }
    g_4b00be++;
}
/* Another way of choosing the maze's sequence of values (the first one
   three times; the largest count when few are left). */
/* @zoombi32 0x00438626 */
void fn_438626()
{
    short value;
    short rows;
    short other;

    g_4b00be = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    fn_437089();
    rows = fn_4371b3(0);
    fn_43824f();
    if (!fn_438280())
        fn_43836f();
    if (rows >= 3) {
        g_4b0096[g_4b00be] = fn_437b23(2, 5);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = fn_437b23(6, 9);
            if (!g_4b0096[g_4b00be]) {
                g_4b0096[g_4b00be] = fn_437b23(10, 16);
                if (!g_4b0096[g_4b00be])
                    g_4b0096[g_4b00be] = fn_437b23(1, 16);
            }
        }
    } else {
        g_4b0096[g_4b00be] = fn_437b23(1, 2);
    }
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
    g_4b00be++;
    g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
    g_4b00be++;
    value = fn_4371b3(g_4b0096[g_4b00be]);
    g_4b00be++;
    if (fn_4381bb() > 4) {
        g_4b0096[g_4b00be] = fn_437b23(1, g_4a2666[value]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            rows = 0;
        } else {
            rows = fn_437acb(g_4b0096[g_4b00be]);
        }
        fn_4371b3(g_4b0096[g_4b00be]);
        g_4b00be++;
        g_4b0096[g_4b00be] = fn_4381da(g_4b0096[g_4b00be - 1], 1, g_4a2666[value]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            other = 0;
        } else {
            other = fn_437acb(g_4b0096[g_4b00be]);
        }
    } else {
        g_4b0096[g_4b00be] = indexOfLargestExcept(0);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            rows = 0;
        } else {
            rows = fn_437acb(g_4b0096[g_4b00be]);
        }
        g_4b00be++;
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        other = 0;
    }
    if (rows > other) {
        rows = g_4b0096[g_4b00be - 1];
        g_4b0096[g_4b00be - 1] = g_4b0096[g_4b00be];
        g_4b0096[g_4b00be] = rows;
    }
    g_4b00be++;
}

/* A third way of choosing the maze's sequence: the first value twice,
   then values with the rarest features (fn_43780d). */
/* @zoombi32 0x004388d8 */
void fn_4388d8()
{
    short most;
    short rows;
    short other;

    g_4b00be = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    fn_437089();
    rows = fn_4371b3(0);
    fn_43824f();
    if (!fn_438280())
        fn_43836f();
    if (rows >= 3) {
        g_4b0096[g_4b00be] = fn_437b23(2, 5);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = fn_437b23(6, 9);
            if (!g_4b0096[g_4b00be]) {
                g_4b0096[g_4b00be] = fn_437b23(10, 16);
                if (!g_4b0096[g_4b00be])
                    g_4b0096[g_4b00be] = fn_437b23(1, 16);
            }
        }
    } else {
        g_4b0096[g_4b00be] = fn_437b23(1, 2);
    }
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
    fn_4370f8(g_4b0096[g_4b00be]);
    g_4b00be = 2;
    fn_4371b3(0);
    fn_4382df();
    fn_438310();
    g_4b0096[g_4b00be] = fn_43780d(g_4b0096[0]);
    if (!g_4b0096[g_4b00be]) {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    }
    g_4b00be++;
    if (g_4afc32 == 1) {
        g_4b0096[g_4b00be] = g_4b0096[g_4b00be - 1];
        g_4b00be++;
    }
    g_4b0096[g_4b00be] = fn_43780d(g_4b0096[0]);
    if (!g_4b0096[g_4b00be]) {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    }
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_43780d(g_4b0096[0]);
    if (!g_4b0096[g_4b00be]) {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    }
    fn_437089();
    rows = fn_4371b3(g_4b0096[0]);
    g_4b00be++;
    if (fn_4381bb() > 4) {
        g_4b0096[g_4b00be] = fn_437b23(1, g_4a2666[rows]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            most = 0;
        } else {
            most = fn_437acb(g_4b0096[g_4b00be]);
        }
        fn_4371b3(g_4b0096[g_4b00be]);
        g_4b00be++;
        g_4b0096[g_4b00be] = fn_4381da(g_4b0096[g_4b00be - 1], 1, g_4a2666[rows]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            other = 0;
        } else {
            other = fn_437acb(g_4b0096[g_4b00be]);
        }
        if (other < most) {
            rows = g_4b0096[g_4b00be - 1];
            g_4b0096[g_4b00be - 1] = g_4b0096[g_4b00be];
            g_4b0096[g_4b00be] = rows;
        }
        g_4b00be++;
    } else {
        if (g_4a210c == 1) {
            g_4b0096[g_4b00be] = indexOfLargestExcept(0);
            if (!g_4b0096[g_4b00be])
                g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            g_4b00be++;
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        } else {
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            g_4b00be++;
            g_4b0096[g_4b00be] = indexOfLargestExcept(0);
            if (!g_4b0096[g_4b00be])
                g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        }
        g_4b00be++;
    }
}

/* A fourth way of choosing the maze's sequence: values with the commonest
   features (fn_437b7b), then the rarest (fn_43780d) twice over. */
/* @zoombi32 0x00438d67 */
void fn_438d67()
{
    short most;
    short first;
    short rows;
    short other;

    g_4b00be = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    fn_437089();
    rows = fn_4371b3(0);
    fn_43824f();
    if (!fn_438280())
        fn_43836f();
    first = fn_43780d(g_4b0096[0]);
    if (rows >= 2) {
        g_4b0096[g_4b00be] = fn_437b7b(2, 4);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = fn_437b7b(5, 8);
            if (!g_4b0096[g_4b00be]) {
                g_4b0096[g_4b00be] = fn_437b7b(9, 12);
                if (!g_4b0096[g_4b00be])
                    g_4b0096[g_4b00be] = fn_437b7b(1, 16);
            }
        }
    } else {
        g_4b0096[g_4b00be] = fn_437b7b(1, 1);
    }
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b0096[g_4b00be + 1] = g_4b0096[g_4b00be];
    fn_4370f8(g_4b0096[g_4b00be]);
    g_4b00be = 2;
    fn_4371b3(0);
    fn_4382df();
    fn_438310();
    g_4b0096[g_4b00be] = first;
    g_4b00be++;
    g_4b0096[g_4b00be] = first;
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_43780d(g_4b0096[0]);
    if (!g_4b0096[g_4b00be]) {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    }
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_43780d(g_4b0096[0]);
    if (!g_4b0096[g_4b00be]) {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    }
    g_4b00be++;
    fn_437089();
    fn_4371b3(g_4b0096[0]);
    rows = fn_4371b3(first);
    fn_4382df();
    fn_438310();
    if (fn_4381bb() > 4) {
        g_4b0096[g_4b00be] = fn_437b23(1, g_4a2666[rows]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
            if (!g_4b0096[g_4b00be])
                g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            most = 0;
        } else {
            most = fn_437acb(g_4b0096[g_4b00be]);
        }
        fn_4371b3(g_4b0096[g_4b00be]);
        g_4b00be++;
        g_4b0096[g_4b00be] = fn_4381da(g_4b0096[g_4b00be - 1], 1, g_4a2666[rows]);
        if (!g_4b0096[g_4b00be]) {
            g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
            if (!g_4b0096[g_4b00be])
                g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            other = 0;
        } else {
            other = fn_437acb(g_4b0096[g_4b00be]);
        }
        if (other < most) {
            rows = g_4b0096[g_4b00be - 1];
            g_4b0096[g_4b00be - 1] = g_4b0096[g_4b00be];
            g_4b0096[g_4b00be] = rows;
        }
        g_4b00be++;
    } else {
        g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        g_4b00be++;
        g_4b0096[g_4b00be] = indexOfLargestExcept(0);
        if (!g_4b0096[g_4b00be])
            g_4b0096[g_4b00be] = g_4affee[randomBetween(g_4b00cc, g_4b00ca)];
        g_4b00be++;
    }
}

/* A fifth way of choosing the maze's sequence: three rows' features
   (fn_437416), then enough of the commonest to cover the Zoombinis left,
   the fourth row's, two from the copy of the rows taken, and two at
   random. */
/* @zoombi32 0x00439190 */
void fn_439190()
{
    short n;
    short last;
    short remaining;
    short total;
    short got;
    short values[17];
    short i;

    g_4b00be = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    remaining = 0;
    fn_437089();
    fn_4371b3(0);
    fn_43824f();
    if (!fn_438280())
        fn_43836f();
    g_4b0096[g_4b00be] = fn_437416(0, 0);
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_437416(0, 0);
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_437416(0, 0);
    if (!g_4b0096[g_4b00be])
        g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    g_4b00be++;
    last = fn_437416(0, 0);
    if (!last)
        last = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
    fn_4371b3(g_4b0096[0]);
    fn_4371b3(g_4b0096[1]);
    total = g_4a26aa[fn_4371b3(g_4b0096[2])];
    for (i = 0, n = 0; i < total; i++)
        if (!i) {
            g_4b0096[g_4b00be] = fn_437b7b(1, 1);
            if (!g_4b0096[g_4b00be])
                g_4b0096[g_4b00be] = g_4b0096[randomBetween(0, 2)];
            got = fn_437acb(g_4b0096[g_4b00be]);
            fn_4371b3(g_4b0096[g_4b00be]);
            values[n] = g_4b0096[g_4b00be];
            n++;
            g_4b00be++;
            i = got;
            remaining = total - got;
        } else if (remaining) {
            values[n] = fn_437b7b(1, remaining);
            if (!values[n])
                values[n] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
            got = fn_437acb(values[n]);
            fn_4371b3(values[n++]);
            i += got;
            remaining = total - i;
        }
    g_4b0096[g_4b00be] = last;
    g_4b00be++;
    fn_437089();
    fn_4371b3(0);
    fn_43824f();
    got = fn_438280();
    if (!got)
        fn_43836f();
    fn_4371b3(last);
    g_4b0096[g_4b00be] = fn_437ea2(1, 3);
    if (!g_4b0096[g_4b00be]) {
        if (got)
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        else
            g_4b0096[g_4b00be] = g_4b0096[0];
    }
    fn_4371b3(g_4b0096[g_4b00be]);
    g_4b00be++;
    g_4b0096[g_4b00be] = fn_437ea2(1, 3);
    if (!g_4b0096[g_4b00be]) {
        if (got)
            g_4b0096[g_4b00be] = g_4b0018[randomBetween(g_4b00c8, g_4b00c6)];
        else
            g_4b0096[g_4b00be] = g_4b0096[0];
    }
    fn_4371b3(g_4b0096[g_4b00be]);
    g_4b00be++;
    g_4b0096[g_4b00be] = randomBetween(1, 20);
    g_4b00be++;
    g_4b0096[g_4b00be] = randomBetween(1, 20);
    g_4b00be++;
}
