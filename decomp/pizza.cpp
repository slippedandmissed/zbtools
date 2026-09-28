/*
 * pizza (0x4402c0-0x44695c): Pizza Pass (scene 9): the trolls 'Arno',
 * 'Willa', 'Shyler'
 */

#include "zoombinis.h"

/* A view's update: redraws button 2 when g_4b15e6 changes, and button 1
   once. */
/* @zoombi32 0x00441127 */
void fn_441127(View *, short region)
{
    if (g_4b15e6) {
        if (!g_4a3d98) {
            g_4a3d98 = 1;
            unionRgnRect(region, &pizzaButtons[2].rect);
        }
    } else if (g_4a3d98) {
        g_4a3d98 = 0;
        unionRgnRect(region, &pizzaButtons[2].rect);
    }
    if (!g_4a3d9a) {
        g_4a3d9a = 1;
        unionRgnRect(region, &pizzaButtons[1].rect);
    }
}

/* Draws button `which` (1: 5, 2: 2, or 1 if g_4b15e6 isn't set; the
   next image if lit) from the bank g_4a3d94, showing it if `show`. */
/* @zoombi32 0x00441071 */
void drawPizzaButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b15e6) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a3d94->offsets[image] + (char *)g_4a3d94), pizzaButtons[which].rect.left,
                      pizzaButtons[which].rect.top, 8);
        if (show)
            showRect(&pizzaButtons[which].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x00441199 */
void closePizza()
{
    g_4b755e = g_4b166c;
    if (g_4b15e4) {
        g_4b15e4 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a3d3c);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b15d0);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Shows the view g_4b162e with script 7001 on (by g_4b161e), adding it
   if need be, placed by fn_442443. */
/* @zoombi32 0x004423d7 */
void fn_4423d7()
{
    View *view = findView(g_4b162e);

    if (view) {
        setViewScript(view, g_4b161e + 7001, 1);
    } else {
        g_4b162e = addView(0x108000, drawCels, runViewScript, g_4b161e + 7001, 6, 0, 0, 0);
        view = findView(g_4b162e);
    }
    view->placed = fn_442443;
}

/* Sends the party's Zoombinis flagged (their word F7 is 1) to their
   places (g_4a3d54). */
/* Not exact: the original turns the view's register into the Snoid's in
   place (add esi, 0x30) where BCC computes it into eax. */
/* @zoombi32 0x004468eb */
void fn_4468eb()
{
    Point where;
    Point *places = g_4a3d54;
    short i;
    View *view;
    Snoid *snoid;

    g_4b1820 = 0;
    for (i = 0; i < g_4b15d4; i++) {
        where.x = places[i].x;
        where.y = places[i].y;
        if ((view = findView(partyViews[i])) != 0 && ((Snoid *)&view->body)->unknownF7 == 1) {
            snoid = (Snoid *)&view->body;
            setSnoidAction(snoid, 0, &where);
        }
    }
}

/* A random topping (of g_4b1624) that troll `troll` (0-2) wants. */
/* @zoombi32 0x00443316 */
short fn_443316(short troll)
{
    short n;

    switch (troll) {
    case 0:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!trollWants[0][n]);
        break;
    case 1:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!trollWants[1][n]);
        break;
    case 2:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!trollWants[2][n]);
        break;
    }
    return n;
}

/* Picks the toppings (g_4b1676) at random: each (of g_4b1624, but not the
   fifth at level 1) with chance g_4b1626 in 1000, until g_4b1628 have been
   picked; if none was, one of the first four. */
/* @zoombi32 0x0044410b */
void fn_44410b()
{
    short none;
    short skip;
    short left;
    short i;

    fillMemory(g_4b1676, 0, 16);
    none = 1;
    skip = -1;
    if (g_4b161e == 1)
        skip = 4;
    left = g_4b1628;
    do {
        for (i = 0; i < g_4b1624; i++)
            if ((short)randomUpTo(1000) < g_4b1626 && !g_4b1676[i] && i != skip) {
                g_4b1676[i]++;
                left--;
                none = 0;
            }
    } while (left > 0);
    if (none)
        g_4b1676[randomUpTo(3)]++;
}

/* Whether the toppings on the pizza (g_4b16da, as a set of eight bits) are
   one of the sets tried already (g_4b16ec, g_4b1708 + 1 of them). */
/* @zoombi32 0x0044460a */
short fn_44460a()
{
    short *toppings = g_4b16da;
    char set;
    short i;

    if (g_4b1708 < 0)
        return 0;
    set = 0;
    if (toppings[0])
        set |= 1;
    if (toppings[1])
        set |= 2;
    if (toppings[2])
        set |= 4;
    if (toppings[3])
        set |= 8;
    if (toppings[4])
        set |= 0x10;
    if (toppings[5])
        set |= 0x20;
    if (toppings[6])
        set |= 0x40;
    if (toppings[7])
        set |= 0x80;
    for (i = 0; i <= g_4b1708; i++)
        if (set == g_4b16ec[i])
            return 1;
    return 0;
}

/* Records the toppings on the pizza (g_4b16da) as a set tried (the next
   of g_4b16ec). */
/* @zoombi32 0x00444556 */
void fn_444556()
{
    short *toppings = g_4b16da;

    g_4b1708++;
    if (toppings[0])
        g_4b16ec[g_4b1708] |= 1;
    if (toppings[1])
        g_4b16ec[g_4b1708] |= 2;
    if (toppings[2])
        g_4b16ec[g_4b1708] |= 4;
    if (toppings[3])
        g_4b16ec[g_4b1708] |= 8;
    if (toppings[4])
        g_4b16ec[g_4b1708] |= 0x10;
    if (toppings[5])
        g_4b16ec[g_4b1708] |= 0x20;
    if (toppings[6])
        g_4b16ec[g_4b1708] |= 0x40;
    if (toppings[7])
        g_4b16ec[g_4b1708] |= 0x80;
}

/* Starts the view g_4a3d42 over (action 1) and puts it in front of the
   troll view that's up (g_4b160e, g_4b1610 or g_4b1612, unless
   g_4b1720), grouped. */
/* @zoombi32 0x0044509b */
void fn_44509b()
{
    View *view = findView(g_4a3d42);

    view->interval = 6;
    setSnoidAction((Snoid *)&view->body, 1, 0);
    if (!g_4b1720) {
        if (g_4b1618 == 1)
            moveView(g_4a3d42, 1, g_4b160e);
        else if (g_4b161a == 1)
            moveView(g_4a3d42, 1, g_4b1610);
        else if (g_4b161c == 1)
            moveView(g_4a3d42, 1, g_4b1612);
    } else {
        g_4b1720 = 0;
    }
    g_4b15f8 = groupViews(g_4a3d42, g_4a3d42, 0, 0, 0, 0);
}

/* A view's notify: unless busy (g_4b15f2, g_4b160a, g_4b1662), counts in
   g_4b165a once all the party is through (g_4b15d6), else shows the view
   g_4b162e (script 7067, or 7068 from level 1) placed by fn_442443. */
/* @zoombi32 0x00445ae1 */
void fn_445ae1(View *, short)
{
    View *view;

    if (!g_4b15f2 && !g_4b160a && !g_4b1662) {
        if (g_4b15d6 >= g_4b15d4) {
            g_4b165a++;
        } else {
            view = findView(g_4b162e);
            if (!g_4b161e)
                setViewScript(view, 7067, 1);
            else
                setViewScript(view, 7068, 1);
            g_4b160a = groupViews(g_4b162e, g_4b162e, 0, 0, 0, 0);
            view->placed = fn_442443;
        }
    }
}

/* A view's placing: drops the cels of toppings (images 57-61 and 67-69)
   not on the pizza (g_4b164a). `i` never moves. */
/* @zoombi32 0x00442443 */
void fn_442443(View *view)
{
    short *cel = (short *)&view->body;
    short i = 0;
    short removed;

    while (cel[i]) {
        removed = 0;
        switch (cel[i]) {
        case 61:
            if (!g_4b164a[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 60:
            if (!g_4b164a[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 59:
            if (!g_4b164a[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 58:
            if (!g_4b164a[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 57:
            if (!g_4b164a[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 67:
            if (!g_4b164a[5]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 68:
            if (!g_4b164a[6]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 69:
            if (!g_4b164a[7]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed)
            cel += 3;
    }
}

/* Starts the troll of the level (g_4b161e: Arno g_4b160e at 0, Willa
   g_4b1610 at 1, Shyler g_4b1612 from 2) on one of its scripts (8014,
   9019-9020 or 10001-10008). */
/* @zoombi32 0x004458c3 */
void fn_4458c3()
{
    View *view;

    g_4b1660 = 1;
    if (!g_4b161e) {
        view = findView(g_4b160e);
        setViewScript(view, 8014, 1);
        g_4b1600 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
    } else if (g_4b161e == 1) {
        view = findView(g_4b1610);
        setViewScript(view, randomUpTo(1) + 9019, 1);
        g_4b1602 = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
    } else if (g_4b161e >= 2) {
        view = findView(g_4b1612);
        setViewScript(view, randomUpTo(7) + 10001, 1);
        g_4b1604 = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
    }
}

/* Steps the trolls' turns on (g_4b165e: 1-4): each of the trolls there
   are at the level in turn (scripts 8032, 9034, 10038), then back to 0. */
/* @zoombi32 0x004459b3 */
void fn_4459b3()
{
    short *step = &g_4b165e;

    if (*step == 1) {
        setViewScript(findView(g_4b160e), 8032, 1);
        g_4b1608 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
        (*step)++;
    } else if (!g_4b161e && *step == 2) {
        *step = 0;
    } else if (g_4b161e >= 1 && *step == 2) {
        setViewScript(findView(g_4b1610), 9034, 1);
        g_4b1608 = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
        (*step)++;
    } else if (g_4b161e == 1 && *step == 3) {
        *step = 0;
    } else if (g_4b161e >= 2 && *step == 3) {
        setViewScript(findView(g_4b1612), 10038, 1);
        g_4b1608 = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
        (*step)++;
    } else if (g_4b161e >= 2 && *step == 4) {
        *step = 0;
    }
}
