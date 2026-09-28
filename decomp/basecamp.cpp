/*
 * basecamp (0x415a30-0x418698): 'BaseCamp.MHK'. The range starts with
 * utilities the whole game uses: wave sounds ('tWAV') by key, two screen
 * wipes, a cheat-code tracker, number helpers and resource preloading.
 */

#include <dos.h>
#include "zoombinis.h"

#define WAVE RESOURCE_TYPE('t', 'W', 'A', 'V')

Wipe wipe;
Blinds blinds;
long cheatCode;
long cheatHash = -1;
long shapeListKind = RESOURCE_TYPE('S', 'H', 'P', 'L');
long soundListKind = RESOURCE_TYPE('S', 'N', 'D', 'L');
long noPreloadKind;
short primes[5] = {2, 3, 5, 7, 11};
short preloaded;
short preloadedCount;

/* Wave sounds (the 'tWAV' type) by key: sound's functions for them. */

/* @zoombi32 0x00415a30 */
unsigned short loadWave(short key)
{
    return loadSoundByKey(key, WAVE);
}

/* @zoombi32 0x00415a46 */
unsigned short fn_415a46(short key)
{
    return fn_411382(key, WAVE);
}

/* @zoombi32 0x00415a5c */
void unloadWave(short key)
{
    unloadSound(key, WAVE);
}

/* @zoombi32 0x00415a72 */
void fn_415a72(short key)
{
    fn_41158c(key, WAVE);
}

/* @zoombi32 0x00415a88 */
short playWaveOn(short key, short channel)
{
    return playSoundOn(key, WAVE, channel);
}

/* @zoombi32 0x00415aa3 */
short fn_415aa3(short key, short channel)
{
    return fn_411bfe(key, WAVE, channel);
}

/* @zoombi32 0x00415abe */
void stopWaves(unsigned short id)
{
    stopSounds(id, WAVE);
}

/* @zoombi32 0x00415ad4 */
void fn_415ad4(unsigned short id)
{
    fn_411e4c(id, WAVE);
}

/* @zoombi32 0x00415aea */
short isWavePlaying(unsigned short id)
{
    return isSoundPlaying(id, WAVE);
}

/* @zoombi32 0x00415b00 */
short playWave(short key, short channel, short eventType, short discard)
{
    return playSound(key, WAVE, channel, eventType, discard);
}

/* Loads a wave and plays it. */
/* @zoombi32 0x00415b25 */
short loadAndPlayWave(short key, short channel, short eventType, short discard)
{
    fn_411382(key, WAVE);
    return playSound(key, WAVE, channel, eventType, discard);
}

/* @zoombi32 0x00415b56 */
short waitForWave(unsigned short id, short eventType, short discard)
{
    return waitForSound(id, WAVE, eventType, discard);
}

/* @zoombi32 0x00415b76 */
short fn_415b76(unsigned short id, short eventType, short discard)
{
    return fn_412084(id, WAVE, eventType, discard);
}

/* @zoombi32 0x00415b96 */
short fn_415b96(unsigned short id, short stop)
{
    return fn_4120a2(id, WAVE, stop);
}

/* @zoombi32 0x00415bb1 */
short fn_415bb1(char value)
{
    return fn_4120c8(value, WAVE);
}

/* @zoombi32 0x00415bc6 */
short waitForWaveValue(char value, short eventType, short discard)
{
    return waitForSoundValue(value, WAVE, eventType, discard);
}

/* @zoombi32 0x00415be5 */
short fn_415be5(char value, short eventType, short discard)
{
    return fn_412159(value, WAVE, eventType, discard);
}

/*
 * A wipe: copies a rectangle from one port to another a line at a time,
 * over `duration` ticks, from one side (direction 0 top, 1 bottom, 2 left,
 * 3 right). Returns whether it finished (an event of `eventType` stops it,
 * finishing the copy at once, and with `discard` the events go too).
 */
/* @zoombi32 0x00415c5f */
short runWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
              short duration, unsigned short direction, short eventType, short discard)
{
    unsigned short interrupted;

    startWipe(to, from, toRect, fromRect, duration, direction);
    do {
        mainLoopEvents();
        if ((interrupted = isEventWaiting(eventType, 0)) != 0)
            break;
    } while (!stepWipe());
    if (interrupted) {
        drawWipe(wipe.steps);
        if (discard)
            discardEvents(eventType);
    }
    return !interrupted;
}

/* @zoombi32 0x00415cce */
void startWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
               short duration, unsigned short direction)
{
    wipe.from = from;
    wipe.to = to;
    wipe.fromRect = *fromRect;
    wipe.toRect = *toRect;
    wipe.direction = direction;
    if (direction <= 1)
        wipe.steps = toRect->bottom - toRect->top;
    else
        wipe.steps = toRect->right - toRect->left;
    wipe.step = fixedDiv((long)wipe.steps << 16, (long)duration << 16);
    wipe.position = wipe.step;
    wipe.done = 0;
    wipe.time = clockTime();
}

/* Advances the wipe with the clock; whether it's done. */
/* @zoombi32 0x00415d55 */
short stepWipe()
{
    if (wipe.done < wipe.steps) {
        unsigned long now = clockTime();
        unsigned long ticks = now - wipe.time;

        wipe.time = now;
        for (unsigned long tick = 0; tick < ticks; tick++)
            wipe.position += wipe.step;
        short lines = wipe.position >> 16;

        if (lines != wipe.done) {
            if (lines >= wipe.steps)
                lines = wipe.steps;
            drawWipe(lines);
        }
    }
    return wipe.done >= wipe.steps;
}

/* Copies the wipe's lines from those done to `upTo`. */
/* @zoombi32 0x00415dab */
void drawWipe(short upTo)
{
    ShortRect from = wipe.fromRect;
    ShortRect to = wipe.toRect;

    switch (wipe.direction) {
    case 0:
        from.top = wipe.fromRect.top + wipe.done;
        from.bottom = wipe.fromRect.top + upTo;
        to.top = wipe.toRect.top + wipe.done;
        to.bottom = wipe.toRect.top + upTo;
        break;
    case 1:
        from.bottom = wipe.fromRect.bottom - wipe.done;
        from.top = wipe.fromRect.bottom - upTo;
        to.bottom = wipe.toRect.bottom - wipe.done;
        to.top = wipe.toRect.bottom - upTo;
        break;
    case 2:
        from.left = wipe.fromRect.left + wipe.done;
        from.right = wipe.fromRect.left + upTo;
        to.left = wipe.toRect.left + wipe.done;
        to.right = wipe.toRect.left + upTo;
        break;
    case 3:
        from.right = wipe.fromRect.right - wipe.done;
        from.left = wipe.fromRect.right - upTo;
        to.right = wipe.toRect.right - wipe.done;
        to.left = wipe.toRect.right - upTo;
        break;
    }
    copyPortBits(wipe.to, wipe.from, to, from, 0);
    wipe.done = upTo;
}

/* Wipes the work port's `rect` onto the screen. */
/* @zoombi32 0x00415ef8 */
short wipeScreen(const ShortRect *rect, short duration, unsigned short direction, short eventType,
                 short discard)
{
    return runWipe(screenPort, workPort, rect, rect, duration, direction, eventType, discard);
}

/* @zoombi32 0x00415f29 */
void startScreenWipe(const ShortRect *rect, short duration, unsigned short direction)
{
    startWipe(screenPort, workPort, rect, rect, duration, direction);
}

/* Notes a key typed, for cheat codes: the last few keys' codes (7 bits
   each) and a running hash of them; key 1 clears them. */
/* @zoombi32 0x00415f50 */
void noteCheatKey(unsigned short key)
{
    cheatCode <<= 7;
    cheatCode |= key & 0x7f;
    cheatHash ^= cheatCode;
    if (key == 1)
        cheatHash = cheatCode = 0;
}

/* @zoombi32 0x00415f8a */
int isCheat(long hash, long code)
{
    if (hash != cheatHash || code != cheatCode)
        return 0;
    return 1;
}

/* A shape's size (its first two big-endian words); none for index 0. */
/* @zoombi32 0x00415fb0 */
void getShapeSize(ResourceList *list, unsigned short index, short *height, short *width)
{
    if (index) {
        unsigned short *data = (unsigned short *)fn_46cafb(list->resources[index - 1]);

        *height = swapShort(data[1]);
        *width = swapShort(data[0]);
    }
}

/* A random number from `low` up to `high`. */
/* @zoombi32 0x0041600d */
short randomBetween(short low, short high)
{
    return randomUpTo(high - low) + low;
}

/* Reduces a fraction, by 2 and by the primes up to 11 no larger than
   `largest`; a negative denominator moves to the numerator. */
/* @zoombi32 0x00416029 */
void reduceFraction(short *numerator, short *denominator, unsigned short largest)
{
    if (*numerator && *denominator && largest >= 2) {
        if (*denominator < 0) {
            *numerator = -*numerator;
            *denominator = -*denominator;
        }
        if (*numerator == *denominator) {
            *numerator = *denominator = 1;
        } else {
            while (!(*numerator & 1) && !(*denominator & 1)) {
                *numerator >>= 1;
                *denominator >>= 1;
            }
            for (short i = 1; i < 5 && primes[i] <= largest; i++)
                while (*numerator % primes[i] == 0 && *denominator % primes[i] == 0) {
                    *numerator /= primes[i];
                    *denominator /= primes[i];
                }
        }
    }
}

/*
 * Venetian blinds: like a wipe from the top, but in stripes `stripe` lines
 * high, all at once; without a port to copy from, the stripes are erased.
 */
/* @zoombi32 0x0041610a */
short runBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                short duration, short stripe, short eventType, short discard)
{
    unsigned short interrupted;

    startBlinds(to, from, toRect, fromRect, duration, stripe);
    do {
        mainLoopEvents();
        if ((interrupted = isEventWaiting(eventType, 0)) != 0)
            break;
    } while (!stepBlinds());
    if (interrupted) {
        drawBlinds(blinds.stripe);
        if (discard)
            discardEvents(eventType);
    }
    return !interrupted;
}

/* @zoombi32 0x00416179 */
void startBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                 short duration, short stripe)
{
    blinds.from = from;
    blinds.to = to;
    blinds.fromRect = *fromRect;
    blinds.toRect = *toRect;
    blinds.stripe = stripe;
    blinds.step = fixedDiv((long)blinds.stripe << 16, (long)duration << 16);
    blinds.position = blinds.step;
    blinds.done = 0;
    blinds.time = clockTime();
    blinds.stripes = (toRect->bottom - toRect->top - 1) / stripe + 1;
}

/* @zoombi32 0x004161fc */
short stepBlinds()
{
    if (blinds.done < blinds.stripe) {
        unsigned long now = clockTime();
        unsigned long ticks = now - blinds.time;

        blinds.time = now;
        for (unsigned long tick = 0; tick < ticks; tick++)
            blinds.position += blinds.step;
        short lines = blinds.position >> 16;

        if (lines != blinds.done) {
            if (lines >= blinds.stripe)
                lines = blinds.stripe;
            drawBlinds(lines);
        }
    }
    return blinds.done >= blinds.stripe;
}

/* @zoombi32 0x00416252 */
void drawBlinds(short upTo)
{
    ShortRect from;
    ShortRect to;
    basePort *saved = getPort();

    setPort(blinds.to);
    from = blinds.fromRect;
    to = blinds.toRect;
    short height = to.bottom - to.top;
    for (short i = 0; i < blinds.stripes; i++) {
        short top = i * blinds.stripe;
        short bottom = top + upTo < height ? upTo + top : height;

        top += blinds.done;
        from.top = blinds.fromRect.top + top;
        from.bottom = blinds.fromRect.top + bottom;
        to.top = top + blinds.toRect.top;
        to.bottom = bottom + blinds.toRect.top;
        if (blinds.from)
            copyPortBits(blinds.to, blinds.from, to, from, 0);
        else
            eraseRect(to);
    }
    blinds.done = upTo;
    setPort(saved);
}

/* @zoombi32 0x00416364 */
short blindsScreen(const ShortRect *rect, short duration, short stripe, short eventType,
                   short discard)
{
    return runBlinds(screenPort, workPort, rect, rect, duration, stripe, eventType, discard);
}

/* @zoombi32 0x00416395 */
void startScreenBlinds(const ShortRect *rect, short duration, short stripe)
{
    startBlinds(screenPort, workPort, rect, rect, duration, stripe);
}

/* Preloads a resource in the background (see preloadDone). */
/* @zoombi32 0x004163bc */
void preloadResource(long type, short id, long *kind)
{
    long resource;

    if ((resource = findResource(type, id, g_4a7f58)) != 0)
        startPreload(resource, preloadDone, *kind);
}

/* A resource preloaded: kept (up to 500, to release later), and a shape
   list's or sound list's members preloaded in turn. */
/* Not exact: the original adds the shape number in 16 bits (not lea), keeps
   the sound list's handle in edi and its count in esi, and passes each sound
   through a frame slot after pushing the type (BCC here reads it first). */
/* @zoombi32 0x004163ee */
void *preloadDone(long event, long id, void *kind)
{
    if (event == 3) {
        setResourcePurgeable(id, 1);
        if (preloadedCount < 500) {
            ((long *)handleData(preloaded))[preloadedCount] = id;
            preloadedCount++;
        }
        if (*(long *)kind == shapeListKind) {
            unsigned short *data = (unsigned short *)fn_46cafb(id);
            short count = swapShort(data[1]);
            short first = swapShort(data[0]);

            for (short i = 0; i < count; i++)
                preloadResource(RESOURCE_TYPE('S', 'H', 'A', 'P'), i + first, &noPreloadKind);
        } else if (*(long *)kind == soundListKind) {
            short handle = fn_46beac(id);
            short count = *(short *)handleData(handle);

            for (short i = 0; i < count; i++)
                preloadResource(RESOURCE_TYPE(0, 'S', 'N', 'D'), ((short *)handleData(handle))[1 + i],
                                &noPreloadKind);
        }
    }
    return 0;
}

/* Forgets the resources preloaded (releasing them, with `release`). */
/* @zoombi32 0x004164f2 */
void freePreloaded(short release)
{
    if (preloaded) {
        if (release) {
            long *ids = (long *)lockHandle(preloaded);

            for (short i = 0; i < preloadedCount; i++)
                releaseResource(ids[i], 0);
        }
        e2DisposeHandle(&preloaded);
    }
}

/* Copies a region's rectangles from one port to another. */
/* @zoombi32 0x00416541 */
void copyRegion(basePort *to, basePort *from, short region)
{
    Region *data = (Region *)lockHandle(region);

    for (short i = 0; i < data->count; i++)
        copyPortBits(to, from, data->rects[i], data->rects[i], 0);
    unlockHandle(region);
}

/* Shows a region's rectangles on screen. */
/* @zoombi32 0x004165c7 */
void showRegion(short region)
{
    Region *data = (Region *)lockHandle(region);

    for (short i = 0; i < data->count; i++)
        showRect(&data->rects[i]);
    unlockHandle(region);
}

/* @zoombi32 0x00416606 */
void getDateTime(short *year, char *month, char *day, char *hour, char *minute)
{
    struct dosdate_t date;
    struct time now;

    _dos_getdate(&date);
    *year = date.year;
    *month = date.month;
    *day = date.day;
    gettime(&now);
    *hour = now.ti_hour;
    *minute = now.ti_min;
}

/* Draws text with an outline: in `outline` offset by a pixel each way,
   then in `color` on top. */
/* @zoombi32 0x00416650 */
void drawOutlinedText(unsigned short outline, unsigned short color, ShortRect rect,
                      unsigned short flags, const char *text)
{
    ShortRect moved;

    setForeColor(Color(outline));
    for (short i = 0; i < 4; i++) {
        moved = rect;
        nudgeRect(&moved, i);
        drawText(moved, flags, text, 0xffff);
    }
    setForeColor(Color(color));
    drawText(rect, flags, text, 0xffff);
}

/* Moves a rectangle a pixel: left, up, right or down (by direction & 3). */
/* @zoombi32 0x00416707 */
void nudgeRect(ShortRect *rect, short direction)
{
    short dx;
    short dy;

    switch (direction & 3) {
    case 0:
        dx = -1;
        dy = 0;
        break;
    case 1:
        dx = 0;
        dy = -1;
        break;
    case 2:
        dx = 1;
        dy = 0;
        break;
    case 3:
        dx = 0;
        dy = 1;
        break;
    }
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
}

/* @zoombi32 0x00416754 */
void fn_416754()
{
    g_4ab518 = g_4ab51a = g_4b0d52 = 0;
    g_4ab52a = g_4ab52c = 0;
    g_4ab512 = 0;
    g_4ab52e = 0;
}

/* @zoombi32 0x00417906 */
long fn_417906(long)
{
    return 0;
}

/*
 * Enters the camp: its slots from the game's state, BaseCamp.MHK and its
 * views, the party back from the journey (if any) returned to the camp,
 * and a greeting chosen by how the journey went.
 */
/* Not exact: register allocation. The original shares ebx between the
   loop counters, `returned` and `reason`, esi between the views, `last` and
   `sound`, and edi between `appended` and `limit`; here `last` gets a frame
   slot and edi caches g_4a4ba0's address instead. */
/* @zoombi32 0x00416789 */
void enterCamp()
{
    Point places[16] = {
        {0x194, 0x14e}, {0x185, 0x162}, {0x16a, 0x153}, {0x15e, 0x16b},
        {0x143, 0x15c}, {0x130, 0x16f}, {0x118, 0x15e}, {0x106, 0x170},
        {0xee, 0x15a}, {0xdd, 0x16e}, {0xc1, 0x160}, {0xb0, 0x173},
        {0x98, 0x15f}, {0x85, 0x171}, {0x6e, 0x155}, {0x5e, 0x169},
    };
    short value;
    short saved;
    short returned;
    short reason;
    short sound;
    short limit;

    campActive = 0;
    fn_416754();
    saved = g_4b87fe;
    g_4b87fe = 0;
    g_4afb32 = 1;
    addSoundRange(20000, 29999, 1);
    addSoundRange(2000, 0x833, 0);
    addSoundRange(0x44c, 0x4af, 1);
    camp = (Camp *)(g_4a4ba0 + 0xce);
    campRow = camp->row;
    campCount = camp->count;
    campLast = campSlotsUsed();
    noteCampSlot(-1);
    openGameFile(&campMap, "BaseCamp.MHK");
    fn_46be2e(campMap);
    loadPaths(1000);
    loadDragCursors(9000);
    loadTerrain(100);
    drawBackdrop(1000);
    loadFeatureGroup(0x44c, 0, 0);
    loadFeatureGroup(0x4b0, 1, 0);
    loadScripts(0x44c, 0x10);
    addScripts(0x4b0, 0x10, 0);
    g_4a0974 = loadImageBank(2000, &campFrameResource);
    campButtonImages = loadImageBank(0x834, &campButtonsResource);
    fn_4148da(0xec, 10);
    g_4ab518 = addView(0xc000, drawCamp, scrollCamp, 0, 6, 0, 0, 0);
    addView(0x9000, drawCampButtons2, 0, 0, 0, 0, 0, 0);
    addView(0x1000, drawCampButtons1, fn_417aec, 0, 0, 0, 0, 0);
    for (short i = 0; i < 16; i++)
        placedViews[i] = addView(0x108a000, drawCels, runViewScript, i + 0x4b0, 7, &places[i], 0, 0);
    g_4ab530[0] = addView(0x1180000, drawCels, runViewScript, 0x452, 6, 0, 0, 0);
    g_4ab530[1] = addView(0x1180000, drawCels, runViewScript, 0x454, 6, 0, 0, 0);
    g_4ab530[2] = addView(0x180000, drawCels, runViewScript, 0x455, 6, 0, 0, 0);
    g_4ab530[3] = addView(0x50180000, drawCels, runViewScript, 0x456, 6, 0, 0, 0);
    g_4ab530[4] = addView(0x1101000, drawCels, runViewScript, 0x453, 6, 0, 0, 0);
    for (short k = 0x457; k <= 0x45b; k++) {
        View *view = findView(addView(0x20000, drawCels, runViewScript, k, 0, 0, 0, 0));
        if (view) {
            runViewScript(view, removedRgn);
            value = ((short *)(g_4a4ba0 + 0x14))[k - 0x457];
            view->body.frameOffset = scriptFrameOffset(scripts[view->body.script], &value, 0);
            view->body.frame = value;
            view->nextUpdate = 0;
            view->body.running = 1;
            view->body.lastFrame++;
            runViewScript(view, removedRgn);
            view->body.lastFrame--;
            view->body.frame = value;
        }
    }
    addView(0x2040000, drawCels, runViewScript, 0x450, 6, 0, 0, 0);
    for (short j = 0x44c; j <= 0x44f; j++)
        addView(0, drawCels, runViewScript, j, 0, 0, 0, 0);
    setViewPlaces(16, places, 1);
    if (party()->count)
        makePartySnoids(0);
    returned = countChosenSnoids();
    *(short *)(g_4a4ba0 + 0x4a) += returned;
    *party() = *savedParty();
    savedParty()->count = 0;
    savedParty()->unknown2 = 1;
    savedParty()->unknown4 = 1;
    if (returned) {
        if (!party()->unknown2 && fn_4572bf()) {
            short last = campLast;
            short appended = returnToCamp();

            campCount += fn_4572bf();
            campLast = campSlotsUsed();
            noteCampSlot(-1);
            if (appended) {
                campRow = (last + 1) / 5 % campRows;
                noteCampSlot(-1);
            }
            party()->unknown2 = 1;
        }
    } else {
        g_4b7562 = 1;
    }
    makePartySnoids(1);
    fn_458cc1(-20);
    updateViews();
    if (returned)
        fn_458f07(0x2d, 0x1e);
    g_4ab52e = *(short *)(g_4a4ba0 + 0x48) >= 625
               && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0xa1fc) < 16;
    if (g_4ab52e) {
        short n = countChosenSnoids();

        g_4ab524 = n && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0xa1fc) <= n;
        g_4ab526 = g_4ab524;
    } else {
        g_4ab526 = g_4ab524 = countChosenSnoids() >= 16;
    }
    setGroupLists(campGroupLists, 2, (short)0xc000);
    highlightItemAt(1, 1);
    drawCampButtons(0, 0, 0, 0);
    showRect(&g_4aa7b8);
    fadeInViews();
    campActive = 1;
    sound = 0;
    reason = -1;
    if (g_4b0d4c) {
        reason = fn_45bdc4(g_4a4ba0 + 0x30);
        g_4b0d4c = 0;
    }
    if (reason == 2 && !*(short *)(g_4a4ba0 + 0x32) && !*(short *)(g_4a4ba0 + 0x38)
        && *(short *)(g_4a4ba0 + 0x4a) <= 16) {
        reason = 1;
        *(unsigned short *)(g_4a4ba0 + 0x30) &= 0xcfff;
    }
    limit = 4;
    if (*(unsigned short *)(g_4a4ba0 + 0x30) & 0x3000)
        limit = 6;
    if (!g_4ab52e) {
        switch (reason) {
        case 0:
            switch (randomBetween(1, limit)) {
            case 1:
                sound = 0x4e51;
                break;
            case 2:
                sound = 0x4e53;
                break;
            case 3:
                sound = 0x4e55;
                break;
            case 4:
                sound = 0x4e56;
                break;
            case 5:
                sound = 0x4e52;
                break;
            case 6:
                sound = 0x4e54;
                break;
            }
            break;
        case 1:
            sound = 0x4e51;
            break;
        case 2:
            sound = 0x4e52;
            break;
        case 12:
            sound = 0x4e54;
            break;
        case 5:
            sound = 0x4e51;
            break;
        }
    } else if (reason != -1) {
        switch (randomBetween(1, 3)) {
        case 1:
            sound = 0x4e53;
            break;
        case 2:
            sound = 0x4e55;
            break;
        case 3:
            sound = 0x4e56;
            break;
        }
    }
    resetViewClock();
    g_4b87fe = saved;
    if (sound)
        queueViewSound(sound, 0);
}

/* Leaves the camp: the party that set out (or, if it was the journey's
   end or g_4a48e6 is set, none) saved, and everything the camp loaded
   freed. */
/* @zoombi32 0x00416ede */
void leaveCamp()
{
    if (campActive) {
        campActive = 0;
        short saved = fn_46bee9(1);

        clearViews();
        if (!viewsLocked) {
            if (g_4a48e6 || g_4b0d50 == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                *savedParty() = *party();
                party()->count = 0;
            } else {
                party()->unknown2 = 1;
                party()->unknown4 = 0;
                *savedParty() = *party();
                party()->unknown2 = 0;
                party()->unknown4 = 1;
                *(short *)(g_4a4ba0 + 0x4a) -= fn_4572bf();
            }
            compactCamp();
            noteCampSlot(-1);
        }
        unloadSounds();
        fn_46c602(&campButtonsResource);
        fn_46c602(&campFrameResource);
        fn_46bee9(saved);
        fn_46ca9c(&campMap);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The camp's idle work: leaving once asked to (g_4b0d52) and sound 996 is
   done, else noting which of buttons 3-6 the cursor is over. */
/* @zoombi32 0x00417000 */
void campIdle()
{
    Point where;

    if (!campBusy && campActive) {
        campBusy = 1;
        updateViews();
        if (g_4b0d52) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                campBusy = 0;
                return;
            }
            if (viewsLocked || !g_4b755a || g_4b755c >= 1) {
                g_4b0d50 = g_4b0d52;
                g_4b0d52 = 0;
                fn_46be2e(0);
                leaveCamp();
            }
        } else {
            short over = 0;

            if (!g_4ab52c && !g_4b9684) {
                getCursorPosition(&where);
                for (short i = 3; !over && i < 7; i++)
                    if (ptInRect(&campButtons[i].rect, where))
                        over = i - 2;
            }
            setDragCursor(over);
        }
        fn_43af6b();
        campBusy = 0;
    }
}

/*
 * A camp button clicked: 1 and 2 set out (with 996 and a walk to 0x13c or
 * 0x190), or say why not (a random one of three, or 0x4e51); 3 leaves; 4-7
 * scroll while held. If already leaving, leaves at once.
 */
/* @zoombi32 0x00417108 */
void campButtonClicked(short button)
{
    Point where;
    short sound;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        leaveCamp();
    } else {
        getCursorPosition(&where);
        switch (button) {
        case 1:
            if (g_4ab524) {
                queueViewSound(996, 0);
                drawCampButtons(button, 1, 0, 1);
                waitForEventFor(0, 2, 0, 1);
                drawCampButtons(button, 0, 0, 1);
                markPlacedSnoids();
                fn_4590b6(0x2a8, 0x13c, 0x2d);
                g_4b0d52 = 10;
            } else {
                if (g_4ab52e) {
                    switch (randomBetween(1, 3)) {
                    case 1:
                        sound = 0x4e53;
                        break;
                    case 2:
                        sound = 0x4e55;
                        break;
                    case 3:
                        sound = 0x4e56;
                        break;
                    }
                } else {
                    sound = 0x4e51;
                }
                queueViewSound(sound, 0);
            }
            break;
        case 2:
            if (g_4ab524) {
                queueViewSound(996, 0);
                drawCampButtons(button, 1, 0, 1);
                waitForEventFor(0, 2, 0, 1);
                drawCampButtons(button, 0, 0, 1);
                markPlacedSnoids();
                fn_4590b6(0x2a8, 0x190, 0x2d);
                g_4b0d52 = 13;
            } else {
                if (g_4ab52e) {
                    switch (randomBetween(1, 3)) {
                    case 1:
                        sound = 0x4e53;
                        break;
                    case 2:
                        sound = 0x4e55;
                        break;
                    case 3:
                        sound = 0x4e56;
                        break;
                    }
                } else {
                    sound = 0x4e51;
                }
                queueViewSound(sound, 0);
            }
            break;
        case 3:
            queueViewSound(999, 0);
            drawCampButtons(button, 1, 0, 1);
            waitForEventFor(0, 2, 0, 1);
            drawCampButtons(button, 0, 0, 1);
            g_4b0d50 = 1;
            leaveCamp();
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            g_4ab512 = button;
            drawCampButtons(button, 1, 0, 1);
            do {
                g_4a080c = button - 3;
                updateCampScroll(0);
                mainLoopEvents();
            } while (isButtonStillDown(g_4b80d0));
            updateCampScroll(1);
            g_4ab512 = 0;
            drawCampButtons(button, 0, 0, 1);
            break;
        }
    }
}

/*
 * The camp's mouse handling (action 1 down, 2 up): picking a Zoombini up
 * from its slot, dropping one in a slot (or back where it was, if it went
 * nowhere), and clicking the other things there.
 */
/* @zoombi32 0x00417350 */
void campMouse(short action)
{
    Point where;
    ShortRect spot;
    short drop;
    short dragged;
    short picked;
    short result;
    short count;
    Snoid *snoid;
    short moved;
    View *view;
    short slot;
    short i;

    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
        fn_46be2e(0);
        leaveCamp();
    } else if (!g_4ab52a || action == 2) {
        getCursorPosition(&where);
        picked = 0;
        view = 0;
        if (action == 1 && g_4b755a <= 0) {
            if ((view = viewAt(where, 1, 1)) == 0) {
                spot.left = spot.right = where.x;
                spot.top = spot.bottom = where.y;
                slot = findCampSlot(campRow, spot, 1);
                if (slot >= 0) {
                    if (campCount > 0)
                        campCount--;
                    initSnoid(&draggedSnoid);
                    draggedSnoid.zoombini = camp->slots[slot].zoombini;
                    for (i = 0; i < 10; i++)
                        draggedSnoid.name[i] = camp->slots[slot].name[i];
                    draggedSnoid.body.x = where.x;
                    draggedSnoid.body.y = where.y;
                    camp->slots[slot].zoombini = 0;
                    fn_4184b7();
                    dragged = addSnoidView(&draggedSnoid, 0);
                    if (dragged) {
                        view = findView(dragged);
                        g_4a080c = -1;
                        picked = 1;
                        action = 2;
                    }
                }
            } else {
                action = 2;
            }
        }
        switch (action) {
        case 1:
            break;
        case 2:
            if (!view && g_4b755a <= 0)
                view = viewAt(where, 1, 1);
            if (view) {
                short placed = 0;

                g_4ab51c = 0;
                g_4ab52c = 1;
                result = dragSnoid(view, where, 0, 0);
                g_4ab52c = 0;
                count = heldPlaceNumber();
                snoid = viewSnoid(view);
                moved = snoid->targetX != snoid->body.x || snoid->targetY != snoid->body.y;
                snoid->unknownF7 = count > 0;
                if (snoid->unknownF7)
                    snoid->unknownF8 = 1;
                spot = view->body.bounds;
                if (sectRect(&spot, &campArea)) {
                    spot = view->body.bounds;
                    drop = findCampSlot(campRow, spot, 0);
                    if (drop >= 0) {
                        noteCampSlot(drop);
                        camp->slots[drop].zoombini = viewSnoid(view)->zoombini;
                        for (i = 0; i < 10; i++)
                            camp->slots[drop].name[i] = viewSnoid(view)->name[i];
                        deleteView(view->id);
                        fn_4184b7();
                        g_4a080c = -1;
                        picked = 0;
                        placed = 1;
                    }
                }
                if (picked) {
                    short back = countSnoidViews() > 32;

                    if (!back && !count && moved)
                        back = 1;
                    if (back) {
                        camp->slots[slot].zoombini = draggedSnoid.zoombini;
                        for (i = 0; i < 10; i++)
                            camp->slots[slot].name[i] = draggedSnoid.name[i];
                        removeView(dragged, 1);
                    }
                    g_4a080c = -1;
                } else if (result && !count && moved && !placed) {
                    claimPlacedView(result, view->id);
                    snoid->unknownF7 = 1;
                    snoid->unknownF8 = 1;
                }
                if (g_4ab52e) {
                    short n = countChosenSnoids();

                    g_4ab524 = n && *(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0xa1fc) <= n;
                } else {
                    g_4ab524 = countChosenSnoids() >= 16;
                }
            } else if ((view = viewAt(where, 0x20000, 1)) != 0) {
                short sound = 0;

                view->body.running = 1;
                switch (view->kind) {
                case 0x457:
                    i = 0;
                    sound = 0x45e;
                    break;
                case 0x458:
                    i = 1;
                    sound = 0x45f;
                    break;
                case 0x459:
                    i = 2;
                    sound = 0x460;
                    break;
                case 0x45a:
                    i = 3;
                    sound = 0x461;
                    break;
                case 0x45b:
                    i = 4;
                    sound = 0x462;
                    break;
                }
                ((short *)(g_4a4ba0 + 0x14))[i] = view->body.frame + 1;
                ((short *)(g_4a4ba0 + 0x14))[i] %= view->body.lastFrame + 1;
                if (sound)
                    queueViewSound(sound, 0);
            } else if ((view = viewAt(where, 0x40000, 1)) != 0) {
                setViewScript(view, 0x451, 1);
                loadViewSounds(view->id, 1);
            } else {
                for (i = 0; i < 5; i++)
                    if (ptInRect(&g_4a0a34[i], where) && (view = findView(g_4ab530[i])) != 0
                        && !view->body.running) {
                        setViewScript(view, 0, 1);
                        loadViewSounds(view->id, 1);
                        i = 5;
                    }
            }
            break;
        }
    }
}

/*
 * Draws the camp's buttons: `button` (1-7), else group 1 (0-2), 2 (3-6)
 * or all, `pressed` or not (buttons 3-6 show pressed if g_4ab512 names
 * them; 0 and 1 are greyed out without g_4ab524). With `show`, shows them.
 */
/* @zoombi32 0x0041790f */
void drawCampButtons(short button, short pressed, short group, short show)
{
    short y;
    ShortRect bounds = campButtonsBounds;
    Color color;
    short toggles;
    short first;
    short last;

    toggles = 0;
    if (!button) {
        switch (group) {
        case 1:
            first = 0;
            last = 3;
            break;
        case 2:
            first = 3;
            last = 7;
            break;
        default:
            first = 0;
            last = 7;
            break;
        }
        bounds = campButtons[first].rect;
        unionRect(&bounds, &campButtons[last - 1].rect);
    } else {
        first = button - 1;
        last = first + 1;
        bounds = campButtons[button - 1].rect;
    }
    for (; first < last; first++) {
        short x = campButtons[first].rect.left;

        y = campButtons[first].rect.top;
        short image = 0;
        switch (first) {
        case 0:
            image = 1;
            if (!g_4ab524) {
                pressed = 0;
                image = 15;
            }
            break;
        case 1:
            image = 3;
            if (!g_4ab524) {
                pressed = 0;
                image = 16;
            }
            break;
        case 2:
            image = 5;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            toggles = 1;
            image = (first - 3) * 2 + 7;
            pressed = 0;
            if (g_4ab512 - 1 == first)
                pressed = 1;
            break;
        }
        if (image) {
            if (pressed)
                image++;
            drawImageData((unsigned short *)(campButtonImages->offsets[image] + (char *)campButtonImages),
                          x, y, 8);
        }
    }
    if (show) {
        if (toggles)
            drawDragCursor(viewListEnd(0));
        showRect(&bounds);
    }
}

/* The camp view's drawing: its buttons, the first group (0-2) and the
   second (3-6). */
/* @zoombi32 0x00417ac4 */
void drawCampButtons1(View *)
{
    drawCampButtons(0, 0, 1, 0);
}

/* @zoombi32 0x00417ad8 */
void drawCampButtons2(View *)
{
    drawCampButtons(0, 0, 2, 0);
}

/* @zoombi32 0x00417aec */
void fn_417aec(View *, short region)
{
    if (g_4ab524) {
        if (!g_4ab526) {
            g_4ab526 = 1;
            unionRgnRect(region, &campButtons[0].rect);
            unionRgnRect(region, &campButtons[1].rect);
        }
    } else if (g_4ab526) {
        g_4ab526 = 0;
        unionRgnRect(region, &campButtons[0].rect);
        unionRgnRect(region, &campButtons[1].rect);
    }
}

/*
 * The slot for a Zoombini in `rect`, among the five rows shown from row
 * `start`: with `occupied`, the occupied slot whose Zoombini's rectangle
 * holds the rectangle's top left; without, the empty slot (a 60-pixel
 * square around its place on screen) that `rect` overlaps most, by more
 * than 625 square pixels. -1 if none.
 */
/* @zoombi32 0x00417b56 */
short findCampSlot(short start, ShortRect rect, short occupied)
{
    unsigned short best = 0xffff;
    short i;
    short slot;
    short count;
    short row;
    ShortRect place;
    short bestArea;
    Point point;
    short column;

    start %= campRows;
    point.x = rect.left;
    point.y = rect.top;
    count = 25;
    slot = start * 5;
    column = row = 0;
    bestArea = 0;
    for (i = 0; i < count; i++, slot++) {
        short index = slot % campShown;
        short zoombini = camp->slots[index].zoombini;

        if ((occupied && zoombini) || (!occupied && !zoombini)) {
            if (zoombini) {
                place = camp->slots[index].rect;
                if (ptInRect(&place, point))
                    return index;
            } else {
                place.left = campX[row * 2 + 1] - 30;
                place.top = campY[row * 2 + 1][column] - 30;
                place.right = place.left + 60;
                place.bottom = place.top + 60;
                if (sectRect(&place, &rect)) {
                    short area = (place.right - place.left) * (place.bottom - place.top);

                    if (area > 625 && area > bestArea) {
                        bestArea = area;
                        best = index;
                    }
                }
            }
        }
        column++;
        if (column >= 5) {
            column = 0;
            row++;
        }
    }
    return best;
}

/*
 * The camp view's update: scrolls the camp as asked (g_4a080c: 1 back a
 * page, 2 back a row, 3 on a row, 4 on a page), a half row (g_4a080e) per
 * step, every view->interval.
 */
/* @zoombi32 0x00417cf1 */
void scrollCamp(View *view, short)
{
    short steps;

    if (clockTime() < view->nextUpdate)
        return;
    view->nextUpdate = clockTime() + view->interval;
    if (view->reset) {
        view->reset = 0;
        view->body.bounds = campArea;
        return;
    }
    if (!g_4a080c)
        return;
    view->changed = 1;
    steps = 1;
    switch (g_4a080c) {
    case 1:
        steps += 4;
        if (!g_4a080e && campRow - steps < 0)
            steps = 0;
    case 2:
        if (steps && !campRow)
            insertCampRow();
        for (; steps; steps--) {
            if (!g_4a080e && campRow > 0) {
                campRow--;
                if (campRow < 0) {
                    campRow = 0;
                    steps = 1;
                }
                g_4a080e = 1;
            } else {
                g_4a080e = 0;
            }
        }
        break;
    case 4:
        steps += 4;
        if (!g_4a080e && campRow + steps > campRows - 5)
            steps = 0;
    case 3:
        for (; steps; steps--) {
            if (!g_4a080e) {
                if (campRow + 1 <= 120 && campRow < campRows - 5)
                    g_4a080e = 1;
            } else {
                g_4a080e = 0;
                campRow++;
                if (campRow >= campRows - 5) {
                    campRow = campRows - 5;
                    if (campRow > 120)
                        campRow = 120;
                    steps = 1;
                }
            }
        }
        break;
    }
    if (!g_4a080e)
        g_4a080c = 0;
}

/* The camp view's drawing: the Zoombinis in the five rows shown (six while
   scrolled half a row), noting where each is, in its frame. */
/* @zoombi32 0x00417ed2 */
void drawCamp(View *)
{
    short layout;
    short count;
    short y;
    short bottom;
    short i;
    short slot;
    Snoid snoid;
    short row;
    short column;

    initSnoid(&snoid);
    campRow %= campRows;
    count = 25;
    slot = campRow * 5;
    column = row = 0;
    if (g_4a080e) {
        layout = 1;
        count += 5;
        bottom = 9;
    } else {
        layout = 3;
        bottom = 12;
    }
    drawImageData((unsigned short *)(g_4a0974->offsets[layout] + (char *)g_4a0974), 0x35, 6, 0);
    for (i = 0; i < count; i++, slot++) {
        short index = slot % campShown;

        if (camp->slots[index].zoombini) {
            short x;

            if (g_4a080e) {
                x = campX[row * 2];
                y = campY[row * 2][column];
            } else {
                x = campX[row * 2 + 1];
                y = campY[row * 2 + 1][column];
            }
            snoid.body.clipped = 0;
            snoid.unknownC0 = -1;
            snoid.body.frame = 0;
            snoid.body.frameOffset = 2;
            snoid.zoombini = camp->slots[index].zoombini;
            snoid.body.x = x;
            snoid.body.y = y;
            setSnoidFacing(&snoid, 0);
            fn_45ab97(&snoid, 0);
            camp->slots[index].rect = snoid.body.bounds;
            drawSnoid(&snoid);
        }
        column++;
        if (column >= 5) {
            column = 0;
            row++;
        }
    }
    drawImageData((unsigned short *)(g_4a0974->offsets[layout + 1] + (char *)g_4a0974), 0x35, bottom,
                  8);
    drawImageData((unsigned short *)(g_4a0974->offsets[5] + (char *)g_4a0974), 0x1f, 0, 8);
}

/*
 * The camp: 625 slots for Zoombinis, shown five to a row. A Zoombini
 * arriving in `slot` (none if negative) is counted; the rows shown then
 * reach two rows past the last used, 10 to 125 of them.
 */
/* @zoombi32 0x004180db */
void noteCampSlot(short slot)
{
    if (slot >= 0 && slot < 625 && campCount < 625) {
        campCount++;
        if (slot > campLast)
            campLast = slot;
    }
    campShown = (campLast + 10) / 5 * 5;
    if (campShown > 625)
        campShown = 625;
    if (campShown < 50)
        campShown = 50;
    campRows = campShown / 5;
    if (campRow > campRows - 5)
        campRow = campRows - 5;
    camp->row = campRow;
    camp->count = campCount;
}

/* One past the last slot used. */
/* @zoombi32 0x004181ac */
short campSlotsUsed()
{
    for (short i = 624; i >= 0; i--)
        if (camp->slots[i].zoombini)
            return i + 1;
    return 0;
}

/* Makes room for a row at the start of the camp, if the first row is used,
   the last is free and there are fewer than 125 rows shown. */
/* @zoombi32 0x004181ce */
void insertCampRow()
{
    short used = 0;
    short room = 1;
    short i;

    for (i = 0; !used && i < 5; i++)
        if (camp->slots[i].zoombini)
            used = 1;
    if (used && campRows < 125) {
        for (i = 620; room && i < 625; i++)
            if (camp->slots[i].zoombini)
                room = 0;
        if (room) {
            for (i = 619; i >= 0; i--) {
                camp->slots[i + 5] = camp->slots[i];
                camp->slots[i].zoombini = 0;
            }
            campLast += 5;
            noteCampSlot(-1);
            campRow++;
            camp->row = campRow;
            updateCampScroll(0);
        }
    }
}

/* Moves the Zoombinis up over empty rows at the start of the camp. */
/* @zoombi32 0x004182b6 */
void compactCamp()
{
    short searching = 1;
    short empty = -5;

    for (short i = 0; searching && i < 625; i++) {
        if (!camp->slots[i].zoombini)
            empty++;
        else
            searching = 0;
    }
    if (empty >= 5) {
        empty = empty / 5 * 5;
        if (empty) {
            for (short i = empty; i < 625; i++) {
                camp->slots[i - empty] = camp->slots[i];
                camp->slots[i].zoombini = 0;
            }
            campLast -= empty;
            campRow -= empty / 5;
            if (campLast < 0)
                campLast = 0;
            if (campRow < 0)
                campRow = 0;
            camp->row = campRow;
        }
    }
}

/*
 * Whether the camp can scroll the way it's asked to (g_4a080c: 1-4), with
 * the sound that starts (2000) or stops (2001) when that changes; with
 * `stop`, stops it.
 */
/* @zoombi32 0x004183ad */
void updateCampScroll(short stop)
{
    short sound = 0;

    if (g_4a080c >= 0) {
        short can = 0;

        switch (g_4a080c) {
        case 1:
            if (campRow > 4)
                can = 1;
            break;
        case 2:
            if (campRow > 0)
                can = 1;
            break;
        case 3:
            if (campRow < campRows - 5 && campRow + 1 <= 120)
                can = 1;
            break;
        case 4:
            if (campRow < campRows - 9 && campRow + 5 <= 120)
                can = 1;
            break;
        }
        if (can != g_4ab51a) {
            g_4ab51a = can;
            switch (g_4ab51a) {
            case 0:
                sound = 2001;
                break;
            case 1:
                sound = 2000;
                break;
            }
        }
        if (stop) {
            if (g_4ab51a)
                sound = 2001;
            g_4ab51a = 0;
        }
        if (sound) {
            if (sound == 2001)
                stopSounds(2000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            queueViewSound(sound, 0);
        }
    }
}

/* @zoombi32 0x004184b7 */
void fn_4184b7()
{
    View *view = findView(g_4ab518);

    if (view)
        view->nextUpdate = 0;
}

/*
 * Puts the Zoombinis back from the journey (fn_4572bf of them) in the camp,
 * at campArrival: after the last used slot if they fit (returns 1), else in
 * the empty slots from the start (returns 0).
 */
/* @zoombi32 0x004184cd */
short returnToCamp()
{
    short appended;
    short count;
    ShortRect place;
    short found;
    short first;
    short i;
    short j;

    place = campArrival;
    count = fn_4572bf();
    appended = found = 0;
    for (i = 624; !found && i >= 0; i--)
        if (camp->slots[i].zoombini) {
            found = 1;
            first = i + 1;
        }
    if (!found)
        first = 0;
    if (first + count <= 624) {
        for (i = 0; i < count; i++) {
            camp->slots[first + i].zoombini = travellers()[i].zoombini;
            camp->slots[first + i].rect = place;
            for (j = 0; j < 10; j++)
                camp->slots[first + i].name[j] = travellers()[i].name[j];
        }
        appended = 1;
    } else {
        short placed = 0;

        for (i = 0; placed < count && i < 625; i++)
            if (!camp->slots[i].zoombini) {
                camp->slots[i].zoombini = travellers()[i].zoombini;
                camp->slots[i].rect = place;
                for (j = 0; j < 10; j++)
                    camp->slots[i].name[j] = travellers()[i].name[j];
                placed++;
            }
    }
    return appended;
}
