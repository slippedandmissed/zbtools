/*
 * view (0x463034-0x465bd0): 'view port', 'gCurrentViewRgn', 'gFeatureClipRgn'
 *
 * The views ("features"): things on screen, each animated by a 'SCRB'
 * script, in a doubly linked list between two plain views, viewHead and
 * viewTail, drawn back to front. updateViews redraws what changed.
 */

#include "zoombinis.h"

/* Sets up the views: the view port, the list's ends (the tail draws the
   drag cursor) and the regions. */
/* @zoombi32 0x0046310c */
void initViews()
{
    viewsLocked = viewsShown = 0;
    soundRanges = 0;
    addSoundRange(0x3e4, 0x3e5, 0);
    createPort(&viewPort, &gameRect, 1, "view port");
    g_4b8a0a = 0;
    setViewsLocked(1);
    views = &viewHead;
    initView(&viewHead, 0, &viewTail, 1);
    viewHead.flags = 0x1008000;
    viewHead.snoid.unknownB2 = 0;
    initView(&viewTail, &viewHead, 0, -1);
    viewTail.flags = 0x1000;
    viewTail.snoid.running = 0;
    viewTail.draw = drawDragCursor;
    viewTail.update = trackDragCursor;
    if (!currentViewRgn) {
        if ((currentViewRgn = newRgn()) == 0)
            notEnoughNearMemory("gCurrentViewRgn");
    }
    if (!removedRgn) {
        if ((removedRgn = newRgn()) == 0)
            notEnoughNearMemory("gRemovedFeatureBounds");
    }
    if (!featureClipRgn) {
        if ((featureClipRgn = newRgn()) == 0)
            notEnoughNearMemory("gFeatureClipRgn");
    }
    viewTail.snoid.bounds = g_4a7bb2;
    for (short i = 0; i < 17; i++) {
        groupLeader[i] = 0;
        g_4b8b32[i] = 0;
        g_4b8b43[i] = 0;
    }
    fn_456c00();
    for (short j = 0; j < 32; j++) {
        g_4b8a0e[j] = 0;
        g_4b8a90[j] = 0;
    }
    g_4b8a0c = 0;
    g_4b8a8e = 0;
    viewsReady = 1;
}

/* @zoombi32 0x004632af */
void closeViews()
{
    short saved = fn_46bee9(1);

    setViewsLocked(0);
    viewsBusy = 1;
    clearViews();
    viewsReady = 0;
    fn_466c95();
    views = 0;
    if (currentViewRgn) {
        disposeRgn(currentViewRgn);
        currentViewRgn = 0;
    }
    if (removedRgn) {
        disposeRgn(removedRgn);
        removedRgn = 0;
    }
    if (featureClipRgn) {
        disposeRgn(featureClipRgn);
        featureClipRgn = 0;
    }
    unloadSounds();
    destroyPort(&viewPort, 1);
    fn_46bee9(saved);
}

/* Removes every view (but the list's ends) and what they use. */
/* @zoombi32 0x00463359 */
void clearViews()
{
    View *view;

    g_4a7d42 = g_4b9688 = 0;
    soundRanges = 0;
    addSoundRange(0x3e4, 0x3e5, 0);
    if (viewsReady) {
        freeDragCursors();
        if (g_4b754a) {
            party()->count = 0;
            party()->unknown2 = 1;
            party()->unknown4 = 1;
        } else {
            fn_459c84(1, 0);
        }
        view = views;
        while (view) {
            if (view->prev && view->next) {
                View *dead = view;

                view->prev->next = view->next;
                view->next->prev = view->prev;
                view = view->next;
                if (dead->region)
                    disposeRgn(dead->region);
                disposePtr(dead);
            } else {
                view = view->next;
            }
        }
        setEmptyRgn(currentViewRgn);
        setEmptyRgn(removedRgn);
        fn_4591cc();
        freeScripts();
        freeTerrain();
        fn_4571a8();
        fn_46c602(&g_4b9670);
        fn_46c602(&g_4b9674);
        for (short i = 0; i < 17; i++) {
            groupLeader[i] = 0;
            g_4b8b32[i] = 0;
            g_4b8b43[i] = 0;
        }
        g_4b754c = g_4b8a0a = 0;
        g_4b755e = 15;
        g_4b7566 = 0;
        g_4b7560 = 1;
        setViewsLocked(1);
        for (short j = 0; j < 32; j++) {
            g_4b8a0e[j] = 0;
            g_4b8a90[j] = 0;
        }
        g_4b8a0c = 0;
        g_4b8a8e = 0;
        g_4b9684 = 0;
        fn_45bfc0(0);
    }
}

/* Removes the Zoombinis' views (kind 1). */
/* @zoombi32 0x00463500 */
void removeDeadViews()
{
    View *view;

    if (viewsReady) {
        fn_459c84(0, 0);
        view = views;
        while (view) {
            if (view->prev && view->next && (view->flags & 0xf) == 1) {
                View *dead = view;

                view->prev->next = view->next;
                view->next->prev = view->prev;
                view = view->next;
                if (dead->region)
                    disposeRgn(dead->region);
                disposePtr(dead);
            } else {
                view = view->next;
            }
        }
    }
}

/* @zoombi32 0x004639ec */
void initView(View *view, View *prev, View *next, short id)
{
    view->prev = prev;
    view->next = next;
    view->draw = 0;
    view->update = 0;
    view->unknown10 = 0;
    view->unknown14 = 0;
    view->snoid.bounds = noRect;
    view->snoid.unknownB4 = noRect;
    view->snoid.unknownB2 = 0;
    view->region = 0;
    view->id = id;
    view->kind = 0;
    view->unknown1e = 0;
    view->snoid.script = 0;
    view->snoid.scriptGroup = 0;
    view->snoid.unknown0 = 0;
    view->snoid.unknown90 = 0;
    view->snoid.frame = 0;
    view->snoid.group = 0;
    view->snoid.running = 1;
    view->flags = 0;
    view->reset = 1;
    view->unknown2e = 0;
    view->changed = 1;
    view->unknown2f = 0;
    view->nextUpdate = 0;
    view->interval = 0;
}

/* Draws a list's first image as the backdrop, then frees the image. */
/* @zoombi32 0x00463ac3 */
void drawBackdropList(ResourceList *images)
{
    drawImage(images, 1, 0, 0, 0, 0x11);
    fn_46c602(&images->resources[0]);
    copyBits(viewPort, workPort, &gameRect);
}

/* The topmost view (or with `backwards`, the bottom one) of a kind (the
   low 23 bits of the flags) at a point. */
/* @zoombi32 0x00463cec */
View *viewAt(Point where, unsigned long mask, short backwards)
{
    View *view;

    mask &= 0x7fffff;
    if (backwards) {
        view = viewListEnd(0);
        for (view = view->prev; view; view = view->prev)
            if (mask == (view->flags & 0x7fffffL) && ptInRect(&view->snoid.bounds, where))
                return view;
    } else {
        view = viewListEnd(1);
        for (view = view->next; view; view = view->next)
            if (mask == (view->flags & 0x7fffffL) && ptInRect(&view->snoid.bounds, where))
                return view;
    }
    return 0;
}

/* The next view with a Zoombini (flag 1), from the start or (with `again`
   0) after the last one found. */
/* @zoombi32 0x00463d81 */
View *nextActorView(short again)
{
    View *view;

    if (again || !lastActorView) {
        view = viewListEnd(1)->next;
    } else {
        view = lastActorView->next;
        lastActorView = 0;
    }
    for (; view; view = view->next)
        if (view->flags & 1) {
            lastActorView = view;
            return view;
        }
    return 0;
}

/* Sets the places Zoombinis stand, and with `apply` moves them there. */
/* @zoombi32 0x00463dce */
void setViewPlaces(short count, Point *places, short apply)
{
    short i;

    for (i = 0; i < 125; i++)
        g_4b86d4[i] = 0;
    viewPlaceCount = count;
    for (i = 0; i < count; i++)
        viewPlaces[i] = places[i];
    if (count && apply) {
        short actors = fn_456e4c();

        lastActorView = 0;
        for (i = 0; i < actors; i++) {
            View *view = nextActorView(0);

            if (view) {
                *(Point *)&view->snoid.x = viewPlaces[i];
                view->snoid.unknownF4 = 0;
            }
        }
    }
}

/* @zoombi32 0x00463e61 */
void loadDragCursors(short id)
{
    dragCursors = loadImageBank(id, &dragCursorResource);
    dragHotX = fn_456dbe(id, &dragHotXResource);
    dragHotY = fn_456dbe(id + 1, &dragHotYResource);
}

/* Shows a drag cursor (1 up to the bank's count) in place of the mouse
   cursor, or with 0 the mouse cursor again. */
/* @zoombi32 0x00463e9e */
void setDragCursor(short cursor)
{
    if (cursor) {
        if (dragCursors && cursor <= dragCursors->count && dragCursor != cursor) {
            dragImage = 0;
            if (!dragCursor)
                hideCursor();
            dragCursor = cursor;
        }
    } else if (!cursor && dragCursor) {
        dragImage = 0;
        unionRgnRect(removedRgn, &dragRect);
        dragRect = noRect;
        dragCursor = 0;
        showCursor();
    }
}

/* @zoombi32 0x00463f40 */
void freeDragCursors()
{
    dragImage = 0;
    dragCursor = 0;
    fn_46c602(&dragCursorResource);
    fn_46c602(&dragHotXResource);
    fn_46c602(&dragHotYResource);
    dragCursors = 0;
    dragRect = noRect;
    setDragCursor(0);
}

/* viewTail's update: moves the drag cursor with the mouse. */
/* @zoombi32 0x00463f95 */
void trackDragCursor(View *, short region)
{
    short cursor = dragCursor;

    if (cursor) {
        unionRgnRect(region, &dragRect);
        getCursorPosition(&dragWhere);
        dragRect.left = dragWhere.x - dragHotX[cursor];
        dragRect.top = dragWhere.y - dragHotY[cursor];
        if (!dragImage) {
            dragImage = (unsigned short *)(dragCursors->offsets[cursor] + (char *)dragCursors);
            dragWidth = swapShort(dragImage[0]);
            dragHeight = swapShort(dragImage[1]);
        }
        dragRect.right = dragRect.left + dragWidth;
        dragRect.bottom = dragRect.top + dragHeight;
        unionRgnRect(region, &dragRect);
    } else {
        dragImage = 0;
    }
}

/* viewTail's drawing. */
/* @zoombi32 0x004640a6 */
void drawDragCursor(View *)
{
    if (dragImage)
        drawImageData(dragImage, dragRect.left, dragRect.top, 8);
}

/* viewHead (with `head`) or viewTail; none before the views are set up. */
/* @zoombi32 0x004640d1 */
View *viewListEnd(short head)
{
    if (views) {
        if (head)
            return views;
        return &viewTail;
    }
    return 0;
}

/* @zoombi32 0x004640f8 */
View *findView(short id)
{
    for (View *view = viewListEnd(0); view; view = view->prev)
        if (id == view->id)
            return view;
    return 0;
}

/* Takes a view out of the list (its area to be redrawn), and with
   `dispose` frees it (returning none). */
/* @zoombi32 0x0046411d */
View *removeView(short id, short dispose)
{
    View *view;

    if (!id)
        return 0;
    view = findView(id);
    if (view) {
        unionRgnRect(removedRgn, &view->snoid.bounds);
        view->prev->next = view->next;
        view->next->prev = view->prev;
        view->next = 0;
        view->prev = 0;
        if (dispose) {
            if (view->region)
                disposeRgn(view->region);
            disposePtr(view);
            view = 0;
        }
    }
    return view;
}

/* Puts a view last, just before viewTail. */
/* @zoombi32 0x0046418b */
void insertViewAtEnd(View *view)
{
    View *tail = viewListEnd(0);

    view->next = tail;
    view->prev = tail->prev;
    view->prev->next = view;
    tail->prev = view;
}

/*
 * Puts a view before the last view with flags 0x1000, else last. The
 * original never moves on from the view before viewTail (the search loops
 * for ever unless that one matches); this keeps that.
 */
/* @zoombi32 0x004641ac */
void insertViewBeforeCursor(View *view)
{
    View *before = viewListEnd(0)->prev;

    while (before) {
        if (before->flags == 0x1000) {
            view->next = before;
            view->prev = before->prev;
            view->prev->next = view;
            before->prev = view;
            return;
        }
    }
    insertViewAtEnd(view);
}

/* @zoombi32 0x004641e8 */
long countViews()
{
    long count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        count++;
    return count;
}

/* @zoombi32 0x00464202 */
void drawBackdrop(short id)
{
    fn_46c011(&backdropImages, id, 0, 1);
    drawBackdropList(backdropImages);
    disposeShapeList(&backdropImages);
}

/* Loads `count` scripts from id `first` as the first group (at most 300
   in all). */
/* @zoombi32 0x00464231 */
void loadScripts(short first, short count)
{
    short i;

    placedViewCount = 0;
    for (i = 0; i < 125; i++)
        g_4b83e4[i] = 0;
    scriptGroups = 0;
    if (count > 300)
        fatalError("Too many main feature SCRBs");
    for (i = 0; i < 8; i++) {
        scriptGroupFirst[i] = 0;
        scriptGroupCount[i] = 0;
    }
    for (i = 0; i < 300; i++) {
        scriptResources[i] = 0;
        scripts[i] = 0;
    }
    for (i = 0; i < count && i < 300; i++)
        scripts[i] = loadSwappedResource(&scriptResources[i], first + i, RESOURCE_TYPE('S', 'C', 'R', 'B'));
    scriptGroupFirst[scriptGroups] = first;
    scriptGroupCount[scriptGroups] = count;
    scriptGroups++;
}

/* Adds a group of `count` scripts from id `first`, loading `limit` of them
   (all if not 1 to count). */
/* @zoombi32 0x0046431e */
void addScripts(short first, short count, short limit)
{
    short i;
    short loaded;

    if (scriptGroups < 8) {
        loaded = 0;
        for (i = 0; i < scriptGroups; i++)
            loaded += scriptGroupCount[i];
        if (loaded && loaded < 300) {
            if (count + loaded > 300)
                fatalError("Too many next group feature SCRBs");
            if (limit <= 0 || limit > count)
                limit = count;
            for (i = loaded; i - loaded < limit && i < 300; i++)
                scripts[i] = loadSwappedResource(&scriptResources[i], first + i - loaded,
                                                 RESOURCE_TYPE('S', 'C', 'R', 'B'));
            scriptGroupFirst[scriptGroups] = first;
            scriptGroupCount[scriptGroups] = count;
            scriptGroups++;
        }
    }
}

/* A script id's index in scripts (-1 if none) and group. */
/* @zoombi32 0x00464409 */
void findScript(short id, short *group, short *index)
{
    short loaded;
    short i;

    *index = -1;
    loaded = 0;
    for (i = 0; i < scriptGroups; i++) {
        short first = scriptGroupFirst[i];
        short last = scriptGroupCount[i] + first - 1;

        if (id >= first && id <= last) {
            *group = i;
            *index = loaded += id - first;
            return;
        }
        loaded += scriptGroupCount[i];
    }
}

/* @zoombi32 0x00464474 */
void freeScripts()
{
    short i;

    disposeShapeList(&backdropImages);
    fn_465c81();
    for (i = 0; i < 8; i++) {
        scriptGroupFirst[i] = 0;
        scriptGroupCount[i] = 0;
    }
    for (i = 0; i < 300; i++)
        fn_46c602(&scriptResources[i]);
    for (i = 0; i < 125; i++)
        g_4b83e4[i] = 0;
    for (i = 0; i < 125; i++)
        g_4b86d4[i] = 0;
    placedViewCount = 0;
    viewPlaceCount = 0;
    scriptGroups = 0;
}

/* Loads the terrain (its first three words big-endian). */
/* @zoombi32 0x0046450c */
void loadTerrain(short id)
{
    short handle;

    loadShape(&terrainResource, id, "Terrain");
    handle = fn_46beac(terrainResource);
    terrain = (unsigned short *)fn_48ea00(handle);
    terrain[0] = swapShort(terrain[0]);
    terrain[1] = swapShort(terrain[1]);
    terrain[2] = swapShort(terrain[2]);
}

/* @zoombi32 0x004645ab */
void freeTerrain()
{
    fn_46c602(&terrainResource);
    terrain = 0;
}

/* Starts a view's script (running) by id, with its unknown10 and 2f. */
/* @zoombi32 0x0046483e */
View *startView(short id, short script, long unknown10, char unknown2f)
{
    View *view = findView(id);

    if (view) {
        setViewScript(view, script, 1);
        view->unknown10 = unknown10;
        view->unknown2f = unknown2f;
        return view;
    }
    return 0;
}

/* Gives a view a script (0: its own again), from the start. */
/* @zoombi32 0x00464876 */
void setViewScript(View *view, short script, short running)
{
    if (!view) {
        fn_462749(script, "Reset NULL viewPtr with script id", 0, 0, 0);
        return;
    }
    if (!script)
        script = view->kind;
    if ((view->flags & 0x40000) && (view->unknown2e || !view->reset)) {
        if (view->unknown1e)
            return;
        if (view->flags & 0x2000000) {
            view->flags &= 0xfdffffff;
            view->unknown1e = -view->kind;
        } else {
            view->unknown1e = view->kind;
        }
    }
    if (script != view->kind || view->reset) {
        if (view->reset)
            view->snoid.bounds = noRect;
        view->kind = script;
        findScript(script, &view->snoid.scriptGroup, &view->snoid.script);
    }
    if (view->snoid.script >= 0) {
        short *data;

        if (!scripts[view->snoid.script])
            scripts[view->snoid.script] = loadSwappedResource(&scriptResources[view->snoid.script],
                                                              script, RESOURCE_TYPE('S', 'C', 'R', 'B'));
        data = scripts[view->snoid.script];
        view->snoid.lastFrame = *data++ - 1;
        view->snoid.frame = 0;
        view->snoid.frameOffset = 1;
        view->nextUpdate = 0;
        view->snoid.running = running;
        view->changed = 1;
        view->reset = view->snoid.lastFrame < 1;
        view->reset = 0;
        view->unknown2e = 1;
        if (view->flags & 0x800000) {
            view->snoid.unknownAa = data[1];
            view->snoid.unknownAc = data[2];
        }
        if (removedRgn) {
            if (view->region) {
                unionRgn(removedRgn, view->region);
                setEmptyRgn(view->region);
            } else {
                unionRgnRect(removedRgn, &view->snoid.bounds);
            }
        }
    } else {
        view->snoid.running = 0;
        view->snoid.unknown0 = 0;
        view->snoid.script = 0;
        view->snoid.scriptGroup = 0;
        view->snoid.frame = 0;
        view->snoid.frameOffset = 1;
        fn_462749(script, "with bogus script ", &view->id, "Set Ftr id ", 1);
    }
}

/* A free group number (1-16), its state cleared; 0 if none. */
/* @zoombi32 0x00464a7d */
short freeViewGroup()
{
    short used[17];

    {
        for (short i = 0; i < 17; i++)
            used[i] = 0;
    }
    {
        for (View *view = viewListEnd(1); view; view = view->next) {
            if (view->snoid.group > 0 && view->snoid.group < 17)
                used[view->snoid.group] = 1;
            else
                view->snoid.group = 0;
        }
    }
    {
        for (short g = 1; g < 17; g++)
            if (!used[g]) {
                groupLeader[g] = 0;
                g_4b8b32[g] = 0;
                g_4b8b43[g] = 0;
                return g;
            }
    }
    return 0;
}

/* Puts up to six views (by id, 0 ending the list) in a new group, if at
   least two are given; returns the group. */
/* @zoombi32 0x00464b14 */
short groupViews(short a, short b, short c, short d, short e, short f)
{
    short ids[6];
    short group;
    short i;

    ids[0] = a;
    ids[1] = b;
    ids[2] = c;
    ids[3] = d;
    ids[4] = e;
    ids[5] = f;
    {
        short count = 0;

        for (i = 0; i < 6; i++)
            if (ids[i])
                count++;
        if (count < 2)
            return 0;
    }
    group = freeViewGroup();
    if (group) {
        for (i = 0; i < 6; i++) {
            if (ids[i]) {
                View *view = findView(ids[i]);

                if (view) {
                    if (view->snoid.group && groupLeader[view->snoid.group] == view->id) {
                        groupLeader[view->snoid.group] = 0;
                        g_4b8b32[view->snoid.group] = 0;
                        g_4b8b43[view->snoid.group] = 0;
                    }
                    view->snoid.group = group;
                }
            } else {
                return group;
            }
        }
    }
    return 0;
}

/* @zoombi32 0x00464c06 */
void pairViews(short a, short b)
{
    View *first = findView(a);
    View *second = findView(b);

    if (first && second) {
        short group = freeViewGroup();

        if (group) {
            first->snoid.group = group;
            second->snoid.group = group;
            g_4b8b43[group] = 1;
        }
    }
}

/* Removes a view and frees it, its area to be redrawn. */
/* @zoombi32 0x00464c53 */
void deleteView(short id)
{
    View *view = removeView(id, 0);

    if (view) {
        groupLeader[view->snoid.group] = 0;
        if (view->region) {
            unionRgn(removedRgn, view->region);
            disposeRgn(view->region);
        } else {
            unionRgnRect(removedRgn, &view->snoid.bounds);
        }
        disposePtr(view);
    }
}

/*
 * Where a frame starts in a script (in shorts from its start), the frame
 * number wrapped into range. A script is a frame count, then (from the
 * second word, or the third with `second`) its commands: three words, or a
 * negative word ending a frame (-0x100 and below take a word more).
 */
/* Not exact: register allocation (the original keeps the position in eax
   and the count in ecx; BCC here swaps them). */
/* @zoombi32 0x00464cbc */
long scriptFrameOffset(short *script, short *frame, short second)
{
    {
        short *at;
        short count;

        count = *script;
        at = script;
        if (second)
            at += 2;
        else
            at += 1;
        if (count <= *frame)
            *frame = count - 1;
        if (*frame < -count)
            *frame = -count;
        if (*frame < 0)
            *frame += count;
        for (count = *frame; count;) {
            short word = *at++;

            if (word >= 0) {
                at += 2;
            } else {
                if (word < -0x100)
                    at++;
                count--;
            }
        }
        return at - script;
    }
}

/* @zoombi32 0x00464d3c */
unsigned long resetViewClock()
{
    viewClockStart = viewClockMark = clockTime();
    if (g_4a4b98)
        g_4a4b98 = 0x40;
    return viewClockMark;
}

/* @zoombi32 0x00464d64 */
unsigned long viewClock()
{
    if (g_4b9684)
        return 0;
    return clockTime() - viewClockStart;
}

/* @zoombi32 0x00464d7d */
void markViewTime()
{
    viewClockMark = clockTime();
}

/* @zoombi32 0x00464d88 */
unsigned long viewTimeSinceMark()
{
    if (g_4b9684)
        return 0;
    return clockTime() - viewClockMark;
}

/* Locks the views' order (unlocking counts how often). */
/* @zoombi32 0x00465146 */
void setViewsLocked(short locked)
{
    if (!locked) {
        viewUnlocks++;
        viewsSorted = 0;
    } else {
        viewUnlocks = 0;
        viewsSorted = 1;
    }
}

/* @zoombi32 0x00465175 */
void fn_465175()
{
    viewsSorted = 1;
}

/* Moves a view next to another: after it with `after` (unless the other
   is viewTail), else before it (unless it is viewHead). */
/* @zoombi32 0x0046517f */
void moveView(short moving, short after, short anchor)
{
    if (moving && moving != anchor) {
        View *there = findView(anchor);

        if (there) {
            View *view = removeView(moving, 0);

            if (view) {
                if ((after && anchor != -1) || (!after && anchor == 1)) {
                    view->next = there->next;
                    view->prev = there;
                    there->next = view;
                    view->next->prev = view;
                } else {
                    view->prev = there->prev;
                    there->prev = view;
                    view->next = there;
                    view->prev->next = view;
                }
            }
        }
    }
}
