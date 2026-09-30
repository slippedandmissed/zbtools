/*
 * maze (0x433510-0x439560): the scene in Maze2.MHK
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "lilly.h"
#include "maze.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* @zoombi32 0x0043595f */
void mazeNoDraw(View *)
{
}

/* @zoombi32 0x00435966 */
void mazeNoUpdate(View *, short)
{
}

/*
 * Only an unsigned constant (or `-=` on a local) gives the original's
 * `sub eax, 50`; a signed one becomes `add eax, -50`. Perhaps a sizeof or an
 * unsigned #define.
 */
/* @zoombi32 0x0043691d */
int valueLess50(long, short value)
{
    return value - 50u;
}

/* The index (1-20) of the largest value, ignoring `exclude`. */
/* @zoombi32 0x00437390 */
short indexOfLargestExcept(short exclude)
{
    short best, bestValue, i;
    for (i = 1, best = 0, bestValue = 0; i < 0x15; i++) {
        if (valueCounts[i] > bestValue && exclude != i) {
            bestValue = valueCounts[i];
            best = i;
        }
    }
    return best;
}

/* @zoombi32 0x00437acb */
short valueCount(short i)
{
    return valueCounts[i];
}

/* How many of valueCounts[1..20] are non-zero. */
/* @zoombi32 0x004381bb */
short countValuesPresent()
{
    short i, count;
    for (i = 1, count = 0; i < 0x15; i++)
        if (valueCounts[i])
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
    loadResourceAs(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    *handle = usedResourceHandle(*resource);
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
        freeResource(resource);
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
        handle = usedResourceHandle(g_4a21b4);
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
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeMazeTable(&g_4afc18, &g_4afc20);
        freeResource(&g_4a21b4);
        freeResource(&g_4afbd8);
        freeResource(&g_4afbdc);
        freeResource(&g_4afbe0);
        freeResource(&g_4afbe4);
        freeResource(&g_4afbc4);
        freeResource(&g_4afbc8);
        freeResource(&g_4afbcc);
        setFreeAtOnce(saved);
        closeGameFile(&g_4afc64);
        fadeOutViews();
        showBusyCursor();
    }
}

/*
 * The maze's buttons: leaves at once if asked to; 1 asks to leave for the
 * map (999, keeping the party); 2, once allowed (g_4afc6a), asks to leave
 * for scene 6 (996). 3 is the maze itself, until a Zoombini is on its way:
 * the first time sets up the Zoombinis' parts; then drags the one under
 * the cursor (not one already in the maze, and at most 10) onto a free
 * starting place (1-14), which it takes up: its square, direction and
 * helper view (by the place's tables). Dropped elsewhere, it goes back to
 * where it was, and the gate for its row (1 or 3) closes.
 */
/* @zoombi32 0x00435264 */
void mazeButtonClicked(short button)
{
    Snoid *snoid;
    Point cursor;
    Point where;
    Point unused[2];
    Point target;
    Point at;
    View *view;
    short *parts;
    View *helper;
    short i;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        setCurrentMap(0);
        closeMaze();
        return;
    }
    switch (button) {
    case 1:
        queueViewSound(999, 0);
        drawMazeButton(button, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawMazeButton(button, 0, 1);
        g_4b755c = 1;
        g_4b0d52 = 1;
        askKeepParty();
        break;
    case 2:
        if (g_4afc6a) {
            queueViewSound(996, 0);
            drawMazeButton(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawMazeButton(button, 0, 1);
            g_4b0d52 = 6;
        }
        break;
    case 3:
        if (g_4b755a > 0)
            break;
        if (!g_4afc38 && featureRowCount > 0) {
            g_4afc38 = 1;
            for (helper = viewListEnd(1); helper; helper = helper->next)
                if (helper->flags == 1) {
                    snoid = (Snoid *)&helper->body;
                    setUpMazeParts(snoid);
                }
        }
        getCursorPosition(&cursor);
        view = viewAt(cursor, 1, 1);
        if (!view)
            view = viewAt(cursor, 0x8001, 1);
        if (!view || g_4afc60 >= 10)
            break;
        g_4afc3c = 1;
        for (i = 0; i < 10; i++)
            if (g_4afc4a[i] == view->id)
                g_4afc3c = 0;
        if (g_4afc3c != 1 || g_4afc3e)
            break;
        snoid = (Snoid *)&view->body;
        parts = (short *)snoid;
        where = *(Point *)&view->body.x;
        dragSnoid(view, cursor, &g_4a2554[parts[35]], 0);
        if ((g_4afc40 = heldPlaceNumber()) > 0) {
            for (i = 0; i < 14; i++)
                if (g_4afc6c[i] == g_4afc6c[g_4afc40] && g_4afc6c[g_4afc40])
                    g_4afc3c = 0;
            if (!g_4afc3c)
                break;
            g_4afc40--;
            view->flags = 0x4008001;
            g_4afc4a[g_4afc60] = view->id;
            g_4afc60++;
            snoid->unknownF1 = g_4a2260[g_4afc40];
            snoid->unknownF2 = g_4a2228[g_4afc40];
            snoid->body.x = g_4a21f0[g_4afc40].x;
            snoid->body.y = g_4a21f0[g_4afc40].y;
            parts[20] = g_4a227c[g_4afc40];
            parts[31] = g_4a2362[g_4afc40].x;
            parts[32] = g_4a2362[g_4afc40].y;
            switch (parts[20]) {
            case 0:
                parts[34] = parts[32] - 1;
                parts[33] = parts[31];
                break;
            case 1:
                parts[33] = parts[31];
                parts[34] = parts[32];
                break;
            case 2:
                parts[34] = parts[32] + 1;
                parts[33] = parts[31];
                break;
            case 3:
                parts[33] = parts[31];
                parts[34] = parts[32];
                break;
            }
            parts[29] = 0;
            parts[36] = g_4a2298[g_4afc40];
            helper = findView(g_4afd2c[g_4afc40]);
            if (helper) {
                parts = (short *)&helper->body;
                parts[50] = view->id;
                parts[44] = g_4afc40 + 1;
            }
            g_4b09a8[g_4b0a04] = g_4afc40;
            g_4b0a04++;
            g_4afc6c[g_4afc40 + 1] = g_4afc40 + 1;
        } else {
            if (((Snoid *)&view->body)->unknownF4 == 4) {
                target = *(Point *)&((Snoid *)&view->body)->targetX;
                at = *(Point *)&view->body.x;
                if (target.x != at.x || target.y != at.y) {
                    unionRgnRect(removedRgn, &view->body.bounds);
                    *(Point *)&((Snoid *)&view->body)->targetX = where;
                    *(Point *)&view->body.unknownAa = where;
                    *(Point *)&view->body.x = where;
                    layOutSnoid(snoid, 0);
                }
            }
            switch (parts[35]) {
            case 1:
                helper = findView(g_4afd8c[0]);
                if (helper)
                    sortFlaggedViews(helper, g_4a2548);
                break;
            case 3:
                helper = findView(g_4afd8c[0]);
                if (helper)
                    sortFlaggedViews(helper, g_4a254c);
                break;
            }
        }
        break;
    }
}

/* The scene's keys: 0x16f calls replayHint (the only one; the check of
   g_4b8803 for other keys is left from the other scenes' cheat keys).
   Returns whether the key was used. */
/* @zoombi32 0x0043570e */
short mazeKey(unsigned short key)
{
    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        replayHint();
        return 1;
    }
    return 0;
}

/* Starts the script of the Zoombini paired with `view` (its body's word 50
   is the Zoombini's view): the entry of its own table (words 37 on) at its
   index (word 36), moving it into `group`. */
/* @zoombi32 0x0043583c */
void startPairedSnoidScript(View *view, short group, ViewNotify, char unknownF8)
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
void startPairedSnoidPoseScript(View *view, short group, ViewNotify, char unknownF8)
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
void startPairedSnoid14003(View *view, short group, ViewNotify, char unknownF8)
{
    short *parts = (short *)&view->body;
    View *other = findView(parts[50]);

    startSnoidScript((Snoid *)&other->body, 14003, 0, unknownF8);
    other->body.group = group;
}

/* A view's placing: its first cel by the hot spots in mazeHotSpotsX/mazeHotSpotsY,
   35 below. */
/* @zoombi32 0x00436321 */
void placeOnHotSpot35(View *view)
{
    short *cel = (short *)&view->body;

    cel[1] -= mazeHotSpotsX[cel[0]];
    cel[2] += 35 - mazeHotSpotsY[cel[0]];
}

/* The same, 25 below. */
/* @zoombi32 0x00436356 */
void placeOnHotSpot25(View *view)
{
    short *cel = (short *)&view->body;

    cel[1] -= mazeHotSpotsX[cel[0]];
    cel[2] += 25 - mazeHotSpotsY[cel[0]];
}

/* The arrival hook (setArrivalHook): a Zoombini arriving in pose 1 or 3
   sets g_4b0a0a or g_4b0a0c. */
/* @zoombi32 0x00435f03 */
void mazeArrivalHook(short id)
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

/* Lists (by index, in place) the entries of valueCounts that are empty. */
/* @zoombi32 0x0043824f */
void listEmptyValues()
{
    short i;

    fillMemory(emptyValues, 0, 42);
    for (i = 0; i < 21; i++)
        if (!valueCounts[i])
            emptyValues[i] = i;
}

/* Packs the list made by listEmptyValues into emptyValueList (from 1), counting them
   in emptyValueCount (anyEmptyValue: any); returns the count. */
/* @zoombi32 0x00438280 */
short packEmptyValues()
{
    short i;

    anyEmptyValue = 0;
    fillMemory(emptyValueList, 0, 42);
    for (i = 0, emptyValueCount = 0; i < 21; i++)
        if (emptyValues[i]) {
            anyEmptyValue = 1;
            emptyValueCount++;
            emptyValueList[emptyValueCount] = emptyValues[i];
        }
    return emptyValueCount;
}

/* The same as listEmptyValues into emptyValues2. */
/* @zoombi32 0x004382df */
void listEmptyValues2()
{
    short i;

    fillMemory(emptyValues2, 0, 42);
    for (i = 0; i < 21; i++)
        if (!valueCounts[i])
            emptyValues2[i] = i;
}

/* The same as packEmptyValues from emptyValues2 into emptyValueList2 (emptyValueCount2, anyEmptyValue2). */
/* @zoombi32 0x00438310 */
short packEmptyValues2()
{
    short i;

    anyEmptyValue2 = 0;
    fillMemory(emptyValueList2, 0, 42);
    for (i = 0, emptyValueCount2 = 0; i < 21; i++)
        if (emptyValues2[i]) {
            anyEmptyValue2 = 1;
            emptyValueCount2++;
            emptyValueList2[emptyValueCount2] = emptyValues2[i];
        }
    return emptyValueCount2;
}

/* Lists all of 0-20 in emptyValueList (count 20). */
/* @zoombi32 0x0043836f */
void listAllValues()
{
    short i;

    for (i = 0; i < 21; i++)
        emptyValueList[i] = i;
    emptyValueCount = 20;
    anyEmptyValue = 1;
}

/* The index (1-19) of the largest value in valueCounts between `low` and
   `high` whose kind (valueKinds) is `kind`'s. */
/* Not exact: BCC caches valueCounts's address in a register here, where the
   original keeps the parameters in registers instead (see findings.md on
   address caching). */
/* @zoombi32 0x004381da */
short largestOfKind(short kind, short low, short high)
{
    short i, best, value;

    for (i = 1, value = 0, best = 0; i < 20; i++)
        if (valueKinds[i] == valueKinds[kind] && valueCounts[i] >= low && valueCounts[i] <= high && value < valueCounts[i]) {
            value = valueCounts[i];
            best = i;
        }
    return best;
}

/* The index (1-20) of the smallest value in valueCounts from `least` on. */
/* @zoombi32 0x00437ade */
short smallestFrom(short least)
{
    short i, best, value;

    for (i = 0, best = 0, value = 21; i < 20; i++)
        if (value > valueCounts[i + 1] && least <= valueCounts[i + 1]) {
            value = valueCounts[i + 1];
            best = i + 1;
        }
    return best;
}

/* The index (1-20) of the largest value in valueCounts between `low` and
   `high`. */
/* Not exact: BCC caches valueCounts's address in a register here, where the
   original keeps the parameters in registers instead (see findings.md on
   address caching). */
/* @zoombi32 0x00437b23 */
short largestBetween(short low, short high)
{
    short i, best, value;

    for (i = 1, best = 0, value = 0; i < 21; i++)
        if (low <= valueCounts[i] && high >= valueCounts[i] && value < valueCounts[i]) {
            value = valueCounts[i];
            best = i;
        }
    return best;
}

/* The index (1-20) of the smallest positive value in valueCounts, ignoring
   `exclude`. */
/* @zoombi32 0x004373cd */
short smallestPositiveExcept(short exclude)
{
    short i, best, value;

    for (i = 1, best = 0, value = 20; i < 21; i++)
        if (value > valueCounts[i] && valueCounts[i] > 0 && exclude != i) {
            value = valueCounts[i];
            best = i;
        }
    return best;
}

/* The first entry set in the rows of featureRows (featureRowCount of them) other
   than in column `which`, plus that column's offset (featureOffsets); 0 if
   none. */
/* @zoombi32 0x00437331 */
short firstFeatureNotIn(short which)
{
    short row, column;

    for (row = 0; row < featureRowCount; row++)
        for (column = 0; column < 4; column++)
            if (column != which && featureRows[row][column])
                return featureRows[row][column] + featureOffsets[column];
    return 0;
}

/* A view's notify: when its script ends (-1) with g_4b0d3a up to
   g_4b0d38, clears g_4b0d3c. */
/* @zoombi32 0x00436045 */
void mazeEndNotify(View *, short event)
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
void noteEvent10(View *, short event)
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

/* A view's drawing: its cels from the bank mazeImages, while it runs and
   stands in the game's area. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x0043692b */
void drawMazeSnoid(View *view)
{
    if (view->body.running && ptInRect(&gameRect, *(Point *)&view->body.x)) {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = mazeImages;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
}

/* The scene's Zoombini views' update: lays the Zoombini out again
   (layOutMazeCels) unless it's in state 1. */
/* @zoombi32 0x00436994 */
void updateMazeSnoid(View *view, short region)
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
            layOutMazeCels(snoid);
            view->changed = 1;
        }
    }
}

/* Copies the chosen Zoombinis' features into the rows of featureRows. */
/* Not exact: in the second loop the original computes the row's address
   before the column's index; BCC does it the other way round however the
   element is written. */
/* @zoombi32 0x00437089 */
void copyChosenFeatures()
{
    short row;
    short column;
    ChosenSnoids *chosen;

    for (row = 0; row < featureRowCount; row++)
        for (column = 0; column < 4; column++)
            featureRows[row][column] = 0;
    chosen = listChosenSnoids();
    for (row = 0; row < featureRowCount; row++)
        for (column = 0; column < 4; column++)
            featureRows[row][column] = chosen->features[row][column];
}

/* Copies into featureRows only the chosen Zoombinis with a feature that is
   `id` (with featureOffsets's offsets); returns how many. */
/* @zoombi32 0x004370f8 */
short copyChosenWithFeature(short id)
{
    short count;
    short row;
    short column;
    short found;
    ChosenSnoids *chosen;

    for (row = 0; row < featureRowCount; row++)
        for (column = 0; column < 4; column++)
            featureRows[row][column] = 0;
    count = 0;
    chosen = listChosenSnoids();
    for (row = 0; row < featureRowCount; row++) {
        found = 0;
        for (column = 0; column < 4; column++)
            if (chosen->features[row][column] + featureOffsets[column] == id)
                found = 1;
        if (found) {
            for (column = 0; column < 4; column++)
                featureRows[row][column] = chosen->features[row][column];
            count++;
        }
    }
    return count;
}

/* The first entry of column `which` in the rows of featureRows that is set
   and isn't `ignore`, plus the column's offset (featureOffsets); 0 if none. */
/* @zoombi32 0x004372bf */
short firstFeatureIn(short which, short ignore)
{
    short row, column;

    for (row = 0; row < featureRowCount; row++)
        for (column = 0; column < 4; column++)
            if (column == which && featureRows[row][column] != ignore && featureRows[row][column])
                return featureRows[row][column] + featureOffsets[column];
    return 0;
}

/* A view's notify: 61 starts its paired Zoombini's script 14004 in its
   group (then told helperDoneNotify); 63 lists the view in g_4b0908. */
/* @zoombi32 0x00435e8a */
void startPairNotify(View *view, short event)
{
    switch (event) {
    case 61: {
        short *parts = (short *)&view->body;
        View *other = findView(parts[50]);

        if (other) {
            startSnoidScript((Snoid *)&other->body, 14004, 0, 1);
            other->body.group = view->body.group;
            other->notify = helperDoneNotify;
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
short *loadHotSpotTable(short which)
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
   script 14004 (then told poseDoneNotify), 63 lists the view in g_4b0908. */
/* @zoombi32 0x00435b9e */
void turnOrStartPairNotify(View *view, short event)
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
            other->notify = poseDoneNotify;
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
   square (squareOccupants, by its words 33 and 34) if the square is still its;
   120 starts its paired Zoombini's script 14007. */
/* @zoombi32 0x00435da5 */
void leaveSquareNotify(View *view, short event)
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
        if (squareOccupants[parts[33]][parts[34]][1] == view->id) {
            squareOccupants[parts[33]][parts[34]][0] = 0;
            squareOccupants[parts[33]][parts[34]][1] = 0;
        }
        break;
    case 120:
        parts = (short *)&view->body;
        other = findView(parts[50]);
        if (other) {
            startSnoidScript((Snoid *)&other->body, 14007, 0, 0);
            other->notify = poseDoneNotify;
            other->body.group = view->body.group;
        }
        break;
    }
}

/* Starts view g_4afd2c[n]'s script (g_4a2308[n], then told mazeViewNotify),
   with its second view's (g_4afd48[n], if g_4a22d0[n]), grouping them with
   its paired Zoombini's view. */
/* @zoombi32 0x0043573e */
void startMazeView(short n)
{
    short *parts;
    View *view;
    View *second;
    View *other;

    second = 0;
    view = findView(g_4afd2c[n]);
    if (view) {
        setViewScript(view, g_4a2308[n], 1);
        view->notify = mazeViewNotify;
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
void helperDoneNotify(View *view, short event)
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
        if (squareOccupants[parts[33]][parts[34]][1] == view->id) {
            squareOccupants[parts[33]][parts[34]][0] = 0;
            squareOccupants[parts[33]][parts[34]][1] = 0;
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
   and lists it in g_4b0958 unless in pose 3; -1 as helperDoneNotify's. */
/* @zoombi32 0x00435c57 */
void poseDoneNotify(View *view, short event)
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
        if (squareOccupants[parts[33]][parts[34]][1] == view->id) {
            squareOccupants[parts[33]][parts[34]][0] = 0;
            squareOccupants[parts[33]][parts[34]][1] = 0;
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

/* Clears the rows of featureRows with a feature that is `id`, counts the
   features of the complete rows left in valueCounts (by value, with
   featureOffsets's offsets), and returns how many complete rows there are. */
/* @zoombi32 0x004371b3 */
short clearRowsWithFeature(short id)
{
    short count;
    short keep;
    short i;
    short row;

    count = 0;
    for (i = 0; i < 21; i++)
        valueCounts[i] = 0;
    for (row = 0; row < featureRowCount; row++) {
        keep = 1;
        for (i = 0; i < 4; i++)
            if (featureRows[row][i] && featureRows[row][i] + featureOffsets[i] == id && id) {
                keep = 0;
                for (i = 0; i < 4; i++)
                    featureRows[row][i] = 0;
                i = 4;
            }
        if (keep)
            for (i = 0; i < 4; i++)
                if (featureRows[row][i]) {
                    valueCounts[featureRows[row][i] + featureOffsets[i]]++;
                } else {
                    keep = 0;
                    i = 4;
                }
        if (keep)
            count++;
    }
    return count;
}

/*
 * The maze's frame: leaves once asked to (and sound 996 is done); unless
 * paused, works through the Zoombinis waiting for each step: ones placed
 * to start (startMazeView), stopped (stepMazeSnoidOn), moving in front, reaching
 * the gates, falling (15090 on), gates to close (g_4b0a0a/c), ones done
 * walking off to their row's exit (by the next of 20 spots in g_4a2406),
 * ones reaching a square (by its kind in g_4b061a: 0 stops, 1 a turn, 2
 * and 3-4 a turning square, 5 a straight one, 6 a blocked one (sound
 * 5103), 20-23 a pose), and pairs meeting; and starts waiting Zoombinis
 * fidgeting now and then while g_4b0d3c (its test of a view's flags is
 * always true: `==` binds before `|`).
 */
/* @zoombi32 0x0043490e */
void mazeFrame()
{
    short *placed = &g_4b0a04;
    short *stopped = &g_4b09fe;
    short *moving = &g_4b09fc;
    View *view;
    short *parts;
    short *spot;
    short done;
    short i;
    short n;
    View *other;
    Snoid *snoid;

    if (g_4a25c8 || !g_4afc68)
        return;
    g_4a25c8 = 1;
    updateViews();
    if (g_4b0d52) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            g_4a25c8 = 0;
            return;
        }
        if (!g_4b9688 || g_4b9688 == 3) {
            if (g_4b9688 == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !g_4b755a || g_4b755c >= 1) {
                g_4b0d50 = g_4b0d52;
                g_4b0d52 = 0;
                setCurrentMap(0);
                closeMaze();
                g_4a25c8 = 0;
                return;
            }
        } else if (g_4b9688 == 2) {
            g_4b9688 = 0;
            g_4b0d52 = 0;
        }
    } else if (g_4b9684) {
        playAmbientSound();
        g_4a25c8 = 0;
        return;
    } else {
        while (*placed)
            startMazeView(g_4b09a8[--*placed]);
        while (*stopped) {
            view = findView(g_4b0930[--*stopped]);
            if (view)
                stepMazeSnoidOn(view);
        }
        while (*moving) {
            view = findView(g_4b0908[--*moving]);
            if (view)
                moveView(view->id, 0, g_4afc2a);
        }
        while (g_4b09fa) {
            view = findView(g_4b08b8[--g_4b09fa]);
            if (view) {
                parts = (short *)&view->body;
                moveView(view->id, 0, g_4afd8c[parts[32]]);
            }
        }
        while (g_4b0a06) {
            view = findView(g_4b09d0[--g_4b0a06]);
            if (view) {
                snoid = (Snoid *)&view->body;
                startSnoidScript((Snoid *)&view->body, snoid->features[3] + 15090, 0, 0);
                view->notifyEnd = 1;
                view->notify = poseDoneNotify;
            }
        }
        if (g_4b0a0a) {
            g_4b0a0a = 0;
            view = findView(g_4afd8c[0]);
            if (view)
                sortFlaggedViews(view, g_4a2548);
        }
        if (g_4b0a0c) {
            g_4b0a0c = 0;
            view = findView(g_4afd8c[0]);
            if (view)
                sortFlaggedViews(view, g_4a254c);
        }
        while (g_4b0a00) {
            view = findView(g_4b0958[--g_4b0a00]);
            if (view) {
                spot = 0;
                parts = (short *)&view->body;
                switch (parts[35]) {
                case 0:
                    spot = &g_4afe52;
                    view->flags = 1;
                    moveView(view->id, 1, g_4afd8c[10]);
                    break;
                case 1:
                    spot = &g_4afe54;
                    view->flags = 0x8001;
                    moveView(view->id, 0, g_4afd8c[1]);
                    break;
                case 2:
                    spot = &g_4afe56;
                    view->flags = 1;
                    moveView(view->id, 1, g_4afd8c[11]);
                    break;
                case 3:
                    spot = &g_4afe58;
                    view->flags = 0x4008001;
                    if (countChosenSnoids() == featureRowCount) {
                        g_4b0d3c = 1;
                        queueViewSound(randomBetween(20055, 20063), 0);
                    }
                    moveView(view->id, 0, g_4afd8c[parts[34]]);
                    break;
                }
                if (spot) {
                    snoid = (Snoid *)&view->body;
                    snoid->unknownF1 = 0;
                    setSnoidAction((Snoid *)&view->body, 7, 0);
                    *(Point *)&((Snoid *)&view->body)->targetX = g_4a2406[parts[35]][(*spot)++];
                    if (*spot > 19)
                        *spot = 0;
                }
            }
        }
        while (g_4b09f8) {
            view = findView(g_4b08e0[--g_4b09f8]);
            if (view) {
                parts = (short *)&view->body;
                if (parts[34] != parts[32]) {
                    moveView(view->id, 0, g_4afd8c[parts[34]]);
                    moveView(parts[41], 1, view->id);
                }
                if (squareOccupants[parts[33]][parts[34]][1] == view->id) {
                    squareOccupants[parts[33]][parts[34]][0] = 0;
                    squareOccupants[parts[33]][parts[34]][1] = 0;
                }
                switch (g_4b061a[parts[33]][parts[34]]) {
                case 0:
                    stepMazeSnoidOn(view);
                    break;
                case 1:
                    putOnSquare(view, g_4b04c8[parts[33]][parts[34]]);
                    break;
                case 2:
                    stepAtTurning(view, g_4b04c8[parts[33]][parts[34]]);
                    break;
                case 3:
                case 4:
                    stepMazeSnoid(view, g_4b04c8[parts[33]][parts[34]]);
                    break;
                case 5:
                    moveMazeSnoidOn(view, g_4b04c8[parts[33]][parts[34]]);
                    break;
                case 6:
                    queueViewSound(5103, 0);
                    stopMazeSnoid(g_4b04c8[parts[33]][parts[34]]);
                    stepMazeSnoidOn(view);
                    break;
                case 20:
                case 21:
                case 22:
                case 23:
                    putSnoidInMaze(view, g_4b061a[parts[33]][parts[34]]);
                    break;
                default:
                    stepMazeSnoidOn(view);
                    break;
                }
            }
        }
        while (g_4b0a02 > 1) {
            view = findView(g_4b0980[--g_4b0a02]);
            if (view) {
                g_4b0980[g_4b0a02] = 0;
                other = findView(g_4b0980[--g_4b0a02]);
                if (other) {
                    g_4b0980[g_4b0a02] = 0;
                    mazeZoombinisMeet(view, other);
                }
            }
        }
        if (g_4b0d3c && g_4b0d3a < g_4b0d38) {
            if (clockTime() - g_4b0d30 > 30) {
                done = 0;
                g_4b0d30 = clockTime();
                for (i = 0; i < featureRowCount && !done; i++) {
                    n = allocateSlot(&g_4b0d34, featureRowCount, 0);
                    if (partyViews[n]) {
                        view = idleSnoidView(partyViews[n]);
                        if (view && view->body.running && (view->flags == 0x8000 | 0x4000001)) {
                            snoid = (Snoid *)&view->body;
                            startSnoidScript((Snoid *)&view->body, snoid->features[3] + 15090, 0, 0);
                            view->notifyEnd = 1;
                            view->notify = mazeEndNotify;
                            g_4b0d3a++;
                            done = 1;
                        }
                    }
                }
            }
        } else if (g_4b0d3a >= g_4b0d38) {
            g_4b0d3a = g_4b0d3c = g_4b0d30 = g_4b0d34 = 0;
        }
    }
    playAmbientSound();
    g_4a25c8 = 0;
}

/* Puts a Zoombini in the maze in pose `pose` - 20 on its square (from
   squarePlaces), with its helper view (word 41: script 10040) and a shadow
   view it adds (word 42: script 10041), grouped; the first to reach pose
   3 turns the go button on. */
/* @zoombi32 0x004350be */
void putSnoidInMaze(View *view, short pose)
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
    *(Point *)&view->body.x = (squarePlaces + parts[34])[parts[33] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, 10040, 1);
        *(Point *)&helper->body.x = *(Point *)&view->body.x;
        helper->placed = placeOnHotSpot25;
        helper->notify = leaveSquareNotify;
        where = *(Point *)&helper->body.x;
        parts[42] = addView(0x900000, drawCels, runViewScript, 10041, 7, &where, 0, 0);
        shadow = findView(parts[42]);
        if (shadow) {
            shadow->placed = placeOnHotSpot35;
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
   scripts; 65, 75 and 85 moveSnoidToSquare; 64, 74 and 84 list the Zoombini's view
   in g_4b08b8 (74 also forgets it); 66, 76 and 86 free its place. */
/* @zoombi32 0x00436092 */
void mazeViewNotify(View *view, short event)
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
        startPairedSnoidScript(view, view->body.group, mazeViewNotify, 0);
        break;
    case 62:
        startPairedSnoidPoseScript(view, view->body.group, mazeViewNotify, 0);
        break;
    case 64:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        break;
    case 65:
        moveSnoidToSquare(view, view->body.group, mazeViewNotify, 1);
        break;
    case 66:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    case 71:
        startPairedSnoidScript(view, view->body.group, mazeViewNotify, 0);
        break;
    case 72:
        startPairedSnoidPoseScript(view, view->body.group, mazeViewNotify, 1);
        break;
    case 74:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        parts[50] = 0;
        break;
    case 75:
        moveSnoidToSquare(view, view->body.group, mazeViewNotify, 0);
        break;
    case 76:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    case 81:
        startPairedSnoidScript(view, view->body.group, mazeViewNotify, 0);
        break;
    case 82:
        startPairedSnoidPoseScript(view, view->body.group, mazeViewNotify, 1);
        break;
    case 84:
        parts = (short *)&view->body;
        g_4b08b8[g_4b09fa] = parts[50];
        g_4b09fa++;
        break;
    case 85:
        moveSnoidToSquare(view, view->body.group, mazeViewNotify, 0);
        break;
    case 86:
        parts = (short *)&view->body;
        claimPlacedView(parts[44], 0);
        g_4afc6c[parts[44]] = 0;
        break;
    }
}

/* Moves the Zoombini paired with `view` onto its square (words 31 and
   32, squarePlaces) by its pose (word 20), adds a helper view there for the
   pose (paired back with the Zoombini), and starts the Zoombini's script
   for the pose (then told mazeSnoidNotify), in `group`. */
/* @zoombi32 0x0043596d */
void moveSnoidToSquare(View *view, short group, ViewNotify, char unknownF8)
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
        *(Point *)&snoid->body.x = (squarePlaces + parts[32])[parts[31] * 13];
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
            helper->placed = placeOnHotSpot35;
            helper->body.group = group;
            parts = (short *)&helper->body;
            parts[50] = other->id;
            runViewScript(helper, removedRgn);
        }
        startSnoidScript((Snoid *)&other->body, script, 0, unknownF8);
        other->notify = mazeSnoidNotify;
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
 * in valueCounts for which some row of featureRows has that feature in a column
 * where no row already taken (takenRows) has the same one; takes that row
 * (into takenRows, up to 20), clears the rows with the value, recounts
 * valueCounts, and returns the value (0: none).
 */
/* Not exact: the original tests the outer loop's condition before its
   first pass (BCC drops that test for a constant start however the loop is
   written), and copies `v` to `best` through dx rather than ax. */
/* @zoombi32 0x0043780d */
short takeRarestValue(short exclude)
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
        if (valueCounts[v] > 0 && valueCounts[v] <= least && exclude != v)
            for (row = 0; row < featureRowCount; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (featureRows[row][column] > 0 && featureRows[row][column] + featureOffsets[column] == v) {
                        for (k = 0; k < 20 && g_4b00d0 < 4; k++)
                            if (takenRows[k][column] > 0 && takenRows[k][column] == featureRows[row][column])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            bestRow = row;
                            least = valueCounts[v];
                            row = featureRowCount;
                        }
                        column = 4;
                    }
                }
    }
    if (best) {
        fillMemory(valueCounts, 0, 42);
        if (g_4b00d0 < 20) {
            takenRows[g_4b00d0][0] = featureRows[bestRow][0];
            takenRows[g_4b00d0][1] = featureRows[bestRow][1];
            takenRows[g_4b00d0][2] = featureRows[bestRow][2];
            takenRows[g_4b00d0][3] = featureRows[bestRow][3];
            g_4b00d0++;
        }
        for (row = 0; row < featureRowCount; row++) {
            for (column = 0; column < 4; column++)
                if (featureRows[row][column] && featureRows[row][column] + featureOffsets[column] == best) {
                    column = 4;
                    for (v = 0; v < 4; v++)
                        featureRows[row][v] = 0;
                }
            if (featureRows[row][0] > 0)
                for (column = 0; column < 4; column++)
                    valueCounts[featureRows[row][column] + featureOffsets[column]]++;
        }
    }
    return best;
}

/*
 * Picks the value (1-20) with the most counts in valueCounts, between `low`
 * and `high`, for which some row of featureRows has that feature in a column
 * where no row already taken (takenRows) has the same one; takes the rows
 * with the value (into takenRows and the copy takenRowsCopy), clears them,
 * recounts valueCounts, and returns the value (0: none).
 */
/* Not exact: the original copies `v` to `best` through dx; this uses ax
   (as in takeRarestValue). */
/* @zoombi32 0x00437b7b */
short takeCommonestValue(short low, short high)
{
    short v;
    short best;
    short most;
    short fresh;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, most = 0; v < 21; v++)
        if (valueCounts[v] > most && valueCounts[v] >= low && valueCounts[v] <= high)
            for (row = 0; row < featureRowCount; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (featureRows[row][column] > 0 && featureRows[row][column] + featureOffsets[column] == v) {
                        for (k = 0; k < 20; k++)
                            if (takenRows[k][column] > 0 && takenRows[k][column] == featureRows[row][column])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            most = valueCounts[v];
                            row = featureRowCount;
                        }
                        column = 4;
                    }
                }
    if (best) {
        fillMemory(valueCounts, 0, 42);
        for (row = 0; row < featureRowCount; row++) {
            for (column = 0; column < 4; column++)
                if (featureRows[row][column] && featureRows[row][column] + featureOffsets[column] == best) {
                    if (g_4b00d0 < 20) {
                        takenRowsCopy[g_4b00d0][0] = featureRows[row][0];
                        takenRowsCopy[g_4b00d0][1] = featureRows[row][1];
                        takenRowsCopy[g_4b00d0][2] = featureRows[row][2];
                        takenRowsCopy[g_4b00d0][3] = featureRows[row][3];
                        takenRows[g_4b00d0][0] = featureRows[row][0];
                        takenRows[g_4b00d0][1] = featureRows[row][1];
                        takenRows[g_4b00d0][2] = featureRows[row][2];
                        takenRows[g_4b00d0][3] = featureRows[row][3];
                        g_4b00d0++;
                    }
                    featureRows[row][0] = 0;
                    featureRows[row][1] = 0;
                    featureRows[row][2] = 0;
                    featureRows[row][3] = 0;
                    column = 4;
                }
            if (featureRows[row][0] > 0)
                for (column = 0; column < 4; column++)
                    valueCounts[featureRows[row][column] + featureOffsets[column]]++;
        }
    }
    return best;
}

/* As takeCommonestValue, checking a Zoombini's features against the rows already
   taken only in the copy (takenRowsCopy), and taking the rows into the
   copy alone. */
/* Not exact: BCC caches featureRows's address in esi, where the original
   keeps `column` there (see findings.md on address caching). */
/* @zoombi32 0x00437ea2 */
short takeCommonestValueCopy(short low, short high)
{
    short (*copies)[4] = takenRowsCopy;
    short v;
    short best;
    short most;
    short fresh;
    short row;
    short column;
    short k;

    for (v = 1, best = 0, most = 0; v < 21; v++)
        if (valueCounts[v] > most && valueCounts[v] >= low && valueCounts[v] <= high)
            for (row = 0; row < featureRowCount; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (featureRows[row][column] > 0 && featureRows[row][column] + featureOffsets[column] == v) {
                        for (k = 0; k < 20; k++)
                            if (copies[k][0] > 0 && copies[k][0] == featureRows[row][0])
                                fresh = 0;
                            else if (copies[k][1] > 0 && copies[k][1] == featureRows[row][1])
                                fresh = 0;
                            else if (copies[k][2] > 0 && copies[k][2] == featureRows[row][2])
                                fresh = 0;
                            else if (copies[k][3] > 0 && copies[k][3] == featureRows[row][3])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            most = valueCounts[v];
                            row = featureRowCount;
                        }
                        column = 4;
                    }
                }
    if (best) {
        fillMemory(valueCounts, 0, 42);
        for (row = 0; row < featureRowCount; row++) {
            for (column = 0; column < 4; column++)
                if (featureRows[row][column] && featureRows[row][column] + featureOffsets[column] == best) {
                    if (g_4b00d0 < 20) {
                        copies[g_4b00d0][0] = featureRows[row][0];
                        copies[g_4b00d0][1] = featureRows[row][1];
                        copies[g_4b00d0][2] = featureRows[row][2];
                        copies[g_4b00d0][3] = featureRows[row][3];
                        g_4b00d0++;
                    }
                    featureRows[row][0] = 0;
                    featureRows[row][1] = 0;
                    featureRows[row][2] = 0;
                    featureRows[row][3] = 0;
                    column = 4;
                }
            if (featureRows[row][0] > 0)
                for (column = 0; column < 4; column++)
                    valueCounts[featureRows[row][column] + featureOffsets[column]]++;
        }
    }
    return best;
}

/* Adds a view for a Zoombini in the maze (drawn by drawMazeSnoid, updated by
   updateMazeSnoid) from `snoid`: gives it the next ten words of g_4b076c (its
   kind, square, line...), records it on its square (g_4b04c8, g_4b061a)
   and in its line's list, and lays it out. */
/* @zoombi32 0x00436d39 */
void addMazeSnoidView(Snoid *snoid)
{
    View *view;
    short id;
    Snoid *made;
    short *parts;
    short i;

    id = addView(1, drawMazeSnoid, updateMazeSnoid, 0, randomBetween(20, 25), snoid, 0, 0);
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
        *(Point *)&made->body.x = (squarePlaces + parts[32])[parts[31] * 13];
        switch (parts[33]) {
        case 1:
            g_4b0a10[g_4b0cfe] = id;
            g_4b0cfe++;
            parts[40] = g_4b0d10[1];
            break;
        case 2:
            g_4b0b6e[g_4b0d00] = id;
            g_4b0d00++;
            parts[40] = g_4b0d10[2];
            break;
        case 3:
            g_4b0ba0[g_4b0d02] = id;
            g_4b0d02++;
            parts[40] = g_4b0d10[3];
            break;
        case 4:
            g_4b0bd2[g_4b0d04] = id;
            g_4b0d04++;
            parts[40] = g_4b0d10[4];
            break;
        case 5:
            g_4b0c04[g_4b0d06] = id;
            g_4b0d06++;
            parts[40] = g_4b0d10[5];
            break;
        case 6:
            g_4b0c36[g_4b0d08] = id;
            g_4b0d08++;
            parts[40] = g_4b0d10[6];
            break;
        case 7:
            g_4b0c68[g_4b0d0a] = id;
            g_4b0d0a++;
            parts[40] = g_4b0d10[7];
            break;
        case 8:
            g_4b0c9a[g_4b0d0c] = id;
            g_4b0d0c++;
            parts[40] = g_4b0d10[8];
            break;
        default:
            g_4b0a10[g_4b0cfe] = id;
            g_4b0cfe++;
            parts[40] = g_4b0d10[1];
            break;
        }
        switch (parts[30]) {
        case 1:
        case 5:
            made->unknownF4 = 2;
            break;
        case 2:
            parts[41] = g_4a25e0[mazeSequence[g_4b00c2]][0] + 1;
            parts[42] = g_4a25e0[mazeSequence[g_4b00c2]][1];
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
        layOutMazeCels(made);
        view->flags = 0x4188000;
        view->nextUpdate = 0;
    }
}

/*
 * As takeRarestValue (up to three rows taken, not `exclude`), remembering the
 * column too: with `whole` 0 it takes just that feature (and the whole row
 * into the copy), else the whole row into both; then clears the row and
 * recounts valueCounts. Returns the value (0: none).
 */
/* @zoombi32 0x00437416 */
short takeRareRow(short exclude, short whole)
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
        if (valueCounts[v] > 0 && valueCounts[v] <= least && exclude != v)
            for (row = 0; row < featureRowCount; row++)
                for (column = 0; column < 4; column++) {
                    fresh = 1;
                    if (featureRows[row][column] > 0 && featureRows[row][column] + featureOffsets[column] == v) {
                        for (k = 0; k < 20 && g_4b00d0 < 3; k++)
                            if (takenRows[k][0] > 0 && takenRows[k][0] == featureRows[row][0])
                                fresh = 0;
                            else if (takenRows[k][1] > 0 && takenRows[k][1] == featureRows[row][1])
                                fresh = 0;
                            else if (takenRows[k][2] > 0 && takenRows[k][2] == featureRows[row][2])
                                fresh = 0;
                            else if (takenRows[k][3] > 0 && takenRows[k][3] == featureRows[row][3])
                                fresh = 0;
                        if (fresh) {
                            best = v;
                            bestRow = row;
                            bestColumn = column;
                            least = valueCounts[v];
                            row = featureRowCount;
                        }
                        column = 4;
                    }
                }
    if (best) {
        if (g_4b00d0 < 4) {
            if (!whole) {
                takenRows[g_4b00d0][bestColumn] = featureRows[bestRow][bestColumn];
                takenRowsCopy[g_4b00d0][0] = featureRows[bestRow][0];
                takenRowsCopy[g_4b00d0][1] = featureRows[bestRow][1];
                takenRowsCopy[g_4b00d0][2] = featureRows[bestRow][2];
                takenRowsCopy[g_4b00d0][3] = featureRows[bestRow][3];
                g_4b00d0++;
            } else {
                takenRowsCopy[g_4b00d0][0] = featureRows[bestRow][0];
                takenRowsCopy[g_4b00d0][1] = featureRows[bestRow][1];
                takenRowsCopy[g_4b00d0][2] = featureRows[bestRow][2];
                takenRowsCopy[g_4b00d0][3] = featureRows[bestRow][3];
                takenRows[g_4b00d0][0] = featureRows[bestRow][0];
                takenRows[g_4b00d0][1] = featureRows[bestRow][1];
                takenRows[g_4b00d0][2] = featureRows[bestRow][2];
                takenRows[g_4b00d0][3] = featureRows[bestRow][3];
                g_4b00d0++;
            }
        }
        featureRows[bestRow][0] = 0;
        featureRows[bestRow][1] = 0;
        featureRows[bestRow][2] = 0;
        featureRows[bestRow][3] = 0;
        fillMemory(valueCounts, 0, 42);
        for (row = 0; row < featureRowCount; row++)
            if (featureRows[row][0])
                for (column = 0; column < 4; column++)
                    valueCounts[featureRows[row][column] + featureOffsets[column]]++;
    }
    return best;
}

/* The maze Zoombinis' notify: at frame 3 of events 20, 30, 40 and 50 it
   counts the Zoombini onto its square (squareOccupants): the first is recorded,
   a second meeting it lists both in g_4b0980 and clears the square (as
   more do); 21, 31, 41, 51 and 61 list it in g_4b08e0. */
/* @zoombi32 0x0043638b */
void mazeSnoidNotify(View *view, short event)
{
    short *parts;
    short count;

    switch (event) {
    case 20:
        if (view->body.frame == 3) {
            parts = (short *)&view->body;
            count = ++squareOccupants[parts[33]][parts[34]][0];
            if (count == 1) {
                squareOccupants[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = squareOccupants[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
            } else if (count > 2) {
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
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
            count = ++squareOccupants[parts[33]][parts[34]][0];
            if (count == 1) {
                squareOccupants[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = squareOccupants[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
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
            count = ++squareOccupants[parts[33]][parts[34]][0];
            if (count == 1) {
                squareOccupants[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = squareOccupants[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
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
            count = ++squareOccupants[parts[33]][parts[34]][0];
            if (count == 1) {
                squareOccupants[parts[33]][parts[34]][1] = view->id;
            } else if (count == 2) {
                g_4b0980[g_4b0a02] = squareOccupants[parts[33]][parts[34]][1];
                g_4b0a02++;
                g_4b0980[g_4b0a02] = view->id;
                g_4b0a02++;
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
            }
            if (count > 2) {
                squareOccupants[parts[33]][parts[34]][1] = 0;
                squareOccupants[parts[33]][parts[34]][0] = 0;
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

/* Adds `count` Zoombini views to the maze (addMazeSnoidView), from a blank
   Zoombini. */
/* @zoombi32 0x00436c71 */
void addMazeSnoids(short count)
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
        addMazeSnoidView(made);
    }
}

/* One way of choosing the maze's sequence of values (mazeSequence, sequenceLength
   of them) from the chosen Zoombinis' features. */
/* @zoombi32 0x00438396 */
void chooseSequence1()
{
    short value;
    short rows;
    short other;

    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    copyChosenFeatures();
    rows = clearRowsWithFeature(0);
    listEmptyValues();
    if (!packEmptyValues())
        listAllValues();
    if (rows >= 3) {
        mazeSequence[sequenceLength] = largestBetween(2, 5);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = largestBetween(6, 9);
            if (!mazeSequence[sequenceLength]) {
                mazeSequence[sequenceLength] = largestBetween(10, 16);
                if (!mazeSequence[sequenceLength])
                    mazeSequence[sequenceLength] = largestBetween(1, 16);
            }
        }
    } else {
        mazeSequence[sequenceLength] = largestBetween(1, 2);
    }
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
    sequenceLength++;
    if (g_4a210c == 2) {
        mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
        sequenceLength++;
    }
    value = clearRowsWithFeature(mazeSequence[sequenceLength]);
    sequenceLength++;
    if (countValuesPresent() > 4) {
        mazeSequence[sequenceLength] = largestBetween(1, g_4a2666[value]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            rows = 0;
        } else {
            rows = valueCount(mazeSequence[sequenceLength]);
        }
        clearRowsWithFeature(mazeSequence[sequenceLength]);
        sequenceLength++;
        mazeSequence[sequenceLength] = largestOfKind(mazeSequence[sequenceLength - 1], 1, g_4a2666[value]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            other = 0;
        } else {
            other = valueCount(mazeSequence[sequenceLength]);
        }
    } else {
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        rows = 0;
        sequenceLength++;
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        other = 0;
    }
    if (rows > other) {
        rows = mazeSequence[sequenceLength - 1];
        mazeSequence[sequenceLength - 1] = mazeSequence[sequenceLength];
        mazeSequence[sequenceLength] = rows;
    }
    sequenceLength++;
}
/* Another way of choosing the maze's sequence of values (the first one
   three times; the largest count when few are left). */
/* @zoombi32 0x00438626 */
void chooseSequence2()
{
    short value;
    short rows;
    short other;

    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    copyChosenFeatures();
    rows = clearRowsWithFeature(0);
    listEmptyValues();
    if (!packEmptyValues())
        listAllValues();
    if (rows >= 3) {
        mazeSequence[sequenceLength] = largestBetween(2, 5);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = largestBetween(6, 9);
            if (!mazeSequence[sequenceLength]) {
                mazeSequence[sequenceLength] = largestBetween(10, 16);
                if (!mazeSequence[sequenceLength])
                    mazeSequence[sequenceLength] = largestBetween(1, 16);
            }
        }
    } else {
        mazeSequence[sequenceLength] = largestBetween(1, 2);
    }
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
    sequenceLength++;
    mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
    sequenceLength++;
    value = clearRowsWithFeature(mazeSequence[sequenceLength]);
    sequenceLength++;
    if (countValuesPresent() > 4) {
        mazeSequence[sequenceLength] = largestBetween(1, g_4a2666[value]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            rows = 0;
        } else {
            rows = valueCount(mazeSequence[sequenceLength]);
        }
        clearRowsWithFeature(mazeSequence[sequenceLength]);
        sequenceLength++;
        mazeSequence[sequenceLength] = largestOfKind(mazeSequence[sequenceLength - 1], 1, g_4a2666[value]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            other = 0;
        } else {
            other = valueCount(mazeSequence[sequenceLength]);
        }
    } else {
        mazeSequence[sequenceLength] = indexOfLargestExcept(0);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            rows = 0;
        } else {
            rows = valueCount(mazeSequence[sequenceLength]);
        }
        sequenceLength++;
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        other = 0;
    }
    if (rows > other) {
        rows = mazeSequence[sequenceLength - 1];
        mazeSequence[sequenceLength - 1] = mazeSequence[sequenceLength];
        mazeSequence[sequenceLength] = rows;
    }
    sequenceLength++;
}

/* A third way of choosing the maze's sequence: the first value twice,
   then values with the rarest features (takeRarestValue). */
/* @zoombi32 0x004388d8 */
void chooseSequence3()
{
    short most;
    short rows;
    short other;

    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    copyChosenFeatures();
    rows = clearRowsWithFeature(0);
    listEmptyValues();
    if (!packEmptyValues())
        listAllValues();
    if (rows >= 3) {
        mazeSequence[sequenceLength] = largestBetween(2, 5);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = largestBetween(6, 9);
            if (!mazeSequence[sequenceLength]) {
                mazeSequence[sequenceLength] = largestBetween(10, 16);
                if (!mazeSequence[sequenceLength])
                    mazeSequence[sequenceLength] = largestBetween(1, 16);
            }
        }
    } else {
        mazeSequence[sequenceLength] = largestBetween(1, 2);
    }
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
    copyChosenWithFeature(mazeSequence[sequenceLength]);
    sequenceLength = 2;
    clearRowsWithFeature(0);
    listEmptyValues2();
    packEmptyValues2();
    mazeSequence[sequenceLength] = takeRarestValue(mazeSequence[0]);
    if (!mazeSequence[sequenceLength]) {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    }
    sequenceLength++;
    if (g_4afc32 == 1) {
        mazeSequence[sequenceLength] = mazeSequence[sequenceLength - 1];
        sequenceLength++;
    }
    mazeSequence[sequenceLength] = takeRarestValue(mazeSequence[0]);
    if (!mazeSequence[sequenceLength]) {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    }
    sequenceLength++;
    mazeSequence[sequenceLength] = takeRarestValue(mazeSequence[0]);
    if (!mazeSequence[sequenceLength]) {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    }
    copyChosenFeatures();
    rows = clearRowsWithFeature(mazeSequence[0]);
    sequenceLength++;
    if (countValuesPresent() > 4) {
        mazeSequence[sequenceLength] = largestBetween(1, g_4a2666[rows]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            most = 0;
        } else {
            most = valueCount(mazeSequence[sequenceLength]);
        }
        clearRowsWithFeature(mazeSequence[sequenceLength]);
        sequenceLength++;
        mazeSequence[sequenceLength] = largestOfKind(mazeSequence[sequenceLength - 1], 1, g_4a2666[rows]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            other = 0;
        } else {
            other = valueCount(mazeSequence[sequenceLength]);
        }
        if (other < most) {
            rows = mazeSequence[sequenceLength - 1];
            mazeSequence[sequenceLength - 1] = mazeSequence[sequenceLength];
            mazeSequence[sequenceLength] = rows;
        }
        sequenceLength++;
    } else {
        if (g_4a210c == 1) {
            mazeSequence[sequenceLength] = indexOfLargestExcept(0);
            if (!mazeSequence[sequenceLength])
                mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            sequenceLength++;
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        } else {
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            sequenceLength++;
            mazeSequence[sequenceLength] = indexOfLargestExcept(0);
            if (!mazeSequence[sequenceLength])
                mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        }
        sequenceLength++;
    }
}

/* A fourth way of choosing the maze's sequence: values with the commonest
   features (takeCommonestValue), then the rarest (takeRarestValue) twice over. */
/* @zoombi32 0x00438d67 */
void chooseSequence4()
{
    short most;
    short first;
    short rows;
    short other;

    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    copyChosenFeatures();
    rows = clearRowsWithFeature(0);
    listEmptyValues();
    if (!packEmptyValues())
        listAllValues();
    first = takeRarestValue(mazeSequence[0]);
    if (rows >= 2) {
        mazeSequence[sequenceLength] = takeCommonestValue(2, 4);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = takeCommonestValue(5, 8);
            if (!mazeSequence[sequenceLength]) {
                mazeSequence[sequenceLength] = takeCommonestValue(9, 12);
                if (!mazeSequence[sequenceLength])
                    mazeSequence[sequenceLength] = takeCommonestValue(1, 16);
            }
        }
    } else {
        mazeSequence[sequenceLength] = takeCommonestValue(1, 1);
    }
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    mazeSequence[sequenceLength + 1] = mazeSequence[sequenceLength];
    copyChosenWithFeature(mazeSequence[sequenceLength]);
    sequenceLength = 2;
    clearRowsWithFeature(0);
    listEmptyValues2();
    packEmptyValues2();
    mazeSequence[sequenceLength] = first;
    sequenceLength++;
    mazeSequence[sequenceLength] = first;
    sequenceLength++;
    mazeSequence[sequenceLength] = takeRarestValue(mazeSequence[0]);
    if (!mazeSequence[sequenceLength]) {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    }
    sequenceLength++;
    mazeSequence[sequenceLength] = takeRarestValue(mazeSequence[0]);
    if (!mazeSequence[sequenceLength]) {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    }
    sequenceLength++;
    copyChosenFeatures();
    clearRowsWithFeature(mazeSequence[0]);
    rows = clearRowsWithFeature(first);
    listEmptyValues2();
    packEmptyValues2();
    if (countValuesPresent() > 4) {
        mazeSequence[sequenceLength] = largestBetween(1, g_4a2666[rows]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
            if (!mazeSequence[sequenceLength])
                mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            most = 0;
        } else {
            most = valueCount(mazeSequence[sequenceLength]);
        }
        clearRowsWithFeature(mazeSequence[sequenceLength]);
        sequenceLength++;
        mazeSequence[sequenceLength] = largestOfKind(mazeSequence[sequenceLength - 1], 1, g_4a2666[rows]);
        if (!mazeSequence[sequenceLength]) {
            mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
            if (!mazeSequence[sequenceLength])
                mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            other = 0;
        } else {
            other = valueCount(mazeSequence[sequenceLength]);
        }
        if (other < most) {
            rows = mazeSequence[sequenceLength - 1];
            mazeSequence[sequenceLength - 1] = mazeSequence[sequenceLength];
            mazeSequence[sequenceLength] = rows;
        }
        sequenceLength++;
    } else {
        mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        sequenceLength++;
        mazeSequence[sequenceLength] = indexOfLargestExcept(0);
        if (!mazeSequence[sequenceLength])
            mazeSequence[sequenceLength] = emptyValues2[randomBetween(anyEmptyValue2, emptyValueCount2)];
        sequenceLength++;
    }
}

/* A fifth way of choosing the maze's sequence: three rows' features
   (takeRareRow), then enough of the commonest to cover the Zoombinis left,
   the fourth row's, two from the copy of the rows taken, and two at
   random. */
/* @zoombi32 0x00439190 */
void chooseSequence5()
{
    short n;
    short last;
    short remaining;
    short total;
    short got;
    short values[17];
    short i;

    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    remaining = 0;
    copyChosenFeatures();
    clearRowsWithFeature(0);
    listEmptyValues();
    if (!packEmptyValues())
        listAllValues();
    mazeSequence[sequenceLength] = takeRareRow(0, 0);
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    sequenceLength++;
    mazeSequence[sequenceLength] = takeRareRow(0, 0);
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    sequenceLength++;
    mazeSequence[sequenceLength] = takeRareRow(0, 0);
    if (!mazeSequence[sequenceLength])
        mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    sequenceLength++;
    last = takeRareRow(0, 0);
    if (!last)
        last = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
    clearRowsWithFeature(mazeSequence[0]);
    clearRowsWithFeature(mazeSequence[1]);
    total = g_4a26aa[clearRowsWithFeature(mazeSequence[2])];
    for (i = 0, n = 0; i < total; i++)
        if (!i) {
            mazeSequence[sequenceLength] = takeCommonestValue(1, 1);
            if (!mazeSequence[sequenceLength])
                mazeSequence[sequenceLength] = mazeSequence[randomBetween(0, 2)];
            got = valueCount(mazeSequence[sequenceLength]);
            clearRowsWithFeature(mazeSequence[sequenceLength]);
            values[n] = mazeSequence[sequenceLength];
            n++;
            sequenceLength++;
            i = got;
            remaining = total - got;
        } else if (remaining) {
            values[n] = takeCommonestValue(1, remaining);
            if (!values[n])
                values[n] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
            got = valueCount(values[n]);
            clearRowsWithFeature(values[n++]);
            i += got;
            remaining = total - i;
        }
    mazeSequence[sequenceLength] = last;
    sequenceLength++;
    copyChosenFeatures();
    clearRowsWithFeature(0);
    listEmptyValues();
    got = packEmptyValues();
    if (!got)
        listAllValues();
    clearRowsWithFeature(last);
    mazeSequence[sequenceLength] = takeCommonestValueCopy(1, 3);
    if (!mazeSequence[sequenceLength]) {
        if (got)
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        else
            mazeSequence[sequenceLength] = mazeSequence[0];
    }
    clearRowsWithFeature(mazeSequence[sequenceLength]);
    sequenceLength++;
    mazeSequence[sequenceLength] = takeCommonestValueCopy(1, 3);
    if (!mazeSequence[sequenceLength]) {
        if (got)
            mazeSequence[sequenceLength] = emptyValueList[randomBetween(anyEmptyValue, emptyValueCount)];
        else
            mazeSequence[sequenceLength] = mazeSequence[0];
    }
    clearRowsWithFeature(mazeSequence[sequenceLength]);
    sequenceLength++;
    mazeSequence[sequenceLength] = randomBetween(1, 20);
    sequenceLength++;
    mazeSequence[sequenceLength] = randomBetween(1, 20);
    sequenceLength++;
}

/* Sets the maze up for a level (0-4): the squares' kinds (g_4b061a, from
   g_4a23be/g_4a239a), the lines' values shuffled (g_4b0d10), a sequence of
   values by one of the ways for the level (alternating between two where
   there are two), and the Zoombinis' views. */
/* @zoombi32 0x00436abf */
void setUpMaze(short level)
{
    short order[12];
    short i;
    short last;
    short pick;

    for (i = 0; i < 11; i++)
        order[i] = g_4a25ca[i];
    for (i = 0; i < 18; i++)
        g_4b061a[g_4a23be[i][0]][g_4a23be[i][1]] = g_4a239a[i];
    g_4b0d10[0] = 0;
    g_4b0d10[10] = 0;
    g_4b0d10[1] = g_4a25ca[1];
    for (last = 8, i = 2; i < 9; i++) {
        pick = randomBetween(2, last);
        g_4b0d10[i] = order[pick];
        for (; pick < last + 1; pick++)
            order[pick] = order[pick + 1];
        last--;
    }
    switch (level) {
    case 0:
        chooseSequence1();
        g_4a210e[0]++;
        if (g_4a210e[0] > 1)
            g_4a210e[0] = 0;
        break;
    case 1:
        if (!g_4a210e[1])
            chooseSequence2();
        else
            chooseSequence3();
        g_4a210e[1]++;
        if (g_4a210e[1] > 1)
            g_4a210e[1] = 0;
        break;
    case 2:
        if (!g_4a210e[2])
            chooseSequence3();
        else
            chooseSequence4();
        g_4a210e[2]++;
        if (g_4a210e[2] > 1)
            g_4a210e[2] = 0;
        break;
    case 3:
        chooseSequence5();
        g_4a210e[3] += 2;
        if (g_4a210e[3] > 2)
            g_4a210e[3] = 0;
        break;
    case 4:
        chooseSequence4();
        break;
    }
    addMazeSnoids(g_4b08b0);
}

/*
 * Opens the maze (Maze2.MHK): resets its state, loads its images, scripts
 * and tables, picks the level (sceneLevel; level 3 with fewer than five
 * Zoombinis plays as 4) and its layout (loadHotSpotTable), adds the views of the
 * layout's pieces and lines, sets the puzzle up (setUpMaze) and brings the
 * Zoombinis in.
 */
/* @zoombi32 0x00433510 */
void openMaze()
{
    Point unused[1];
    Point places[20] = {{287, 394}, {260, 426}, {224, 447}, {188, 441}, {157, 455}, {263, 384}, {219, 397}, {184, 388}, {155, 402}, {121, 417}, {226, 354}, {189, 349}, {156, 354}, {131, 375}, {85, 394}, {164, 311}, {125, 324}, {79, 352}, {29, 318}, {15, 285}};
    short i;
    short kind;
    View *view;

    g_4a7d40 = 0;
    g_4b0d52 = 0;
    g_4afc68 = 0;
    g_4afc6a = 0;
    g_4afc3c = 1;
    g_4afc3e = 0;
    g_4afc40 = 0;
    g_4afc38 = 0;
    g_4afc3a = 0;
    g_4b0d26 = 0;
    emptyValueCount = 0;
    anyEmptyValue = 0;
    g_4afc2e = 0;
    g_4afc48 = 0;
    g_4afc46 = 0;
    g_4afc44 = 0;
    g_4b00d0 = 0;
    g_4afc2c = 0;
    g_4a2116 = 0;
    g_4b00ce = 0;
    fillMemory(takenRows, 0, 160);
    fillMemory(takenRowsCopy, 0, 160);
    fillMemory(featureRows, 0, 160);
    fillMemory(valueCounts, 0, 42);
    fillMemory(mazeSequence, 0, 20);
    fillMemory(g_4b00aa, 0, 20);
    fillMemory(emptyValueList2, 0, 42);
    fillMemory(emptyValueList, 0, 42);
    sequenceLength = 0;
    g_4b00c0 = 0;
    g_4b00c2 = 0;
    g_4a2550 = 0;
    g_4a2552 = 0;
    fillMemory(g_4b04c8, 0, 338);
    fillMemory(g_4b061a, 0, 338);
    g_4afc60 = 0;
    g_4b08b0 = 0;
    g_4b076c = 0;
    g_4b08b0 = 0;
    g_4b08b2 = 0;
    g_4b08b4 = 1;
    g_4afe52 = 16;
    g_4afe54 = 0;
    g_4afe56 = 0;
    g_4afe58 = 0;
    hintSound = 0;
    fillMemory(g_4afc6c, 0, 30);
    fillMemory(g_4b08b8, 0, 40);
    fillMemory(g_4b08e0, 0, 40);
    fillMemory(g_4b0908, 0, 40);
    fillMemory(g_4b0930, 0, 40);
    fillMemory(g_4b0958, 0, 40);
    fillMemory(g_4b09a8, 0, 40);
    g_4b09f8 = 0;
    g_4b09fa = 0;
    g_4b09fc = 0;
    g_4b09fe = 0;
    g_4b0a00 = 0;
    g_4b0a02 = 0;
    g_4b0a04 = 0;
    g_4b0a06 = 0;
    g_4b0d3a = 0;
    g_4b0d38 = 0;
    g_4b0d3c = 0;
    g_4b0a08 = 0;
    g_4b0a0a = 0;
    g_4b0a0c = 0;
    fillMemory(squareOccupants, 0, 1014);
    fillMemory(g_4b09d0, 0, 40);
    fillMemory(g_4afd8c, 0, 32);
    fillMemory(g_4b0980, 0, 40);
    fillMemory(g_4afd26, 0, 6);
    fillMemory(g_4afdac, 0, 6);
    fillMemory(g_4afd2c, 0, 28);
    fillMemory(g_4afd48, 0, 28);
    fillMemory(g_4afc92, 0, 28);
    fillMemory(g_4b0a10, 0, 350);
    fillMemory(g_4b0b6e, 0, 50);
    fillMemory(g_4b0ba0, 0, 50);
    fillMemory(g_4b0bd2, 0, 50);
    fillMemory(g_4b0c04, 0, 50);
    fillMemory(g_4b0c36, 0, 50);
    fillMemory(g_4b0c68, 0, 50);
    fillMemory(g_4b0c9a, 0, 50);
    fillMemory(g_4b0ccc, 0, 50);
    g_4b0cfe = 0;
    g_4b0d00 = 0;
    g_4b0d02 = 0;
    g_4b0d04 = 0;
    g_4b0d06 = 0;
    g_4b0d08 = 0;
    g_4b0d0a = 0;
    g_4b0d0c = 0;
    g_4b0d0e = 0;
    fillMemory(g_4b0d10, 0, 22);
    for (i = 0; i < 11; i++)
        g_4afc4a[i] = 0;
    openGameFile(&g_4afc64, "Maze2.MHK");
    setCurrentMap(g_4afc64);
    loadTerrain(100);
    drawBackdrop(5000);
    loadFeatureGroup(7000, 0, 0);
    loadFeatureGroup(8000, 1, 0);
    loadFeatureGroup(9000, 2, 0);
    loadFeatureGroup(10000, 3, 0);
    loadFeatureGroup(12000, 4, 0);
    loadScripts(7000, 28);
    addScripts(8000, 14, 0);
    addScripts(9000, 8, 0);
    addScripts(10000, 44, 0);
    addScripts(12000, 2, 0);
    setArrivalHook(mazeArrivalHook);
    loadSnoidScripts(14000, 8, 0);
    addSnoidScripts(15000, 96, 0);
    g_4afbd8 = 0;
    squarePlaces = (Point *)loadShortTable(16000, &g_4afbd8);
    loadMazeTable(&g_4afc18, &g_4afc20, 16501, &g_4afc24);
    mazeHotSpotsX = loadShortTable(17000, &g_4afbe0);
    mazeHotSpotsY = loadShortTable(17001, &g_4afbe4);
    mazeImages = loadImageBank(5100, &g_4afbc4);
    g_4afbd0 = loadShortTable(18000, &g_4afbc8);
    g_4afbd4 = loadShortTable(18001, &g_4afbcc);
    fadeOutViews();
    copyPaletteRange(10, 236);
    g_4afc2a = addView(0x4188000, drawCels, runViewScript, 12001, 7, 0, 0, 0);
    setViewPlaces(20, places, 1);
    makePartySnoids(0);
    featureRowCount = listChosenSnoids()->count;
    g_4b0d38 = featureRowCount - 1;
    g_4afc32 = sceneLevel();
    if (g_4afc32 == 3 && featureRowCount < 5)
        g_4afc32++;
    g_4b076c = loadHotSpotTable(g_4afc32);
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind) {
            kind--;
            switch (g_4a22ec[kind]) {
            case 1:
                g_4afd26[g_4a22ec[kind]] = addView(0x4188000, drawCels, runViewScript, g_4a22ec[kind] + 9005, 7, 0, 0, 0);
                break;
            }
        }
    }
    for (i = 0; i < 16; i++)
        placedViews[i] = 0;
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind) {
            kind--;
            g_4afc92[kind] = addView(0x4108000, drawCels, runViewScript, kind + 7000, 6, 0, 0, 0);
        }
    }
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind) {
            kind--;
            placedViewCount = kind;
            placedViews[kind] = addView(0x508a000, drawCels, runViewScript, kind + 7014, 7, &g_4a21f0[kind], 0, 0);
        } else if (!placedViews[i]) {
            g_4b83e4[i] = -10;
        }
    }
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind >= 7 && kind <= 9) {
            kind--;
            switch (kind) {
            case 6:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                addView(0x4008000, drawCels, runViewScript, 8006, 0, 0, 0, 0);
                g_4afc44 = g_4afd2c[kind];
                break;
            case 7:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                addView(0x4008000, drawCels, runViewScript, 8007, 0, 0, 0, 0);
                g_4afc44 = g_4afd2c[kind];
                break;
            case 8:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                g_4afc44 = g_4afd2c[kind];
                break;
            default:
                g_4afd2c[kind] = 0;
                break;
            }
            view = findView(g_4afd2c[kind]);
            if (view) {
                short *parts = (short *)&view->body;

                parts[45] = g_4a22ec[kind];
            }
            if (g_4a22d0[kind])
                g_4afd48[kind] = addView(0x5988000, drawCels, runViewScript, g_4a2308[kind] + 1, 7, &g_4a232a[kind], 0, 0);
        }
    }
    if (g_4afc44) {
        i = addView(0x4008000, drawCels, runViewScript, 8005, 0, 0, 0, 0);
        moveView(i, 1, g_4afc44);
        g_4afc44 = i;
    } else {
        addView(0x4008000, drawCels, runViewScript, 8010, 0, 0, 0, 0);
    }
    g_4afd26[0] = addView(0x4180000, drawCels, runViewScript, 9005, 7, 0, 0, 0);
    if (featureRowCount > 0) {
        g_4b08b0 = g_4b076c[0];
        setUpMaze(g_4afc32);
    }
    for (i = 0; i < 3; i++)
        g_4afd8c[i] = addView(0x4008000, mazeNoDraw, mazeNoUpdate, 8011, 0, 0, 0, 0);
    for (i = 3; i < 11; i++)
        g_4afd8c[i] = addView(0x4008000, mazeNoDraw, mazeNoUpdate, 8011, 0, 0, 0, 0);
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind) {
            kind--;
            switch (g_4a22ec[kind]) {
            case 2:
                g_4afd26[g_4a22ec[kind]] = addView(0x180000, drawCels, runViewScript, g_4a22ec[kind] + 9005, 7, 0, 0, 0);
                break;
            }
        }
    }
    for (i = 1; i < 10; i++) {
        kind = g_4b076c[i];
        if (kind && (kind < 7 || kind > 9)) {
            kind--;
            switch (kind) {
            case 3:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                addView(0x4008000, drawCels, runViewScript, 8002, 0, 0, 0, 0);
                g_4afc46 = g_4afd2c[kind];
                break;
            case 4:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                addView(0x4008000, drawCels, runViewScript, 8003, 0, 0, 0, 0);
                g_4afc46 = g_4afd2c[kind];
                break;
            case 5:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                g_4afc46 = g_4afd2c[kind];
                break;
            case 12:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                addView(0x4000000, drawCels, runViewScript, 8009, 0, 0, 0, 0);
                g_4afc48 = g_4afd2c[kind];
                break;
            case 13:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                g_4afc48 = g_4afd2c[kind];
                break;
            default:
                g_4afd2c[kind] = addView(0x4988000, drawCels, runViewScript, g_4a2308[kind], 7, &g_4a232a[kind], 0, 0);
                break;
            }
            view = findView(g_4afd2c[kind]);
            if (view) {
                short *parts = (short *)&view->body;

                parts[45] = g_4a22ec[kind];
            }
            if (g_4a22d0[kind])
                g_4afd48[kind] = addView(0x5980000, drawCels, runViewScript, g_4a2308[kind] + 1, 7, &g_4a232a[kind], 0, 0);
        }
    }
    if (g_4afc46) {
        i = addView(0x4008000, drawCels, runViewScript, 8001, 0, 0, 0, 0);
        moveView(i, 1, g_4afc46);
        g_4afc46 = i;
    }
    if (g_4afc48) {
        i = addView(0x4008000, drawCels, runViewScript, 8008, 0, 0, 0, 0);
        moveView(i, 1, g_4afc48);
        g_4afc48 = i;
    }
    addView(0x4008000, drawCels, runViewScript, 8011, 0, 0, 0, 0);
    addView(0x4000000, drawCels, runViewScript, 8004, 0, 0, 0, 0);
    addView(0x4000000, drawCels, runViewScript, 8000, 0, 0, 0, 0);
    for (i = 11; i < 12; i++)
        g_4afd8c[i] = addView(0x4008000, mazeNoDraw, mazeNoUpdate, 8011, 0, 0, 0, 0);
    loadShape(&g_4a21b4, 6000, "Map/Go Buttons");
    addView(0x1000, drawMazeButtons, updateMazeButtons, 0, 0, 0, 0, 0);
    fadeOutViews();
    copyPaletteRange(10, 236);
    enterSnoids(0);
    updateViews();
    staggerSnoids(45, 30);
    chooseSnoids(0, 0);
    setGroupLists(&g_4a2194, 1, -0x4000);
    drawMazeButton(1, 0, 0);
    drawMazeButton(2, 0, 0);
    showRect(&g_4aa7b8);
    fadeInViews();
    queueViewSound(997, 0);
    chooseSnoids(0, 0);
    resetViewClock();
    g_4a7d40 = 1;
    g_4afc68 = 1;
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(11000, 11000, 0);
    addSoundRange(5104, 5104, 0);
    addSoundRange(10000, 10000, 0);
    addSoundRange(10002, 10004, 0);
    addSoundRange(9000, 9001, 0);
    addSoundRange(5100, 5103, 0);
    addSoundRange(12000, 12000, 0);
    addSoundRange(10001, 10001, 0);
    queueViewSound(sceneLevel() + 30035, 0);
    campHint((short *)(g_4a4ba0 + 0x44));
    hintSound = 20068;
    requestViewSort();
}
