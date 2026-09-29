/*
 * xfer (0x4696f0-0x46be28): 'xfer.MHK'
 */

#include <stdio.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"
#include "xfer.h"

/* Resets scene 2's state. */
/* @zoombi32 0x004696f0 */
void resetScene2()
{
    short i;

    g_4b0d52 = g_4b9914 = 0;
    g_4b9904 = g_4b9906 = g_4b9908 = g_4b990a = 0;
    g_4b9922 = g_4b9924 = g_4b9926 = g_4b9920 = g_4b9928 = 0;
    g_4b9912 = 0;
    g_4b992c = 0;
    for (i = 0; i < 3; i++)
        g_4b990c[i] = 1;
    for (i = 0; i < 4; i++)
        g_4b98f8[i] = 0;
    for (i = 0; i < 2; i++)
        g_4b9900[i] = 0;
    for (i = 0; i < 17; i++)
        g_4b98e4[i] = 0;
    g_4b98f6 = g_4b991c = g_4b991c = 0;
    g_4a4b98 = 0;
    g_4b9916 = 0;
    g_4b98da = population();
    g_4b98dc = -1;
    g_4b9918 = g_4b991a = 0;
    g_4b98e0 = 0;
}

/* @zoombi32 0x0046b07b */
long scene2Key(long)
{
    return 0;
}

/* Fills `levels` (17) with the highest level (1-4) each place has
   reached, by the game state's bits (or all g_4b754a); then notes the
   place of scene g_4b0d54 in g_4b991c, and its level in g_4b991e. */
/* @zoombi32 0x0046b084 */
void fn_46b084(char *levels)
{
    short i;
    short bits;
    short value;
    short saved;

    for (i = 0; i <= 16; i++) {
        if (g_4b754a)
            value = g_4b754a;
        else {
            value = bits = 0;
            switch (i) {
            case 0:
                bits = 1;
                break;
            case 1:
            case 2:
            case 3:
                bits = g_4a4ba0[0x55 + i] & 0xf;
                break;
            case 4:
                bits = g_4a4ba0[0x50] & 0xf;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                bits = g_4a4ba0[0x54 + i] & 0xf;
                break;
            case 11:
                bits = *(short *)(g_4a4ba0 + 0x52) & 0xf;
                break;
            case 12:
            case 13:
            case 14:
                bits = g_4a4ba0[0x53 + i] & 0xf;
                break;
            case 15:
                bits = g_4a4ba0[0x51] & 0xf;
                break;
            case 16:
                bits = *(short *)(g_4a4ba0 + 0x52) & 0xf0;
                bits = bits >> 4;
                break;
            }
            if (bits & 1)
                value = 1;
            if (bits & 2)
                value = 2;
            if (bits & 4)
                value = 3;
            if (bits & 8)
                value = 4;
        }
        levels[i] = value;
    }
    g_4b991c = i = 0;
    saved = currentScene;
    currentScene = g_4b0d54;
    value = sceneLevel() + 1;
    currentScene = saved;
    switch (g_4b0d54) {
    case 7:
        i = 1;
        bits = value;
        break;
    case 8:
        i = 2;
        bits = levels[1];
        break;
    case 9:
        i = 3;
        bits = levels[2];
        break;
    case 4:
        i = 4;
        bits = levels[3];
        break;
    case 10:
        i = 5;
        bits = value;
        break;
    case 11:
        i = 6;
        bits = levels[5];
        break;
    case 12:
        i = 7;
        bits = levels[6];
        break;
    case 5:
        if (g_4b0d56 == 12) {
            i = 11;
            bits = levels[7];
        } else {
            i = 16;
            bits = levels[10];
        }
        break;
    case 13:
        i = 8;
        bits = value;
        break;
    case 14:
        i = 9;
        bits = levels[8];
        break;
    case 15:
        i = 10;
        bits = levels[9];
        break;
    case 16:
        i = 12;
        bits = value;
        break;
    case 17:
        i = 13;
        bits = levels[12];
        break;
    case 18:
        i = 14;
        bits = levels[13];
        break;
    case 6:
        i = 15;
        bits = levels[14];
        break;
    }
    if (i) {
        g_4b991c = i;
        g_4b991e = bits;
        levels[i] = bits - 1;
        if (levels[i] < 1) {
            if (g_4b98dc < 0)
                g_4b98dc = bits - 1;
            levels[i] = -1;
        }
    }
}

/* The map view's placed callback (map g_4b9916, 1-4): picks each cel's
   image by the places' levels (g_4b98e4): the first ones show the places
   reached, the rest each place's level. Notes the image of place g_4b991c
   in g_4b9928 (1-4). */
/* @zoombi32 0x0046b326 */
void fn_46b326(View *view)
{
    short images[10];
    short last;
    short base;
    short found = 0;
    short count;
    short i;
    short place;
    ViewCel *cel;

    switch (g_4b9916) {
    case 1:
        base = 0;
        count = 5;
        last = 8;
        break;
    case 2:
        base = 5;
        count = 6;
        last = 9;
        break;
    case 3:
        base = 10;
        count = 6;
        last = 9;
        break;
    case 4:
        base = 15;
        count = 6;
        last = 9;
        break;
    default:
        return;
    }
    images[0] = 0;
    for (i = 1; i <= last; i++) {
        images[i] = 0;
        if (i >= count) {
            place = g_4a7ee0[0][base + i - count + 1];
            if (g_4b991c && place == g_4b991c) {
                g_4b991c = 0;
                found = i;
                switch (g_4b9916) {
                case 1:
                    switch (found) {
                    case 5:
                        g_4b9928 = 1;
                        break;
                    case 6:
                        g_4b9928 = 2;
                        break;
                    case 7:
                        g_4b9928 = 3;
                        break;
                    case 8:
                        g_4b9928 = 4;
                        break;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                    switch (found) {
                    case 6:
                        g_4b9928 = 1;
                        break;
                    case 7:
                        g_4b9928 = 2;
                        break;
                    case 8:
                        g_4b9928 = 3;
                        break;
                    case 9:
                        g_4b9928 = 4;
                        break;
                    }
                    break;
                }
            }
            place = g_4b98e4[place];
            if (place > 0)
                images[i] = i + place * 4;
            else {
                if (g_4b98dc < 0)
                    g_4b98dc = 0;
                if (i == count)
                    images[i] = g_4b98dc * 4 + i;
                else if (i > count && place == -1 && images[i - 1] > last)
                    images[i] = g_4b98dc * 4 + i;
            }
        } else if (!base) {
            if (g_4b98e4[i])
                images[i] = i;
        } else {
            place = g_4a7ee0[0][base + i - 1];
            if (g_4b98e4[place])
                images[i] = i;
            else if (place == 11) {
                if (g_4b98e4[16])
                    images[i] = i;
            } else if (place == 16) {
                if (g_4b98e4[11])
                    images[i] = i;
            }
        }
    }
    cel = view->body.cels;
    while (cel->image) {
        if (found && found == cel->image)
            found = 0;
        if (images[cel->image]) {
            cel->image = images[cel->image];
            cel++;
        } else
            removeFirstCel(cel);
    }
}

/* A Zoombini's view's script events: 250-253 face it that way; 240-243
   note a way to face (g_4b9904, then 1-4) when it next turns round (0);
   26 faces it left and moves it after g_4b9906; 10-11 start the view of
   g_4b9900 for g_4b990c; 50 counts one more in town and starts
   g_4b9926's view. */
/* @zoombi32 0x0046b5ce */
void fn_46b5ce(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);
    View *started;

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
        g_4b9904 = event - 239;
        break;
    case 26:
        setSnoidFacing(snoid, 0);
        moveView(view->id, 0, g_4b9906);
        if (g_4b9908 >= 0)
            g_4b9908++;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4b9904) {
            setSnoidFacing(snoid, g_4b9904 - 1);
            g_4b9904 = 0;
        }
        snoid->unknownF0++;
        if (!g_4b9916 && snoid->unknownF0 == 2)
            moveView(view->id, 1, g_4b9906);
        break;
    case 10:
    case 11:
        if (g_4b990c[event - 10]) {
            g_4b990c[event - 10] = 0;
            started = findView(g_4b9900[event - 10]);
            if (started) {
                started->flags = 0x188000;
                setViewScript(started, 0, 1);
            }
        }
        break;
    case 50:
        g_4b98da++;
        startView(g_4b9926, 0, 0, 0);
        break;
    case -1:
        break;
    }
}

/* Draws the population sign's view while it runs (then stops it): its
   cels and "zoombiniville population N", straight to the screen. */
/* @zoombi32 0x0046b761 */
void fn_46b761(View *view)
{
    Color saved;
    ShortRect rect;
    char text[32];

    if (view->body.running) {
        drawCels(view);
        saved = setForeColor(Color(45));
        rect = view->body.bounds;
        rect.left += 16;
        rect.top += 8;
        sprintf(text, "%s%d", levelTexts[10], g_4b98da);
        drawOutlinedText(0x70, 0xd1, rect, 1, text);
        copyPortBits(viewPort, workPort, view->body.bounds, view->body.bounds, 0);
        view->body.running = 0;
        setForeColor(saved);
    }
}

/* Scene 2's frame: leaves for the scene due; else goes on to scene
   g_4b0d54 after 300 ticks (and sound g_4b9914), and now and then starts
   something moving: the next of the party (with fn_46b5ce), one of the
   views g_4b98f8 or g_4b98f6, or g_4b990a's once g_4b9908 passes 4. */
/* @zoombi32 0x0046ace4 */
void scene2Frame()
{
    View *view;
    Snoid *snoid;

    if (g_4a7ede || !g_4b98d8)
        return;
    g_4a7ede = 1;
    updateViews();
    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        if (g_4a7e68) {
            g_4a7e68 = 0;
            g_4b0d50 = 1;
        }
        fn_46be2e(0);
        closeScene2();
        g_4a7ede = 0;
        return;
    }
    if (!g_4b9684) {
        if (g_4b87fe && g_4b9914) {
            if (!isSoundPlaying(g_4b9914, RESOURCE_TYPE(0, 'S', 'N', 'D')) && viewClock() > 300)
                g_4b0d52 = g_4b0d54;
        } else if (viewClock() > 300)
            g_4b0d52 = g_4b0d54;
        if (!g_4b0d52 && clockTime() > g_4b98e0) {
            if (g_4b9908 > 4) {
                g_4b9908 = -1;
                view = findView(g_4b990a);
                if (view) {
                    setViewScript(view, 0, 1);
                    view->notify = fn_46b747;
                }
            }
            if (!g_4b9916) {
                g_4b98e0 = randomBetween(3, 6) * 30 + clockTime();
                if (randomBetween(1, 100) > 40 || !g_4b9912) {
                    g_4b9912 = 1;
                    if (g_4b9918 < g_4b991a) {
                        view = findView(partyViews[g_4b9918]);
                        if (view) {
                            snoid = viewSnoid(view);
                            snoid->unknownF2 = 0;
                            snoid->unknownF0 = 0;
                            startSnoidScript(snoid, snoid->features[3] + 5199, 0, 1);
                            view->notify = fn_46b5ce;
                            view->notifyEnd = 1;
                        }
                        g_4b9918++;
                    }
                } else {
                    short n = randomBetween(0, 4);

                    switch (n) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        view = findView(g_4b98f8[n]);
                        if (view && !view->body.running)
                            setViewScript(view, 0, 1);
                        break;
                    case 4:
                        if (g_4b9910) {
                            g_4b9910 = 0;
                            view = findView(g_4b98f6);
                            if (view && !view->body.running) {
                                view->flags = 0x188000;
                                setViewScript(view, 0, 1);
                            }
                        }
                        break;
                    }
                }
            } else if (g_4b9916 == 5) {
                g_4b98e0 = randomBetween(3, 6) * 40 + clockTime();
                if (g_4b9918 < g_4b991a) {
                    view = findView(partyViews[g_4b9918]);
                    if (view) {
                        snoid = viewSnoid(view);
                        snoid->unknownF2 = 0;
                        startSnoidScript(snoid, snoid->features[3] + 6199, 0, 1);
                        view->notify = fn_46b5ce;
                        view->notifyEnd = 1;
                    }
                    g_4b9918++;
                }
            }
        }
    }
    g_4a7ede = 0;
}

/* Scene 2's clicks: once a scene is due (g_4b0d52), leaves for it (for
   scene 1 if g_4a7e68); 1 goes on to scene g_4b0d54. */
/* @zoombi32 0x0046b00e */
void scene2Clicked(short which)
{
    if (g_4b98d8) {
        if (g_4b0d52) {
            g_4b0d50 = g_4b0d52;
            g_4b0d52 = 0;
            if (g_4a7e68) {
                g_4a7e68 = 0;
                g_4b0d50 = 1;
            }
            fn_46be2e(0);
            closeScene2();
        } else
            switch (which) {
            case 1:
                g_4b0d52 = g_4b0d54;
                break;
            }
    }
}

/* Spreads the marks in g_4b9944 over the grid (fn_46bb0c on each one's
   neighbours) until `permille` thousandths of the cells counted by
   fn_46b872 are taken, or a pass takes none; returns how many are left.
   Only the right and bottom edges are checked. */
/* @zoombi32 0x0046b9a2 */
unsigned long fn_46b9a2(long permille)
{
    char *cell;
    char *next;
    unsigned long before;
    unsigned long limit;
    unsigned long i;
    unsigned long x;
    unsigned long y;

    limit = g_4b9930 - g_4b9930 * permille / 1000;
    while (g_4b9934 > limit) {
        before = g_4b9934;
        for (i = 0; i < 24; i++)
            if (g_4b99a4[i]) {
                g_4b99a4[i] = 0;
                x = g_4b9944[i].x;
                y = g_4b9944[i].y;
                cell = y * g_4b9938 + x + g_4b99bc;
                if (y + 1 < g_4b993c) {
                    next = cell + g_4b9938;
                    fn_46bb0c(next, x, y + 1);
                    fn_46bb0c(next - 1, x - 1, y + 1);
                    if (x + 1 < g_4b9940)
                        fn_46bb0c(next + 1, x + 1, y + 1);
                }
                if (y - 1 > 0) { /* unsigned: true for row 0 too */
                    next = cell - g_4b9938;
                    fn_46bb0c(next, x, y - 1);
                    fn_46bb0c(next - 1, x - 1, y - 1);
                    if (x + 1 < g_4b9940)
                        fn_46bb0c(next + 1, x + 1, y - 1);
                }
                fn_46bb0c(cell - 1, x - 1, y);
                if (x + 1 < g_4b9940)
                    fn_46bb0c(cell + 1, ++x, y);
            }
        if (before == g_4b9934)
            g_4b9934 = 0;
    }
    return g_4b9934;
}

/* @zoombi32 0x0046b747 */
void fn_46b747(View *, short event)
{
    if (event == 30)
        g_4b0d52 = g_4b0d54;
}

/* Opens scene 2, the journey on from a place (route g_4a7e68, 1-16: from
   scene g_4b0d56 to g_4b0d54): the backdrop, views and sounds of the map
   the next place is on (g_4b9916: 0 Zoombini Isle, 1-4 the maps, 5 the
   town), a sound chosen by the camp's hint and the level, the party and,
   on the maps, the grid being filled in under the map's name. */
/* @zoombi32 0x004697f1 */
void openScene2()
{
    short mapView;
    short visits;
    short to;
    Point places[16];
    short from;
    short fromPlace;
    short toPlace;
    short scene;
    short level;
    short hint;
    short scripts;
    short backdrop;
    short group;
    short i;
    View *view;
    Font *font;

    g_4b98d8 = 0;
    g_4a7d3c++;
    resetScene2();
    addSoundRange(20000, 29999, 1);
    setViewsLocked(0);
    openGameFile(&g_4b98d4, "xfer.MHK");
    fn_46be2e(g_4b98d4);
    switch (g_4a7e68) {
    case 1:
        from = 3;
        to = 7;
        fromPlace = 0;
        toPlace = 1;
        break;
    case 2:
        from = 7;
        to = 8;
        fromPlace = 1;
        toPlace = 2;
        break;
    case 3:
        from = 8;
        to = 9;
        fromPlace = 2;
        toPlace = 3;
        break;
    case 4:
        from = 9;
        to = 4;
        fromPlace = 3;
        toPlace = 4;
        break;
    case 5:
        from = 4;
        to = 10;
        fromPlace = 4;
        toPlace = 5;
        break;
    case 6:
        from = 10;
        to = 11;
        fromPlace = 5;
        toPlace = 6;
        break;
    case 7:
        from = 11;
        to = 12;
        fromPlace = 6;
        toPlace = 7;
        break;
    case 8:
        from = 12;
        to = 5;
        fromPlace = 7;
        toPlace = 11;
        break;
    case 9:
        from = 4;
        to = 13;
        fromPlace = 4;
        toPlace = 8;
        break;
    case 10:
        from = 13;
        to = 14;
        fromPlace = 8;
        toPlace = 9;
        break;
    case 11:
        from = 14;
        to = 15;
        fromPlace = 9;
        toPlace = 10;
        break;
    case 12:
        from = 15;
        to = 5;
        fromPlace = 10;
        toPlace = 16;
        break;
    case 13:
        from = 5;
        to = 16;
        fromPlace = 11;
        toPlace = 12;
        break;
    case 14:
        from = 16;
        to = 17;
        fromPlace = 12;
        toPlace = 13;
        break;
    case 15:
        from = 17;
        to = 18;
        fromPlace = 13;
        toPlace = 14;
        break;
    case 16:
        from = 18;
        to = 6;
        fromPlace = 14;
        toPlace = 15;
        break;
    default:
        from = 0;
        break;
    }
    if (from) {
        g_4b0d56 = from;
        g_4b0d54 = to;
        fn_46b084(g_4b98e4);
        if (g_4b98e4[fromPlace] < 0)
            g_4b98e4[fromPlace] = 1;
        g_4b98dc = g_4b98e4[toPlace];
        g_4b98e4[toPlace] = -1;
    } else
        fn_46b084(g_4b98e4);
    switch (g_4b0d54) {
    case 5:
        g_4b9916 = 2;
        if (g_4b0d56 == 15)
            g_4b9916 = 3;
        visits = *(short *)(g_4a4ba0 + 0x3e);
        break;
    case 7:
        g_4b9916 = 0;
        visits = *(short *)(g_4a4ba0 + 0x2a);
        break;
    case 8:
        g_4b9916 = 1;
        visits = *(short *)(g_4a4ba0 + 0x2c);
        break;
    case 9:
        g_4b9916 = 1;
        visits = *(short *)(g_4a4ba0 + 0x2e);
        break;
    case 4:
        g_4b9916 = 1;
        visits = *(short *)(g_4a4ba0 + 0x30);
        break;
    case 10:
        g_4b9916 = 2;
        visits = *(short *)(g_4a4ba0 + 0x32);
        break;
    case 11:
        g_4b9916 = 2;
        visits = *(short *)(g_4a4ba0 + 0x34);
        break;
    case 12:
        g_4b9916 = 2;
        visits = *(short *)(g_4a4ba0 + 0x36);
        break;
    case 13:
        g_4b9916 = 3;
        visits = *(short *)(g_4a4ba0 + 0x38);
        break;
    case 14:
        g_4b9916 = 3;
        visits = *(short *)(g_4a4ba0 + 0x3a);
        break;
    case 15:
        g_4b9916 = 3;
        visits = *(short *)(g_4a4ba0 + 0x3c);
        break;
    case 16:
        g_4b9916 = 4;
        visits = *(short *)(g_4a4ba0 + 0x40);
        break;
    case 17:
        g_4b9916 = 4;
        visits = *(short *)(g_4a4ba0 + 0x42);
        break;
    case 18:
        g_4b9916 = 4;
        visits = *(short *)(g_4a4ba0 + 0x44);
        break;
    case 6:
        visits = *(short *)(g_4a4ba0 + 0x46);
        g_4b9916 = 5;
        break;
    }
    scene = currentScene;
    currentScene = g_4b0d54;
    level = sceneLevel() + 1;
    hint = campHint(&visits);
    currentScene = scene;
    scripts = 0;
    switch (g_4b9916) {
    case 0:
        switch (hint) {
        case 0:
            if (level >= 2 && level <= 3)
                switch (randomBetween(1, 6)) {
                case 1:
                    g_4b9914 = 20094;
                    break;
                case 2:
                    g_4b9914 = 20095;
                    break;
                case 3:
                    g_4b9914 = 20096;
                    break;
                case 4:
                    g_4b9914 = 20097;
                    break;
                case 5:
                    g_4b9914 = 20098;
                    break;
                case 6:
                    g_4b9914 = 20099;
                    break;
                }
            else
                switch (randomBetween(1, 5)) {
                case 1:
                    g_4b9914 = 20094;
                    break;
                case 2:
                    g_4b9914 = 20095;
                    break;
                case 3:
                    g_4b9914 = 20096;
                    break;
                case 4:
                    g_4b9914 = 20097;
                    break;
                case 5:
                    g_4b9914 = 20099;
                    break;
                }
            break;
        case 1:
            g_4b9914 = 20094;
            break;
        case 2:
        case 12:
            g_4b9914 = 20098;
            break;
        case 5:
            if (level >= 2 && level <= 3)
                g_4b9914 = 20098;
            else
                g_4b9914 = 20094;
            break;
        }
        backdrop = 5000;
        scripts = 9;
        break;
    case 1:
        switch (g_4b0d54) {
        case 8:
            switch (hint) {
            default:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20007;
                    break;
                case 2:
                    g_4b9914 = 20008;
                    break;
                case 3:
                    g_4b9914 = 20009;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
                g_4b9914 = 20008;
                break;
            }
            break;
        case 9:
            switch (hint) {
            default:
                if (level >= 2)
                    switch (randomBetween(1, 2)) {
                    case 1:
                        g_4b9914 = 20011;
                        break;
                    case 2:
                        g_4b9914 = 20012;
                        break;
                    }
                else
                    switch (randomBetween(1, 2)) {
                    case 1:
                        g_4b9914 = 20010;
                        break;
                    case 2:
                        g_4b9914 = 20012;
                        break;
                    }
                break;
            case 1:
                g_4b9914 = 20010;
                break;
            case 2:
            case 12:
                g_4b9914 = 20011;
                break;
            case 5:
                if (level >= 2)
                    g_4b9914 = 20011;
                else
                    g_4b9914 = 20010;
                break;
            }
            break;
        case 4:
            switch (randomBetween(1, 2)) {
            case 1:
                g_4b9914 = 20009;
                break;
            case 2:
                g_4b9914 = 20012;
                break;
            }
            break;
        }
        backdrop = 1000;
        scripts = 3;
        break;
    case 2:
        switch (g_4b0d54) {
        case 10:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        g_4b9914 = 20013;
                        break;
                    case 2:
                        g_4b9914 = 20014;
                        break;
                    case 3:
                        g_4b9914 = 20015;
                        break;
                    case 4:
                        g_4b9914 = 20016;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        g_4b9914 = 20013;
                        break;
                    case 2:
                        g_4b9914 = 20014;
                        break;
                    case 3:
                        g_4b9914 = 20016;
                        break;
                    }
                break;
            case 1:
            case 5:
                g_4b9914 = 20014;
                break;
            case 2:
            case 12:
                g_4b9914 = 20015;
                break;
            }
            break;
        case 11:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        g_4b9914 = 20017;
                        break;
                    case 2:
                        g_4b9914 = 20018;
                        break;
                    case 3:
                        g_4b9914 = 20019;
                        break;
                    case 4:
                        g_4b9914 = 20020;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        g_4b9914 = 20017;
                        break;
                    case 2:
                        g_4b9914 = 20018;
                        break;
                    case 3:
                        g_4b9914 = 20020;
                        break;
                    }
                break;
            case 1:
                g_4b9914 = 20018;
                break;
            case 2:
            case 12:
                g_4b9914 = 20019;
                break;
            case 5:
                if (level >= 2)
                    g_4b9914 = 20019;
                else
                    g_4b9914 = 20018;
                break;
            }
            break;
        case 12:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20021;
                    break;
                case 2:
                    g_4b9914 = 20022;
                    break;
                case 3:
                    g_4b9914 = 20024;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                g_4b9914 = 20022;
                break;
            }
            break;
        case 5:
            switch (randomBetween(1, 3)) {
            case 1:
                g_4b9914 = 20016;
                break;
            case 2:
                g_4b9914 = 20020;
                break;
            case 3:
                g_4b9914 = 20024;
                break;
            }
            break;
        }
        backdrop = 2000;
        scripts = 3;
        break;
    case 3:
        switch (g_4b0d54) {
        case 13:
            switch (hint) {
            case 0:
                if (level == 1 || level == 3)
                    switch (randomBetween(1, 3)) {
                    case 1:
                        g_4b9914 = 20025;
                        break;
                    case 2:
                        g_4b9914 = 20026;
                        break;
                    case 3:
                        g_4b9914 = 20028;
                        break;
                    }
                else
                    switch (randomBetween(1, 4)) {
                    case 1:
                        g_4b9914 = 20025;
                        break;
                    case 2:
                        g_4b9914 = 20026;
                        break;
                    case 3:
                        g_4b9914 = 20027;
                        break;
                    case 4:
                        g_4b9914 = 20028;
                        break;
                    }
                break;
            case 1:
            case 5:
                g_4b9914 = 20026;
                break;
            case 2:
            case 12:
                g_4b9914 = 20026;
                break;
            }
            break;
        case 14:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20029;
                    break;
                case 2:
                    g_4b9914 = 20030;
                    break;
                case 3:
                    g_4b9914 = 20031;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                g_4b9914 = 20030;
                break;
            }
            break;
        case 15:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20032;
                    break;
                case 2:
                    g_4b9914 = 20033;
                    break;
                case 3:
                    g_4b9914 = 20034;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                g_4b9914 = 20033;
                break;
            }
            break;
        case 5:
            switch (randomBetween(1, 3)) {
            case 1:
                g_4b9914 = 20028;
                break;
            case 2:
                g_4b9914 = 20031;
                break;
            case 3:
                g_4b9914 = 20034;
                break;
            }
            break;
        }
        backdrop = 3000;
        scripts = 3;
        break;
    case 4:
        switch (g_4b0d54) {
        case 16:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20035;
                    break;
                case 2:
                    g_4b9914 = 20036;
                    break;
                case 3:
                    g_4b9914 = 20037;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                g_4b9914 = 20036;
                break;
            }
            break;
        case 17:
            switch (hint) {
            case 0:
                if (level >= 2)
                    switch (randomBetween(1, 4)) {
                    case 1:
                        g_4b9914 = 20000;
                        break;
                    case 2:
                        g_4b9914 = 20001;
                        break;
                    case 3:
                        g_4b9914 = 20002;
                        break;
                    case 4:
                        g_4b9914 = 20003;
                        break;
                    }
                else
                    switch (randomBetween(1, 3)) {
                    case 1:
                        g_4b9914 = 20000;
                        break;
                    case 2:
                        g_4b9914 = 20001;
                        break;
                    case 3:
                        g_4b9914 = 20003;
                        break;
                    }
                break;
            case 1:
                g_4b9914 = 20002;
                break;
            case 2:
            case 5:
            case 12:
                if (level >= 2)
                    g_4b9914 = 20002;
                else
                    g_4b9914 = 20001;
                break;
            }
            break;
        case 18:
            switch (hint) {
            case 0:
                switch (randomBetween(1, 3)) {
                case 1:
                    g_4b9914 = 20004;
                    break;
                case 2:
                    g_4b9914 = 20005;
                    break;
                case 3:
                    g_4b9914 = 20006;
                    break;
                }
                break;
            case 1:
            case 2:
            case 5:
            case 12:
                g_4b9914 = 20005;
                break;
            }
            break;
        }
        backdrop = 4000;
        scripts = 3;
        break;
    case 5:
        switch (hint) {
        default:
            switch (randomBetween(1, 4)) {
            case 1:
                g_4b9914 = 20100;
                break;
            case 2:
                g_4b9914 = 20101;
                break;
            case 3:
                g_4b9914 = 20102;
                break;
            case 4:
                g_4b9914 = 20103;
                break;
            }
            /* falls through: always 20100 */
        case 1:
        case 5:
            g_4b9914 = 20100;
            break;
        }
        backdrop = 6000;
        scripts = 9;
        break;
    }
    group = backdrop + 100;
    mapView = 0;
    drawBackdrop(backdrop);
    if (scripts) {
        loadFeatureGroup(group, 0, 0);
        loadScripts(group, scripts);
        if (backdrop >= 1000 && backdrop <= 4000) {
            g_4b9920 = backdrop + 200;
            loadFeatureGroup(g_4b9920, 1, 0);
            addScripts(g_4b9920, 1, 0);
        }
        fn_4148da(10, 236);
        if (!g_4b9916) {
            for (i = 5102; i <= 5103; i++)
                g_4b9900[i - 5102] = addView(0x1188000, drawCels, runViewScript, i, 6, 0, 0, 0);
            for (i = 5104; i <= 5107; i++)
                g_4b98f8[i - 5104] = addView(0x1188000, drawCels, runViewScript, i, 6, 0, 0, 0);
            g_4b98f6 = addView(0x1188000, drawCels, runViewScript, 5108, 6, 0, 0, 0);
            useAltSnoids(0);
            for (i = 0; i < 16; i++) {
                places[i].x = 200;
                places[i].y = 235;
            }
            setViewPlaces(16, places, 1);
            makePartySnoids(0);
            g_4b9906 = addView(0, drawCels, runViewScript, 5100, 0, 0, 0, 0);
            addView(0, drawCels, runViewScript, 5101, 0, 0, 0, 0);
            loadSnoidScripts(5199, 1, 0);
            addSnoidScripts(5200, 5, 0);
        } else if (g_4b9916 < 5) {
            mapView = addView(0xc10c000, drawCels, runViewScript, group, 6, 0, 0, 0);
            view = findView(mapView);
            if (view)
                view->placed = fn_46b326;
        } else {
            g_4b990a = addView(0x1188000, drawCels, runViewScript, 6108, 6, 0, 0, 0);
            g_4b9926 = addView(0, fn_46b761, runViewScript, 6105, 0, 0, 0, 0);
            g_4b9906 = addView(0, drawCels, runViewScript, 6104, 0, 0, 0, 0);
            for (i = 0; i < 16; i++) {
                places[i].x = -22;
                places[i].y = randomBetween(0, 3) * 6 + 282;
            }
            setViewPlaces(16, places, 1);
            makePartySnoids(0);
            for (i = 6100; i <= 6103; i++)
                addView(0, drawCels, runViewScript, i, 0, 0, 0, 0);
            loadSnoidScripts(5199, 1, 0);
            addSnoidScripts(6200, 5, 0);
            g_4b9922 = addView(0x1180000, drawCels, runViewScript, 6106, 6, 0, 0, 0);
            g_4b9924 = addView(0x1180000, drawCels, runViewScript, 6107, 6, 0, 0, 0);
        }
    } else
        fn_4148da(10, 236);
    if (g_4b9916 >= 1 && g_4b9916 <= 4) {
        g_4b9920 = addView(0x4000000, fn_46bc51, fn_46bdde, g_4b9920, 4, 0, 0, 0);
        view = findView(g_4b9920);
        if (view)
            view->placed = fn_46bbce;
        for (i = 0; i < 16; i++) {
            places[i].x = -22;
            places[i].y = 445;
        }
        setViewPlaces(16, places, 1);
        makePartySnoids(0);
        addView(0, drawCels, runViewScript, group + 1, 0, 0, 0, 0);
        addView(0, drawCels, runViewScript, group + 2, 0, 0, 0, 0);
    }
    startView(g_4b9926, 0, 0, 0);
    updateViews();
    if (g_4b9916 >= 1 && g_4b9916 <= 4) {
        Color saved;

        font = setFont(fonts[2]);
        saved = setForeColor(Color(10));
        drawOutlinedText(45, 10, mapTitleRects[g_4b9916 - 1], 0x22, levelTexts[g_4b9916 + 5]);
        copyPortBits(viewPort, workPort, gameRect, gameRect, 0);
        setForeColor(saved);
        setFont(font);
        deleteView(mapView);
    }
    setGroupLists(xferGroups, 1, (short)0xc000);
    if (g_4b9914)
        queueViewSound(g_4b9914, 1);
    showRect(&g_4aa7b8);
    fadeInViews();
    resetViewClock();
    g_4b991a = countChosenSnoids();
    g_4b98d8 = 1;
    startView(g_4b9924, 0, 0, 0);
    startView(g_4b9922, 0, 0, 0);
    if (g_4b9916 >= 1 && g_4b9916 <= 4)
        sendSnoids(670, 445, 90);
}

/* Closes scene 2. */
/* @zoombi32 0x0046ac6e */
void closeScene2()
{
    if (g_4b98d8) {
        g_4b98d8 = 0;
        short saved = fn_46bee9(1);

        g_4a7e68 = 0;
        useAltSnoids(1);
        chooseSnoids(1, 1);
        clearViews();
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4b98d4);
        fadeOutViews();
        fn_4624fc();
        g_4a4b98 = 64;
        if (g_4a7d3c)
            g_4a7d3c--;
    }
}

/* Sets up the grid (`rows` of `columns` cells, `stride` apart): cells
   `from1` become `to1` and `from2` `to2`, counted in g_4b9930 (and
   g_4b9934); fn_46bb0c then marks those taken as `taken1` and `taken2`.
   The first of g_4b9944 is `start`, kept inside the grid. */
/* @zoombi32 0x0046b872 */
void fn_46b872(char *grid, unsigned long stride, unsigned long rows, unsigned long columns,
               unsigned char from1, unsigned char from2, char to1, char to2, char taken1,
               char taken2, Point &start)
{
    unsigned long i;
    char *row;
    unsigned long y;
    char *cell;
    unsigned long x;

    for (i = 0; i < 24; i++) {
        g_4b99a4[i] = 0;
        g_4b9944[i].x = 0;
        g_4b9944[i].y = 0;
    }
    g_4b9930 = 0;
    row = grid;
    for (y = 0; y < rows; y++) {
        cell = row;
        for (x = 0; x < columns; x++) {
            char c = *cell;

            if (c == from1) {
                g_4b9930++;
                *cell = to1;
            }
            if (c == from2) {
                g_4b9930++;
                *cell = to2;
            }
            cell++;
        }
        row += stride;
    }
    g_4b99c0 = to1;
    g_4b99c2 = to2;
    g_4b99c1 = taken1;
    g_4b99c3 = taken2;
    g_4b99bc = grid;
    g_4b9938 = stride;
    g_4b993c = rows;
    g_4b9940 = columns;
    g_4b9934 = g_4b9930;
    if (1) {
        x = start.x;
        y = start.y;
    } else {
        x = 0;
        y = 0;
    }
    if (x > columns)
        x = columns - 1;
    if (y > rows)
        y = rows - 1;
    g_4b99a4[0] = 1;
    g_4b9944[0].x = x;
    g_4b9944[0].y = y;
}

/* Marks the cell at `cell` taken (g_4b99c0 becomes g_4b99c1, g_4b99c2
   becomes g_4b99c3), noting the point in a free one of g_4b9944. */
/* @zoombi32 0x0046bb0c */
void fn_46bb0c(char *cell, long x, long y)
{
    short i;

    if (*cell == g_4b99c0)
        for (i = 0; i < 24; i++)
            if (!g_4b99a4[i]) {
                g_4b9944[i].x = x;
                g_4b9944[i].y = y;
                g_4b99a4[i] = 1;
                if (g_4b9934 > 0)
                    g_4b9934--;
                *cell = g_4b99c1;
                return;
            }
    if (*cell == g_4b99c2)
        for (i = 0; i < 24; i++)
            if (!g_4b99a4[i]) {
                g_4b9944[i].x = x;
                g_4b9944[i].y = y;
                g_4b99a4[i] = 1;
                if (g_4b9934 > 0)
                    g_4b9934--;
                *cell = g_4b99c3;
                return;
            }
}

/* A view's placed callback: keeps one of its first four cels by
   g_4b9928 (1-4) as the first, alone. */
/* @zoombi32 0x0046bbce */
void fn_46bbce(View *view)
{
    ViewCel *cels = view->body.cels;

    switch (g_4b9928) {
    case 1:
    default:
        cels[1].image = 0;
        break;
    case 2:
        cels[0].image = cels[1].image;
        cels[0].x = cels[1].x;
        cels[0].y = cels[1].y;
        cels[1].image = 0;
        break;
    case 3:
        cels[0].image = cels[2].image;
        cels[0].x = cels[2].x;
        cels[0].y = cels[2].y;
        cels[1].image = 0;
        break;
    case 4:
        cels[0].image = cels[3].image;
        cels[0].x = cels[3].x;
        cels[0].y = cels[3].y;
        cels[1].image = 0;
        break;
    }
}

/* A view's draw callback: while it changes, fills in its first cel's
   image (a grid, the header's words big-endian) 7 thousandths more each
   time: first setting up the grid (fn_46b872) from where map g_4b9916's
   place g_4b9928 starts, with marks by g_4b991e's level, then spreading
   them (fn_46b9a2). */
/* @zoombi32 0x0046bc51 */
void fn_46bc51(View *view)
{
    char to1;
    char to2;
    char taken1;
    char taken2;
    ViewCel *cel;
    short start;
    unsigned short *header;
    ImageBank *bank;

    if (!view->changed) {
        drawCels(view);
        return;
    }
    if (!g_4b992c) {
        cel = view->body.cels;
        bank = groupBanks[view->body.scriptGroup];
        if (cel->image) {
            start = (g_4b9916 - 1) * 4 + g_4b9928 - 1;
            if (start < 0 || start > 15)
                start = 0;
            switch (g_4b991e) {
            default:
                to1 = '.';
                to2 = '/';
                taken1 = '0';
                taken2 = '1';
                break;
            case 2:
                to1 = '0';
                to2 = '1';
                taken1 = '2';
                taken2 = '3';
                break;
            case 3:
                to1 = '2';
                to2 = '3';
                taken1 = '4';
                taken2 = '5';
                break;
            case 4:
                to1 = '4';
                to2 = '5';
                taken1 = '6';
                taken2 = '7';
                break;
            }
            header = (unsigned short *)((char *)bank + bank->offsets[cel->image]);
            fn_46b872((char *)(header + 4), swapShort(header[2]), swapShort(header[1]),
                      swapShort(header[0]), 1, 2, to1, to2, taken1, taken2, g_4a7f08[start]);
        }
    } else
        fn_46b9a2(g_4b992c);
    g_4b992c += 7;
    if (g_4b992c > 1000)
        g_4b992c = 1000;
    drawCels(view);
}

/* A view update: runs the script, and when due redraws it. */
/* Not exact: the original keeps `region` in esi (loaded once); here it is
   read from the stack at each use, `register` or not. */
/* @zoombi32 0x0046bdde */
void fn_46bdde(View *view, short region)
{
    if (!g_4b9684) {
        runViewScript(view, region);
        if (view->nextUpdate <= updateTime) {
            view->changed = 1;
            view->nextUpdate = updateTime + view->interval;
            unionRgnRect(region, &view->body.bounds);
        }
    }
}
