/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "bctwo.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

/* Resets scene 5's state. */
/* @zoombi32 0x00418698 */
void resetScene5()
{
    g_4ab650 = g_4ab652 = g_4b0d52 = 0;
    g_4ab662 = g_4ab668 = g_4ab664 = 0;
    g_4ab64a = 0;
    g_4ab666 = 1;
    g_4ab67e = 0;
}

/* Closes scene 5: the party stays at the camp (all of it when leaving
   for the map or scene 1, else those present, counted off the town's
   population), and the book is tidied. */
/* @zoombi32 0x00418d40 */
void closeScene5()
{
    short saved;

    if (g_4ab660) {
        g_4ab660 = 0;
        saved = fn_46bee9(1);
        clearViews();
        if (!viewsLocked) {
            if (g_4a48e6 || g_4b0d50 == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                waitingParties()[2] = *party();
                party()->count = 0;
            } else {
                party()->unknown2 = 1;
                party()->unknown4 = 0;
                waitingParties()[2] = *party();
                party()->unknown2 = 0;
                party()->unknown4 = 1;
                *(short *)(g_4a4ba0 + 0x4c) -= fn_4572bf();
            }
            fn_41a024();
            fn_419e49(-1);
        }
        unloadSounds();
        fn_46c2db(&g_4a0ac4);
        fn_46c2db(&g_4a0ac0);
        fn_46bee9(saved);
        fn_46ca9c(&g_4ab658);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Scene 5's frame: leaves for the scene due (once sound 996 is done);
   else shows the drag cursor for the button (4-7) under the cursor, and
   keeps view g_4ab67c's script 6001 going while g_4ab668. */
/* @zoombi32 0x00418e62 */
void scene5Frame()
{
    Point where;
    short button;
    short i;
    View *view;

    if (g_4a0ce8 || !g_4ab660)
        return;
    g_4a0ce8 = 1;
    updateViews();
    if (g_4b0d52) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            g_4a0ce8 = 0;
            return;
        }
        if (viewsLocked || !g_4b755a || g_4b755c >= 1) {
            g_4b0d50 = g_4b0d52;
            g_4b0d52 = 0;
            fn_46be2e(0);
            closeScene5();
        }
    } else {
        button = 0;
        if (!g_4ab664 && !g_4b9684) {
            getCursorPosition(&where);
            for (i = 3; !button && i < 7; i++)
                if (ptInRect(&campButtons[i].rect, where))
                    button = i - 2;
        }
        setDragCursor(button);
        if (g_4ab668) {
            view = findView(g_4ab67c);
            if (!view->body.running) {
                setViewScript(view, 6001, 1);
                view->flags = 0x88000;
            }
        }
    }
    playAmbientSound();
    g_4a0ce8 = 0;
}

/* Scene 5's clicks: once a scene is due, leaves for it; 1 leaves for
   the map (scene 16) when g_4ab65c, else says a line; 3 leaves for scene
   1; 4-7 scroll the book while held. */
/* @zoombi32 0x00418fa7 */
void scene5Clicked(short which)
{
    Point where;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        closeScene5();
        return;
    }
    getCursorPosition(&where);
    switch (which) {
    case 1:
        if (g_4ab65c) {
            queueViewSound(996, 0);
            fn_4196b1(which, 1, 0, 1);
            waitForEventFor(0, 2, 0, 1);
            fn_4196b1(which, 0, 0, 1);
            markPlacedSnoids();
            sendSnoids(680, 316, 45);
            g_4b0d52 = 16;
        } else
            switch (randomBetween(1, 3)) {
            case 1:
                queueViewSound(20084, 0);
                break;
            case 2:
                queueViewSound(20085, 0);
                break;
            case 3:
                queueViewSound(20082, 0);
                break;
            }
        break;
    case 3:
        queueViewSound(999, 0);
        fn_4196b1(which, 1, 0, 1);
        waitForEventFor(0, 2, 0, 1);
        fn_4196b1(which, 0, 0, 1);
        g_4b0d50 = 1;
        closeScene5();
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        g_4ab64a = which;
        fn_4196b1(which, 1, 0, 1);
        do {
            g_4a0abc = which - 3;
            fn_41a11b(0, 0);
            mainLoopEvents();
        } while (isButtonStillDown(1));
        fn_41a11b(1, 0);
        g_4ab64a = 0;
        fn_4196b1(which, 0, 0, 1);
        break;
    }
}

/* @zoombi32 0x004196a8 */
long fn_4196a8(long)
{
    return 0;
}

/* Draws the camp's button `button` (1-7), lit or not, or with 0 a group
   of them (`group` 1: 1-3, 2: 4-7, else all); button 1 is out while
   g_4ab65c is clear, 4-7 lit for the one chosen (g_4ab64a). With `show`
   shows them (redrawing the dragged view over 4-7). */
/* @zoombi32 0x004196b1 */
void fn_4196b1(short button, short lit, short group, short show)
{
    short y;
    ShortRect unused; /* unused, like `color`: they only take stack space */
    ShortRect rect = g_4a0cea;
    Color color;
    short dragging = 0;
    short first;
    short last;
    short image;
    short x;

    if (!button) {
        switch (group) {
        case 1:
            first = 0;
            last = 3;
            break;
        case 2:
            first = 3;
            last = 7;
            break;
        default:
            first = 0;
            last = 7;
            break;
        }
        rect = campButtons[first].rect;
        unionRect(&rect, &campButtons[last - 1].rect);
    } else {
        first = button - 1;
        last = first + 1;
        rect = campButtons[button - 1].rect;
    }
    for (; first < last; first++) {
        x = campButtons[first].rect.left;
        y = campButtons[first].rect.top;
        image = 0;
        switch (first) {
        case 0:
            image = 1;
            if (!g_4ab65c) {
                lit = 0;
                image = 15;
            }
            break;
        case 2:
            image = 5;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            dragging = 1;
            image = (first - 3) * 2 + 7;
            lit = 0;
            if (g_4ab64a - 1 == first)
                lit = 1;
            break;
        }
        if (image) {
            if (lit)
                image++;
            drawImage(g_4a0ac4, image, x, y, 8, 17);
        }
    }
    if (show) {
        if (dragging)
            drawDragCursor(viewListEnd(0));
        showRect(&rect);
    }
}

/* The entry shown from row `row` on (25 of them, in 5 lines of 5) that
   `rect`'s corner is on, if `taken`; else the empty one `rect` covers
   most (by over 625 pixels); -1 if none. */
/* Not exact: the original keeps `column` in ebx (with `row`) and `n` in
   esi; here they are the other way round. */
/* @zoombi32 0x004198be */
short fn_4198be(short row, ShortRect rect, short taken)
{
    short found = -1;
    short i;
    short index;
    short count;
    short line;
    ShortRect cell;
    short best;
    Point where;
    short column;
    short n;
    short zoombini;
    short area;

    row %= g_4ab642;
    where.x = rect.left;
    where.y = rect.top;
    count = 25;
    index = row * 5;
    column = line = 0;
    best = 0;
    for (i = 0; i < count; i++, index++) {
        n = index % g_4ab644;
        zoombini = (short)g_4ab64c->entries[n].zoombini;
        if (taken && zoombini || !taken && !zoombini) {
            if (zoombini) {
                cell = g_4ab64c->entries[n].rect;
                if (ptInRect(&cell, where))
                    return n;
            } else {
                cell.left = cellX[line * 2 + 1] - 30;
                cell.top = cellY[line * 2 + 1][column] - 30;
                cell.right = cell.left + 60;
                cell.bottom = cell.top + 60;
                if (sectRect(&cell, &rect)) {
                    area = (cell.right - cell.left) * (cell.bottom - cell.top);
                    if (area > 625 && area > best) {
                        best = area;
                        found = n;
                    }
                }
            }
        }
        if (++column >= 5) {
            column = 0;
            line++;
        }
    }
    return found;
}

/* View draw callbacks: draw buttons 1-3, and 4-7. */
/* @zoombi32 0x0041983f */
void fn_41983f(View *)
{
    fn_4196b1(0, 0, 1, 0);
}

/* @zoombi32 0x00419853 */
void fn_419853(View *)
{
    fn_4196b1(0, 0, 2, 0);
}

/* A view update: redraws button 0 whenever g_4ab65c changes. */
/* @zoombi32 0x00419867 */
void fn_419867(View *, short region)
{
    if (g_4ab65c) {
        if (!g_4ab65e) {
            g_4ab65e = 1;
            unionRgnRect(region, &campButtons[0].rect);
        }
    } else if (g_4ab65e) {
        g_4ab65e = 0;
        unionRgnRect(region, &campButtons[0].rect);
    }
}

/* Counts entry `n` (0-624) in: g_4ab646 of them, g_4ab648 the highest;
   g_4ab644 is then that rounded up past a multiple of 5 (50-625), and
   g_4ab642 a fifth of it; g_4ab640 stays within 5 of the end. The first
   entry keeps g_4ab640 and the count. */
/* @zoombi32 0x00419e49 */
void fn_419e49(short n)
{
    if (n >= 0 && n < 625 && g_4ab646 < 625) {
        g_4ab646++;
        if (n > g_4ab648)
            g_4ab648 = n;
    }
    g_4ab644 = (g_4ab648 + 10) / 5 * 5;
    if (g_4ab644 > 625)
        g_4ab644 = 625;
    if (g_4ab644 < 50)
        g_4ab644 = 50;
    g_4ab642 = g_4ab644 / 5;
    if (g_4ab640 > g_4ab642 - 5)
        g_4ab640 = g_4ab642 - 5;
    g_4ab64c->row = g_4ab640;
    g_4ab64c->count = g_4ab646;
}

/* Draws the camp's book of waiting Zoombinis (a view draw callback): 25
   of them from row g_4ab640 on, in 5 lines of 5 (half a line lower, and
   one line more, while g_4a0abe), noting where each is drawn. */
/* Not exact: the original keeps `column` in ebx, `n` in esi and `line` in
   edi; here they get edi, ebx and esi (declaration order and `register`
   don't change that; fn_4198be has the same swap). */
/* @zoombi32 0x00419c3a */
void fn_419c3a(View *)
{
    short image;
    short count;
    short y;
    short i;
    short index;
    short dx;
    short dy;
    Snoid snoid;
    short n;
    short column;
    short line;
    short x;

    initSnoid(&snoid);
    g_4ab640 %= g_4ab642;
    count = 25;
    index = g_4ab640 * 5;
    column = line = 0;
    if (g_4a0abe) {
        image = 1;
        count += 5;
        dy = -3;
        dx = -1;
    } else {
        image = 3;
        dy = 0;
        dx = 0;
    }
    drawImage(g_4a0ac0, image, 140, 23, 0, 17);
    for (i = 0; i < count; i++, index++) {
        n = index % g_4ab644;
        if (g_4ab64c->entries[n].zoombini) {
            if (g_4a0abe) {
                x = cellX[line * 2];
                y = cellY[line * 2][column];
            } else {
                x = cellX[line * 2 + 1];
                y = cellY[line * 2 + 1][column];
            }
            snoid.body.clipped = 0;
            snoid.unknownC0 = -1;
            snoid.body.frame = 0;
            snoid.body.frameOffset = 2;
            snoid.zoombini = g_4ab64c->entries[n].zoombini;
            snoid.body.x = x;
            snoid.body.y = y;
            setSnoidFacing(&snoid, 0);
            layOutSnoid(&snoid, 0);
            g_4ab64c->entries[n].rect = snoid.body.bounds;
            drawSnoid(&snoid);
        }
        if (++column >= 5) {
            column = 0;
            line++;
        }
    }
    drawImage(g_4a0ac0, image + 1, dx + 141, dy + 28, 8, 17);
    drawImage(g_4a0ac0, 5, 101, 0, 8, 17);
}

/* The book's view update: when due, scrolls the book the way pressed
   (g_4a0abc: 1 up a page, 2 up, 3 down, 4 down a page), half a line at a
   time (g_4a0abe), making room at the start when at the top. */
/* @zoombi32 0x00419a59 */
void fn_419a59(View *view, short)
{
    short steps;

    if (clockTime() >= view->nextUpdate) {
        view->nextUpdate = clockTime() + view->interval;
        if (view->reset) {
            view->reset = 0;
            view->body.bounds = g_4a0be8;
        } else if (g_4a0abc) {
            view->changed = 1;
            steps = 1;
            switch (g_4a0abc) {
            case 1:
                steps += 4;
                if (!g_4a0abe && g_4ab640 - steps < 0)
                    steps = 0;
                /* falls through: 5 steps up */
            case 2:
                if (steps && !g_4ab640)
                    fn_419f3a();
                if (steps)
                    do {
                        if (!g_4a0abe && g_4ab640 > 0) {
                            g_4ab640--;
                            if (g_4ab640 < 0) {
                                g_4ab640 = 0;
                                steps = 1;
                            }
                            g_4a0abe = 1;
                        } else
                            g_4a0abe = 0;
                    } while (--steps);
                break;
            case 4:
                steps += 4;
                if (!g_4a0abe && g_4ab640 + steps > g_4ab642 - 5)
                    steps = 0;
                /* falls through: 5 steps down */
            case 3:
                if (steps)
                    do {
                        if (!g_4a0abe) {
                            if (g_4ab640 + 1 <= 120 && g_4ab640 < g_4ab642 - 5)
                                g_4a0abe = 1;
                        } else {
                            g_4a0abe = 0;
                            g_4ab640++;
                            if (g_4ab640 >= g_4ab642 - 5) {
                                g_4ab640 = g_4ab642 - 5;
                                if (g_4ab640 > 120)
                                    g_4ab640 = 120;
                                steps = 1;
                            }
                        }
                    } while (--steps);
                break;
            }
            if (!g_4a0abe)
                g_4a0abc = 0;
        }
    }
}

/* Makes room for a row at the start, when one is taken, there are under
   125 rows and the last row is free: moves every entry down a row. */
/* @zoombi32 0x00419f3a */
void fn_419f3a()
{
    short taken = 0;
    short free = 1;
    short i;

    for (i = 0; !taken && i < 5; i++)
        if (g_4ab64c->entries[i].zoombini)
            taken = 1;
    if (taken && g_4ab642 < 125) {
        for (i = 620; free && i < 625; i++)
            if (g_4ab64c->entries[i].zoombini)
                free = 0;
        if (free) {
            for (i = 619; i >= 0; i--) {
                g_4ab64c->entries[i + 5] = g_4ab64c->entries[i];
                g_4ab64c->entries[i].zoombini = 0;
            }
            g_4ab648 += 5;
            fn_419e49(-1);
            g_4ab640++;
            g_4ab64c->row = g_4ab640;
            fn_41a11b(0, 1);
        }
    }
}

/* The index of the last of 625 entries with a value, or 0. */
/* @zoombi32 0x00419f1a */
short fn_419f1a()
{
    for (short i = 0x270; i >= 0; i--)
        if (g_4ab64c->entries[i].zoombini)
            return i;
    return 0;
}

/* Has view g_4ab650 update at once. */
/* @zoombi32 0x0041a225 */
void fn_41a225()
{
    View *view = findView(g_4ab650);

    if (view)
        view->nextUpdate = 0;
}

/* Drops the whole rows of empty entries at the start. */
/* @zoombi32 0x0041a024 */
void fn_41a024()
{
    short searching = 1;
    short empty = -5;
    short i;

    for (i = 0; searching && i < 625; i++)
        if (!g_4ab64c->entries[i].zoombini)
            empty++;
        else
            searching = 0;
    if (empty >= 5) {
        empty = empty / 5 * 5;
        if (empty) {
            for (i = empty; i < 625; i++) {
                g_4ab64c->entries[i - empty] = g_4ab64c->entries[i];
                g_4ab64c->entries[i].zoombini = 0;
            }
            g_4ab648 -= empty;
            g_4ab640 -= empty / 5;
            if (g_4ab648 < 0)
                g_4ab648 = 0;
            if (g_4ab640 < 0)
                g_4ab640 = 0;
            g_4ab64c->row = g_4ab640;
        }
    }
}

/* Lights the scroll button pressed (g_4a0abc) if it can scroll that way,
   with a sound when that changes; `quiet` puts it out. */
/* @zoombi32 0x0041a11b */
void fn_41a11b(short quiet, short)
{
    short sound = 0;
    short lit;

    if (g_4a0abc >= 0) {
        lit = 0;
        switch (g_4a0abc) {
        case 1:
            if (g_4ab640 > 4)
                lit = 1;
            break;
        case 2:
            if (g_4ab640 > 0)
                lit = 1;
            break;
        case 3:
            if (g_4ab640 < g_4ab642 - 5 && g_4ab640 + 1 <= 120)
                lit = 1;
            break;
        case 4:
            if (g_4ab640 < g_4ab642 - 9 && g_4ab640 + 5 <= 120)
                lit = 1;
            break;
        }
        if (lit != g_4ab652) {
            g_4ab652 = lit;
            switch (g_4ab652) {
            case 0:
                sound = 2001;
                break;
            case 1:
                sound = 2000;
                break;
            }
        }
        if (quiet) {
            if (g_4ab652)
                sound = 2001;
            g_4ab652 = 0;
        }
        if (sound) {
            if (sound == 2001)
                stopSounds(2000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            queueViewSound(sound, 0);
        }
    }
}

/* Adds the party's Zoombinis after the last taken entry; if they don't
   fit there, into the free ones (taking the name and features of the
   party's Zoombini with the entry's number, as the original does).
   Returns whether they fitted after the last. */
/* @zoombi32 0x0041a23b */
short fn_41a23b()
{
    short added;
    short count;
    ShortRect empty = g_4a0d76;
    short found;
    short last;
    short i;
    short j;

    count = fn_4572bf();
    added = found = 0;
    for (i = 624; !found && i >= 0; i--)
        if (g_4ab64c->entries[i].zoombini) {
            found = 1;
            last = i + 1;
        }
    if (!found)
        last = 0;
    if (last + count <= 624) {
        for (i = 0; i < count; i++) {
            g_4ab64c->entries[last + i].zoombini = party()->travellers[i].zoombini;
            g_4ab64c->entries[last + i].rect = empty;
            for (j = 0; j < 10; j++)
                g_4ab64c->entries[last + i].name[j] = party()->travellers[i].name[j];
        }
        added = 1;
    } else
        for (last = 0, i = 0; last < count && i < 625; i++)
            if (!g_4ab64c->entries[i].zoombini) {
                g_4ab64c->entries[i].zoombini = party()->travellers[i].zoombini;
                g_4ab64c->entries[i].rect = empty;
                for (j = 0; j < 10; j++)
                    g_4ab64c->entries[i].name[j] = party()->travellers[i].name[j];
                last++;
            }
    return added;
}
