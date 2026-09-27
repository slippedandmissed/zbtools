/*
 * view (0x463034-0x465bd0): 'view port', 'gCurrentViewRgn', 'gFeatureClipRgn'
 *
 * The views ("features"): things on screen, each animated by a 'SCRB'
 * script, in a doubly linked list between two plain views, viewHead and
 * viewTail, drawn back to front. updateViews redraws what changed.
 */

#include <stdio.h>
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
    viewHead.body.clipped = 0;
    initView(&viewTail, &viewHead, 0, -1);
    viewTail.flags = 0x1000;
    viewTail.body.running = 0;
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
    viewTail.body.bounds = g_4a7bb2;
    for (short i = 0; i < 17; i++) {
        groupLeader[i] = 0;
        g_4b8b32[i] = 0;
        g_4b8b43[i] = 0;
    }
    resetSnoids();
    for (short j = 0; j < 32; j++) {
        viewSounds.sounds[j] = 0;
        viewSounds2.sounds[j] = 0;
    }
    viewSounds.active = 0;
    viewSounds2.active = 0;
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
        freePaths();
        freeScripts();
        freeTerrain();
        freeSnoidScripts();
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
            viewSounds.sounds[j] = 0;
            viewSounds2.sounds[j] = 0;
        }
        viewSounds.active = 0;
        viewSounds2.active = 0;
        g_4b9684 = 0;
        setArrivalHook(0);
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

/*
 * Redraws what changed: every view's update adds what it changes to the
 * current region; then (clipped to that) the backdrop and the views that
 * changed are drawn back to front in the work port, the sounds they ask
 * for started, and the region shown on screen.
 */
/* @zoombi32 0x0046356c */
void updateViews()
{
    basePort *saved;
    View *view;

    if (!viewsBusy) {
        viewsBusy = 1;
        saved = getPort();
        setPort(workPort);
        if (viewsPaused) {
            if (!viewsStep) {
                viewsBusy = 0;
                setPort(saved);
                return;
            }
            viewsStep = 0;
        }
        if (fillViews)
            fillPortRect(gameRect, Color(14), 0);
        if (showFps) {
            Color color;

            fpsFrames++;
            updateTime = clockTime();
            unsigned long elapsed = updateTime - fpsTime;

            if (elapsed && fpsFrames > 8) {
                unsigned long rate = fpsFrames * 600 / elapsed;

                fpsFrames = 0;
                fpsTime = updateTime;
                if (rate < fpsMin)
                    fpsMin = rate;
                if (rate > fpsMax)
                    fpsMax = rate;
                sprintf(fpsText, "%d-%d-%d", (short)fpsMin, (short)rate, (short)fpsMax);
                fillPortRect(fpsRect, Color(0xff), 0);
                color = setForeColor(Color(0));
                drawText(fpsRect, 0x22, fpsText, 0xffff);
                copyPortBits(screenPort, workPort, fpsRect, fpsRect, 0);
                setForeColor(color);
            }
        }
        unionRgn(currentViewRgn, removedRgn);
        setEmptyRgn(removedRgn);
        updateTime = clockTime();
        for (view = views; view; view = view->next)
            if (view->update)
                view->update(view, currentViewRgn);
        if (viewsSorted && !g_4b9684)
            sortViews();
        sectRgnWithRect(currentViewRgn, &gameRect);
        compactRgn(currentViewRgn);
        setClip(currentViewRgn);
        copyPortBits(workPort, viewPort, gameRect, gameRect, 0);
        for (view = views; view; view = view->next) {
            if (view->changed) {
                if (view->body.clipped)
                    unionRgnRect(currentViewRgn, &view->body.clip);
                else if (view->region)
                    unionRgn(currentViewRgn, view->region);
                else
                    unionRgnRect(currentViewRgn, &view->body.bounds);
                sectRgnWithRect(currentViewRgn, &gameRect);
                setClip(currentViewRgn);
            }
            if (view->draw)
                view->draw(view);
            view->changed = 0;
        }
        {
            short sound = playViewSounds(&viewSounds, g_4b87fe, 1);

            if (sound)
                lastViewSound = sound;
        }
        playViewSounds(&viewSounds2, g_4b87ff, 0);
        if (fillViews)
            unionRgnRect(currentViewRgn, &gameRect);
        compactRgn(currentViewRgn);
        copyRegion(screenPort, workPort, currentViewRgn);
        setEmptyRgn(currentViewRgn);
        setClipRect(gameRect);
        setPort(saved);
        if (g_4b9686) {
            if (g_4b9686 > 0)
                closeDialog(g_4b9686);
            g_4b9686 = 0;
            if (g_4b966c == 4) {
                g_4b966c = 0;
                fn_469669();
            }
            if (g_4b966c == 3) {
                g_4b966c++;
                g_4b9686 = -1;
            }
            if (g_4b966c == 2) {
                g_4b9686 = 1;
                g_4b966c++;
            }
        }
        viewsBusy = 0;
    }
}

/* @zoombi32 0x004639ec */
void initView(View *view, View *prev, View *next, short id)
{
    view->prev = prev;
    view->next = next;
    view->draw = 0;
    view->update = 0;
    view->notify = 0;
    view->placed = 0;
    view->body.bounds = noRect;
    view->body.clip = noRect;
    view->body.clipped = 0;
    view->region = 0;
    view->id = id;
    view->kind = 0;
    view->unknown1e = 0;
    view->body.script = 0;
    view->body.scriptGroup = 0;
    view->body.cels[0].image = 0;
    view->body.celsEnd = 0;
    view->body.frame = 0;
    view->body.group = 0;
    view->body.running = 1;
    view->flags = 0;
    view->reset = 1;
    view->unknown2e = 0;
    view->changed = 1;
    view->notifyEnd = 0;
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
            if (mask == (view->flags & 0x7fffffL) && ptInRect(&view->body.bounds, where))
                return view;
    } else {
        view = viewListEnd(1);
        for (view = view->next; view; view = view->next)
            if (mask == (view->flags & 0x7fffffL) && ptInRect(&view->body.bounds, where))
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
        short actors = countChosenSnoids();

        lastActorView = 0;
        for (i = 0; i < actors; i++) {
            View *view = nextActorView(0);

            if (view) {
                *(Point *)&view->body.x = viewPlaces[i];
                viewSnoid(view)->unknownF4 = 0;
            }
        }
    }
}

/* @zoombi32 0x00463e61 */
void loadDragCursors(short id)
{
    dragCursors = loadImageBank(id, &dragCursorResource);
    dragHotX = loadShortTable(id, &dragHotXResource);
    dragHotY = loadShortTable(id + 1, &dragHotYResource);
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
        unionRgnRect(removedRgn, &view->body.bounds);
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
    freeFeatureGroups();
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

/* Starts a view's script (running) by id, with whom to tell of its events. */
/* @zoombi32 0x0046483e */
View *startView(short id, short script, ViewNotify notify, char notifyEnd)
{
    View *view = findView(id);

    if (view) {
        setViewScript(view, script, 1);
        view->notify = notify;
        view->notifyEnd = notifyEnd;
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
            view->body.bounds = noRect;
        view->kind = script;
        findScript(script, &view->body.scriptGroup, &view->body.script);
    }
    if (view->body.script >= 0) {
        short *data;

        if (!scripts[view->body.script])
            scripts[view->body.script] = loadSwappedResource(&scriptResources[view->body.script],
                                                              script, RESOURCE_TYPE('S', 'C', 'R', 'B'));
        data = scripts[view->body.script];
        view->body.lastFrame = *data++ - 1;
        view->body.frame = 0;
        view->body.frameOffset = 1;
        view->nextUpdate = 0;
        view->body.running = running;
        view->changed = 1;
        view->reset = view->body.lastFrame < 1;
        view->reset = 0;
        view->unknown2e = 1;
        if (view->flags & 0x800000) {
            view->body.unknownAa = data[1];
            view->body.unknownAc = data[2];
        }
        if (removedRgn) {
            if (view->region) {
                unionRgn(removedRgn, view->region);
                setEmptyRgn(view->region);
            } else {
                unionRgnRect(removedRgn, &view->body.bounds);
            }
        }
    } else {
        view->body.running = 0;
        view->body.cels[0].image = 0;
        view->body.script = 0;
        view->body.scriptGroup = 0;
        view->body.frame = 0;
        view->body.frameOffset = 1;
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
            if (view->body.group > 0 && view->body.group < 17)
                used[view->body.group] = 1;
            else
                view->body.group = 0;
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
                    if (view->body.group && groupLeader[view->body.group] == view->id) {
                        groupLeader[view->body.group] = 0;
                        g_4b8b32[view->body.group] = 0;
                        g_4b8b43[view->body.group] = 0;
                    }
                    view->body.group = group;
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
            first->body.group = group;
            second->body.group = group;
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
        groupLeader[view->body.group] = 0;
        if (view->region) {
            unionRgn(removedRgn, view->region);
            disposeRgn(view->region);
        } else {
            unionRgnRect(removedRgn, &view->body.bounds);
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

/* Loads an image bank ('tBMP'), trying four times; its count and offsets
   big-endian. */
/* Not exact: register allocation (the original keeps the bank in edx and
   the index in ecx; BCC here swaps them). */
/* @zoombi32 0x004651ee */
ImageBank *loadImageBank(short id, long *resource)
{
    short tries = 4;
    short error = 1;

    g_4a4974 = 1;
    while (error && tries) {
        fn_46c4fe(resource, RESOURCE_TYPE('t', 'B', 'M', 'P'), id, 0, 0);
        if (*resource) {
            fn_46beac(*resource);
            error = decompressImage(fn_46beac(*resource));
            if (!error)
                error = getPortError();
        }
        if (error)
            fn_46c602(resource);
        tries--;
    }
    if (!error) {
        ImageBank *bank = (ImageBank *)fn_48ea00(fn_46beac(*resource));

        bank->count = swapShort(bank->count);
        for (short i = 1; i <= bank->count; i++)
            bank->offsets[i] = swapLong(bank->offsets[i]);
        g_4a4974 = 0;
        return bank;
    }
    fatalError("Out of memory loading SHP@");
    return 0;
}

/* Loads a resource of big-endian words, swapping them all. */
/* @zoombi32 0x004652f5 */
short *loadSwappedResource(long *resource, short id, long type)
{
    short handle;
    short *at;
    short *data;

    fn_46c4fe(resource, type, id, 0, 1);
    handle = fn_46beac(*resource);
    at = (short *)fn_48ea00(handle);
    data = at;
    for (unsigned long size = handleSize(handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
    return data;
}

/* @zoombi32 0x004655d3 */
void fadeInViews()
{
    viewsBusy = 1;
    fadePalette(g_4aabe8, 1, 0xfe, 0, 1, 0);
    viewsBusy = 0;
    viewsShown = 1;
    fn_4624f4();
}

/* @zoombi32 0x0046560b */
void fadeOutViews()
{
    if (viewsShown) {
        viewsBusy = 1;
        viewsShown = 0;
        fadePalette(0, 1, 0xfe, 0, 1, 0);
        if (g_4a48e6) {
            fillPortRect(gameRect, Color(0), 0);
            showRect(&gameRect);
        }
        viewsBusy = 0;
    }
}

/* The value of the range a sound is in (0 if none), and the range's rank
   (32 less its index). */
/* @zoombi32 0x00465692 */
short soundRangeFor(short sound, short *rank)
{
    *rank = 0;
    {
        for (short i = 0; i < soundRanges; i++)
            if (sound >= soundRangeLow[i] && sound <= soundRangeHigh[i]) {
                *rank = 32 - i;
                return soundRangeValue[i];
            }
    }
    return 0;
}

/* @zoombi32 0x004656e7 */
void addSoundRange(short low, short high, short value)
{
    if (soundRanges < 32) {
        soundRangeLow[soundRanges] = low;
        soundRangeHigh[soundRanges] = high;
        soundRangeValue[soundRanges] = value;
        soundRanges++;
    }
}

/* Keeps only the sound in the best-ranked range (the last of equals). */
/* @zoombi32 0x00465738 */
void pickViewSounds(SoundChannels *channels)
{
    short bestIndex;
    short rank;
    short bestRank;
    short keep[32];
    short best;
    short i;

    for (i = 0; i < 32; i++)
        keep[i] = 0;
    best = bestRank = 0;
    for (i = 0; i < 32; i++) {
        if (channels->sounds[i]) {
            short value = soundRangeFor(channels->sounds[i], &rank);

            if (best) {
                if (rank >= bestRank) {
                    keep[bestIndex] = 0;
                    bestIndex = i;
                    bestRank = rank;
                    best = channels->sounds[i];
                    keep[i] = 1;
                    if (value && !channels->unknown42[i])
                        channels->unknown42[i] = 1;
                }
            } else {
                keep[i] = 1;
                best = channels->sounds[i];
                bestIndex = i;
                bestRank = rank;
                if (value && !channels->unknown42[i])
                    channels->unknown42[i] = 1;
            }
        }
    }
    for (i = 0; i < 32; i++)
        if (!keep[i])
            channels->sounds[i] = 0;
}

/* Loads the sounds a view's script plays (now, with `now`). */
/* @zoombi32 0x00465a5e */
void loadViewSounds(short id, short now)
{
    short count;
    short sounds[4];
    long saved;

    if (g_4b87fe) {
        View *view = findView(id);

        if (view) {
            saved = g_4a7f58;
            count = 4;
            viewSoundList(view, &count, sounds);
            for (short i = 0; i < count; i++) {
                if (sounds[i] < 1000 || sounds[i] >= 20000)
                    fn_46be2e(g_4b7b4c);
                else
                    g_4a7f58 = saved;
                if (now)
                    fn_411382(sounds[i], RESOURCE_TYPE(0, 'S', 'N', 'D'));
                else
                    loadSoundByKey(sounds[i], RESOURCE_TYPE(0, 'S', 'N', 'D'));
            }
            g_4a7f58 = saved;
        }
    }
}

/*
 * The sounds a view's script plays (up to *count; *count becomes how many):
 * those its frames' ends name, and for a Zoombini (flag 1) those its
 * commands 201-217 ask for (fn_45b8b0 picks them for its features).
 */
/* @zoombi32 0x0046583a */
void viewSoundList(View *view, short *count, short *sounds)
{
    short frames;
    short max;
    Snoid *snoid;
    short *at;
    short left;
    short value;

    if (view) {
        if (view->flags & 1) {
            snoid = viewSnoid(view);
            switch (snoid->unknownF4) {
            default:
                at = baseSnoidScripts[snoid->body.script];
                break;
            case 8:
            case 9:
                at = snoidScripts[snoid->body.script];
                break;
            }
            frames = *at++;
            at++;
        } else {
            snoid = 0;
            at = scripts[view->body.script];
            frames = *at++;
        }
        max = *count;
        *count = 0;
        for (; frames; frames--) {
            left = 24;
            do {
                short word;

                left--;
                word = *at++;
                if (word >= 0) {
                    at += 2;
                } else {
                    if (word < -0x100) {
                        value = *at++;
                        if (value && *count < max) {
                            sounds[*count] = value;
                            (*count)++;
                        }
                    }
                    if (snoid && (word &= 0xff) != 0 && --word >= 200 && word <= 0xef) {
                        switch (word) {
                        case 200:
                            value = 8;
                            break;
                        case 201:
                            value = 6;
                            break;
                        case 202:
                            value = 7;
                            break;
                        case 203:
                            value = 10;
                            break;
                        case 204:
                            value = 2;
                            break;
                        case 205:
                            value = 12;
                            break;
                        case 206:
                            value = 1;
                            break;
                        case 207:
                            value = 9;
                            break;
                        case 208:
                            value = 0;
                            break;
                        case 209:
                            value = 4;
                            break;
                        case 210:
                            value = 5;
                            break;
                        case 211:
                            value = 3;
                            break;
                        case 212:
                            value = 11;
                            break;
                        case 213:
                            value = 13;
                            break;
                        case 214:
                            value = 14;
                            break;
                        case 215:
                            value = 15;
                            break;
                        case 216:
                            value = 16;
                            break;
                        default:
                            value = 0;
                            break;
                        }
                        if (value && *count < max) {
                            sounds[*count] = fn_45b8b0(snoid, value);
                            (*count)++;
                        }
                    }
                    if (left)
                        left = 0;
                }
            } while (left);
        }
    }
}

/* Reports a sound test: kind 1 a sound, 2 a streamed sound, 3 MIDI. */
/* @zoombi32 0x00465b19 */
void noteSoundTest(short sound, short kind)
{
    char snd[16] = "snd:";
    char streamed[16] = "s-snd:";
    char midi[20] = "midi test:";
    char text[32] = "";

    switch (kind) {
    case 1:
        strcpy(text, snd);
        break;
    case 2:
        strcpy(text, streamed);
        break;
    case 3:
        strcpy(text, midi);
        break;
    }
    intToDecimal(sound, text + strlen(text));
    showNameTag(text, 0, 1);
}

/*
 * Plays the sounds views asked for in an update: forgets finished ones,
 * picks one (with `pick`) and starts the new ones, from the sounds' map
 * (MIDI from 30000, streamed from 20000). With `played` 0, drops them all.
 * Returns the last sound started.
 */
/* @zoombi32 0x0046535f */
short playViewSounds(SoundChannels *channels, short played, short pick)
{
    long saved;
    char state;
    long type;
    short last = 0;
    short i;

    if (played) {
        if (channels->active) {
            for (i = 0; i < 32; i++) {
                if (channels->sounds[i]) {
                    state = channels->state[i];
                    channels->state[i] = 0;
                    if (isSoundPlaying(channels->sounds[i], RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                        channels->state[i] = channels->unknown42[i] + 1;
                    } else if (state) {
                        unloadSound(channels->sounds[i], RESOURCE_TYPE(0, 'S', 'N', 'D'));
                        channels->sounds[i] = 0;
                    }
                }
            }
            if (pick)
                pickViewSounds(channels);
            for (i = 0; i < 32; i++) {
                if (channels->sounds[i] && !channels->state[i]) {
                    saved = g_4a7f58;
                    type = RESOURCE_TYPE(0, 'S', 'N', 'D');
                    if (channels->sounds[i] >= 30000) {
                        fn_46be2e(g_4b7b50);
                        type = RESOURCE_TYPE('t', 'M', 'I', 'D');
                    } else if (channels->sounds[i] < 1000) {
                        fn_46be2e(g_4b7b4c);
                    } else if (channels->sounds[i] >= 20000) {
                        fn_46be2e(g_4b7b4c);
                        channels->unknown42[i] = 1;
                    }
                    if (channels->unknown42[i]) {
                        short ok;

                        last = channels->sounds[i];
                        ok = fn_411bfe(last, type, -1);
                        if (ok)
                            channels->state[i] = 2;
                        if (g_4b8803) {
                            if (!ok)
                                fn_462749(last, "Could not Get/Start s-sound ", 0, 0, 1);
                            else if (soundTests)
                                noteSoundTest(last, 2);
                        }
                    } else {
                        last = channels->sounds[i];
                        playSoundOn(last, type, -1);
                        if (g_4b8803) {
                            if (!playSoundOn(last, type, -1))
                                fn_462749(last, "Could not Get/Start sound ", 0, 0, 1);
                            else if (soundTests)
                                noteSoundTest(last, 1);
                        }
                        channels->state[i] = 1;
                    }
                    g_4a7f58 = saved;
                }
            }
            {
                short to;

                for (to = i = 0; i < 32; i++)
                    if (channels->sounds[i] && to != i) {
                        channels->sounds[to] = channels->sounds[i];
                        channels->unknown42[to] = channels->unknown42[i];
                        channels->state[to] = channels->state[i];
                        to++;
                        channels->sounds[i] = 0;
                    }
            }
        }
    } else {
        for (i = 0; i < 32; i++)
            channels->sounds[i] = 0;
    }
    channels->active = 0;
    return last;
}

/*
 * Re-sorts the views: those with flag 0x8000 stay first in their order,
 * then those with 0x4000000; the rest (0x4000000 set on them unless they
 * have 0x1000) and then the Zoombinis (kinds 1 and 2) are sorted by where
 * they stand and merged in.
 */
/* @zoombi32 0x00464da1 */
void sortViews()
{
    View *otherTail;
    View *actors;
    View *others;
    View *top;
    View *topTail;
    View *actorTail;

    {
        View *end = viewListEnd(1);
        View *next = end->next;

        end->next = 0;
        actors = others = top = 0;
        actorTail = otherTail = topTail = 0;
        while (next) {
            View *view = next;

            next = next->next;
            if (view->flags & 0x8000) {
                end->next = view;
                view->prev = end;
                view->next = 0;
                end = end->next;
            } else if (view->flags & 0x4000000) {
                if (!top) {
                    top = topTail = view;
                    topTail->prev = 0;
                    topTail->next = 0;
                } else {
                    topTail->next = view;
                    view->prev = topTail;
                    view->next = 0;
                    topTail = view;
                }
            } else if (view->flags != 1 && view->flags != 2) {
                if (!(view->flags & 0x1000))
                    view->flags |= 0x4000000;
                if (!others) {
                    others = otherTail = view;
                    otherTail->prev = 0;
                    otherTail->next = 0;
                } else {
                    otherTail->next = view;
                    view->prev = otherTail;
                    view->next = 0;
                    otherTail = view;
                }
            } else {
                if (!actors) {
                    actors = actorTail = view;
                    actorTail->prev = 0;
                    actorTail->next = 0;
                } else {
                    actorTail->next = view;
                    view->prev = actorTail;
                    view->next = 0;
                    actorTail = view;
                }
            }
        }
        next = top;
        while (next) {
            View *view = next;

            next = next->next;
            end->next = view;
            view->prev = end;
            end = end->next;
            end->next = 0;
        }
    }
    views = mergeViewList(views, sortViewList(others));
    views = mergeViewList(views, sortViewList(actors));
}

/* Sorts a list of views (by insertion) by where they stand: their bounds'
   bottom, then left; those with flag 0x1000 go last. */
/* @zoombi32 0x00464ee9 */
View *sortViewList(View *list)
{
    ShortRect other;
    ShortRect rect;
    View *sorted;

    {
        View *view;
        View *at;

        view = at = 0;
        if (list) {
            sorted = at = list;
            list = list->next;
            sorted->prev = 0;
            sorted->next = 0;
        } else {
            return 0;
        }
        while (list) {
            view = list;
            list = list->next;
            rect = view->body.bounds;
            at = sorted;
            while (at) {
                other = at->body.bounds;
                if (!(view->flags & 0x1000)
                    && (rect.bottom < other.bottom
                        || (rect.bottom == other.bottom && rect.left < other.left))) {
                    view->prev = at->prev;
                    view->next = at;
                    at->prev = view;
                    if (view->prev)
                        view->prev->next = view;
                    else
                        sorted = view;
                    at = 0;
                } else if (!at->next) {
                    at->next = view;
                    view->prev = at;
                    view->next = 0;
                    at = 0;
                } else {
                    at = at->next;
                }
            }
        }
    }
    return sorted;
}

/*
 * Merges a sorted list of views into the view list, after the views with
 * flag 0x8000: before the first view it stands in front of, unless that
 * view's flags 0x40000000, 0x10000000 or 0x20000000 keep it behind.
 */
/* @zoombi32 0x00464fc7 */
View *mergeViewList(View *into, View *list)
{
    View *after;
    View *head;
    ShortRect other;
    ShortRect rect;
    ShortRect span;

    {
        View *at;

        head = into;
        for (at = into; at->next && (at->flags & 0x8000); at = at->next)
            ;
        after = at;
        while (list) {
            View *view = list;

            list = list->next;
            rect = view->body.bounds;
            span.left = 0;
            span.right = 0x27f;
            span.top = rect.top;
            span.bottom = rect.bottom;
            at = after;
            if (view->flags & 0x1000) {
                for (; at && at->next; at = at->next)
                    ;
                at->next = view;
                view->prev = at;
                view->next = 0;
            } else {
                while (at) {
                    if (at->flags & 0x1000) {
                        view->prev = at->prev;
                        view->next = at;
                        at->prev = view;
                        view->prev->next = view;
                        at = 0;
                    } else if (!at->next) {
                        at->next = view;
                        view->prev = at;
                        view->next = 0;
                        after = view;
                        at = 0;
                    } else {
                        other = at->body.bounds;
                        if (rect.bottom < other.bottom
                            || (rect.bottom == other.bottom && rect.left < other.left)) {
                            unsigned long flags = at->flags;

                            if (rect.bottom < other.top
                                || (!((flags & 0x40000000) && rect.left < other.left)
                                    && !((flags & 0x10000000) && rect.right > other.right)
                                    && !((flags & 0x20000000) && rect.top < other.top))) {
                                view->prev = at->prev;
                                view->next = at;
                                at->prev = view;
                                if (view->prev)
                                    view->prev->next = view;
                                else
                                    head = view;
                                after = view->next;
                                at = 0;
                            }
                        }
                    }
                    if (at)
                        at = at->next;
                }
            }
        }
    }
    return head;
}

/*
 * Adds a view with a new id (one more than the highest): at the end, or
 * with `target` -3 after view g_4b8a0a, or with an id after that view (with
 * `after`) or before it. Flag 1 copies a Zoombini's body from `data`, flag
 * 2 a larger one; 0x800000 takes its place from `data`; 0x2000 records it
 * as placed there; 0x8000 makes it g_4b8a0a.
 */
/* @zoombi32 0x00463afe */
short addView(unsigned long flags, ViewDraw draw, ViewUpdate update, short kind, long interval,
              void *data, short after, short target)
{
    short id;
    View *at;
    View *view;

    for (at = views, id = 0; at; at = at->next)
        if (at->id > id)
            id = at->id;
    id++;
    {
        short found = 0;

        for (at = views; !found && at; at = at->next) {
            if (at->next && at->next->id == -1) {
                found = 1;
            } else if (target == -3 && at->id == g_4b8a0a) {
                found = 1;
            } else if (target && at->id == target) {
                if (after) {
                    found = 1;
                } else if (at->id != 1) {
                    at = at->prev;
                    found = 1;
                }
            }
            if (found) {
                if (flags & 0x8000)
                    g_4b8a0a = id;
                {
                    unsigned long size = 0xec;

                    if (flags & 1)
                        size += 0x47;
                    else if (flags & 2)
                        size += 0x16e;
                    view = (View *)newPtr(size);
                }
                if (flags & 1)
                    *viewSnoid(view) = *(Snoid *)data;
                else if (flags & 2)
                    *(LargeViewBody *)&view->body = *(LargeViewBody *)data;
                initView(view, at, at->next, id);
                at->next = view;
                view->next->prev = view;
                if (flags & 0x800000) {
                    *(Point *)&view->body.x = *(Point *)data;
                    *(Point *)&view->body.unknownAa = *(Point *)&view->body.x;
                }
                view->draw = draw;
                view->update = update;
                view->kind = kind;
                view->flags = flags;
                view->interval = interval;
                found = 1;
            }
        }
    }
    if ((flags & 0x2000) && placedViewCount < 125) {
        runViewScript(view, removedRgn);
        placedViews[placedViewCount] = id;
        placedViewPoints[placedViewCount] = *(Point *)data;
        g_4b83e4[placedViewCount] = 0;
        placedViewCount++;
    }
    return id;
}

/* Debugging: outlines and labels the views (or only the `only`th): with
   its number and whether it runs, or its id. */
/* @zoombi32 0x004645bd */
void drawViewLabels(short only)
{
    Color saved;
    View *view = viewListEnd(1);
    int n = 1;

    saved = setForeColor(Color(11));
    for (; view; view = view->next, n++) {
        short labelled = 0;

        if (!only || n == only) {
            if (!emptyRect(&view->body.bounds)) {
                char text[16];

                if (labelActorsOnly) {
                    if ((view->flags & 0xf) == 1) {
                        fillPortRect(view->body.bounds, Color(14), 0);
                        frameRect(view->body.bounds);
                        if (viewSnoid(view)->unknownF7)
                            text[0] = '+';
                        else
                            text[0] = '-';
                        text[1] = 0;
                        if (labelIds)
                            intToDecimal(view->id, text + 1);
                        labelled = 1;
                    }
                } else {
                    fillPortRect(view->body.bounds, Color(14), 0);
                    frameRect(view->body.bounds);
                    if (labelIds) {
                        intToDecimal(view->id, text);
                    } else {
                        if (view->body.running)
                            text[0] = '+';
                        else
                            text[0] = '-';
                        intToDecimal(n, text + 1);
                    }
                    if (view->id == -1) {
                        short length = strlen(text);

                        text[length++] = ':';
                        intToDecimal((short)countViews(), text + length);
                    }
                    labelled = 1;
                }
                if (labelled) {
                    drawText(view->body.bounds, 0x22, text, 0xffff);
                    copyPortBits(screenPort, workPort, view->body.bounds, view->body.bounds, 0);
                }
            }
        }
    }
    setForeColor(saved);
}
