/*
 * ferry (0x42160c-0x424274): Captain Cajun's ferry (scene 13), 'Ferry.MHK'
 */

#include <stdlib.h>

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

/* A view draw: while running, draws its cels from ferryImages. The original
   passes the cel's image, x and y as three `*cel++` arguments, relying on
   BCC's left-to-right evaluation; this indexes, then steps. */
/* @zoombi32-functional 0x004224ea */
void drawFerrySnoid(View *view)
{
    short *cel;

    if (view->body.running)
        for (cel = (short *)view->body.cels; *cel; cel += 3)
            drawImageData((unsigned short *)((char *)ferryImages + ferryImages->offsets[cel[0]]), cel[1], cel[2], 8);
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

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at 131, sets g_4abb40 with g_4abb46; at
   the end (-1), moves one from g_4b755a to g_4b755c. */
/* @zoombi32 0x0042403b */
void fn_42403b(View *view, short event)
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
    case 131:
        if (g_4abb46)
            g_4abb40 = 1;
        break;
    case -1:
        if (g_4b755a) {
            g_4b755a--;
            g_4b755c++;
        }
        break;
    }
}

/* Resets scene 13's state; the pace g_4abdb4 by g_4b2b00. */
/* @zoombi32 0x0042160c */
void resetScene13()
{
    short i;

    g_4b966e = 0;
    g_4abdac = g_4abb1a = 0;
    for (i = 0; i < 16; i++)
        g_4abba2[i] = g_4abbc2[i] = 0;
    for (i = 0; i < 7; i++) {
        g_4abb4a[i] = 0;
        g_4abb58[i] = 0;
    }
    g_4abb48 = g_4abb46 = g_4abb66 = g_4abb68 = g_4abb32 = 0;
    g_4b755e = 100;
    g_4abb1e = g_4b0d52 = g_4abb70 = 0;
    g_4abb40 = 0;
    g_4abb3e = g_4abb3a = g_4abb3c = 0;
    g_4abb6c = g_4abb7c = 0;
    g_4abb42 = g_4abb44 = g_4abb7e = 0;
    g_4abb80 = 0;
    g_4abb18 = g_4abb2e = 0;
    g_4abdb0 = 0;
    g_4abdb8 = 0;
    if (g_4b2b00)
        g_4abdb4 = 120;
    else
        g_4abdb4 = 60;
    g_4b755a = g_4b755c = 0;
}

/* The script (4000 on) for a Zoombini (by its feet and its place,
   unknownF0) doing `which` (1-14). */
/* @zoombi32 0x0042339f */
short ferryScript(View *view, short which)
{
    short script = 0;
    Snoid *snoid = viewSnoid(view);
    short feet = snoid->features[3];
    short spot = snoid->unknownF0;

    switch (which) {
    case 1:
        script = 4000;
        break;
    case 2:
        script = 4000;
        break;
    case 3:
        script = 4001;
        break;
    case 4:
        script = 4002;
        break;
    case 5:
        script = 4003;
        break;
    case 6:
        if (spot <= 16)
            script = spot + 4004;
        else
            script = spot + 4000;
        break;
    case 7:
    case 8:
        if (spot >= 17 && spot <= 19) {
            script = (spot - 17) * 2 + 4020;
            if (which == 8)
                script++;
        }
        break;
    case 9:
    case 10:
    case 11:
        script = feet + 4025;
        if (which == 10)
            script += 5;
        if (which == 11)
            script += 10;
        break;
    case 12:
        script = feet + 4040;
        break;
    case 13:
        script = feet + 4045;
        break;
    case 14:
        if (spot <= 3)
            script = 4058;
        else if (spot <= 11)
            script = 4057;
        else if (spot <= 16)
            script = 4056;
        else
            script = feet + 4050;
        break;
    }
    return script;
}

/* A notify for the ferry's views: 0 turns (and puts g_4abb30 back after
   g_4abb24 the first time), 1 starts g_4abb20 once, 2 and 140 move
   g_4abb30 after or before g_4abb32, 218 a random sound (4100-4124); 137
   and the end (-1) stop the script. */
/* @zoombi32 0x00423512 */
void fn_423512(View *view, short event)
{
    Snoid *snoid;

    switch (event) {
    case 0:
        if (g_4abb42 && view->id == g_4abb30) {
            g_4abb42 = 0;
            moveView(g_4abb30, 0, g_4abb24);
        }
        snoid = viewSnoid(view);
        snoid->unknownF2 = !snoid->unknownF2;
        break;
    case 1:
        if (!g_4abb2e) {
            moveView(g_4abb30, 0, g_4abb20);
            g_4abb2e = 1;
            startView(g_4abb20, 0, fn_4234c9, 1);
        }
        break;
    case 2:
        moveView(g_4abb30, 1, g_4abb32);
        break;
    case 137:
        view->body.running = 0;
        break;
    case 140:
        moveView(g_4abb30, 0, g_4abb32);
        break;
    case 218:
        queueViewSound(randomBetween(4100, 4124), 0);
        break;
    case -1:
        view->body.running = 0;
        break;
    }
}

/*
 * The ferry's layOutSnoid: lays out a Zoombini's cels for its ferry script's
 * current frame. The script's second word is the order of its feature
 * layers, set up in unknownC2 when it changes (unknownC0); the rest is as
 * layOutSnoid, with the ferry's hot spots and images.
 */
/* @zoombi32 0x00422747 */
short ferryLayOutSnoid(Snoid *snoid, short *event)
{
    short *layers;
    short sound;
    short *script;
    short last;
    short offsetX;
    short offsetY;
    ShortRect rect;
    short *cel;
    short *at;
    short i;
    short word;

    sound = 0;
    if (event)
        *event = 0;
    snoid->body.bounds.left = 0;
    snoid->body.bounds.top = 0;
    snoid->body.bounds.right = 0;
    snoid->body.bounds.bottom = 0;
    cel = (short *)snoid->body.cels;
    layers = snoid->unknownC2;
    last = 5;
    script = ferryScripts[snoid->body.script];
    word = script[1];
    at = script + snoid->body.frameOffset;
    if (word != snoid->unknownC0) {
        snoid->unknownC0 = word;
        switch (word) {
        case 0:
            layers[1] = feetLayers[snoid->features[3]];
            layers[2] = 0;
            layers[3] = noseLayers[snoid->features[2]];
            layers[4] = eyesLayers[snoid->features[1]];
            layers[5] = hairLayers[snoid->features[0]];
            break;
        case 1:
            layers[1] = feetLayers[snoid->features[3]];
            layers[2] = noseLayers[snoid->features[2]];
            layers[3] = 0;
            layers[4] = eyesLayers[snoid->features[1]];
            layers[5] = hairLayers[snoid->features[0]];
            break;
        case 2:
            layers[1] = 0;
            layers[2] = eyesLayers[snoid->features[1]];
            layers[3] = noseLayers[snoid->features[2]];
            layers[4] = feetLayers[snoid->features[3]];
            layers[5] = hairLayers[snoid->features[0]];
            break;
        case 3:
            layers[1] = 0;
            layers[2] = feetLayers[snoid->features[3]];
            layers[3] = noseLayers[snoid->features[2]];
            layers[4] = eyesLayers[snoid->features[1]];
            layers[5] = hairLayers[snoid->features[0]];
            break;
        }
    }
    layers = snoid->unknownC2 + 1;
    offsetY = -snoid->body.unknownAc;
    if (*at > 0) {
        offsetX = -snoid->body.unknownAa;
        snoid->body.x = at[1] + offsetX;
        snoid->body.y = at[2] + offsetY;
    }
    i = 0;
    if (!snoid->unknownF2) {
        for (; i <= last; i++) {
            word = *at++;
            if (!word) {
                at += 2;
                *cel++ = 0;
                *cel++ = 0;
                *cel++ = 0;
            } else if (word > 0) {
                word = (word + layers[i]) * 2 - 1;
                *cel++ = word;
                *cel++ = offsetX + *at++ - ferryHotX[word];
                *cel++ = offsetY + *at++ - ferryHotY[word];
            } else {
                if (word < -0x100)
                    sound = *at++;
                if (event)
                    *event = word & 0xff;
                if (i)
                    *cel = 0;
                i = last + 1;
            }
        }
    } else {
        for (; i <= last; i++) {
            word = *at++;
            if (!word) {
                at += 2;
                *cel++ = 0;
                *cel++ = 0;
                *cel++ = 0;
            } else if (word > 0) {
                word = (word + layers[i]) * 2;
                *cel++ = word;
                *cel++ = offsetX + *at++ - ferryHotX[word];
                *cel++ = offsetY + *at++ - ferryHotY[word];
            } else {
                if (word < -0x100)
                    sound = *at++;
                if (event)
                    *event = word & 0xff;
                if (i)
                    *cel = 0;
                i = last + 1;
            }
        }
    }
    cel = (short *)snoid->body.cels;
    while (*cel) {
        unsigned short *image = (unsigned short *)(ferryImages->offsets[*cel] + (char *)ferryImages);

        cel++;
        rect.left = *cel++;
        rect.top = *cel++;
        rect.right = swapShort(image[0]) + rect.left;
        rect.bottom = swapShort(image[1]) + rect.top;
        unionRect(&snoid->body.bounds, &rect);
    }
    if (event) {
        snoid->body.frameOffset = at - script;
        snoid->body.frame++;
    }
    return sound;
}

/* A view draw: draws both buttons, unlit. */
/* @zoombi32 0x00421bdf */
void drawFerryButtons(View *)
{
    drawFerryButton(1, 0, 0);
    drawFerryButton(2, 0, 0);
}

/*
 * The ferry's startSnoidScript: starts a Zoombini's view on ferry script
 * `id` (4000 on: its own scripts, unknownF4 1; others as 0), placed so that
 * its first positioned frame is at `anchor`, if given.
 */
/* @zoombi32 0x00422c82 */
void startFerryScript(View *view, short id, Point *anchor)
{
    Snoid *snoid = viewSnoid(view);
    short index;
    short originX;
    short originY;
    short frame;
    short found;
    short *data;
    short n;

    snoid->unknownC0 = -1;
    if (snoid->body.group) {
        if (groupLeader[snoid->body.group] == view->id)
            groupLeader[snoid->body.group] = 0;
        g_4b8b43[snoid->body.group] = 0;
    }
    snoid->body.group = 0;
    if (id >= 4000) {
        snoid->unknownF4 = 1;
        index = id - 4000;
    } else {
        snoid->unknownF4 = 0;
        index = 0;
    }
    snoid->body.running = 1;
    snoid->body.script = index;
    if (!ferryScripts[index])
        ferryScripts[index] = loadSwappedResource(&ferryScriptResources[index], index + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    data = ferryScripts[index];
    snoid->body.frame = 0;
    snoid->body.frameOffset = 2;
    snoid->body.lastFrame = data[0];
    if (!snoid->unknownF4)
        snoid->unknownF5 = 1;
    else
        snoid->unknownF5 = 0;
    if (removedRgn)
        unionRgnRect(removedRgn, &snoid->body.bounds);
    originX = snoid->body.x;
    originY = snoid->body.y;
    if (snoid->unknownF4 == 1) {
        if (anchor) {
            n = -1;
            found = 0;
            while (!found) {
                frame = n;
                short *at = data + scriptFrameOffset(data, &frame, 1);

                if (*at > 0) {
                    originX = anchor->x;
                    originY = anchor->y;
                    found = 1;
                    data = at;
                }
                n--;
                if (abs(n) > snoid->body.lastFrame)
                    found = 1;
            }
        } else {
            data += 2;
        }
    } else {
        data += 2;
    }
    if (*data > 0) {
        snoid->body.unknownAa = data[1] - originX;
        snoid->body.unknownAc = data[2] - originY;
    }
    ferryLayOutSnoid(snoid, 0);
    if (removedRgn)
        unionRgnRect(removedRgn, &snoid->body.bounds);
}

/* Starts the g_4abb46 Zoombinis of g_4abb4a moving on (7021), the last
   one (8) going ahead of g_4abb24, counted in g_4b755a until it's done
   (fn_42403b); then one fewer. */
/* @zoombi32 0x00423f84 */
void fn_423f84()
{
    short i;
    View *view;
    short script;

    for (i = 0; i < g_4abb46; i++) {
        view = findView(g_4abb4a[i]);
        if (view) {
            if (i == g_4abb46 - 1) {
                script = ferrySnoidScript(view, 8);
                if (script) {
                    g_4b755a++;
                    moveView(view->id, 0, g_4abb24);
                    startSnoidScript(viewSnoid(view), script, 0, 0);
                    view->notifyEnd = 1;
                    view->notify = fn_42403b;
                }
            } else {
                script = ferrySnoidScript(view, 7021);
                if (script)
                    startSnoidScript(viewSnoid(view), script, 0, 0);
            }
        }
    }
    if (g_4abb46)
        g_4abb46--;
}

/*
 * The ferry's updateSnoidView, for a Zoombini on a ferry script: when due,
 * idles (now and then, by g_4a4b98, fidgeting with 2 or 3) or runs its
 * script a frame; at the script's end, back to 4000 and tells the notify
 * (-1).
 */
/* @zoombi32 0x004225cf */
void updateFerrySnoid(View *view, short region)
{
    short event;
    short changed = 0;
    Snoid *snoid;

    if (!view->body.running || g_4b9684)
        return;
    {
        short due = view->nextUpdate <= updateTime;

        if (!due)
            return;
    }
    view->nextUpdate = updateTime + view->interval;
    snoid = viewSnoid(view);
    switch (snoid->unknownF4) {
    case 0:
    default:
        if (snoid->unknownF5) {
            snoid->unknownF5 = 0;
            changed = 1;
        } else if (g_4a4b98 && snoid->unknownF8++ > g_4a4b98 + 16) {
            short which = randomBetween(1, 100) <= 50 ? 2 : 3;
            short script = ferryScript(view, which);

            if (script) {
                startFerryScript(view, script, 0);
                changed = 1;
                snoid->unknownF8 = 1;
            }
        }
        break;
    case 1:
        changed = 1;
        break;
    }
    if (changed) {
        unionRgnRect(region, &view->body.bounds);
        if (snoid->body.lastFrame > 1) {
            if (snoid->body.frame >= snoid->body.lastFrame) {
                startFerryScript(view, 0, 0);
                if (view->notifyEnd && view->notify)
                    view->notify(view, -1);
                view->notify = 0;
                view->changed = 1;
                return;
            }
            short sound = ferryLayOutSnoid(snoid, &event);

            if (sound)
                queueViewSound(sound, 0);
            if (view->notify && event)
                view->notify(view, event - 1);
        } else {
            ferryLayOutSnoid(snoid, 0);
        }
        view->changed = 1;
    }
}

/* Adds a view for a Zoombini (if it has feet) on the ferry's scripts;
   returns its id (0 for none). */
/* @zoombi32 0x00423327 */
short addFerrySnoid(Snoid *snoid)
{
    short id = 0;
    View *view;
    short i;

    if (snoid->features[3]) {
        for (i = 0; i < 16; i++)
            snoid->unknownC2[i] = 0;
        snoid->unknownC0 = -1;
        id = addView(1, drawFerrySnoid, updateFerrySnoid, 0, 6, snoid, 0, 0);
        view = findView(id);
        if (view) {
            startFerryScript(view, 0, 0);
            view->nextUpdate = 0;
            view->flags = 0x4000002;
        }
    }
    return id;
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at 60, starts g_4abb68 on 13; at the end
   (-1), sets g_4abb3e. */
/* @zoombi32 0x00423ebb */
void fn_423ebb(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);
    View *other;

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
    case 60:
        other = findView(g_4abb68);
        if (other) {
            short script = ferryScript(other, 13);

            startFerryScript(other, script, 0);
        }
        break;
    case -1:
        g_4abb3e = 1;
        break;
    }
}
