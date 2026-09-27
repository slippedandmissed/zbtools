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
void scrollCamp(View *view, long)
{
    short steps;

    if (clockTime() < view->nextUpdate)
        return;
    view->nextUpdate = clockTime() + view->interval;
    if (view->reset) {
        view->reset = 0;
        view->unknownCe = g_4a0920;
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
            fn_4666b7(sound, 0);
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
                camp->slots[first + i].unknownC[j] = travellers()[i].unknown9[j];
        }
        appended = 1;
    } else {
        short placed = 0;

        for (i = 0; placed < count && i < 625; i++)
            if (!camp->slots[i].zoombini) {
                camp->slots[i].zoombini = travellers()[i].zoombini;
                camp->slots[i].rect = place;
                for (j = 0; j < 10; j++)
                    camp->slots[i].unknownC[j] = travellers()[i].unknown9[j];
                placed++;
            }
    }
    return appended;
}
