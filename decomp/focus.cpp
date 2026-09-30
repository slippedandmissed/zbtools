/*
 * focus (0x4124a4-0x413c24): keyboard and mouse focus for on-screen controls.
 * Items (with bounds, a hotspot and a key) are arranged in groups, each with
 * its handlers (enter, leave, press, hit test, ...), and groups in lists; the
 * focus moves by Tab, keys, or the mouse, and presses are tracked like the
 * Mac's TrackControl. Its state is in the globals from 0x4aa490 (InputState).
 */

#include <string.h>
#include "zoombinis.h"
#include "debug.h"
#include "events.h"
#include "focus.h"
#include "platform.h"

GroupList *groupLists = 0;
short keyboardMoved = 0;

InputItem *highlightedItem;
unsigned short inputFlags;
short groupListCount;
GroupList *currentList;
Group *currentGroup;
InputItem *currentItem;
Cursor searchCursor;
InputItem *enteredItem;
short searchKind;
Point *searchPoint;
InputItem *searchItem;
short searchColumn;
short searchRow;
short searchKey;
unsigned short searchFlags;
short inputMode;
short hovering;
void (*mouseHook)(Point *where);

/* Installs the lists of groups to move the focus over, and numbers their
   items. */
/* @zoombi32 0x004124a4 */
void setGroupLists(GroupList *lists, short count, short flags)
{
    groupLists = lists;
    groupListCount = count;
    inputFlags = flags;
    numberAllItems();
}

/* The mouse is at a point (with `button` pressed, if not 0): hovers over the
   item there and tracks the press; whether there was an item. */
/* @zoombi32 0x004124cc */
short handleMouse(Point *where, unsigned short button)
{
    InputState saved;
    short handled;

    saveInputState(&saved, 1);
    inputMode = 0;
    hovering = button == 0;
    if (focusItemAtPoint(where)) {
        enterFocusedItem();
        if (button)
            trackPress(button);
        handled = 1;
    } else {
        leaveEnteredItem();
        handled = 0;
    }
    loadInputState(&saved, 1);
    return handled;
}

/* Hovers over the item at a point (entering it), or leaves the one entered;
   the item, if any. */
/* @zoombi32 0x00412537 */
InputItem *hoverItemAtPoint(Point *where)
{
    InputState saved;
    InputItem *item;

    saveInputState(&saved, 1);
    inputMode = 0;
    if (focusItemAtPoint(where)) {
        enterFocusedItem();
        item = currentItem;
    } else {
        leaveEnteredItem();
        item = 0;
    }
    loadInputState(&saved, 1);
    return item;
}

/* trackPress for an item, keeping the state. */
/* @zoombi32 0x00412587 */
short trackItemPress(InputItem *item, unsigned short button)
{
    InputState saved;
    short result;

    saveInputState(&saved, 1);
    inputMode = 0;
    if (focusItem(item))
        result = trackPress(button);
    else
        result = 0;
    loadInputState(&saved, 1);
    return result;
}

/*
 * Tracks a press of the mouse button on the focused item until it's let go,
 * following the mouse (the group's hit test) and switching the item as it
 * goes: the group's kind makes it a push button, one that latches, or a
 * toggle. Whether anything changed.
 *
 * Not exact: at `target = over ^ wasOn` the original loads `over` (ebx) and
 * XORs in `wasOn` (esi); every form tried (either order, declaration orders,
 * types) loads `wasOn` first.
 */
/* @zoombi32 0x004125d3 */
short trackPress(unsigned short button)
{
    short target, held;
    Point where;
    unsigned short wasOn;
    short over, changed;

    changed = 0;
    wasOn = (currentItem->flags & 4) == 4;
    target = !wasOn;
    over = 1;
    highlightFocus();
    if (wasOn && !(currentGroup->flags & 0x10)) {
        if (moveOverridden() && !handlersOverridden())
            callItemHandlers(1);
        return 0;
    }
    do {
        changed |= setFocusedOn(target, 0);
        getHookedMousePosition(&where);
        mainLoopEvents();
        held = isButtonStillDown(button) && (currentGroup->flags & 0xe000) != 0x2000;
        if (held && (currentGroup->flags & 0xe000) != 0x8000) {
            over = hitTestFocus(&where);
            if ((currentGroup->flags & 0xe000) == 0x4000)
                held &= over;
            else
                target = over ^ wasOn;
        }
    } while (held);
    if (currentGroup->flags & 0xe000 || over) {
        changed |= setFocusedOn(target, 1);
        if (!(currentGroup->flags & 4))
            changed |= setFocusedOn(wasOn, 1);
    }
    return changed;
}

/*
 * Sets the focused item on or off, following its group's rules: flag 8 of
 * the group gives the on-state and flag 0x20 the value that make the list
 * exclusive here (switchOffOthers, before or after depending on the group's
 * kind); the list's `changed` callback hears of it. Whether it did.
 */
/* @zoombi32 0x00412722 */
short setFocusedOn(short on, short value)
{
    short wasOn, onState, notValue, early;

    wasOn = (currentItem->flags & 4) == 4;
    notValue = (currentGroup->flags & 0x20) != 0x20;
    onState = (currentGroup->flags & 8) == 8;
    early = (currentGroup->flags & 0xe000) == 0x2000 || (currentGroup->flags & 0xe000) == 0x8000;

    if (on == onState && notValue == value && early)
        switchOffOthers();
    if (on != wasOn)
        toggleFocusedItem();
    if (on == onState && notValue == value) {
        if (!early)
            switchOffOthers();
        if (currentList->changed)
            currentList->changed(searchCursor.c.x);
        return 1;
    }
    return 0;
}

/* Whether a point is on the focused item: its group's hit test, or the
   engine's. */
/* @zoombi32 0x0041280d */
short hitTestFocus(Point *where)
{
    if (!currentGroup->handlers->hitTest)
        return ptInRect(&currentItem->bounds, *where);
    else
        return currentGroup->handlers->hitTest(where, currentItem);
}

/* Whether an item is available in the current mode (inputMode): in mode 0,
   unless flag 1 or 2 is set; in mode 1, unless flag 1 is. */
/* @zoombi32 0x00412844 */
short itemAvailable(InputItem *item)
{
    if (!item)
        return 0;
    switch (inputMode) {
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

/* The same test as itemAvailable, for the flags in inputFlags. */
/* @zoombi32 0x00412884 */
short listsAvailable()
{
    if (!groupLists)
        return 0;
    switch (inputMode) {
    case 0:
        if (inputFlags & 1 || inputFlags & 2)
            return 0;
        break;
    case 1:
        if (inputFlags & 1)
            return 0;
        break;
    }
    return 1;
}

/* In mode 0 a list with flag 1 or 2, in mode 1 one with flag 1, is counted
   (its groups' sizes and its length) instead of taken. */
/* @zoombi32 0x004128c6 */
short listSearchable(GroupList *list)
{
    short i;

    if (!list)
        return 0;
    switch (inputMode) {
    case 0:
        if (list->flags & 1 || list->flags & 2) {
            for (i = 0; i < list->count; i++)
                searchCursor.b.x += list->groups[i].count;
            searchCursor.a.y += list->count;
            return 0;
        }
        break;
    case 1:
        if (list->flags & 1) {
            for (i = 0; i < list->count; i++)
                searchCursor.b.x += list->groups[i].count;
            searchCursor.a.y += list->count;
            return 0;
        }
        break;
    }
    return 1;
}

/* The same test as listSearchable, for one group. */
/* @zoombi32 0x0041295f */
short groupSearchable(Group *group)
{
    if (!group)
        return 0;
    if (hovering && !group->handlers->enter)
        return 0;
    switch (inputMode) {
    case 0:
        if (group->flags & 1 || group->flags & 2) {
            searchCursor.b.x += group->count;
            searchCursor.c.x += group->count;
            return 0;
        }
        break;
    case 1:
        if (group->flags & 1) {
            searchCursor.b.x += group->count;
            searchCursor.c.x += group->count;
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

    if (moveOverridden()) {
        if (currentItem != highlightedItem) {
            saveInputState(&saved, 0);
            inputMode = 1;
            if (focusItem(highlightedItem)) {
                highlightedItem = 0;
                if (!handlersOverridden())
                    callItemHandlers(currentItem->flags & 4);
            }
            loadInputState(&saved, 0);
        }
        highlightedItem = currentItem;
        if (keyboardMoved)
            moveMouseToFocus();
    }
}

/* Switches the focused item on or off (flag 4), with its sound. */
/* @zoombi32 0x00412a6e */
void toggleFocusedItem()
{
    short off = (currentItem->flags & 4) != 4;

    callItemHandlers(off);
    currentItem->flags ^= 4;
    if (currentGroup->sounds)
        playSound(currentGroup->sounds[searchCursor.c.y * 2 - off - 1], RESOURCE_TYPE(0, 'S', 'N', 'D'), 0,
                  0, 1);
}

/* In a list whose items are exclusive (flag 4), switches off every item that's
   on apart from the focused one. */
/* @zoombi32 0x00412ace */
void switchOffOthers()
{
    InputState saved;

    if (currentList->flags & 4) {
        saveInputState(&saved, 0);
        searchKind = 4;
        searchFlags = 4;
        searchCursor.b.y = 1;
        searchCursor.c.y = 0;
        while (searchListForward(currentList, searchCursor.b.y - 1, searchCursor.c.y))
            if (currentItem != saved.item)
                toggleFocusedItem();
        loadInputState(&saved, 0);
    }
}

/* Calls callback with currentItem if there is one; returns whether it did. */
/* @zoombi32 0x00412b4d */
short callItemHandler(void (*callback)(InputItem *item))
{
    if (!callback)
        return 0;
    callback(currentItem);
    return 1;
}

/* Calls the handlerC handler, or else (defaultHandler4) its default. */
/* @zoombi32 0x00412b6b */
void callHandlerC()
{
    if (handlersOverridden() || !callItemHandler(currentGroup->handlers->handlerC))
        defaultHandler4();
}

/* Calls the handler8 handler, or else (defaultHandler0) its default. */
/* @zoombi32 0x00412b8f */
void callHandler8()
{
    if (handlersOverridden() || !callItemHandler(currentGroup->handlers->handler8))
        defaultHandler0();
}

/* Calls the handler for the current item: the default (defaultHandler10) if
   something is switched off (flag 2), else one of a pair, for highlightedItem or
   another item, and `alternative` or not. */
/* @zoombi32 0x00412bb3 */
void callItemHandlers(short alternative)
{
    if (currentGroup->flags & 0x80 && currentItem != enteredItem)
        return;
    if (inputFlags & 2 || currentList->flags & 2 || currentGroup->flags & 2 || currentItem->flags & 2) {
        defaultHandler10();
        return;
    }
    if (currentItem == highlightedItem) {
        if (alternative)
            callHandlerC();
        else
            callHandler8();
    } else if (alternative)
        defaultHandler4();
    else
        defaultHandler0();
}

/* The same choice as callItemHandlers, for the other set of handlers, by the
   item's flag 4. */
/* @zoombi32 0x00412c3d */
void callOtherHandlers()
{
    if (currentGroup->flags & 0x80 && currentItem != enteredItem)
        return;
    if (inputFlags & 2 || currentList->flags & 2 || currentGroup->flags & 2 || currentItem->flags & 2) {
        defaultHandler24();
        return;
    }
    if (currentItem == highlightedItem) {
        if (currentItem->flags & 4)
            callHandler20();
        else
            callHandler1C();
    } else if (currentItem->flags & 4)
        defaultHandler18();
    else
        defaultHandler14();
}

/* @zoombi32 0x00412cc0 */
void defaultHandler4()
{
    callItemHandler(currentGroup->handlers->handler4);
}

/* @zoombi32 0x00412cd0 */
void defaultHandler0()
{
    callItemHandler(currentGroup->handlers->handler0);
}

/* @zoombi32 0x00412cdf */
void defaultHandler10()
{
    callItemHandler(currentGroup->handlers->handler10);
}

/* Calls the handler20 handler, or else (defaultHandler18) its default. */
/* @zoombi32 0x00412cef */
void callHandler20()
{
    if (handlersOverridden() || !callItemHandler(currentGroup->handlers->handler20))
        defaultHandler18();
}

/* Calls the handler1C handler, or else (defaultHandler14) its default. */
/* @zoombi32 0x00412d13 */
void callHandler1C()
{
    if (handlersOverridden() || !callItemHandler(currentGroup->handlers->handler1C))
        defaultHandler14();
}

/* @zoombi32 0x00412d37 */
void defaultHandler18()
{
    callItemHandler(currentGroup->handlers->handler18);
}

/* @zoombi32 0x00412d47 */
void defaultHandler14()
{
    callItemHandler(currentGroup->handlers->handler14);
}

/* @zoombi32 0x00412d57 */
void defaultHandler24()
{
    callItemHandler(currentGroup->handlers->handler24);
}

/* Enters the focused item (calls its group's enter handler), leaving the
   one entered before. */
/* @zoombi32 0x00412d67 */
void enterFocusedItem()
{
    if (enteredItem) {
        if (enteredItem == currentItem)
            return;
        leaveEnteredItem();
    }
    enteredItem = currentItem;
    callItemHandler(currentGroup->handlers->enter);
}

/* Leaves the item entered last (calls its group's leave handler). */
/* @zoombi32 0x00412d9c */
void leaveEnteredItem()
{
    InputState saved;

    if (enteredItem) {
        saveInputState(&saved, 0);
        inputMode = 1;
        if (focusItem(enteredItem))
            callItemHandler(currentGroup->handlers->leave);
        enteredItem = 0;
        loadInputState(&saved, 0);
    }
}

/* Where an item is, keeping the state. */
/* @zoombi32 0x00412df4 */
void getItemPosition(InputItem *item, Cursor *where)
{
    InputState saved;

    saveInputState(&saved, 1);
    inputMode = 2;
    focusItem(item);
    *where = searchCursor;
    loadInputState(&saved, 1);
}

/* The item at a position (from 1), if any, keeping the state. */
/* @zoombi32 0x00412e44 */
InputItem *itemAt(short x, short y)
{
    InputState saved;
    InputItem *item;

    if (x <= 0 || y <= 0)
        return 0;
    saveInputState(&saved, 1);
    inputMode = 2;
    if (focusItemAt(x, y))
        item = currentItem;
    else
        item = 0;
    loadInputState(&saved, 1);
    return item;
}

/*
 * A key was pressed: Return presses the highlighted item (or the one under
 * the mouse, if the mouse follows the focus), Tab and Shift+Tab (0x800 is
 * Shift, see addModifierKeys) move the focus, and other keys press the item
 * they select. The item pressed, if any; Tab keys are used up (*key = 0).
 */
/* @zoombi32 0x00412e9f */
InputItem *handleKey(unsigned short *key)
{
    InputState saved;
    Point where;
    short highlighted, direction;
    InputItem *item;

    if (!moveOverridden())
        return 0;
    saveInputState(&saved, 1);
    keyboardMoved = 1;
    inputMode = 0;
    highlighted = focusItem(highlightedItem);
    if (*key == '\r') {
        if (handlersOverridden()) {
            getHookedMousePosition(&where);
            if (focusItemAtPoint(&where)) {
                pressFocusedItem();
                loadInputState(&saved, 1);
                return highlightedItem;
            }
        } else if (highlighted) {
            pressFocusedItem();
            loadInputState(&saved, 1);
            return highlightedItem;
        }
    }
    direction = 1;
    switch (*key) {
    case 0x809:
        direction = -direction;
    case 9:
        if (highlighted)
            stepFocus(direction);
        *key = 0;
        loadInputState(&saved, 1);
        return 0;
    }
    if (focusItemByKey(*key)) {
        pressFocusedItem();
        item = currentItem;
        loadInputState(&saved, 1);
        return item;
    }
    loadInputState(&saved, 1);
    return 0;
}

/* Moves the focus to the next or previous item and highlights it. */
/* @zoombi32 0x00412fb5 */
void stepFocus(short direction)
{
    searchKind = 5;
    if (moveFocus(direction)) {
        highlightFocus();
        if (!handlersOverridden())
            callItemHandlers(currentItem->flags & 4);
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
    short list = searchCursor.a.x - 1;
    short group = searchCursor.b.y - 1;
    short item;

    if (direction > 0)
        item = searchCursor.c.y;
    else
        item = searchCursor.c.y - 2;
    do {
        if (direction > 0) {
            if (currentList->flags & 8) {
                if (searchListForward(currentList, group, item))
                    return 1;
                group = item = 0;
            } else {
                if (searchForward(list, group, item))
                    return 1;
                list = group = item = 0;
            }
        } else {
            if (currentList->flags & 8) {
                if (searchListBackward(currentList, group, item))
                    return 1;
                group = currentList->count - 1;
                item = currentList->groups[group].count - 1;
            } else {
                if (searchBackward(list, group, item))
                    return 1;
                list = groupListCount - 1;
                group = groupLists[list].count - 1;
                item = groupLists[list].groups[group].count - 1;
            }
        }
        tries--;
    } while (tries);
    return 0;
}

/* Presses the focused item from the keyboard: shows it pressed for 30 ticks
   (running the main loop), then switches it as a click would. */
/* @zoombi32 0x00413129 */
void pressFocusedItem()
{
    unsigned short wasOn;
    short target;
    unsigned long start;

    wasOn = (currentItem->flags & 4) == 4;
    target = !wasOn;
    highlightFocus();
    if (wasOn && !(currentGroup->flags & 0x10))
        return;
    start = clockTicks();
    setFocusedOn(target, 0);
    while (clockTicks() <= start + 30)
        mainLoopEvents();
    setFocusedOn(target, 1);
    if (!(currentGroup->flags & 4))
        setFocusedOn(wasOn, 1);
}

/* Highlights the item at a position (from 1), keeping the state; the item. */
/* @zoombi32 0x004131a3 */
InputItem *highlightItemAt(short x, short y)
{
    InputState saved;
    InputItem *item;

    if (x <= 0 || y <= 0)
        return 0;
    if (!moveOverridden())
        return 0;
    saveInputState(&saved, 1);
    keyboardMoved = 1;
    inputMode = 0;
    if (focusItemAt(x, y)) {
        highlightFocus();
        if (!handlersOverridden())
            callItemHandlers(currentItem->flags & 4);
        item = currentItem;
    } else
        item = 0;
    loadInputState(&saved, 1);
    return item;
}

/* Calls the handler of the item at a position (from 1), keeping the state. */
/* @zoombi32 0x00413237 */
void activateItemAt(short x, short y)
{
    InputState saved;

    if (x > 0 && y > 0) {
        saveInputState(&saved, 1);
        inputMode = 1;
        if (focusItemAt(x, y))
            callItemHandlers(currentItem->flags & 4);
        loadInputState(&saved, 1);
    }
}

/* Calls every item's handlers (search 6, in mode 1), keeping the state. */
/* @zoombi32 0x00413295 */
void visitAllItems()
{
    InputState saved;

    saveInputState(&saved, 1);
    searchKind = 6;
    inputMode = 1;
    searchForward(0, 0, 0);
    loadInputState(&saved, 1);
}

/* Moves the mouse, if handlersOverridden says it follows the focus. */
/* @zoombi32 0x004132d2 */
void moveMouseTo(short x, short y)
{
    if (handlersOverridden())
        setCursorPosition(x, y);
}

/* Moves the mouse to the focused item (its centre or its hotspot), if it
   follows the focus. */
/* @zoombi32 0x00413312 */
void moveMouseToFocus()
{
    if (handlersOverridden()) {
        if (currentGroup->flags & 0x40) {
            InputItem *item = currentItem;
            setCursorPosition((item->bounds.right + item->bounds.left) / 2,
                              (item->bounds.bottom + item->bounds.top) / 2);
        } else
            setCursorPosition(currentItem->hotspot.x, currentItem->hotspot.y);
    }
}

/* Flag 0x8000 of inputFlags applies with a mouse, 0x4000 without one. */
/* @zoombi32 0x0041336f */
short handlersOverridden()
{
    short noMouse = !(unsigned short)isMousePresent();
    return inputFlags & 0x8000 && !noMouse || inputFlags & 0x4000 && noMouse;
}

/* Flag 0x2000 of inputFlags applies with a mouse, 0x1000 without one. */
/* @zoombi32 0x004133a4 */
short moveOverridden()
{
    short noMouse = !(unsigned short)isMousePresent();
    return inputFlags & 0x2000 && !noMouse || inputFlags & 0x1000 && noMouse;
}

/* Moves the focus to the first item at a point (by its group's hit test). */
/* @zoombi32 0x004133d9 */
short focusItemAtPoint(Point *where)
{
    searchKind = 0;
    searchPoint = where;
    return searchForward(0, 0, 0);
}

/* Moves the focus to the first item with a key (either case). */
/* @zoombi32 0x004133fc */
short focusItemByKey(short key)
{
    searchKind = 3;
    searchKey = toUpperAscii(key);
    return searchForward(0, 0, 0);
}

/* Moves the focus to the item at a position. */
/* @zoombi32 0x00413427 */
short focusItemAt(short x, short y)
{
    searchKind = 2;
    searchColumn = x;
    searchRow = y;
    return searchForward(0, 0, 0);
}

/* Moves the focus to an item. */
/* @zoombi32 0x00413456 */
short focusItem(InputItem *item)
{
    if (!itemAvailable(item))
        return 0;
    searchKind = 1;
    searchItem = item;
    return searchForward(0, 0, 0);
}

/* Searches all lists (groupLists) from list `list`, group `group`, item `start`,
   onwards (searchListForward); whether an item was found. */
/* @zoombi32 0x0041348b */
short searchForward(short list, short group, short start)
{
    short found, i, j;
    GroupList *current;

    memset(&searchCursor, 0, sizeof searchCursor);
    if (!listsAvailable())
        return 0;
    searchCursor.a.y = group;
    searchCursor.b.x = start;
    for (i = 0; i < list; i++) {
        searchCursor.a.y += groupLists[i].count;
        for (j = 0; j < groupLists[i].count; j++)
            searchCursor.b.x += groupLists[i].groups[j].count;
    }
    searchCursor.a.x = list;
    found = 0;
    for (current = &groupLists[list]; searchCursor.a.x < groupListCount && !found; current++) {
        found = searchListForward(current, group, start);
        group = start = 0;
        searchCursor.a.x++;
    }
    if (!found)
        searchCursor.a.x = searchCursor.a.y = searchCursor.b.x = 0;
    return found;
}

/* The same as searchForward, backwards (each earlier list from its last item). */
/* @zoombi32 0x0041357a */
short searchBackward(short list, short group, short start)
{
    short found, i, j;
    GroupList *current;

    memset(&searchCursor, 0, sizeof searchCursor);
    if (!listsAvailable())
        return 0;
    searchCursor.a.y = group;
    searchCursor.b.x = start;
    for (i = 0; i < list; i++) {
        searchCursor.a.y += groupLists[i].count;
        for (j = 0; j < groupLists[i].count; j++)
            searchCursor.b.x += groupLists[i].groups[j].count;
    }
    searchCursor.a.x = list;
    found = 0;
    for (current = &groupLists[list]; searchCursor.a.x >= 0 && !found; current--) {
        found = searchListBackward(current, group, start);
        group = groupLists[searchCursor.a.x - 1].count - 1;
        start = groupLists[searchCursor.a.x - 1].groups[group].count - 1;
        searchCursor.a.x--;
    }
    searchCursor.a.x++;
    searchCursor.a.y++;
    searchCursor.b.x++;
    if (!found)
        searchCursor.a.x = searchCursor.a.y = searchCursor.b.x = 0;
    return found;
}

/* Searches a list's groups from group `first`, item `start`, onwards
   (searchGroupForward), moving the cursor along; whether an item was found. */
/* @zoombi32 0x00413693 */
short searchListForward(GroupList *list, short first, short start)
{
    short found, i;
    Group *group;

    searchCursor.b.y = searchCursor.c.x = searchCursor.c.y = 0;
    if (!listSearchable(list))
        return 0;
    currentList = list;
    searchCursor.c.x = start;
    for (i = 0; i < first; i++) {
        searchCursor.b.x += list->groups[i].count;
        searchCursor.c.x += list->groups[i].count;
    }
    searchCursor.b.y = first;
    found = 0;
    for (group = &list->groups[first]; searchCursor.b.y < list->count && !found; group++) {
        found = searchGroupForward(group, start);
        start = 0;
        searchCursor.b.y++;
        searchCursor.a.y++;
    }
    if (!found)
        searchCursor.b.y = searchCursor.c.x = 0;
    return found;
}

/* The same as searchListForward, backwards (each earlier group from its last item). */
/* @zoombi32 0x00413755 */
short searchListBackward(GroupList *list, short first, short start)
{
    short found, i;
    Group *group;

    searchCursor.b.y = searchCursor.c.x = searchCursor.c.y = 0;
    if (!listSearchable(list))
        return 0;
    currentList = list;
    searchCursor.c.x = start;
    for (i = 0; i < first; i++) {
        searchCursor.b.x += list->groups[i].count;
        searchCursor.c.x += list->groups[i].count;
    }
    searchCursor.b.y = first;
    found = 0;
    for (group = &list->groups[first]; searchCursor.b.y >= 0 && !found; group--) {
        found = searchGroupBackward(group, start);
        start = list->groups[searchCursor.b.y - 1].count - 1;
        searchCursor.b.y--;
        searchCursor.a.y--;
    }
    searchCursor.b.y++;
    searchCursor.c.x++;
    if (!found)
        searchCursor.b.y = searchCursor.c.x = 0;
    return found;
}

/* Searches a group's items from `start` onwards with matchItem, moving the
   cursor along; whether one was found (the cursor's index is then its). */
/* @zoombi32 0x0041382a */
short searchGroupForward(Group *group, short start)
{
    short found;
    InputItem *item;

    searchCursor.c.y = 0;
    if (!groupSearchable(group))
        return 0;
    currentGroup = group;
    searchCursor.c.y = start;
    found = 0;
    for (item = &group->items[start]; searchCursor.c.y < group->count && !found; item++) {
        found = matchItem(item);
        searchCursor.c.y++;
        searchCursor.c.x++;
        searchCursor.b.x++;
    }
    if (!found)
        searchCursor.c.y = 0;
    return found;
}

/* The same as searchGroupForward, backwards from `start`. */
/* @zoombi32 0x004138a2 */
short searchGroupBackward(Group *group, short start)
{
    short found;
    InputItem *item;

    searchCursor.c.y = 0;
    if (!groupSearchable(group))
        return 0;
    currentGroup = group;
    searchCursor.c.y = start;
    found = 0;
    for (item = group->items + start; searchCursor.c.y >= 0 && !found; item--) {
        found = matchItem(item);
        searchCursor.c.y--;
        searchCursor.c.x--;
        searchCursor.b.x--;
    }
    searchCursor.c.y++;
    if (!found)
        searchCursor.c.y = 0;
    return found;
}

/*
 * Whether an item is what's being looked for (searchKind): 0 the one at a
 * point (by its group's hit test), 1 a particular item, 2 the one past the cursor, 3 the
 * one with a key, 4 one with some flags, 5 any; 6 and 7 visit them (calling
 * their handlers, or giving them their places). Makes it the current item.
 */
/* @zoombi32 0x0041391d */
short matchItem(InputItem *item)
{
    short found;

    if (!itemAvailable(item))
        return 0;
    currentItem = item;
    switch (searchKind) {
    case 0:
        found = hitTestFocus(searchPoint);
        break;
    case 1:
        found = item == searchItem;
        break;
    case 2:
        found = searchColumn == searchCursor.a.x + 1 && searchRow == searchCursor.c.x + 1;
        break;
    case 3:
        found = toUpperAscii(item->key) == searchKey;
        break;
    case 4:
        found = (item->flags & searchFlags) != 0;
        break;
    case 5:
        found = 1;
        break;
    case 6:
        callOtherHandlers();
        found = 0;
        break;
    case 7:
        item->cursor = searchCursor;
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
void loadInputState(InputState *state, short all)
{
    currentList = state->list;
    currentGroup = state->group;
    currentItem = state->item;
    searchCursor.a = state->cursorA;
    searchCursor.b = state->cursorB;
    searchCursor.c = state->cursorC;
    if (all) {
        searchKind = state->search;
        searchPoint = state->point;
        searchItem = state->searchItem;
        searchColumn = state->searchColumn;
        searchRow = state->searchRow;
        searchKey = state->searchKey;
        searchFlags = state->searchFlags;
        inputMode = state->mode;
        keyboardMoved = state->keyboardMoved;
        hovering = state->hovering;
    }
}

/* Saves the input state (the part from +0x18 only with `all`). */
/* @zoombi32 0x00413afd */
void saveInputState(InputState *state, short all)
{
    state->list = currentList;
    state->group = currentGroup;
    state->item = currentItem;
    state->cursorA = searchCursor.a;
    state->cursorB = searchCursor.b;
    state->cursorC = searchCursor.c;
    if (all) {
        state->search = searchKind;
        state->point = searchPoint;
        state->searchItem = searchItem;
        state->searchColumn = searchColumn;
        state->searchRow = searchRow;
        state->searchKey = searchKey;
        state->searchFlags = searchFlags;
        state->mode = inputMode;
        state->keyboardMoved = keyboardMoved;
        state->hovering = hovering;
    }
}

/* Where the mouse is, also passed to the hook setMouseHook set, if any. */
/* @zoombi32 0x00413bad */
void getHookedMousePosition(Point *where)
{
    getMousePosition(where);
    if (mouseHook)
        mouseHook(where);
}

/* @zoombi32 0x00413bcf */
void setMouseHook(void (*hook)(Point *where))
{
    mouseHook = hook;
}

/* Gives every item its position (search 7, in mode 2), keeping the state. */
/* @zoombi32 0x00413be4 */
void numberAllItems()
{
    InputState saved;

    saveInputState(&saved, 1);
    searchKind = 7;
    inputMode = 2;
    searchForward(0, 0, 0);
    loadInputState(&saved, 1);
}
