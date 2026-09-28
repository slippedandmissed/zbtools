/*
 * ferry (0x42160c-0x424274): Captain Cajun's ferry (scene 13), 'Ferry.MHK'
 */

#include "zoombinis.h"
#include "features.h"
#include "ferry.h"
#include "module_4623b8.h"
#include "view.h"

/* The buttons' view update: redraws button 2 as g_4abb7a and g_4abb7c
   together change, and button 1 once. */
/* @zoombi32 0x00421bfc */
void fn_421bfc(View *, short region)
{
    if (g_4abb7a && g_4abb7c) {
        if (!g_4a16cc) {
            g_4a16cc = 1;
            unionRgnRect(region, &ferryButtons[1].rect);
        }
    } else if (g_4a16cc) {
        g_4a16cc = 0;
        unionRgnRect(region, &ferryButtons[1].rect);
    }
    if (!g_4a16ce) {
        g_4a16ce = 1;
        unionRgnRect(region, &ferryButtons[0].rect);
    }
}

/* Scene 13's keys (with debugging on, g_4b8803, or else only 0x16f):
   0x16f fn_466b93; L reports g_4abb6a (from 1). Returns whether the key
   was used. */
/* @zoombi32 0x00422491 */
short scene13Key(unsigned short key)
{
    short used = 0;

    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        fn_466b93();
        used = 1;
        break;
    case 'L':
    case 'l':
        used = 1;
        debugMessage(g_4abb6a + 1, 0, 0, 0, 0);
        break;
    }
    return used;
}

/* A view draw: while running, draws its cels from g_4abb94. The original
   passes the cel's image, x and y as three `*cel++` arguments, relying on
   BCC's left-to-right evaluation; this indexes, then steps. */
/* @zoombi32-functional 0x004224ea */
void fn_4224ea(View *view)
{
    short *cel;

    if (view->body.running)
        for (cel = (short *)view->body.cels; *cel; cel += 3)
            drawImageData((unsigned short *)((char *)g_4abb94 + g_4abb94->offsets[cel[0]]), cel[1], cel[2], 8);
}

/* Clears the scripts 4000-4058 and loads the first four. */
/* @zoombi32 0x00422537 */
void loadFerryScripts()
{
    short i;

    for (i = 0; i < 59; i++) {
        ferryScriptResources[i] = 0;
        ferryScripts[i] = 0;
    }
    for (i = 0; i < 4; i++)
        ferryScripts[i] = loadSwappedResource(&ferryScriptResources[i], i + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
}

/* Loads script `id` (4000-4058). */
/* @zoombi32 0x0042258a */
void loadFerryScript(short id)
{
    short n = id - 4000;

    if (n >= 0 && n < 59)
        ferryScripts[n] = loadSwappedResource(&ferryScriptResources[n], n + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
}

/* A notify: at event 136, starts g_4abb32's and g_4abb30's scripts. */
/* @zoombi32 0x004234c9 */
void fn_4234c9(View *, short event)
{
    View *view;

    switch (event) {
    case 136:
        view = findView(g_4abb32);
        if (view)
            view->body.running = 1;
        view = findView(g_4abb30);
        if (view)
            view->body.running = 1;
        break;
    case -1:
        break;
    }
}
