/*
 * module_4124a4 (0x4124a4-0x413c24): no strings; uses isMousePresent and toUpperAscii (input?)
 */

#include "zoombinis.h"

/* Passes `value` to the current handlers' slot 0x28, or else to the engine. */
/* @zoombi32 0x0041280d */
void fn_41280d(long value)
{
    if (!g_4aa494->handlers->handler28)
        fn_480b80(g_4aa498, value);
    else
        g_4aa494->handlers->handler28(value, g_4aa498);
}

/* Whether an item is available in the current mode (g_4aa4c0): in mode 0,
   unless flag 1 or 2 is set; in mode 1, unless flag 1 is. */
/* @zoombi32 0x00412844 */
short fn_412844(InputItem *item)
{
    if (!item)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (item->flags & 1 || item->flags & 2)
            return 0;
        break;
    case 1:
        if (item->flags & 1)
            return 0;
        break;
    }
    return 1;
}

/* The same test as fn_412844, for the flags in g_4aa48a. */
/* @zoombi32 0x00412884 */
short fn_412884()
{
    if (!g_4a01ac)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (g_4aa48a & 1 || g_4aa48a & 2)
            return 0;
        break;
    case 1:
        if (g_4aa48a & 1)
            return 0;
        break;
    }
    return 1;
}

/* In mode 0 a list with flag 1 or 2, in mode 1 one with flag 1, is counted
   (its entries' sizes and its length) instead of taken. */
/* @zoombi32 0x004128c6 */
short fn_4128c6(ItemList *list)
{
    short i;

    if (!list)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (list->flags & 1 || list->flags & 2) {
            for (i = 0; i < list->count; i++)
                g_4aa4a0.x += list->entries[i].size;
            g_4aa49c.y += list->count;
            return 0;
        }
        break;
    case 1:
        if (list->flags & 1) {
            for (i = 0; i < list->count; i++)
                g_4aa4a0.x += list->entries[i].size;
            g_4aa49c.y += list->count;
            return 0;
        }
        break;
    }
    return 1;
}

/* The same test as fn_4128c6, for one entry. */
/* @zoombi32 0x0041295f */
short fn_41295f(ListEntry *entry)
{
    if (!entry)
        return 0;
    if (g_4aa4c2 && !entry->owner->unknown2c)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (entry->flags & 1 || entry->flags & 2) {
            g_4aa4a0.x += entry->size;
            g_4aa4a4.x += entry->size;
            return 0;
        }
        break;
    case 1:
        if (entry->flags & 1) {
            g_4aa4a0.x += entry->size;
            g_4aa4a4.x += entry->size;
            return 0;
        }
        break;
    }
    return 1;
}

/* Calls callback with g_4aa498 if there is one; returns whether it did. */
/* @zoombi32 0x00412b4d */
short fn_412b4d(void (*callback)(InputItem *item))
{
    if (!callback)
        return 0;
    callback(g_4aa498);
    return 1;
}

/* Calls the handlerC handler, or else (fn_412cc0) its default. */
/* @zoombi32 0x00412b6b */
void fn_412b6b()
{
    if (fn_41336f() || !fn_412b4d(g_4aa494->handlers->handlerC))
        fn_412cc0();
}

/* Calls the handler8 handler, or else (fn_412cd0) its default. */
/* @zoombi32 0x00412b8f */
void fn_412b8f()
{
    if (fn_41336f() || !fn_412b4d(g_4aa494->handlers->handler8))
        fn_412cd0();
}

/* Calls the handler for the current item: the default (fn_412cdf) if
   something is switched off (flag 2), else one of a pair, for g_4aa484 or
   another item, and `alternative` or not. */
/* @zoombi32 0x00412bb3 */
void fn_412bb3(short alternative)
{
    if (g_4aa494->flags & 0x80 && g_4aa498 != g_4aa4a8)
        return;
    if (g_4aa48a & 2 || g_4aa490->flags & 2 || g_4aa494->flags & 2 || g_4aa498->flags & 2) {
        fn_412cdf();
        return;
    }
    if (g_4aa498 == g_4aa484) {
        if (alternative)
            fn_412b6b();
        else
            fn_412b8f();
    } else if (alternative)
        fn_412cc0();
    else
        fn_412cd0();
}

/* The same choice as fn_412bb3, for the other set of handlers, by the
   item's flag 4. */
/* @zoombi32 0x00412c3d */
void fn_412c3d()
{
    if (g_4aa494->flags & 0x80 && g_4aa498 != g_4aa4a8)
        return;
    if (g_4aa48a & 2 || g_4aa490->flags & 2 || g_4aa494->flags & 2 || g_4aa498->flags & 2) {
        fn_412d57();
        return;
    }
    if (g_4aa498 == g_4aa484) {
        if (g_4aa498->flags & 4)
            fn_412cef();
        else
            fn_412d13();
    } else if (g_4aa498->flags & 4)
        fn_412d37();
    else
        fn_412d47();
}

/* @zoombi32 0x00412cc0 */
void fn_412cc0()
{
    fn_412b4d(g_4aa494->handlers->handler4);
}

/* @zoombi32 0x00412cd0 */
void fn_412cd0()
{
    fn_412b4d(g_4aa494->handlers->handler0);
}

/* @zoombi32 0x00412cdf */
void fn_412cdf()
{
    fn_412b4d(g_4aa494->handlers->handler10);
}

/* Calls the handler20 handler, or else (fn_412d37) its default. */
/* @zoombi32 0x00412cef */
void fn_412cef()
{
    if (fn_41336f() || !fn_412b4d(g_4aa494->handlers->handler20))
        fn_412d37();
}

/* Calls the handler1C handler, or else (fn_412d47) its default. */
/* @zoombi32 0x00412d13 */
void fn_412d13()
{
    if (fn_41336f() || !fn_412b4d(g_4aa494->handlers->handler1C))
        fn_412d47();
}

/* @zoombi32 0x00412d37 */
void fn_412d37()
{
    fn_412b4d(g_4aa494->handlers->handler18);
}

/* @zoombi32 0x00412d47 */
void fn_412d47()
{
    fn_412b4d(g_4aa494->handlers->handler14);
}

/* @zoombi32 0x00412d57 */
void fn_412d57()
{
    fn_412b4d(g_4aa494->handlers->handler24);
}

/* Flag 0x80 of g_4aa48b applies with a mouse, 0x40 without one. */
/* @zoombi32 0x0041336f */
short fn_41336f()
{
    short noMouse = !(unsigned short)isMousePresent();
    return g_4aa48b & 0x80 && !noMouse || g_4aa48b & 0x40 && noMouse;
}

/* Flag 0x20 of g_4aa48b applies with a mouse, 0x10 without one. */
/* @zoombi32 0x004133a4 */
short fn_4133a4()
{
    short noMouse = !(unsigned short)isMousePresent();
    return g_4aa48b & 0x20 && !noMouse || g_4aa48b & 0x10 && noMouse;
}

/* Loads the input state (the part from +0x18 only with `all`). */
/* @zoombi32 0x00413a4e */
void fn_413a4e(InputState *state, short all)
{
    g_4aa490 = state->list;
    g_4aa494 = state->handlers;
    g_4aa498 = state->item;
    g_4aa49c = state->unknownC;
    g_4aa4a0 = state->unknown10;
    g_4aa4a4 = state->unknown14;
    if (all) {
        g_4aa4ac = state->unknown18;
        g_4aa4b0 = state->unknown1A;
        g_4aa4b4 = state->unknown1E;
        g_4aa4b8 = state->unknown22;
        g_4aa4ba = state->unknown24;
        g_4aa4bc = state->unknown26;
        g_4aa4be = state->unknown28;
        g_4aa4c0 = state->mode;
        g_4a01b0 = state->unknown2C;
        g_4aa4c2 = state->unknown2E;
    }
}

/* Saves the input state (the part from +0x18 only with `all`). */
/* @zoombi32 0x00413afd */
void fn_413afd(InputState *state, short all)
{
    state->list = g_4aa490;
    state->handlers = g_4aa494;
    state->item = g_4aa498;
    state->unknownC = g_4aa49c;
    state->unknown10 = g_4aa4a0;
    state->unknown14 = g_4aa4a4;
    if (all) {
        state->unknown18 = g_4aa4ac;
        state->unknown1A = g_4aa4b0;
        state->unknown1E = g_4aa4b4;
        state->unknown22 = g_4aa4b8;
        state->unknown24 = g_4aa4ba;
        state->unknown26 = g_4aa4bc;
        state->unknown28 = g_4aa4be;
        state->mode = g_4aa4c0;
        state->unknown2C = g_4a01b0;
        state->unknown2E = g_4aa4c2;
    }
}

/* Where the mouse is, also passed to the hook fn_413bcf set, if any. */
/* @zoombi32 0x00413bad */
void fn_413bad(Point *where)
{
    getMousePosition(where);
    if (g_4aa4c4)
        g_4aa4c4(where);
}

/* @zoombi32 0x00413bcf */
void fn_413bcf(void (*hook)(Point *where))
{
    g_4aa4c4 = hook;
}
