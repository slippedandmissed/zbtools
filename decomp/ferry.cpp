/*
 * ferry (0x42160c-0x424274): Captain Cajun's ferry (scene 13), 'Ferry.MHK'
 */

#include "zoombinis.h"
#include "e2memory.h"
#include "features.h"
#include "ferry.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "snoids.h"
#include "sound.h"
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

/* Draws button `which` (1 or 2; 2 is dim unless g_4abb7a), lit or not,
   and shows it if asked. */
/* @zoombi32 0x00421b46 */
void drawFerryButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4abb7a) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a1650->offsets[image] + (char *)g_4a1650), ferryButtons[which - 1].rect.left,
                      ferryButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&ferryButtons[which - 1].rect);
    }
}

/* Closes scene 13. */
/* @zoombi32 0x00421c78 */
void closeScene13()
{
    short i;

    if (g_4abb78) {
        g_4abb78 = 0;
        short saved = fn_46bee9(1);

        if (g_4a48e6) {
            setSnoidsRunning(1);
            chooseSnoids(1, 0);
        }
        clearViews();
        unloadSounds();
        fn_46c602(&g_4abb84);
        fn_46c602(&g_4abb88);
        fn_46c602(&g_4abb8c);
        fn_46c602(&g_4abb90);
        for (i = 0; i < 59; i++)
            fn_46c602(&ferryScriptResources[i]);
        fn_46bee9(saved);
        fn_46ca9c(&g_4abb74);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The script for a Zoombini (by its feet) doing `which`: 1-5, 8, 9, 7016
   and 7021 (2 by g_4abb2e and g_4abb1e). */
/* @zoombi32 0x00423cf1 */
short ferrySnoidScript(View *view, short which)
{
    short script = 0;
    Snoid *snoid = viewSnoid(view);
    short feet = snoid->features[3];

    switch (which) {
    case 1:
        return feet + 7035;
    case 2:
        if (!g_4abb2e)
            script = 7041;
        else if (g_4abb1e == 3)
            script = 7005;
        else
            script = 7000;
        script += feet - 1;
        break;
    case 3:
        return 7010;
    case 4:
        return feet + 7010;
    case 5:
        return feet + 7030;
    case 7016:
        return feet + 7015;
    case 7021:
        return feet + 7020;
    case 8:
        return feet + 7025;
    case 9:
        return feet + 5999;
    }
    return script;
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at the end (-1), sets g_4abb3c. */
/* @zoombi32 0x00423d9d */
void fn_423d9d(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);

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
        g_4abb18 = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4abb18) {
            setSnoidFacing(snoid, g_4abb18 - 1);
            g_4abb18 = 0;
        }
        break;
    case -1:
        g_4abb3c = 1;
        break;
    }
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at the end (-1), sets g_4abb3a. */
/* @zoombi32 0x00423e2c */
void fn_423e2c(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);

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
        g_4abb18 = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4abb18) {
            setSnoidFacing(snoid, g_4abb18 - 1);
            g_4abb18 = 0;
        }
        break;
    case -1:
        g_4abb3a = 1;
        break;
    }
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at the end (-1), stops its script. */
/* @zoombi32 0x00424104 */
void fn_424104(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);

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
        g_4abb18 = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4abb18) {
            setSnoidFacing(snoid, g_4abb18 - 1);
            g_4abb18 = 0;
        }
        break;
    case -1:
        view->body.running = 0;
        break;
    }
}
