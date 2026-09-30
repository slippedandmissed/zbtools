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
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* The buttons' view update: redraws button 2 as fleensGoReady and fleensEntered
   together change, and button 1 once. */
/* @zoombi32 0x00421bfc */
void updateFleensButtons(View *, short region)
{
    if (fleensGoReady && fleensEntered) {
        if (!fleensButton2Lit) {
            fleensButton2Lit = 1;
            unionRgnRect(region, &fleensButtons[1].rect);
        }
    } else if (fleensButton2Lit) {
        fleensButton2Lit = 0;
        unionRgnRect(region, &fleensButtons[1].rect);
    }
    if (!fleensButton1Drawn) {
        fleensButton1Drawn = 1;
        unionRgnRect(region, &fleensButtons[0].rect);
    }
}

/* Scene 13's keys (with debugging on, debugMessagesOn, or else only 0x16f):
   0x16f replayHint; L reports fleensLevel (from 1). Returns whether the key
   was used. */
/* @zoombi32 0x00422491 */
short fleensKey(unsigned short key)
{
    short used = 0;

    if (!debugMessagesOn && key != 0x16f)
        return 0;
    switch (key) {
    case 0x16f:
        replayHint();
        used = 1;
        break;
    case 'L':
    case 'l':
        used = 1;
        debugMessage(fleensLevel + 1, 0, 0, 0, 0);
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

/* A notify: at event 136, starts activeSnoid's and activeFleen's scripts. */
/* @zoombi32 0x004234c9 */
void fleensStartNotify(View *, short event)
{
    View *view;

    switch (event) {
    case 136:
        view = findView(activeSnoid);
        if (view)
            view->body.running = 1;
        view = findView(activeFleen);
        if (view)
            view->body.running = 1;
        break;
    case -1:
        break;
    }
}

/* Draws button `which` (1 or 2; 2 is dim unless fleensGoReady), lit or not,
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
        if (!fleensGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(fleensButtonImages->offsets[image] + (char *)fleensButtonImages), fleensButtons[which - 1].rect.left,
                      fleensButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&fleensButtons[which - 1].rect);
    }
}

/* Closes scene 13. */
/* @zoombi32 0x00421c78 */
void closeFleens()
{
    short i;

    if (fleensOpen) {
        fleensOpen = 0;
        short saved = setFreeAtOnce(1);

        if (leavingGame) {
            setSnoidsRunning(1);
            chooseSnoids(1, 0);
        }
        clearViews();
        unloadSounds();
        freeResource(&fleensButtonResource);
        freeResource(&fleenImagesResource);
        freeResource(&fleenHotXResource);
        freeResource(&fleenHotYResource);
        for (i = 0; i < 59; i++)
            freeResource(&fleenScriptResources[i]);
        setFreeAtOnce(saved);
        closeGameFile(&fleensFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* The script for a Zoombini (by its feet) doing `which`: 1-5, 8, 9, 7016
   and 7021 (2 by fleensView0Started and pickedFleensFound). */
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
        if (!fleensView0Started)
            script = 7041;
        else if (pickedFleensFound == 3)
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
   when the next turn (0) ends; at the end (-1), sets walkerStep3Due. */
/* @zoombi32 0x00423d9d */
void fleensWalkerNotifyC(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        break;
    case -1:
        walkerStep3Due = 1;
        break;
    }
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at the end (-1), sets walkerStep9Due. */
/* @zoombi32 0x00423e2c */
void fleensWalkerNotifyA(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        break;
    case -1:
        walkerStep9Due = 1;
        break;
    }
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at the end (-1), stops its script. */
/* @zoombi32 0x00424104 */
void fleensWalkerStopNotify(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        break;
    case -1:
        view->body.running = 0;
        break;
    }
}

/* A Zoombini's notify: 250-253 face that way, 240-243 set the facing for
   when the next turn (0) ends; at 131, sets lineMoveDue with lineLength; at
   the end (-1), moves one from snoidsOnTheirWay to snoidsArrived. */
/* @zoombi32 0x0042403b */
void fleensMovingOnNotify(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        break;
    case 131:
        if (lineLength)
            lineMoveDue = 1;
        break;
    case -1:
        if (snoidsOnTheirWay) {
            snoidsOnTheirWay--;
            snoidsArrived++;
        }
        break;
    }
}

/* Resets scene 13's state; the pace fleensFidgetInterval by fidgetPaceFlag. */
/* @zoombi32 0x0042160c */
void resetFleens()
{
    short i;

    hintSound = 0;
    fleenScriptsToLoad = fleensFidgetsAllowed = 0;
    for (i = 0; i < 16; i++)
        fleenViews[i] = fleenClicked[i] = 0;
    for (i = 0; i < 7; i++) {
        lineSnoids[i] = 0;
        lineFleens[i] = 0;
    }
    fleensFidgets = lineLength = walkerSnoid = walkerFleen = activeSnoid = 0;
    placeSnapRadius = 100;
    pickedFleensFound = sceneDue = putDownFleen = 0;
    lineMoveDue = 0;
    walkerStep4Due = walkerStep9Due = walkerStep3Due = 0;
    lineStepDue = fleensEntered = 0;
    fleenBehindDue = snoidBehindDue = leaderBusy = 0;
    leaderWalking = 0;
    pendingFleensFacing = fleensView0Started = 0;
    lastFleensFidgetTime = 0;
    fleensFidgetersUsed = 0;
    if (fidgetPaceFlag)
        fleensFidgetInterval = 120;
    else
        fleensFidgetInterval = 60;
    snoidsOnTheirWay = snoidsArrived = 0;
}

/* The script (4000 on) for a Zoombini (by its feet and its place,
   pathDirection) doing `which` (1-14). */
/* @zoombi32 0x0042339f */
short fleenScript(View *view, short which)
{
    short script = 0;
    Snoid *snoid = viewSnoid(view);
    short feet = snoid->features[3];
    short spot = snoid->pathDirection;

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

/* A notify for the fleens' views: 0 turns (and puts activeFleen back after
   fleensViews[2] the first time), 1 starts fleensViews[0] once, 2 and 140 move
   activeFleen after or before activeSnoid, 218 a random sound (4100-4124); 137
   and the end (-1) stop the script. */
/* @zoombi32 0x00423512 */
void fleensViewNotify(View *view, short event)
{
    Snoid *snoid;

    switch (event) {
    case 0:
        if (fleenBehindDue && view->id == activeFleen) {
            fleenBehindDue = 0;
            moveView(activeFleen, 0, fleensViews[2]);
        }
        snoid = viewSnoid(view);
        snoid->facingLeft = !snoid->facingLeft;
        break;
    case 1:
        if (!fleensView0Started) {
            moveView(activeFleen, 0, fleensViews[0]);
            fleensView0Started = 1;
            startView(fleensViews[0], 0, fleensStartNotify, 1);
        }
        break;
    case 2:
        moveView(activeFleen, 1, activeSnoid);
        break;
    case 137:
        view->body.running = 0;
        break;
    case 140:
        moveView(activeFleen, 0, activeSnoid);
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
 * layers, set up in layers when it changes (drawnFacing); the rest is as
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
    layers = snoid->layers;
    last = 5;
    script = fleenScripts[snoid->body.script];
    word = script[1];
    at = script + snoid->body.frameOffset;
    if (word != snoid->drawnFacing) {
        snoid->drawnFacing = word;
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
    layers = snoid->layers + 1;
    offsetY = -snoid->body.waypointY;
    if (*at > 0) {
        offsetX = -snoid->body.waypointX;
        snoid->body.x = at[1] + offsetX;
        snoid->body.y = at[2] + offsetY;
    }
    i = 0;
    if (!snoid->facingLeft) {
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
 * `id` (4000 on: its own scripts, action 1; others as 0), placed so that
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

    snoid->drawnFacing = -1;
    if (snoid->body.group) {
        if (groupLeader[snoid->body.group] == view->id)
            groupLeader[snoid->body.group] = 0;
        groupFlagsB[snoid->body.group] = 0;
    }
    snoid->body.group = 0;
    if (id >= 4000) {
        snoid->action = 1;
        index = id - 4000;
    } else {
        snoid->action = 0;
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
    if (!snoid->action)
        snoid->pose = 1;
    else
        snoid->pose = 0;
    if (removedRgn)
        unionRgnRect(removedRgn, &snoid->body.bounds);
    originX = snoid->body.x;
    originY = snoid->body.y;
    if (snoid->action == 1) {
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
        snoid->body.waypointX = data[1] - originX;
        snoid->body.waypointY = data[2] - originY;
    }
    layOutFleen(snoid, 0);
    if (removedRgn)
        unionRgnRect(removedRgn, &snoid->body.bounds);
}

/* Starts the lineLength Zoombinis of lineSnoids moving on (7021), the last
   one (8) going ahead of fleensViews[2], counted in snoidsOnTheirWay until it's done
   (fleensMovingOnNotify); then one fewer. */
/* @zoombi32 0x00423f84 */
void moveFleenZoombinisOn()
{
    short i;
    View *view;
    short script;

    for (i = 0; i < lineLength; i++) {
        view = findView(lineSnoids[i]);
        if (view) {
            if (i == lineLength - 1) {
                script = fleensSnoidScript(view, 8);
                if (script) {
                    snoidsOnTheirWay++;
                    moveView(view->id, 0, fleensViews[2]);
                    startSnoidScript(viewSnoid(view), script, 0, 0);
                    view->notifyEnd = 1;
                    view->notify = fleensMovingOnNotify;
                }
            } else {
                script = fleensSnoidScript(view, 7021);
                if (script)
                    startSnoidScript(viewSnoid(view), script, 0, 0);
            }
        }
    }
    if (lineLength)
        lineLength--;
}

/*
 * The fleens' updateSnoidView, for a fleen: when due,
 * idles (now and then, by snoidIdleDelay, fidgeting with 2 or 3) or runs its
 * script a frame; at the script's end, back to 4000 and tells the notify
 * (-1).
 */
/* @zoombi32 0x004225cf */
void updateFleen(View *view, short region)
{
    short event;
    short changed = 0;
    Snoid *snoid;

    if (!view->body.running || dialogFlags)
        return;
    {
        short due = view->nextUpdate <= updateTime;

        if (!due)
            return;
    }
    view->nextUpdate = updateTime + view->interval;
    snoid = viewSnoid(view);
    switch (snoid->action) {
    case 0:
    default:
        if (snoid->pose) {
            snoid->pose = 0;
            changed = 1;
        } else if (snoidIdleDelay && snoid->idleTicks++ > snoidIdleDelay + 16) {
            short which = randomBetween(1, 100) <= 50 ? 2 : 3;
            short script = fleenScript(view, which);

            if (script) {
                startFleenScript(view, script, 0);
                changed = 1;
                snoid->idleTicks = 1;
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
            snoid->layers[i] = 0;
        snoid->drawnFacing = -1;
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
   when the next turn (0) ends; at 60, starts walkerFleen on 13; at the end
   (-1), sets walkerStep4Due. */
/* @zoombi32 0x00423ebb */
void fleensWalkerNotifyE(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        break;
    case 60:
        other = findView(walkerFleen);
        if (other) {
            short script = fleenScript(other, 13);

            startFleenScript(other, script, 0);
        }
        break;
    case -1:
        walkerStep4Due = 1;
        break;
    }
}

/* The level's rules for making fleens, in the game's state: 0-3 shift
   each feature's value (1-5), 4-7 (from level 2) move each feature to
   another's place (from 1; 0: stays). */
inline char *fleenRules()
{
    return gameState + 0xc;
}

/*
 * Makes the fleens, one for each traveller (fleensTravellers of them), with its
 * features changed by the level's rules: picks up to three to stand apart
 * (pickedFleens, placed by table 5000), the others by table
 * 5001; each gets its features changed by the level's rules (new rules on
 * a new game, fleensLevel 1 or 3), and the views are stacked in order.
 */
/* Not exact: BCC32 keeps gameState's address in edi here (dropping any one
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

    fleensTravellers = countPresentTravellers();
    if (!fleensTravellers)
        return;
    flag = 0;
    count = 0;
    for (i = 0; i < 18; i++)
        views[i] = 0;
    b = a = 0;
    pickedPlaces = loadShortTable(5000, &pickedResource);
    otherPlaces = loadShortTable(5001, &otherResource);
    pickedFleens[0] = randomBetween(1, fleensTravellers);
    switch (fleensTravellers) {
    case 1:
        pickedFleensFound = 2;
        fleensView0Started = 1;
        break;
    case 2:
        pickedFleensFound = 1;
        break;
    }
    if (fleensTravellers >= 2)
        for (pickedFleens[1] = pickedFleens[0]; pickedFleens[1] == pickedFleens[0];)
            pickedFleens[1] = randomBetween(1, fleensTravellers);
    if (fleensTravellers >= 3)
        for (pickedFleens[2] = pickedFleens[0]; pickedFleens[2] == pickedFleens[0] || pickedFleens[2] == pickedFleens[1];)
            pickedFleens[2] = randomBetween(1, fleensTravellers);
    if (!gameState[0xc] || fleensLevel == 1 || fleensLevel == 3)
        for (j = 0; j < 4; j++)
            gameState[0xc + j] = randomBetween(1, 5);
    if (fleensLevel > 1) {
        if (!gameState[0x10] || fleensLevel == 3) {
            gameState[0x10] = randomBetween(2, 4);
            used = 1 << (gameState[0x10] - 1);
            for (j = 5; j < 8; j++)
                gameState[0xc + j] = swapFeatures[allocateSlot(&used, 4, 0)];
        }
    } else {
        for (j = 4; j < 8; j++)
            gameState[0xc + j] = 0;
    }
    for (i = 0; i < fleensTravellers; i++) {
        if (!gameState[i * 19 + 0xa93c])
            continue;
        for (j = 0; j < 4; j++) {
            char value = ((gameState + i * 19)[j + 0xa934] + gameState[0xc + j] - 2) % 5 + 1;

            if (gameState[0xc + 4 + j])
                snoid.features[gameState[0xc + 4 + j] - 1] = value;
            else
                snoid.features[j] = value;
        }
        snoid.angle = 0;
        snoid.facingLeft = 0;
        if (i + 1 == pickedFleens[0] || i + 1 == pickedFleens[1] || i + 1 == pickedFleens[2]) {
            if (*pickedPlaces > a) {
                snoid.pathDirection = a + *otherPlaces;
                snoid.body.x = pickedPlaces[a * 2 + 1];
                snoid.body.y = pickedPlaces[a * 2 + 2];
                a++;
            }
        } else if (*otherPlaces > b) {
            snoid.pathDirection = b;
            snoid.body.x = otherPlaces[b * 2 + 1];
            snoid.body.y = otherPlaces[b * 2 + 2];
            b++;
            flag = 1;
        }
        for (j = 0; j < 10; j++)
            snoid.name[j] = (gameState + i * 19)[j + 0xa93d];
        snoid.home = *(Point *)&snoid.body.x;
        *(Point *)&snoid.body.waypointX = *(Point *)&snoid.body.x;
        *(Point *)&snoid.targetX = *(Point *)&snoid.body.x;
        snoid.pathIndex = 0;
        snoid.path = 0;
        snoid.stepX = 0;
        snoid.stepY = 0;
        snoid.idleTicks = randomBetween(0, 80);
        snoid.chosen = 1;
        fleenViews[i] = addFleen(&snoid);
        fleenClicked[i] = 0;
        if (flag) {
            flag = 0;
            views[count] = fleenViews[i];
            count++;
        }
    }
    freeResource(&pickedResource);
    freeResource(&otherResource);
    moveView(views[2], 0, views[1]);
    moveView(views[5], 0, views[4]);
    moveView(views[8], 1, views[7]);
    moveView(views[9], 1, views[8]);
    moveView(views[10], 0, views[8]);
    moveView(views[11], 0, views[10]);
}

/* Scene 13's clicks: 1 leaves (asking whether to keep the party), 2 sends
   the Zoombinis on (once ready, fleensGoReady and fleensEntered), counted in
   snoidsOnTheirWay, 3 (while nothing's moving) drags a Zoombini: one placed
   (chosen) freely, another only when its fleen (fleenViews) is idle, noting where it was put (putDownSnoid,
   putDownFleen) or sending it back to a free place; with practiceLevel, a click on a
   fleen makes its Zoombini jump. */
/* @zoombi32 0x00422192 */
void fleensClicked(short which)
{
    Point where;
    Snoid *snoid;
    View *view;
    short id;
    short fleen;
    short i;
    View *other;
    short moved;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeFleens();
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawFleensButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFleensButton(which, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (!fleensGoReady || !fleensEntered)
            break;
        queueViewSound(996, 0);
        drawFleensButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawFleensButton(which, 0, 1);
        chooseSnoids(1, 0);
        lineMoveDue = 1;
        snoidsArrived = 0;
        snoidsOnTheirWay = lineLength;
        sceneDue = 14;
        break;
    case 3:
        if (leaderWalking || snoidsOnTheirWay > 0 || pickedFleensFound >= 3)
            break;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (view)
            id = viewSnoid(view)->action;
        if (view && (!id || id == 6)) {
            snoid = viewSnoid(view);
            if (viewSnoid(view)->chosen) {
                if (!leaderBusy) {
                    dragInPlace = 1;
                    dragSnoid(view, where, 0, 0);
                }
            } else {
                id = view->id;
                for (i = 0; i < fleensPartySize; i++)
                    if (id == partyViews[i]) {
                        fleenClicked[i] = 1;
                        fleen = fleenViews[i];
                        i = fleensPartySize;
                    }
                other = findView(fleen);
                if (other && viewSnoid(other)->chosen) {
                    if (id == putDownSnoid)
                        putDownSnoid = putDownFleen = 0;
                    moved = dragSnoid(view, where, 0, 0);
                    if (heldPlaceNumber()) {
                        putDownSnoid = id;
                        putDownFleen = fleen;
                    } else if (moved) {
                        if (snoid->body.x != snoid->targetX || snoid->body.y != snoid->targetY)
                            pickFreePlace((Point *)&snoid->targetX, viewPlaces, 16, 500);
                    }
                }
            }
        }
        if (practiceLevel) {
            view = viewAt(where, 2, 1);
            if (view) {
                moved = view->id;
                for (i = 0; i < fleensPartySize; i++)
                    if (moved == fleenViews[i]) {
                        other = idleSnoidView(partyViews[i]);
                        if (other) {
                            viewSnoid(other)->pose = 15;
                            setSnoidAction(viewSnoid(other), 3, 0);
                        }
                        i = fleensPartySize;
                    }
            }
        }
        break;
    }
}

/*
 * activeFleen's notify: besides turning (250-253, 240-243, 0), moves
 * views into order (0, 8, 9), sends the picked fleens on or back by
 * pickedFleensFound when activeFleen is in place (6), moves the Zoombinis in line along
 * (132), stops (137), sets the fleens in a range of places off (133-135),
 * adds a random extra (30), and has activeFleen act (4, 5, 7: a fleen script,
 * maybe placed at an anchor).
 */
/* Not exact: in case 30 the original calls randomBetween before pushing
   addView's first three arguments (as for a temporary in eax); a
   temporary here lands in ebx, and the call inline is evaluated in order. */
/* @zoombi32 0x0042365a */
void fleensLeaderNotify(View *view, short event)
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
        pendingFleensFacing = event - 239;
        break;
    case 0:
        snoid->facingLeft = !snoid->facingLeft;
        if (pendingFleensFacing) {
            setSnoidFacing(snoid, pendingFleensFacing - 1);
            pendingFleensFacing = 0;
        }
        if (snoidBehindDue && view->id == activeSnoid) {
            snoidBehindDue = 0;
            view->flags |= 0x4000000;
            moveView(activeSnoid, 0, fleensViews[2]);
        }
        break;
    case 4:
        snoidBehindDue = 1;
        actor = findView(activeFleen);
        if (actor) {
            spot = viewSnoid(actor)->pathDirection;
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
        actor = findView(activeFleen);
        if (!actor)
            break;
        spot = viewSnoid(actor)->pathDirection;
        if (spot < 17 || spot > 19)
            break;
        other = findView(view1000);
        if (!other)
            break;
        if (pickedFleensFound < 1 || pickedFleensFound > 3)
            break;
        {
            short id = pickedFleensFound + 1000;

            setViewScript(other, id, 1);
        }
        for (i = 0; i < 3; i++)
            if (pickedFleens[i] && fleenViews[pickedFleens[i] - 1] == activeFleen)
                pickedFleens[i] = 0;
        for (i = 0; i < 3; i++)
            if (pickedFleens[i]) {
                other = findView(fleenViews[pickedFleens[i] - 1]);
                if (other && pickedFleensFound < 3) {
                    script = fleenScript(other, pickedFleensFound + 6);
                    if (script) {
                        startFleenScript(other, script, 0);
                        other->notify = fleensViewNotify;
                    }
                }
            }
        fleensGoReady = pickedFleensFound == 3;
        break;
    case 7:
        at.x = 214;
        at.y = 207;
        if (!fleensView0Started) {
            which = 9;
            anchor = &at;
        } else if (pickedFleensFound == 3) {
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
        fleenBehindDue = 1;
        break;
    case 8:
        view->flags |= 0x4000000;
        moveView(view->id, 0, fleensViews[0]);
        break;
    case 132:
        if (!lineStepDue)
            break;
        lineStepDue = 0;
        seven = lineLength == 7;
        for (i = 0; i < lineLength - 1; i++) {
            other = findView(lineSnoids[i]);
            if (other) {
                script = fleensSnoidScript(other, 7016);
                if (script)
                    startSnoidScript(viewSnoid(other), script, 0, 0);
                if (!i && seven) {
                    other->notifyEnd = 1;
                    other->notify = fleensWalkerNotifyC;
                }
            }
            other = findView(lineFleens[i]);
            if (other) {
                script = fleenScript(other, 12);
                if (script)
                    startFleenScript(other, script, 0);
            }
        }
        if (seven) {
            walkerSnoid = lineSnoids[0];
            walkerFleen = lineFleens[0];
            for (i = 1; i < lineLength; i++) {
                lineSnoids[i - 1] = lineSnoids[i];
                lineFleens[i - 1] = lineFleens[i];
            }
            lineLength--;
        }
        break;
    case 9:
        moveView(view->id, 0, activeFleen);
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
            if (fleenClicked[i] < 2) {
                other = findView(fleenViews[i]);
                if (other && viewSnoid(other)->pathDirection >= low && viewSnoid(other)->pathDirection <= high) {
                    fleenClicked[i] = 2;
                    script = fleenScript(other, 14);
                    if (script) {
                        startFleenScript(other, script, 0);
                        other->notifyEnd = 1;
                        if (event != 135)
                            other->notify = fleensViewNotify;
                    }
                }
            }
        break;
    case 28:
        fleensLeaderNotify(view, 8);
        break;
    case 30:
        other = findView(addView(0x100000, drawCels, runViewScript, randomBetween(0, 2) + 1004, 6, 0, 0, 0));
        if (other) {
            other->flags |= 0x1000;
            setViewScript(other, 0, 1);
            other->notify = fleensExtraNotify;
            other->notifyEnd = 1;
        }
        break;
    case -1:
        snoidIdleDelay = 64;
        leaderWalking = 0;
        leaderBusy = 0;
        activeFleen = activeSnoid = 0;
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
            actor = findView(activeFleen);
        if (actor) {
            viewSnoid(actor)->chosen = 0;
            script = fleenScript(actor, which);
            if (script) {
                startFleenScript(actor, script, anchor);
                actor->notify = fleensViewNotify;
                if (seat)
                    viewSnoid(actor)->pathDirection = 20;
            }
        }
    }
}

/* The extra's notify (fleensLeaderNotify's 30): 135 passes on to fleensLeaderNotify; at the
   end (-1), frees the views, sets fleensEntered, picks the Zoombinis, plays a
   cheer when all made it (20055-20063) or now and then a remark
   (20045-20048), deletes fleensViews[3] and sets fleensFidgetsAllowed by how many are
   chosen. */
/* @zoombi32 0x00424195 */
void fleensExtraNotify(View *view, short event)
{
    short n;

    switch (event) {
    case 135:
        fleensLeaderNotify(view, event);
        break;
    case -1:
        setViewsLocked(0);
        fleensEntered = 1;
        chooseSnoids(1, 0);
        n = countChosenSnoids();
        if (n) {
            if (n == fleensPartySize)
                queueViewSound(randomBetween(20055, 20063), 0);
            else if (randomBetween(0, 4) > fleensLevel || (*(short *)(gameState + 0x38) & 0xfff) <= 3)
                queueViewSound(randomBetween(20045, 20048), 1);
        }
        deleteView(fleensViews[3]);
        n = countChosenSnoids();
        if (n == 16)
            fleensFidgetsAllowed = 13;
        else if (n > 8)
            fleensFidgetsAllowed = n - 8;
        break;
    }
}

/* Opens scene 13: Fleens.MHK, its sounds, images and scripts, the views
   (the backdrop's seven, view1000, the placed spot, the buttons), the
   fleens (addFleens), the party; the first Zoombini walks in (activeFleen's
   notify), and a hint or greeting. */
/* @zoombi32 0x00421738 */
void openFleens()
{
    Point places[16] = {{238, 368}, {185, 417}, {155, 448}, {197, 396}, {160, 357}, {164, 384},
                        {150, 416}, {116, 357}, {130, 386}, {109, 418}, {117, 448}, {74, 348},
                        {89, 384}, {67, 418}, {76, 450}, {56, 379}};
    Point place = {438, 357};
    short i;
    unsigned long flags;
    View *view;
    short script;

    fleensOpen = fleensGoReady = 0;
    resetFleens();
    fleensLevel = sceneLevel();
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
    openGameFile(&fleensFile, "Fleens.MHK");
    setCurrentMap(fleensFile);
    drawBackdrop(300);
    loadTerrain(500);
    fleenImages = loadImageBank(4000, &fleenImagesResource);
    fleensButtonImages = loadImageBank(400, &fleensButtonResource);
    loadFeatureGroup(1000, 0, 0);
    loadFeatureGroup(1100, 1, 0);
    loadFeatureGroup(1200, 2, 0);
    fleenHotX = loadShortTable(4000, &fleenHotXResource);
    fleenHotY = loadShortTable(4001, &fleenHotYResource);
    loadScripts(1000, 7);
    addScripts(1100, 1, 0);
    addScripts(1200, 7, 0);
    loadFleenScripts();
    loadSnoidScripts(6000, 5, 0);
    addSnoidScripts(7000, 46, 26);
    view1000 = addView(0x108000, drawCels, runViewScript, 1000, 6, 0, 0, 0);
    placedViews[0] = addView(0x108a000, drawCels, runViewScript, 1100, 7, &place, 0, 0);
    addView(0x1000, drawFleensButtons, updateFleensButtons, 0, 0, 0, 0, 0);
    setViewPlaces(16, places, 1);
    addFleens();
    for (i = 1200; i <= 1206; i++) {
        if (i == 1200)
            flags = 0x4180000;
        else
            flags = 0x4000000;
        fleensViews[i - 1200] = addView(flags, drawCels, runViewScript, i, 6, 0, 0, 0);
    }
    copyPaletteRange(10, 236);
    makePartySnoids(0);
    enterSnoids(0);
    updateViews();
    staggerSnoids(45, 200);
    view = findView(partyViews[0]);
    if (view) {
        script = fleensSnoidScript(view, 1);
        if (script) {
            Point anchor = {236, 395};

            leaderWalking = 1;
            loadSnoidScript(script);
            startSnoidScript(viewSnoid(view), script, &anchor, 0);
            view->nextUpdate = 0;
            view->notify = fleensLeaderNotify;
            view->notifyEnd = 1;
            if (snoidsOnTheirWay > 0)
                snoidsOnTheirWay--;
        }
    }
    updateViews();
    setGroupLists(fleensGroups, 1, (short)0xc000);
    drawFleensButton(1, 0, 0);
    drawFleensButton(2, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    fleensOpen = 1;
    switch (campHint((short *)(gameState + 0x38))) {
    case 2:
        hintSound = 20080;
        break;
    default:
        if (fleensLevel == 1 || fleensLevel == 3)
            hintSound = randomBetween(20079, 20080);
        else
            hintSound = 20079;
        break;
    }
    fleensPartySize = countSnoidViews();
    fleenScriptsToLoad = 8;
}

/*
 * Scene 13's frame: leaves when asked (once sound 996 ends and the moving
 * Zoombinis are done); starts a Zoombini put at a place (putDownSnoid,
 * putDownFleen) walking up to its fleen, counting it in lineSnoids/lineFleens and
 * in pickedFleensFound if the fleen was one picked; or now and then (fleensFidgetInterval)
 * sends a waiting Zoombini on (5), up to fleensFidgetsAllowed; starts walkerSnoid on the
 * script its notifies asked for (3, 9, 4); moves the line on (moveFleenZoombinisOn);
 * and loads the scripts 4051-4058 one a frame.
 */
/* @zoombi32 0x00421d1e */
void fleensFrame()
{
    View *view;
    short script;
    short done;
    short i;

    if (inFleensFrame || !fleensOpen)
        return;
    inFleensFrame = 1;
    updateViews();
    if (sceneDue && !isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeFleens();
                inFleensFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (!activeSnoid) {
        if (putDownFleen) {
            view = idleSnoidView(putDownSnoid);
            if (view && !dragInPlace) {
                viewSnoid(view)->chosen = 1;
                activeFleen = putDownFleen;
                putDownFleen = 0;
                activeSnoid = view->id;
                snoidIdleDelay = 0;
                leaderBusy = 1;
                unloadSounds();
                for (i = 0; i < 3; i++)
                    if (pickedFleens[i] && fleenViews[pickedFleens[i] - 1] == activeFleen)
                        pickedFleensFound++;
                lineStepDue = 1;
                lineSnoids[lineLength] = putDownSnoid;
                lineFleens[lineLength] = activeFleen;
                lineLength++;
                if (pickedFleensFound > 2)
                    endDragNow = 1;
                script = fleensSnoidScript(view, 2);
                if (script) {
                    if (!fleensView0Started)
                        loadSnoidScript(script);
                    startSnoidScript(viewSnoid(view), script, 0, 0);
                    view->notify = fleensLeaderNotify;
                    view->notifyEnd = 1;
                }
            }
        } else if (fleensEntered && fleensFidgets < fleensFidgetsAllowed && clockTime() - lastFleensFidgetTime > fleensFidgetInterval) {
            done = 0;
            lastFleensFidgetTime = clockTime();
            activeSnoid = 1;
            do {
                view = idleSnoidView(partyViews[allocateSlot(&fleensFidgetersUsed, fleensPartySize, 0)]);
                if (view && view->body.running && view->flags == 1) {
                    script = fleensSnoidScript(view, 5);
                    if (script) {
                        if (view->body.x <= 270)
                            startSnoidScript(viewSnoid(view), script, 0, 0);
                        fleensFidgets++;
                        done = 1;
                    }
                }
            } while (!done);
            activeSnoid = 0;
        }
    }
    if (walkerStep3Due) {
        walkerStep3Due = 0;
        view = findView(walkerSnoid);
        if (view) {
            script = fleensSnoidScript(view, 3);
            startSnoidScript(viewSnoid(view), script, 0, 0);
            view->notify = fleensWalkerNotifyA;
            view->flags |= 0x4000000;
            for (i = 0; i < fleensPartySize; i++)
                if (partyViews[i] == walkerSnoid) {
                    moveView(walkerSnoid, 1, fleenViews[i]);
                    i = fleensPartySize;
                }
        }
    } else if (walkerStep9Due) {
        walkerStep9Due = 0;
        view = findView(walkerSnoid);
        if (view) {
            script = fleensSnoidScript(view, 9);
            startSnoidScript(viewSnoid(view), script, 0, 0);
            view->notify = fleensWalkerNotifyE;
        }
    } else if (walkerStep4Due) {
        walkerStep4Due = 0;
        view = findView(walkerSnoid);
        if (view) {
            script = fleensSnoidScript(view, 4);
            viewSnoid(view)->facingLeft = 1;
            startSnoidScript(viewSnoid(view), script, 0, 0);
            view->notifyEnd = 1;
            view->notify = fleensWalkerStopNotify;
        }
    }
    if (lineMoveDue) {
        lineMoveDue = 0;
        moveFleenZoombinisOn();
    }
    if (fleenScriptsToLoad) {
        loadFleenScript(8 - fleenScriptsToLoad + 4051);
        fleenScriptsToLoad--;
    }
    playAmbientSound();
    inFleensFrame = 0;
}
