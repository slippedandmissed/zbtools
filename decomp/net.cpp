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
