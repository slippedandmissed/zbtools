/*
 * ferry (0x41f8cc-0x42160c): Captain Cajun's ferry (scene 10), 'Ferry.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "e2memory.h"
#include "features.h"
#include "ferry.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Draws button `which` (1 or 2; 2 is dim unless g_4abaae), lit or not,
   and shows it if asked. */
/* @zoombi32 0x0041fdee */
void drawFerryButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4abaae) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a147c->offsets[image] + (char *)g_4a147c), ferryButtons[which - 1].rect.left,
                      ferryButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&ferryButtons[which - 1].rect);
    }
}

/* The buttons' view update: redraws button 2 as g_4abaae changes, and
   button 1 once. */
/* @zoombi32 0x0041fea4 */
void fn_41fea4(View *, short region)
{
    if (g_4abaae) {
        if (!g_4a1570) {
            g_4a1570 = 1;
            unionRgnRect(region, &ferryButtons[1].rect);
        }
    } else if (g_4a1570) {
        g_4a1570 = 0;
        unionRgnRect(region, &ferryButtons[1].rect);
    }
    if (!g_4a1572) {
        g_4a1572 = 1;
        unionRgnRect(region, &ferryButtons[0].rect);
    }
}

/* Closes scene 10. */
/* @zoombi32 0x0041ff16 */
void closeScene10()
{
    if (g_4abaac) {
        g_4abaac = g_4abaa2 = g_4abaa4 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        fn_46c602(&g_4a151c);
        unloadSounds();
        if (g_4abaf8) {
            disposePtr(g_4abaf8);
            g_4abaf8 = 0;
        }
        fn_46bee9(saved);
        fn_46ca9c(&g_4abaa8);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Moves g_4abac0 to g_4abac2 and starts its view on g_4a1440's script
   for g_4abaee. */
/* @zoombi32 0x004209b8 */
void fn_4209b8()
{
    View *view;

    g_4abac2 = g_4abac0;
    g_4abac0 = 0;
    view = findView(g_4abac2);
    if (view) {
        view->flags &= ~0x800000;
        setViewScript(view, g_4a1440[g_4abaee], 1);
        view->flags |= 0x800000;
    }
}

/* Starts g_4abaf2's Zoombini on `script` (anchored at g_4aba98) in
   `group`, with `notify` if given. */
/* @zoombi32 0x00420a08 */
void fn_420a08(short group, short script, ViewNotify notify, char unknownF8)
{
    View *view = findView(g_4abaf2);

    if (view) {
        startSnoidScript(viewSnoid(view), script, g_4aba98, unknownF8);
        view->body.group = group;
        loadViewSounds(g_4abaf2, 0);
        if (notify)
            view->notify = notify;
    }
}

/* A notify: 6 starts g_4ababe's script in this view's group; other events
   turn the Zoombini (turnSnoid). */
/* @zoombi32 0x00420c82 */
void fn_420c82(View *view, short event)
{
    View *other;

    switch (event) {
    default:
        turnSnoid(view, event);
        break;
    case 6:
        other = findView(g_4ababe);
        if (other) {
            setViewScript(other, 0, 1);
            other->body.group = view->body.group;
        }
        break;
    }
}

/* Moves the placed Zoombinis (flag 1 and unknownF7) and two kinds of
   view (flags 0x748c2000, 0x74980000) `dx` along, cels and all. */
/* @zoombi32 0x0042113f */
void fn_42113f(View *, short dx)
{
    View *view;
    ViewCel *cel;

    for (view = viewListEnd(1)->next; view; view = view->next)
        if (view->flags == 1 && viewSnoid(view)->unknownF7 || view->flags == 0x748c2000 || view->flags == 0x74980000) {
            view->body.group = 0;
            view->body.x += dx;
            for (cel = view->body.cels; cel->image; cel++)
                cel->x += dx;
        }
}

/* Resets scene 10's state. */
/* @zoombi32 0x0041f8cc */
void resetScene10()
{
    g_4aba88 = 0;
    g_4b966e = 0;
    g_4abb10 = g_4abaf4 = 0;
    g_4abb08 = g_4abb0a = 0;
    g_4abb0c = 1;
    g_4aba90 = 0;
    g_4abaee = g_4abaf0 = g_4abaf2 = g_4abaa6 = 0;
    g_4aba98 = 0;
    g_4abab0 = g_4abaa2 = g_4abaa4 = 0;
    g_4abb06 = g_4abb04 = g_4abaa0 = g_4b0d52 = 0;
    g_4abab6 = g_4abab8 = g_4ababa = g_4ababc = g_4ababe = 0;
}

/* Finds a free waiting place (of ferryPlaces, noting which Zoombini is
   nearest each in sortedIds) into *spot: an even one (from either end at
   random), else an odd one, else 0. */
/* @zoombi32 0x004214ab */
void findFerryPlace(short *spot)
{
    Point none = {0, 0};
    short i;
    short found;
    short skip;
    short j;

    spotTaken(&none, 0, 500);
    for (i = 0; i < 20; i++) {
        skip = 0;
        found = spotNear(&ferryPlaces[i], 500, skip);
        for (j = 0; found && j < i; j++)
            if (found == sortedIds[j]) {
                skip++;
                found = spotNear(&ferryPlaces[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = found;
    }
    found = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 18; found == -1 && i >= 0; i -= 2)
            if (!sortedIds[i])
                found = i;
    } else {
        for (i = 0; found == -1 && i <= 18; i += 2)
            if (!sortedIds[i])
                found = i;
    }
    if (found == -1) {
        if (randomBetween(1, 100) <= 50) {
            for (i = 19; found == -1 && i >= 1; i -= 2)
                if (!sortedIds[i])
                    found = i;
        } else {
            for (i = 1; found == -1 && i <= 19; i += 2)
                if (!sortedIds[i])
                    found = i;
        }
    }
    if (found == -1)
        found = 0;
    *spot = found;
}

/* Starts the next Zoombini (g_4abaf0) on its way by `n` (g_4abaee; 0 or
   over 9: 0): Captain Cajun's view (g_4abab4) plays 1604-1607, and for 7-9
   the views g_4ababc and g_4ababe are made anew about g_4aba8c. */
/* @zoombi32 0x00420f85 */
void fn_420f85(short n)
{
    View *view;
    Point at;

    g_4abaee = n;
    if (!g_4abaf0)
        return;
    g_4abaf2 = g_4abaf0;
    g_4abaf0 = 0;
    if (n >= 10)
        n = 0;
    view = findView(g_4abab4);
    if (!view)
        return;
    switch (n) {
    case 0:
        g_4abb16 = 1605;
        g_4aba92.x = 122;
        g_4aba92.y = 164;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        if (randomBetween(1, 100) <= 50)
            g_4abb16 = 1604;
        else
            g_4abb16 = 1606;
        g_4aba92 = g_4aba8c;
        break;
    case 7:
    case 8:
    case 9:
        g_4abb16 = 1607;
        deleteView(g_4ababc);
        deleteView(g_4ababe);
        g_4ababc = addView(0x1180000, drawCels, runViewScript, 1705, 6, 0, 0, -1);
        g_4aba92.x = 236;
        g_4aba92.y = 474;
        g_4aba9c = g_4aba8c;
        at.x = g_4aba9c.x - 14;
        at.y = g_4aba9c.y - 14;
        g_4ababe = addView(0x1980000, drawCels, runViewScript, 1706, 6, &at, 0, -1);
        break;
    }
    setViewScript(view, g_4abb16, 1);
    loadViewSounds(g_4abab4, 1);
    g_4aba98 = 0;
    view->notify = fn_420a60;
    groupViews(view->id, g_4abac0, g_4abaf2, 0, 0, 0);
    setViewsLocked(0);
}
