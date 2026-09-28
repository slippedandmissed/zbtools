/*
 * pizza (0x4402c0-0x44695c): Pizza Pass (scene 9): the trolls 'Arno',
 * 'Willa', 'Shyler'
 */

#include <stdio.h>

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
    short i;
    View *view;
    Snoid *snoid;

    g_4b1820 = 0;
    for (i = 0; i < g_4b15d4; i++) {
        where.x = g_4a3d54[i].x;
        where.y = g_4a3d54[i].y;
        if ((view = findView(partyViews[i])) != 0 && ((Snoid *)&view->body)->unknownF7 == 1) {
            snoid = (Snoid *)&view->body;
            setSnoidAction(snoid, 0, &where);
        }
    }
}

/* A random topping (of g_4b1624) that troll `troll` (0-2: Arno, Willa, Shyler)
   wants. */
/* @zoombi32 0x00443316 */
short fn_443316(short troll)
{
    short n;

    switch (troll) {
    case 0:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!arnoWants[n]);
        break;
    case 1:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!willaWants[n]);
        break;
    case 2:
        do
            n = randomUpTo(g_4b1624 - 1);
        while (!shylerWants[n]);
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
   not on the pizza (g_4b164a-g_4b1658). `i` never
   moves. */
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
            if (!g_4b164a) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 60:
            if (!g_4b164c) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 59:
            if (!g_4b164e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 58:
            if (!g_4b1650) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 57:
            if (!g_4b1652) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 67:
            if (!g_4b1654) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 68:
            if (!g_4b1656) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 69:
            if (!g_4b1658) {
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

/* The view drawing the two buttons. */
/* @zoombi32 0x0044110a */
void drawPizzaButtonsView(View *)
{
    drawPizzaButton(1, 0, 0);
    drawPizzaButton(2, 0, 0);
}

/* Brings the next of the party (g_4b15d6) up to the pizza spot (g_4a3d44)
   when the last one's done (g_4b15da), unless busy; counts in g_4b165a
   once they've all been. */
/* Not exact: BCC keeps g_4b15d6's address in esi (see docs/findings.md on
   cached global addresses); the original addresses it directly. */
/* @zoombi32 0x00445789 */
void fn_445789()
{
    View *view;

    if (!g_4b165a && !g_4b1662 && (g_4b15d6 == -1 || g_4b15da)) {
        g_4b15da = 0;
        if (++g_4b15d6 >= g_4b15d4) {
            g_4b165a++;
            return;
        }
        g_4b15d8 = 0;
        if (g_4b15d6 < 0) {
            g_4b15d6 = 0;
        } else {
            view = findView(partyViews[g_4b15d6]);
            if (view->body.x == g_4a3d44.x) {
                g_4a3d42 = view->id;
                return;
            }
        }
        if (g_4b15d6 < g_4b15d4) {
            view = findView(partyViews[g_4b15d6]);
            if (!view)
                return;
            setSnoidAction((Snoid *)&view->body, 7, 0);
            *(Point *)&((Snoid *)&view->body)->targetX = g_4a3d44;
            view->interval = 2;
            g_4a3d42 = view->id;
            g_4b15fa = groupViews(g_4a3d42, g_4a3d42, 0, 0, 0, 0);
            if (g_4b15ee)
                g_4b171e++;
        } else {
            g_4b165a++;
        }
    }
}

/* Puts the Zoombini at the pizza (g_4a3d42) and the trolls up (g_4b160e,
   g_4b1610, g_4b1612, as g_4b1618-g_4b161c say) in front of each other,
   behind g_4b1616; and the view g_4b1664 too. */
/* @zoombi32 0x00446035 */
void fn_446035()
{
    if (g_4b1664) {
        moveView(g_4b1664, 1, g_4b1616);
        if (g_4b1618 == 3) {
            if (g_4b1712 >= 0)
                moveView(g_4b160e, 0, g_4b1734[0].view);
            else
                moveView(g_4b160e, 0, g_4b1664);
        }
    }
    if (g_4a3d42 < 0)
        return;
    moveView(g_4a3d42, 0, g_4b1616);
    if (g_4b1618 == 1) {
        moveView(g_4b160e, 0, g_4a3d42);
        if (g_4b161c == 1) {
            moveView(g_4b1612, 0, g_4b160e);
            if (g_4b161a == 1)
                moveView(g_4b1610, 0, g_4b1612);
        } else if (g_4b161a == 1) {
            moveView(g_4b1610, 0, g_4b160e);
        }
    } else if (g_4b161c == 1) {
        moveView(g_4b1612, 0, g_4a3d42);
        if (g_4b161a == 1)
            moveView(g_4b1610, 0, g_4b1612);
    } else if (g_4b161a == 1) {
        moveView(g_4b1610, 0, g_4a3d42);
    }
}

/* Clears the toppings (g_4b16ca, and the ones shown, g_4b164a-g_4b1658) and sets
   the topping views (toppingViews) to the level's scripts (7005 on). */
/* @zoombi32 0x00446198 */
void fn_446198()
{
    fillMemory(g_4b16ca, 0, 16);
    g_4b1652 = g_4b1650 = g_4b164c = g_4b164e = g_4b164a = g_4b1654 = g_4b1656 = g_4b1658 = 0;
    switch (g_4b161e) {
    case 0:
        setViewScript(findView(toppingViews[0]), 7005, 1);
        setViewScript(findView(toppingViews[1]), 7007, 1);
        setViewScript(findView(toppingViews[3]), 7011, 1);
        setViewScript(findView(toppingViews[2]), 7009, 1);
        setViewScript(findView(toppingViews[4]), 7013, 1);
        break;
    case 1:
        setViewScript(findView(toppingViews[0]), 7015, 1);
        setViewScript(findView(toppingViews[1]), 7017, 1);
        setViewScript(findView(toppingViews[3]), 7021, 1);
        setViewScript(findView(toppingViews[2]), 7019, 1);
        setViewScript(findView(toppingViews[5]), 7023, 1);
        setViewScript(findView(toppingViews[6]), 7025, 1);
        break;
    case 2:
        setViewScript(findView(toppingViews[0]), 7027, 1);
        setViewScript(findView(toppingViews[1]), 7029, 1);
        setViewScript(findView(toppingViews[3]), 7033, 1);
        setViewScript(findView(toppingViews[2]), 7031, 1);
        setViewScript(findView(toppingViews[4]), 7035, 1);
        setViewScript(findView(toppingViews[5]), 7037, 1);
        setViewScript(findView(toppingViews[6]), 7039, 1);
        break;
    case 3:
        setViewScript(findView(toppingViews[0]), 7041, 1);
        setViewScript(findView(toppingViews[1]), 7043, 1);
        setViewScript(findView(toppingViews[3]), 7047, 1);
        setViewScript(findView(toppingViews[2]), 7045, 1);
        setViewScript(findView(toppingViews[4]), 7049, 1);
        setViewScript(findView(toppingViews[5]), 7051, 1);
        setViewScript(findView(toppingViews[6]), 7053, 1);
        setViewScript(findView(toppingViews[7]), 7055, 1);
        break;
    }
}

/* A view's placing: drops the cels of toppings not on the pizza
   (g_4b16da; images 5-24 by topping, 25-40 by level too: 29-32 only at
   level 3) and moves the
   rest by (g_4b1666, g_4b1668), or, while g_4b1630, to there from where
   the first one was. */
/* @zoombi32 0x00442a9f */
void fn_442a9f(View *view)
{
    short dx;
    short dy;
    short *cel = (short *)&view->body;
    short removed;
    short first = 1;

    while (*cel) {
        removed = 0;
        switch (*cel) {
        case 5:
        case 6:
        case 7:
        case 8:
            if (!g_4b16da[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            if (!g_4b16da[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!g_4b16da[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 17:
        case 18:
        case 19:
        case 20:
            if (!g_4b16da[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 21:
        case 22:
        case 23:
        case 24:
            if (!g_4b16da[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 25:
        case 26:
        case 27:
        case 28:
            if (!g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 37:
        case 38:
        case 39:
        case 40:
            if (!g_4b16da[5] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 33:
        case 34:
        case 35:
        case 36:
            if (!g_4b16da[6] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 29:
        case 30:
        case 31:
        case 32:
            if (!g_4b16da[7] || g_4b161e != 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += g_4b1666;
                cel[2] += g_4b1668;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += g_4b1666 - dx;
                cel[2] += g_4b1668 - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: fn_442a9f for the toppings troll 0 wants
   (images 156-191). */
/* @zoombi32 0x0044468e */
void fn_44468e(View *view)
{
    short dx;
    short dy;
    short *cel = (short *)&view->body;
    short removed;
    short first = 1;

    while (*cel) {
        removed = 0;
        switch (*cel) {
        case 156:
        case 157:
        case 158:
        case 159:
            if (!arnoWants[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 160:
        case 161:
        case 162:
        case 163:
            if (!arnoWants[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 164:
        case 165:
        case 166:
        case 167:
            if (!arnoWants[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 168:
        case 169:
        case 170:
        case 171:
            if (!arnoWants[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 172:
        case 173:
        case 174:
        case 175:
            if (!arnoWants[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 176:
        case 177:
        case 178:
        case 179:
            if (!g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!arnoWants[5] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!arnoWants[6] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!arnoWants[7] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += g_4b1666;
                cel[2] += g_4b1668;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += g_4b1666 - dx;
                cel[2] += g_4b1668 - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: fn_442a9f for the toppings troll 1 wants
   (images 156-191, and 212 when g_4b1618 is 3). */
/* @zoombi32 0x0044485d */
void fn_44485d(View *view)
{
    short dx;
    short dy;
    short *cel = (short *)&view->body;
    short removed;
    short first = 1;

    while (*cel) {
        removed = 0;
        switch (*cel) {
        case 156:
        case 157:
        case 158:
        case 159:
            if (!willaWants[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 160:
        case 161:
        case 162:
        case 163:
            if (!willaWants[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 164:
        case 165:
        case 166:
        case 167:
            if (!willaWants[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 168:
        case 169:
        case 170:
        case 171:
            if (!willaWants[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 172:
        case 173:
        case 174:
        case 175:
            if (!willaWants[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 176:
        case 177:
        case 178:
        case 179:
            if (!g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!willaWants[5] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!willaWants[6] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!willaWants[7] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 212:
            if (g_4b1618 == 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += g_4b1666;
                cel[2] += g_4b1668;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += g_4b1666 - dx;
                cel[2] += g_4b1668 - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: fn_442a9f for the toppings troll 2 wants
   (images 156-191). */
/* @zoombi32 0x00444a93 */
void fn_444a93(View *view)
{
    short dx;
    short dy;
    short *cel = (short *)&view->body;
    short removed;
    short first = 1;

    while (*cel) {
        removed = 0;
        switch (*cel) {
        case 156:
        case 157:
        case 158:
        case 159:
            if (!shylerWants[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 160:
        case 161:
        case 162:
        case 163:
            if (!shylerWants[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 164:
        case 165:
        case 166:
        case 167:
            if (!shylerWants[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 168:
        case 169:
        case 170:
        case 171:
            if (!shylerWants[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 172:
        case 173:
        case 174:
        case 175:
            if (!shylerWants[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 176:
        case 177:
        case 178:
        case 179:
            if (!g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!shylerWants[5] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!shylerWants[6] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!shylerWants[7] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += g_4b1666;
                cel[2] += g_4b1668;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += g_4b1666 - dx;
                cel[2] += g_4b1668 - dy;
            }
            cel += 3;
        }
    }
}

/*
 * What troll `troll` (0-2) makes of the pizza (g_4b16da): 0 if it has one
 * topping the troll doesn't want, 4 if more; else 2 if it has all the
 * troll wants, 1 if not all. (3 is never returned.)
 */
/* @zoombi32 0x0044338b */
short fn_44338b(short troll)
{
    short never;
    short wanted;
    short right;
    short wrong;
    short i;

    never = wanted = right = wrong = 0;
    switch (troll) {
    case 0:
        for (i = 0; i < g_4b1624; i++)
            if (arnoWants[i])
                wanted++;
        for (i = 0; i < g_4b1624; i++)
            if (g_4b16da[i]) {
                if (arnoWants[i])
                    right++;
                else
                    wrong++;
            }
        if (never)
            return 3;
        if (wrong == 1)
            return 0;
        if (wrong > 1)
            return 4;
        break;
    case 1:
        for (i = 0; i < g_4b1624; i++)
            if (willaWants[i])
                wanted++;
        for (i = 0; i < g_4b1624; i++)
            if (g_4b16da[i]) {
                if (willaWants[i])
                    right++;
                else
                    wrong++;
            }
        if (never)
            return 3;
        if (wrong == 1)
            return 0;
        if (wrong > 1)
            return 4;
        break;
    case 2:
        for (i = 0; i < g_4b1624; i++)
            if (shylerWants[i])
                wanted++;
        for (i = 0; i < g_4b1624; i++)
            if (g_4b16da[i]) {
                if (shylerWants[i])
                    right++;
                else
                    wrong++;
            }
        if (never)
            return 3;
        if (wrong == 1)
            return 0;
        if (wrong > 1)
            return 4;
        break;
    }
    if (right == wanted)
        return 2;
    return 1;
}

/* Now and then, while no troll is busy (g_4b1600-g_4b1604), has one of the
   trolls there are at the level fidget (8034-8035, 9019-9020, or 10001 or
   10006-10008). */
/* Not exact: the original keeps `view` in ebx and `r` in esi; BCC swaps
   them, whatever the declaration order. */
/* @zoombi32 0x00446745 */
void fn_446745()
{
    short r;
    View *view;

    if (!g_4b1600 && !g_4b1602 && !g_4b1604) {
        switch (g_4b161e) {
        case 0:
            if (g_4b1618 != 3) {
                view = findView(g_4b160e);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            }
            break;
        case 1:
            if (randomUpTo(1000) < 500 && g_4b1618 == 1) {
                view = findView(g_4b160e);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            } else if (g_4b161a == 1) {
                view = findView(g_4b1610);
                setViewScript(view, randomUpTo(1) + 9019, 1);
            }
            break;
        case 2:
        case 3:
            r = randomUpTo(1000);
            if (r < 300 && g_4b1618 == 1) {
                view = findView(g_4b160e);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            } else if (r < 600 && g_4b161a == 1) {
                view = findView(g_4b1610);
                setViewScript(view, randomUpTo(1) + 9019, 1);
            } else if (g_4b161c == 1) {
                r = randomUpTo(3);
                if (!r)
                    r = 1;
                else
                    r += 5;
                view = findView(g_4b1612);
                setViewScript(view, r + 10000, 1);
            }
            break;
        }
    }
}

/* Records the pizza as tried and has the troll whose turn it is
   (g_4b1614, or g_4b1670 if set, counted down: 0 Arno, 1 Willa, 2
   Shyler) react (8020, 9026 or 10030), placed by fn_442c6c; Willa's turn
   is skipped while g_4b16bc. Clears the view g_4b1664. */
/* @zoombi32 0x00444c62 */
void fn_444c62()
{
    View *view;

    deleteView(g_4b1664);
    g_4b1664 = 0;
    if (g_4b1670)
        g_4b1614 = g_4b1670;
    g_4b1614--;
    fn_444556();
    if (!g_4b1614) {
        view = findView(g_4b160e);
        setViewScript(view, 8020, 1);
        moveView(g_4b160e, 1, g_4b1616);
        if (g_4b1620 > 0)
            moveView(g_4a3d42, 1, g_4b160e);
        g_4b1606 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
    } else if (g_4b1614 == 1) {
        if (g_4b16bc) {
            g_4b15fe = ++g_4b1614;
            return;
        }
        view = findView(g_4b1610);
        setViewScript(view, 9026, 1);
        view->notify = fn_4441a8;
        moveView(g_4b1610, 1, g_4b1616);
        moveView(g_4a3d42, 1, g_4b1610);
        g_4b1606 = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
    } else if (g_4b1614 == 2) {
        view = findView(g_4b1612);
        setViewScript(view, 10030, 1);
        moveView(g_4b1612, 1, g_4b1616);
        g_4b1606 = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
    }
    view->placed = fn_442c6c;
    g_4b1614 = 0;
}

/* The trolls there are (Arno always; Willa if g_4b161a, Shyler if
   g_4b161c) eat (8024-8031, 9030-9033, 10035-10037), each placing the
   toppings it wants. */
/* @zoombi32 0x00444391 */
void fn_444391()
{
    View *view;
    short r;

    if (!g_4b161a && !g_4b161c) {
        view = findView(g_4b160e);
        setViewScript(view, randomUpTo(1) + 8024, 1);
        view->placed = fn_44468e;
    } else if (g_4b161a && !g_4b161c) {
        r = randomUpTo(1);
        view = findView(g_4b160e);
        setViewScript(view, r + 8026, 1);
        view->placed = fn_44468e;
        view = findView(g_4b1610);
        setViewScript(view, r + 9030, 1);
        view->placed = fn_44485d;
    } else if (!g_4b161a && g_4b161c) {
        r = randomUpTo(1);
        view = findView(g_4b160e);
        setViewScript(view, r + 8028, 1);
        view->placed = fn_44468e;
        view = findView(g_4b1612);
        setViewScript(view, r + 10035, 1);
        view->placed = fn_444a93;
    } else if (g_4b161a && g_4b161c) {
        r = randomUpTo(1);
        view = findView(g_4b160e);
        setViewScript(view, r + 8030, 1);
        view->placed = fn_44468e;
        view = findView(g_4b1610);
        setViewScript(view, r + 9032, 1);
        view->placed = fn_44485d;
        view = findView(g_4b1612);
        setViewScript(view, r + 10036, 1);
        view->placed = fn_444a93;
    }
    g_4b15ec = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
}

/* A view's placing: fn_442a9f with the troll's images (156-191, and 212
   when g_4b1618 is 3) for the toppings on the pizza (g_4b16da). */
/* @zoombi32 0x00442c6c */
void fn_442c6c(View *view)
{
    short dx;
    short dy;
    short *cel = (short *)&view->body;
    short removed;
    short first = 1;

    while (*cel) {
        removed = 0;
        switch (*cel) {
        case 156:
        case 157:
        case 158:
        case 159:
            if (!g_4b16da[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 160:
        case 161:
        case 162:
        case 163:
            if (!g_4b16da[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 164:
        case 165:
        case 166:
        case 167:
            if (!g_4b16da[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 168:
        case 169:
        case 170:
        case 171:
            if (!g_4b16da[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 172:
        case 173:
        case 174:
        case 175:
            if (!g_4b16da[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 176:
        case 177:
        case 178:
        case 179:
            if (!g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!g_4b16da[5] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!g_4b16da[6] || !g_4b161e) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!g_4b16da[7] || g_4b161e != 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 212:
            if (g_4b1618 == 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += g_4b1666;
                cel[2] += g_4b1668;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += g_4b1666 - dx;
                cel[2] += g_4b1668 - dy;
            }
            cel += 3;
        }
    }
}

/*
 * Willa's notify: 32 puts the pizza up (g_4b1664, script 12000) in front;
 * 60 has the Zoombini at the pizza step (13000 on, by its feet); 99 puts
 * her in front of Arno; when her script ends, the Zoombini reacts (13005,
 * 13010 or 13015 on, by the troll up; notify fn_444e0c) and the pizza with
 * it (12001, 12006 or 12011 on).
 */
/* @zoombi32 0x004441a8 */
void fn_4441a8(View *, short event)
{
    View *view;
    short feet;
    short script;

    switch (event) {
    case 32:
        view = findView(g_4b1664);
        if (view) {
            setViewScript(view, 12000, 1);
        } else {
            g_4b1664 = addView(0x108000, drawCels, runViewScript, 12000, 6, 0, 0, 0);
            view = findView(g_4b1664);
            moveView(g_4b1616, 1, g_4b1664);
        }
        view->placed = fn_442a9f;
        fn_446035();
        g_4b15f4 = groupViews(g_4b1664, g_4b1664, 0, 0, 0, 0);
        fn_4423d7();
        break;
    case 60:
        view = findView(g_4a3d42);
        feet = ((Snoid *)&view->body)->features[3] - 1;
        startSnoidScript((Snoid *)&view->body, feet + 13000, 0, 0);
        view->notifyEnd = 1;
        view->notify = fn_4441a8;
        break;
    case 99:
        if (g_4b1618 == 1)
            moveView(g_4b1610, 0, g_4b160e);
        break;
    case -1:
        view = findView(g_4a3d42);
        feet = ((Snoid *)&view->body)->features[3] - 1;
        if (g_4b1618 == 1) {
            script = feet + 13005;
            feet += 12001;
        } else if (g_4b161a == 1) {
            script = feet + 13010;
            feet += 12006;
        } else {
            script = feet + 13015;
            feet += 12011;
        }
        startSnoidScript((Snoid *)&view->body, script, 0, 0);
        view->notifyEnd = 0;
        view->notify = fn_444e0c;
        view = findView(g_4b1664);
        setViewScript(view, feet, 1);
        view->placed = fn_442a9f;
        fn_446035();
        g_4b15f6 = groupViews(g_4b1664, g_4a3d42, 0, 0, 0, 0);
        fn_4423d7();
        break;
    }
}

/*
 * The notify of the Zoombini at the pizza: 0 flips which way it faces
 * and turns it the way g_4b170c says; 240-243 note a turn to make,
 * 250-253 turn it; 61 (unless g_4b171a) sends it off along the path to the
 * troll up (14000, 14002 or 14004 on, the next if it's the last, sound
 * 8040); when its script ends it walks on to the pizza spot (g_4a3d44), or
 * if it was the last to go, it's done and the next may come (g_4b15da).
 */
/* @zoombi32 0x00444e0c */
void fn_444e0c(View *view, short event)
{
    Point where;
    Snoid *snoid = (Snoid *)&view->body;
    short last;
    View *zoombini;

    switch (event) {
    case 250:
    case 251:
    case 252:
    case 253:
        setSnoidFacing(snoid, event - 250);
        break;
    case 240:
    case 241:
    case 242:
    case 243:
        g_4b170c = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b170c) {
            setSnoidFacing(snoid, g_4b170c - 1);
            g_4b170c = 0;
        }
        break;
    case 61:
        if (g_4b171a) {
            g_4b171a = 0;
            break;
        }
        where.x = 180;
        where.y = 327;
        zoombini = findView(g_4a3d42);
        last = 0;
        if (!g_4b1646)
            last = 1;
        if (g_4b1618 == 1) {
            if (!g_4b1646) {
                where.x = 34;
                where.y = 59;
            }
            startSnoidScript((Snoid *)&zoombini->body, g_4b1646 + 14000, &where, last);
            queueViewSound(8040, 0);
        } else if (g_4b161a == 1) {
            if (!g_4b1646) {
                where.x = 46;
                where.y = 46;
            }
            startSnoidScript((Snoid *)&zoombini->body, g_4b1646 + 14002, &where, last);
            queueViewSound(8040, 0);
        } else {
            if (!g_4b1646) {
                where.x = 95;
                where.y = 27;
            }
            startSnoidScript((Snoid *)&zoombini->body, g_4b1646 + 14004, &where, last);
            queueViewSound(8040, 0);
        }
        zoombini->notify = fn_444e0c;
        if (g_4b1646) {
            zoombini->notifyEnd = 1;
            g_4b15e0 = 0;
        } else {
            zoombini->notifyEnd = 1;
            g_4b15e0 = zoombini;
        }
        zoombini->interval = 6;
        break;
    case -1:
        if (!g_4b15e0) {
            zoombini = findView(g_4a3d42);
            where = g_4a3d44;
            if (zoombini) {
                setSnoidAction((Snoid *)&zoombini->body, 7, 0);
                *(Point *)&((Snoid *)&zoombini->body)->targetX = where;
            }
            g_4b15ea = 1;
            g_4b15da = 0;
        } else if (g_4b15e0) {
            g_4b83e4[0] = 0;
            ((Snoid *)&g_4b15e0->body)->unknownF7 = 0;
            g_4b15e0->body.running = 0;
            g_4b15e0 = 0;
            g_4b15da = 1;
            g_4b15ee++;
        }
        g_4b15d8 = 1;
        break;
    }
}

/* A pizza is served: counts down the pizzas left (g_4b1620; g_4b1646 set
   while some are); the troll up takes it (8022, 9028 or 10032 on) and the
   Zoombini at the pizza gets its notify, unless it's the last with none
   left to judge, when the pizza view starts over (fn_44509b). */
/* @zoombi32 0x00445153 */
void fn_445153()
{
    View *view;

    g_4b165c = 0;
    g_4b1646 = 0;
    if (--g_4b1620 >= 0)
        g_4b1646 = 1;
    if (!g_4b1620)
        g_4b1648++;
    if (g_4b1648 || !g_4b1646) {
        if (g_4b1618 == 1) {
            view = findView(g_4b160e);
            setViewScript(view, g_4b1646 + 8022, 1);
            if (g_4b1648) {
                g_4b16bc++;
                g_4b1600 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
            } else {
                g_4b15fc = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
            }
        } else if (g_4b161a == 1) {
            view = findView(g_4b1610);
            setViewScript(view, g_4b1646 + 9028, 1);
            g_4b15fc = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
        } else if (g_4b161c == 1) {
            view = findView(g_4b1612);
            setViewScript(view, g_4b1646 + 10032, 1);
            g_4b15fc = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
        }
        view->notifyEnd = 0;
        view->notify = fn_444e0c;
        g_4b1648 = g_4b171a = 0;
    } else {
        fn_44509b();
        g_4b171a++;
        g_4b15fc = 1000;
    }
    claimPlacedView(1, 0);
}

/* Shows four pizzas, each with two of the toppings a, b, c and d (a and
   b, b and c, c and d, a and d), recording each as tried and shown
   (g_4b1734; scripts 12042 on, g_4b170e counting). */
/* @zoombi32 0x00446487 */
void fn_446487(short a, short b, short c, short d)
{
    fillMemory(g_4b16da, 0, 16);
    g_4b16da[a] = 1;
    g_4b16da[b] = 1;
    fn_444556();
    g_4b1712++;
    g_4b1734[g_4b1712].set = g_4b16ec[g_4b1708];
    g_4b1734[g_4b1712].unknown4 = 4;
    g_4b170e++;
    g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    g_4b1734[g_4b1712].script = g_4b170e + 12025;
    findView(g_4b1734[g_4b1712].view)->placed = fn_442a9f;
    updateViews();
    fillMemory(g_4b16da, 0, 16);
    g_4b16da[b] = 1;
    g_4b16da[c] = 1;
    fn_444556();
    g_4b1712++;
    g_4b1734[g_4b1712].set = g_4b16ec[g_4b1708];
    g_4b1734[g_4b1712].unknown4 = 4;
    g_4b170e++;
    g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    g_4b1734[g_4b1712].script = g_4b170e + 12025;
    findView(g_4b1734[g_4b1712].view)->placed = fn_442a9f;
    updateViews();
    fillMemory(g_4b16da, 0, 16);
    g_4b16da[c] = 1;
    g_4b16da[d] = 1;
    fn_444556();
    g_4b1712++;
    g_4b1734[g_4b1712].set = g_4b16ec[g_4b1708];
    g_4b1734[g_4b1712].unknown4 = 4;
    g_4b170e++;
    g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    g_4b1734[g_4b1712].script = g_4b170e + 12025;
    findView(g_4b1734[g_4b1712].view)->placed = fn_442a9f;
    updateViews();
    fillMemory(g_4b16da, 0, 16);
    g_4b16da[a] = 1;
    g_4b16da[d] = 1;
    fn_444556();
    g_4b1712++;
    g_4b1734[g_4b1712].set = g_4b16ec[g_4b1708];
    g_4b1734[g_4b1712].unknown4 = 4;
    g_4b170e++;
    g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 0, 0, 0, 0);
    g_4b1734[g_4b1712].script = g_4b170e + 12025;
    findView(g_4b1734[g_4b1712].view)->placed = fn_442a9f;
    updateViews();
    fillMemory(g_4b16da, 0, 16);
}

/* Debugging: shows what each troll there wants and the meal on the pizza
   (g_4b16ca) in a box at the top right. */
/* @zoombi32 0x00443e2e */
void fn_443e2e()
{
    ShortRect box = {400, 1, 600, 80};
    ShortRect arnoLine = {400, 1, 600, 20};
    ShortRect willaLine = {400, 21, 600, 40};
    ShortRect shylerLine = {400, 41, 600, 60};
    ShortRect mealLine = {400, 61, 600, 80};
    Color saved;
    char arno[32];
    char willa[32];
    char shyler[32];
    char meal[32];

    saved = setForeColor(Color(11));
    sprintf(arno, "Arno    %d %d %d %d %d %d %d %d", arnoWants[0], arnoWants[1], arnoWants[2],
            arnoWants[3], arnoWants[4], arnoWants[5], arnoWants[6], arnoWants[7]);
    sprintf(willa, "Willa   %d %d %d %d %d %d %d %d", willaWants[0], willaWants[1], willaWants[2],
            willaWants[3], willaWants[4], willaWants[5], willaWants[6], willaWants[7]);
    sprintf(shyler, "Shyler  %d %d %d %d %d %d %d %d", shylerWants[0], shylerWants[1], shylerWants[2],
            shylerWants[3], shylerWants[4], shylerWants[5], shylerWants[6], shylerWants[7]);
    sprintf(meal, "Meal     %d %d %d %d %d %d %d %d", g_4b16ca[0], g_4b16ca[1], g_4b16ca[2], g_4b16ca[3],
            g_4b16ca[4], g_4b16ca[5], g_4b16ca[6], g_4b16ca[7]);
    fillPortRect(box, Color(14), 0);
    frameRect(box);
    if (g_4b1618 == 1)
        drawText(arnoLine, 0x22, arno, 0xffff);
    if (g_4b161a == 1)
        drawText(willaLine, 0x22, willa, 0xffff);
    if (g_4b161c == 1)
        drawText(shylerLine, 0x22, shyler, 0xffff);
    drawText(mealLine, 0x22, meal, 0xffff);
    setForeColor(saved);
    showRect(&box);
}

/*
 * A troll reacts to the pizza by its verdict (fn_44338b): for 0 or 4, the
 * troll is picked from those there (with both Willa and Shyler there, it
 * sets g_4b161a again from g_4b161c first); 1 (not all it wants) it
 * asks for more (8000, 9021 or 10009 on, by g_4b16b6, g_4b16be,
 * g_4b16c4); otherwise it grumbles (8015, 9017 or 10027 on).
 */
/* @zoombi32 0x00445b80 */
void fn_445b80(short troll, short verdict)
{
    View *view;
    long both;

    g_4b16ea = 0;
    if (!verdict || verdict == 4) {
        if (g_4b1618 == 1) {
            if (g_4b161a != 1 && g_4b161c != 1)
                troll = 0;
            else if (g_4b161a == 1 && g_4b161c != 1)
                troll = randomUpTo(1);
            else if (g_4b161a != 1 && g_4b161c == 1)
                troll = randomUpTo(1) * 2;
            else {
                if (g_4b161c != 1)
                    both = 0;
                else
                    both = 1;
                if ((g_4b161a = both) != 0)
                    troll = randomUpTo(2);
            }
        } else if (g_4b161a == 1) {
            if (g_4b161c != 1)
                troll = 1;
            else
                troll = randomUpTo(1) + 1;
        } else {
            troll = 2;
        }
    }
    switch (troll) {
    case 0:
        g_4b15fe = 1;
        if (verdict == 1) {
            view = findView(g_4b160e);
            setViewScript(view, g_4b16b6 + 8000, 1);
            fn_446035();
            g_4b1600 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
            g_4b1670 = 1;
            g_4b1710 = 5;
        } else {
            view = findView(g_4b160e);
            setViewScript(view, randomUpTo(1) + 8015, 1);
            fn_446035();
            g_4b1600 = groupViews(g_4b160e, g_4b160e, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    case 1:
        g_4b15fe = 2;
        if (verdict == 1) {
            view = findView(g_4b1610);
            setViewScript(view, g_4b16be + 9021, 1);
            fn_446035();
            g_4b1602 = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
            g_4b1670 = 2;
            g_4b1710 = 6;
        } else {
            view = findView(g_4b1610);
            setViewScript(view, randomUpTo(1) + 9017, 1);
            fn_446035();
            g_4b1602 = groupViews(g_4b1610, g_4b1610, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    case 2:
        g_4b15fe = 3;
        if (verdict == 1) {
            view = findView(g_4b1612);
            setViewScript(view, g_4b16c4 + 10009, 1);
            fn_446035();
            g_4b1604 = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
            g_4b1670 = 3;
            g_4b1710 = 7;
        } else {
            view = findView(g_4b1612);
            setViewScript(view, randomUpTo(2) + 10027, 1);
            fn_446035();
            g_4b1604 = groupViews(g_4b1612, g_4b1612, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    }
}

/*
 * Shows the pizza just judged (g_4b1710: 4 thrown, 5-7 on the pile of
 * the troll that took it) and records it (g_4b1734): thrown ones cycle
 * through 16 scripts (12025 on, skipping 13; once they've all been used,
 * g_4a3dcc, the view already there is reused); piled ones take the next
 * of three places per pile (12016, 12019 or 12022 on), each in front of
 * the one it replaces.
 */
/* @zoombi32 0x00445307 */
void fn_445307()
{
    short behind;
    View *view;
    short i;

    g_4b1712++;
    g_4b1734[g_4b1712].set = g_4b16ec[g_4b1708];
    g_4b1734[g_4b1712].unknown4 = g_4b1710;
    behind = 0;
    switch (g_4b1710) {
    case 4:
        if (++g_4b170e >= 16) {
            g_4b170e = 0;
            g_4a3dcc++;
        } else if (g_4b170e == 13) {
            g_4b170e = 14;
        }
        if (!g_4a3dcc) {
            g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12025, 6, 0, 0, 0);
            g_4b1734[g_4b1712].script = g_4b170e + 12025;
            view = findView(g_4b1734[g_4b1712].view);
            view->placed = fn_442a9f;
        } else {
            for (i = 0; i < 28; i++)
                if (g_4b1734[i].script == g_4b170e + 12025) {
                    view = findView(g_4b1734[i].view);
                    setViewScript(view, g_4b170e + 12025, 1);
                    view->placed = fn_442a9f;
                    break;
                }
        }
        break;
    case 5:
        if (++g_4b1714 > 2)
            g_4b1714 = 0;
        if (g_4b1722[g_4b1714])
            behind = g_4b1722[g_4b1714];
        g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1714 + 12016, 6, 0, 0, 0);
        g_4b1734[g_4b1712].script = g_4b1714 + 12016;
        view = findView(g_4b1734[g_4b1712].view);
        view->placed = fn_442a9f;
        if (behind)
            moveView(g_4b1734[g_4b1712].view, 1, behind);
        g_4b1722[g_4b1714] = g_4b1734[g_4b1712].view;
        g_4b1734[g_4b1712].unknown4 = 5;
        break;
    case 6:
        if (++g_4b1716 > 2)
            g_4b1716 = 0;
        if (g_4b1728[g_4b1716])
            behind = g_4b1728[g_4b1716];
        g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1716 + 12019, 6, 0, 0, 0);
        g_4b1734[g_4b1712].script = g_4b1716 + 12019;
        view = findView(g_4b1734[g_4b1712].view);
        view->placed = fn_442a9f;
        if (behind)
            moveView(g_4b1734[g_4b1712].view, 1, behind);
        g_4b1728[g_4b1716] = g_4b1734[g_4b1712].view;
        g_4b1734[g_4b1712].unknown4 = 6;
        break;
    case 7:
        if (++g_4b1718 > 2)
            g_4b1718 = 0;
        if (g_4b172e[g_4b1718])
            behind = g_4b172e[g_4b1718];
        g_4b1734[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1718 + 12022, 6, 0, 0, 0);
        g_4b1734[g_4b1712].script = g_4b1718 + 12022;
        view = findView(g_4b1734[g_4b1712].view);
        view->placed = fn_442a9f;
        if (behind)
            moveView(g_4b1734[g_4b1712].view, 1, behind);
        g_4b172e[g_4b1718] = g_4b1734[g_4b1712].view;
        g_4b1734[g_4b1712].unknown4 = 7;
        break;
    }
    view->notifyEnd = 1;
    view->notify = fn_445ae1;
    fn_446035();
    if (g_4b1734[g_4b1712].unknown4 == 4) {
        moveView(g_4b1734[g_4b1712].view, 1, g_4b1616);
    } else if (g_4b1734[g_4b1712].unknown4 == 6 && g_4b1728[1]) {
        moveView(g_4b1728[1], 0, g_4b1610);
        moveView(g_4b1728[0], 0, g_4b1728[1]);
    }
    moveView(g_4a3d38, 0, -1);
    g_4b160c = groupViews(g_4b1734[g_4b1712].view, g_4b1734[g_4b1712].view, 0, 0, 0, 0);
}

/* A topping button (4-11: the eight toppings, those the level has; 3
   serves the pizza): toggles the topping on the meal (g_4b16ca) and its
   view (by level), and redraws the pizza; 3 has the pizza carried off
   (7057, or 7058 from level 1; 7066) and clears the toppings, without
   the redraw. */
/* @zoombi32 0x00442560 */
void fn_442560(short button)
{
    short redraw;
    View *view;

    if (g_4b15f2)
        return;
    redraw = 1;
    switch (button) {
    case 3:
        redraw = 0;
        view = findView(g_4b162e);
        if (!g_4b161e)
            setViewScript(view, 7057, 1);
        else
            setViewScript(view, 7058, 1);
        view = findView(g_4b15f0);
        setViewScript(view, 7066, 1);
        g_4b15f2 = groupViews(g_4b15f0, g_4b15f0, 0, 0, 0, 0);
        view->notify = fn_4441a8;
        fn_446198();
        break;
    case 4:
        g_4b1652 ^= 1;
        g_4b16ca[0] = g_4b1652;
        view = findView(toppingViews[0]);
        if (!g_4b161e)
            setViewScript(view, g_4b1652 + 7005, 1);
        else if (g_4b161e == 1)
            setViewScript(view, g_4b1652 + 7015, 1);
        else if (g_4b161e == 2)
            setViewScript(view, g_4b1652 + 7027, 1);
        else
            setViewScript(view, g_4b1652 + 7041, 1);
        break;
    case 5:
        g_4b1650 ^= 1;
        g_4b16ca[1] = g_4b1650;
        view = findView(toppingViews[1]);
        if (!g_4b161e)
            setViewScript(view, g_4b1650 + 7007, 1);
        else if (g_4b161e == 1)
            setViewScript(view, g_4b1650 + 7017, 1);
        else if (g_4b161e == 2)
            setViewScript(view, g_4b1650 + 7029, 1);
        else
            setViewScript(view, g_4b1650 + 7043, 1);
        break;
    case 6:
        g_4b164e ^= 1;
        g_4b16ca[2] = g_4b164e;
        view = findView(toppingViews[2]);
        if (!g_4b161e)
            setViewScript(view, g_4b164e + 7009, 1);
        else if (g_4b161e == 1)
            setViewScript(view, g_4b164e + 7019, 1);
        else if (g_4b161e == 2)
            setViewScript(view, g_4b164e + 7031, 1);
        else
            setViewScript(view, g_4b164e + 7045, 1);
        break;
    case 7:
        g_4b164c ^= 1;
        g_4b16ca[3] = g_4b164c;
        view = findView(toppingViews[3]);
        if (!g_4b161e)
            setViewScript(view, g_4b164c + 7011, 1);
        else if (g_4b161e == 1)
            setViewScript(view, g_4b164c + 7021, 1);
        else if (g_4b161e == 2)
            setViewScript(view, g_4b164c + 7033, 1);
        else
            setViewScript(view, g_4b164c + 7047, 1);
        break;
    case 8:
        g_4b164a ^= 1;
        g_4b16ca[4] = g_4b164a;
        view = findView(toppingViews[4]);
        if (!g_4b161e)
            setViewScript(view, g_4b164a + 7013, 1);
        else if (g_4b161e == 2)
            setViewScript(view, g_4b164a + 7035, 1);
        else if (g_4b161e == 3)
            setViewScript(view, g_4b164a + 7049, 1);
        break;
    case 9:
        if (g_4b161e) {
            g_4b1654 ^= 1;
            g_4b16ca[5] = g_4b1654;
            view = findView(toppingViews[5]);
            if (g_4b161e == 1)
                setViewScript(view, g_4b1654 + 7023, 1);
            else if (g_4b161e == 2)
                setViewScript(view, g_4b1654 + 7037, 1);
            else
                setViewScript(view, g_4b1654 + 7051, 1);
        }
        break;
    case 10:
        if (g_4b161e) {
            g_4b1656 ^= 1;
            g_4b16ca[6] = g_4b1656;
            view = findView(toppingViews[6]);
            if (g_4b161e == 1)
                setViewScript(view, g_4b1656 + 7025, 1);
            else if (g_4b161e == 2)
                setViewScript(view, g_4b1656 + 7039, 1);
            else
                setViewScript(view, g_4b1656 + 7053, 1);
        }
        break;
    case 11:
        if (g_4b161e == 3) {
            g_4b1658 ^= 1;
            g_4b16ca[7] = g_4b1658;
            view = findView(toppingViews[7]);
            setViewScript(view, g_4b1658 + 7055, 1);
        }
        break;
    }
    if (redraw)
        fn_4423d7();
}

/* The scene's keys (debugging ones only while debugging messages are on):
   A shows the trolls' wants; R, O, D in turn arm the rest (g_4b15e8), then
   P makes the trolls eat, N, W and S step Arno, Willa and Shyler through
   their scripts, and space sets the pizzas left (g_4b1620) to g_4b1634.
   Notes the time of the key (g_4b1824). */
/* @zoombi32 0x00442166 */
short pizzaKey(unsigned short key)
{
    g_4b1824 = clockTime();
    if (!g_4b8803 && key != 367)
        return 0;
    switch (key) {
    case 367:
        fn_466b93();
        return 1;
    case 'A':
    case 'a':
        fn_443e2e();
        return 1;
    case 'R':
        if (g_4b15e8 > 2)
            g_4b15e8++;
        else
            g_4b15e8 = 1;
        return 1;
    case 'O':
        if (g_4b15e8 == 1)
            g_4b15e8++;
        return 1;
    case 'D':
        if (g_4b15e8 == 2)
            g_4b15e8++;
        return 1;
    case 'P':
    case 'p':
        if (g_4b15e8 >= 3) {
            fn_444391();
            g_4b15ec = 0;
            return 1;
        }
        break;
    case 'N':
    case 'n':
        if (g_4b15e8 >= 3) {
            if (g_4a3d9e >= 36)
                g_4a3d9e = 0;
            setViewScript(findView(g_4b160e), g_4a3d9e++ + 8000, 1);
            fn_446035();
            return 1;
        }
        break;
    case 'S':
    case 's':
        if (g_4b15e8 >= 3) {
            if (g_4a3da2 >= 39)
                g_4a3da2 = 0;
            setViewScript(findView(g_4b1612), g_4a3da2++ + 10000, 1);
            fn_446035();
            return 1;
        }
        break;
    case 'W':
    case 'w':
        if (g_4b15e8 >= 3) {
            if (g_4a3da0 >= 35)
                g_4a3da0 = 0;
            setViewScript(findView(g_4b1610), g_4a3da0++ + 9000, 1);
            fn_446035();
            return 1;
        }
        break;
    case ' ':
        if (g_4b15e8 >= 3) {
            g_4b1620 = g_4b1634;
            return 1;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
