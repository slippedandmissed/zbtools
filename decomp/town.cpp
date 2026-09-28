/*
 * town (0x45c0f4-0x45e2d8): the town (scene 0), 'Town.MHK'
 */

#include <stdlib.h>
#include <string.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

/* Opens scene 0. */
/* @zoombi32 0x0045c12e */
void openScene0()
{
    g_4b7cf4 = g_4b7cf8 = g_4b7cf6 = 0;
    g_4b0d52 = g_4b7cec = 0;
    g_4b7cf0 = 1;
    setGroupLists(townGroups, 1, (short)0xc000);
    g_4b7cf4 = 1;
}

/* Scene 0's clicks: moves g_4a7410 on from 1 to 2 (always, with
   g_4b7cf8); 1 or -1 goes back to the scene fn_454c10 picks. */
/* @zoombi32 0x0045c391 */
void scene0Clicked(short which)
{
    if (g_4b7cf8)
        g_4a7410 = 2;
    if (which > 0 && g_4a7410 == 1)
        g_4a7410 = 2;
    if (abs(which) == 1) {
        g_4b7cf0 = g_4b7cf6 = 0;
        g_4b0d52 = fn_454c10();
    }
}

/* Every 1800 ticks, reads the time for the clock: its minute hand
   (0-11, in fives) and hour hand (0-11). */
/* @zoombi32 0x0045c4c9 */
void fn_45c4c9()
{
    unsigned long now;
    short year;
    char ignored;

    now = clockTime();
    if (now > g_4b7efc + 1800) {
        g_4b7efc = now;
        getDateTime(&year, &ignored, &ignored, (char *)&clockHour, (char *)&clockMinute);
        clockMinute = clockMinute / 5;
        clockHour = clockHour % 12;
    }
}

/* Sets whether the views g_4b7ece and the first g_4b7f02 party views run
   their scripts. */
/* @zoombi32 0x0045ccca */
void fn_45ccca(short running)
{
    short i;
    View *view;

    for (i = 0; i <= 19; i++) {
        view = findView(g_4b7ece[i]);
        if (view)
            view->body.running = running;
    }
    for (i = 0; i < g_4b7f02; i++) {
        view = findView(partyViews[i]);
        if (view)
            view->body.running = running;
    }
}

/* A view update: once reset, adds the button to `region`. */
/* @zoombi32 0x0045cf8b */
void fn_45cf8b(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &townButtons[0].rect);
    }
}

/* A script from 3000-3002 by g_4a4ba0's +0x46. */
/* @zoombi32 0x0045d04c */
short fn_45d04c()
{
    short script;

    script = ((*(short *)(g_4a4ba0 + 0x46) - 1) & 0xfff) % 3 + 3000;
    if (script < 3000)
        script = 3000;
    if (script >= 3003)
        script = 3002;
    return script;
}

/* A view's placed callback: drops its cels whose image is past g_4b7e10. */
/* @zoombi32 0x0045daf7 */
void fn_45daf7(View *view)
{
    ViewCel *cel;

    for (cel = view->body.cels; cel->image;)
        if (cel->image > g_4b7e10)
            removeFirstCel(cel);
        else
            cel++;
}

/* A notify: negates the view's entry in g_4b7ece (the first 19) and counts
   it in g_4b7f10. */
/* @zoombi32 0x0045e29e */
void fn_45e29e(View *view, short)
{
    short id = view->id;
    short i;

    for (i = 0; i < 19; i++)
        if (id == g_4b7ece[i]) {
            g_4b7ece[i] = -g_4b7ece[i];
            g_4b7f10++;
            break;
        }
}

/* Closes scene 0, leaving its palette and clip for the next scene, and
   reloads the Zoombinis and dialogs. */
/* @zoombi32 0x0045c175 */
void closeScene0()
{
    if (g_4b7cf4) {
        g_4b7cf4 = 0;
        short saved = fn_46bee9(1);

        fn_455273(1);
        viewsShown = 1;
        setTakeStatic(1);
        realizePalette(getPortPalette(), 1);
        setClipRect(gameRect);
        discardEvents(3);
        fn_46bee9(saved);
        fadeOutViews();
        fn_4624fc();
        loadSnoids(0);
        loadDialogs();
        g_4b2aea = 1;
        showCursor();
    }
}

/* Moves every view with flag 2 a screen (320) left or right, wrapping
   round the town's 1920 pixels; a Zoombini's anchor (unknownAa) keeps its
   place relative to it. */
/* @zoombi32 0x0045ce80 */
void scrollTown(short left)
{
    View *view;
    Snoid *snoid;
    short x;
    short old;

    for (view = viewListEnd(1); view; view = view->next)
        if (view->flags & 2) {
            snoid = viewSnoid(view);
            x = snoid->body.x;
            old = x;
            if (left) {
                x -= 320;
                if (x < -320)
                    x += 1920;
            } else {
                x += 320;
                if (x > 1599)
                    x -= 1920;
            }
            snoid->body.x = x;
            if (snoid->features[0])
                snoid->body.unknownAa += old - x;
            snoid->unknownF5 = -1;
            view->nextUpdate = 0;
        }
}

/* Draws button `which` (only 1), lit or not, and shows it if asked. */
/* @zoombi32 0x0045ceff */
void drawTownButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a74c8->offsets[image] + (char *)g_4a74c8), townButtons[which - 1].rect.left,
                      townButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&townButtons[which - 1].rect);
    }
}

/* Closes scene 6. */
/* @zoombi32 0x0045cfae */
void closeScene6()
{
    if (g_4b7e00) {
        g_4b7e00 = 0;
        short saved = fn_46bee9(1);

        useAltSnoids(1);
        if (!viewsLocked) {
            viewsLocked = 1;
            *(short *)(g_4a4ba0 + 0xa92e) = 0;
            *(short *)(g_4a4ba0 + 0xa930) = 1;
            *(short *)(g_4a4ba0 + 0xa932) = 1;
        }
        clearViews();
        unloadSounds();
        fn_46c602(&g_4a74c4);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b7dfc);
        fadeOutViews();
        fn_4624fc();
        g_4a74dc = -1;
    }
}

/* Finds the hotspot under the cursor (into *where): the first of the
   g_4b7eb2 non-empty rectangles g_4b7e12 holding it sets g_4b7eb4, its
   number (g_4b7e92, from 1) in g_4b7eb6 and a script by it in g_4b7eb8. */
/* @zoombi32 0x0045d715 */
void fn_45d715(Point *where)
{
    short i;

    getCursorPosition(where);
    g_4b7eb4 = 0;
    for (i = 0; !g_4b7eb4 && i < g_4b7eb2; i++)
        if (!emptyRect(&g_4b7e12[i]) && ptInRect(&g_4b7e12[i], *where)) {
            i = g_4b7e92[i];
            g_4b7eb8 = g_4a7582[i] + 1003;
            g_4b7eb6 = i + 1;
            g_4b7eb4 = 1;
        }
}

/* Scene 0's keys: Ctrl-Q quits; any other key clicks. */
/* @zoombi32 0x0045c0f4 */
short scene0Key(unsigned short key)
{
    switch (key) {
    case 0x1b:
    case ' ':
    default:
        scene0Clicked(1);
        return 1;
    case 0x11:
        closeScene0();
        g_4b80e0 = -1;
        return 1;
    }
}

/* Resets scene 6's state; the pace g_4b7f08 by g_4b2b00. */
/* @zoombi32 0x0045c3ec */
void resetScene6()
{
    short i;

    g_4b7eba = 0;
    for (i = 0; i < 4; i++)
        g_4b7e08[i] = 0;
    g_4b7f00 = g_4b7f02 = g_4b7f10 = 0;
    g_4b7f04 = 0;
    g_4b7f0c = 0;
    if (g_4b2b00)
        g_4b7f08 = 600;
    else
        g_4b7f08 = 120;
    g_4b7ec8 = g_4b7eca = g_4b7ecc = 0;
    g_4b7eb6 = g_4b7eb8 = g_4b7ec4 = g_4b7ec6 = 0;
    g_4a74dc = -1;
    g_4b7ef8 = 0;
    g_4b7ec0 = 0;
    for (i = 0; i <= 19; i++)
        g_4b7ece[i] = 0;
    g_4b7efc = 0;
    fn_45c4c9();
    g_4b7ef6 = 0;
    g_4b7f12 = 0;
}

/* A view draw: draws the button, unlit. */
/* @zoombi32 0x0045cf79 */
void drawTownButtons(View *)
{
    drawTownButton(1, 0, 0);
}

/* Moves the Zoombinis who arrived (the travellers, fn_4572bf of them) into
   the town's free slots, up to 625. */
/* @zoombi32 0x0045dfb1 */
void settleTravellers()
{
    short arrived;
    short found;
    short i;
    short j;

    arrived = fn_4572bf();
    found = 0;
    for (i = 624; !found && i >= 0; i--)
        if (townSlots->slots[i].zoombini)
            found = 1;
    if (found)
        ;
    for (j = 0, i = 0; j < arrived && i < 625 && townSlots->count < 625; i++)
        if (!townSlots->slots[i].zoombini) {
            townSlots->count++;
            townSlots->slots[i].zoombini = travellers()[j].zoombini;
            strcpy(townSlots->slots[i].name, travellers()[j].name);
            j++;
        }
}

/* The clock's view (two cels, its hands): hidden while g_4a74dc is set
   and unless g_4b7ef8 and g_4a4ba0's +0x1e is 1 or 2. Shows the time
   (fn_45c4c9), or with g_4b7ef6 winds the hands round that many times
   (faster, from where they are). */
/* @zoombi32 0x0045cd27 */
void drawClock(View *view)
{
    ViewCel *cels = view->body.cels;

    cels[0].image = 0;
    if (g_4a74dc > 0) {
        view->changed = 0;
        return;
    }
    if (g_4b7ef8 <= 0)
        return;
    if (*(short *)(g_4a4ba0 + 0x1e) != 1 && *(short *)(g_4a4ba0 + 0x1e) != 2)
        return;
    if (g_4b7ef6) {
        if (g_4b7ef6 < 0) {
            g_4b7f14 = clockMinute;
            g_4b7f15 = clockHour;
            g_4b7ef6 = abs(g_4b7ef6);
            view->interval = 2;
        } else {
            clockMinute++;
            if (clockMinute > 11) {
                clockMinute = 0;
                clockHour++;
                if (clockHour > 11)
                    clockHour = 0;
            }
            if (g_4b7f14 == clockMinute && g_4b7f15 == clockHour) {
                g_4b7ef6--;
                if (!g_4b7ef6) {
                    view->interval = 6;
                    g_4b7efc = 0;
                    fn_45c4c9();
                }
            }
        }
    } else {
        fn_45c4c9();
    }
    cels[0].image = clockMinute + 1;
    cels[1].image = clockHour + 13;
    cels[0].y = cels[1].y = 218;
    if (*(short *)(g_4a4ba0 + 0x1e) == 1)
        cels[0].x = cels[1].x = 626;
    else
        cels[0].x = cels[1].x = 307;
}

/* An image's data in a bank. */
inline unsigned short *bankImage(ImageBank *bank, short image)
{
    return (unsigned short *)((char *)bank + bank->offsets[image]);
}

/* A view's placed callback: its cels 7-22 are the groups' records (by
   recordGroups): those set become hotspots (g_4b7e12, 56 by 28 about
   g_4a74de's point for the group plus the cel's; 4 also sets g_4b7ef8),
   the others are dropped. */
/* @zoombi32 0x0045db25 */
void fn_45db25(View *view)
{
    ShortRect rect;
    ImageBank *volatile bank = groupBanks[view->body.scriptGroup];
    short *cel = (short *)view->body.cels;
    short n;

    g_4b7eb2 = g_4b7ef8 = 0;
    while (*cel)
        if (*cel >= 7 && *cel <= 22) {
            n = *cel - 7;
            if (recordGroups()[n]) {
                g_4b7e92[g_4b7eb2] = n;
                if (n == 4)
                    g_4b7ef8 = 1;
                bankImage(bank, *cel); /* unused, as in the original */
                cel++;
                rect.left = g_4a74de[n].x + *cel - 28;
                cel++;
                rect.right = rect.left + 56;
                rect.top = g_4a74de[n].y + *cel - 14;
                cel++;
                rect.bottom = rect.top + 28;
                g_4b7e12[g_4b7eb2] = rect;
            } else {
                g_4b7e12[g_4b7eb2] = noRect;
                g_4b7e92[g_4b7eb2] = -1;
                removeFirstCel((ViewCel *)cel);
            }
            g_4b7eb2++;
        } else {
            cel += 3;
        }
}

/* Scene 0's frame: plays the logo movie (Data\Logo025.MOV) once, then
   clicks; leaves when asked (g_4b0d52). */
/* @zoombi32 0x0045c212 */
void scene0Frame()
{
    if (g_4a7412 || !g_4b7cf4)
        return;
    g_4a7412 = 1;
    if (g_4b2ad4 && !g_4b7cf8) {
        if (fn_455229() == 1)
            g_4b7cf0 = 1;
    } else if (g_4b7cf8 || !g_4b2ad4 && g_4b7cf6) {
        g_4b7cf6 = 0;
        g_4b0d52 = fn_454c10();
    }
    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        closeScene0();
    } else if (g_4b7cf0) {
        switch (g_4b7cec) {
        case 0:
            g_4b7cf0 = 0;
            g_4b7cec++;
            if (!g_4b2ad4) {
                g_4a7410 = 1;
                logoPath[0] = 0;
                strcpy(logoPath, installDir);
                strcat(logoPath, "Data\\");
                strcat(logoPath, "Logo025.MOV");
                if (fn_45537f(logoPath)) {
                    scene0Clicked(1);
                } else {
                    g_4b7cf6 = 1;
                    hideCursor();
                }
            }
            break;
        case 1:
            g_4b2ad6 = 0;
            g_4b7cec++;
            scene0Clicked(-1);
            break;
        }
    }
    g_4a7412 = 0;
}
