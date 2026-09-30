/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "bctwo.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
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

/* Opens scene 5, the camp: the book of the Zoombinis waiting there (kept
   at +0x3688 in the game's state), the Zoombinis back at the camp and the
   party (which joins the book when it doesn't carry on), and a line by
   the camp's hint. */
/* @zoombi32 0x004186dc */
void openScene5()
{
    short choices;
    short saved;
    short highest;
    Point places[16] = {{490, 372}, {458, 359}, {450, 384}, {412, 376}, {393, 398}, {365, 386},
                        {348, 405}, {321, 389}, {304, 410}, {278, 397}, {264, 417}, {234, 400},
                        {218, 420}, {197, 398}, {177, 418}, {152, 403}};
    short n; /* the loop, then the Zoombinis chosen, then the hint */
    short m; /* whether the party fitted, then the line to say */

    g_4ab660 = 0;
    resetScene5();
    saved = g_4b87fe;
    g_4b87fe = 0;
    g_4afb32 = 1;
    addSoundRange(20000, 29999, 1);
    addSoundRange(2000, 2099, 0);
    addSoundRange(6000, 6099, 1);
    g_4ab64c = (CampEntries *)(g_4a4ba0 + 0x3688);
    g_4ab640 = g_4ab64c->row;
    g_4ab646 = g_4ab64c->count;
    g_4ab648 = fn_419f1a();
    fn_419e49(-1);
    openGameFile(&g_4ab658, "bctwo.mhk");
    fn_46be2e(g_4ab658);
    loadPaths(1000);
    loadDragCursors(10000);
    loadTerrain(100);
    drawBackdrop(5000);
    loadFeatureGroup(6000, 0, 0);
    loadFeatureGroup(7000, 1, 0);
    loadScripts(6000, 14);
    addScripts(7000, 16, 0);
    fn_46c011(&g_4a0ac0, 8000, 0, 1);
    fn_46c011(&g_4a0ac4, 9000, 0, 1);
    g_4ab650 = addView(0xc000, fn_419c3a, fn_419a59, 0, 6, 0, 0, 0);
    addView(0x9000, fn_419853, 0, 0, 0, 0, 0, 0);
    addView(0x1000, fn_41983f, fn_419867, 0, 0, 0, 0, 0);
    for (n = 0; n < 16; n++)
        placedViews[n] = addView(0x108a000, drawCels, runViewScript, n + 7000, 7, &places[n], 0, 0);
    g_4ab66a[9] = addView(0x5188000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
    g_4ab66a[0] = addView(0x188000, drawCels, runViewScript, 6005, 6, 0, 0, 0);
    g_4ab66a[1] = addView(0x188000, drawCels, runViewScript, 6011, 6, 0, 0, 0);
    g_4ab66a[2] = addView(0x1188000, drawCels, runViewScript, 6010, 6, 0, 0, 0);
    g_4ab66a[3] = addView(0x4188000, drawCels, runViewScript, 6002, 6, 0, 0, 0);
    g_4ab66a[4] = addView(0x4180000, drawCels, runViewScript, 6004, 6, 0, 0, 0);
    g_4ab66a[5] = addView(0x1188000, drawCels, runViewScript, 6009, 6, 0, 0, 0);
    g_4ab66a[6] = addView(0x5188000, drawCels, runViewScript, 6006, 6, 0, 0, 0);
    g_4ab66a[7] = addView(0x5188000, drawCels, runViewScript, 6007, 6, 0, 0, 0);
    g_4ab66a[8] = addView(0x5188000, drawCels, runViewScript, 6008, 6, 0, 0, 0);
    fn_4148da(10, 236);
    setViewPlaces(16, places, 1);
    if (party()->count)
        makePartySnoids(0);
    n = countChosenSnoids();
    *(short *)(g_4a4ba0 + 0x4c) += n;
    *party() = waitingParties()[2];
    waitingParties()[2].count = 0;
    waitingParties()[2].unknown2 = 1;
    waitingParties()[2].unknown4 = 1;
    if (n) {
        if (!party()->unknown2 && fn_4572bf()) {
            highest = g_4ab648;
            m = fn_41a23b();
            g_4ab646 += fn_4572bf();
            g_4ab648 = fn_419f1a();
            fn_419e49(-1);
            if (m) {
                g_4ab640 = (highest + 1) / 5 % g_4ab642;
                fn_419e49(-1);
            }
            party()->unknown2 = 1;
        }
    } else
        g_4b7562 = 1;
    makePartySnoids(1);
    enterSnoids(-20);
    updateViews();
    if (n)
        staggerSnoids(45, 30);
    g_4ab67e = *(short *)(g_4a4ba0 + 0x48) >= 625
               && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c)
                          + waitingParties()[0].count
                      < 16;
    if (g_4ab67e) {
        short count = countChosenSnoids();

        g_4ab65c = count
                   && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c)
                              + waitingParties()[0].count
                          <= count;
        g_4ab65e = g_4ab65c;
    } else
        g_4ab65e = g_4ab65c = countChosenSnoids() >= 16;
    setGroupLists(campGroups, 2, (short)0xc000);
    highlightItemAt(1, 1);
    fn_4196b1(0, 0, 0, 0);
    showRect(&g_4aa7b8);
    fadeInViews();
    g_4ab660 = 1;
    m = 0;
    n = -1;
    if (g_4b0d4c) {
        n = campHint((short *)(g_4a4ba0 + 0x3e));
        g_4b0d4c = 0;
    }
    if (n == 2 && !*(short *)(g_4a4ba0 + 0x40) && *(short *)(g_4a4ba0 + 0x4c) <= 16) {
        n = 1;
        *(short *)(g_4a4ba0 + 0x3e) &= 0xcfff;
    }
    choices = 3;
    if (*(short *)(g_4a4ba0 + 0x3e) & 0x3000)
        choices = 4;
    switch (n) {
    case 0:
        switch (randomBetween(1, choices)) {
        case 1:
            m = 20084;
            break;
        case 2:
            m = 20085;
            break;
        case 3:
            m = 20082;
            break;
        case 4:
            m = 20083;
            break;
        }
        break;
    case 1:
        m = 20082;
        break;
    case 2:
    case 12:
        m = 20083;
        break;
    case 5:
        m = 20082;
        break;
    }
    resetViewClock();
    g_4b87fe = saved;
    if (m)
        queueViewSound(m, 0);
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

/* The camp's drags (event 1 pressed, 2 dragging): a Zoombini picked out
   of the book (taken from its entry) or one on the ground is dragged;
   dropped on an empty cell of the book it goes in it, and a Zoombini
   picked out of the book goes back if dropped nowhere useful (or with over
   32 about). g_4ab65c then says whether enough are chosen to leave.
   Clicking elsewhere starts the camp's thing there (g_4a0c58). */
/* @zoombi32 0x0041914d */
void fn_41914d(short event)
{
    Point where;
    ShortRect rect;
    ShortRect unused; /* only takes stack space */
    short slot;
    short added;
    short picked;
    short dropped;
    short place;
    Snoid *snoid;
    short moved;
    View *view;
    short entry;
    short i;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        closeScene5();
        return;
    }
    if (g_4ab662 && event != 2)
        return;
    getCursorPosition(&where);
    picked = 0;
    view = 0;
    if (event == 1 && g_4b755a <= 0) {
        view = viewAt(where, 1, 1);
        if (!view) {
            rect.left = rect.right = where.x;
            rect.top = rect.bottom = where.y;
            entry = fn_4198be(g_4ab640, rect, 1);
            if (entry >= 0) {
                if (g_4ab646 > 0)
                    g_4ab646--;
                initSnoid(&g_4ab680);
                g_4ab680.zoombini = g_4ab64c->entries[entry].zoombini;
                for (i = 0; i < 10; i++)
                    g_4ab680.name[i] = g_4ab64c->entries[entry].name[i];
                g_4ab680.body.x = where.x;
                g_4ab680.body.y = where.y;
                g_4ab64c->entries[entry].zoombini = 0;
                fn_41a225();
                added = addSnoidView(&g_4ab680, 0);
                if (added) {
                    view = findView(added);
                    g_4a0abc = -1;
                    picked = 1;
                    event = 2;
                }
            }
        } else
            event = 2;
    }
    switch (event) {
    case 1:
        break;
    case 2:
        if (!view && g_4b755a <= 0)
            view = viewAt(where, 1, 1);
        if (view) {
            i = 0;
            g_4ab654 = 0;
            g_4ab664 = 1;
            dropped = dragSnoid(view, where, 0, 0);
            g_4ab664 = 0;
            place = heldPlaceNumber();
            snoid = viewSnoid(view);
            moved = !(snoid->targetX == snoid->body.x && snoid->targetY == snoid->body.y);
            snoid->unknownF7 = place > 0;
            if (snoid->unknownF7)
                snoid->unknownF8 = 1;
            rect = view->body.bounds;
            if (sectRect(&rect, &g_4a0be8)) {
                rect = view->body.bounds;
                slot = fn_4198be(g_4ab640, rect, 0);
                if (slot >= 0) {
                    fn_419e49(slot);
                    g_4ab64c->entries[slot].zoombini = viewSnoid(view)->zoombini;
                    for (i = 0; i < 10; i++)
                        g_4ab64c->entries[slot].name[i] = viewSnoid(view)->name[i];
                    deleteView(view->id);
                    fn_41a225();
                    g_4a0abc = -1;
                    picked = 0;
                    i = 1;
                }
            }
            if (picked) {
                short back = countSnoidViews() > 32;

                if (!back && !place && moved)
                    back = 1;
                if (back) {
                    g_4ab64c->entries[entry].zoombini = g_4ab680.zoombini;
                    for (i = 0; i < 10; i++)
                        g_4ab64c->entries[entry].name[i] = g_4ab680.name[i];
                    removeView(added, 1);
                }
                g_4a0abc = -1;
            } else if (dropped && !place && moved && !i) {
                claimPlacedView(dropped, view->id);
                snoid->unknownF7 = 1;
                snoid->unknownF8 = 1;
            }
            if (g_4ab67e) {
                short chosen = countChosenSnoids();

                g_4ab65c = chosen
                           && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c)
                                      + waitingParties()[0].count
                                  <= chosen;
            } else
                g_4ab65c = countChosenSnoids() >= 16;
        } else
            for (i = 0; i < 10; i++)
                if (ptInRect(&g_4a0c58[i], where)) {
                    view = findView(g_4ab66a[i]);
                    if (view && !view->body.running) {
                        switch (i) {
                        case 3: {
                            short script;

                            if (g_4ab666) {
                                g_4ab666 = 0;
                                script = 6002;
                            } else {
                                g_4ab666 = 1;
                                script = 6003;
                            }
                            setViewScript(view, script, 1);
                            break;
                        }
                        case 1:
                            if (view->kind == 6013)
                                setViewScript(view, 6011, 1);
                            else
                                setViewScript(view, view->kind + 1, 1);
                            break;
                        case 9:
                            if (!g_4ab668) {
                                setViewScript(view, 0, 1);
                                g_4ab668 = 1;
                            }
                            break;
                        default:
                            setViewScript(view, 0, 1);
                            break;
                        }
                        i = 10;
                        loadViewSounds(view->id, 1);
                    }
                }
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
