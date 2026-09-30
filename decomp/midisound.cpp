/*
 * midisound (Mohawk engine): the MIDI mapper's messages, and MIDI sounds
 * (a sequencer for Mohawk MIDI files)
 */

/* @flags -p -x- */

#include <stdlib.h>

#include "zoombinis.h"
#include "os_fixed.h"
#include "os_refcount.h"

static unsigned short midiUsers;

static char setupEndMarker[] = "setup end";
static char loopStartMarker[] = "loop start";
static char loopEndMarker[] = "loop end";

/* midiOutLongMsg for a map: sysex messages go to the device, except the
   engine's own (F0 00 00 34 target command channel), which mute and unmute
   channels; other messages are mapped one by one. */
/* Not exact: the original keeps `other` in ebx (after `p`) and `channel` on the
   stack; BCC32 4.5 does the reverse. */
/* @zoombi32 0x00478814 */
short midiMapLongMsg(long handle, MidiHeader *header, unsigned short size)
{
    MidiMap *map;
    MidiDevice *device;
    short error;
    unsigned char *data;
    unsigned long length;
    unsigned char *end;
    unsigned char status;
    int channel;
    unsigned char *p;
    unsigned char *q;
    MidiMap *other;

    if (!(header->dwFlags & MHDR_PREPARED))
        return MIDIERR_UNPREPARED;
    map = midiMap(handle);
    if (!map)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    device = map->device;
    error = 0;
    data = (unsigned char *)header->lpData;
    length = header->dwBufferLength;
    p = data;
    end = p + length;
    status = 0;
    while (p < end) {
        if (*p == 0xf0) {
            status = 0;
            for (q = p + 1; *q != 0xf7; q++)
                ;
            if (!p[1] && !p[2] && p[3] == 0x34) {
                short target = p[4] & 0x40 ? p[4] | 0xff80 : p[4];
                if (!target || target == map->device->target)
                    switch (p[5]) {
                    case 0:
                        memset(map->channelMuted, 0, sizeof map->channelMuted);
                        break;
                    case 1:
                        channel = p[6] & 0xf;
                        map->channelMuted[channel] = 1;
                        resetChannel(map, channel);
                        break;
                    case 2:
                        map->channelMuted[p[6] & 0xf] = 0;
                        break;
                    case 0x10:
                        memset(device->muted, 0, sizeof device->muted);
                        break;
                    case 0x11:
                        channel = p[6] & 0xf;
                        device->muted[channel] = 1;
                        other = device->maps;
                        do {
                            resetChannel(other, channel);
                            other = other->next;
                        } while (other != device->maps);
                        break;
                    case 0x12:
                        device->muted[p[6] & 0xf] = 0;
                        break;
                    }
            } else {
                header->lpData = (LPSTR)p;
                header->dwBufferLength = q - p + 1;
                if ((error = midiOutLongMsg(device->out, header, size)) != 0)
                    break;
                while (!(header->dwFlags & MHDR_DONE))
                    continue;
            }
            p = q + 1;
            continue;
        }
        if (*p > 0xf0) {
            error = MMSYSERR_INVALPARAM;
            break;
        }
        if (*p & 0x80)
            status = *p++;
        else if (!status) {
            error = MMSYSERR_INVALPARAM;
            break;
        }
        switch (status & 0xf0) {
        case 0x80:
        case 0x90:
        case 0xa0:
        case 0xb0:
        case 0xe0:
            error = mapShortMsg(map, status | (unsigned short)p[0] << 8 | (unsigned long)p[1] << 16);
            p += 2;
            break;
        case 0xc0:
        case 0xd0:
            error = mapShortMsg(map, status | (unsigned short)p[0] << 8);
            p++;
            break;
        default:
            error = MMSYSERR_INVALPARAM;
        }
        if (error)
            break;
    }
    header->lpData = (LPSTR)data;
    header->dwBufferLength = length;
    return error;
}

/* Turns off the notes a map has on in a channel. */
/* @zoombi32 0x00478b09 */
void resetChannel(MidiMap *map, int channel)
{
    int mapped = map->device->channels[channel];
    int key;

    for (key = 0; key < 128; key++)
        while (map->notes[channel][key]) {
            midiOutShortMsg(map->device->out, (mapped | 0x80) + (key << 8));
            map->notes[channel][key]--;
            map->device->playingCount--;
        }
}

/* Sends a short message through a map: muted channels are dropped, note-on
   velocities follow the map's curve, notes on are counted (so they can be
   turned off), and the channel is remapped. */
/* @zoombi32 0x00478b7f */
short mapShortMsg(MidiMap *map, unsigned long message)
{
    unsigned char velocity;
    int key;
    int channel = message & 0xf;
    int type = message & 0xf0;
    unsigned char scaled;
    int mapped;

    switch (type) {
    case 0x90:
        if ((velocity = message >> 16 & 0x7f) == 0)
            goto noteOff;
        if (map->channelMuted[channel] || map->device->muted[channel])
            return 0;
        key = (unsigned short)message >> 8;
        if (map->notes[channel][key] == 0xff)
            return 0;
        if ((scaled = map->velocities[velocity]) != velocity) {
            if ((velocity = scaled) == 0)
                return 0;
            message = message & 0xff00ffff | (unsigned long)velocity << 16;
        }
        map->notes[channel][key]++;
        map->device->playingCount++;
        break;
    case 0x80:
    noteOff:
        if (map->channelMuted[channel] || map->device->muted[channel])
            return 0;
        key = (unsigned short)message >> 8;
        if (!map->notes[channel][key])
            return 0;
        if (type == 0x80) {
            velocity = message >> 16 & 0x7f;
            if (map->velocities[velocity] != velocity)
                message = message & 0xff00ffff | (unsigned long)velocity << 16;
        }
        map->notes[channel][key]--;
        map->device->playingCount--;
        break;
    }
    mapped = map->device->channels[channel];
    if (mapped != channel)
        message = message & 0xfffffff0 | mapped;
    return midiOutShortMsg(map->device->out, message);
}

/* Not exact: the original keeps `map` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00478d09 */
short midiMapShortMsg(long handle, unsigned long message)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    return mapShortMsg(map, message);
}

/* @zoombi32 0x00478d38 */
MidiMap *midiMap(long handle)
{
    MidiMap *map = (MidiMap *)handle;

    if (map && map->tag == 0x4d4d6170)
        return map;
    return 0;
}

/* Caches (or uncaches) the drum keys and patches the file lists, on
   devices that cache. */
/* @zoombi32 0x00478d50 */
short __cdecl midiObj::cachePatches(short cache)
{
    unsigned short drumChannel;
    WORD cachedKeys[128];
    WORD cachedPatches[128];
    WORD mask;
    int i;
    short error;

    if (!keys && !patches || !(caps.dwSupport & MIDICAPS_CACHE))
        return 0;
    error = 0;
    if (keys) {
        getDrumChannel(device, &drumChannel);
        mask = 1 << drumChannel;
        memset(cachedKeys, 0, sizeof cachedKeys);
        for (i = 0; i < byteSwapShort(*(unsigned short *)(keys + 8)); i++)
            cachedKeys[byteSwapShort(*(unsigned short *)(keys + i * 4 + 0xa)) & 0x7f] =
                byteSwapShort(*(unsigned short *)(keys + i * 4 + 0xc)) & mask;
        if ((error = midiMapCacheDrumPatches((long)map, 0, cachedKeys,
                                             cache ? MIDI_CACHE_ALL : MIDI_UNCACHE)) != 0
            && error != MMSYSERR_NOTSUPPORTED)
            return error;
    }
    if (patches) {
        memset(cachedPatches, 0, sizeof cachedPatches);
        for (i = 0; i < byteSwapShort(*(unsigned short *)(patches + 8)); i++) {
            WORD banks = byteSwapShort(*(unsigned short *)(patches + i * 4 + 0xc));
            cachedPatches[byteSwapShort(*(unsigned short *)(patches + i * 4 + 0xa)) & 0x7f] = banks;
        }
        if ((error = midiMapCachePatches((long)map, 0, cachedPatches,
                                         cache ? MIDI_CACHE_ALL : MIDI_UNCACHE)) != 0
            && error != MMSYSERR_NOTSUPPORTED && keys && cache == 1)
            midiMapCacheDrumPatches((long)map, 0, cachedKeys, MIDI_UNCACHE);
    }
    return error;
}

/* A MIDI sound from a Mohawk MIDI file in a handle: finds its tracks and
   key and patch lists, and reads it to its start. */
/* Not exact: the original keeps `tracks` in ebx (before `midi`); BCC32 4.5 puts it
   on the stack. */
/* @zoombi32 0x00478f0b */
audioObj *__cdecl newMidiSound(short data)
{
    unsigned long size;
    unsigned char *end;
    unsigned short format;
    unsigned short division;
    midiObj *midi;
    unsigned long *file;
    unsigned char *header;
    unsigned short tracks;
    unsigned char *chunk;

    file = (unsigned long *)lockHandleAlias(data);
    size = byteSwapLong(file[1]) + 8;
    end = (unsigned char *)file + (size + 1 & ~1);
    if (byteSwapLong(file[0]) != 0x4d48574b || byteSwapLong(file[2]) != 0x4d494449) {
        unlockHandle(data);
        setSoundError(0x29d0);
        return 0;
    }
    header = (unsigned char *)(file + 3);
    format = byteSwapShort(*(unsigned short *)(header + 8));
    tracks = byteSwapShort(*(unsigned short *)(header + 0xa));
    division = byteSwapShort(*(unsigned short *)(header + 0xc));
    if (byteSwapLong(*(unsigned long *)header) != 0x4d546864 || division & 0x8000 || format >= 2
        || !tracks) {
        unlockHandle(data);
        setSoundError(0x29d1);
        return 0;
    }
    if ((midi = (midiObj *)newPtr((unsigned short)(tracks * 16 + 0x184))) == 0) {
        unlockHandle(data);
        setSoundError(memError());
        return 0;
    }
    new (midi) midiObj;
    midi->tag = 0x414f626a;
    midi->kind = 0;
    midi->rate = makeFixed(1, 0);
    midi->volume = makeFixed(1, 0);
    midi->active = sound.active;
    midi->data = data;
    midi->file = file;
    midi->fileSize = size;
    midi->format = format;
    midi->division = division;
    chunk = header + byteSwapLong(*(unsigned long *)(header + 4)) + 8;
    do {
        switch (byteSwapLong(*(unsigned long *)chunk)) {
        case 0x4d54726b: {
            MidiTrack *track = &midi->tracks[midi->trackCount++];
            track->start = chunk + 8;
            break;
        }
        case 0x4b657923:
            if (midi->keys) {
                setSoundError(0x29d0);
                goto fail;
            }
            midi->keys = chunk;
            break;
        case 0x50726723:
            if (midi->patches) {
                setSoundError(0x29d0);
                goto fail;
            }
            midi->patches = chunk;
            break;
        }
        chunk = chunk + (byteSwapLong(*(unsigned long *)(chunk + 4)) + 1 & ~1) + 8;
    } while (chunk < end);
    if (chunk > end) {
        setSoundError(0x29d0);
    fail:
        disposePtr(midi);
        unlockHandle(data);
        return 0;
    }
    midi->seek(0);
    midi->seek(-1);
    midi->audioObj::looping = midi->loopEnd != 0; /* the sound loops (not midiObj::looping) */
    midi->seek(0);
    midi->resetLoop();
    return midi;
}

/* @zoombi32 0x004791a1 */
void __cdecl midiObj::release()
{
    unlockHandle(data);
}

/* Opens a map on the device, caches the file's patches and prepares the
   file (for its sysex). */
/* @zoombi32 0x004791b6 */
short __cdecl midiObj::openDevice()
{
    short error;

    if ((error = midiMapOpen(&map, device, 0, 0, 0)) != 0)
        return setSoundError(error == MMSYSERR_ALLOCATED ? 0x29cd
                                                         : error == MMSYSERR_BADDEVICEID ? 0x29ce
                                                                                         : 0x29cc);
    getMidiDevCaps(device, &caps, sizeof caps);
    if (cachePatches(1)) {
        midiMapClose((long)map);
        return setSoundError(0x29cc);
    }
    memset(&header, 0, sizeof header);
    header.lpData = (LPSTR)file;
    header.dwBufferLength = fileSize;
    if (midiMapPrepareHeader((long)map, &header, sizeof header)) {
        cachePatches(0);
        midiMapClose((long)map);
        return setSoundError(0x29cc);
    }
    initLock(&lock, 0);
    call.proc = midiStep;
    call.data = this;
    setupDone = 0;
    return setSoundError(0);
}

/* @zoombi32 0x004792c6 */
short __cdecl midiObj::setDeviceRate(long speed)
{
    if (speed > 0x1000000 || speed < 0x100)
        return setSoundError(0x29d2);
    enterLock(&lock);
    tempoScale = fixedDiv(0x10000, speed);
    leaveLock(&lock);
    return setSoundError(0);
}

/* @zoombi32 0x0047931d */
short __cdecl midiObj::setDeviceVolume(long curve)
{
    return setSoundError(setMidiMapTable((long)map, curve) ? 0x29d3 : 0);
}

/* Starts playing (after the setup section, first time round) on a timer. */
/* @zoombi32 0x00479349 */
short __cdecl midiObj::startDevice(short paused)
{
    unsigned long at;

    if (setupEnd && setupEnd < ticks && !setupDone) {
        at = ticks;
        seek(0);
        seekTo(setupEnd, 1);
        seek(at);
    }
    if (!paused) {
        advance(0, 1, 1);
        step = nextStep();
        interval = fixedMul(makeFixed(step, 0), tempoScale);
        startTime = timerTime();
        timer = newTimer(fixedToInt(interval), 0, midiTimer, (long)this);
    }
    return setSoundError(0);
}

/* @zoombi32 0x0047941b */
void __cdecl midiObj::haltDevice()
{
    lockTimers();
    if (started) {
        killTimer(timer);
        timer = 0;
        unlockTimers();
        advance(fixedDiv(timerTime() - startTime, tempoScale), 1, 1);
    } else
        unlockTimers();
    midiMapReset((long)map);
}

/* @zoombi32 0x0047947d */
void __cdecl midiObj::closeDevice()
{
    midiMapUnprepareHeader((long)map, &header, sizeof header);
    cachePatches(0);
    midiMapReset((long)map);
    midiMapClose((long)map);
    removeLock(&lock);
}

/* @zoombi32 0x004794bd */
long __cdecl midiObj::deviceHandle()
{
    return (long)map;
}

/* @zoombi32 0x004794c8 */
long __cdecl midiObj::position()
{
    unsigned long at;

    enterLock(&lock);
    at = ticks;
    leaveLock(&lock);
    return at;
}

/* @zoombi32 0x004794f0 */
void __cdecl midiObj::pause()
{
    SoundNotice notice;

    killTimer(timer);
    timer = 0;
    advance(fixedDiv(timerTime() - startTime, tempoScale), 1, 1);
    started = 0;
    playing = 1;
    notice.what = 2;
    notifySound(this, &notice);
}

/* @zoombi32 0x00479556 */
void __cdecl midiObj::endLoop()
{
    endingLoop = 1;
}

/* @zoombi32 0x00479564 */
void __cdecl midiObj::resetLoop()
{
    lastLoop = 0;
    endingLoop = 0;
    loopCount = 0;
    loopStart = 0;
}

/* @zoombi32 0x0047958c */
void __cdecl midiObj::resume()
{
    SoundNotice notice;

    advance(0, 1, 1);
    step = nextStep();
    interval = fixedMul(makeFixed(step, 0), tempoScale);
    startTime = timerTime();
    timer = newTimer(fixedToInt(interval), 0, midiTimer, (long)this);
    playing = 0;
    started = 1;
    notice.what = 3;
    notifySound(this, &notice);
}

/* Moves to the cue point `text` (leaving the position alone if there's
   none). */
/* @zoombi32 0x00479625 */
short __cdecl midiObj::setText(const char *text, unsigned short size)
{
    unsigned long at = ticks;

    seek(0);
    findText = text;
    findLength = size;
    found = 0;
    while (!found && finishedTracks < trackCount)
        advance(nextStep(), 0, 0);
    findText = 0;
    if (!found) {
        seek(at);
        return setSoundError(0x29d4);
    }
    return setSoundError(0);
}

/* Not exact: the original keeps `this` in ebx; BCC32 4.5 uses eax. */
/* @zoombi32 0x004796bf */
short __cdecl midiObj::play(SoundNotify proc, long data)
{
    if (finishedTracks < trackCount)
        return audioObj::play(proc, data);
    return setSoundError(0x29cf);
}

/* Moves on by `elapsed` ticks, running the tracks' events that are due
   (sending them if `play`, and telling the owner of cue points if
   `notify`), and jumps back to the loop's start if a loop's end asks. */
/* @zoombi32 0x00479730 */
void __cdecl midiObj::advance(unsigned short elapsed, short play, short notify)
{
    unsigned long before;
    unsigned long delta;
    long ms;
    unsigned long sum;
    unsigned short i;
    MidiTrack *track;
    short error;

    ticks += elapsed;
    ms = fixedMul(makeFixed(elapsed, 0), msPerTick);
    before = time;
    sum = fixedFraction(ms) + fraction;
    fraction = lowWord(sum);
    time += fixedToInt(ms) + highWord(sum);
    delta = time - before;
    if (delta >= nextEvent)
        nextEvent = 0xffffffff;
    looping = 0;
    for (i = 0; i < trackCount; i++) {
        track = &tracks[i];
        if (track->done)
            continue;
        track->delta -= delta;
        if (track->delta <= 0)
            dispatch(track, play, notify);
        if (track->done)
            finishedTracks++;
        else if ((unsigned long)track->delta < nextEvent)
            nextEvent = track->delta;
    }
    if (looping) {
        error = sound.error;
        seek(loopStart);
        sound.error = error;
        if (play)
            advance(0, 1, 0);
    }
}

/* Sets the tempo (microseconds per quarter note), and the conversions
   between ticks and ms. */
/* @zoombi32 0x0047987c */
void __cdecl midiObj::setTempo(unsigned long microseconds)
{
    long ratio;

    tempo = microseconds;
    ratio = fixedDiv(microseconds << 8, makeFixed(division, 0));
    ticksPerMs = fixedDiv(ratio, 0x3e800);
    msPerTick = fixedDiv(0x3e800, ratio);
    unsigned long most = fixedMul(0x7fff, msPerTick);
    maxMs = most > 0x7fff ? 0x7fff : most;
    most = fixedMul(0x7fff, ticksPerMs);
    maxTicks = most > 0x7fff ? 0x7fff : most;
}

/* The ticks to the next event (at most the longest step). */
/* @zoombi32 0x0047991a */
unsigned short __cdecl midiObj::nextStep()
{
    long ms;

    if (!nextEvent)
        return 0;
    if (maxMs < nextEvent)
        ms = makeFixed(maxMs, 0);
    else
        ms = makeFixed(nextEvent, 0) - makeFixed(0, fraction);
    return fixedRound(fixedMul(ms, ticksPerMs) + 0x8000);
}

/* A MIDI variable-length number. */
/* @zoombi32 0x0047998e */
unsigned long __cdecl readVarLen(unsigned char **p)
{
    unsigned char c = *(*p)++;
    unsigned long value = c & 0x7f;

    while (c & 0x80) {
        c = *(*p)++;
        value = (c & 0x7f) + (value << 7);
    }
    return value;
}

/* @zoombi32 0x004799bd */
short __cdecl midiObj::seek(long at)
{
    return seekTo(at, 0);
}

/* Moves to `position` (in ticks), from the start if it's behind. */
/* @zoombi32 0x004799d3 */
short __cdecl midiObj::seekTo(unsigned long at, short play)
{
    unsigned short i;
    MidiTrack *track;

    if (!at || at < ticks) {
        setTempo(500000);
        ticks = 0;
        time = 0;
        fraction = 0;
        finishedTracks = 0;
        nextEvent = 0xffffffff;
        for (i = 0; i < trackCount; i++) {
            track = &tracks[i];
            track->status = 0;
            track->done = 0;
            track->cursor = track->start;
            track->delta = readVarLen(&track->cursor);
            if ((unsigned long)track->delta < nextEvent)
                nextEvent = track->delta;
        }
    }
    while (at > ticks && finishedTracks < trackCount) {
        unsigned short ahead = nextStep();
        if (at > ahead + ticks)
            advance(ahead, play, 0);
        else
            advance(at - ticks, play, 0);
    }
    if (finishedTracks == trackCount && at > ticks && at != 0xffffffff)
        return setSoundError(0x29cf);
    return setSoundError(0);
}

/* The timer's call: moves on a step, and sets the timer for the next. */
/* Not exact: `midi` and `next` swap registers (esi and edi), and the original
   loads 0 into eax for `error` where BCC32 4.5 drops the dead store. */
/* @zoombi32 0x00479b13 */
void midiStep(void *data)
{
    midiObj *midi = (midiObj *)data;
    SoundNotice notice;
    long next;
    short error;

    midi->advance(midi->step, 1, 1);
    error = 0;
    if (midi->finishedTracks < midi->trackCount) {
        midi->step = midi->nextStep();
        next = fixedMul(makeFixed(midi->step, 0), midi->tempoScale);
        next += makeFixed(0, (unsigned short)midi->interval);
        midi->interval = next;
        if (makeFixed(1, 0) > midi->interval)
            midi->interval = makeFixed(1, 0);
        midi->startTime = timerTime();
        midi->timer = newTimer(fixedToInt(midi->interval), 0, midiTimer, (long)data);
        if (!midi->timer) {
            error = timerError();
            midi->timer = 0;
            midi->started = 0;
            notice.what = 1;
            notice.value = error;
            notifySound(midi, &notice);
        }
    }
}

/* @zoombi32 0x00479c17 */
void midiTimer(long, long data)
{
    midiObj *midi = (midiObj *)data;

    deferCall(&midi->lock, &midi->call);
}

/* Runs a track's events that are due; the ticks to its next. */
/* Not exact: the original increments `p` before reading through the old value in
   `*p++`; BCC32 4.5 increments after. */
/* @zoombi32 0x00479c34 */
long __cdecl midiObj::dispatch(MidiTrack *track, short play, short notify)
{
    unsigned char *p;
    unsigned char type;
    char *q;
    unsigned short count;
    unsigned char *start;
    unsigned char saved;
    unsigned long message;
    SoundNotice notice;
    unsigned long length;
    unsigned char status;

    p = track->cursor;
    do {
        status = *p++;
        if (status == 0xff) {
            track->status = 0;
            type = *p++;
            length = readVarLen(&p);
            switch (type) {
            case 7:
                notice.what = 0;
                notice.value = length > 0xffff ? (unsigned short)0xffff : (unsigned short)length;
                notice.data = (char *)p;
                if (play)
                    notifySound(this, &notice);
                if (findText && findLength <= notice.value)
                    found |= !memicmp(findText, notice.data, findLength);
                break;
            case 0x2f:
                track->done = 1;
                if (duration < ticks)
                    duration = ticks;
                return track->delta = 0;
            case 6:
                if (length >= 9 && !memicmp(p, setupEndMarker, 9)) {
                    setupEnd = ticks;
                    if (play)
                        setupDone = 1;
                } else if (length >= 10 && !memicmp(p, loopStartMarker, 10))
                    loopStart = ticks;
                else if (length >= 8 && !memicmp(p, loopEndMarker, 8)) {
                    loopEnd = ticks;
                    if (notify && !lastLoop && !endingLoop) {
                        if (!loopCount) {
                            for (q = (char *)p + 8, count = length - 8; count && *q != '#'; count--, q++)
                                ;
                            loopCount = count ? count > 1 ? parseNumber(q + 1) : 0 : 0xffff;
                        }
                        if (loopCount) {
                            looping = 1;
                            if (loopCount != 0xffff)
                                lastLoop = !--loopCount;
                        }
                    }
                }
                break;
            case 0x51:
                setTempo(((unsigned long)p[0] << 16) + ((unsigned short)p[1] << 8) + p[2]);
                break;
            }
            p += length;
        } else if (status == 0xf0) {
            track->status = 0;
            length = readVarLen(&p);
            if (play) {
                start = p - 1;
                saved = *start;
                *start = 0xf0;
                sysex.lpData = (LPSTR)start;
                sysex.dwBufferLength = length + 1;
                sysex.dwFlags = MHDR_PREPARED;
                midiMapLongMsg((long)map, &sysex, sizeof sysex);
                *start = saved;
            }
            p += length;
        } else if (status == 0xf7) {
            track->status = 0;
            length = readVarLen(&p);
            if (play) {
                sysex.lpData = (LPSTR)p;
                sysex.dwBufferLength = length;
                sysex.dwFlags = MHDR_PREPARED;
                midiMapLongMsg((long)map, &sysex, sizeof sysex);
            }
            p += length;
        } else if (status > 0xf0) {
            track->done = 1;
            finishedTracks++;
            return track->delta = 0;
        } else {
            if (!(status & 0x80)) {
                status = track->status;
                p--;
            }
            track->status = status;
            switch (status & 0xf0) {
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xb0:
            case 0xe0:
                message = status | (unsigned short)p[0] << 8 | (unsigned long)p[1] << 16;
                p += 2;
                break;
            case 0xc0:
            case 0xd0:
                message = status | (unsigned short)p[0] << 8;
                p++;
                break;
            }
            if (play)
                midiMapShortMsg((long)map, message);
        }
        track->delta += readVarLen(&p);
    } while (track->delta <= 0);
    track->cursor = p;
    return track->delta;
}

/* @zoombi32 0x0047a066 */
long __cdecl parseNumber(const char *text)
{
    return atol(text);
}

/* @zoombi32 0x0047a074 */
short unsupportedMidiCall(short)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* Starts MIDI (the mapper, then initWaveMix) for its first user. */
/* @zoombi32 0x0047a07f */
unsigned short initMidi()
{
    short error;

    if (!midiUsers) {
        if ((error = initMidiMap()) != 0)
            return error;
        if ((error = initWaveMix()) != 0) {
            closeMidiMaps();
            return error;
        }
    }
    midiUsers++;
    return 0;
}

/* @zoombi32 0x0047a0c0 */
void closeMidi()
{
    if (midiUsers > 0 && !--midiUsers) {
        closeMidiMaps();
        closeWaveMix();
    }
}
