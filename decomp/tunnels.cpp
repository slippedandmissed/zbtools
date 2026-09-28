/*
 * tunnels (0x45e2d8-0x4623b8): Stone Cold Caves (scene 8), 'Tunnels.MHK'
 */

#include "zoombinis.h"
#include "e2memory.h"
#include "module_4623b8.h"
#include "sound.h"
#include "tunnels.h"
#include "view.h"

/* Closes scene 8. */
/* @zoombi32 0x0045ea2b */
void closeScene8()
{
    if (g_4b7fb8) {
        g_4b7fb8 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a7708);
        g_4b7564 = 0;
        fn_46bee9(saved);
        fn_46ca9c(&g_4b7fb4);
        fadeOutViews();
        fn_4624fc();
    }
}

/* A notify: at the end (-1), clears g_4b7fd2 and moves g_4b7fee on from
   1 to 2. */
/* @zoombi32 0x0045fa56 */
void fn_45fa56(View *, short event)
{
    switch (event) {
    case -1:
        g_4b7fd2 = 0;
        if (g_4b7fee == 1)
            g_4b7fee++;
        break;
    }
}

/* A notify: at the end (-1), fn_465175, and if g_4b8004, copies g_4b8000
   and g_4b8004 into g_4b7fd4 and g_4b7fd6 and clears g_4b7fd8. */
/* @zoombi32 0x0045fb10 */
void fn_45fb10(View *, short event)
{
    switch (event) {
    case -1:
        fn_465175();
        if (g_4b8004) {
            g_4b7fd4 = g_4b8000;
            g_4b7fd6 = g_4b8004;
            g_4b7fd8 = 0;
        }
        break;
    }
}

/* Adds `entry` to `list` if it has room (five). */
/* @zoombi32 0x00460527 */
void fn_460527(TunnelList *list, TunnelEntry entry)
{
    if (list->count < 5) {
        list->entries[list->count] = entry;
        list->count++;
    }
}
