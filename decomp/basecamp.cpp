/*
 * basecamp (0x415a30-0x418698): 'BaseCamp.MHK'. The range starts with
 * utilities the whole game uses: wave sounds ('tWAV') by key, two screen
 * wipes, a cheat-code tracker, number helpers and resource preloading.
 */

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

/* @zoombi32 0x00417906 */
long fn_417906(long)
{
    return 0;
}
