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
short fn_44027b(short, short)
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
short zoombiniMadeAllowed()
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

/*
 * Steps a maze Zoombini on a square in its direction (word 20, set from
 * word 38 of the view `other`, which, if its word 39 is set, first turns
 * to its next open way (words 34-37; sound 5101/5102 in turn) and gets
 * pose 3). Lands on a square of kind 5 (g_4b061a): that square's view
 * gives up its partner (word 43, listed in g_4b0930), which takes the
 * direction. Then places it and starts its walking script (from word 21)
 * with its helper view's (script 10000 on), grouped. Reads `other`'s words
 * even when there's no such view.
 */
/* Not exact: the original keeps `other` in eax from the start (loading it
   before `view`), and evaluates startSnoidScript's script number before
   the Snoid's address; otherwise the same. */
/* @zoombi32 0x0043a2c8 */
void fn_43a2c8(View *view, short other)
{
    short *parts = (short *)&view->body;
    View *paired = findView(other);
    short *its;
    View *helper;

    if (paired)
        its = (short *)&paired->body;
    parts[20] = its[38];
    if (its[39]) {
        queueViewSound(g_4a2116 + 5101, 0);
        g_4a2116++;
        if (g_4a2116 > 1)
            g_4a2116 = 0;
        its[38]++;
        if (its[38] > 3)
            its[38] = 0;
        while (!its[34 + its[38]]) {
            its[38]++;
            if (its[38] > 3)
                its[38] = 0;
        }
        ((Snoid *)&paired->body)->unknownF4 = 3;
    }
    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34] = 0;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33] = 12;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34] = 12;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33] = 0;
        break;
    }
    short kind = g_4b061a[parts[33]][parts[34]];

    if (kind == 5) {
        paired = findView(g_4b04c8[parts[33]][parts[34]]);
        if (paired) {
            its = (short *)&paired->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                paired = findView(its[43]);
                its[43] = 0;
                if (paired) {
                    its = (short *)&paired->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = fn_436321;
    }
    startSnoidScript((Snoid *)&view->body, parts[21 + parts[20]], 0, 0);
    view->notify = fn_43638b;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
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
   the feature chosen in its group of five for the Zoombini being made), or
   just button `which`, lit or not; stores the area drawn in *bounds. */
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
            if (i % 5 + 1 == g_4b1484.features[i / 5])
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
            view->placed = markerPlaced;
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

/* Lands the flying marker for place n: shows it (script 7023, or 7024 at
   the higher levels) there, and counts that place's group (g_4b11aa, by
   g_4b119e) into g_4b0e6c; a place with none counts in g_4b144e. */
/* @zoombi32 0x0043da30 */
void fn_43da30(short n)
{
    View *view;

    if (n >= 0) {
        if (g_4b12ac <= 1) {
            g_4b1452 = g_4a2b7e[n].x;
            g_4b1454 = g_4a2b7e[n].y;
        } else {
            g_4b1452 = g_4a2be2[n].x;
            g_4b1454 = g_4a2be2[n].y;
        }
        g_4b1450++;
        view = findView(g_4b13cc[g_4b13ca]);
        if (view) {
            if (g_4b12ac < 2)
                setViewScript(view, 7023, 1);
            else
                setViewScript(view, 7024, 1);
        } else {
            if (g_4b12ac < 2)
                g_4b13cc[g_4b13ca] = addView(0x4108000, drawCels, runViewScript, 7023, 6, 0, 0, 0);
            else
                g_4b13cc[g_4b13ca] = addView(0x4108000, drawCels, runViewScript, 7024, 6, 0, 0, 0);
            view = findView(g_4b13cc[g_4b13ca]);
        }
        if (view) {
            view->placed = markerPlaced;
            moveView(g_4b13cc[g_4b13ca], 0, g_4b12ba[0]);
        }
        g_4b144e = 0;
        if ((g_4b145a = g_4b11aa[g_4b119e]) < 1) {
            g_4b145a = 0;
            g_4b1466 = 0;
            g_4b144e++;
        } else {
            g_4b147c++;
        }
        g_4b0e6c += g_4b145a;
        g_4b11aa[n] = -1;
    }
}

/* Steps a maze Zoombini one square on in its direction (word 20; within
   the 13 by 13 board); a square of kind 5 there hands its partner on (to
   g_4b0930, turned the same way). Then puts it there with its helper view
   (script 10000 on) and starts its script. */
/* @zoombi32 0x00439cb4 */
void fn_439cb4(View *view)
{
    short *parts = (short *)&view->body;
    short *its;
    View *helper;

    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34]++;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33]--;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34]--;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33]++;
        break;
    }
    short kind = g_4b061a[parts[33]][parts[34]];

    if (kind == 5) {
        View *other = findView(g_4b04c8[parts[33]][parts[34]]);

        if (other) {
            its = (short *)&other->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                View *partner = findView(its[43]);

                its[43] = 0;
                if (partner) {
                    its = (short *)&partner->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = fn_436321;
    }
    startSnoidScript((Snoid *)&view->body, parts[21 + parts[20]], 0, 0);
    view->notify = fn_43638b;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* As fn_439cb4 for a Zoombini reaching a turning view (`other`): if its
   feature (the view's word 41) is the view's (word 42), it turns the
   view's way (word 38) first; it stops at the board's edge. */
/* Not exact: the original keeps `other` in eax from the start (loading it
   before `snoid` and `parts`); this loads it at the call. */
/* @zoombi32 0x0043a0e8 */
void fn_43a0e8(View *view, short other)
{
    Snoid *snoid;
    short *parts;
    short *its;
    View *helper;
    View *turning;

    snoid = (Snoid *)&view->body;
    parts = (short *)&view->body;
    turning = findView(other);
    if (turning)
        its = (short *)&turning->body;
    if (snoid->features[its[41] - 1] == its[42])
        parts[20] = its[38];
    parts[31] = parts[33];
    parts[32] = parts[34];
    switch (parts[20]) {
    case 0:
        parts[34]--;
        if (parts[34] < 0)
            parts[34] = 0;
        break;
    case 1:
        parts[33]++;
        if (parts[33] > 12)
            parts[33] = 12;
        break;
    case 2:
        parts[34]++;
        if (parts[34] > 12)
            parts[34] = 12;
        break;
    case 3:
        parts[33]--;
        if (parts[33] < 0)
            parts[33] = 0;
        break;
    }
    short kind = g_4b061a[parts[33]][parts[34]];

    if (kind == 5) {
        View *square = findView(g_4b04c8[parts[33]][parts[34]]);

        if (square) {
            its = (short *)&square->body;
            if (its[43]) {
                g_4b0930[g_4b09fe] = its[43];
                g_4b09fe++;
                View *partner = findView(its[43]);

                its[43] = 0;
                if (partner) {
                    its = (short *)&partner->body;
                    its[20] = parts[20];
                }
            }
        }
    }
    *(Point *)&view->body.x = (g_4afbf0 + parts[32])[parts[31] * 13];
    view->body.x += 4;
    view->body.y += -38;
    helper = findView(parts[41]);
    if (helper) {
        setViewScript(helper, parts[20] + 10000, 1);
        helper->body.x = view->body.x;
        helper->body.y = view->body.y;
        helper->placed = fn_436321;
    }
    short script = parts[21 + parts[20]];

    startSnoidScript((Snoid *)&view->body, script, 0, 0);
    view->notify = fn_43638b;
    if (helper)
        groupViews(view->id, helper->id, 0, 0, 0, 0);
}

/* Draws the box at the top right with two codes ("SC", "SH" or "MC", by
   g_4b1178 and g_4b117a) and a third (by g_4b117c from level 2, else
   "PR"). */
/* @zoombi32 0x0043d524 */
void fn_43d524()
{
    ShortRect whole = {500, 1, 600, 27};
    ShortRect left = {500, 1, 549, 27};
    ShortRect right = {550, 1, 600, 27};
    Color saved;
    char names[3][3] = {"SC", "SH", "MC"};

    saved = setForeColor(Color(0xb));
    fillPortRect(Rect(whole), Color(0xe), 0);
    frameRect(Rect(whole));
    drawText(Rect(left), 0x22, names[g_4b1178], 0xffff);
    drawText(Rect(whole), 0x22, names[g_4b117a], 0xffff);
    if (g_4b12ac >= 2)
        drawText(Rect(right), 0x22, names[g_4b117c], 0xffff);
    else
        drawText(Rect(right), 0x22, "PR", 0xffff);
    setForeColor(saved);
    showRect(&whole);
}

/* Adds the scene's standing views: five at 8000 on (g_4b12ba), 8005, the
   guide (9151, or 9153 at the higher levels; grouped), 7018, 10018, the
   three code views (10002, 10007 and 10012 on, by g_4b143e/42/46; the
   first from level 2) and 7000 (grouped). */
/* @zoombi32 0x0043c6df */
void fn_43c6df()
{
    short i;

    for (i = 0; i < 5; i++)
        g_4b12ba[i] = addView(0x4188000, drawCels, runViewScript, i + 8000, 6, 0, 0, 0);
    g_4b12b8 = addView(0x4188000, drawCels, runViewScript, 8005, 6, 0, 0, 0);
    if (g_4b12ac <= 1)
        g_4b12ca = addView(0x4108000, drawCels, runViewScript, 9151, 6, 0, 0, 0);
    else
        g_4b12ca = addView(0x4108000, drawCels, runViewScript, 9153, 6, 0, 0, 0);
    g_4b141e = groupViews(g_4b12ca, g_4b12ca, 0, 0, 0, 0);
    g_4b12b6 = addView(0x4181000, drawCels, runViewScript, 7018, 6, 0, 0, 0);
    g_4b13fe = addView(0x188000, drawCels, runViewScript, 10018, 6, 0, 0, 0);
    if (g_4b12ac >= 2)
        g_4b1400 = addView(0x4108000, drawCels, runViewScript, g_4b143e + 10002, 6, 0, 0, 0);
    g_4b1402 = addView(0x4108000, drawCels, runViewScript, g_4b1442 + 10007, 6, 0, 0, 0);
    g_4b1404 = addView(0x4108000, drawCels, runViewScript, g_4b1446 + 10012, 6, 0, 0, 0);
    g_4b13c6 = addView(0x4188000, drawCels, runViewScript, 7000, 6, 0, 0, 0);
    g_4b142a = groupViews(g_4b13c6, g_4b13c6, 0, 0, 0, 0);
}

/*
 * The queue of places (g_4b75ee, at g_4a3324): with `where`, gives the
 * first free place and its number; otherwise moves the Zoombinis up into
 * each free place from up to five places behind (by where the place is
 * in its row of five), one at a time.
 */
/* @zoombi32 0x0043ffd5 */
void fn_43ffd5(Point *where, short *slot)
{
    short d1;
    short d2;
    short d3;
    short d4;
    short waiting;
    short i;
    short step;
    Snoid *snoid;

    if (where) {
        for (i = 0; i < 16; i++)
            if (!g_4b75ee[i]) {
                *where = g_4a3324[i];
                *slot = i;
                return;
            }
        return;
    }
    g_4b7564 = 1;
    for (i = 0; i < 15; i++)
        if (!g_4b75ee[i]) {
            step = d1 = d2 = d3 = d4 = 0;
            switch (i) {
            case 0:
                step = 1;
                d1 = 2;
                d2 = 3;
                d3 = 4;
                d4 = 5;
                break;
            case 1:
            case 6:
            case 11:
                step = 1;
                d1 = 2;
                d2 = 3;
                d3 = 4;
                break;
            case 2:
            case 7:
            case 12:
                step = 1;
                d1 = 2;
                d2 = 3;
                break;
            case 3:
            case 8:
            case 13:
                step = 1;
                d1 = 2;
                break;
            case 4:
            case 9:
            case 14:
                step = 1;
                break;
            }
            waiting = 1;
            while (step && waiting) {
                if (g_4b75ee[i + step]) {
                    snoid = findSnoid(g_4b75ee[i + step], 1);
                    if (snoid) {
                        snoid->unknownEa = -1;
                        if (i + step < 17) {
                            *(Point *)&snoid->targetX = g_4a3324[i];
                            setSnoidAction(snoid, 7, 0);
                        } else {
                            snoid->targetX = 326;
                            snoid->targetY = 390;
                            setSnoidAction(snoid, 7, 0);
                            updateSnoidView(snoidView(snoid), removedRgn);
                            *(Point *)&snoid->targetX = g_4a3324[i];
                        }
                        g_4b75ee[i] = g_4b75ee[i + step];
                        g_4b75ee[i + step] = 0;
                        waiting = 0;
                    }
                }
                step = d1;
                d1 = d2;
                d2 = d3;
                d3 = d4;
                d4 = 0;
            }
        }
    updateViews();
    g_4b7564 = 0;
}

/* Scene 3's keys: 23 brings back the two views g_4b15b0 and g_4b15b2
   (adding them first if the game state's +0x20 is set). Returns whether it
   handled the key. */
/* @zoombi32 0x0043ec8c */
short fn_43ec8c(unsigned short key)
{
    short handled = 0;

    switch (key) {
    case 23:
        if (*(short *)(g_4a4ba0 + 0x20))
            fn_440218();
        fn_43ff1d(0);
        handled = 1;
        break;
    }
    return handled;
}

/* Leaves the scene if asked to (g_4b0d52 names where to): returns whether
   it did. */
/* @zoombi32 0x0043ee2a */
short leaveNetIfAsked()
{
    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        fn_43eb13();
        return 1;
    }
    return 0;
}

/* Scene 3's frame: leaving once asked to (g_4b0d52) and sound 996 is
   done; g_4b15b8 is cleared while g_4b755a isn't set. */
/* @zoombi32 0x0043ebf4 */
void netIdle()
{
    if (!netBusy && g_4b15a4) {
        netBusy = 1;
        updateViews();
        if (g_4b0d52) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                netBusy = 0;
                return;
            }
            if (viewsLocked || !g_4b755a || g_4b755c >= 1)
                leaveNetIfAsked();
        }
        if (!g_4b755a && g_4b15b8)
            g_4b15b8 = 0;
        netBusy = 0;
    }
}

/* Draws a Zoombini from its parts in the bank g_4b159c where it stands:
   feet, body, eyes, nose, then hair. */
/* @zoombi32 0x0043f9d3 */
void drawZoombiniParts(Snoid *snoid)
{
    short x = snoid->body.x;
    short y = snoid->body.y;

    if (snoid->features[3])
        fn_43f985(snoid->features[3] + 16, x, y);
    fn_43f985(1, x, y);
    if (snoid->features[1])
        fn_43f985(snoid->features[1] + 6, x, y);
    if (snoid->features[2])
        fn_43f985(snoid->features[2] + 11, x, y);
    if (snoid->features[0])
        fn_43f985(snoid->features[0] + 1, x, y);
}

/* Scene 15's keys (debugging ones only while debugging messages are on). */
/* @zoombi32 0x0043c655 */
short netKey(unsigned short key)
{
    if (!g_4b8803 && key != 367)
        return 0;
    switch (key) {
    case 367:
        fn_466b93();
        return 1;
    case 'L':
    case 'l':
        fn_43d524();
        return 1;
    case ' ':
        g_4b140a = g_4b140e;
        g_4b140c = 17 - g_4b140a;
        if (g_4b142c) {
            g_4b142c = 0;
            g_4b1410++;
        }
        return 1;
    }
    return 0;
}

/* Draws the panel's buttons (1-7, from the bank g_4b15a0; some lit, some
   greyed by the scene's state), or just button `which`, lit or not; the
   second shows the Zoombini being made, the third its name (g_4b157d, if
   g_4b15aa). Shows the area drawn if `show`. */
/* @zoombi32 0x0043f5ea */
void drawNetPanel(short which, short lit, short show)
{
    short y;
    short count;
    ShortRect rect;
    ShortRect bounds = g_4a3376;
    Color saved;
    short i;
    short x;
    short image;

    if (!which) {
        i = 0;
        count = 7;
        bounds = g_4a31cc[1].rect;
        unionRect(&bounds, &g_4a31cc[7].rect);
    } else {
        i = which - 1;
        count = i + 1;
        bounds = g_4a31cc[which].rect;
    }
    for (; i < count; i++) {
        x = g_4a31cc[i + 1].rect.left;
        y = g_4a31cc[i + 1].rect.top;
        image = 0;
        switch (i) {
        case 0:
            image = 2;
            if (*(short *)(g_4a4ba0 + 0x48) >= 625 || g_4b15a8 || !g_4b15aa) {
                lit = 0;
                image = 1;
            }
            break;
        case 1:
            drawZoombiniParts(&g_4b1484);
            break;
        case 2:
            if (g_4b9684 & 0x10)
                return;
            drawImageData((unsigned short *)(g_4b15a0->offsets[13] + (char *)g_4b15a0), x, y, 8);
            rect = g_4a31cc[i + 1].rect;
            saved = setForeColor(Color(45));
            if (g_4b15aa) {
                rect.top++;
                rect.left += 4;
                drawText(rect, 0x22, g_4b157d, 0xffff);
                rect.top--;
                rect.left -= 4;
            }
            setForeColor(saved);
            break;
        case 3:
            image = 4;
            if (!g_4b15a8 && g_4b15a6)
                image = 6;
            break;
        case 4:
            image = 11;
            break;
        case 5:
            image = 9;
            if (!g_4b15a8) {
                lit = 0;
                image = 8;
            }
            break;
        }
        if (image) {
            if (lit)
                image++;
            drawImageData((unsigned short *)(g_4b15a0->offsets[image] + (char *)g_4b15a0), x, y, 8);
        }
    }
    if (show)
        showRect(&bounds);
}

/* Counts the Zoombini being made (g_4b1484) in or out of the numbers of
   each kind (at most two), noting in g_4b15aa whether it's complete (and,
   when adding, not one too many). */
/* @zoombi32 0x0043fdcc */
void countZoombiniMade(short add)
{
    Snoid *made = &g_4b1484;
    short i;
    short hair;
    short eyes;
    short nose;
    short feet;

    if (add) {
        g_4b15aa = zoombiniMadeAllowed();
    } else {
        g_4b15aa = 1;
        for (i = 0; i < 4; i++)
            if (!made->features[i])
                g_4b15aa = 0;
    }
    if (g_4b15aa) {
        hair = made->features[0] - 1;
        eyes = made->features[1] - 1;
        nose = made->features[2] - 1;
        feet = made->features[3] - 1;
        if (add) {
            if (zoombiniCounts()[hair][eyes][nose][feet] < 2)
                zoombiniCounts()[hair][eyes][nose][feet]++;
        } else if (zoombiniCounts()[hair][eyes][nose][feet] > 0) {
            zoombiniCounts()[hair][eyes][nose][feet]--;
        }
    }
}

/*
 * Picks features for the Zoombini being made: random ones (all of them if
 * `rename`, else those not chosen) until it's a kind with fewer than two,
 * and after 64 tries the last such kind in order (the search never stops
 * early: its flag is never set). Names it if `rename` or the first pick
 * failed.
 */
/* @zoombi32 0x0043fb0f */
void pickZoombiniMade(short rename)
{
    short tries = 0;
    short found;
    short hair;
    short eyes;
    short nose;
    short feet;
    short i;

    g_4b15aa = 0;
    while (!g_4b15aa && tries < 64) {
        tries++;
        if (tries >= 64) {
            found = 0;
            rename = 1;
            for (hair = 0; !found && hair < 5; hair++)
                for (eyes = 0; !found && eyes < 5; eyes++)
                    for (nose = 0; !found && nose < 5; nose++)
                        for (feet = 0; !found && feet < 5; feet++)
                            if (zoombiniCounts()[hair][eyes][nose][feet] < 2) {
                                g_4b1484.features[0] = hair + 1;
                                g_4b1484.features[1] = eyes + 1;
                                g_4b1484.features[2] = nose + 1;
                                g_4b1484.features[3] = feet + 1;
                            }
        } else {
            for (i = 0; i < 4; i++)
                if (!g_4b1484.features[i] || rename)
                    g_4b1484.features[i] = randomBetween(1, 5);
        }
        g_4b15aa = zoombiniMadeAllowed();
        if (!g_4b15aa)
            rename = 1;
    }
    if (rename)
        makeName(g_4b157d, 10);
    g_4b15ac = 1;
}

/* The view drawing the panel and the net's buttons. */
/* @zoombi32 0x0043fc7d */
void drawNetButtonsView(View *)
{
    drawNetPanel(0, 0, 0);
    fn_43f856(0, 0, 0);
}

/* A net button clicked (1-20, in four groups of five): picks that feature
   for the Zoombini being made (sound 1000), or drops it if already picked
   (1004), or refuses (1008, never: fn_44027b allows every feature). Stops sound
   g_4b15b6 first, and redraws the panel's third button if the Zoombini's
   completeness changes. */
/* @zoombi32 0x0043ecbb */
void netButtonClicked(short button)
{
    ShortRect rect;
    short group;
    short chosen;

    if (leaveNetIfAsked())
        return;
    if (g_4b15b6 && isSoundPlaying(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        stopSounds(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        g_4b15b6 = 0;
    }
    rect = g_4a2efc[button].rect;
    group = 0;
    while (button >= 6) {
        button -= 5;
        group++;
    }
    chosen = g_4b1484.features[group];
    if (chosen && button == chosen) {
        queueViewSound(1004, 0);
        chosen += group * 5;
        fn_43f856(chosen, 0, &rect);
        showRect(&rect);
        g_4b1484.features[group] = 0;
    } else if (fn_44027b(group, button - 1)) {
        if (chosen) {
            chosen += group * 5;
            fn_43f856(chosen, 0, &rect);
            showRect(&rect);
        }
        queueViewSound(1000, 0);
        fn_43f856(group * 5 + button, 1, &rect);
        showRect(&rect);
        g_4b1484.features[group] = button;
    } else {
        queueViewSound(1008, 0);
    }
    short allowed = zoombiniMadeAllowed();

    if (allowed != g_4b15aa) {
        g_4b15aa = allowed;
        drawNetPanel(3, 1, 1);
    }
    g_4b15ac = 1;
}

/*
 * Which entry (of g_4b142e) of the tables g_4b0e78 and g_4b0f72 (and, from
 * level 2 (g_4b12ac), g_4b106c) holds the codes g_4b1446 and g_4b1442 (and
 * g_4b143e), in the order g_4b1178 (levels 0-1) or g_4b1468 (from level 2)
 * says; -1 if none.
 */
/* Not exact: the original keeps all four table pointers in registers (edx,
   ecx, esi, edi) with no stack frame; BCC leaves one on the stack, in any
   declaration order. */
/* @zoombi32 0x0043dbf3 */
short findCodeEntry()
{
    short *count = &g_4b142e;
    short *first = g_4b0e78;
    short *code = &g_4b1446;
    short *second = g_4b0f72;
    short i;

    switch (g_4b12ac) {
    case 0:
    case 1:
        if (g_4b1178 == 2) {
            for (i = 0; i < *count; i++)
                if (first[i] == *code && second[i] == g_4b1442)
                    return i;
        } else {
            for (i = 0; i < *count; i++)
                if (second[i] == *code && first[i] == g_4b1442)
                    return i;
        }
        break;
    case 2:
    case 3:
        if (g_4b1468 == 0) {
            for (i = 0; i < *count; i++)
                if (first[i] == *code && second[i] == g_4b1442 && g_4b106c[i] == g_4b143e)
                    return i;
        } else if (g_4b1468 == 1) {
            for (i = 0; i < *count; i++)
                if (first[i] == g_4b1442 && second[i] == *code && g_4b106c[i] == g_4b143e)
                    return i;
        } else if (g_4b1468 == 2) {
            for (i = 0; i < *count; i++)
                if (first[i] == g_4b143e && second[i] == *code && g_4b106c[i] == g_4b1442)
                    return i;
        } else if (g_4b1468 == 3) {
            for (i = 0; i < *count; i++)
                if (first[i] == *code && second[i] == g_4b143e && g_4b106c[i] == g_4b1442)
                    return i;
        } else if (g_4b1468 == 4) {
            for (i = 0; i < *count; i++)
                if (first[i] == g_4b1442 && second[i] == g_4b143e && g_4b106c[i] == *code)
                    return i;
        } else if (g_4b1468 == 5) {
            for (i = 0; i < *count; i++)
                if (first[i] == g_4b143e && second[i] == g_4b1442 && g_4b106c[i] == *code)
                    return i;
        }
        break;
    }
    return -1;
}

/*
 * Plays a scene's ambient sounds: every 3-4 seconds, unless the last is
 * still playing, a random one of the scene's (none in scenes 6 and 14),
 * not repeating one until all have played; every 16th time, first unloads
 * sounds 900-944.
 */
/* @zoombi32 0x0043af6b */
void playAmbientSound()
{
    unsigned long now;
    short sound;
    short i;

    if (g_4b87fe && g_4b87ff) {
        now = clockTime();
        if (now >= ambientSoundTime) {
            if (isSoundPlaying(ambientSound, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                ambientSoundTime = now + randomBetween(180, 240);
                return;
            }
            ambientSoundTime = now + randomBetween(180, 240);
            sound = 0;
            switch (currentScene) {
            case 7:
                sound = scene7Sounds[allocateSlot(&scene7SoundsUsed, 9, 0)];
                break;
            case 8:
                sound = scene8Sounds[allocateSlot(&scene8SoundsUsed, 9, 0)];
                break;
            case 9:
                sound = scene9Sounds[allocateSlot(&scene9SoundsUsed, 12, 0)];
                break;
            case 4:
                sound = scene4Sounds[allocateSlot(&scene4SoundsUsed, 15, 0)];
                break;
            case 10:
                sound = scene10Sounds[allocateSlot(&scene10SoundsUsed, 19, 0)];
                break;
            case 11:
                sound = scene11Sounds[allocateSlot(&scene11SoundsUsed, 20, 0)];
                break;
            case 12:
                sound = scene12Sounds[allocateSlot(&scene12SoundsUsed, 13, 0)];
                break;
            case 5:
                sound = scene5Sounds[allocateSlot(&scene5SoundsUsed, 10, 0)];
                break;
            case 13:
                sound = scene13Sounds[allocateSlot(&scene13SoundsUsed, 13, 0)];
                break;
            case 15:
                sound = scene15Sounds[allocateSlot(&scene15SoundsUsed, 17, 0)];
                break;
            case 16:
                sound = scene16Sounds[allocateSlot(&scene16SoundsUsed, 10, 0)];
                break;
            case 18:
                sound = scene18Sounds[allocateSlot(&scene18SoundsUsed, 10, 0)];
                break;
            case 17:
                sound = scene17Sounds[allocateSlot(&scene17SoundsUsed, 10, 0)];
                break;
            }
            if (sound) {
                ambientSoundCount++;
                ambientSoundCount %= 16;
                if (!ambientSoundCount)
                    for (i = 900; i <= 944; i++)
                        fn_41158c(i, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                queueViewSound(sound, 0);
                ambientSound = sound;
            }
        }
    }
}

/*
 * Moves on to the next scene (g_4b0d50): leaving a group's last puzzle
 * (9, 12, 15, 18) for its camp notes the level passed in the game state
 * (g_4b0d4c: the puzzle left). Whether to go by the map (scene 2) first
 * depends on the scenes left and entered (not while g_4b754a, g_4b7562 or
 * g_4b0d4a; always while g_4a7e68). Then unlocks the views, sets the next
 * ambient sound 15 seconds off, clips to the game's area and opens the
 * scene.
 */
/* Not exact: the original keeps `scene` in esi, `next` in edi and `viaMap`
   in ebx; BCC rotates them (ebx, esi, edi) whatever the declaration
   order. */
/* @zoombi32 0x0043ac20 */
void enterNextScene()
{
    short *scene = &currentScene;
    short *next = &g_4b0d50;
    short viaMap = 0;

    if (*next == -1)
        return;
    if (!g_4b754a) {
        short level = sceneLevel();

        if (g_4b7558 && level)
            level--;
        level &= 3;
        short bit = 1 << level;

        switch (*scene) {
        case 9:
            if (*next == 4) {
                g_4b0d4c = 9;
                g_4a4ba0[0x50] |= bit;
            }
            break;
        case 12:
            if (*next == 5) {
                g_4b0d4c = 12;
                *(short *)(g_4a4ba0 + 0x52) |= (char)bit;
            }
            break;
        case 15:
            if (*next == 5) {
                g_4b0d4c = 15;
                *(short *)(g_4a4ba0 + 0x52) |= bit << 4;
            }
            break;
        case 18:
            if (*next == 6) {
                g_4b0d4c = 18;
                g_4a4ba0[0x51] |= bit;
            }
            break;
        }
    }
    if (g_4b754a) {
        viaMap = 0;
        if (*scene != 1 && *next != 3 && *next != 0)
            *next = 1;
    } else if (*scene != 1 && *scene != 2 && *scene != 6 && *next != 1) {
        switch (*scene) {
        case 0:
            viaMap = 0;
            break;
        case 3:
            viaMap = 1;
            break;
        case 7:
        case 8:
        case 9:
            viaMap = 1;
            break;
        case 4:
            viaMap = 1;
            break;
        case 10:
        case 11:
        case 12:
            viaMap = 1;
            break;
        case 13:
        case 14:
        case 15:
            viaMap = 1;
            break;
        case 5:
            viaMap = 1;
            break;
        case 16:
        case 17:
        case 18:
            viaMap = 1;
            break;
        }
    }
    *(short *)(g_4a4ba0 + 0xca) = *scene;
    if (*next != 0 && *next != 2)
        savedScene() = *next;
    if (g_4b7562 || g_4b0d4a)
        viaMap = 0;
    if (g_4a7e68)
        viaMap = 1;
    if (viaMap) {
        g_4b0d56 = *scene;
        g_4b0d54 = *next;
        *scene = 2;
        *next = -1;
    } else {
        g_4b0d56 = *scene;
        *scene = *next;
        *next = -1;
        g_4b0d54 = -1;
    }
    if (g_4b754a) {
        *(short *)(g_4a4ba0 + 0x54) = 0;
    } else {
        if (!viewsLocked)
            g_4afb32 = 1;
        switch (*scene) {
        case 7:
        case 10:
        case 13:
        case 16:
            *(short *)(g_4a4ba0 + 0x54) = 1;
            break;
        }
    }
    viewsLocked = 0;
    viewsPaused = fillViews = 0;
    ambientSoundTime = clockTime() + 900;
    setClipRect(gameRect);
    if (scenes[*scene]->open)
        scenes[*scene]->open();
}

/*
 * The marker's placing: turns each of its cels' images (1-184) into the
 * ones for the codes chosen (g_4b143e, g_4b1442 and g_4b1446, and their
 * second parts g_4b1440, g_4b1444 and g_4b1448; -1: none), by two tables
 * of five; with g_4b1450 set, also moves the cels to the marker's place
 * (g_4b1452, g_4b1454). A cel below 1 stops it there for good.
 */
/* @zoombi32 0x0043d70d */
void markerPlaced(View *view)
{
    volatile short row2;
    volatile short col1b;
    volatile short row2b;
    volatile short row0b;
    short columns[5] = {2, 3, 0, 1, 4};
    short rows[5] = {4, 0, 2, 1, 3};
    short col1;
    short row0;
    short *cels;
    short i;

    col1 = row2 = row0 = -1;
    col1b = row2b = row0b = -1;
    if (g_4b1442 != -1)
        col1 = columns[g_4b1442];
    if (g_4b1446 != -1)
        row2 = rows[g_4b1446];
    if (g_4b143e != -1)
        row0 = rows[g_4b143e];
    if (g_4b1444 != -1)
        col1b = columns[g_4b1444];
    if (g_4b1448 != -1)
        row2b = rows[g_4b1448];
    if (g_4b1440 != -1)
        row0b = rows[g_4b1440];
    cels = (short *)&view->body;
    i = 0;
    while (cels[i]) {
        if (cels[i] < 1)
            continue;
        if (cels[i] < 185) {
            if (cels[i] < 6 && col1b >= 0 && row0b != -1)
                cels[i] = row0b * 12 + col1b + 6;
            else if (cels[i] < 6 && col1b != -1 && row0b == -1)
                cels[i] = col1b + 1;
            else if (cels[i] >= 6 && cels[i] < 11 && col1 >= 0 && row0 != -1)
                cels[i] += row0 * 12 + col1;
            else if (cels[i] >= 6 && cels[i] < 11 && col1 != -1)
                cels[i] = col1 + 1;
            else if (cels[i] >= 11 && cels[i] < 18 && row0 != -1)
                cels[i] += row0 * 12;
            else if (cels[i] >= 66 && cels[i] < 88 && row2 >= 0)
                cels[i] += row2 * 22;
            else if (cels[i] && cels[i] >= 176 && row2b != -1)
                cels[i] = row2b * 22 + 66;
            if (g_4b1450) {
                if (!i) {
                    cels[i + 1] = g_4b1452;
                    cels[i + 2] = g_4b1454;
                } else if (!g_4b1456) {
                    if (g_4b12ac < 2)
                        cels[i + 1] = g_4b1452 + 21;
                    else
                        cels[i + 1] = g_4b1452 + 3;
                    cels[i + 2] = g_4b1454 + 7;
                } else {
                    cels[i + 1] = g_4b1452 + 4;
                    cels[i + 2] = g_4b1454 + 3;
                }
            }
        }
        i += 3;
    }
}
