/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "bctwo.h"
#include "features.h"
#include "graphics.h"
#include "sound.h"
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
