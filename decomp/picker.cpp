/*
 * picker (0x42f920-0x433510): 'Picker.MHK', 'New Game', 'Snoids to practice with = '
 */

#include "zoombinis.h"
#include "e2memory.h"
#include "features.h"
#include "module_4623b8.h"
#include "picker.h"
#include "sound.h"
#include "view.h"

/* @zoombi32 0x0042fc89 */
void fn_42fc89(Counters *object)
{
    Triple *counters = &object->counters;
    if (object->mode == 2) {
        counters->a--;
        counters->b++;
        counters->c += 2;
    }
}

/* @zoombi32 0x004320da */
long fn_4320da(long)
{
    return 0;
}

/* @zoombi32 0x004334f0 */
void fn_4334f0(long, short value)
{
    if (value == -1 && g_4afb90 < 0)
        g_4afb90 = -g_4afb90;
}

/* Closes scenes 19 and 21 (Picker.MHK). */
/* @zoombi32 0x0043190d */
void closeScene19()
{
    if (g_4afb14) {
        showCursor();
        g_4afb14 = 0;
        short saved = fn_46bee9(1);

        removeDeadViews();
        clearViews();
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4afb10);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Closes scene 20 (Picker.MHK), keeping g_4afbb8 in g_4a4b98. */
/* @zoombi32 0x004325c4 */
void closeScene20()
{
    if (g_4afb14) {
        g_4a4b98 = g_4afbb8;
        fn_465175();
        g_4afb14 = 0;
        short saved = fn_46bee9(1);

        removeDeadViews();
        clearViews();
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4afb10);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Copies g_4a1f74 from the port *g_4afb28 to the screen and starts the
   view g_4afb34 on script 1002. */
/* @zoombi32 0x0043151e */
void fn_43151e()
{
    copyPortBits(viewPort, *g_4afb28, g_4a1f74, g_4a1f74, 0);
    startView(g_4afb34, 1002, 0, 0);
}

/* A notify: 0 deletes the view pickerData.view; at the end (-1), g_4afb3a (2
   calls fn_465175) is cleared. */
/* @zoombi32 0x00431e5e */
void fn_431e5e(View *, short event)
{
    short id;

    switch (event) {
    case 0:
        id = pickerData.view;
        pickerData.view = 0;
        deleteView(id);
        break;
    case -1:
        if (g_4afb3a == 2)
            fn_465175();
        g_4afb3a = 0;
        break;
    }
}

/* A view's placed callback: raises cel g_4b754a (1-4) by four images and
   moves it two pixels up and left. */
/* @zoombi32 0x00430f8e */
void fn_430f8e(View *view)
{
    ViewBody *body = &view->body;

    switch (g_4b754a) {
    case 1:
        body->cels[1].image += 4;
        body->cels[1].x += -2;
        body->cels[1].y += -2;
        break;
    case 2:
        body->cels[2].image += 4;
        body->cels[2].x += -2;
        body->cels[2].y += -2;
        break;
    case 3:
        body->cels[3].image += 4;
        body->cels[3].x += -2;
        body->cels[3].y += -2;
        break;
    case 4:
        body->cels[4].image += 4;
        body->cels[4].x += -2;
        body->cels[4].y += -2;
        break;
    }
}

/* A drifting view's placed callback: moves it by its speed, wrapping round
   the screen (-10 to 650 across, -10 to 490 down), and puts its cel
   there. */
/* @zoombi32 0x00433388 */
void driftView(View *view)
{
    DriftingBody *body = (DriftingBody *)&view->body;

    body->x += body->dx;
    body->y += body->dy;
    if (body->x > 650)
        body->x = -10;
    else if (body->x < -10)
        body->x = 650;
    if (body->y > 490)
        body->y = -10;
    else if (body->y < -10)
        body->y = 490;
    body->cels[0].x = body->x;
    body->cels[0].y = body->y;
}

/* Resets g_4afb7e-g_4afb88 (g_4afb82, g_4afb84: the screen's centre) and
   makes the view g_4afb7c again (script 1010, placed callback
   fn_432cec). */
/* @zoombi32 0x00432905 */
void fn_432905()
{
    View *view;

    g_4afb80 = 0;
    g_4afb82 = 320;
    g_4afb84 = 240;
    g_4afb7e = 0;
    g_4afb86 = g_4afb88 = 0;
    deleteView(g_4afb7c);
    g_4afb7c = addView(0, drawCels, runViewCels, 1010, 4, 0, 0, 0);
    view = findView(g_4afb7c);
    if (view)
        view->placed = fn_432cec;
}

/* Draws a view with its cels and, in colour 45, its text (kept in its
   body from +0x3c) centred in its bounds, when it's running and to be
   redrawn. */
/* @zoombi32 0x00430ff2 */
void fn_430ff2(View *view)
{
    Color saved;

    if (view->body.running && view->reset) {
        view->reset = 0;
        drawCels(view);
        saved = setForeColor(Color(45));
        drawText(view->body.bounds, 0x22, (char *)&view->body.cels[10], 0xffff);
        setForeColor(saved);
    }
}

/* A view's placed callback: shows three two-digit numbers in its first six
   cels (images counted from the first cel's): the most in the roster
   (+0x22, raised to pickerData.counts.unknown26 if need be; that wraps
   at 100), pickerData.counts.unknown26 and pickerData.counts.unknown24. */
/* @zoombi32 0x004320e3 */
void fn_4320e3(View *view)
{
    ViewBody *body;
    short first;
    short tens;

    if (pickerData.counts.unknown26 > 99)
        pickerData.counts.unknown26 = 0;
    if (*(short *)(g_4a4ba0 + 0x22) < pickerData.counts.unknown26)
        *(short *)(g_4a4ba0 + 0x22) = pickerData.counts.unknown26;
    body = &view->body;
    first = body->cels[0].image;
    tens = *(short *)(g_4a4ba0 + 0x22) / 10;
    body->cels[0].image = first + tens;
    body->cels[1].image = *(short *)(g_4a4ba0 + 0x22) - tens * 10 + first;
    tens = pickerData.counts.unknown26 / 10;
    body->cels[2].image = first + tens;
    body->cels[3].image = pickerData.counts.unknown26 - tens * 10 + first;
    tens = pickerData.counts.unknown24 / 10;
    body->cels[4].image = first + tens;
    body->cels[5].image = pickerData.counts.unknown24 - tens * 10 + first;
}
