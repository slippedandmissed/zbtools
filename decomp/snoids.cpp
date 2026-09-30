/*
 * snoids (0x456c00-0x45c0f4): 'Too many snoid NORMAL scripts', 'Zoombini.MHK', name syllables
 */

#include <stdlib.h>
#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "features.h"
#include "graphics.h"
#include "loading.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* @zoombi32 0x00456c00 */
void resetSnoids()
{
    noPaths = levelJustRaised = 0;
    arrivalHook = 0;
    practiceLevel = 0;
    skipJourneyMap = hideArrivedPlaced = dragInProgress = 0;
    keepDragPose = 0;
    claimOnArrival = 1;
    placeSnapRadius = 15;
    placesClaimable = 1;
    setSnoidMode(1);
    dragInPlace = 0;
}

/* Opens the sound files (with `files`), else loads the Zoombinis' images,
   scripts and hotspot tables. */
/* @zoombi32 0x00456c67 */
void loadSnoids(short files)
{
    if (files) {
        openGameFile(&midiMapFile, "MidiMPC.MHK");
        openGameFile(&soundsMap, "Zoombini.MHK");
        setCurrentMap(soundsMap);
    } else {
        snoidImages = loadImageBank(3000, &snoidImagesResource);
        snoidImages2 = loadImageBank(3100, &snoidImages2Resource);
        if (!snoidTablesLoaded) {
            loadBaseSnoidScripts();
            snoidTables[0] = loadShortTable(100, &snoidTableResources[0]);
            snoidTables[1] = loadShortTable(101, &snoidTableResources[1]);
            snoidTables[2] = loadShortTable(102, &snoidTableResources[2]);
            snoidTables[3] = loadShortTable(103, &snoidTableResources[3]);
            snoidTablesLoaded = 1;
        }
        snoidImages3 = loadImageBank(3001, &snoidImages3Resource);
    }
}

/* @zoombi32 0x00456d3b */
void closeSnoids()
{
    short saved = setFreeAtOnce(1);

    if (gameState) {
        disposePtr(gameState);
        gameState = 0;
    }
    if (snoidTablesLoaded)
        snoidTablesLoaded = 0;
    useAltSnoids(1);
    freeBaseSnoidScripts();
    freeSnoidTables();
    freeResource(&snoidImagesResource);
    freeResource(&snoidImages2Resource);
    freeResource(&snoidImages3Resource);
    setFreeAtOnce(saved);
    closeGameFile(&soundsMap);
    closeGameFile(&midiMapFile);
}

/* Loads a table of big-endian words ('REGS'), swapping them. */
/* @zoombi32 0x00456dbe */
short *loadShortTable(short id, long *resource)
{
    short handle;
    short *at;
    short *data;

    *resource = 0;
    loadResourceAs(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    handle = usedResourceHandle(*resource);
    at = (short *)lockHandleAlias(handle);
    data = at;
    for (unsigned long size = handleSize(handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
    return data;
}

/* @zoombi32 0x00456e2e */
void freeSnoidTables()
{
    for (short i = 0; i < 4; i++)
        freeResource(&snoidTableResources[i]);
}

/* How many Zoombinis' views run with chosen set. */
/* @zoombi32 0x00456e4c */
short countChosenSnoids()
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if ((view->flags & 1) && view->body.running && viewSnoid(view)->chosen)
            count++;
    return count;
}

/* How many Zoombinis' views there are. */
/* @zoombi32 0x00456e7f */
short countSnoidViews()
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if (view->flags & 1)
            count++;
    return count;
}

/* Loads the Zoombinis' 51 base scripts ('SCRS' 100 on). */
/* @zoombi32 0x00456e9f */
void loadBaseSnoidScripts()
{
    for (short i = 0; i < 51; i++) {
        baseSnoidScriptResources[i] = 0;
        baseSnoidScripts[i] =
            loadSwappedResource(&baseSnoidScriptResources[i], i + 100, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    }
}

/* @zoombi32 0x00456edb */
void freeBaseSnoidScripts()
{
    for (short i = 0; i < 51; i++)
        freeResource(&baseSnoidScriptResources[i]);
}

/* Loads `count` Zoombini scripts from id `first` as the first group,
   loading `limit` of them (all if not 1 to count). */
/* @zoombi32 0x00456ef9 */
void loadSnoidScripts(short first, short count, short limit)
{
    short i;

    if (count > 110)
        fatalError("Too many snoid REJECT scripts");
    snoidScriptGroups = 0;
    for (i = 0; i < 2; i++) {
        snoidScriptGroupFirst[i] = 0;
        snoidScriptGroupCount[i] = 0;
    }
    for (i = 0; i < 110; i++) {
        snoidScripts[i] = 0;
        snoidScriptResources[i] = 0;
    }
    if (limit <= 0 || limit > count)
        limit = count;
    for (i = 0; i < limit && i < 110; i++)
        snoidScripts[i] =
            loadSwappedResource(&snoidScriptResources[i], first + i, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    snoidScriptGroupFirst[snoidScriptGroups] = first;
    snoidScriptGroupCount[snoidScriptGroups] = count;
    snoidScriptGroups++;
}

/* @zoombi32 0x00456fd8 */
void addSnoidScripts(short first, short count, short limit)
{
    short i;
    short loaded;

    if (snoidScriptGroups < 2) {
        loaded = 0;
        for (i = 0; i < snoidScriptGroups; i++)
            loaded += snoidScriptGroupCount[i];
        if (loaded) {
            if (count + loaded > 110)
                fatalError("Too many snoid NORMAL scripts");
            if (limit <= 0 || limit > count)
                limit = count;
            for (i = loaded; i - loaded < limit && i < 110; i++)
                snoidScripts[i] = loadSwappedResource(&snoidScriptResources[i], first + i - loaded,
                                                      RESOURCE_TYPE('S', 'C', 'R', 'S'));
            snoidScriptGroupFirst[snoidScriptGroups] = first;
            snoidScriptGroupCount[snoidScriptGroups] = count;
            snoidScriptGroups++;
        }
    }
}

/* A Zoombini script id's index in snoidScripts (-1 if none) and group. */
/* @zoombi32 0x004570b5 */
void findSnoidScript(short id, short *group, short *index)
{
    short loaded;
    short i;

    *index = -1;
    loaded = 0;
    for (i = 0; i < snoidScriptGroups; i++) {
        short first = snoidScriptGroupFirst[i];
        short last = snoidScriptGroupCount[i] + first - 1;

        if (id >= first && id <= last) {
            *group = i;
            *index = loaded += id - first;
            return;
        }
        loaded += snoidScriptGroupCount[i];
    }
}

/* Loads a Zoombini script by id, if it isn't loaded. */
/* @zoombi32 0x00457120 */
void loadSnoidScript(short id)
{
    short loaded;
    short i;
    short index;

    loaded = 0;
    for (i = 0; i < snoidScriptGroups; i++) {
        short first = snoidScriptGroupFirst[i];
        short last = snoidScriptGroupCount[i] + first - 1;

        if (id >= first && id <= last) {
            index = id - first + loaded;
            if (!snoidScripts[index])
                snoidScripts[index] =
                    loadSwappedResource(&snoidScriptResources[index], id, RESOURCE_TYPE('S', 'C', 'R', 'S'));
            return;
        }
        loaded += snoidScriptGroupCount[i];
    }
}

/* @zoombi32 0x004571a8 */
void freeSnoidScripts()
{
    for (short i = 0; i < 110; i++)
        freeResource(&snoidScriptResources[i]);
    snoidScriptGroups = 0;
    noPaths = 0;
}

/* Draws a Zoombini's cels. */
/* Functional: the original reads each cel's words as it pushes them. */
/* @zoombi32-functional 0x004571d8 */
void drawSnoid(Snoid *snoid)
{
    short *cel = (short *)snoid->body.cels;

    while (*cel && *cel <= snoidImages->count) {
        unsigned short *image = (unsigned short *)(snoidImages->offsets[*cel++] + (char *)snoidImages);
        short x = *cel++;
        short y = *cel++;

        drawImageData(image, x, y, 8);
    }
}

/* Adds a view for a Zoombini at a place, heading for another, first
   updated at `when`; returns its id. */
/* @zoombi32 0x00457221 */
short placeSnoid(Snoid *snoid, unsigned long when, short x, short y, short targetX, short targetY)
{
    Point saved = *(Point *)&snoid->body.x;
    short id;

    snoid->angle = 1;
    snoid->facingLeft = 0;
    snoid->body.x = x;
    snoid->body.y = y;
    snoid->targetX = targetX;
    snoid->targetY = targetY;
    snoid->body.cels[0].image = 0;
    snoid->body.celsEnd = 0;
    snoid->idleTicks = randomBetween(0, 0x40);
    id = addSnoidView(snoid, 1);
    {
        View *view = findView(id);

        if (view)
            view->nextUpdate = when;
    }
    snoid->angle = 0;
    *(Point *)&snoid->body.x = saved;
    return id;
}

/* Adds a view for a Zoombini (if it has one); returns its id. */
/* @zoombi32 0x004574ae */
short addSnoidView(Snoid *snoid, short placed)
{
    short id = 0;

    if (snoid->features[3]) {
        for (short i = 0; i < 16; i++)
            snoid->layers[i] = 0;
        id = addView(1, drawSnoidView, updateSnoidView, 0, 6, snoid, 0, 0);
        {
            View *view = findView(id);

            if (view) {
                short action;

                if (!placed)
                    action = 0;
                else
                    action = 7;
                setSnoidAction(viewSnoid(view), action, 0);
                view->nextUpdate = 0;
            }
        }
    }
    return id;
}

/* A Zoombini view's drawing: its cels (from the second bank with
   action 9), clipped to its clip rectangle if it has one. */
/* Functional: as drawCels. */
/* @zoombi32-functional 0x00457527 */
void drawSnoidView(View *view)
{
    if (view->body.running) {
        if (view->body.clipped) {
            copyRgn(featureClipRgn, currentViewRgn);
            sectRgnWithRect(featureClipRgn, &view->body.clip);
            setClip(featureClipRgn);
        }
        short *cel = (short *)view->body.cels;
        ImageBank *bank = snoidImages;

        if (viewSnoid(view)->action == 9)
            bank = snoidImages2;
        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
        if (view->body.clipped)
            setClip(currentViewRgn);
    }
}

/* @zoombi32 0x004572bf */
short countPresentTravellers()
{
    int count = 0;
    for (short i = 0; i < *(short *)(gameState + 0xa92e); i++)
        if (*(gameState + 0xa93c + i * 0x13) != 0)
            count++;
    return count;
}

/* @zoombi32 0x00457f96 */
void releaseHeldPlace()
{
    if (placeHeld) {
        placeClaims[heldPlace] = 0;
        placeHeld = 0;
    }
}

/* The place held (from 1), if any. */
/* @zoombi32 0x00457fbb */
short heldPlaceNumber()
{
    if (placeHeld)
        return heldPlace + 1;
    return 0;
}

/* Where placed view `n` (from 1) was placed; (0, 0) if out of range. */
/* @zoombi32 0x00457fd0 */
void placedViewPoint(Point *where, short n)
{
    if (n > 0 && n < 125)
        *where = placedViewPoints[n - 1];
    else
        where->x = where->y = 0;
}

/* Where place `n` (from 1) is; (0, 0) if out of range. */
/* @zoombi32 0x00457fff */
void viewPlace(Point *where, short n)
{
    if (n > 0 && n < 125)
        *where = viewPlaces[n - 1];
    else
        where->x = where->y = 0;
}

/* Marks placed view `n` (from 1) as taken by view `id`. */
/* @zoombi32 0x0045802e */
void claimPlacedView(short n, short id)
{
    if (n > 0 && n < 125) {
        if (id >= 0)
            placeClaims[n - 1] = id;
    }
}

/* @zoombi32 0x0045b39a */
void setSpotCorner(short value)
{
    spotCorner = value & 3;
}

/* @zoombi32 0x0045bfc0 */
void setArrivalHook(SnoidArrived hook)
{
    arrivalHook = hook;
}

/*
 * Makes views for the party's Zoombinis: those on board (unless the party
 * has unknown2 set), placed in turn by viewPlace, and with `all` the others
 * too (unless unknown4), where they stood. Empties the party.
 */
/* @zoombi32 0x004572f0 */
void makePartySnoids(short all)
{
    Snoid snoid;
    short placed;
    short slot;
    short i;

    snoidsArrived = snoidsOnTheirWay = placed = 0;
    slot = countSnoidViews();
    for (i = slot; i < 32; i++)
        partyViews[i] = 0;
    for (i = 0; i < party()->count; i++) {
        if (slot < 32
            && ((travellers()[i].onboard && !party()->unknown2)
                || (!travellers()[i].onboard && all && !party()->unknown4))) {
            short j;

            for (j = 0; j < 4; j++)
                snoid.features[j] = travellers()[i].features[j];
            snoid.chosen = travellers()[i].onboard;
            if (snoid.chosen) {
                viewPlace((Point *)&snoid.body.x, placed + 1);
                placed++;
                snoid.angle = 1;
                snoid.facingLeft = 0;
            } else {
                *(Point *)&snoid.body.x = travellers()[i].place;
                snoid.angle = 1;
                snoid.facingLeft = 0;
            }
            for (j = 0; j < 10; j++)
                snoid.name[j] = travellers()[i].name[j];
            snoid.home = *(Point *)&snoid.body.x;
            *(Point *)&snoid.body.waypointX = *(Point *)&snoid.body.x;
            *(Point *)&snoid.targetX = *(Point *)&snoid.body.x;
            snoid.pathIndex = 0;
            snoid.path = 0;
            snoid.stepX = 0;
            snoid.stepY = 0;
            snoid.pathDirection = 0;
            snoid.idleTicks = randomBetween(0, 0x40);
            partyViews[slot] = addSnoidView(&snoid, 0);
            slot++;
        }
    }
    party()->count = 0;
}

/*
 * A Zoombini view's update, when due (in step with its group): by what it
 * is doing (action): 0 standing (blinking now and then), 1 and 2
 * turning, 3 a shake, 4 jumping to its target, 7 setting off and 0x70
 * walking there (a step at a time; on arriving it claims the placed view
 * and place it stands on), 10 leaving; then runs its script a frame.
 */
/* @zoombi32 0x004575e6 */
void updateSnoidView(View *view, short region)
{
    short stepX;
    short stepY;
    short dx;
    short dy;
    short event;
    short changed;
    short moving;
    Snoid *snoid;

    changed = 0;
    if (!view->body.running)
        return;
    if (dialogFlags)
        return;
    {
        short due;

        if (view->body.group) {
            if (!groupLeader[view->body.group])
                groupLeader[view->body.group] = view->id;
            if (groupLeader[view->body.group] == view->id)
                groupFlagsA[view->body.group] = due = view->nextUpdate <= updateTime;
            else
                due = groupFlagsA[view->body.group];
        } else {
            due = view->nextUpdate <= updateTime;
        }
        if (!due)
            return;
    }
    view->nextUpdate = updateTime + view->interval;
    snoid = viewSnoid(view);
    switch (snoid->action) {
    case 7:
        choosePath(snoid, (Point *)&snoid->targetX);
        stepAlongPath(snoid);
        snoid->action = 0x70;
    case 0x70:
        dx = snoid->body.waypointX - snoid->body.x;
        dy = snoid->body.y - snoid->body.waypointY;
        moving = 1;
        if (!dx && !dy && !stepAlongPath(snoid)) {
            ShortRect rect;
            short found;
            short i;

            moving = 0;
            unionRgnRect(currentViewRgn, &snoid->body.bounds);
            snoid->idleTicks = 0;
            setSnoidAction(snoid, snoidMode, 0);
            if (snoidsOnTheirWay > 0) {
                snoidsOnTheirWay--;
                snoidsArrived++;
            }
            if (hideArrivedPlaced && snoid->chosen == 2)
                view->flags |= 0x4000000;
            rect.left = snoid->body.x - placeSnapRadius;
            rect.right = snoid->body.x + placeSnapRadius;
            rect.top = snoid->body.y - placeSnapRadius;
            rect.bottom = snoid->body.y + placeSnapRadius;
            if (claimOnArrival) {
                found = 0;
                for (i = 0; !found && i < placedViewCount; i++)
                    if (!placeClaims[i] && ptInRect(&rect, placedViewPoints[i])) {
                        found = 1;
                        placeClaims[i] = view->id;
                    }
            }
            found = 0;
            for (i = 0; !found && i < viewPlaceCount; i++)
                if (!viewPlaceOwners[i] && ptInRect(&rect, viewPlaces[i])) {
                    found = 1;
                    viewPlaceOwners[i] = view->id;
                }
            if (arrivalHook)
                arrivalHook(view->id);
        }
        stepX = snoid->stepX;
        stepY = snoid->stepY;
        changed = 1;
        if (!moving)
            break;
        if (dx) {
            if (dx < 0) {
                snoid->body.x -= abs(dx) < abs(stepX) ? abs(dx) : abs(stepX);
                snoid->facingLeft = 1;
            } else {
                snoid->body.x += abs(dx) < abs(stepX) ? abs(dx) : abs(stepX);
                snoid->facingLeft = 0;
            }
        }
        if (dy) {
            if (dy < 0)
                snoid->body.y += abs(dy) < abs(stepY) ? abs(dy) : abs(stepY);
            else
                snoid->body.y -= abs(dy) < abs(stepY) ? abs(dy) : abs(stepY);
        }
        break;
    case 5:
        changed = 1;
        break;
    case 6:
        changed = 1;
        break;
    case 1:
    case 2:
        snoid->idleTicks = 0;
        if (snoid->action == 1) {
            if (!snoid->facingLeft) {
                switch (snoid->angle) {
                case 2:
                    snoid->angle = 1;
                    break;
                case 1:
                default:
                    snoid->angle = 0;
                    snoid->facingLeft = 1;
                    break;
                }
            } else {
                snoid->angle = 1;
                snoid->action = 0;
            }
        } else if (snoid->facingLeft) {
            switch (snoid->angle) {
            case 2:
                snoid->angle = 1;
                break;
            case 1:
            default:
                snoid->angle = 0;
                snoid->facingLeft = 0;
                break;
            }
        } else {
            snoid->angle = 1;
            snoid->action = 0;
        }
        changed = 1;
    case 0:
        if (snoid->pose) {
            snoid->pose = 0;
            changed = 1;
        }
        if (snoidIdleDelay && !dialogFlags && snoid->idleTicks++ > snoidIdleDelay) {
            snoid->idleTicks = 0;
            if (!altSnoids && randomBetween(0, 100) < 10) {
                snoid->pose = randomBetween(0, 7);
                setSnoidAction(snoid, 6, 0);
                changed = 1;
            }
        }
        break;
    case 4:
        changed = 1;
        if (snoid->targetX - snoid->body.x || snoid->body.y - snoid->targetY) {
            snoid->angle = 1;
            snoid->facingLeft = 0;
            *(Point *)&snoid->body.x = *(Point *)&snoid->targetX;
        } else {
            view->interval = 6;
            snoid->idleTicks = 0;
            setSnoidAction(snoid, snoidMode, 0);
        }
        break;
    case 8:
    case 9:
        changed = 1;
        break;
    case 3:
        if (snoid->pose < 6) {
            for (short j = 0; j <= 4; j++) {
                short image = snoid->body.cels[j].image;

                snoid->body.cels[j].image = snoid->body.cels[j + 10].image;
                snoid->body.cels[j + 10].image = image;
            }
            snoid->pose++;
        } else {
            setSnoidAction(snoid, 0, 0);
            snoid->idleTicks = 0;
        }
        view->changed = 1;
        return;
    case 10:
        setSnoidAction(snoid, 7, 0);
        snoidsOnTheirWay++;
        return;
    }
    if (!changed)
        return;
    unionRgnRect(region, &view->body.bounds);
    if (snoid->action == 4) {
        layOutSnoid(snoid, 0);
    } else if (snoid->body.lastFrame > 1) {
        if (snoid->body.frame >= snoid->body.lastFrame) {
            switch (snoid->action) {
            case 5:
                view->notify = 0;
                snoid->body.frame = 2;
                if (altSnoids)
                    snoid->body.frame = 0;
                snoid->body.frameOffset =
                    scriptFrameOffset(baseSnoidScripts[snoid->body.script], &snoid->body.frame, 1);
                break;
            case 7:
            case 0x70:
                snoid->body.frame = 1;
                snoid->body.frameOffset =
                    scriptFrameOffset(baseSnoidScripts[snoid->body.script], &snoid->body.frame, 1);
                break;
            case 8:
            case 9:
                if (snoid->idleTicks == 1) {
                    groupLeader[view->body.group] = 0;
                    view->body.group = 0;
                    snoid->body.running = 0;
                    snoid->body.frame = 0;
                    snoid->body.frameOffset = 2;
                } else {
                    unionRgnRect(currentViewRgn, &snoid->body.bounds);
                    setSnoidAction(snoid, 0, 0);
                }
                if (view->notifyEnd && view->notify)
                    view->notify(view, -1);
                view->notify = 0;
                view->changed = 1;
                return;
            default:
                unionRgnRect(currentViewRgn, &snoid->body.bounds);
                setSnoidAction(snoid, 0, 0);
                view->notify = 0;
                view->changed = 1;
                return;
            }
        }
        {
            short sound = layOutSnoid(snoid, &event);

            if (sound)
                queueViewSound(sound, 0);
        }
        if (event) {
            event--;
            if (event >= 200 && event <= 0xef) {
                short i;

                switch (event) {
                case 200:
                    i = 8;
                    break;
                case 201:
                    i = 6;
                    break;
                case 202:
                    i = 7;
                    break;
                case 203:
                    i = 10;
                    break;
                case 204:
                    i = 2;
                    break;
                case 205:
                    i = 12;
                    break;
                case 206:
                    i = 1;
                    break;
                case 207:
                    i = 9;
                    break;
                case 208:
                    i = 0;
                    break;
                case 209:
                    i = 4;
                    break;
                case 210:
                    i = 5;
                    break;
                case 211:
                    i = 3;
                    break;
                case 212:
                    i = 11;
                    break;
                case 213:
                    i = 13;
                    break;
                case 214:
                    i = 14;
                    break;
                case 215:
                    i = 15;
                    break;
                case 216:
                    i = 16;
                    break;
                case 217:
                    i = 17;
                    break;
                default:
                    i = 0;
                    break;
                }
                if (i)
                    queueViewSound(snoidSound(snoid, i), 0);
                event = 0;
            } else if (view->notify) {
                view->notify(view, event);
            }
        }
    } else {
        layOutSnoid(snoid, 0);
    }
    view->changed = 1;
}

/*
 * Drags a Zoombini with the mouse (from `where`, kept inside `bounds` or
 * the game area, `track` told of each position), lighting up the placed
 * view it's over; on release it heads for that view's place, or (if
 * settleSnoid doesn't place it) back where it was. Returns the placed view
 * it started on (from 1).
 */
/* @zoombi32 0x00458059 */
short dragSnoid(View *view, Point where, const ShortRect *bounds, void (*track)(Point where))
{
    short dx;
    short dy;
    short id;
    short target;
    short which;
    Point last;
    Point current;
    Point start;
    View *dragged;
    View *placed;
    View *old;
    ShortRect rect;
    ShortRect area;
    unsigned long savedInterval;
    unsigned long savedFlags;
    short moved;
    short savedBlink;
    short hit;
    short prevId;
    ShortRect limits;
    Snoid *snoid;
    short found;

    if (bounds != 0)
        limits = *bounds;
    else
        limits = gameRect;
    savedBlink = snoidIdleDelay;
    snoidIdleDelay = hit = prevId = 0;
    heldPlace = placeHeld = 0;
    id = view->id;
    if (dragInPlace)
        prevId = view->prev->id;
    if ((dragged = removeView(id, 0)) == 0)
        return 0;
    dragInProgress = 1;
    current = where;
    dragged->id = -3;
    savedFlags = dragged->flags;
    dragged->flags |= 0x4001000;
    insertViewAtEnd(dragged);
    if ((!practiceLevel && viewSnoid(view)->name[0]) || showPositions)
        showNameTag(viewSnoid(view)->name, 0, 0);
    savedInterval = dragged->interval;
    dragged->nextUpdate = 0;
    dragged->interval = 3;
    snoid = viewSnoid(dragged);
    snoid->idleTicks = 0;
    start = *(Point *)&snoid->body.x;
    unionRgnRect(removedRgn, &snoid->body.bounds);
    if (!keepDragPose)
        setSnoidAction(snoid, 5, 0);
    unionRgnRect(currentViewRgn, &snoid->body.bounds);
    last = current;
    dx = current.x - start.x;
    dy = current.y - start.y;
    rect.left = start.x - placeSnapRadius;
    rect.right = start.x + placeSnapRadius;
    rect.top = start.y - placeSnapRadius;
    rect.bottom = start.y + placeSnapRadius;
    found = 0;
    {
        short i;

        for (i = 0; !found && i < placedViewCount; i++)
            if (ptInRect(&rect, placedViewPoints[i])) {
                hit = i + 1;
                found = 1;
                if (placeClaims[i] == id)
                    placeClaims[i] = 0;
            }
        found = 0;
        for (i = 0; !found && i < viewPlaceCount; i++)
            if (ptInRect(&rect, viewPlaces[i])) {
                found = 1;
                if (viewPlaceOwners[i] == id)
                    viewPlaceOwners[i] = 0;
            }
    }
    target = 0;
    moved = 1;
    while (keepDragging()) {
        getCursorPosition(&current);
        if (track)
            track(current);
        if (last.x > current.x) {
            if (!snoid->facingLeft)
                snoid->facingLeft = 1;
        } else if (last.x < current.x) {
            if (snoid->facingLeft)
                snoid->facingLeft = 0;
        }
        if (!dragInPlace) {
            dragX = current.x - dx;
            if (dragX < limits.left)
                dragX = limits.left;
            if (dragX > limits.right)
                dragX = limits.right;
            snoid->body.x = dragX;
            dragY = current.y - dy;
            if (dragY < limits.top)
                dragY = limits.top;
            if (dragY > limits.bottom)
                dragY = limits.bottom;
            snoid->body.y = dragY;
            {
                short ddx = last.x - current.x;
                short ddy = last.y - current.y;

                if (!moved && (ddx || ddy))
                    moved = 1;
            }
        } else {
            moved = 1;
        }
        last = current;
        if (placedViewCount && (moved || endDragNow)) {
            short i;

            moved = 0;
            rect.left = snoid->body.x - placeSnapRadius;
            rect.right = snoid->body.x + placeSnapRadius;
            rect.top = snoid->body.y - placeSnapRadius;
            rect.bottom = snoid->body.y + placeSnapRadius;
            found = 0;
            if (!endDragNow) {
                for (i = 0; !found && i < placedViewCount && placesClaimable; i++)
                    if (!placeClaims[i] && ptInRect(&rect, placedViewPoints[i])) {
                        if ((placed = findView(placedViews[i])) != 0) {
                            found = 1;
                            if (placed->id != target) {
                                if ((old = findView(target)) != 0 && old->body.running) {
                                    old->flags |= 0x10000;
                                    old->nextUpdate = 0;
                                    mainLoopEvents();
                                }
                                which = i;
                                target = placed->id;
                                placed->body.running = 1;
                                placed->body.frame = 0;
                                placed->body.frameOffset = 1;
                            }
                        }
                    }
            }
            if (!found && target) {
                if ((old = findView(target)) != 0 && old->body.running) {
                    old->flags |= 0x10000;
                    old->nextUpdate = 0;
                    mainLoopEvents();
                }
                target = 0;
            }
        }
        mainLoopEvents();
        resetViewClock();
    }
    if (target && !dragInPlace) {
        placeClaims[which] = id;
        heldPlace = which;
        placeHeld = id;
        if ((old = findView(target)) != 0 && old->body.running) {
            old->flags |= 0x10000;
            old->nextUpdate = 0;
        }
    }
    if (showPositions) {
        View *other = findView(-2);

        if (other && other->body.running)
            other->body.running = 0;
    } else {
        hideNameTag();
    }
    area = dragged->body.bounds;
    unionRgnRect(removedRgn, &area);
    if (!dragInPlace) {
        if (target)
            *(Point *)&snoid->targetX = placedViewPoints[which];
        else if (currentScene == 6)
            *(Point *)&snoid->targetX = start;
        else if (!settleSnoid(dragged))
            *(Point *)&snoid->targetX = start;
    }
    snoid->idleTicks = 1;
    unionRgnRect(removedRgn, &snoid->body.bounds);
    if (!keepDragPose) {
        if (dragInPlace) {
            snoid->facingLeft = 0;
            setSnoidAction(snoid, 0, 0);
        } else {
            setSnoidAction(snoid, 4, 0);
        }
    }
    dragged->id = id;
    dragged->flags = savedFlags;
    dragged->interval = savedInterval;
    snoidIdleDelay = savedBlink;
    if (dragInPlace)
        moveView(id, 1, prevId);
    dragInPlace = 0;
    endDragNow = 0;
    dragInProgress = 0;
    return hit;
}

/*
 * Shows a name tag (view -2) with `text`, large or small, for `duration`
 * ms (0: until taken down).
 */
/* @zoombi32 0x004589ce */
void showNameTag(const char *text, unsigned long duration, short large)
{
    View *view;

    deleteView(-2);
    if (!dialogFlags) {
        view = (View *)newPtr(0xec);
        if (view) {
            initView(view, 0, 0, -2);
            strcpy((char *)&view->body, text);
            view->draw = drawNameTag;
            view->update = updateNameTag;
            view->body.frameOffset = 0;
            if (large)
                view->unknown1e = 1;
            else
                view->unknown1e = 0;
            view->flags |= 0x4001000;
            if (duration)
                { duration += clockTime(); view->body.frameOffset = duration; }
            insertViewAtEnd(view);
        }
    }
}

/* Takes down the name tag. */
/* @zoombi32 0x00458a61 */
void hideNameTag()
{
    View *view = findView(-2);

    if (view && view->body.running) {
        view->body.running = 0;
        unionRgnRect(removedRgn, &view->body.bounds);
        unionRgnRect(currentViewRgn, &view->body.bounds);
    }
}

/* Draws the name tag (with the dragged Zoombini's position, if showing them). */
/* @zoombi32 0x00458aaa */
void drawNameTag(View *view)
{
    Color saved;
    ShortRect rect;
    short image;
    char text[24];

    if (view->body.running) {
        saved = setForeColor(Color(0x2d));
        if (view->unknown1e) {
            rect = largeNameTagRect;
            image = 2;
        } else {
            rect = nameTagRect;
            image = 1;
        }
        drawImageData((unsigned short *)((char *)snoidImages3 + snoidImages3->offsets[image]), rect.left,
                      rect.top, 8);
        if (showPositions && dragInProgress) {
            short length;

            intToDecimal(dragX, text);
            length = strlen(text);
            text[length++] = ',';
            text[length++] = ' ';
            text[length] = 0;
            intToDecimal(dragY, text + length);
            drawText(Rect(rect), 0x22, text, 0xffff);
        } else {
            drawText(Rect(rect), 0x22, (char *)&view->body, 0xffff);
        }
        setForeColor(saved);
    }
}

/* @zoombi32 0x00458c17 */
void updateNameTag(View *view, short region)
{
    if (view->body.running) {
        if (view->reset) {
            ShortRect rect;

            if (view->unknown1e)
                rect = largeNameTagRect;
            else
                rect = nameTagRect;
            rect.bottom = 0x244;
            unionRgnRect(region, &rect);
            view->body.bounds = rect;
            view->changed = 1;
            view->reset = 0;
        }
        if (showPositions)
            view->changed = 1;
        if (view->body.frameOffset && view->body.frameOffset <= updateTime)
            hideNameTag();
    }
}

/* Loads the paths Zoombinis walk ('NODE' and 'PATH' resources `id`). */
/* @zoombi32 0x0045915d */
void loadPaths(short id)
{
    short handle;

    pathNodes = (PathNodes *)loadSwappedResource(&pathNodesResource, id, RESOURCE_TYPE('N', 'O', 'D', 'E'));
    loadResourceAs(&pathsResource, RESOURCE_TYPE('P', 'A', 'T', 'H'), id, 0, 1);
    handle = usedResourceHandle(pathsResource);
    paths = (Paths *)lockHandleAlias(handle);
    swapInPlace(paths->count);
}

/* @zoombi32 0x004591cc */
void freePaths()
{
    freeResource(&pathsResource);
    freeResource(&pathNodesResource);
    paths = 0;
    pathNodes = 0;
    nextPathToDraw = 0;
}

/* @zoombi32 0x00459c02 */
void toggleShowPositions()
{
    showPositions = !showPositions;
}

/* Sets every Zoombini's view's script running (or not). */
/* @zoombi32 0x0045a44c */
void setSnoidsRunning(short running)
{
    for (View *view = viewListEnd(1); view; view = view->next)
        if (view->flags & 1)
            view->body.running = running;
}

/* Marks the Zoombinis holding places (chosen). */
/* @zoombi32 0x0045a477 */
void markPlacedSnoids()
{
    for (short i = 0; i < 125; i++)
        if (placeClaims[i]) {
            View *view = findView(placeClaims[i]);

            if (view && (view->flags & 1))
                viewSnoid(view)->chosen = 1;
        }
}

/* Runs the Zoombini view `id`'s script, setting its chosen. */
/* @zoombi32 0x0045a4b2 */
void runSnoid(short id, short chosen)
{
    for (View *view = viewListEnd(1); view; view = view->next)
        if ((view->flags & 1) && id == view->id) {
            view->body.running = 1;
            viewSnoid(view)->chosen = chosen;
        }
}

/* @zoombi32 0x0045aaff */
void setSnoidMode(short mode)
{
    switch (mode) {
    case -1:
        snoidMode = 1;
        break;
    case 1:
        snoidMode = 2;
        break;
    default:
        snoidMode = 0;
        break;
    }
}

/* The Zoombini view `id`'s snoid, if it's idle (state 0). */
/* @zoombi32 0x0045b37a */
View *idleSnoidView(short id)
{
    View *view = findView(id);

    if (!view || viewSnoid(view)->action)
        return 0;
    return view;
}

/* The Zoombini `id`'s snoid (0 if it isn't one), stopping its clock if asked. */
/* @zoombi32 0x0045b9ef */
Snoid *findSnoid(short id, short wake)
{
    Snoid *snoid = 0;
    View *view = findView(id);

    if (view && (view->flags & 1)) {
        snoid = viewSnoid(view);
        if (wake)
            view->nextUpdate = 0;
    }
    return snoid;
}

/* Lists the chosen Zoombinis' features (those running with chosen set). */
/* @zoombi32 0x00459c17 */
ChosenSnoids *listChosenSnoids()
{
    View *view;
    short n;

    chosenSnoids.count = countChosenSnoids();
    for (view = nextActorView(1), n = 0; view && n < chosenSnoids.count; view = nextActorView(0))
        if (view->body.running && viewSnoid(view)->chosen) {
            for (short i = 0; i < 4; i++)
                chosenSnoids.features[n][i] = viewSnoid(view)->features[i];
            n++;
        }
    return &chosenSnoids;
}

/*
 * Sets which Zoombinis are chosen (chosen): the first 20 running (all,
 * first, if `run`).
 */
/* @zoombi32 0x0045a3e1 */
void chooseSnoids(short chosen, short run)
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if (view->flags & 1) {
            if (run)
                view->body.running = 1;
            if (count < 20) {
                if (view->body.running) {
                    viewSnoid(view)->chosen = chosen;
                    if (chosen)
                        count++;
                } else {
                    viewSnoid(view)->chosen = 0;
                }
            } else {
                viewSnoid(view)->chosen = 0;
            }
        }
}

/* Whether a placed view is near `where`. */
/* @zoombi32 0x0045a6e0 */
short nearPlacedView(Point where)
{
    ShortRect rect;

    rect.left = where.x - placeSnapRadius;
    rect.right = where.x + placeSnapRadius;
    rect.top = where.y - placeSnapRadius;
    rect.bottom = where.y + placeSnapRadius;
    for (short i = 0; i < placedViewCount; i++)
        if (ptInRect(&rect, placedViewPoints[i]))
            return 1;
    return 0;
}

/* Told of a Zoombini's script's events: turns it round when it's turning. */
/* @zoombi32 0x0045ab35 */
void turnSnoid(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);

    switch (snoid->action) {
    case 8:
    case 9:
        snoid->facingLeft = !snoid->facingLeft;
        short *at = snoidScripts[snoid->body.script] + snoid->body.frameOffset;
        if (*at > 0)
            snoid->body.waypointX = at[1] - snoid->body.x;
        break;
    }
}

/* Sets up a new Zoombini. */
/* @zoombi32 0x0045bf41 */
void initSnoid(Snoid *snoid)
{
    snoid->body.cels[0].image = 0;
    snoid->body.celsEnd = 0;
    snoid->body.script = 1;
    snoid->body.lastFrame = 1;
    snoid->drawnFacing = -1;
    snoid->stepX = snoid->stepY = 0;
    snoid->pathDirection = 0;
    snoid->angle = 1;
    snoid->facingLeft = 0;
    snoid->action = 0;
    snoid->pose = 1;
    snoid->chosen = 1;
    snoid->idleTicks = 0;
    snoid->name[0] = 0;
}

/*
 * The sound a Zoombini makes for script command 201 + `which`: most vary
 * with its hair and nose.
 */
/* @zoombi32 0x0045b8b0 */
short snoidSound(Snoid *snoid, short which)
{
    short sound = 0;
    short varies = 1;

    switch (which) {
    case 8:
        sound = 300;
        break;
    case 6:
        sound = 250;
        break;
    case 7:
        sound = 275;
        break;
    case 10:
        sound = 350;
        break;
    case 2:
        sound = 150;
        break;
    case 12:
        sound = 400;
        break;
    case 1:
        sound = 125;
        break;
    case 9:
        sound = 325;
        break;
    case 0:
        sound = 100;
        break;
    case 4:
        sound = 200;
        break;
    case 5:
        sound = 225;
        break;
    case 3:
        sound = 175;
        break;
    case 11:
        sound = 375;
        break;
    case 13:
        sound = 475;
        break;
    case 14:
        sound = 450;
        break;
    case 15:
        sound = 425;
        break;
    case 16:
        sound = randomBetween(1800, 1814);
        varies = 0;
        break;
    case 17:
        sound = 99;
        varies = 0;
        break;
    }
    if (sound && varies) {
        switch (snoid->features[0]) {
        case 0:
        case 1:
            break;
        case 2:
            sound += 5;
            break;
        case 3:
            sound += 20;
            break;
        case 4:
            sound += 15;
            break;
        case 5:
            sound += 10;
            break;
        }
        sound = sound + snoid->features[2] - 1;
    }
    return sound;
}

/*
 * Lists the idle Zoombinis' spots (but `ignore`'s), and whether any is
 * within `radius` (squared) of `where`.
 */
/* @zoombi32 0x0045b3af */
short spotTaken(Point *where, View *ignore, short radius)
{
    short found;
    short i;

    spotCount = 0;
    spotRadius = radius;
    for (View *view = viewListEnd(1); view; view = view->next)
        if ((view->flags & 1)
            && (!viewSnoid(view)->action || viewSnoid(view)->action == 6 || viewSnoid(view)->action == 3)
            && (!ignore || (ignore && ignore->id != view->id))) {
            spots[spotCount] = *(Point *)&view->body.x;
            spotIds[spotCount] = view->id;
            spotCount++;
        }
    found = 0;
    for (i = 0; !found && i < spotCount; i++)
        if ((unsigned long)((where->x - spots[i].x) * (where->x - spots[i].x)
                            + (where->y - spots[i].y) * (where->y - spots[i].y))
            < spotRadius)
            found = 1;
    return found;
}

/*
 * The Zoombini whose spot (as spotTaken listed them) is within `radius`
 * (squared) of `where`, after `skip` others (0: none).
 */
/* @zoombi32 0x0045b4ca */
short spotNear(Point *where, short radius, short skip)
{
    short found;
    short i;

    spotRadius = radius;
    found = 0;
    for (i = 0; !found && i < spotCount; i++)
        if ((unsigned long)((where->x - spots[i].x) * (where->x - spots[i].x)
                            + (where->y - spots[i].y) * (where->y - spots[i].y))
            < spotRadius) {
            if (skip)
                skip--;
            else
                found = spotIds[i];
        }
    return found;
}

/* Draws one of a Zoombini's features (1-4: hair, eyes, nose, feet) at `rect`. */
/* @zoombi32 0x0045bfcf */
void drawFeature(short feature, short value, ShortRect *rect)
{
    short image;

    if (feature >= 1 && feature <= 4 && value >= 1 && value <= 5) {
        switch (feature) {
        case 1:
            image = hairImages[value];
            break;
        case 2:
            image = eyesImages[value];
            break;
        case 3:
            image = noseImages[value];
            break;
        case 4:
            image = feetImages[value];
            break;
        }
        image = image * 2 + 1;
        if (image <= snoidImages->count) {
            unsigned short *data = (unsigned short *)((char *)snoidImages + snoidImages->offsets[image]);

            rect->left -= snoidTables[0][image];
            rect->top -= snoidTables[1][image];
            drawImageData(data, rect->left, rect->top, 8);
            rect->right = swapShort(data[0]) + rect->left;
            rect->bottom = swapShort(data[1]) + rect->top;
            showRect(rect);
        }
    }
}

/* Makes up a Zoombini's name (of up to `size` - 2 letters) in `name`. */
/* @zoombi32 0x0045885c */
void makeName(char *name, short size)
{
    short target;
    short k;
    short ending;
    char c;
    short length;
    unsigned short vowel;
    short i;

    for (i = 0; i < size; i++)
        name[i] = 0;
    target = randomBetween(4, size - 2);
    length = 0;
    vowel = randomBetween(1, 100) < 40;
    while (length < target) {
        ending = 0;
        if (vowel) {
            vowel = !vowel;
            k = (randomBetween(1, 31) - 1) * 2;
            c = vowelSounds[k];
            if (vowelSounds[k + 1] != ' ') {
                name[length++] = c;
                c = vowelSounds[k + 1];
            }
        } else {
            vowel = !vowel;
            if (length > 1 || randomBetween(1, 100) <= 33) {
                k = (randomBetween(1, 40) - 1) * 2;
                c = consonantPairs[k];
                if (consonantPairs[k + 1] != ' ') {
                    name[length++] = c;
                    c = consonantPairs[k + 1];
                }
                ending = 1;
            } else {
                c = consonants[randomBetween(1, 32) - 1];
            }
        }
        name[length++] = c;
        if (ending && length >= target)
            name[length - 1] = nameEndings[randomBetween(1, 6) - 1];
        if (length == 2 && name[0] == name[1])
            length = 1;
    }
}

/*
 * Lists the chosen Zoombinis (and the running ones, if `running`) by where
 * they're heading, left to right.
 */
/* @zoombi32 0x00458f90 */
void sortSnoids(short running)
{
    short x;
    short id;

    sortedCount = 0;
    for (View *view = viewListEnd(1); view; view = view->next)
        if (view->flags & 1) {
            Snoid *snoid = viewSnoid(view);
            short include = 0;

            if (running && view->body.running)
                include = 1;
            if (snoid->chosen || include) {
                short done;
                short j;

                x = snoid->targetX;
                id = view->id;
                done = 0;
                for (j = 0; !done && j < sortedCount; j++)
                    if (sortedX[j] > x) {
                        for (done = sortedCount; done > j; done--) {
                            sortedX[done] = sortedX[done - 1];
                            sortedIds[done] = sortedIds[done - 1];
                        }
                        done = 1;
                        sortedX[j] = x;
                        sortedIds[j] = id;
                    }
                if (!done) {
                    sortedX[sortedCount] = x;
                    sortedIds[sortedCount] = id;
                }
                sortedCount++;
            }
        }
}

/*
 * Picks the path a Zoombini takes toward `target`: of the paths through the
 * node nearest `target`, the node nearest the Zoombini (its path in
 * path, the node's place in it, from 1, in pathIndex, and which way
 * to walk it in pathDirection). Without paths it heads straight there.
 */
/* @zoombi32 0x004595c2 */
void choosePath(Snoid *snoid, Point *target)
{
    short nodeCount;
    short pathCount;
    short nearest;
    Point at;
    PathNodes *nodes;
    long best;
    long dx;
    long dy;
    long distance;

    if (!paths || !pathNodes || noPaths) {
        *(Point *)&snoid->body.waypointX = *(Point *)&snoid->targetX;
        return;
    }
    at = *(Point *)&snoid->body.x;
    nodes = pathNodes;
    Paths *list = paths;
    nodeCount = nodes->count;
    nearest = 0;
    best = 999999;
    for (short i = 0; i < nodeCount; i++) {
        dx = nodes->nodes[i].x - target->x;
        dy = nodes->nodes[i].y - target->y;
        distance = dx * dx + dy * dy;
        if (distance <= best) {
            best = distance;
            nearest = i + 1;
        }
    }
    pathCount = list->count;
    best = 999999;
    for (short path = 0; path < pathCount; path++)
        for (short j = 0; j < 24; j++)
            if (list->nodes[path][j] == nearest) {
                for (short k = 0; k < 24; k++)
                    if (list->nodes[path][k]) {
                        dx = nodes->nodes[list->nodes[path][k] - 1].x - at.x;
                        dy = nodes->nodes[list->nodes[path][k] - 1].y - at.y;
                        distance = dx * dx + dy * dy;
                        if (distance <= best) {
                            best = distance;
                            snoid->pathIndex = k + 1;
                            snoid->path = path;
                            snoid->pathDirection = 1;
                            if (j && j <= k)
                                snoid->pathDirection = -1;
                        }
                    }
                j = 24;
            }
}

/*
 * Steps a walking Zoombini toward its next waypoint (waypointX: along its
 * path, or its target once that's nearer than the path's next node),
 * setting its step (stepX, stepY) and, if its heading changes, its
 * walk script. Whether it has anywhere to go.
 */
/* @zoombi32 0x004591f8 */
short stepAlongPath(Snoid *snoid)
{
    Point next;
    short oldHeading;
    short stepX;
    short stepY;
    short moving;
    long toNode;
    short dx;
    short dy;
    short steps;

    moving = 0;
    if (paths && pathNodes && !noPaths) {
        short arrived;

        if (snoid->pathIndex >= 0) {
            arrived = 0;
            PathNodes *nodes = pathNodes;
            Paths *list = paths;
            char node = list->nodes[snoid->path][snoid->pathIndex];

            snoid->pathIndex += snoid->pathDirection;
            if (node) {
                long ex;
                long ey;

                next.x = nodes->nodes[node - 1].x;
                next.y = nodes->nodes[node - 1].y;
                ex = snoid->body.x - next.x;
                ey = snoid->body.y - next.y;
                toNode = ex * ex + ey * ey;
                ex = snoid->body.x - snoid->targetX;
                ey = snoid->body.y - snoid->targetY;
                ex = ex * ex + ey * ey;
                if (ex <= toNode)
                    arrived = 1;
            } else {
                arrived = 1;
            }
        } else {
            arrived = 1;
        }
        if (arrived)
            *(Point *)&snoid->body.waypointX = *(Point *)&snoid->targetX;
        else
            *(Point *)&snoid->body.waypointX = next;
    }
    dx = snoid->body.waypointX - snoid->body.x;
    dy = snoid->body.y - snoid->body.waypointY;
    if (dx || dy) {
        short slope;

        oldHeading = snoid->pose;
        if (dx)
            slope = (dy * 1024) / abs(dx);
        else if (dy < 0)
            slope = -1410;
        else
            slope = 1410;
        if (slope <= -1409)
            snoid->pose = 0;
        else if (slope <= -332)
            snoid->pose = 1;
        else if (slope < 332)
            snoid->pose = 2;
        else if (slope < 1409)
            snoid->pose = 3;
        else
            snoid->pose = 4;
        switch (snoid->pose) {
        case 0:
            stepX = 5;
            stepY = -15;
            break;
        case 1:
            stepX = 13;
            stepY = -10;
            break;
        case 2:
            stepX = 16;
            stepY = 8;
            break;
        case 3:
            stepX = 13;
            stepY = 10;
            break;
        case 4:
            stepX = 5;
            stepY = 15;
            break;
        }
        if (abs(stepX) >= abs(stepY)) {
            snoid->stepX = stepX;
            steps = abs(dx) / abs(stepX);
            if (steps)
                snoid->stepY = dy / steps;
            else
                snoid->stepY = dy;
            if (!snoid->stepY && dy)
                snoid->stepY = dy / abs(dy);
        } else {
            snoid->stepY = stepY;
            steps = abs(dy) / abs(stepY);
            if (steps)
                snoid->stepX = dx / steps;
            else
                snoid->stepX = dx;
            if (!snoid->stepX && dx)
                snoid->stepX = dx / abs(dx);
        }
        moving = 1;
        if (oldHeading != snoid->pose) {
            short *script;

            snoid->body.script = snoid->features[3] * 5 + snoid->pose;
            script = baseSnoidScripts[snoid->body.script];
            snoid->body.frameOffset = scriptFrameOffset(script, &snoid->body.frame, 1);
            setSnoidFacing(snoid, script[1]);
        }
    }
    return moving;
}

/*
 * Sets the images a Zoombini is drawn with (layers: its features', and
 * 0 for its body) for which way it faces (0-2), in the order they overlap.
 */
/* @zoombi32 0x0045b06a */
void setSnoidFacing(Snoid *snoid, short facing)
{
    if (facing != snoid->drawnFacing) {
        snoid->drawnFacing = facing;
        short *layers = snoid->layers;

        if (snoid->action == 9) {
            switch (facing) {
            case 0:
                layers[1] = altFeetImages[snoid->features[3]];
                layers[2] = 0;
                layers[3] = altNoseImages[snoid->features[2]];
                layers[4] = altEyesImages[snoid->features[1]];
                layers[5] = altHairImages[snoid->features[0]];
                break;
            case 1:
                layers[1] = altFeetImages[snoid->features[3]];
                layers[2] = altNoseImages[snoid->features[2]];
                layers[3] = 0;
                layers[4] = altEyesImages[snoid->features[1]];
                layers[5] = altHairImages[snoid->features[0]];
                break;
            case 2:
                layers[1] = 0;
                layers[2] = altEyesImages[snoid->features[1]];
                layers[3] = altNoseImages[snoid->features[2]];
                layers[4] = altFeetImages[snoid->features[3]];
                layers[5] = altHairImages[snoid->features[0]];
                break;
            }
        } else {
            switch (facing) {
            case 0:
                layers[1] = feetImages[snoid->features[3]];
                layers[2] = 0;
                layers[3] = noseImages[snoid->features[2]];
                layers[4] = eyesImages[snoid->features[1]];
                layers[5] = hairImages[snoid->features[0]];
                break;
            case 1:
                layers[1] = feetImages[snoid->features[3]];
                layers[2] = noseImages[snoid->features[2]];
                layers[3] = 0;
                layers[4] = eyesImages[snoid->features[1]];
                layers[5] = hairImages[snoid->features[0]];
                break;
            case 2:
                layers[1] = 0;
                layers[2] = eyesImages[snoid->features[1]];
                layers[3] = noseImages[snoid->features[2]];
                layers[4] = feetImages[snoid->features[3]];
                layers[5] = hairImages[snoid->features[0]];
                break;
            }
        }
    }
}

/*
 * Starts a Zoombini doing `action` (0-10; its state, action), at `where`
 * if given: picks its script and starts it.
 */
/* @zoombi32 0x0045a75b */
void setSnoidAction(Snoid *snoid, short action, Point *where)
{
    short frame;
    short which;
    short sound;
    short script;

    frame = 0;
    if (action < 0 || action > 10)
        action = 0;
    groupLeader[snoid->body.group] = 0;
    snoid->body.group = 0;
    switch (action) {
    case 3:
        if (!snoid->action) {
            if (snoid->angle != 1)
                snoid->angle = 1;
            short mask = snoid->pose; /* features to show changed (8: feet ... 1: hair) */
            for (short i = 0; i <= 4; i++) {
                short image = 0;

                switch (i) {
                case 0:
                    if (mask & 8)
                        image = snoid->features[3] + 435;
                    break;
                case 1:
                    break;
                case 2:
                    if (mask & 4)
                        image = snoid->features[2] + 440;
                    break;
                case 3:
                    if (mask & 2)
                        image = snoid->features[1] + 430;
                    break;
                case 4:
                    if (mask & 1)
                        image = snoid->features[0] + 425;
                    break;
                }
                if (!image) {
                    snoid->body.cels[i + 10].image = snoid->body.cels[i].image;
                } else {
                    image = image * 2 - 1;
                    if (snoid->facingLeft)
                        image++;
                    snoid->body.cels[i + 10].image = image;
                }
            }
            snoid->pose = 0;
            snoid->action = 3;
            return;
        }
        /* fall through */
    case 0:
    case 1:
    case 2:
    case 10:
        script = snoid->angle;
        break;
    case 4:
        snoid->pathDirection = 0;
        script = snoid->angle;
        break;
    case 5:
        script = snoid->features[3] + 45;
        switch (snoid->angle) {
        case 0:
        default:
            frame = 0;
            break;
        case 1:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        }
        if (altSnoids) {
            frame = 0;
            script = snoid->angle;
        }
        break;
    case 7:
    case 112:
        script = snoid->features[3] * 5 + snoid->pose;
        break;
    case 6:
        switch (snoid->angle) {
        case 0:
        default:
            snoid->angle = 1;
        case 1:
            script = snoid->pose + 30;
            break;
        case 2:
            script = snoid->pose + 38;
            break;
        }
        if (snoidIdleDelay && soundOn) {
            if (randomBetween(1, 100) <= 50)
                which = 4;
            else
                which = 5;
            ambientCounter = ++ambientCounter % 32;
            if (!ambientCounter)
                for (sound = 100; sound <= 424; sound++)
                    unloadSoundNow(sound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            queueViewSound(snoidSound(snoid, which), 0);
        }
        break;
    case 8:
    case 9:
    default:
        action = 0;
        script = snoid->angle;
        break;
    }
    snoid->drawnFacing = -1;
    snoid->action = action;
    if (!action)
        snoid->pose = 1;
    else
        snoid->pose = 0;
    snoid->body.running = 1;
    if (where)
        *(Point *)&snoid->body.x = *where;
    snoid->body.frame = 0;
    snoid->body.frameOffset = 2;
    snoid->body.script = script;
    short *data = baseSnoidScripts[script];
    script = data[1];
    snoid->body.lastFrame = data[0];
    if (frame) {
        snoid->body.frameOffset = scriptFrameOffset(data, &frame, 1);
        snoid->body.frame = frame;
    }
    unionRgnRect(currentViewRgn, &snoid->body.bounds);
    setSnoidFacing(snoid, script);
    layOutSnoid(snoid, 0);
    unionRgnRect(currentViewRgn, &snoid->body.bounds);
}

/*
 * Sets the chosen Zoombinis doing action 7 off one by one, right to left:
 * the first after `delay` ms, then every `interval` ms.
 */
/* Not exact: the original keeps `interval` on the stack; this puts it in edi. */
/* @zoombi32 0x00458f07 */
void staggerSnoids(unsigned long interval, unsigned long delay)
{
    sortSnoids(0);
    if (sortedCount && staggerDue) {
        staggerDue = 0;
        unsigned long when = clockTime() + delay;

        for (short i = sortedCount - 1; i >= 0; i--) {
            View *view = findView(sortedIds[i]);
            Snoid *snoid = viewSnoid(view);

            if ((snoid->action == 7 || snoid->action == 112) && snoid->chosen) {
                view->nextUpdate = when;
                when += interval;
            }
        }
    }
}

/*
 * Sends the idle chosen Zoombinis walking (action 10) to (x, y), right to
 * left, one every `interval` ms.
 */
/* @zoombi32 0x004590b6 */
void sendSnoids(short x, short y, unsigned long interval)
{
    unsigned long when;

    snoidsArrived = snoidsOnTheirWay = 0;
    sortSnoids(0);
    if (sortedCount) {
        when = clockTime();
        for (short i = sortedCount - 1; i >= 0; i--) {
            View *view = findView(sortedIds[i]);
            Snoid *snoid = viewSnoid(view);

            if (snoid->chosen && !snoid->action) {
                *(Point *)&snoid->body.waypointX = *(Point *)&snoid->body.x;
                snoid->targetX = x;
                snoid->targetY = y;
                setSnoidAction(snoid, 10, 0);
                view->nextUpdate = when;
                when += interval;
            }
        }
    }
}

/*
 * Picks a free one of `count` places (none of the idle Zoombinis within
 * `radius` of it), from the left or the right at random, into *result.
 */
/* @zoombi32 0x0045be33 */
void pickFreePlace(Point *result, Point *places, short count, short radius)
{
    short skip;
    Point place;
    Point origin;
    short id;
    short i;

    origin = spotOrigin;
    spotTaken(&origin, 0, radius);
    for (i = 0; i < count; i++) {
        skip = 0;
        place = places[i];
        id = spotNear(&place, radius, skip);
        for (short j = 0; id && j < i; j++)
            if (id == sortedIds[j]) {
                skip++;
                id = spotNear(&place, radius, skip);
                j = 0;
            }
        sortedIds[i] = id;
    }
    id = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = count - 1; id == -1 && i >= 0; i--)
            if (!sortedIds[i])
                id = i;
    } else {
        for (i = 0; id == -1 && i < count; i++)
            if (!sortedIds[i])
                id = i;
    }
    if (id == -1)
        id = 0;
    *result = places[id];
}

/*
 * Places the chosen Zoombinis as a scene starts: those already standing on
 * placed views or view places claim them; unless the game says otherwise,
 * the last quarter walk in (action 7) from off the left edge to the first
 * free view places, `dy` below them.
 */
/* @zoombi32 0x00458cc1 */
void enterSnoids(short dy)
{
    short i;
    short count;
    short placed;
    short x;
    short first;
    ShortRect rect;
    View *view;

    x = -50;
    snoidsArrived = snoidsOnTheirWay = 0;
    if (*(short *)(gameState + 0x20) || skipJourneyMap) {
        skipJourneyMap = 0;
        staggerDue = 0;
    } else {
        staggerDue = 1;
    }
    count = countChosenSnoids();
    view = nextActorView(1);
    placed = 0;
    first = count * 3 / 4;
    for (i = 0; i < count; i++) {
        if (view) {
            Snoid *snoid = viewSnoid(view);

            if (snoid->chosen) {
                if (staggerDue && i >= first) {
                    if (first + placed < viewPlaceCount) {
                        snoid->body.x = x;
                        snoid->body.y = viewPlaces[first + placed].y + dy;
                        *(Point *)&snoid->targetX = viewPlaces[first + placed];
                        setSnoidAction(snoid, 7, 0);
                        view->nextUpdate = 0;
                        snoidsOnTheirWay++;
                        placed++;
                    }
                } else {
                    short found;
                    short j;

                    rect.left = snoid->body.x - placeSnapRadius;
                    rect.right = snoid->body.x + placeSnapRadius;
                    rect.top = snoid->body.y - placeSnapRadius;
                    rect.bottom = snoid->body.y + placeSnapRadius;
                    found = 0;
                    for (j = 0; !found && j < placedViewCount; j++)
                        if (!placeClaims[j] && ptInRect(&rect, placedViewPoints[j])) {
                            found = 1;
                            placeClaims[j] = view->id;
                        }
                    found = 0;
                    for (j = 0; !found && j < viewPlaceCount; j++)
                        if (!viewPlaceOwners[j] && ptInRect(&rect, viewPlaces[j])) {
                            found = 1;
                            viewPlaceOwners[j] = view->id;
                        }
                }
            }
        }
        view = nextActorView(0);
    }
}

/*
 * Finds a Zoombini a spot (its target) away from the idle ones (within
 * `radius`, squared): in `area`, trying a 5 by 4 grid from spotCorner, else
 * near where it's going (or, if it's to walk there, where it stands).
 */
/* @zoombi32 0x0045b57a */
void findSpot(View *view, ShortRect *area, short walk, short radius)
{
    Snoid *snoid;
    Point at;
    Point spot;
    short column;
    short row;
    short width;
    short height;
    short stagger;
    short done;
    short taken;

    taken = 1;
    done = 0;
    spotRadius = radius;
    at = *(Point *)&view->body.x;
    if (walk)
        spotTaken(&at, view, spotRadius);
    snoid = viewSnoid(view);
    if (area) {
        width = area->right - area->left;
        height = area->bottom - area->top;
        stagger = width / 10;
    }
    switch (spotCorner) {
    case 0:
        column = 1;
        row = 1;
        break;
    case 1:
        column = 5;
        row = 1;
        break;
    case 2:
        column = 1;
        row = 4;
        break;
    case 3:
        column = 5;
        row = 4;
        break;
    }
    while (taken) {
        if (area) {
            spot.x = randomBetween(0, 5) + (width * column / 5 + area->left);
            spot.y = height * row / 4 + area->top;
            if (!(row & 1))
                spot.x += stagger;
        } else {
            short dx = randomBetween(-5, 5);
            short dy = 0;

            if (walk) {
                spot.x = dx * 4 + at.x;
                spot.y = dy * 4 + at.y;
            } else {
                spot.x = snoid->targetX + dx * 4;
                spot.y = snoid->targetY + dy * 4;
            }
        }
        if (spot.x < 0)
            spot.x = 0;
        if (spot.x > 640)
            spot.x = 640;
        if (spot.y < 0)
            spot.y = 0;
        if (spot.y > 480)
            spot.y = 480;
        taken = 0;
        for (short i = 0; !taken && i < spotCount; i++)
            if ((unsigned long)((spot.x - spots[i].x) * (spot.x - spots[i].x)
                                + (spot.y - spots[i].y) * (spot.y - spots[i].y))
                < spotRadius)
                taken = 1;
        if (!taken)
            *(Point *)&snoid->targetX = spot;
        switch (spotCorner) {
        case 0:
            column++;
            if (column > 5) {
                column = 1;
                row++;
                if (row > 4) {
                    row = 1;
                    done = 1;
                }
            }
            break;
        case 1:
            column--;
            if (column < 1) {
                column = 5;
                row++;
                if (row > 4) {
                    row = 1;
                    done = 1;
                }
            }
            break;
        case 2:
            column++;
            if (column > 5) {
                column = 1;
                row--;
                if (row < 1) {
                    row = 4;
                    done = 1;
                }
            }
            break;
        case 3:
            column--;
            if (column < 1) {
                column = 5;
                row--;
                if (row < 1) {
                    row = 4;
                    done = 1;
                }
            }
            break;
        }
        if (done)
            taken = 0;
    }
    if (walk)
        setSnoidAction(snoid, 10, 0);
}

/*
 * Draws the path nodes, numbered, and the next path in turn over them (a
 * debugging aid).
 */
/* @zoombi32 0x00459796 */
void drawPaths()
{
    short pathCount;
    short node;
    Paths *list;
    ShortRect rect;
    char text[8];
    Color saved;
    Color savedLabel;
    PathNodes *nodes;
    short nodeCount;
    short i;

    if (paths && pathNodes) {
        list = paths;
        nodes = pathNodes;
        pathCount = list->count;
        nodeCount = nodes->count;
        saved = setForeColor(Color(0xb));
        for (i = 0; i < nodeCount; i++) {
            rect.left = nodes->nodes[i].x - 10;
            rect.top = nodes->nodes[i].y - 10;
            rect.right = rect.left + 20;
            rect.bottom = rect.top + 20;
            fillPortRect(Rect(rect), Color(0xe), 0);
            frameRect(Rect(rect));
            intToDecimal(i + 1, text);
            drawText(Rect(rect), 0x22, text, 0xffff);
        }
        setForeColor(saved);
        if (nextPathToDraw >= pathCount)
            nextPathToDraw = 0;
        saved = setForeColor(Color(nextPathToDraw + 0x21));
        node = list->nodes[nextPathToDraw][0] - 1;
        moveTo(nodes->nodes[node].x, nodes->nodes[node].y);
        rect.left = nodes->nodes[node].x - 10;
        rect.top = nodes->nodes[node].y - 10;
        rect.right = rect.left + 20;
        rect.bottom = rect.top + 20;
        for (i = 1; i < 24; i++) {
            char next = list->nodes[nextPathToDraw][i];

            if (next) {
                lineTo(nodes->nodes[next - 1].x, nodes->nodes[next - 1].y);
                fillPortRect(Rect(rect), Color(nextPathToDraw + 0x21), 0);
                savedLabel = setForeColor(Color(0xb));
                frameRect(Rect(rect));
                intToDecimal(node + 1, text);
                drawText(Rect(rect), 0x22, text, 0xffff);
                setForeColor(savedLabel);
                moveTo(nodes->nodes[next - 1].x, nodes->nodes[next - 1].y);
                rect.left = nodes->nodes[next - 1].x - 10;
                rect.top = nodes->nodes[next - 1].y - 10;
                rect.right = rect.left + 20;
                rect.bottom = rect.top + 20;
                node = next - 1;
            } else {
                fillPortRect(Rect(rect), Color(nextPathToDraw + 0x21), 0);
                savedLabel = setForeColor(Color(0xb));
                frameRect(Rect(rect));
                intToDecimal(node + 1, text);
                drawText(Rect(rect), 0x22, text, 0xffff);
                setForeColor(savedLabel);
                i = 24;
            }
        }
        showRect(&gameRect);
        nextPathToDraw++;
        setForeColor(saved);
    }
}

/*
 * Settles a dropped Zoombini where it stands (if the terrain there is 1:
 * then it returns 1), moving it on if another is in the way.
 */
/* Not exact: the original keeps y in edx; this keeps it on the stack. */
/* @zoombi32 0x00458772 */
short settleSnoid(View *view)
{
    Snoid *snoid;
    long x;
    long y;
    short settled = 0;

    if (!(view->flags & 1))
        return settled;
    snoid = viewSnoid(view);
    *(Point *)&snoid->targetX = *(Point *)&snoid->body.x;
    if (terrain) {
        x = snoid->body.x / 4;
        y = snoid->body.y / 4;
        if (x < 0)
            x = 0;
        if (x >= terrain->width)
            x = terrain->width - 1;
        if (y < 0)
            y = 0;
        if (y >= terrain->height)
            y = terrain->height - 1;
        if (terrain->cells[terrain->rowBytes * y + x] == 1) {
            *(Point *)&snoid->targetX = *(Point *)&snoid->body.x;
            settled = 1;
        }
    }
    if (spotTaken((Point *)&snoid->targetX, view, 36))
        findSpot(view, 0, 0, 36);
    return settled;
}

/*
 * Whether the drag goes on: while the button is held, or (click to drag)
 * until it's clicked again; a quick click on a Zoombini turns the drag
 * into a click-to-drag if dragClicks allows.
 */
/* @zoombi32 0x0045ba1f */
short keepDragging()
{
    if (!dragging) {
        endDragNow = 0;
    } else if (endDragNow == 1) {
        endDragNow = 2;
        if (dragging) {
            dragging = 0;
            if (hideDragCursor)
                showCursor();
            discardEvents(3);
            return 0;
        }
    }
    short going = 1;

    if (!dragging) {
        dragging = 1;
        dragButtonDown = isButtonStillDown(buttonDown);
        clickToDrag = clickToDragOption;
        if (hideDragCursor)
            hideCursor();
    } else {
        short down = isButtonStillDown(buttonDown);

        if (dragInPlace && !down) {
            dragging = 0;
            if (hideDragCursor)
                showCursor();
            going = 0;
        } else if (clickToDrag) {
            if (isInputWaiting(2))
                down = 1;
            if (dragButtonDown && !down)
                dragButtonDown = 0;
            if (!dragButtonDown && down) {
                going = 0;
                dragging = 0;
                if (hideDragCursor)
                    showCursor();
            }
        } else if (!down) {
            if (dragClicks && clockTime() - lastClickTime < clickTime) {
                clickToDrag = 1;
                dragButtonDown = down;
            } else {
                going = 0;
                dragging = 0;
                if (hideDragCursor)
                    showCursor();
            }
        }
    }
    if (!going)
        discardEvents(3);
    return going;
}

/*
 * Switches the Zoombinis to their other look (feature images 0xc80 and the
 * alt*Images tables) and back (`restore`).
 */
/* @zoombi32 0x0045bbba */
void useAltSnoids(short restore)
{
    short i;

    if (restore) {
        if (altSnoids) {
            altSnoids = 0;
            snoidTables[0] = savedSnoidTables[0];
            snoidTables[1] = savedSnoidTables[1];
            snoidImages = savedSnoidImages;
            for (i = 0; i < 6; i++) {
                feetImages[i] = savedFeetImages[i];
                noseImages[i] = savedNoseImages[i];
                eyesImages[i] = savedEyesImages[i];
                hairImages[i] = savedHairImages[i];
            }
            for (i = 0; i < 3; i++)
                freeResource(&altSnoidResources[i]);
        }
    } else if (!altSnoids) {
        altSnoids = 1;
        savedSnoidTables[0] = snoidTables[0];
        savedSnoidTables[1] = snoidTables[1];
        savedSnoidImages = snoidImages;
        for (i = 0; i < 6; i++) {
            savedFeetImages[i] = feetImages[i];
            savedNoseImages[i] = noseImages[i];
            savedEyesImages[i] = eyesImages[i];
            savedHairImages[i] = hairImages[i];
            feetImages[i] = otherFeetImages[i];
            noseImages[i] = otherNoseImages[i];
            eyesImages[i] = otherEyesImages[i];
            hairImages[i] = otherHairImages[i];
        }
        long saved = currentMapFile;

        setCurrentMap(soundsMap);
        snoidImages = loadImageBank(0xc80, &altSnoidResources[0]);
        snoidTables[0] = loadShortTable(0xc80, &altSnoidResources[1]);
        snoidTables[1] = loadShortTable(0xc81, &altSnoidResources[2]);
        currentMapFile = saved;
    }
}

/*
 * The difficulty level (0-3) of the current scene: practiceLevel's, if set
 * (1-4), else the level reached in its group of scenes (or the scenes
 * before them).
 */
/* @zoombi32 0x0045b2d1 */
short sceneLevel()
{
    short level = 0;

    if (practiceLevel >= 1 && practiceLevel <= 4) {
        level = practiceLevel - 1;
    } else {
        short last;
        short group = sceneGroup(&last);

        if (group) {
            level = puzzleLevels()[group];
        } else {
            switch (currentScene) {
            case 0:
            case 3:
            case 4:
                level = puzzleLevels()[1];
                break;
            case 5:
                level = puzzleLevels()[2];
                if (level < puzzleLevels()[3])
                    level = puzzleLevels()[3];
                break;
            case 6:
                level = puzzleLevels()[4];
                break;
            }
        }
    }
    return level;
}

/*
 * Counts a visit to the camp in *visits (its low 12 bits) and picks the
 * hint to give: 1 at the first level; at the second, 2 then 12, once each
 * (flags 0x1000, 0x2000 in *visits); 5 if practiceLevel is set; else 0.
 */
/* @zoombi32 0x0045bdc4 */
short campHint(short *visits)
{
    short hint = 0;

    if (!practiceLevel) {
        if ((*visits & 0xfff) < 0xfff)
            (*visits)++;
        switch (sceneLevel()) {
        case 0:
            hint = 1;
            break;
        case 1:
            if (!(*visits & 0x1000)) {
                hint = 2;
                *visits |= 0x1000;
            } else if (!(*visits & 0x2000)) {
                hint = 12;
                *visits |= 0x2000;
            }
            break;
        }
    } else {
        hint = 5;
    }
    return hint;
}

/*
 * Lays out a Zoombini's cels for its script's current frame: each of the
 * frame's parts (up to 6, or 16 in state 9) is a feature layer (layers)
 * added to the part's image, mirrored when facing left (facingLeft), placed
 * by the image's hot spot. A negative word ends the frame: its low byte
 * goes in *event (and then the script moves on), and one below -0x100 is
 * followed by a sound, returned.
 */
/* Not exact: in the dead check, the original computes the view's id's
   address (`sub eax, 0x16`) before loading bank->count. */
/* @zoombi32 0x0045ab97 */
short layOutSnoid(Snoid *snoid, short *event)
{
    short *layers;
    short *hotX;
    short *hotY;
    short sound;
    short *script;
    short last;
    short offsetX;
    short offsetY;
    ShortRect rect;
    short *cel;
    short *at;
    ImageBank *bank;
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
    switch (snoid->action) {
    default:
        last = 5;
        script = baseSnoidScripts[snoid->body.script];
        at = script + snoid->body.frameOffset;
        layers++;
        offsetX = snoid->body.x;
        offsetY = snoid->body.y;
        hotX = snoidTables[0];
        hotY = snoidTables[1];
        break;
    case 8:
        last = 5;
        script = snoidScripts[snoid->body.script];
        at = script + snoid->body.frameOffset;
        layers++;
        offsetY = -snoid->body.waypointY;
        if (*at > 0) {
            offsetX = -snoid->body.waypointX;
            snoid->body.x = at[1] + offsetX;
            snoid->body.y = at[2] + offsetY;
        }
        hotX = snoidTables[0];
        hotY = snoidTables[1];
        break;
    case 9:
        last = 15;
        script = snoidScripts[snoid->body.script];
        at = script + snoid->body.frameOffset;
        if (*at <= 18)
            layers++;
        offsetY = -snoid->body.waypointY;
        if (*at > 0) {
            offsetX = -snoid->body.waypointX;
            snoid->body.x = at[1] + offsetX;
            snoid->body.y = at[2] + offsetY;
        }
        hotX = snoidTables[2];
        hotY = snoidTables[3];
        break;
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
                *cel++ = offsetX + *at++ - hotX[word];
                *cel++ = offsetY + *at++ - hotY[word];
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
                *cel++ = offsetX + *at++ - hotX[word];
                *cel++ = offsetY + *at++ - hotY[word];
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
    bank = snoid->action == 9 ? snoidImages2 : snoidImages;
    if (0) {
        for (short *check = cel; *check; check += 3)
            if (*check > bank->count) {
                debugMessage(bank->count, " ixy[].Part > ", &snoidView(snoid)->id, "Snoid id ", 1);
                *check = 1;
            }
    }
    while (*cel && *cel <= bank->count) {
        unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

        cel++;
        rect.left = *cel++;
        rect.top = *cel++;
        rect.right = swapShort(image[0]) + rect.left;
        rect.bottom = swapShort(image[1]) + rect.top;
        unionRect(&snoid->body.bounds, &rect);
    }
    if (snoid->body.clipped)
        sectRect(&snoid->body.bounds, &snoid->body.clip);
    if (event) {
        snoid->body.frameOffset = at - script;
        snoid->body.frame++;
    }
    return sound;
}

/*
 * Starts a Zoombini on snoid script `id` (state 9 or 8 by its group),
 * placed so that its first positioned frame is at `anchor`, if given.
 */
/* @zoombi32 0x0045a4f2 */
void startSnoidScript(Snoid *snoid, short id, Point *anchor, char idleTicks)
{
    short frame;
    short group;
    short index;
    short originX;
    short originY;
    short facing;
    short found;
    short *data;

    found = 0;
    groupLeader[snoid->body.group] = 0;
    snoid->body.group = 0;
    findSnoidScript(id, &group, &index);
    if (index < 0) {
        debugMessage(id, "bogus snoid script id", 0, 0, 0);
        return;
    }
    unionRgnRect(removedRgn, &snoid->body.bounds);
    snoid->drawnFacing = -1;
    switch (group) {
    case 0:
        snoid->action = 9;
        break;
    case 1:
        snoid->action = 8;
        break;
    }
    snoid->body.frame = 0;
    snoid->body.frameOffset = 2;
    snoid->pose = 0;
    snoid->body.script = index;
    snoid->body.running = 1;
    if (!snoidScripts[index])
        snoidScripts[index] = loadSwappedResource(&snoidScriptResources[index], id, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    data = snoidScripts[index];
    snoid->body.lastFrame = data[0];
    facing = data[1];
    snoid->idleTicks = idleTicks;
    originX = snoid->body.x;
    originY = snoid->body.y;
    if (anchor) {
        short n = -1;

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
    if (*data > 0) {
        snoid->body.waypointX = data[1] - originX;
        snoid->body.waypointY = data[2] - originY;
    }
    setSnoidFacing(snoid, facing);
    layOutSnoid(snoid, 0);
    unionRgnRect(removedRgn, &snoid->body.bounds);
}

/*
 * Records the party as a scene ends: the Zoombinis on screen (the chosen
 * first; all count as on board if `all`) become the party. In a puzzle
 * scene, notes how far it got: those left behind join the group's waiting
 * party, or if none were lost and the scene was passed, records the pass
 * (a dated record for the group's last scene, and the next level).
 */
/* Not exact: register allocation (the original keeps the views in edx and
   the loop counter in esi, with one fewer stack slot). */
/* @zoombi32 0x00459c84 */
void recordParty(short ending, short all)
{
    short n;
    short lost;
    short pass;
    unsigned short group;
    short last;
    unsigned short chosen;
    char ignored;
    short savedAll;
    Party *waiting;
    short i;
    View *view;

    if (!all) {
        claimOnArrival = 1;
        setSnoidMode(1);
    }
    if (viewsLocked)
        return;
    if (practiceLevel && ending) {
        party()->count = 0;
        party()->unknown2 = 1;
        party()->unknown4 = 1;
        return;
    }
    party()->unknown2 = 0;
    party()->unknown4 = 0;
    if (!all)
        for (view = viewListEnd(1); view; view = view->next)
            if (view->flags & 1) {
                view->flags = 1;
                if (!view->body.running) {
                    view->body.running = 1;
                    viewSnoid(view)->chosen = 0;
                }
                if (viewSnoid(view)->chosen)
                    viewSnoid(view)->chosen = 1;
            }
    if (leavingGame && currentScene != 4 && currentScene != 5 && currentScene != 6)
        chooseSnoids(1, 1);
    party()->count = countSnoidViews();
    chosen = 1;
    n = 0;
    savedAll = all;
    if (currentScene == 4 || currentScene == 5)
        all = 0;
    for (pass = 0; pass < 2; pass++) {
        View *actor = nextActorView(1);

        for (i = 0; i < party()->count && n < 32; actor = nextActorView(0), i++)
            if (actor) {
                unsigned short isChosen = 0;

                if (viewSnoid(actor)->chosen)
                    isChosen = 1;
                if (all || isChosen == chosen) {
                    short j;

                    for (j = 0; j < 4; j++)
                        party()->travellers[n].features[j] = viewSnoid(actor)->features[j];
                    party()->travellers[n].place = *(Point *)&actor->body.x;
                    for (j = 0; j < 10; j++)
                        party()->travellers[n].name[j] = viewSnoid(actor)->name[j];
                    party()->travellers[n].onboard = chosen || all ? 1 : 0;
                    n++;
                }
            }
        chosen = !chosen;
    }
    all = savedAll;
    lost = party()->count - countPresentTravellers();
    group = sceneGroup(&last);
    levelJustRaised = 0;
    if (group && !all) {
        short scene = currentScene - 7;

        switch (puzzleLevels()[group]) {
        case 0:
            sceneFlags()[scene] |= 1;
            break;
        case 1:
            sceneFlags()[scene] |= 2;
            break;
        case 2:
            sceneFlags()[scene] |= 4;
            break;
        case 3:
            sceneFlags()[scene] |= 8;
            break;
        }
        if (lost) {
            short from;
            short slot;
            short k;

            switch (group) {
            case 1:
                waiting = &waitingParties()[0];
                break;
            case 2:
            case 3:
                waiting = &waitingParties()[1];
                *(short *)(gameState + 0x4a) += lost;
                break;
            case 4:
                waiting = &waitingParties()[2];
                *(short *)(gameState + 0x4c) += lost;
                break;
            }
            if (waiting->unknown2)
                for (i = 0; i < waiting->count; i++)
                    if (waiting->travellers[i].onboard) {
                        waiting->count--;
                        for (k = i; k < waiting->count; k++)
                            waiting->travellers[k] = waiting->travellers[k + 1];
                        i--;
                    }
            waiting->unknown2 = waiting->unknown4 = 0;
            for (from = 0; party()->travellers[from].onboard; from++)
                ;
            slot = -1;
            for (i = 0; slot < 0 && i <= waiting->count; i++)
                if (!waiting->travellers[i].onboard || i == waiting->count)
                    slot = i;
            if (slot < 0) {
                waiting->count = 0;
                slot = 0;
            }
            for (k = 0; k < lost; k++)
                if (waiting->count < 32 && slot < 32) {
                    short j;

                    for (j = 0; j < 4; j++)
                        waiting->travellers[slot].features[j] = party()->travellers[from].features[j];
                    for (j = 0; j < 10; j++)
                        waiting->travellers[slot].name[j] = party()->travellers[from].name[j];
                    waiting->travellers[slot].onboard = 1;
                    slot++;
                    from++;
                    waiting->count++;
                }
            switch (group) {
            case 1:
                waitingParties()[0] = *waiting;
                break;
            case 2:
            case 3:
                waitingParties()[1] = *waiting;
                break;
            case 4:
                waitingParties()[2] = *waiting;
                break;
            }
            *(short *)(gameState + 0x54) = 0;
            party()->count -= lost;
        } else if (*(short *)(gameState + 0x54) && !leavingGame) {
            short done;

            switch (puzzleLevels()[group]) {
            case 0:
                sceneFlags()[scene] |= 0x10;
                break;
            case 1:
                sceneFlags()[scene] |= 0x20;
                break;
            case 2:
                sceneFlags()[scene] |= 0x40;
                break;
            case 3:
                sceneFlags()[scene] |= 0x80;
                break;
            }
            done = 0;
            for (i = 0; !done && i < 16; i++)
                if (recordGroups()[i] && recordGroups()[i] == (char)group
                    && recordLevels()[i] == (char)(puzzleLevels()[group] + 1))
                    done = 1;
            for (i = 0; last && !done && i < 16; i++)
                if (!recordGroups()[i]) {
                    getDateTime(&recordYears()[i], &recordMonths()[i], &recordDays()[i], &ignored, &ignored);
                    recordGroups()[i] = group;
                    recordLevels()[i] = puzzleLevels()[group] + 1;
                    i = 16;
                }
            if (last && puzzleLevels()[group] < 3) {
                puzzleLevels()[group]++;
                levelJustRaised = 1;
            }
        }
    }
}
