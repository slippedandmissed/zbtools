/*
 * module_4124a4 (0x4124a4-0x413c24): no strings; uses isMousePresent and toUpperAscii (input?)
 */

#include <string.h>
#include "zoombinis.h"

/* Installs the lists of groups to move the focus over, and numbers their
   items. */
/* @zoombi32 0x004124a4 */
void setGroupLists(GroupList *lists, short count, unsigned short flags)
{
    g_4a01ac = lists;
    g_4aa48c = count;
    g_4aa48a = flags;
    numberAllItems();
}

/* Hovers over the item a handler accepts `value` for (entering it), or
   leaves the one entered; the item, if any. */
/* @zoombi32 0x00412537 */
InputItem *hoverItemByHandler(long value)
{
    InputState saved;
    InputItem *item;

    fn_413afd(&saved, 1);
    g_4aa4c0 = 0;
    if (focusItemByHandler(value)) {
        enterFocusedItem();
        item = g_4aa498;
    } else {
        leaveEnteredItem();
        item = 0;
    }
    fn_413a4e(&saved, 1);
    return item;
}

/* fn_4125d3 for an item, keeping the state. */
/* @zoombi32 0x00412587 */
short fn_412587(InputItem *item, short on)
{
    InputState saved;
    short result;

    fn_413afd(&saved, 1);
    g_4aa4c0 = 0;
    if (focusItem(item))
        result = fn_4125d3(on);
    else
        result = 0;
    fn_413a4e(&saved, 1);
    return result;
}

/*
 * Sets the focused item on or off, following its group's rules: flag 8 of
 * the group gives the on-state and flag 0x20 the value that make the list
 * exclusive here (switchOffOthers, before or after depending on the group's
 * kind); the list's `changed` callback hears of it. Whether it did.
 */
/* @zoombi32 0x00412722 */
short fn_412722(short on, short value)
{
    short wasOn, onState, notValue, early;

    wasOn = (g_4aa498->flags & 4) == 4;
    notValue = (g_4aa494->flags & 0x20) != 0x20;
    onState = (g_4aa494->flags & 8) == 8;
    early = (g_4aa494->flags & 0xe000) == 0x2000 || (g_4aa494->flags & 0xe000) == 0x8000;

    if (on == onState && notValue == value && early)
        switchOffOthers();
    if (on != wasOn)
        toggleFocusedItem();
    if (on == onState && notValue == value) {
        if (!early)
            switchOffOthers();
        if (g_4aa490->changed)
            g_4aa490->changed(g_4aa49c.c.x);
        return 1;
    }
    return 0;
}

/* Passes `value` to the current handlers' slot 0x28, or else to the engine;
   the answer. */
/* @zoombi32 0x0041280d */
short fn_41280d(long value)
{
    if (!g_4aa494->handlers->handler28)
        return fn_480b80(g_4aa498, value);
    else
        return g_4aa494->handlers->handler28(value, g_4aa498);
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
   (its groups' sizes and its length) instead of taken. */
/* @zoombi32 0x004128c6 */
short fn_4128c6(GroupList *list)
{
    short i;

    if (!list)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (list->flags & 1 || list->flags & 2) {
            for (i = 0; i < list->count; i++)
                g_4aa49c.b.x += list->groups[i].count;
            g_4aa49c.a.y += list->count;
            return 0;
        }
        break;
    case 1:
        if (list->flags & 1) {
            for (i = 0; i < list->count; i++)
                g_4aa49c.b.x += list->groups[i].count;
            g_4aa49c.a.y += list->count;
            return 0;
        }
        break;
    }
    return 1;
}

/* The same test as fn_4128c6, for one group. */
/* @zoombi32 0x0041295f */
short fn_41295f(Group *group)
{
    if (!group)
        return 0;
    if (g_4aa4c2 && !group->handlers->enter)
        return 0;
    switch (g_4aa4c0) {
    case 0:
        if (group->flags & 1 || group->flags & 2) {
            g_4aa49c.b.x += group->count;
            g_4aa49c.c.x += group->count;
            return 0;
        }
        break;
    case 1:
        if (group->flags & 1) {
            g_4aa49c.b.x += group->count;
            g_4aa49c.c.x += group->count;
            return 0;
        }
        break;
    }
    return 1;
}

/* Moves the highlight to the focused item (redrawing the one highlighted
   before), and the mouse with it if the lists ask for that. */
/* @zoombi32 0x004129e1 */
void highlightFocus()
{
    InputState saved;

    if (fn_4133a4()) {
        if (g_4aa498 != highlightedItem) {
            fn_413afd(&saved, 0);
            g_4aa4c0 = 1;
            if (focusItem(highlightedItem)) {
                highlightedItem = 0;
                if (!fn_41336f())
                    fn_412bb3(g_4aa498->flags & 4);
            }
            fn_413a4e(&saved, 0);
        }
        highlightedItem = g_4aa498;
        if (g_4a01b0)
            moveMouseToFocus();
    }
}

/* Switches the focused item on or off (flag 4), with its sound. */
/* @zoombi32 0x00412a6e */
void toggleFocusedItem()
{
    short off = (g_4aa498->flags & 4) != 4;

    fn_412bb3(off);
    g_4aa498->flags ^= 4;
    if (g_4aa494->sounds)
        fn_41200c(g_4aa494->sounds[g_4aa49c.c.y * 2 - off - 1], RESOURCE_TYPE(0, 'S', 'N', 'D'), 0,
                  0, 1);
}

/* In a list whose items are exclusive (flag 4), switches off every item that's
   on apart from the focused one. */
/* @zoombi32 0x00412ace */
void switchOffOthers()
{
    InputState saved;

    if (g_4aa490->flags & 4) {
        fn_413afd(&saved, 0);
        g_4aa4ac = 4;
        g_4aa4be = 4;
        g_4aa49c.b.y = 1;
        g_4aa49c.c.y = 0;
        while (fn_413693(g_4aa490, g_4aa49c.b.y - 1, g_4aa49c.c.y))
            if (g_4aa498 != saved.item)
                toggleFocusedItem();
        fn_413a4e(&saved, 0);
    }
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
   something is switched off (flag 2), else one of a pair, for highlightedItem or
   another item, and `alternative` or not. */
/* @zoombi32 0x00412bb3 */
void fn_412bb3(short alternative)
{
    if (g_4aa494->flags & 0x80 && g_4aa498 != enteredItem)
        return;
    if (g_4aa48a & 2 || g_4aa490->flags & 2 || g_4aa494->flags & 2 || g_4aa498->flags & 2) {
        fn_412cdf();
        return;
    }
    if (g_4aa498 == highlightedItem) {
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
    if (g_4aa494->flags & 0x80 && g_4aa498 != enteredItem)
        return;
    if (g_4aa48a & 2 || g_4aa490->flags & 2 || g_4aa494->flags & 2 || g_4aa498->flags & 2) {
        fn_412d57();
        return;
    }
    if (g_4aa498 == highlightedItem) {
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

/* Enters the focused item (calls its group's enter handler), leaving the
   one entered before. */
/* @zoombi32 0x00412d67 */
void enterFocusedItem()
{
    if (enteredItem) {
        if (enteredItem == g_4aa498)
            return;
        leaveEnteredItem();
    }
    enteredItem = g_4aa498;
    fn_412b4d(g_4aa494->handlers->enter);
}

/* Leaves the item entered last (calls its group's leave handler). */
/* @zoombi32 0x00412d9c */
void leaveEnteredItem()
{
    InputState saved;

    if (enteredItem) {
        fn_413afd(&saved, 0);
        g_4aa4c0 = 1;
        if (focusItem(enteredItem))
            fn_412b4d(g_4aa494->handlers->leave);
        enteredItem = 0;
        fn_413a4e(&saved, 0);
    }
}

/* Where an item is, keeping the state. */
/* @zoombi32 0x00412df4 */
void getItemPosition(InputItem *item, Cursor *where)
{
    InputState saved;

    fn_413afd(&saved, 1);
    g_4aa4c0 = 2;
    focusItem(item);
    *where = g_4aa49c;
    fn_413a4e(&saved, 1);
}

/* The item at a position (from 1), if any, keeping the state. */
/* @zoombi32 0x00412e44 */
InputItem *itemAt(short x, short y)
{
    InputState saved;
    InputItem *item;

    if (x <= 0 || y <= 0)
        return 0;
    fn_413afd(&saved, 1);
    g_4aa4c0 = 2;
    if (focusItemAt(x, y))
        item = g_4aa498;
    else
        item = 0;
    fn_413a4e(&saved, 1);
    return item;
}

/* Moves the focus to the next or previous item and highlights it. */
/* @zoombi32 0x00412fb5 */
void stepFocus(short direction)
{
    g_4aa4ac = 5;
    if (moveFocus(direction)) {
        highlightFocus();
        if (!fn_41336f())
            fn_412bb3(g_4aa498->flags & 4);
    }
}

/*
 * Moves the focus to the next item (direction > 0) or the previous one,
 * within the current list if it has flag 8, else across all lists, wrapping
 * round once; whether there was one.
 */
/* @zoombi32 0x00412ff6 */
short moveFocus(short direction)
{
    short tries = 2;
    short list = g_4aa49c.a.x - 1;
    short group = g_4aa49c.b.y - 1;
    short item;

    if (direction > 0)
        item = g_4aa49c.c.y;
    else
        item = g_4aa49c.c.y - 2;
    do {
        if (direction > 0) {
            if (g_4aa490->flags & 8) {
                if (fn_413693(g_4aa490, group, item))
                    return 1;
                group = item = 0;
            } else {
                if (fn_41348b(list, group, item))
                    return 1;
                list = group = item = 0;
            }
        } else {
            if (g_4aa490->flags & 8) {
                if (fn_413755(g_4aa490, group, item))
                    return 1;
                group = g_4aa490->count - 1;
                item = g_4aa490->groups[group].count - 1;
            } else {
                if (fn_41357a(list, group, item))
                    return 1;
                list = g_4aa48c - 1;
                group = g_4a01ac[list].count - 1;
                item = g_4a01ac[list].groups[group].count - 1;
            }
        }
        tries--;
    } while (tries);
    return 0;
}

/* Calls the handler of the item at a position (from 1), keeping the state. */
/* @zoombi32 0x00413237 */
void activateItemAt(short x, short y)
{
    InputState saved;

    if (x > 0 && y > 0) {
        fn_413afd(&saved, 1);
        g_4aa4c0 = 1;
        if (focusItemAt(x, y))
            fn_412bb3(g_4aa498->flags & 4);
        fn_413a4e(&saved, 1);
    }
}

/* Calls every item's handlers (search 6, in mode 1), keeping the state. */
/* @zoombi32 0x00413295 */
void visitAllItems()
{
    InputState saved;

    fn_413afd(&saved, 1);
    g_4aa4ac = 6;
    g_4aa4c0 = 1;
    fn_41348b(0, 0, 0);
    fn_413a4e(&saved, 1);
}

/* Moves the mouse, if fn_41336f says it follows the focus. */
/* @zoombi32 0x004132d2 */
void moveMouseTo(short x, short y)
{
    if (fn_41336f())
        setCursorPosition(x, y);
}

/* Moves the mouse to the focused item (its centre or its hotspot), if it
   follows the focus. */
/* @zoombi32 0x00413312 */
void moveMouseToFocus()
{
    if (fn_41336f()) {
        if (g_4aa494->flags & 0x40) {
            InputItem *item = g_4aa498;
            setCursorPosition((item->bounds.right + item->bounds.left) / 2,
                              (item->bounds.bottom + item->bounds.top) / 2);
        } else
            setCursorPosition(g_4aa498->hotspot.x, g_4aa498->hotspot.y);
    }
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

/* Moves the focus to the first item whose group's handler accepts `value`. */
/* @zoombi32 0x004133d9 */
short focusItemByHandler(long value)
{
    g_4aa4ac = 0;
    g_4aa4b0 = value;
    return fn_41348b(0, 0, 0);
}

/* Moves the focus to the first item with a key (either case). */
/* @zoombi32 0x004133fc */
short focusItemByKey(short key)
{
    g_4aa4ac = 3;
    g_4aa4bc = toUpperAscii(key);
    return fn_41348b(0, 0, 0);
}

/* Moves the focus to the item at a position. */
/* @zoombi32 0x00413427 */
short focusItemAt(short x, short y)
{
    g_4aa4ac = 2;
    g_4aa4b8 = x;
    g_4aa4ba = y;
    return fn_41348b(0, 0, 0);
}

/* Moves the focus to an item. */
/* @zoombi32 0x00413456 */
short focusItem(InputItem *item)
{
    if (!fn_412844(item))
        return 0;
    g_4aa4ac = 1;
    g_4aa4b4 = item;
    return fn_41348b(0, 0, 0);
}

/* Searches all lists (g_4a01ac) from list `list`, group `group`, item `start`,
   onwards (fn_413693); whether an item was found. */
/* @zoombi32 0x0041348b */
short fn_41348b(short list, short group, short start)
{
    short found, i, j;
    GroupList *current;

    memset(&g_4aa49c, 0, sizeof g_4aa49c);
    if (!fn_412884())
        return 0;
    g_4aa49c.a.y = group;
    g_4aa49c.b.x = start;
    for (i = 0; i < list; i++) {
        g_4aa49c.a.y += g_4a01ac[i].count;
        for (j = 0; j < g_4a01ac[i].count; j++)
            g_4aa49c.b.x += g_4a01ac[i].groups[j].count;
    }
    g_4aa49c.a.x = list;
    found = 0;
    for (current = &g_4a01ac[list]; g_4aa49c.a.x < g_4aa48c && !found; current++) {
        found = fn_413693(current, group, start);
        group = start = 0;
        g_4aa49c.a.x++;
    }
    if (!found)
        g_4aa49c.a.x = g_4aa49c.a.y = g_4aa49c.b.x = 0;
    return found;
}

/* The same as fn_41348b, backwards (each earlier list from its last item). */
/* @zoombi32 0x0041357a */
short fn_41357a(short list, short group, short start)
{
    short found, i, j;
    GroupList *current;

    memset(&g_4aa49c, 0, sizeof g_4aa49c);
    if (!fn_412884())
        return 0;
    g_4aa49c.a.y = group;
    g_4aa49c.b.x = start;
    for (i = 0; i < list; i++) {
        g_4aa49c.a.y += g_4a01ac[i].count;
        for (j = 0; j < g_4a01ac[i].count; j++)
            g_4aa49c.b.x += g_4a01ac[i].groups[j].count;
    }
    g_4aa49c.a.x = list;
    found = 0;
    for (current = &g_4a01ac[list]; g_4aa49c.a.x >= 0 && !found; current--) {
        found = fn_413755(current, group, start);
        group = g_4a01ac[g_4aa49c.a.x - 1].count - 1;
        start = g_4a01ac[g_4aa49c.a.x - 1].groups[group].count - 1;
        g_4aa49c.a.x--;
    }
    g_4aa49c.a.x++;
    g_4aa49c.a.y++;
    g_4aa49c.b.x++;
    if (!found)
        g_4aa49c.a.x = g_4aa49c.a.y = g_4aa49c.b.x = 0;
    return found;
}

/* Searches a list's groups from group `first`, item `start`, onwards
   (fn_41382a), moving the cursor along; whether an item was found. */
/* @zoombi32 0x00413693 */
short fn_413693(GroupList *list, short first, short start)
{
    short found, i;
    Group *group;

    g_4aa49c.b.y = g_4aa49c.c.x = g_4aa49c.c.y = 0;
    if (!fn_4128c6(list))
        return 0;
    g_4aa490 = list;
    g_4aa49c.c.x = start;
    for (i = 0; i < first; i++) {
        g_4aa49c.b.x += list->groups[i].count;
        g_4aa49c.c.x += list->groups[i].count;
    }
    g_4aa49c.b.y = first;
    found = 0;
    for (group = &list->groups[first]; g_4aa49c.b.y < list->count && !found; group++) {
        found = fn_41382a(group, start);
        start = 0;
        g_4aa49c.b.y++;
        g_4aa49c.a.y++;
    }
    if (!found)
        g_4aa49c.b.y = g_4aa49c.c.x = 0;
    return found;
}

/* The same as fn_413693, backwards (each earlier group from its last item). */
/* @zoombi32 0x00413755 */
short fn_413755(GroupList *list, short first, short start)
{
    short found, i;
    Group *group;

    g_4aa49c.b.y = g_4aa49c.c.x = g_4aa49c.c.y = 0;
    if (!fn_4128c6(list))
        return 0;
    g_4aa490 = list;
    g_4aa49c.c.x = start;
    for (i = 0; i < first; i++) {
        g_4aa49c.b.x += list->groups[i].count;
        g_4aa49c.c.x += list->groups[i].count;
    }
    g_4aa49c.b.y = first;
    found = 0;
    for (group = &list->groups[first]; g_4aa49c.b.y >= 0 && !found; group--) {
        found = fn_4138a2(group, start);
        start = list->groups[g_4aa49c.b.y - 1].count - 1;
        g_4aa49c.b.y--;
        g_4aa49c.a.y--;
    }
    g_4aa49c.b.y++;
    g_4aa49c.c.x++;
    if (!found)
        g_4aa49c.b.y = g_4aa49c.c.x = 0;
    return found;
}

/* Searches a group's items from `start` onwards with fn_41391d, moving the
   cursor along; whether one was found (the cursor's index is then its). */
/* @zoombi32 0x0041382a */
short fn_41382a(Group *group, short start)
{
    short found;
    InputItem *item;

    g_4aa49c.c.y = 0;
    if (!fn_41295f(group))
        return 0;
    g_4aa494 = group;
    g_4aa49c.c.y = start;
    found = 0;
    for (item = &group->items[start]; g_4aa49c.c.y < group->count && !found; item++) {
        found = fn_41391d(item);
        g_4aa49c.c.y++;
        g_4aa49c.c.x++;
        g_4aa49c.b.x++;
    }
    if (!found)
        g_4aa49c.c.y = 0;
    return found;
}

/* The same as fn_41382a, backwards from `start`. */
/* @zoombi32 0x004138a2 */
short fn_4138a2(Group *group, short start)
{
    short found;
    InputItem *item;

    g_4aa49c.c.y = 0;
    if (!fn_41295f(group))
        return 0;
    g_4aa494 = group;
    g_4aa49c.c.y = start;
    found = 0;
    for (item = group->items + start; g_4aa49c.c.y >= 0 && !found; item--) {
        found = fn_41391d(item);
        g_4aa49c.c.y--;
        g_4aa49c.c.x--;
        g_4aa49c.b.x--;
    }
    g_4aa49c.c.y++;
    if (!found)
        g_4aa49c.c.y = 0;
    return found;
}

/*
 * Whether an item is what's being looked for (g_4aa4ac): 0 whatever the
 * group's handler says, 1 a particular item, 2 the one past the cursor, 3 the
 * one with a key, 4 one with some flags, 5 any; 6 and 7 visit them (calling
 * their handlers, or giving them their places). Makes it the current item.
 */
/* @zoombi32 0x0041391d */
short fn_41391d(InputItem *item)
{
    short found;

    if (!fn_412844(item))
        return 0;
    g_4aa498 = item;
    switch (g_4aa4ac) {
    case 0:
        found = fn_41280d(g_4aa4b0);
        break;
    case 1:
        found = item == g_4aa4b4;
        break;
    case 2:
        found = g_4aa4b8 == g_4aa49c.a.x + 1 && g_4aa4ba == g_4aa49c.c.x + 1;
        break;
    case 3:
        found = toUpperAscii(item->key) == g_4aa4bc;
        break;
    case 4:
        found = (item->flags & g_4aa4be) != 0;
        break;
    case 5:
        found = 1;
        break;
    case 6:
        fn_412c3d();
        found = 0;
        break;
    case 7:
        item->cursor = g_4aa49c;
        item->cursor.a.x++;
        item->cursor.a.y++;
        item->cursor.b.x++;
        item->cursor.c.x++;
        item->cursor.b.y++;
        item->cursor.c.y++;
        found = 0;
        break;
    }
    return found;
}

/* Loads the input state (the part from +0x18 only with `all`). */
/* @zoombi32 0x00413a4e */
void fn_413a4e(InputState *state, short all)
{
    g_4aa490 = state->list;
    g_4aa494 = state->group;
    g_4aa498 = state->item;
    g_4aa49c.a = state->cursorA;
    g_4aa49c.b = state->cursorB;
    g_4aa49c.c = state->cursorC;
    if (all) {
        g_4aa4ac = state->search;
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
    state->group = g_4aa494;
    state->item = g_4aa498;
    state->cursorA = g_4aa49c.a;
    state->cursorB = g_4aa49c.b;
    state->cursorC = g_4aa49c.c;
    if (all) {
        state->search = g_4aa4ac;
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

/* Gives every item its position (search 7, in mode 2), keeping the state. */
/* @zoombi32 0x00413be4 */
void numberAllItems()
{
    InputState saved;

    fn_413afd(&saved, 1);
    g_4aa4ac = 7;
    g_4aa4c0 = 2;
    fn_41348b(0, 0, 0);
    fn_413a4e(&saved, 1);
}
