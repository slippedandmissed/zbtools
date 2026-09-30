/*
 * pizza (0x4402c0-0x44695c): Pizza Pass (scene 9): the trolls 'Arno',
 * 'Willa', 'Shyler'
 */

#include <stdio.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "net.h"
#include "pizza.h"
#include "platform.h"
#include "random.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* A view's update: redraws button 2 when pizzaGoReady changes, and button 1
   once. */
/* @zoombi32 0x00441127 */
void updatePizzaButtons(View *, short region)
{
    if (pizzaGoReady) {
        if (!g_4a3d98) {
            g_4a3d98 = 1;
            unionRgnRect(region, &pizzaButtons[1].rect);
        }
    } else if (g_4a3d98) {
        g_4a3d98 = 0;
        unionRgnRect(region, &pizzaButtons[1].rect);
    }
    if (!g_4a3d9a) {
        g_4a3d9a = 1;
        unionRgnRect(region, &pizzaButtons[0].rect);
    }
}

/* Draws button `which` (1: 5, 2: 2, or 1 if pizzaGoReady isn't set; the
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
        if (!pizzaGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a3d94->offsets[image] + (char *)g_4a3d94), pizzaButtons[which - 1].rect.left,
                      pizzaButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&pizzaButtons[which - 1].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x00441199 */
void closePizza()
{
    g_4b755e = g_4b166c;
    if (g_4b15e4) {
        g_4b15e4 = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&g_4a3d3c);
        setFreeAtOnce(saved);
        closeGameFile(&g_4b15d0);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Shows the view g_4b162e with script 7001 on (by pizzaLevel), adding it
   if need be, placed by placeMealToppings. */
/* @zoombi32 0x004423d7 */
void showMealView()
{
    View *view = findView(g_4b162e);

    if (view) {
        setViewScript(view, pizzaLevel + 7001, 1);
    } else {
        g_4b162e = addView(0x108000, drawCels, runViewScript, pizzaLevel + 7001, 6, 0, 0, 0);
        view = findView(g_4b162e);
    }
    view->placed = placeMealToppings;
}

/* Sends the party's Zoombinis flagged (their word F7 is 1) to their
   places (g_4a3d54). */
/* Not exact: the original turns the view's register into the Snoid's in
   place (add esi, 0x30) where BCC computes it into eax. */
/* @zoombi32 0x004468eb */
void sendFlaggedToPlaces()
{
    Point where;
    short i;
    View *view;
    Snoid *snoid;

    g_4b1820 = 0;
    for (i = 0; i < pizzaPartySize; i++) {
        where.x = g_4a3d54[i].x;
        where.y = g_4a3d54[i].y;
        if ((view = findView(partyViews[i])) != 0 && ((Snoid *)&view->body)->unknownF7 == 1) {
            snoid = (Snoid *)&view->body;
            setSnoidAction(snoid, 0, &where);
        }
    }
}

/* A random topping (of toppingCount) that troll `troll` (0-2: Arno, Willa, Shyler)
   wants. */
/* @zoombi32 0x00443316 */
short randomWantedTopping(short troll)
{
    short n;

    switch (troll) {
    case 0:
        do
            n = randomUpTo(toppingCount - 1);
        while (!arnoWants[n]);
        break;
    case 1:
        do
            n = randomUpTo(toppingCount - 1);
        while (!willaWants[n]);
        break;
    case 2:
        do
            n = randomUpTo(toppingCount - 1);
        while (!shylerWants[n]);
        break;
    }
    return n;
}

/* Picks the toppings (pickedToppings) at random: each (of toppingCount, but not the
   fifth at level 1) with chance g_4b1626 in 1000, until g_4b1628 have been
   picked; if none was, one of the first four. */
/* @zoombi32 0x0044410b */
void pickToppings()
{
    short none;
    short skip;
    short left;
    short i;

    fillMemory(pickedToppings, 0, 16);
    none = 1;
    skip = -1;
    if (pizzaLevel == 1)
        skip = 4;
    left = g_4b1628;
    do {
        for (i = 0; i < toppingCount; i++)
            if ((short)randomUpTo(1000) < g_4b1626 && !pickedToppings[i] && i != skip) {
                pickedToppings[i]++;
                left--;
                none = 0;
            }
    } while (left > 0);
    if (none)
        pickedToppings[randomUpTo(3)]++;
}

/* Whether the toppings on the pizza (pizzaToppings, as a set of eight bits) are
   one of the sets tried already (triedPizzas, lastTriedPizza + 1 of them). */
/* @zoombi32 0x0044460a */
short pizzaTriedBefore()
{
    short *toppings = pizzaToppings;
    char set;
    short i;

    if (lastTriedPizza < 0)
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
    for (i = 0; i <= lastTriedPizza; i++)
        if (set == triedPizzas[i])
            return 1;
    return 0;
}

/* Records the toppings on the pizza (pizzaToppings) as a set tried (the next
   of triedPizzas). */
/* @zoombi32 0x00444556 */
void recordPizzaTried()
{
    short *toppings = pizzaToppings;

    lastTriedPizza++;
    if (toppings[0])
        triedPizzas[lastTriedPizza] |= 1;
    if (toppings[1])
        triedPizzas[lastTriedPizza] |= 2;
    if (toppings[2])
        triedPizzas[lastTriedPizza] |= 4;
    if (toppings[3])
        triedPizzas[lastTriedPizza] |= 8;
    if (toppings[4])
        triedPizzas[lastTriedPizza] |= 0x10;
    if (toppings[5])
        triedPizzas[lastTriedPizza] |= 0x20;
    if (toppings[6])
        triedPizzas[lastTriedPizza] |= 0x40;
    if (toppings[7])
        triedPizzas[lastTriedPizza] |= 0x80;
}

/* Starts the view zoombiniAtPizza over (action 1) and puts it in front of the
   troll view that's up (arnoView, willaView or shylerView, unless
   g_4b1720), grouped. */
/* @zoombi32 0x0044509b */
void restartPizzaView()
{
    View *view = findView(zoombiniAtPizza);

    view->interval = 6;
    setSnoidAction((Snoid *)&view->body, 1, 0);
    if (!g_4b1720) {
        if (arnoState == 1)
            moveView(zoombiniAtPizza, 1, arnoView);
        else if (willaState == 1)
            moveView(zoombiniAtPizza, 1, willaView);
        else if (shylerState == 1)
            moveView(zoombiniAtPizza, 1, shylerView);
    } else {
        g_4b1720 = 0;
    }
    g_4b15f8 = groupViews(zoombiniAtPizza, zoombiniAtPizza, 0, 0, 0, 0);
}

/* A view's notify: unless busy (g_4b15f2, g_4b160a, pizzaSolved), counts in
   g_4b165a once all the party is through (nextZoombini), else shows the view
   g_4b162e (script 7067, or 7068 from level 1) placed by placeMealToppings. */
/* @zoombi32 0x00445ae1 */
void pizzaDoneNotify(View *, short)
{
    View *view;

    if (!g_4b15f2 && !g_4b160a && !pizzaSolved) {
        if (nextZoombini >= pizzaPartySize) {
            g_4b165a++;
        } else {
            view = findView(g_4b162e);
            if (!pizzaLevel)
                setViewScript(view, 7067, 1);
            else
                setViewScript(view, 7068, 1);
            g_4b160a = groupViews(g_4b162e, g_4b162e, 0, 0, 0, 0);
            view->placed = placeMealToppings;
        }
    }
}

/* A view's placing: drops the cels of toppings (images 57-61 and 67-69)
   not on the pizza (g_4b164a-g_4b1658). `i` never
   moves. */
/* @zoombi32 0x00442443 */
void placeMealToppings(View *view)
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

/* Starts the troll of the level (pizzaLevel: Arno arnoView at 0, Willa
   willaView at 1, Shyler shylerView from 2) on one of its scripts (8014,
   9019-9020 or 10001-10008). */
/* @zoombi32 0x004458c3 */
void startLevelTroll()
{
    View *view;

    g_4b1660 = 1;
    if (!pizzaLevel) {
        view = findView(arnoView);
        setViewScript(view, 8014, 1);
        g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
    } else if (pizzaLevel == 1) {
        view = findView(willaView);
        setViewScript(view, randomUpTo(1) + 9019, 1);
        g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
    } else if (pizzaLevel >= 2) {
        view = findView(shylerView);
        setViewScript(view, randomUpTo(7) + 10001, 1);
        g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
    }
}

/* Steps the trolls' turns on (g_4b165e: 1-4): each of the trolls there
   are at the level in turn (scripts 8032, 9034, 10038), then back to 0. */
/* @zoombi32 0x004459b3 */
void stepTrollTurns()
{
    short *step = &g_4b165e;

    if (*step == 1) {
        setViewScript(findView(arnoView), 8032, 1);
        g_4b1608 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
        (*step)++;
    } else if (!pizzaLevel && *step == 2) {
        *step = 0;
    } else if (pizzaLevel >= 1 && *step == 2) {
        setViewScript(findView(willaView), 9034, 1);
        g_4b1608 = groupViews(willaView, willaView, 0, 0, 0, 0);
        (*step)++;
    } else if (pizzaLevel == 1 && *step == 3) {
        *step = 0;
    } else if (pizzaLevel >= 2 && *step == 3) {
        setViewScript(findView(shylerView), 10038, 1);
        g_4b1608 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
        (*step)++;
    } else if (pizzaLevel >= 2 && *step == 4) {
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

/* Brings the next of the party (nextZoombini) up to the pizza spot (g_4a3d44)
   when the last one's done (g_4b15da), unless busy; counts in g_4b165a
   once they've all been. */
/* Not exact: BCC keeps nextZoombini's address in esi (see docs/findings.md on
   cached global addresses); the original addresses it directly. */
/* @zoombi32 0x00445789 */
void bringNextZoombini()
{
    View *view;

    if (!g_4b165a && !pizzaSolved && (nextZoombini == -1 || g_4b15da)) {
        g_4b15da = 0;
        if (++nextZoombini >= pizzaPartySize) {
            g_4b165a++;
            return;
        }
        g_4b15d8 = 0;
        if (nextZoombini < 0) {
            nextZoombini = 0;
        } else {
            view = findView(partyViews[nextZoombini]);
            if (view->body.x == g_4a3d44.x) {
                zoombiniAtPizza = view->id;
                return;
            }
        }
        if (nextZoombini < pizzaPartySize) {
            view = findView(partyViews[nextZoombini]);
            if (!view)
                return;
            setSnoidAction((Snoid *)&view->body, 7, 0);
            *(Point *)&((Snoid *)&view->body)->targetX = g_4a3d44;
            view->interval = 2;
            zoombiniAtPizza = view->id;
            g_4b15fa = groupViews(zoombiniAtPizza, zoombiniAtPizza, 0, 0, 0, 0);
            if (g_4b15ee)
                g_4b171e++;
        } else {
            g_4b165a++;
        }
    }
}

/* Puts the Zoombini at the pizza (zoombiniAtPizza) and the trolls up (arnoView,
   willaView, shylerView, as arnoState-shylerState say) in front of each other,
   behind g_4b1616; and the view pizzaView too. */
/* @zoombi32 0x00446035 */
void orderPizzaViews()
{
    if (pizzaView) {
        moveView(pizzaView, 1, g_4b1616);
        if (arnoState == 3) {
            if (g_4b1712 >= 0)
                moveView(arnoView, 0, shownPizzas[0].view);
            else
                moveView(arnoView, 0, pizzaView);
        }
    }
    if (zoombiniAtPizza < 0)
        return;
    moveView(zoombiniAtPizza, 0, g_4b1616);
    if (arnoState == 1) {
        moveView(arnoView, 0, zoombiniAtPizza);
        if (shylerState == 1) {
            moveView(shylerView, 0, arnoView);
            if (willaState == 1)
                moveView(willaView, 0, shylerView);
        } else if (willaState == 1) {
            moveView(willaView, 0, arnoView);
        }
    } else if (shylerState == 1) {
        moveView(shylerView, 0, zoombiniAtPizza);
        if (willaState == 1)
            moveView(willaView, 0, shylerView);
    } else if (willaState == 1) {
        moveView(willaView, 0, zoombiniAtPizza);
    }
}

/* Clears the toppings (mealToppings, and the ones shown, g_4b164a-g_4b1658) and sets
   the topping views (toppingViews) to the level's scripts (7005 on). */
/* @zoombi32 0x00446198 */
void clearToppings()
{
    fillMemory(mealToppings, 0, 16);
    g_4b1652 = g_4b1650 = g_4b164c = g_4b164e = g_4b164a = g_4b1654 = g_4b1656 = g_4b1658 = 0;
    switch (pizzaLevel) {
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
   (pizzaToppings; images 5-24 by topping, 25-40 by level too: 29-32 only at
   level 3) and moves the
   rest by (toppingsDx, toppingsDy), or, while g_4b1630, to there from where
   the first one was. */
/* @zoombi32 0x00442a9f */
void placePizzaToppings(View *view)
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
            if (!pizzaToppings[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            if (!pizzaToppings[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!pizzaToppings[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 17:
        case 18:
        case 19:
        case 20:
            if (!pizzaToppings[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 21:
        case 22:
        case 23:
        case 24:
            if (!pizzaToppings[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 25:
        case 26:
        case 27:
        case 28:
            if (!pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 37:
        case 38:
        case 39:
        case 40:
            if (!pizzaToppings[5] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 33:
        case 34:
        case 35:
        case 36:
            if (!pizzaToppings[6] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 29:
        case 30:
        case 31:
        case 32:
            if (!pizzaToppings[7] || pizzaLevel != 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += toppingsDx;
                cel[2] += toppingsDy;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += toppingsDx - dx;
                cel[2] += toppingsDy - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: placePizzaToppings for the toppings troll 0 wants
   (images 156-191). */
/* @zoombi32 0x0044468e */
void placeArnoToppings(View *view)
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
            if (!pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!arnoWants[5] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!arnoWants[6] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!arnoWants[7] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += toppingsDx;
                cel[2] += toppingsDy;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += toppingsDx - dx;
                cel[2] += toppingsDy - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: placePizzaToppings for the toppings troll 1 wants
   (images 156-191, and 212 when arnoState is 3). */
/* @zoombi32 0x0044485d */
void placeWillaToppings(View *view)
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
            if (!pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!willaWants[5] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!willaWants[6] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!willaWants[7] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 212:
            if (arnoState == 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += toppingsDx;
                cel[2] += toppingsDy;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += toppingsDx - dx;
                cel[2] += toppingsDy - dy;
            }
            cel += 3;
        }
    }
}

/* A view's placing: placePizzaToppings for the toppings troll 2 wants
   (images 156-191). */
/* @zoombi32 0x00444a93 */
void placeShylerToppings(View *view)
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
            if (!pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!shylerWants[5] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!shylerWants[6] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!shylerWants[7] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += toppingsDx;
                cel[2] += toppingsDy;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += toppingsDx - dx;
                cel[2] += toppingsDy - dy;
            }
            cel += 3;
        }
    }
}

/*
 * What troll `troll` (0-2) makes of the pizza (pizzaToppings): 0 if it has one
 * topping the troll doesn't want, 4 if more; else 2 if it has all the
 * troll wants, 1 if not all. (3 is never returned.)
 */
/* @zoombi32 0x0044338b */
short judgePizza(short troll)
{
    short never;
    short wanted;
    short right;
    short wrong;
    short i;

    never = wanted = right = wrong = 0;
    switch (troll) {
    case 0:
        for (i = 0; i < toppingCount; i++)
            if (arnoWants[i])
                wanted++;
        for (i = 0; i < toppingCount; i++)
            if (pizzaToppings[i]) {
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
        for (i = 0; i < toppingCount; i++)
            if (willaWants[i])
                wanted++;
        for (i = 0; i < toppingCount; i++)
            if (pizzaToppings[i]) {
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
        for (i = 0; i < toppingCount; i++)
            if (shylerWants[i])
                wanted++;
        for (i = 0; i < toppingCount; i++)
            if (pizzaToppings[i]) {
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
void trollFidget()
{
    short r;
    View *view;

    if (!g_4b1600 && !g_4b1602 && !g_4b1604) {
        switch (pizzaLevel) {
        case 0:
            if (arnoState != 3) {
                view = findView(arnoView);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            }
            break;
        case 1:
            if (randomUpTo(1000) < 500 && arnoState == 1) {
                view = findView(arnoView);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            } else if (willaState == 1) {
                view = findView(willaView);
                setViewScript(view, randomUpTo(1) + 9019, 1);
            }
            break;
        case 2:
        case 3:
            r = randomUpTo(1000);
            if (r < 300 && arnoState == 1) {
                view = findView(arnoView);
                setViewScript(view, randomUpTo(1) + 8034, 1);
            } else if (r < 600 && willaState == 1) {
                view = findView(willaView);
                setViewScript(view, randomUpTo(1) + 9019, 1);
            } else if (shylerState == 1) {
                r = randomUpTo(3);
                if (!r)
                    r = 1;
                else
                    r += 5;
                view = findView(shylerView);
                setViewScript(view, r + 10000, 1);
            }
            break;
        }
    }
}

/* Records the pizza as tried and has the troll whose turn it is
   (trollTurn, or g_4b1670 if set, counted down: 0 Arno, 1 Willa, 2
   Shyler) react (8020, 9026 or 10030), placed by placeTrollToppings; Willa's turn
   is skipped while g_4b16bc. Clears the view pizzaView. */
/* @zoombi32 0x00444c62 */
void trollReacts()
{
    View *view;

    deleteView(pizzaView);
    pizzaView = 0;
    if (g_4b1670)
        trollTurn = g_4b1670;
    trollTurn--;
    recordPizzaTried();
    if (!trollTurn) {
        view = findView(arnoView);
        setViewScript(view, 8020, 1);
        moveView(arnoView, 1, g_4b1616);
        if (pizzasLeft > 0)
            moveView(zoombiniAtPizza, 1, arnoView);
        g_4b1606 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
    } else if (trollTurn == 1) {
        if (g_4b16bc) {
            g_4b15fe = ++trollTurn;
            return;
        }
        view = findView(willaView);
        setViewScript(view, 9026, 1);
        view->notify = willaNotify;
        moveView(willaView, 1, g_4b1616);
        moveView(zoombiniAtPizza, 1, willaView);
        g_4b1606 = groupViews(willaView, willaView, 0, 0, 0, 0);
    } else if (trollTurn == 2) {
        view = findView(shylerView);
        setViewScript(view, 10030, 1);
        moveView(shylerView, 1, g_4b1616);
        g_4b1606 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
    }
    view->placed = placeTrollToppings;
    trollTurn = 0;
}

/* The trolls there are (Arno always; Willa if willaState, Shyler if
   shylerState) eat (8024-8031, 9030-9033, 10035-10037), each placing the
   toppings it wants. */
/* @zoombi32 0x00444391 */
void trollsEat()
{
    View *view;
    short r;

    if (!willaState && !shylerState) {
        view = findView(arnoView);
        setViewScript(view, randomUpTo(1) + 8024, 1);
        view->placed = placeArnoToppings;
    } else if (willaState && !shylerState) {
        r = randomUpTo(1);
        view = findView(arnoView);
        setViewScript(view, r + 8026, 1);
        view->placed = placeArnoToppings;
        view = findView(willaView);
        setViewScript(view, r + 9030, 1);
        view->placed = placeWillaToppings;
    } else if (!willaState && shylerState) {
        r = randomUpTo(1);
        view = findView(arnoView);
        setViewScript(view, r + 8028, 1);
        view->placed = placeArnoToppings;
        view = findView(shylerView);
        setViewScript(view, r + 10035, 1);
        view->placed = placeShylerToppings;
    } else if (willaState && shylerState) {
        r = randomUpTo(1);
        view = findView(arnoView);
        setViewScript(view, r + 8030, 1);
        view->placed = placeArnoToppings;
        view = findView(willaView);
        setViewScript(view, r + 9032, 1);
        view->placed = placeWillaToppings;
        view = findView(shylerView);
        setViewScript(view, r + 10036, 1);
        view->placed = placeShylerToppings;
    }
    g_4b15ec = groupViews(arnoView, arnoView, 0, 0, 0, 0);
}

/* A view's placing: placePizzaToppings with the troll's images (156-191, and 212
   when arnoState is 3) for the toppings on the pizza (pizzaToppings). */
/* @zoombi32 0x00442c6c */
void placeTrollToppings(View *view)
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
            if (!pizzaToppings[4]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 160:
        case 161:
        case 162:
        case 163:
            if (!pizzaToppings[3]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 164:
        case 165:
        case 166:
        case 167:
            if (!pizzaToppings[2]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 168:
        case 169:
        case 170:
        case 171:
            if (!pizzaToppings[1]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 172:
        case 173:
        case 174:
        case 175:
            if (!pizzaToppings[0]) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 176:
        case 177:
        case 178:
        case 179:
            if (!pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 188:
        case 189:
        case 190:
        case 191:
            if (!pizzaToppings[5] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 184:
        case 185:
        case 186:
        case 187:
            if (!pizzaToppings[6] || !pizzaLevel) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 180:
        case 181:
        case 182:
        case 183:
            if (!pizzaToppings[7] || pizzaLevel != 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        case 212:
            if (arnoState == 3) {
                removeFirstCel((ViewCel *)cel);
                removed++;
            }
            break;
        }
        if (!removed) {
            if (!g_4b1630) {
                cel[1] += toppingsDx;
                cel[2] += toppingsDy;
            } else {
                if (first) {
                    dx = cel[1];
                    dy = cel[2];
                    first = 0;
                }
                cel[1] += toppingsDx - dx;
                cel[2] += toppingsDy - dy;
            }
            cel += 3;
        }
    }
}

/*
 * Willa's notify: 32 puts the pizza up (pizzaView, script 12000) in front;
 * 60 has the Zoombini at the pizza step (13000 on, by its feet); 99 puts
 * her in front of Arno; when her script ends, the Zoombini reacts (13005,
 * 13010 or 13015 on, by the troll up; notify pizzaZoombiniNotify) and the pizza with
 * it (12001, 12006 or 12011 on).
 */
/* @zoombi32 0x004441a8 */
void willaNotify(View *, short event)
{
    View *view;
    short feet;
    short script;

    switch (event) {
    case 32:
        view = findView(pizzaView);
        if (view) {
            setViewScript(view, 12000, 1);
        } else {
            pizzaView = addView(0x108000, drawCels, runViewScript, 12000, 6, 0, 0, 0);
            view = findView(pizzaView);
            moveView(g_4b1616, 1, pizzaView);
        }
        view->placed = placePizzaToppings;
        orderPizzaViews();
        g_4b15f4 = groupViews(pizzaView, pizzaView, 0, 0, 0, 0);
        showMealView();
        break;
    case 60:
        view = findView(zoombiniAtPizza);
        feet = ((Snoid *)&view->body)->features[3] - 1;
        startSnoidScript((Snoid *)&view->body, feet + 13000, 0, 0);
        view->notifyEnd = 1;
        view->notify = willaNotify;
        break;
    case 99:
        if (arnoState == 1)
            moveView(willaView, 0, arnoView);
        break;
    case -1:
        view = findView(zoombiniAtPizza);
        feet = ((Snoid *)&view->body)->features[3] - 1;
        if (arnoState == 1) {
            script = feet + 13005;
            feet += 12001;
        } else if (willaState == 1) {
            script = feet + 13010;
            feet += 12006;
        } else {
            script = feet + 13015;
            feet += 12011;
        }
        startSnoidScript((Snoid *)&view->body, script, 0, 0);
        view->notifyEnd = 0;
        view->notify = pizzaZoombiniNotify;
        view = findView(pizzaView);
        setViewScript(view, feet, 1);
        view->placed = placePizzaToppings;
        orderPizzaViews();
        g_4b15f6 = groupViews(pizzaView, zoombiniAtPizza, 0, 0, 0, 0);
        showMealView();
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
void pizzaZoombiniNotify(View *view, short event)
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
        zoombini = findView(zoombiniAtPizza);
        last = 0;
        if (!g_4b1646)
            last = 1;
        if (arnoState == 1) {
            if (!g_4b1646) {
                where.x = 34;
                where.y = 59;
            }
            startSnoidScript((Snoid *)&zoombini->body, g_4b1646 + 14000, &where, last);
            queueViewSound(8040, 0);
        } else if (willaState == 1) {
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
        zoombini->notify = pizzaZoombiniNotify;
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
            zoombini = findView(zoombiniAtPizza);
            where = g_4a3d44;
            if (zoombini) {
                setSnoidAction((Snoid *)&zoombini->body, 7, 0);
                *(Point *)&((Snoid *)&zoombini->body)->targetX = where;
            }
            g_4b15ea = 1;
            g_4b15da = 0;
        } else if (g_4b15e0) {
            placeClaims[0] = 0;
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

/* A pizza is served: counts down the pizzas left (pizzasLeft; g_4b1646 set
   while some are); the troll up takes it (8022, 9028 or 10032 on) and the
   Zoombini at the pizza gets its notify, unless it's the last with none
   left to judge, when the pizza view starts over (restartPizzaView). */
/* @zoombi32 0x00445153 */
void servePizza()
{
    View *view;

    g_4b165c = 0;
    g_4b1646 = 0;
    if (--pizzasLeft >= 0)
        g_4b1646 = 1;
    if (!pizzasLeft)
        g_4b1648++;
    if (g_4b1648 || !g_4b1646) {
        if (arnoState == 1) {
            view = findView(arnoView);
            setViewScript(view, g_4b1646 + 8022, 1);
            if (g_4b1648) {
                g_4b16bc++;
                g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            } else {
                g_4b15fc = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            }
        } else if (willaState == 1) {
            view = findView(willaView);
            setViewScript(view, g_4b1646 + 9028, 1);
            g_4b15fc = groupViews(willaView, willaView, 0, 0, 0, 0);
        } else if (shylerState == 1) {
            view = findView(shylerView);
            setViewScript(view, g_4b1646 + 10032, 1);
            g_4b15fc = groupViews(shylerView, shylerView, 0, 0, 0, 0);
        }
        view->notifyEnd = 0;
        view->notify = pizzaZoombiniNotify;
        g_4b1648 = g_4b171a = 0;
    } else {
        restartPizzaView();
        g_4b171a++;
        g_4b15fc = 1000;
    }
    claimPlacedView(1, 0);
}

/* Shows four pizzas, each with two of the toppings a, b, c and d (a and
   b, b and c, c and d, a and d), recording each as tried and shown
   (shownPizzas; scripts 12042 on, g_4b170e counting). */
/* @zoombi32 0x00446487 */
void showFourPizzas(short a, short b, short c, short d)
{
    fillMemory(pizzaToppings, 0, 16);
    pizzaToppings[a] = 1;
    pizzaToppings[b] = 1;
    recordPizzaTried();
    g_4b1712++;
    shownPizzas[g_4b1712].set = triedPizzas[lastTriedPizza];
    shownPizzas[g_4b1712].unknown4 = 4;
    g_4b170e++;
    shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    shownPizzas[g_4b1712].script = g_4b170e + 12025;
    findView(shownPizzas[g_4b1712].view)->placed = placePizzaToppings;
    updateViews();
    fillMemory(pizzaToppings, 0, 16);
    pizzaToppings[b] = 1;
    pizzaToppings[c] = 1;
    recordPizzaTried();
    g_4b1712++;
    shownPizzas[g_4b1712].set = triedPizzas[lastTriedPizza];
    shownPizzas[g_4b1712].unknown4 = 4;
    g_4b170e++;
    shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    shownPizzas[g_4b1712].script = g_4b170e + 12025;
    findView(shownPizzas[g_4b1712].view)->placed = placePizzaToppings;
    updateViews();
    fillMemory(pizzaToppings, 0, 16);
    pizzaToppings[c] = 1;
    pizzaToppings[d] = 1;
    recordPizzaTried();
    g_4b1712++;
    shownPizzas[g_4b1712].set = triedPizzas[lastTriedPizza];
    shownPizzas[g_4b1712].unknown4 = 4;
    g_4b170e++;
    shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 6, 0, 0, 0);
    shownPizzas[g_4b1712].script = g_4b170e + 12025;
    findView(shownPizzas[g_4b1712].view)->placed = placePizzaToppings;
    updateViews();
    fillMemory(pizzaToppings, 0, 16);
    pizzaToppings[a] = 1;
    pizzaToppings[d] = 1;
    recordPizzaTried();
    g_4b1712++;
    shownPizzas[g_4b1712].set = triedPizzas[lastTriedPizza];
    shownPizzas[g_4b1712].unknown4 = 4;
    g_4b170e++;
    shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12041, 0, 0, 0, 0);
    shownPizzas[g_4b1712].script = g_4b170e + 12025;
    findView(shownPizzas[g_4b1712].view)->placed = placePizzaToppings;
    updateViews();
    fillMemory(pizzaToppings, 0, 16);
}

/* Debugging: shows what each troll there wants and the meal on the pizza
   (mealToppings) in a box at the top right. */
/* @zoombi32 0x00443e2e */
void drawTrollWants()
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
    sprintf(meal, "Meal     %d %d %d %d %d %d %d %d", mealToppings[0], mealToppings[1], mealToppings[2], mealToppings[3],
            mealToppings[4], mealToppings[5], mealToppings[6], mealToppings[7]);
    fillPortRect(box, Color(14), 0);
    frameRect(box);
    if (arnoState == 1)
        drawText(arnoLine, 0x22, arno, 0xffff);
    if (willaState == 1)
        drawText(willaLine, 0x22, willa, 0xffff);
    if (shylerState == 1)
        drawText(shylerLine, 0x22, shyler, 0xffff);
    drawText(mealLine, 0x22, meal, 0xffff);
    setForeColor(saved);
    showRect(&box);
}

/*
 * A troll reacts to the pizza by its verdict (judgePizza): for 0 or 4, the
 * troll is picked from those there (with both Willa and Shyler there, it
 * sets willaState again from shylerState first); 1 (not all it wants) it
 * asks for more (8000, 9021 or 10009 on, by g_4b16b6, g_4b16be,
 * g_4b16c4); otherwise it grumbles (8015, 9017 or 10027 on).
 */
/* @zoombi32 0x00445b80 */
void trollVerdict(short troll, short verdict)
{
    View *view;
    long both;

    g_4b16ea = 0;
    if (!verdict || verdict == 4) {
        if (arnoState == 1) {
            if (willaState != 1 && shylerState != 1)
                troll = 0;
            else if (willaState == 1 && shylerState != 1)
                troll = randomUpTo(1);
            else if (willaState != 1 && shylerState == 1)
                troll = randomUpTo(1) * 2;
            else {
                if (shylerState != 1)
                    both = 0;
                else
                    both = 1;
                if ((willaState = both) != 0)
                    troll = randomUpTo(2);
            }
        } else if (willaState == 1) {
            if (shylerState != 1)
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
            view = findView(arnoView);
            setViewScript(view, g_4b16b6 + 8000, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            g_4b1670 = 1;
            g_4b1710 = 5;
        } else {
            view = findView(arnoView);
            setViewScript(view, randomUpTo(1) + 8015, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    case 1:
        g_4b15fe = 2;
        if (verdict == 1) {
            view = findView(willaView);
            setViewScript(view, g_4b16be + 9021, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            g_4b1670 = 2;
            g_4b1710 = 6;
        } else {
            view = findView(willaView);
            setViewScript(view, randomUpTo(1) + 9017, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    case 2:
        g_4b15fe = 3;
        if (verdict == 1) {
            view = findView(shylerView);
            setViewScript(view, g_4b16c4 + 10009, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            g_4b1670 = 3;
            g_4b1710 = 7;
        } else {
            view = findView(shylerView);
            setViewScript(view, randomUpTo(2) + 10027, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            g_4b1710 = 4;
        }
        break;
    }
}

/*
 * Shows the pizza just judged (g_4b1710: 4 thrown, 5-7 on the pile of
 * the troll that took it) and records it (shownPizzas): thrown ones cycle
 * through 16 scripts (12025 on, skipping 13; once they've all been used,
 * g_4a3dcc, the view already there is reused); piled ones take the next
 * of three places per pile (12016, 12019 or 12022 on), each in front of
 * the one it replaces.
 */
/* @zoombi32 0x00445307 */
void showJudgedPizza()
{
    short behind;
    View *view;
    short i;

    g_4b1712++;
    shownPizzas[g_4b1712].set = triedPizzas[lastTriedPizza];
    shownPizzas[g_4b1712].unknown4 = g_4b1710;
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
            shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b170e + 12025, 6, 0, 0, 0);
            shownPizzas[g_4b1712].script = g_4b170e + 12025;
            view = findView(shownPizzas[g_4b1712].view);
            view->placed = placePizzaToppings;
        } else {
            for (i = 0; i < 28; i++)
                if (shownPizzas[i].script == g_4b170e + 12025) {
                    view = findView(shownPizzas[i].view);
                    setViewScript(view, g_4b170e + 12025, 1);
                    view->placed = placePizzaToppings;
                    break;
                }
        }
        break;
    case 5:
        if (++g_4b1714 > 2)
            g_4b1714 = 0;
        if (g_4b1722[g_4b1714])
            behind = g_4b1722[g_4b1714];
        shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1714 + 12016, 6, 0, 0, 0);
        shownPizzas[g_4b1712].script = g_4b1714 + 12016;
        view = findView(shownPizzas[g_4b1712].view);
        view->placed = placePizzaToppings;
        if (behind)
            moveView(shownPizzas[g_4b1712].view, 1, behind);
        g_4b1722[g_4b1714] = shownPizzas[g_4b1712].view;
        shownPizzas[g_4b1712].unknown4 = 5;
        break;
    case 6:
        if (++g_4b1716 > 2)
            g_4b1716 = 0;
        if (g_4b1728[g_4b1716])
            behind = g_4b1728[g_4b1716];
        shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1716 + 12019, 6, 0, 0, 0);
        shownPizzas[g_4b1712].script = g_4b1716 + 12019;
        view = findView(shownPizzas[g_4b1712].view);
        view->placed = placePizzaToppings;
        if (behind)
            moveView(shownPizzas[g_4b1712].view, 1, behind);
        g_4b1728[g_4b1716] = shownPizzas[g_4b1712].view;
        shownPizzas[g_4b1712].unknown4 = 6;
        break;
    case 7:
        if (++g_4b1718 > 2)
            g_4b1718 = 0;
        if (g_4b172e[g_4b1718])
            behind = g_4b172e[g_4b1718];
        shownPizzas[g_4b1712].view = addView(0x4108000, drawCels, runViewScript, g_4b1718 + 12022, 6, 0, 0, 0);
        shownPizzas[g_4b1712].script = g_4b1718 + 12022;
        view = findView(shownPizzas[g_4b1712].view);
        view->placed = placePizzaToppings;
        if (behind)
            moveView(shownPizzas[g_4b1712].view, 1, behind);
        g_4b172e[g_4b1718] = shownPizzas[g_4b1712].view;
        shownPizzas[g_4b1712].unknown4 = 7;
        break;
    }
    view->notifyEnd = 1;
    view->notify = pizzaDoneNotify;
    orderPizzaViews();
    if (shownPizzas[g_4b1712].unknown4 == 4) {
        moveView(shownPizzas[g_4b1712].view, 1, g_4b1616);
    } else if (shownPizzas[g_4b1712].unknown4 == 6 && g_4b1728[1]) {
        moveView(g_4b1728[1], 0, willaView);
        moveView(g_4b1728[0], 0, g_4b1728[1]);
    }
    moveView(g_4a3d38, 0, -1);
    g_4b160c = groupViews(shownPizzas[g_4b1712].view, shownPizzas[g_4b1712].view, 0, 0, 0, 0);
}

/* A topping button (4-11: the eight toppings, those the level has; 3
   serves the pizza): toggles the topping on the meal (mealToppings) and its
   view (by level), and redraws the pizza; 3 has the pizza carried off
   (7057, or 7058 from level 1; 7066) and clears the toppings, without
   the redraw. */
/* @zoombi32 0x00442560 */
void toppingButton(short button)
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
        if (!pizzaLevel)
            setViewScript(view, 7057, 1);
        else
            setViewScript(view, 7058, 1);
        view = findView(g_4b15f0);
        setViewScript(view, 7066, 1);
        g_4b15f2 = groupViews(g_4b15f0, g_4b15f0, 0, 0, 0, 0);
        view->notify = willaNotify;
        clearToppings();
        break;
    case 4:
        g_4b1652 ^= 1;
        mealToppings[0] = g_4b1652;
        view = findView(toppingViews[0]);
        if (!pizzaLevel)
            setViewScript(view, g_4b1652 + 7005, 1);
        else if (pizzaLevel == 1)
            setViewScript(view, g_4b1652 + 7015, 1);
        else if (pizzaLevel == 2)
            setViewScript(view, g_4b1652 + 7027, 1);
        else
            setViewScript(view, g_4b1652 + 7041, 1);
        break;
    case 5:
        g_4b1650 ^= 1;
        mealToppings[1] = g_4b1650;
        view = findView(toppingViews[1]);
        if (!pizzaLevel)
            setViewScript(view, g_4b1650 + 7007, 1);
        else if (pizzaLevel == 1)
            setViewScript(view, g_4b1650 + 7017, 1);
        else if (pizzaLevel == 2)
            setViewScript(view, g_4b1650 + 7029, 1);
        else
            setViewScript(view, g_4b1650 + 7043, 1);
        break;
    case 6:
        g_4b164e ^= 1;
        mealToppings[2] = g_4b164e;
        view = findView(toppingViews[2]);
        if (!pizzaLevel)
            setViewScript(view, g_4b164e + 7009, 1);
        else if (pizzaLevel == 1)
            setViewScript(view, g_4b164e + 7019, 1);
        else if (pizzaLevel == 2)
            setViewScript(view, g_4b164e + 7031, 1);
        else
            setViewScript(view, g_4b164e + 7045, 1);
        break;
    case 7:
        g_4b164c ^= 1;
        mealToppings[3] = g_4b164c;
        view = findView(toppingViews[3]);
        if (!pizzaLevel)
            setViewScript(view, g_4b164c + 7011, 1);
        else if (pizzaLevel == 1)
            setViewScript(view, g_4b164c + 7021, 1);
        else if (pizzaLevel == 2)
            setViewScript(view, g_4b164c + 7033, 1);
        else
            setViewScript(view, g_4b164c + 7047, 1);
        break;
    case 8:
        g_4b164a ^= 1;
        mealToppings[4] = g_4b164a;
        view = findView(toppingViews[4]);
        if (!pizzaLevel)
            setViewScript(view, g_4b164a + 7013, 1);
        else if (pizzaLevel == 2)
            setViewScript(view, g_4b164a + 7035, 1);
        else if (pizzaLevel == 3)
            setViewScript(view, g_4b164a + 7049, 1);
        break;
    case 9:
        if (pizzaLevel) {
            g_4b1654 ^= 1;
            mealToppings[5] = g_4b1654;
            view = findView(toppingViews[5]);
            if (pizzaLevel == 1)
                setViewScript(view, g_4b1654 + 7023, 1);
            else if (pizzaLevel == 2)
                setViewScript(view, g_4b1654 + 7037, 1);
            else
                setViewScript(view, g_4b1654 + 7051, 1);
        }
        break;
    case 10:
        if (pizzaLevel) {
            g_4b1656 ^= 1;
            mealToppings[6] = g_4b1656;
            view = findView(toppingViews[6]);
            if (pizzaLevel == 1)
                setViewScript(view, g_4b1656 + 7025, 1);
            else if (pizzaLevel == 2)
                setViewScript(view, g_4b1656 + 7039, 1);
            else
                setViewScript(view, g_4b1656 + 7053, 1);
        }
        break;
    case 11:
        if (pizzaLevel == 3) {
            g_4b1658 ^= 1;
            mealToppings[7] = g_4b1658;
            view = findView(toppingViews[7]);
            setViewScript(view, g_4b1658 + 7055, 1);
        }
        break;
    }
    if (redraw)
        showMealView();
}

/* The scene's keys (debugging ones only while debugging messages are on):
   A shows the trolls' wants; R, O, D in turn arm the rest (g_4b15e8), then
   P makes the trolls eat, N, W and S step Arno, Willa and Shyler through
   their scripts, and space sets the pizzas left (pizzasLeft) to g_4b1634.
   Notes the time of the key (g_4b1824). */
/* @zoombi32 0x00442166 */
short pizzaKey(unsigned short key)
{
    g_4b1824 = clockTime();
    if (!debugMessagesOn && key != 367)
        return 0;
    switch (key) {
    case 367:
        replayHint();
        return 1;
    case 'A':
    case 'a':
        drawTrollWants();
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
            trollsEat();
            g_4b15ec = 0;
            return 1;
        }
        break;
    case 'N':
    case 'n':
        if (g_4b15e8 >= 3) {
            if (g_4a3d9e >= 36)
                g_4a3d9e = 0;
            setViewScript(findView(arnoView), g_4a3d9e++ + 8000, 1);
            orderPizzaViews();
            return 1;
        }
        break;
    case 'S':
    case 's':
        if (g_4b15e8 >= 3) {
            if (g_4a3da2 >= 39)
                g_4a3da2 = 0;
            setViewScript(findView(shylerView), g_4a3da2++ + 10000, 1);
            orderPizzaViews();
            return 1;
        }
        break;
    case 'W':
    case 'w':
        if (g_4b15e8 >= 3) {
            if (g_4a3da0 >= 35)
                g_4a3da0 = 0;
            setViewScript(findView(willaView), g_4a3da0++ + 9000, 1);
            orderPizzaViews();
            return 1;
        }
        break;
    case ' ':
        if (g_4b15e8 >= 3) {
            pizzasLeft = g_4b1634;
            return 1;
        }
        break;
    default:
        return 0;
    }
    return 0;
}

/*
 * Shares the toppings picked (pickToppings) out among the trolls there (at
 * random; from level 2 each troll gets at least one, taken from the troll
 * with more), then, at level 1 or 3, shows four pizzas made from two
 * toppings the troll with the fewest wants and one each the others want.
 */
/* @zoombi32 0x00442ea2 */
void shareToppings()
{
    short shyler;
    short a;
    short b;
    short c;
    short d;
    short i;
    short arno;
    short willa;
    short fewest;

    fillMemory(arnoWants, 0, 16);
    fillMemory(willaWants, 0, 16);
    fillMemory(shylerWants, 0, 16);
    pickToppings();
    arno = 0;
    willa = 0;
    shyler = 0;
    a = b = c = d = -1;
    switch (pizzaLevel) {
    case 0:
        for (i = 0; i < toppingCount; i++)
            arnoWants[i] = pickedToppings[i];
        break;
    case 1:
        for (i = 0; i < toppingCount; i++)
            if (pickedToppings[i]) {
                if (!randomUpTo(1)) {
                    arnoWants[i] = 1;
                    arno++;
                } else {
                    willaWants[i] = 1;
                    willa++;
                }
            }
        if (!arno && !willa) {
            i = randomUpTo(toppingCount - 1);
            if (randomUpTo(1000) < 500)
                willaWants[i] = 1;
            else
                arnoWants[i] = 1;
        }
        break;
    case 2:
    case 3:
        for (i = 0; i < toppingCount; i++)
            if (pickedToppings[i]) {
                short r = randomUpTo(2);

                if (!r) {
                    arnoWants[i] = 1;
                    arno++;
                } else if (r == 1) {
                    willaWants[i] = 1;
                    willa++;
                } else {
                    shylerWants[i] = 1;
                    shyler++;
                }
            }
        do {
            if (!arno) {
                if (willa > shyler) {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!willaWants[i]);
                    willaWants[i] = 0;
                    willa--;
                    arnoWants[i] = 1;
                    arno = 1;
                } else {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!shylerWants[i]);
                    shylerWants[i] = 0;
                    shyler--;
                    arnoWants[i] = 1;
                    arno = 1;
                }
            }
            if (!willa) {
                if (arno > shyler) {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!arnoWants[i]);
                    arnoWants[i] = 0;
                    arno--;
                    willaWants[i] = 1;
                    willa = 1;
                } else {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!shylerWants[i]);
                    shylerWants[i] = 0;
                    shyler--;
                    willaWants[i] = 1;
                    willa = 1;
                }
            }
            if (!shyler) {
                if (arno > willa) {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!arnoWants[i]);
                    arnoWants[i] = 0;
                    arno--;
                    shylerWants[i] = 1;
                    shyler = 1;
                } else {
                    do
                        i = randomUpTo(toppingCount - 1);
                    while (!willaWants[i]);
                    willaWants[i] = 0;
                    willa--;
                    shylerWants[i] = 1;
                    shyler = 1;
                }
            }
        } while (!arno || !willa || !shyler);
        if (pizzaLevel == 2)
            break;
        fewest = 0;
        if (willa > arno) {
            fewest = 1;
            if (willa < shyler)
                fewest = 2;
        } else if (arno < shyler) {
            fewest = 2;
        }
        switch (fewest) {
        case 0:
            a = randomWantedTopping(0);
            do
                c = randomWantedTopping(0);
            while (c == a);
            b = randomWantedTopping(1);
            d = randomWantedTopping(2);
            break;
        case 1:
            a = randomWantedTopping(1);
            do
                c = randomWantedTopping(1);
            while (c == a);
            b = randomWantedTopping(0);
            d = randomWantedTopping(2);
            break;
        case 2:
            a = randomWantedTopping(2);
            do
                c = randomWantedTopping(2);
            while (c == a);
            b = randomWantedTopping(0);
            d = randomWantedTopping(1);
            break;
        }
        showFourPizzas(a, b, c, d);
        break;
    }
}

/*
 * A pizza served to troll `troll` (0-2), unless it's done (3): it reacts
 * by its verdict (judgePizza): 2 (all it wants) it's satisfied (state 2);
 * 1 it wants more (the next of its scripts; the pizza goes on its pile
 * the first time); 0 and 4 it rejects it; 3 never happens. Once every
 * troll there is satisfied, the puzzle is solved (pizzaSolved).
 */
/* @zoombi32 0x00443521 */
void pizzaServedTo(short troll, short)
{
    View *view;

    g_4b1824 = clockTime();
    view = 0;
    switch (troll) {
    case 0:
        if (arnoState == 3)
            break;
        switch (judgePizza(0)) {
        case 2:
            view = findView(arnoView);
            setViewScript(view, randomUpTo(2) + 8017, 1);
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            arnoState = 2;
            g_4b16ea++;
            break;
        case 1:
            view = findView(arnoView);
            setViewScript(view, g_4b16b6 + 8000, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            if (++g_4b16b6 >= 6)
                g_4b16b6 = 5;
            g_4a3d40 = 0;
            if (!g_4b1670) {
                g_4b1670 = 1;
                g_4b1710 = 5;
            }
            g_4b15fe = 1;
            g_4b16ea = 0;
            break;
        case 0:
            view = findView(arnoView);
            setViewScript(view, g_4b16b8 + 8006, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            if (++g_4b16b8 >= 2)
                g_4b16b8 = 0;
            g_4b16ea = 0;
            g_4b15fe = 1;
            g_4b1710 = 4;
            break;
        case 4:
            view = findView(arnoView);
            setViewScript(view, g_4b16ba + 8008, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            if (++g_4b16ba >= 6)
                g_4b16ba = 0;
            g_4b16ea = 0;
            g_4b15fe = 1;
            g_4b1710 = 4;
            break;
        case 3:
            view = findView(arnoView);
            setViewScript(view, randomUpTo(1) + 8015, 1);
            orderPizzaViews();
            g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
            g_4b16ea = 0;
            g_4b15fe = 1;
            g_4b1710 = 4;
            break;
        }
        break;
    case 1:
        if (willaState == 3)
            break;
        switch (judgePizza(1)) {
        case 2:
            view = findView(willaView);
            setViewScript(view, randomUpTo(6) + 9010, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            willaState = 2;
            g_4b16ea++;
            break;
        case 1:
            view = findView(willaView);
            setViewScript(view, g_4b16be + 9021, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            if (++g_4b16be >= 5)
                g_4b16be = 4;
            g_4a3d40 = 1;
            if (!g_4b1670) {
                g_4b1670 = 2;
                g_4b1710 = 6;
            }
            g_4b15fe = 2;
            g_4b16ea = 0;
            break;
        case 0:
            view = findView(willaView);
            setViewScript(view, g_4b16c0 + 9000, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            if (++g_4b16c0 >= 5)
                g_4b16c0 = 0;
            g_4b16ea = 0;
            g_4b15fe = 2;
            if (!g_4b1670)
                g_4b1710 = 4;
            break;
        case 4:
            view = findView(willaView);
            setViewScript(view, g_4b16c2 + 9005, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            if (++g_4b16c2 >= 5)
                g_4b16c2 = 0;
            g_4b16ea = 0;
            g_4b15fe = 2;
            if (!g_4b1670)
                g_4b1710 = 4;
            break;
        case 3:
            view = findView(willaView);
            setViewScript(view, randomUpTo(1) + 9017, 1);
            orderPizzaViews();
            g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
            g_4b16ea = 0;
            g_4b15fe = 2;
            g_4b1710 = 4;
            break;
        }
        break;
    case 2:
        if (shylerState == 3)
            break;
        switch (judgePizza(2)) {
        case 2:
            view = findView(shylerView);
            setViewScript(view, randomUpTo(3) + 10023, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            shylerState = 2;
            g_4b16ea++;
            break;
        case 1:
            view = findView(shylerView);
            setViewScript(view, g_4b16c4 + 10009, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            if (++g_4b16c4 >= 5)
                g_4b16c4 = 4;
            g_4a3d40 = 2;
            if (!g_4b1670) {
                g_4b1670 = 3;
                g_4b1710 = 7;
            }
            g_4b16ea = 0;
            g_4b15fe = 3;
            break;
        case 0:
            view = findView(shylerView);
            setViewScript(view, g_4b16c6 + 10014, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            if (++g_4b16c6 >= 6)
                g_4b16c6 = 0;
            g_4b16ea = 0;
            g_4b15fe = 3;
            if (!g_4b1670)
                g_4b1710 = 4;
            break;
        case 4:
            view = findView(shylerView);
            setViewScript(view, g_4b16c8 + 10020, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            if (++g_4b16c8 >= 3)
                g_4b16c8 = 0;
            g_4b16ea = 0;
            g_4b15fe = 3;
            if (!g_4b1670)
                g_4b1710 = 4;
            break;
        case 3:
            view = findView(shylerView);
            setViewScript(view, randomUpTo(2) + 10027, 1);
            orderPizzaViews();
            g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
            g_4b16ea = 0;
            g_4b15fe = 3;
            g_4b1710 = 4;
            break;
        }
        break;
    }
    if (view)
        view->placed = placeTrollToppings;
    if (arnoState >= 2 && !willaState && !shylerState)
        pizzaSolved = 1;
    else if (arnoState >= 2 && willaState >= 2 && !shylerState)
        pizzaSolved = 1;
    else if (arnoState >= 2 && !willaState && shylerState >= 2)
        pizzaSolved = 1;
    else if (arnoState >= 2 && willaState >= 2 && shylerState >= 2)
        pizzaSolved = 1;
    if (pizzaSolved)
        g_4b181c = pizzaPartySize - 1;
}

/* Plays sound `sound` and waits for it (awaitSound); unloads it unless
   `keep`. Returns whether it was cut short. */
/* @zoombi32 0x00445feb */
short playAndWait(short sound, short keep)
{
    short stopped = 0;
    short played;

    playSoundOn(sound, RESOURCE_TYPE(0, 'S', 'N', 'D'), 0);
    played = awaitSound(sound, RESOURCE_TYPE(0, 'S', 'N', 'D'), 3, 1);
    if (!keep)
        unloadSoundNow(sound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    if (!played)
        stopped++;
    return stopped;
}

/* Says one of the scene's introductions (0-4; sounds 15000-15006), if
   sound is on, stopping when one's cut short. */
/* @zoombi32 0x00445eb3 */
void sayIntroduction(short which)
{
    short stopped;

    if (!soundOn || which > 4)
        return;
    switch (which) {
    case 0:
        stopped = playAndWait(15005, 0);
        waitForEventFor(0, 60, 0, 1);
        if (!stopped)
            playAndWait(15006, 0);
        break;
    case 1:
        stopped = playAndWait(15000, 0);
        if (!stopped)
            playAndWait(15001, 0);
        break;
    case 2:
        playAndWait(15002, 0);
        break;
    case 3:
        stopped = playAndWait(15003, 0);
        if (!stopped)
            playAndWait(15004, 0);
        break;
    case 4:
        stopped = playAndWait(15003, 0);
        if (!stopped) {
            stopped = playAndWait(15004, 0);
            if (!stopped) {
                waitForEventFor(0, 20, 0, 1);
                stopped = playAndWait(15005, 0);
                if (!stopped) {
                    waitForEventFor(0, 60, 0, 1);
                    playAndWait(15006, 0);
                }
            }
        }
        break;
    }
}

/*
 * The scene's buttons: leaves at once if asked to; 1 asks to leave for the
 * map (999, keeping the party); 2, once the party is through (pizzaGoReady),
 * sends the Zoombinis on (996) and asks to leave for scene 4; 3 (and 12)
 * serves the pizza when nothing's going on; 4-11 toggle toppings (8 not
 * at level 1, 11 only at level 3); 13 (debugging, armed) drags a Zoombini
 * to the pizza spot. Notes the time (g_4b1824).
 */
/* @zoombi32 0x00441e78 */
void pizzaButtonClicked(short button)
{
    Point cursor;
    View *view;

    g_4b1824 = clockTime();
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closePizza();
        return;
    }
    switch (button) {
    case 1:
        queueViewSound(999, 0);
        drawPizzaButton(button, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawPizzaButton(button, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (g_4b1820)
            sendFlaggedToPlaces();
        if (pizzaGoReady) {
            queueViewSound(0, 0);
            drawPizzaButton(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawPizzaButton(button, 0, 1);
            chooseSnoids(1, 0);
            g_4b7564 = 0;
            queueViewSound(996, 0);
            sendSnoids(690, 250, 45);
            sceneDue = 4;
            g_4b15ec = g_4b1820 = 0;
        }
        break;
    case 3:
    case 12:
        if (!g_4b171e && !g_4b1660 && !pizzaSolved && !g_4b165a && !g_4b1600 && !g_4b1602 && !g_4b1604 && !g_4b160c
            && placeClaims[0]) {
            g_4b171e++;
            g_4b1824 = clockTime();
            if (nextZoombini == -1)
                bringNextZoombini();
            memcpy(pizzaToppings, mealToppings, 16);
            if (!g_4b165a)
                toppingButton(3);
        }
        break;
    case 13:
        if (g_4b15e8 >= 6 && snoidsOnTheirWay <= 0 && g_4b15d8) {
            claimPlacedView(1, 0);
            getCursorPosition(&cursor);
            view = viewAt(cursor, 1, 1);
            if (view) {
                dragSnoid(view, cursor, 0, 0);
                g_4b1672 = view->id;
                if (heldPlaceNumber()) {
                    g_4b15d8 = 0;
                    zoombiniAtPizza = view->id;
                    toppingButton(3);
                }
            }
        }
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        toppingButton(button);
        break;
    case 8:
        if (pizzaLevel != 1)
            toppingButton(button);
        break;
    case 9:
    case 10:
        toppingButton(button);
        break;
    case 11:
        if (pizzaLevel == 3)
            toppingButton(button);
        break;
    }
}

/*
 * The scene's frame: leaves once asked to (and sound 996 is done); has a
 * troll fidget after a minute's quiet; then steps the scene's sequences on
 * as each group of views finishes (groupLeader): a pizza judged (g_4b15f6:
 * a set tried before is rejected by the first troll wanting more, else
 * each troll there in turn takes it), a troll done eating (g_4b1600-
 * g_4b1604: satisfied trolls step aside and the next comes up), the
 * trolls' turns, a pizza's reaction and piling, the next Zoombini up;
 * walks the Zoombini at the pizza to its spot, and sends waiting ones
 * fidgeting (13035 on) while g_4b1820. Several calls to pizzaDoneNotify pass
 * `view` before it's set, as the original does (pizzaDoneNotify ignores it).
 */
/* Not exact: BCC caches groupLeader's address in edi and g_4b165c's in
   esi; the original the other way round (and `reacted` in edi). */
/* @zoombi32 0x004411f2 */
void pizzaFrame()
{
    View *view;
    short reacted;
    short done;
    short n;
    short script;
    View *waiting;

    if (g_4a3d9c || !g_4b15e4)
        return;
    g_4a3d9c = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            g_4a3d9c = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closePizza();
                g_4a3d9c = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (clockTime() - g_4b1824 > 3600) {
        g_4b1824 = clockTime();
        if (!dialogFlags)
            trollFidget();
    }
    if (g_4b15f2 && !groupLeader[g_4b15f2])
        g_4b15f2 = 0;
    if (g_4b15f4) {
        if (!groupLeader[g_4b15f4])
            g_4b15f4 = 0;
    } else if (g_4b15f6) {
        if (!groupLeader[g_4b15f6]) {
            g_4b1670 = 0;
            g_4b15f6 = 0;
            g_4b1674 = pizzaTriedBefore();
            if (!g_4b1674) {
                if (arnoState == 1)
                    pizzaServedTo(0, 0);
                else if (willaState == 1 && !g_4b165c)
                    pizzaServedTo(1, 1);
                else if (shylerState == 1 && !g_4b165c)
                    pizzaServedTo(2, 2);
            } else {
                reacted = 0;
                if (arnoState == 1 && judgePizza(0) == 1) {
                    trollVerdict(0, 1);
                    reacted++;
                }
                if (willaState == 1 && !reacted && judgePizza(1) == 1) {
                    trollVerdict(1, 1);
                    reacted++;
                }
                if (shylerState == 1 && !reacted && judgePizza(2) == 1) {
                    trollVerdict(2, 1);
                    reacted++;
                }
                if (!reacted)
                    trollVerdict(2, 0);
            }
        }
    } else if (g_4b1600) {
        if (!groupLeader[g_4b1600]) {
            g_4b1600 = g_4b1630 = 0;
            if (g_4b16bc) {
                g_4b16bc = 0;
                g_4b15fc = 1000;
            } else {
                if (arnoState == 3) {
                    if (g_4b1722[0])
                        moveView(arnoView, 0, g_4b1722[0]);
                    pizzaDoneNotify(view, 0);
                }
                if (g_4b1660) {
                    g_4b1660 = 0;
                    g_4b7564 = 1;
                    pizzaDoneNotify(view, 0);
                } else if (arnoState == 2) {
                    deleteView(pizzaView);
                    pizzaView = 0;
                    view = findView(arnoView);
                    setViewScript(view, 8021, 1);
                    if (pizzaPartySize - g_4b15ee > 3)
                        g_4b1820++;
                    moveView(arnoView, 1, g_4b1616);
                    moveView(zoombiniAtPizza, 1, arnoView);
                    arnoState = 3;
                    g_4a3d40 = 0;
                    view->placed = placeArnoToppings;
                    g_4b1600 = groupViews(arnoView, arnoView, 0, 0, 0, 0);
                    g_4b1720++;
                    restartPizzaView();
                    g_4b165c++;
                    trollTurn = 0;
                    g_4b15fe = 0;
                } else {
                if (pizzaSolved) {
                    trollsEat();
                    pizzaGoReady = 1;
                } else if (g_4b1674) {
                    g_4b16ea = 0;
                } else if (willaState == 1 && !g_4b165c) {
                    pizzaServedTo(1, 1);
                } else if (shylerState == 1 && !g_4b165c) {
                    pizzaServedTo(2, 2);
                } else {
                    g_4b165c = 0;
                }
                g_4b16ea = 0;
                }
            }
        }
    } else if (g_4b1602) {
        if (!groupLeader[g_4b1602]) {
            g_4b1602 = g_4b1630 = 0;
            if (willaState == 3) {
                if (g_4b1728[0])
                    moveView(willaView, 0, g_4b1728[0]);
                if (g_4b1728[1])
                    moveView(g_4b1728[1], 1, g_4b1728[0]);
                if (g_4b1728[2])
                    moveView(g_4b1728[2], 1, g_4b1728[1]);
                pizzaDoneNotify(view, 0);
            }
            if (g_4b1660) {
                g_4b1660 = 0;
                g_4b7564 = 1;
                pizzaDoneNotify(view, 0);
            } else if (willaState == 2) {
                deleteView(pizzaView);
                pizzaView = 0;
                view = findView(willaView);
                setViewScript(view, 9027, 1);
                if (pizzaPartySize - g_4b15ee > 3)
                    g_4b1820++;
                moveView(willaView, 1, g_4b1616);
                moveView(zoombiniAtPizza, 1, willaView);
                willaState = 3;
                g_4a3d40 = 1;
                g_4b1602 = groupViews(willaView, willaView, 0, 0, 0, 0);
                view->placed = placeWillaToppings;
                g_4b1720++;
                restartPizzaView();
                g_4b165c++;
                trollTurn = 0;
                g_4b15fe = 0;
            } else {
                if (pizzaSolved) {
                    trollsEat();
                    pizzaGoReady = 1;
                } else if (g_4b1674) {
                    g_4b16ea = 0;
                } else if (shylerState == 1 && !g_4b165c) {
                    pizzaServedTo(2, 2);
                } else {
                    g_4b165c = 0;
                }
                g_4b16ea = 0;
            }
        }
    } else if (g_4b1604) {
        if (!groupLeader[g_4b1604]) {
            g_4b1604 = g_4b1630 = 0;
            if (shylerState == 3) {
                if (g_4b172e[0])
                    moveView(shylerView, 0, g_4b172e[0]);
                pizzaDoneNotify(view, 0);
            }
            if (g_4b1660) {
                g_4b1660 = 0;
                g_4b7564 = 1;
                pizzaDoneNotify(view, 0);
            } else if (shylerState == 2) {
                deleteView(pizzaView);
                pizzaView = 0;
                view = findView(shylerView);
                setViewScript(view, 10031, 1);
                if (pizzaPartySize - g_4b15ee > 3)
                    g_4b1820++;
                moveView(shylerView, 1, g_4b1616);
                shylerState = 3;
                g_4a3d40 = 2;
                view->placed = placeShylerToppings;
                g_4b1604 = groupViews(shylerView, shylerView, 0, 0, 0, 0);
                g_4b1720++;
                restartPizzaView();
                g_4b165c++;
                trollTurn = 0;
                g_4b15fe = 0;
            } else {
                if (pizzaSolved) {
                    trollsEat();
                    pizzaGoReady = 1;
                } else if (g_4b1674) {
                    g_4b16ea = 0;
                } else {
                    g_4b165c = 0;
                }
                g_4b16ea = 0;
            }
        }
    } else if (g_4b1608) {
        if (!groupLeader[g_4b1608]) {
            g_4b1608 = 0;
            stepTrollTurns();
            if (!g_4b165e)
                startLevelTroll();
        }
    } else if (g_4b15fc) {
        if (g_4b15fc == 1000) {
            g_4b15fc = 0;
            trollTurn = g_4b15fe;
            g_4b15fe = 0;
            trollReacts();
        } else if (!groupLeader[g_4b15fc]) {
            g_4b15fc = 0;
            trollTurn = g_4b15fe;
            g_4b15fe = 0;
            trollReacts();
        }
    } else if (g_4b15fe) {
        servePizza();
    } else if (g_4b1606) {
        if (!groupLeader[g_4b1606]) {
            g_4b1606 = 0;
            showJudgedPizza();
        }
    } else if (g_4b160c) {
        if (!groupLeader[g_4b160c]) {
            g_4b160c = 0;
            if (arnoState == 3)
                moveView(arnoView, 0, shownPizzas[0].view);
            if (willaState == 3)
                moveView(willaView, 0, shownPizzas[0].view);
            if (shylerState == 3)
                moveView(shylerView, 0, shownPizzas[0].view);
        }
    } else if (g_4b160a) {
        if (!groupLeader[g_4b160a]) {
            g_4b171e = g_4b160a = 0;
            if (g_4b15da) {
                bringNextZoombini();
                g_4b166e++;
            }
        }
    } else if (g_4b15ec) {
        if (!groupLeader[g_4b15ec]) {
            g_4b15ec = 0;
            if (!g_4b15ee)
                queueViewSound(randomBetween(20055, 20063), 0);
            else
                queueViewSound(randomBetween(20045, 20048), 0);
        }
    }
    if (g_4b15f8) {
        if (!groupLeader[g_4b15f8]) {
            g_4b15f8 = 0;
            view = findView(zoombiniAtPizza);
            if (view) {
                setSnoidAction((Snoid *)&view->body, 7, 0);
                *(Point *)&((Snoid *)&view->body)->targetX = g_4a3d44;
                g_4b15fa = groupViews(zoombiniAtPizza, zoombiniAtPizza, 0, 0, 0, 0);
            }
        }
    } else if (g_4b15fa && !g_4b166e) {
        if (!groupLeader[g_4b15fa]) {
            g_4b15fa = 0;
            setSnoidAction((Snoid *)&findView(zoombiniAtPizza)->body, 2, 0);
            if (g_4b15ee)
                g_4b171e = 0;
        }
    } else {
        g_4b166e = 0;
    }
    if (g_4b1820 && pizzaPartySize - g_4b15ee < 5)
        g_4b1820 = 0;
    if (g_4b1820 && g_4b181e < g_4b181c) {
        if (clockTime() - g_4b1814 > 30) {
            done = 0;
            if (pizzaPartySize - g_4b15ee < 4) {
                g_4b1820 = g_4b171e = 0;
                pizzaDoneNotify(view, 0);
            } else {
                g_4b1814 = clockTime();
                do {
                    n = allocateSlot(&g_4b1818, pizzaPartySize, 0);
                    if (partyViews[n] != zoombiniAtPizza) {
                        waiting = idleSnoidView(partyViews[n]);
                        if (waiting && waiting->body.running && waiting->flags == 1) {
                            script = ((Snoid *)&waiting->body)->features[3] - 1 + 13035;
                            if (script) {
                                startSnoidScript((Snoid *)&waiting->body, script, 0, 0);
                                g_4b181e++;
                                done = 1;
                            }
                        }
                    }
                } while (!done);
            }
        }
    } else if (g_4b181e >= g_4b181c) {
        g_4b181e = g_4b1820 = g_4b1814 = g_4b1818 = 0;
    }
    playAmbientSound();
    g_4a3d9c = 0;
}

/* Opens Pizza Pass: resets the scene's state, sets up the level's
   buttons (from the tables g_4a35b8-g_4a3b34), loads Pizza.MHK's backdrop,
   images, features and scripts, adds the pizza, the topping views and the
   trolls there at the level, the toppings and what each troll wants
   (shareToppings), brings the party in, and says an introduction. */
/* @zoombi32 0x004402c0 */
void openPizza()
{
    short saved;
    short i;

    unloadSounds();
    sceneDue = g_4b15ee = 0;
    g_4b15e4 = pizzaGoReady = 0;
    g_4b1824 = clockTime();
    hintSound = g_4b166e = 0;
    g_4b15f2 = g_4b162e = g_4b1630 = g_4b1632 = 0;
    g_4b15f4 = g_4b15f6 = g_4b15ea = g_4b16bc = 0;
    lastTriedPizza = nextZoombini = -1;
    g_4b164a = g_4b164c = g_4b164e = g_4b1650 = g_4b1652 = 0;
    g_4b166a = g_4b1654 = g_4b1656 = g_4b1658 = g_4b16ea = 0;
    pizzaSolved = g_4b1600 = g_4b1602 = g_4b1604 = 0;
    g_4b16b6 = g_4b16b8 = g_4b16ba = g_4b1606 = 0;
    g_4b16be = g_4b16c0 = g_4b16c2 = g_4b171e = 0;
    g_4b16c4 = g_4b16c6 = g_4b16c8 = g_4b15e8 = 0;
    trollTurn = g_4b1710 = g_4b1660 = g_4b160a = g_4b15ec = 0;
    g_4b165a = g_4b165c = g_4b1608 = g_4b160c = 0;
    g_4b15f8 = g_4b15fa = g_4b15fc = g_4b15fe = 0;
    g_4b1814 = 0;
    g_4b1818 = 0;
    g_4b1820 = g_4b1720 = 0;
    for (i = 0; i < 3; i++)
        g_4b1722[i] = g_4b1728[i] = g_4b172e[i] = 0;
    g_4b15e0 = 0;
    g_4b15d8 = 1;
    g_4b165e = g_4b170a = 1;
    g_4b1712 = g_4b1714 = g_4b1716 = g_4b1718 = g_4b170e = -1;
    fillMemory(shownPizzas, 0, 224);
    pizzaLevel = sceneLevel();
    fillMemory(pizzaToppings, 0, 16);
    fillMemory(mealToppings, 0, 16);
    fillMemory(triedPizzas, 0, 28);
    if (!pizzaLevel)
        memcpy(pizzaButtons, g_4a35b8, sizeof g_4a35b8);
    else if (pizzaLevel == 1)
        memcpy(pizzaButtons, g_4a378c, sizeof g_4a378c);
    else if (pizzaLevel == 2)
        memcpy(pizzaButtons, g_4a3960, sizeof g_4a3960);
    else if (pizzaLevel == 3)
        memcpy(pizzaButtons, g_4a3b34, sizeof g_4a3b34);
    openGameFile(&g_4b15d0, "Pizza.MHK");
    setCurrentMap(g_4b15d0);
    loadPaths(1000);
    loadTerrain(100);
    drawBackdrop(5000);
    g_4a3d94 = loadImageBank(6000, &g_4a3d3c);
    loadFeatureGroup(7000, 0, 0);
    loadFeatureGroup(8000, 1, 0);
    loadFeatureGroup(12000, 2, 0);
    loadScripts(7000, 69);
    addScripts(8000, 36, 0);
    addScripts(12000, 45, 0);
    switch (pizzaLevel) {
    case 1:
        loadFeatureGroup(9000, 3, 0);
        addScripts(9000, 35, 0);
        break;
    case 2:
    case 3:
        loadFeatureGroup(9000, 3, 0);
        loadFeatureGroup(10000, 4, 0);
        addScripts(9000, 35, 0);
        addScripts(10000, 39, 0);
        break;
    }
    g_4a3d38 = addView(0x1000, drawPizzaButtonsView, updatePizzaButtons, 0, 0, 0, 0, 0);
    loadSnoidScripts(14000, 6, 0);
    addSnoidScripts(13000, 40, 0);
    g_4b166c = g_4b755e;
    g_4b755e = 25;
    placedViews[0] = addView(0x108a000, drawCels, runViewScript, 7063, 7, &g_4a3d44, 0, 0);
    g_4b15f0 = addView(0x188000, drawCels, runViewScript, 7000, 6, 0, 0, 0);
    loadViewSounds(g_4b15f0, 0);
    g_4b15f2 = groupViews(g_4b15f0, g_4b15f0, 0, 0, 0, 0);
    g_4b162c = pizzaLevel + 5;
    arnoState = willaState = shylerState = g_4b171a = 0;
    switch (pizzaLevel) {
    case 0:
        toppingViews[0] = addView(0x188000, drawCels, runViewScript, 7005, 6, 0, 0, 0);
        toppingViews[1] = addView(0x188000, drawCels, runViewScript, 7007, 6, 0, 0, 0);
        toppingViews[2] = addView(0x188000, drawCels, runViewScript, 7009, 6, 0, 0, 0);
        toppingViews[3] = addView(0x188000, drawCels, runViewScript, 7011, 6, 0, 0, 0);
        toppingViews[4] = addView(0x188000, drawCels, runViewScript, 7013, 6, 0, 0, 0);
        g_4b1622 = 1;
        toppingCount = 5;
        g_4b1628 = 2;
        g_4b1626 = 500;
        g_4b162a = 0;
        pizzasLeft = 6;
        arnoState = 1;
        break;
    case 1:
        toppingViews[0] = addView(0x188000, drawCels, runViewScript, 7015, 6, 0, 0, 0);
        toppingViews[1] = addView(0x188000, drawCels, runViewScript, 7017, 6, 0, 0, 0);
        toppingViews[2] = addView(0x188000, drawCels, runViewScript, 7019, 6, 0, 0, 0);
        toppingViews[3] = addView(0x188000, drawCels, runViewScript, 7021, 6, 0, 0, 0);
        toppingViews[5] = addView(0x188000, drawCels, runViewScript, 7023, 6, 0, 0, 0);
        toppingViews[6] = addView(0x188000, drawCels, runViewScript, 7025, 6, 0, 0, 0);
        g_4b1622 = 2;
        toppingCount = 7;
        g_4b1626 = 800;
        g_4b1628 = 3;
        g_4b162a = 0;
        pizzasLeft = 7;
        arnoState = 1;
        willaState = 1;
        break;
    case 2:
        toppingViews[0] = addView(0x188000, drawCels, runViewScript, 7027, 6, 0, 0, 0);
        toppingViews[1] = addView(0x188000, drawCels, runViewScript, 7029, 6, 0, 0, 0);
        toppingViews[2] = addView(0x188000, drawCels, runViewScript, 7031, 6, 0, 0, 0);
        toppingViews[3] = addView(0x188000, drawCels, runViewScript, 7033, 6, 0, 0, 0);
        toppingViews[4] = addView(0x188000, drawCels, runViewScript, 7035, 6, 0, 0, 0);
        toppingViews[5] = addView(0x188000, drawCels, runViewScript, 7037, 6, 0, 0, 0);
        toppingViews[6] = addView(0x188000, drawCels, runViewScript, 7039, 6, 0, 0, 0);
        g_4b1622 = 2;
        toppingCount = 7;
        g_4b1626 = 1000;
        g_4b1628 = 3;
        g_4b162a = 1;
        pizzasLeft = 7;
        arnoState = 1;
        willaState = 1;
        shylerState = 1;
        break;
    case 3:
        toppingViews[0] = addView(0x188000, drawCels, runViewScript, 7041, 6, 0, 0, 0);
        toppingViews[1] = addView(0x188000, drawCels, runViewScript, 7043, 6, 0, 0, 0);
        toppingViews[2] = addView(0x188000, drawCels, runViewScript, 7045, 6, 0, 0, 0);
        toppingViews[3] = addView(0x188000, drawCels, runViewScript, 7047, 6, 0, 0, 0);
        toppingViews[4] = addView(0x188000, drawCels, runViewScript, 7049, 6, 0, 0, 0);
        toppingViews[5] = addView(0x188000, drawCels, runViewScript, 7051, 6, 0, 0, 0);
        toppingViews[6] = addView(0x188000, drawCels, runViewScript, 7053, 6, 0, 0, 0);
        toppingViews[7] = addView(0x188000, drawCels, runViewScript, 7055, 6, 0, 0, 0);
        g_4b1622 = 3;
        toppingCount = 8;
        g_4b1626 = 1000;
        g_4b1628 = 4;
        g_4b162a = 2;
        pizzasLeft = 7;
        arnoState = 1;
        willaState = 1;
        shylerState = 1;
        break;
    }
    copyPaletteRange(10, 236);
    saved = soundOn;
    soundOn = 0;
    g_4b1648 = 0;
    g_4b1634 = pizzasLeft;
    shareToppings();
    showMealView();
    setViewPlaces(16, g_4a3d54, 1);
    makePartySnoids(0);
    enterSnoids(200);
    g_4b15dc = listChosenSnoids();
    pizzaPartySize = g_4b15dc->count;
    g_4b181c = 3;
    if (g_4b181c > pizzaPartySize)
        g_4b181c = pizzaPartySize - 1;
    g_4b181e = 0;
    staggerSnoids(45, 30);
    chooseSnoids(0, 0);
    setGroupLists(g_4a3d18, 1, (short)0xc000);
    drawPizzaButton(1, 0, 0);
    drawPizzaButton(2, 0, 0);
    showRect(&shownGameRect);
    addSoundRange(20000, 29999, 1);
    addSoundRange(8024, 8029, 1);
    addSoundRange(15000, 15099, 1);
    addSoundRange(10000, 10032, 1);
    addSoundRange(9000, 9025, 1);
    addSoundRange(8000, 8023, 1);
    addSoundRange(996, 997, 0);
    addSoundRange(12000, 12000, 0);
    addSoundRange(7008, 7009, 0);
    addSoundRange(12001, 12001, 0);
    addSoundRange(475, 499, 0);
    addSoundRange(14000, 14000, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(8030, 8999, 1);
    addSoundRange(9026, 9999, 1);
    addSoundRange(10033, 10099, 1);
    addSoundRange(12002, 12002, 0);
    addSoundRange(7000, 7007, 0);
    addSoundRange(12003, 12099, 0);
    addSoundRange(13000, 13099, 0);
    soundOn = saved;
    if (!pizzaLevel)
        hintSound = 20071;
    else
        hintSound = 20072;
    switch (campHint((short *)(gameState + 0x2e))) {
    case 1:
        sayIntroduction(0);
        break;
    case 5:
        if (!pizzaLevel) {
            sayIntroduction(0);
            break;
        }
    default: {
        short r = randomUpTo(100);

        if (r > 75)
            r = 3;
        else if (r > 50)
            r = 2;
        else if (r > 25)
            r = 1;
        else
            r = 0;
        sayIntroduction(r);
        break;
    }
    }
    if (willaState)
        willaView = addView(0x188000, drawCels, runViewScript, 9034, 6, 0, 0, 0);
    arnoView = addView(0x188000, drawCels, runViewScript, 8032, 6, 0, 0, 0);
    if (shylerState)
        shylerView = addView(0x188000, drawCels, runViewScript, 10038, 6, 0, 0, 0);
    g_4b1616 = addView(0x4108000, drawCels, runViewScript, 8033, 6, 0, 0, 0);
    saved = soundOn;
    soundOn = 0;
    updateViews();
    fadeInViews();
    g_4b15e4 = 1;
    g_4b171e = 1;
    soundOn = saved;
    stepTrollTurns();
    setViewsLocked(0);
    placeClaims[0] = 1;
}
