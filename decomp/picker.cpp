/*
 * picker (0x42f920-0x433510): 'Picker.MHK', 'New Game', 'Snoids to practice with = '
 */

#include "zoombinis.h"

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

/* A notify: 0 deletes the view g_4af8ac; at the end (-1), g_4afb3a (2
   calls fn_465175) is cleared. */
/* @zoombi32 0x00431e5e */
void fn_431e5e(View *, short event)
{
    short id;

    switch (event) {
    case 0:
        id = g_4af8ac;
        g_4af8ac = 0;
        deleteView(id);
        break;
    case -1:
        if (g_4afb3a == 2)
            fn_465175();
        g_4afb3a = 0;
        break;
    }
}
