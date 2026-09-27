/*
 * snoids (0x456c00-0x45c0f4): 'Too many snoid NORMAL scripts', 'Zoombini.MHK', name syllables
 */

#include <stdlib.h>
#include "zoombinis.h"

/* @zoombi32 0x00456c00 */
void resetSnoids()
{
    g_4b7564 = g_4b7558 = 0;
    arrivalHook = 0;
    g_4b754a = 0;
    g_4b7562 = g_4b7566 = g_4b7568 = 0;
    g_4b7552 = 0;
    g_4b7554 = 1;
    g_4b755e = 15;
    g_4b7560 = 1;
    fn_45aaff(1);
    g_4b7556 = 0;
}

/* Opens the sound files (with `files`), else loads the Zoombinis' images,
   scripts and hotspot tables. */
/* @zoombi32 0x00456c67 */
void loadSnoids(short files)
{
    if (files) {
        openGameFile(&g_4b7b50, "MidiMPC.MHK");
        openGameFile(&g_4b7b4c, "Zoombini.MHK");
        fn_46be2e(g_4b7b4c);
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
    short saved = fn_46bee9(1);

    if (g_4a4ba0) {
        disposePtr(g_4a4ba0);
        g_4a4ba0 = 0;
    }
    if (snoidTablesLoaded)
        snoidTablesLoaded = 0;
    fn_45bbba(1);
    freeBaseSnoidScripts();
    freeSnoidTables();
    fn_46c602(&snoidImagesResource);
    fn_46c602(&snoidImages2Resource);
    fn_46c602(&snoidImages3Resource);
    fn_46bee9(saved);
    fn_46ca9c(&g_4b7b4c);
    fn_46ca9c(&g_4b7b50);
}

/* Loads a table of big-endian words ('REGS'), swapping them. */
/* @zoombi32 0x00456dbe */
short *loadShortTable(short id, long *resource)
{
    short handle;
    short *at;
    short *data;

    *resource = 0;
    fn_46c4fe(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    handle = fn_46beac(*resource);
    at = (short *)fn_48ea00(handle);
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
        fn_46c602(&snoidTableResources[i]);
}

/* How many Zoombinis' views run with unknownF7 set. */
/* @zoombi32 0x00456e4c */
short countChosenSnoids()
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if ((view->flags & 1) && view->body.running && viewSnoid(view)->unknownF7)
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
        fn_46c602(&baseSnoidScriptResources[i]);
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
        fn_46c602(&snoidScriptResources[i]);
    snoidScriptGroups = 0;
    g_4b7564 = 0;
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

    snoid->unknownF1 = 1;
    snoid->unknownF2 = 0;
    snoid->body.x = x;
    snoid->body.y = y;
    snoid->targetX = targetX;
    snoid->targetY = targetY;
    snoid->body.cels[0].image = 0;
    snoid->body.celsEnd = 0;
    snoid->unknownF8 = randomBetween(0, 0x40);
    id = addSnoidView(snoid, 1);
    {
        View *view = findView(id);

        if (view)
            view->nextUpdate = when;
    }
    snoid->unknownF1 = 0;
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
            snoid->unknownC2[i] = 0;
        id = addView(1, drawSnoidView, updateSnoidView, 0, 6, snoid, 0, 0);
        {
            View *view = findView(id);

            if (view) {
                short action;

                if (!placed)
                    action = 0;
                else
                    action = 7;
                fn_45a75b(viewSnoid(view), action, 0);
                view->nextUpdate = 0;
            }
        }
    }
    return id;
}

/* A Zoombini view's drawing: its cels (from the second bank with
   unknownF4 9), clipped to its clip rectangle if it has one. */
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

        if (viewSnoid(view)->unknownF4 == 9)
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
short fn_4572bf()
{
    int count = 0;
    for (short i = 0; i < *(short *)(g_4a4ba0 + 0xa92e); i++)
        if (*(g_4a4ba0 + 0xa93c + i * 0x13) != 0)
            count++;
    return count;
}

/* @zoombi32 0x00457f96 */
void releaseHeldPlace()
{
    if (placeHeld) {
        g_4b83e4[heldPlace] = 0;
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
            g_4b83e4[n - 1] = id;
    }
}

/* @zoombi32 0x0045b39a */
void fn_45b39a(short value)
{
    g_4a4ce6 = value & 3;
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

    g_4b755c = g_4b755a = placed = 0;
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
            snoid.unknownF7 = travellers()[i].onboard;
            if (snoid.unknownF7) {
                viewPlace((Point *)&snoid.body.x, placed + 1);
                placed++;
                snoid.unknownF1 = 1;
                snoid.unknownF2 = 0;
            } else {
                *(Point *)&snoid.body.x = travellers()[i].place;
                snoid.unknownF1 = 1;
                snoid.unknownF2 = 0;
            }
            for (j = 0; j < 10; j++)
                snoid.name[j] = travellers()[i].name[j];
            snoid.home = *(Point *)&snoid.body.x;
            *(Point *)&snoid.body.unknownAa = *(Point *)&snoid.body.x;
            *(Point *)&snoid.targetX = *(Point *)&snoid.body.x;
            snoid.unknownEa = 0;
            snoid.unknownEb = 0;
            snoid.unknownEc = 0;
            snoid.unknownEe = 0;
            snoid.unknownF0 = 0;
            snoid.unknownF8 = randomBetween(0, 0x40);
            partyViews[slot] = addSnoidView(&snoid, 0);
            slot++;
        }
    }
    party()->count = 0;
}

/*
 * A Zoombini view's update, when due (in step with its group): by what it
 * is doing (unknownF4): 0 standing (blinking now and then), 1 and 2
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
    if (g_4b9684)
        return;
    {
        short due;

        if (view->body.group) {
            if (!groupLeader[view->body.group])
                groupLeader[view->body.group] = view->id;
            if (groupLeader[view->body.group] == view->id)
                g_4b8b32[view->body.group] = due = view->nextUpdate <= updateTime;
            else
                due = g_4b8b32[view->body.group];
        } else {
            due = view->nextUpdate <= updateTime;
        }
        if (!due)
            return;
    }
    view->nextUpdate = updateTime + view->interval;
    snoid = viewSnoid(view);
    switch (snoid->unknownF4) {
    case 7:
        fn_4595c2(snoid, (Point *)&snoid->targetX);
        fn_4591f8(snoid);
        snoid->unknownF4 = 0x70;
    case 0x70:
        dx = snoid->body.unknownAa - snoid->body.x;
        dy = snoid->body.y - snoid->body.unknownAc;
        moving = 1;
        if (!dx && !dy && !fn_4591f8(snoid)) {
            ShortRect rect;
            short found;
            short i;

            moving = 0;
            unionRgnRect(currentViewRgn, &snoid->body.bounds);
            snoid->unknownF8 = 0;
            fn_45a75b(snoid, g_4b756a, 0);
            if (g_4b755a > 0) {
                g_4b755a--;
                g_4b755c++;
            }
            if (g_4b7566 && snoid->unknownF7 == 2)
                view->flags |= 0x4000000;
            rect.left = snoid->body.x - g_4b755e;
            rect.right = snoid->body.x + g_4b755e;
            rect.top = snoid->body.y - g_4b755e;
            rect.bottom = snoid->body.y + g_4b755e;
            if (g_4b7554) {
                found = 0;
                for (i = 0; !found && i < placedViewCount; i++)
                    if (!g_4b83e4[i] && ptInRect(&rect, placedViewPoints[i])) {
                        found = 1;
                        g_4b83e4[i] = view->id;
                    }
            }
            found = 0;
            for (i = 0; !found && i < viewPlaceCount; i++)
                if (!g_4b86d4[i] && ptInRect(&rect, viewPlaces[i])) {
                    found = 1;
                    g_4b86d4[i] = view->id;
                }
            if (arrivalHook)
                arrivalHook(view->id);
        }
        stepX = snoid->unknownEc;
        stepY = snoid->unknownEe;
        changed = 1;
        if (!moving)
            break;
        if (dx) {
            if (dx < 0) {
                snoid->body.x -= abs(dx) < abs(stepX) ? abs(dx) : abs(stepX);
                snoid->unknownF2 = 1;
            } else {
                snoid->body.x += abs(dx) < abs(stepX) ? abs(dx) : abs(stepX);
                snoid->unknownF2 = 0;
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
        snoid->unknownF8 = 0;
        if (snoid->unknownF4 == 1) {
            if (!snoid->unknownF2) {
                switch (snoid->unknownF1) {
                case 2:
                    snoid->unknownF1 = 1;
                    break;
                case 1:
                default:
                    snoid->unknownF1 = 0;
                    snoid->unknownF2 = 1;
                    break;
                }
            } else {
                snoid->unknownF1 = 1;
                snoid->unknownF4 = 0;
            }
        } else if (snoid->unknownF2) {
            switch (snoid->unknownF1) {
            case 2:
                snoid->unknownF1 = 1;
                break;
            case 1:
            default:
                snoid->unknownF1 = 0;
                snoid->unknownF2 = 0;
                break;
            }
        } else {
            snoid->unknownF1 = 1;
            snoid->unknownF4 = 0;
        }
        changed = 1;
    case 0:
        if (snoid->unknownF5) {
            snoid->unknownF5 = 0;
            changed = 1;
        }
        if (g_4a4b98 && !g_4b9684 && snoid->unknownF8++ > g_4a4b98) {
            snoid->unknownF8 = 0;
            if (!g_4a4cea && randomBetween(0, 100) < 10) {
                snoid->unknownF5 = randomBetween(0, 7);
                fn_45a75b(snoid, 6, 0);
                changed = 1;
            }
        }
        break;
    case 4:
        changed = 1;
        if (snoid->targetX - snoid->body.x || snoid->body.y - snoid->targetY) {
            snoid->unknownF1 = 1;
            snoid->unknownF2 = 0;
            *(Point *)&snoid->body.x = *(Point *)&snoid->targetX;
        } else {
            view->interval = 6;
            snoid->unknownF8 = 0;
            fn_45a75b(snoid, g_4b756a, 0);
        }
        break;
    case 8:
    case 9:
        changed = 1;
        break;
    case 3:
        if (snoid->unknownF5 < 6) {
            for (short j = 0; j <= 4; j++) {
                short image = snoid->body.cels[j].image;

                snoid->body.cels[j].image = snoid->body.cels[j + 10].image;
                snoid->body.cels[j + 10].image = image;
            }
            snoid->unknownF5++;
        } else {
            fn_45a75b(snoid, 0, 0);
            snoid->unknownF8 = 0;
        }
        view->changed = 1;
        return;
    case 10:
        fn_45a75b(snoid, 7, 0);
        g_4b755a++;
        return;
    }
    if (!changed)
        return;
    unionRgnRect(region, &view->body.bounds);
    if (snoid->unknownF4 == 4) {
        fn_45ab97(snoid, 0);
    } else if (snoid->body.lastFrame > 1) {
        if (snoid->body.frame >= snoid->body.lastFrame) {
            switch (snoid->unknownF4) {
            case 5:
                view->notify = 0;
                snoid->body.frame = 2;
                if (g_4a4cea)
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
                if (snoid->unknownF8 == 1) {
                    groupLeader[view->body.group] = 0;
                    view->body.group = 0;
                    snoid->body.running = 0;
                    snoid->body.frame = 0;
                    snoid->body.frameOffset = 2;
                } else {
                    unionRgnRect(currentViewRgn, &snoid->body.bounds);
                    fn_45a75b(snoid, 0, 0);
                }
                if (view->notifyEnd && view->notify)
                    view->notify(view, -1);
                view->notify = 0;
                view->changed = 1;
                return;
            default:
                unionRgnRect(currentViewRgn, &snoid->body.bounds);
                fn_45a75b(snoid, 0, 0);
                view->notify = 0;
                view->changed = 1;
                return;
            }
        }
        {
            short sound = fn_45ab97(snoid, &event);

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
                    queueViewSound(fn_45b8b0(snoid, i), 0);
                event = 0;
            } else if (view->notify) {
                view->notify(view, event);
            }
        }
    } else {
        fn_45ab97(snoid, 0);
    }
    view->changed = 1;
}
