/*
 * net (0x439560-0x4402c0): 'Net.MHK'
 */

#include "zoombinis.h"

/* Splices a list in after another. */
/* @zoombi32 0x0043a772 */
void spliceList(Link *other, Link *list)
{
    if (other && list) {
        Link *last = list;
        while (last->next)
            last = last->next;
        list->prev = other;
        last->next = other->next;
        other->next = list;
        last->next->prev = last;
    }
}

/*
 * Which of the four groups of three scenes (7-18) the current scene is in
 * (1-4; 0 if none), and in *last whether it's the group's last.
 */
/* @zoombi32 0x0043af02 */
short sceneGroup(short *last)
{
    short group = 0;

    *last = 0;
    if (currentScene >= 7 && currentScene <= 18) {
        if (currentScene == 9 || currentScene == 12 || currentScene == 15 || currentScene == 18)
            *last = 1;
        group = ((currentScene - 7) / 3 & 3) + 1;
    }
    return group;
}

/* @zoombi32 0x0044027b */
short fn_44027b(long, long)
{
    return 1;
}

/* Sets up a maze Zoombini's parts (its body's words 20-45): its scripts
   for each move, by its feet. */
/* @zoombi32 0x00439560 */
void fn_439560(Snoid *snoid)
{
    short unused[2];
    short *parts = (short *)snoid;

    parts[20] = -1;
    parts[21] = snoid->features[3] + 15014;
    parts[22] = snoid->features[3] + 15019;
    parts[23] = snoid->features[3] + 15024;
    parts[24] = snoid->features[3] + 15029;
    parts[25] = snoid->features[3] + 15055;
    parts[26] = snoid->features[3] + 15060;
    parts[27] = snoid->features[3] + 15065;
    parts[28] = snoid->features[3] + 15070;
    parts[29] = 0;
    parts[31] = 0;
    parts[32] = 0;
    parts[33] = 0;
    parts[34] = 0;
    parts[35] = 0;
    parts[36] = 0;
    parts[37] = snoid->features[3] + 14999;
    parts[38] = snoid->features[3] + 15004;
    parts[39] = snoid->features[3] + 15009;
    parts[41] = 0;
    parts[42] = 0;
    parts[45] = snoid->features[3] - 1;
}

/* Sorts a list of views by where they stand (their bounds' bottom, then
   left), for drawing back to front; returns the new head. */
/* @zoombi32 0x0043a69a */
View *fn_43a69a(View *list)
{
    View *sorted;
    View *view;
    View *at;
    ShortRect other;
    ShortRect bounds;

    view = at = 0;
    if (list) {
        sorted = at = list;
        list = list->next;
        sorted->prev = 0;
        sorted->next = 0;
    } else {
        return 0;
    }
    while (list) {
        view = list;
        list = list->next;
        bounds = view->body.bounds;
        for (at = sorted; at;) {
            other = at->body.bounds;
            if (bounds.bottom < other.bottom || bounds.bottom == other.bottom && bounds.left < other.left) {
                view->prev = at->prev;
                view->next = at;
                at->prev = view;
                if (view->prev)
                    view->prev->next = view;
                else
                    sorted = view;
                at = 0;
            } else if (!at->next) {
                at->next = view;
                view->prev = at;
                view->next = 0;
                at = 0;
            } else {
                at = at->next;
            }
        }
    }
    return sorted;
}

/* Takes the views with exactly `flags` out of the view list, sorts them
   (fn_43a69a) and puts them back after `after`. */
/* @zoombi32 0x0043a5f6 */
void fn_43a5f6(View *after, unsigned long flags)
{
    View *first;
    View *view;
    View *prev;
    View *following;
    View *last;
    View *next;

    if (after && flags) {
        next = viewListEnd(1)->next;
        first = 0;
        last = 0;
        while (next) {
            view = next;
            next = next->next;
            if (view->flags == flags) {
                if (!first) {
                    first = last = view;
                    prev = view->prev;
                    following = view->next;
                    if (prev)
                        prev->next = following;
                    if (following)
                        following->prev = prev;
                    view->prev = 0;
                    view->next = 0;
                } else {
                    last->next = view;
                    prev = view->prev;
                    following = view->next;
                    if (prev)
                        prev->next = following;
                    if (following)
                        following->prev = prev;
                    view->prev = last;
                    view->next = 0;
                    last = view;
                }
            }
        }
        if (first)
            spliceList((Link *)after, (Link *)fn_43a69a(first));
    }
}

/* A view's placing: a second cel, from its word 20, where the first is. */
/* @zoombi32 0x0043d6e2 */
void fn_43d6e2(View *view)
{
    short *parts = (short *)&view->body;

    parts[3] = parts[20];
    parts[4] = parts[1];
    parts[5] = parts[2];
    parts[6] = 0;
}

/* Sets g_4b15a8 if there are Zoombinis chosen and either at least
   g_4b15ae of them or 625 counted in the game (the whole population). */
/* @zoombi32 0x00440286 */
void fn_440286()
{
    g_4b15a8 = 0;
    short count = countChosenSnoids();

    if (count)
        g_4b15a8 = count >= g_4b15ae || *(short *)(g_4a4ba0 + 0x48) >= 625;
}

/* Closes the scene. */
/* @zoombi32 0x0043b820 */
void closeNet()
{
    if (g_4b12a8) {
        g_4b12a8 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a2e54);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b12a4);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Draws image `which` of the bank g_4b159c at (x, y) by its hot spot
   (g_4a3386, g_4a339c). */
/* @zoombi32 0x0043f985 */
void fn_43f985(short which, short x, short y)
{
    if (which)
        drawImageData((unsigned short *)(g_4b159c->offsets[which] + (char *)g_4b159c), x - g_4a3386[which],
                      y - g_4a339c[which], 8);
}

/* Adds the views g_4b15b0 (script 4104) and g_4b15b2 (4105) if they
   aren't there. */
/* @zoombi32 0x00440218 */
void fn_440218()
{
    if (!g_4b15b0)
        g_4b15b0 = addView(0x800c000, drawCels, runViewScript, 4104, 7, 0, 0, 0);
    if (!g_4b15b2)
        g_4b15b2 = addView(0x8008000, drawCels, runViewScript, 4105, 9, 0, 0, 0);
}

/* Fills column n % 5 of g_4b0e78 with `a` and row n / 5 of g_4b0f72 with
   `b` (5 by 5). */
/* Not exact: the original keeps `n` in esi and `row` in edi; BCC puts `n`
   in ecx and `row` in esi. */
/* @zoombi32 0x0043c8e2 */
void fn_43c8e2(short a, short b, short n)
{
    short row;
    short rowStart;
    short column;
    short i;

    row = n / 5;
    rowStart = n - n % 5;
    column = n - row * 5;
    for (i = 0; i < 5; i++) {
        g_4b0e78[column + i * 5] = a;
        g_4b0f72[rowStart + i] = b;
    }
}

/* The same for cube n (5 by 5 by 5): its row in g_4b0e78 and g_4b0f72,
   and `c` in g_4b106c by its column. */
/* @zoombi32 0x0043c94f */
void fn_43c94f(short a, short b, short c, short n)
{
    short rest;
    short plane;
    short column;
    short i;

    plane = n / 25;
    rest = n % 25;
    plane *= 5;
    rest /= 5;
    column = n % 5;
    for (i = 0; i < 5; i++) {
        g_4b0e78[rest + i * 5] = a;
        g_4b0f72[plane + i] = b;
    }
    g_4b106c[column] = c;
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   g_4b12aa), lit or not, and with `show` shows it. */
/* @zoombi32 0x0043b6f8 */
void drawNetButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b12aa) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a2e60->offsets[image] + (char *)g_4a2e60), g_4a288a[which].rect.left,
                      g_4a288a[which].rect.top, 8);
        if (show)
            showRect(&g_4a288a[which].rect);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x0043b791 */
void drawNetButtons(View *)
{
    drawNetButton(1, 0, 0);
    drawNetButton(2, 0, 0);
}

/* A view's update: adds buttons 3 (when g_4b12aa changes) and 2 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0043b7ae */
void updateNetButtons(View *, short region)
{
    if (g_4b12aa) {
        if (!g_4a2ea4) {
            g_4a2ea4 = 1;
            unionRgnRect(region, &g_4a288a[3].rect);
        }
    } else if (g_4a2ea4) {
        g_4a2ea4 = 0;
        unionRgnRect(region, &g_4a288a[3].rect);
    }
    if (!g_4a2ea6) {
        g_4a2ea6 = 1;
        unionRgnRect(region, &g_4a288a[2].rect);
    }
}

/* Resets the Zoombini being made (g_4b1484): no features, a new name. */
/* @zoombi32 0x0043e620 */
void fn_43e620()
{
    short i;

    g_4b755e = 60;
    g_4b15a6 = g_4b15aa = g_4b15ac = 0;
    g_4b0d52 = 0;
    g_4b15b0 = g_4b15b2 = g_4b15b6 = 0;
    for (i = 0; i < 4; i++)
        g_4b1484.features[i] = 0;
    g_4b1484.name[0] = 0;
    makeName(g_4b1484.name, 10);
    for (i = 0; i < 16; i++)
        g_4b75ee[i] = 0;
    g_4b15b8 = 0;
    g_4b755c = g_4b755a = 0;
}

/* Whether the Zoombini being made is complete (its features valid, any
   out of range cleared) and fewer than two of its kind exist yet. */
/* @zoombi32 0x0043fa67 */
short fn_43fa67()
{
    short i;
    short hair;
    short eyes;
    short nose;
    short feet;

    for (i = 0; i < 4; i++) {
        if (g_4b1484.features[i] < 0 || g_4b1484.features[i] > 5)
            g_4b1484.features[i] = 0;
        if (!g_4b1484.features[i])
            return 0;
    }
    hair = g_4b1484.features[0] - 1;
    eyes = g_4b1484.features[1] - 1;
    nose = g_4b1484.features[2] - 1;
    feet = g_4b1484.features[3] - 1;
    if (zoombiniCounts()[hair][eyes][nose][feet] >= 2)
        return 0;
    return 1;
}

/* Shows the two views g_4b15b0 and g_4b15b2 by the setting at game state
   +0x26 (0-3), stepping it on unless `keep`. */
/* @zoombi32 0x0043ff1d */
void fn_43ff1d(short keep)
{
    View *first;
    View *second;
    short a;
    short b;

    first = findView(g_4b15b0);
    second = findView(g_4b15b2);
    if (first && second) {
        if (!keep)
            (*(short *)(g_4a4ba0 + 0x26))++;
        if (*(short *)(g_4a4ba0 + 0x26) > 3)
            *(short *)(g_4a4ba0 + 0x26) = 0;
        switch (*(short *)(g_4a4ba0 + 0x26)) {
        case 0:
            a = 1;
            b = 1;
            break;
        case 1:
            a = 0;
            b = 0;
            break;
        case 2:
            a = 1;
            b = 0;
            break;
        case 3:
            a = 0;
            b = 1;
            break;
        }
        first->body.running = a;
        first->body.group = 0;
        second->body.running = b;
        second->body.group = 0;
    }
}

/* Splits the g_4b0e66 Zoombinis into groups (g_4b1182, g_4b0e76 of them)
   of three, two and one in turn, then evens out the overshoot by taking
   one from groups of two or more; if that can't be done, one each. */
/* Not exact: the original caches g_4b0e76's address in edi, and negates
   `left` through a 32-bit copy (movsx eax, dx / mov edx, eax / neg eax /
   mov edx, eax), perhaps an inline function's parameter. */
/* @zoombi32 0x0043e370 */
void fn_43e370()
{
    short i;
    short size;
    short n;
    short left;

    for (i = 0; i < 12; i++)
        g_4b1182[i] = 0;
    left = g_4b0e66;
    size = 4;
    n = 0;
    do {
        size--;
        if (size < 1)
            size = 3;
        g_4b1182[n] = size;
        n++;
        left -= size;
    } while (left > 0);
    g_4b0e76 = n;
    if (left) {
        left = -left;
        do {
            for (i = 0; i < g_4b0e76; i++)
                if (g_4b1182[i] >= 2 && left) {
                    g_4b1182[i]--;
                    left--;
                }
            if (left) {
                n = 0;
                for (i = 0; i < g_4b0e76; i++)
                    if (g_4b11aa[i] > 1)
                        n++;
                if (!n) {
                    g_4b0e76 = g_4b0e66;
                    for (i = 0; i < g_4b0e76; i++)
                        g_4b1182[i] = 1;
                    left = 0;
                }
            }
        } while (left);
    }
}

/* Moves a maze Zoombini on to the square it's heading for (words 33 and
   34), pairs the view `other` with it, and starts its script for its pose
   (from words 25 on) with its helper view's (script 10036 on), grouped. */
/* @zoombi32 0x0043a510 */
void fn_43a510(View *view, short other)
{
    short *parts = (short *)&view->body;
    View *helper;
    View *paired;

    parts[31] = parts[33];
    parts[32] = parts[34];
    paired = findView(other);
    if (paired) {
        short *its = (short *)&paired->body;

        its[43] = view->id;
    }
    *(Point *)&view->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10036, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = fn_436321;
    }
    startSnoidScript((Snoid *)&view->body, parts[25 + parts[20]], 0, 0);
    view->notify = fn_43638b;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* Closes the scene, leaving the party waiting (or, when it's leaving,
   taking it on). */
/* @zoombi32 0x0043eb13 */
void fn_43eb13()
{
    if (g_4b15a4) {
        g_4b15a4 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        if (!viewsLocked) {
            if (g_4a48e6 || g_4b0d50 == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                *waitingParties() = *party();
                party()->count = 0;
            } else {
                waitingParties()->count = 0;
            }
        }
        unloadSounds();
        fn_46c602(&g_4b1590);
        fn_46c602(&g_4b158c);
        fn_46c602(&g_4b1594);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b1588);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Sends the next Zoombini of the party (g_4b0e68 of g_4b0e66) to the
   first free place of three (g_4b1438, from the last), while g_4b144a lets
   it (for g_4b145c more); counts in g_4b11a0 the places left when none
   are left to send. */
/* @zoombi32 0x0043cfc3 */
void fn_43cfc3()
{
    Point target;
    short i;
    View *view;

    target.x = 233;
    target.y = 392;
    for (i = 2; i >= 0; i--)
        if (!g_4b1438[i]) {
            if (g_4b0e68 < g_4b0e66) {
                if (!g_4b144a)
                    return;
                if (!--g_4b145c)
                    g_4b144a = 0;
                view = findView(partyViews[g_4b0e68]);
                if (view) {
                    setSnoidAction((Snoid *)&view->body, 10, 0);
                    *(Point *)&((Snoid *)&view->body)->targetX = target;
                    g_4b0d5c = g_4b0e68;
                    g_4b1438[i] = partyViews[g_4b0e68];
                    g_4b0e68++;
                    g_4b119a = i;
                    g_4b1462 = 1;
                    return;
                }
            } else {
                g_4b11a0++;
            }
        }
    g_4b145c = 0;
    g_4b144a = 0;
}

/* Puts a maze Zoombini on its square (words 33 and 34) with its helper
   view (script 10030, told fn_435b9e) and a second view it adds (word 42:
   10031), and starts its script 14006 (then told fn_435f3d), grouped. */
/* @zoombi32 0x00439fc3 */
void fn_439fc3(View *view, short)
{
    short *parts = (short *)&view->body;
    View *helper;
    Point where;

    *(Point *)&view->body.x = (g_4afbf0 + parts[34])[parts[33] * 13];
    view->body.x += 3;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, 10030, 1);
        *(Point *)&helper->body.x = *(Point *)&view->body.x;
        helper->placed = fn_436321;
        helper->notify = fn_435b9e;
        where = *(Point *)&helper->body.x;
        parts[42] = addView(0x4988000, drawCels, runViewScript, 10031, 7, &where, 0, 0);
        helper = findView(parts[42]);
        if (helper) {
            setViewScript(helper, 10031, 1);
            helper->placed = fn_436321;
        }
        view->body.x += 17;
        view->body.y += 5;
        startSnoidScript((Snoid *)&view->body, 14006, 0, 1);
        view->notify = fn_435f3d;
        moveView(parts[42], 0, view->id);
        groupViews(parts[41], view->id, parts[42], 0, 0, 0);
    }
}

/* Draws the net's buttons (1-20, from the bank g_4b1598; each lit if it's
   the one chosen in its group of five, g_4b1540), or just button `which`,
   lit or not; stores the area drawn in *bounds. */
/* @zoombi32 0x0043f856 */
void fn_43f856(short which, short lit, ShortRect *bounds)
{
    ShortRect rect = g_4a337e;
    short i;
    short image;

    if (!which) {
        unionRect(&rect, &g_4a2efc[1].rect);
        unionRect(&rect, &g_4a2efc[20].rect);
        for (i = 0; i < 20; i++) {
            image = i + i + 1;
            if (i % 5 + 1 == g_4b1540[i / 5])
                image++;
            drawImageData((unsigned short *)(g_4b1598->offsets[image] + (char *)g_4b1598), g_4a2efc[i + 1].rect.left,
                          g_4a2efc[i + 1].rect.top, 8);
        }
    } else {
        rect = g_4a2efc[which].rect;
        i = which - 1;
        image = i + i + 1;
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4b1598->offsets[image] + (char *)g_4b1598), g_4a2efc[i + 1].rect.left,
                      g_4a2efc[i + 1].rect.top, 8);
    }
    if (bounds)
        *bounds = rect;
}

/* Stops a maze Zoombini (state 2) and moves on the Zoombinis in its
   line's list (by its word 33): those in pose 4 turn to their next
   direction (word 38) with a way open (words 34-37); those in pose 5 with
   a partner (word 43) list it in g_4b0930. */
/* @zoombi32 0x00439e55 */
void fn_439e55(short id)
{
    View *view = findView(id);
    short *list;
    short n;

    if (view) {
        ((Snoid *)&view->body)->unknownF4 = 2;
        short *parts = (short *)&view->body;

        parts[46] = 0;
        switch (parts[33]) {
        case 1:
            list = g_4b0a10;
            n = 0;
            break;
        case 2:
            list = g_4b0b6e;
            n = g_4b0d00;
            break;
        case 3:
            list = g_4b0ba0;
            n = g_4b0d02;
            break;
        case 4:
            list = g_4b0bd2;
            n = g_4b0d04;
            break;
        case 5:
            list = g_4b0c04;
            n = g_4b0d06;
            break;
        case 6:
            list = g_4b0c36;
            n = g_4b0d08;
            break;
        case 7:
            list = g_4b0c68;
            n = g_4b0d0a;
            break;
        case 8:
            list = g_4b0c9a;
            n = g_4b0d0c;
            break;
        default:
            list = g_4b0a10;
            n = 0;
            break;
        }
        while (n) {
            n--;
            view = findView(list[n]);
            if (view) {
                parts = (short *)&view->body;
                if (parts[30] == 4) {
                    parts[38]++;
                    if (parts[38] > 3)
                        parts[38] = 0;
                    while (!parts[34 + parts[38]]) {
                        parts[38]++;
                        if (parts[38] > 3)
                            parts[38] = 0;
                    }
                    ((Snoid *)&view->body)->unknownF4 = 3;
                }
                if (parts[30] == 5 && parts[43]) {
                    g_4b0930[g_4b09fe] = parts[43];
                    g_4b09fe++;
                    parts[43] = 0;
                }
            }
        }
    }
}

/* The scene's update: adds to the region to redraw the buttons whose state
   changed (24 with the cheat modifier, 26 and 21 with g_4b15a8, 21 with
   g_4b15aa, 22 when g_4b15ac asks). */
/* @zoombi32 0x0043fc9a */
void fn_43fc9a(View *, short region)
{
    if (!g_4b9684 && *(short *)(g_4a4ba0 + 0x48) < 625 && addModifierKeys(0) == 0x800) {
        if (!g_4b15a6) {
            g_4b15a6 = 1;
            unionRgnRect(region, &g_4a2efc[24].rect);
        }
    } else if (g_4b15a6) {
        g_4b15a6 = 0;
        unionRgnRect(region, &g_4a2efc[24].rect);
    }
    if (g_4b15a8) {
        if (!g_4a33b2) {
            g_4a33b2 = 1;
            unionRgnRect(region, &g_4a2efc[26].rect);
            unionRgnRect(region, &g_4a2efc[21].rect);
        }
    } else if (g_4a33b2) {
        g_4a33b2 = 0;
        unionRgnRect(region, &g_4a2efc[26].rect);
        unionRgnRect(region, &g_4a2efc[21].rect);
    }
    if (!g_4b15aa) {
        if (g_4a33b4) {
            g_4a33b4 = 0;
            unionRgnRect(region, &g_4a2efc[21].rect);
        }
    } else if (!g_4a33b4) {
        g_4a33b4 = 1;
        unionRgnRect(region, &g_4a2efc[21].rect);
    }
    if (g_4b15ac) {
        g_4b15ac = 0;
        unionRgnRect(region, &g_4a2efc[22].rect);
    }
}

/* Sends a Zoombini's marker flying from its place (g_4a2b7e, or g_4a2be2
   at the higher levels) to (484, 318) in six steps (fn_43e5a7), unless one
   is flying already. */
/* @zoombi32 0x0043e435 */
void fn_43e435(short n)
{
    short scripts[3] = {1, 0, 2};

    if (!g_4b1456) {
        g_4b119e = n;
        g_4b1456++;
        if (g_4b12ac <= 1) {
            g_4b1452 = g_4a2b7e[n].x;
            g_4b1454 = g_4a2b7e[n].y;
        } else {
            g_4b1452 = g_4a2be2[n].x;
            g_4b1454 = g_4a2be2[n].y;
        }
        g_4b117e = (484 - g_4b1452) / 6;
        g_4b1180 = (318 - g_4b1454) / 6;
        g_4b1452 = 484;
        g_4b1454 = 318;
        g_4b13ca++;
        g_4b13cc[g_4b13ca] = addView(0x4100000, drawCels, fn_43e5a7, scripts[g_4a2e58] + 7020, 6, 0, 0, 0);
        View *view = findView(g_4b13cc[g_4b13ca]);

        if (view) {
            view->unknown1e = n;
            g_4b1450++;
            view->placed = fn_43d70d;
            view->interval = 3;
            moveView(g_4b13cc[g_4b13ca], 0, g_4b12b6);
        }
    }
}

/* The flying marker's update: steps it on (g_4b1452/4 by g_4b117e/80),
   and after five steps (or when the game says) lands it (fn_43da30). */
/* @zoombi32 0x0043e5a7 */
void fn_43e5a7(View *view, short region)
{
    if (++g_4b1456 > 5 || *(short *)(g_4a4ba0 + 0x20)) {
        g_4b1456 = 0;
        view->update = runViewScript;
        fn_43da30(view->unknown1e);
    } else {
        g_4b1452 -= g_4b117e;
        g_4b1454 -= g_4b1180;
    }
    runViewCels(view, region);
    waitForEventFor(0, 2, 0, 1);
}
