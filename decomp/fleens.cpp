/*
 * fleens (0x42160c-0x424274): the Fleens (scene 13), 'Fleens.MHK'
 */

#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "fleens.h"
#include "focus.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "platform.h"
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
            unionRgnRect(region, &fleensButtons[1].rect);
        }
    } else if (g_4a16cc) {
        g_4a16cc = 0;
        unionRgnRect(region, &fleensButtons[1].rect);
    }
    if (!g_4a16ce) {
        g_4a16ce = 1;
        unionRgnRect(region, &fleensButtons[0].rect);
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

/* A view draw: while running, draws its cels from fleenImages. The original
   passes the cel's image, x and y as three `*cel++` arguments, relying on
   BCC's left-to-right evaluation; this indexes, then steps. */
/* @zoombi32-functional 0x004224ea */
void drawFleen(View *view)
{
    short *cel;

    if (view->body.running)
        for (cel = (short *)view->body.cels; *cel; cel += 3)
            drawImageData((unsigned short *)((char *)fleenImages + fleenImages->offsets[cel[0]]), cel[1], cel[2], 8);
}

/* Clears the scripts 4000-4058 and loads the first four. */
/* @zoombi32 0x00422537 */
void loadFleenScripts()
{
    short i;

    for (i = 0; i < 59; i++) {
        fleenScriptResources[i] = 0;
        fleenScripts[i] = 0;
    }
    for (i = 0; i < 4; i++)
        fleenScripts[i] = loadSwappedResource(&fleenScriptResources[i], i + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
}

/* Loads script `id` (4000-4058). */
/* @zoombi32 0x0042258a */
void loadFleenScript(short id)
{
    short n = id - 4000;

    if (n >= 0 && n < 59)
        fleenScripts[n] = loadSwappedResource(&fleenScriptResources[n], n + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
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
void drawFleensButton(short which, short lit, short show)
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
        drawImageData((unsigned short *)(g_4a1650->offsets[image] + (char *)g_4a1650), fleensButtons[which - 1].rect.left,
                      fleensButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&fleensButtons[which - 1].rect);
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
            fn_46c602(&fleenScriptResources[i]);
        fn_46bee9(saved);
        fn_46ca9c(&g_4abb74);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The script for a Zoombini (by its feet) doing `which`: 1-5, 8, 9, 7016
   and 7021 (2 by g_4abb2e and g_4abb1e). */
/* @zoombi32 0x00423cf1 */
short fleensSnoidScript(View *view, short which)
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
short fleenScript(View *view, short which)
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

/* A notify for the fleens' views: 0 turns (and puts g_4abb30 back after
   fleensViews[2] the first time), 1 starts fleensViews[0] once, 2 and 140 move
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
            moveView(g_4abb30, 0, fleensViews[2]);
        }
        snoid = viewSnoid(view);
        snoid->unknownF2 = !snoid->unknownF2;
        break;
    case 1:
        if (!g_4abb2e) {
            moveView(g_4abb30, 0, fleensViews[0]);
            g_4abb2e = 1;
            startView(fleensViews[0], 0, fn_4234c9, 1);
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
 * The fleens' layOutSnoid: lays out a fleen's cels for its fleen script's
 * current frame. The script's second word is the order of its feature
 * layers, set up in unknownC2 when it changes (unknownC0); the rest is as
 * layOutSnoid, with the fleens' hot spots and images.
 */
/* @zoombi32 0x00422747 */
short layOutFleen(Snoid *snoid, short *event)
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
    script = fleenScripts[snoid->body.script];
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
                *cel++ = offsetX + *at++ - fleenHotX[word];
                *cel++ = offsetY + *at++ - fleenHotY[word];
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
                *cel++ = offsetX + *at++ - fleenHotX[word];
                *cel++ = offsetY + *at++ - fleenHotY[word];
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
        unsigned short *image = (unsigned short *)(fleenImages->offsets[*cel] + (char *)fleenImages);

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
void drawFleensButtons(View *)
{
    drawFleensButton(1, 0, 0);
    drawFleensButton(2, 0, 0);
}

/*
 * The fleens' startSnoidScript: starts a fleen's view on fleen script
 * `id` (4000 on: its own scripts, unknownF4 1; others as 0), placed so that
 * its first positioned frame is at `anchor`, if given.
 */
/* @zoombi32 0x00422c82 */
void startFleenScript(View *view, short id, Point *anchor)
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
    if (!fleenScripts[index])
        fleenScripts[index] = loadSwappedResource(&fleenScriptResources[index], index + 4000, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    data = fleenScripts[index];
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
    layOutFleen(snoid, 0);
    if (removedRgn)
        unionRgnRect(removedRgn, &snoid->body.bounds);
}

/* Starts the g_4abb46 Zoombinis of g_4abb4a moving on (7021), the last
   one (8) going ahead of fleensViews[2], counted in g_4b755a until it's done
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
                script = fleensSnoidScript(view, 8);
                if (script) {
                    g_4b755a++;
                    moveView(view->id, 0, fleensViews[2]);
                    startSnoidScript(viewSnoid(view), script, 0, 0);
                    view->notifyEnd = 1;
                    view->notify = fn_42403b;
                }
            } else {
                script = fleensSnoidScript(view, 7021);
                if (script)
                    startSnoidScript(viewSnoid(view), script, 0, 0);
            }
        }
    }
    if (g_4abb46)
        g_4abb46--;
}

/*
 * The fleens' updateSnoidView, for a fleen: when due,
 * idles (now and then, by g_4a4b98, fidgeting with 2 or 3) or runs its
 * script a frame; at the script's end, back to 4000 and tells the notify
 * (-1).
 */
/* @zoombi32 0x004225cf */
void updateFleen(View *view, short region)
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
            short script = fleenScript(view, which);

            if (script) {
                startFleenScript(view, script, 0);
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
                startFleenScript(view, 0, 0);
                if (view->notifyEnd && view->notify)
                    view->notify(view, -1);
                view->notify = 0;
                view->changed = 1;
                return;
            }
            short sound = layOutFleen(snoid, &event);

            if (sound)
                queueViewSound(sound, 0);
            if (view->notify && event)
                view->notify(view, event - 1);
        } else {
            layOutFleen(snoid, 0);
        }
        view->changed = 1;
    }
}

/* Adds a view for a fleen (if it has feet) on the fleens' scripts;
   returns its id (0 for none). */
/* @zoombi32 0x00423327 */
short addFleen(Snoid *snoid)
{
    short id = 0;
    View *view;
    short i;

    if (snoid->features[3]) {
        for (i = 0; i < 16; i++)
            snoid->unknownC2[i] = 0;
        snoid->unknownC0 = -1;
        id = addView(1, drawFleen, updateFleen, 0, 6, snoid, 0, 0);
        view = findView(id);
        if (view) {
            startFleenScript(view, 0, 0);
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
            short script = fleenScript(other, 13);

            startFleenScript(other, script, 0);
        }
        break;
    case -1:
        g_4abb3e = 1;
        break;
    }
}

/* The level's rules for making fleens, in the game's state: 0-3 shift
   each feature's value (1-5), 4-7 (from level 2) move each feature to
   another's place (from 1; 0: stays). */
inline char *fleenRules()
{
    return g_4a4ba0 + 0xc;
}

/*
 * Makes the fleens, one for each traveller (g_4abdbc of them), with its
 * features changed by the level's rules: picks up to three to stand apart
 * (pickedFleens, placed by table 5000), the others by table
 * 5001; each gets its features changed by the level's rules (new rules on
 * a new game, g_4abb6a 1 or 3), and the views are stacked in order.
 */
/* Not exact: BCC32 keeps g_4a4ba0's address in edi here (dropping any one
   of the rule loops stops it), where the original loads the pointer at
   each use; the code is otherwise the same. */
/* @zoombi32 0x00422e90 */
void addFleens()
{
    short b;
    short a;
    short *pickedPlaces;
    short *otherPlaces;
    long otherResource;
    long pickedResource;
    short count;
    short flag;
    unsigned long used;
    Snoid snoid;
    short views[18];
    short i;
    short j;

    g_4abdbc = fn_4572bf();
    if (!g_4abdbc)
        return;
    flag = 0;
    count = 0;
    for (i = 0; i < 18; i++)
        views[i] = 0;
    b = a = 0;
    pickedPlaces = loadShortTable(5000, &pickedResource);
    otherPlaces = loadShortTable(5001, &otherResource);
    pickedFleens[0] = randomBetween(1, g_4abdbc);
    switch (g_4abdbc) {
    case 1:
        g_4abb1e = 2;
        g_4abb2e = 1;
        break;
    case 2:
        g_4abb1e = 1;
        break;
    }
    if (g_4abdbc >= 2)
        for (pickedFleens[1] = pickedFleens[0]; pickedFleens[1] == pickedFleens[0];)
            pickedFleens[1] = randomBetween(1, g_4abdbc);
    if (g_4abdbc >= 3)
        for (pickedFleens[2] = pickedFleens[0]; pickedFleens[2] == pickedFleens[0] || pickedFleens[2] == pickedFleens[1];)
            pickedFleens[2] = randomBetween(1, g_4abdbc);
    if (!g_4a4ba0[0xc] || g_4abb6a == 1 || g_4abb6a == 3)
        for (j = 0; j < 4; j++)
            g_4a4ba0[0xc + j] = randomBetween(1, 5);
    if (g_4abb6a > 1) {
        if (!g_4a4ba0[0x10] || g_4abb6a == 3) {
            g_4a4ba0[0x10] = randomBetween(2, 4);
            used = 1 << (g_4a4ba0[0x10] - 1);
            for (j = 5; j < 8; j++)
                g_4a4ba0[0xc + j] = g_4a16d2[allocateSlot(&used, 4, 0)];
        }
    } else {
        for (j = 4; j < 8; j++)
            g_4a4ba0[0xc + j] = 0;
    }
    for (i = 0; i < g_4abdbc; i++) {
        if (!g_4a4ba0[i * 19 + 0xa93c])
            continue;
        for (j = 0; j < 4; j++) {
            char value = ((g_4a4ba0 + i * 19)[j + 0xa934] + g_4a4ba0[0xc + j] - 2) % 5 + 1;

            if (g_4a4ba0[0xc + 4 + j])
                snoid.features[g_4a4ba0[0xc + 4 + j] - 1] = value;
            else
                snoid.features[j] = value;
        }
        snoid.unknownF1 = 0;
        snoid.unknownF2 = 0;
        if (i + 1 == pickedFleens[0] || i + 1 == pickedFleens[1] || i + 1 == pickedFleens[2]) {
            if (*pickedPlaces > a) {
                snoid.unknownF0 = a + *otherPlaces;
                snoid.body.x = pickedPlaces[a * 2 + 1];
                snoid.body.y = pickedPlaces[a * 2 + 2];
                a++;
            }
        } else if (*otherPlaces > b) {
            snoid.unknownF0 = b;
            snoid.body.x = otherPlaces[b * 2 + 1];
            snoid.body.y = otherPlaces[b * 2 + 2];
            b++;
            flag = 1;
        }
        for (j = 0; j < 10; j++)
            snoid.name[j] = (g_4a4ba0 + i * 19)[j + 0xa93d];
        snoid.home = *(Point *)&snoid.body.x;
        *(Point *)&snoid.body.unknownAa = *(Point *)&snoid.body.x;
        *(Point *)&snoid.targetX = *(Point *)&snoid.body.x;
        snoid.unknownEa = 0;
        snoid.unknownEb = 0;
        snoid.unknownEc = 0;
        snoid.unknownEe = 0;
        snoid.unknownF8 = randomBetween(0, 80);
        snoid.unknownF7 = 1;
        g_4abba2[i] = addFleen(&snoid);
        g_4abbc2[i] = 0;
        if (flag) {
            flag = 0;
            views[count] = g_4abba2[i];
            count++;
        }
    }
    fn_46c602(&pickedResource);
    fn_46c602(&otherResource);
    moveView(views[2], 0, views[1]);
    moveView(views[5], 0, views[4]);
    moveView(views[8], 1, views[7]);
    moveView(views[9], 1, views[8]);
    moveView(views[10], 0, views[8]);
    moveView(views[11], 0, views[10]);
}

/* Scene 13's clicks: 1 leaves (asking whether to keep the party), 2 sends
   the Zoombinis on (once ready, g_4abb7a and g_4abb7c), counted in
   g_4b755a, 3 (while nothing's moving) drags a Zoombini: one placed
   (unknownF7) freely, another only when its fleen (g_4abba2) is idle, noting where it was put (g_4abb6e,
   g_4abb70) or sending it back to a free place; with g_4b754a, a click on a
   fleen makes its Zoombini jump. */
/* @zoombi32 0x00422192 */
void scene13Clicked(short which)
{
    Point where;
    Snoid *snoid;
    View *view;
    short id;
    short fleen;
    short i;
    View *other;
    short moved;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        closeScene13();
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawFleensButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFleensButton(which, 0, 1);
        g_4b0d52 = 1;
        askKeepParty();
        break;
    case 2:
        if (!g_4abb7a || !g_4abb7c)
            break;
        queueViewSound(996, 0);
        drawFleensButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFleensButton(which, 0, 1);
        chooseSnoids(1, 0);
        g_4abb40 = 1;
        g_4b755c = 0;
        g_4b755a = g_4abb46;
        g_4b0d52 = 14;
        break;
    case 3:
        if (g_4abb80 || g_4b755a > 0 || g_4abb1e >= 3)
            break;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (view)
            id = viewSnoid(view)->unknownF4;
        if (view && (!id || id == 6)) {
            snoid = viewSnoid(view);
            if (viewSnoid(view)->unknownF7) {
                if (!g_4abb7e) {
                    g_4b7556 = 1;
                    dragSnoid(view, where, 0, 0);
                }
            } else {
                id = view->id;
                for (i = 0; i < g_4abba0; i++)
                    if (id == partyViews[i]) {
                        g_4abbc2[i] = 1;
                        fleen = g_4abba2[i];
                        i = g_4abba0;
                    }
                other = findView(fleen);
                if (other && viewSnoid(other)->unknownF7) {
                    if (id == g_4abb6e)
                        g_4abb6e = g_4abb70 = 0;
                    moved = dragSnoid(view, where, 0, 0);
                    if (heldPlaceNumber()) {
                        g_4abb6e = id;
                        g_4abb70 = fleen;
                    } else if (moved) {
                        if (snoid->body.x != snoid->targetX || snoid->body.y != snoid->targetY)
                            pickFreePlace((Point *)&snoid->targetX, viewPlaces, 16, 500);
                    }
                }
            }
        }
        if (g_4b754a) {
            view = viewAt(where, 2, 1);
            if (view) {
                moved = view->id;
                for (i = 0; i < g_4abba0; i++)
                    if (moved == g_4abba2[i]) {
                        other = idleSnoidView(partyViews[i]);
                        if (other) {
                            viewSnoid(other)->unknownF5 = 15;
                            setSnoidAction(viewSnoid(other), 3, 0);
                        }
                        i = g_4abba0;
                    }
            }
        }
        break;
    }
}

/*
 * g_4abb30's notify: besides turning (250-253, 240-243, 0), moves
 * views into order (0, 8, 9), sends the picked fleens on or back by
 * g_4abb1e when g_4abb30 is in place (6), moves the Zoombinis in line along
 * (132), stops (137), sets the fleens in a range of places off (133-135),
 * adds a random extra (30), and has g_4abb30 act (4, 5, 7: a fleen script,
 * maybe placed at an anchor).
 */
/* Not exact: in case 30 the original calls randomBetween before pushing
   addView's first three arguments (as for a temporary in eax); a
   temporary here lands in ebx, and the call inline is evaluated in order. */
/* @zoombi32 0x0042365a */
void fn_42365a(View *view, short event)
{
    short which;
    short low;
    short high;
    short act;
    short seven;
    short seat;
    Point at;
    Point *anchor;
    View *actor = 0;
    View *other;
    Snoid *snoid;
    short i;
    short script;
    short spot;

    anchor = 0;
    seat = act = 0;
    snoid = viewSnoid(view);
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
        if (g_4abb44 && view->id == g_4abb32) {
            g_4abb44 = 0;
            view->flags |= 0x4000000;
            moveView(g_4abb32, 0, fleensViews[2]);
        }
        break;
    case 4:
        g_4abb44 = 1;
        actor = findView(g_4abb30);
        if (actor) {
            spot = viewSnoid(actor)->unknownF0;
            if (spot >= 0 && spot <= 16) {
                which = 5;
                act = 1;
            }
        }
        break;
    case 5:
        which = 6;
        at.x = 429;
        at.y = 223;
        anchor = &at;
        act = 1;
        break;
    case 6:
        actor = findView(g_4abb30);
        if (!actor)
            break;
        spot = viewSnoid(actor)->unknownF0;
        if (spot < 17 || spot > 19)
            break;
        other = findView(g_4abb1c);
        if (!other)
            break;
        if (g_4abb1e < 1 || g_4abb1e > 3)
            break;
        {
            short id = g_4abb1e + 1000;

            setViewScript(other, id, 1);
        }
        for (i = 0; i < 3; i++)
            if (pickedFleens[i] && g_4abba2[pickedFleens[i] - 1] == g_4abb30)
                pickedFleens[i] = 0;
        for (i = 0; i < 3; i++)
            if (pickedFleens[i]) {
                other = findView(g_4abba2[pickedFleens[i] - 1]);
                if (other && g_4abb1e < 3) {
                    script = fleenScript(other, g_4abb1e + 6);
                    if (script) {
                        startFleenScript(other, script, 0);
                        other->notify = fn_423512;
                    }
                }
            }
        g_4abb7a = g_4abb1e == 3;
        break;
    case 7:
        at.x = 214;
        at.y = 207;
        if (!g_4abb2e) {
            which = 9;
            anchor = &at;
        } else if (g_4abb1e == 3) {
            at.x = -56;
            at.y = 241;
            anchor = &at;
            which = 11;
        } else {
            which = 10;
            anchor = &at;
        }
        seat = 1;
        act = 1;
        claimPlacedView(1, 0);
        g_4abb42 = 1;
        break;
    case 8:
        view->flags |= 0x4000000;
        moveView(view->id, 0, fleensViews[0]);
        break;
    case 132:
        if (!g_4abb6c)
            break;
        g_4abb6c = 0;
        seven = g_4abb46 == 7;
        for (i = 0; i < g_4abb46 - 1; i++) {
            other = findView(g_4abb4a[i]);
            if (other) {
                script = fleensSnoidScript(other, 7016);
                if (script)
                    startSnoidScript(viewSnoid(other), script, 0, 0);
                if (!i && seven) {
                    other->notifyEnd = 1;
                    other->notify = fn_423d9d;
                }
            }
            other = findView(g_4abb58[i]);
            if (other) {
                script = fleenScript(other, 12);
                if (script)
                    startFleenScript(other, script, 0);
            }
        }
        if (seven) {
            g_4abb66 = g_4abb4a[0];
            g_4abb68 = g_4abb58[0];
            for (i = 1; i < g_4abb46; i++) {
                g_4abb4a[i - 1] = g_4abb4a[i];
                g_4abb58[i - 1] = g_4abb58[i];
            }
            g_4abb46--;
        }
        break;
    case 9:
        moveView(view->id, 0, g_4abb30);
        break;
    case 137:
        view->body.running = 0;
        break;
    case 133:
    case 134:
    case 135:
        switch (event) {
        case 133:
            low = 0;
            high = 3;
            break;
        case 134:
            low = 4;
            high = 16;
            break;
        case 135:
            low = 20;
            high = 25;
            break;
        }
        for (i = 0; i < 16; i++)
            if (g_4abbc2[i] < 2) {
                other = findView(g_4abba2[i]);
                if (other && viewSnoid(other)->unknownF0 >= low && viewSnoid(other)->unknownF0 <= high) {
                    g_4abbc2[i] = 2;
                    script = fleenScript(other, 14);
                    if (script) {
                        startFleenScript(other, script, 0);
                        other->notifyEnd = 1;
                        if (event != 135)
                            other->notify = fn_423512;
                    }
                }
            }
        break;
    case 28:
        fn_42365a(view, 8);
        break;
    case 30:
        other = findView(addView(0x100000, drawCels, runViewScript, randomBetween(0, 2) + 1004, 6, 0, 0, 0));
        if (other) {
            other->flags |= 0x1000;
            setViewScript(other, 0, 1);
            other->notify = fn_424195;
            other->notifyEnd = 1;
        }
        break;
    case -1:
        g_4a4b98 = 64;
        g_4abb80 = 0;
        g_4abb7e = 0;
        g_4abb30 = g_4abb32 = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 10:
    case 11:
    case 19:
        break;
    }
    if (act) {
        if (!actor)
            actor = findView(g_4abb30);
        if (actor) {
            viewSnoid(actor)->unknownF7 = 0;
            script = fleenScript(actor, which);
            if (script) {
                startFleenScript(actor, script, anchor);
                actor->notify = fn_423512;
                if (seat)
                    viewSnoid(actor)->unknownF0 = 20;
            }
        }
    }
}

/* The extra's notify (fn_42365a's 30): 135 passes on to fn_42365a; at the
   end (-1), frees the views, sets g_4abb7c, picks the Zoombinis, plays a
   cheer when all made it (20055-20063) or now and then a remark
   (20045-20048), deletes fleensViews[3] and sets g_4abb1a by how many are
   chosen. */
/* @zoombi32 0x00424195 */
void fn_424195(View *view, short event)
{
    short n;

    switch (event) {
    case 135:
        fn_42365a(view, event);
        break;
    case -1:
        setViewsLocked(0);
        g_4abb7c = 1;
        chooseSnoids(1, 0);
        n = countChosenSnoids();
        if (n) {
            if (n == g_4abba0)
                queueViewSound(randomBetween(20055, 20063), 0);
            else if (randomBetween(0, 4) > g_4abb6a || (*(short *)(g_4a4ba0 + 0x38) & 0xfff) <= 3)
                queueViewSound(randomBetween(20045, 20048), 1);
        }
        deleteView(fleensViews[3]);
        n = countChosenSnoids();
        if (n == 16)
            g_4abb1a = 13;
        else if (n > 8)
            g_4abb1a = n - 8;
        break;
    }
}

/* Opens scene 13: Fleens.MHK, its sounds, images and scripts, the views
   (the backdrop's seven, g_4abb1c, the placed spot, the buttons), the
   fleens (addFleens), the party; the first Zoombini walks in (g_4abb30's
   notify), and a hint or greeting. */
/* @zoombi32 0x00421738 */
void openScene13()
{
    Point places[16] = {{238, 368}, {185, 417}, {155, 448}, {197, 396}, {160, 357}, {164, 384},
                        {150, 416}, {116, 357}, {130, 386}, {109, 418}, {117, 448}, {74, 348},
                        {89, 384}, {67, 418}, {76, 450}, {56, 379}};
    Point place = {438, 357};
    short i;
    unsigned long flags;
    View *view;
    short script;

    g_4abb78 = g_4abb7a = 0;
    resetScene13();
    g_4abb6a = sceneLevel();
    addSoundRange(20000, 29999, 1);
    addSoundRange(1000, 1002, 1);
    addSoundRange(300, 324, 0);
    addSoundRange(425, 499, 0);
    addSoundRange(99, 99, 0);
    addSoundRange(375, 399, 0);
    addSoundRange(7000, 7099, 0);
    addSoundRange(1800, 1899, 0);
    addSoundRange(1200, 4099, 1);
    addSoundRange(4100, 4199, 0);
    addSoundRange(175, 199, 0);
    addSoundRange(900, 944, 0);
    openGameFile(&g_4abb74, "Fleens.MHK");
    fn_46be2e(g_4abb74);
    drawBackdrop(300);
    loadTerrain(500);
    fleenImages = loadImageBank(4000, &g_4abb88);
    g_4a1650 = loadImageBank(400, &g_4abb84);
    loadFeatureGroup(1000, 0, 0);
    loadFeatureGroup(1100, 1, 0);
    loadFeatureGroup(1200, 2, 0);
    fleenHotX = loadShortTable(4000, &g_4abb8c);
    fleenHotY = loadShortTable(4001, &g_4abb90);
    loadScripts(1000, 7);
    addScripts(1100, 1, 0);
    addScripts(1200, 7, 0);
    loadFleenScripts();
    loadSnoidScripts(6000, 5, 0);
    addSnoidScripts(7000, 46, 26);
    g_4abb1c = addView(0x108000, drawCels, runViewScript, 1000, 6, 0, 0, 0);
    placedViews[0] = addView(0x108a000, drawCels, runViewScript, 1100, 7, &place, 0, 0);
    addView(0x1000, drawFleensButtons, fn_421bfc, 0, 0, 0, 0, 0);
    setViewPlaces(16, places, 1);
    addFleens();
    for (i = 1200; i <= 1206; i++) {
        if (i == 1200)
            flags = 0x4180000;
        else
            flags = 0x4000000;
        fleensViews[i - 1200] = addView(flags, drawCels, runViewScript, i, 6, 0, 0, 0);
    }
    fn_4148da(10, 236);
    makePartySnoids(0);
    enterSnoids(0);
    updateViews();
    staggerSnoids(45, 200);
    view = findView(partyViews[0]);
    if (view) {
        script = fleensSnoidScript(view, 1);
        if (script) {
            Point anchor = {236, 395};

            g_4abb80 = 1;
            loadSnoidScript(script);
            startSnoidScript(viewSnoid(view), script, &anchor, 0);
            view->nextUpdate = 0;
            view->notify = fn_42365a;
            view->notifyEnd = 1;
            if (g_4b755a > 0)
                g_4b755a--;
        }
    }
    updateViews();
    setGroupLists(fleensGroups, 1, (short)0xc000);
    drawFleensButton(1, 0, 0);
    drawFleensButton(2, 0, 0);
    showRect(&g_4aa7b8);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    g_4abb78 = 1;
    switch (campHint((short *)(g_4a4ba0 + 0x38))) {
    case 2:
        g_4b966e = 20080;
        break;
    default:
        if (g_4abb6a == 1 || g_4abb6a == 3)
            g_4b966e = randomBetween(20079, 20080);
        else
            g_4b966e = 20079;
        break;
    }
    g_4abba0 = countSnoidViews();
    g_4abdac = 8;
}
