/*
 * sound (0x411350-0x4121cc): 'waveform', 'midi', 'unknown chunk type:', 'unable to prepare'
 */

#include <stdio.h>
#include <string.h>
#include "zoombinis.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "jointext.h"
#include "loading.h"
#include "platform.h"
#include "sound.h"

/* The module's messages (named, not literals: the original addresses each
   directly). */
char msgUnableToCreate[] = "unable to create";
char textSound[] = "sound";
char textMidi[] = "midi";
char textWaveform[] = "waveform";
char msgUnknownChunk[] = "unknown chunk type:";
char msgUnableToPrepare[] = "unable to prepare";
char msgSeekError[] = "seek error:";
char msgUnableToStart[] = "unable to start";
char msgPrematureExit[] = "premature exit:";
char formatJoin[] = "%s%s";
char formatErrorNumber[] = ":error #%d";
char formatSoundId[] = "%s id #%u";
char msgDeviceFailed[] = " sound device or driver has failed to respond.";

/* Loads the sound with a key (of a type): the key if it's loaded, else -1. */
/* @zoombi32 0x00411350 */
unsigned short loadSoundByKey(short key, long type)
{
    unsigned short result = 0xffff;
    Entry *entry;

    if ((entry = getSound(key, type)) != 0 && loadSound(entry))
        result = key;
    return result;
}

/*
 * Loads the sound with a key (of a type), finding its resource first unless
 * g_4aa428 is set: the key if it's loaded, 0 if sound is off, else -1. Sounds
 * no larger than g_4a009c are loaded by loadSoundByKey.
 */
/* @zoombi32 0x00411382 */
unsigned short fn_411382(short key, long type)
{
    Entry *entry;
    unsigned short result;

    if (g_4aa428)
        return loadSoundByKey(key, type);
    result = 0xffff;
    if (fn_4121a5(1))
        return 0;
    if ((entry = findOrAddSound(key, type)) != 0) {
        if (entry->handle)
            return key;
        if (!(entry->unknownA = fn_46c402(type, key, 1))) {
            if (g_4aa42c)
                reportSoundError(key, type, 0, 0);
            removeSound(&entry);
            loadFailed = 0;
        } else {
            entry->type = 0;
            if (resourceSize(entry->unknownA) <= g_4a009c)
                return loadSoundByKey(key, type);
            if (!entry->unknown2 && !resourceHandle(entry->unknownA))
                entry->unknown2 = 1;
            if (loadSound(entry))
                result = key;
        }
    }
    return result;
}

/* Stops and unloads the sound with a key (of a type), releasing its resource. */
/* @zoombi32 0x0041153c */
void unloadSound(short key, long type)
{
    Entry *entry;

    if ((entry = fn_4115f5(key, type)) != 0) {
        stopSounds(key, type);
        fn_4117a8(entry);
        fn_46c5b7(&entry->unknownA);
        if (fn_46bee2() == 1)
            removeSound(&entry);
    }
}

/* unloadSound with fn_46bee9's setting at 1. */
/* @zoombi32 0x0041158c */
void fn_41158c(short key, long type)
{
    short saved = fn_46bee9(1);

    unloadSound(key, type);
    fn_46bee9(saved);
}

/* The sound with a key (of a type), loading its resource and type (a
   big-endian tag, as on the Mac) if it isn't loaded. */
/* @zoombi32 0x00411478 */
Entry *getSound(short key, long type)
{
    Entry *entry;

    if (fn_4121a5(1))
        return 0;
    if (!(entry = findOrAddSound(key, type)))
        return 0;
    if (entry->handle)
        return entry;
    fn_46c4fe(&entry->unknownA, type, key, textSound, g_4aa42c);
    if (!entry->unknownA)
        removeSound(&entry);
    else
        setSoundType(&entry, key, swapLong(*(long *)(fn_46cafb(entry->unknownA) + 8)));
    return entry;
}

/* The sound with a key (of a type), added to the list if it isn't there. */
/* @zoombi32 0x004115b1 */
Entry *findOrAddSound(short key, long type)
{
    Entry *entry = fn_4115f5(key, type);

    if (!entry) {
        entry = addSound(key, type);
        if (!entry && !g_4aa42a)
            reportSoundError(key, type, 0, 0);
    }
    return entry;
}

/* Finds the sound with a key, of a type ('SND' matches any). */
/* @zoombi32 0x004115f5 */
Entry *fn_4115f5(short key, long tag)
{
    Entry *entry = g_4a00a0;
    while (entry && (key != entry->key || (tag != 0x534e44 && tag != soundTypes[entry->type])))
        entry = entry->next;
    return entry;
}

/* Adds a sound (of a key and type) at the end of the list; the new entry. */
/* @zoombi32 0x0041162c */
Entry *addSound(short key, long type)
{
    Entry **link;

    for (link = &g_4a00a0; *link; link = &(*link)->next)
        ;
    if (allocateBlock((void **)link, sizeof(Entry))) {
        memset(*link, 0, sizeof(Entry));
        (*link)->key = key;
        setSoundType(link, key, type);
    }
    return *link;
}

/* Frees a sound and takes it out of the list. */
/* @zoombi32 0x0041167a */
void removeSound(Entry **entry)
{
    Entry **link;
    Entry *next;

    fn_4117a8(*entry);
    fn_46c602(&(*entry)->unknownA);
    for (link = &g_4a00a0; *link != *entry; link = &(*link)->next)
        ;
    next = (*link)->next;
    freeAndClear((void **)link);
    *link = next;
    *entry = 0;
}

/* Sets a sound's type from its resource type: waves and MIDI ('SND' leaves
   it); anything else is reported, or the sound dropped. */
/* @zoombi32 0x004116c1 */
void setSoundType(Entry **entry, short key, long type)
{
    short kind;

    if (type == RESOURCE_TYPE(0, 'S', 'N', 'D'))
        return;
    if (type == RESOURCE_TYPE('W', 'A', 'V', 'E') || type == RESOURCE_TYPE('t', 'W', 'A', 'V'))
        kind = 0;
    else if (type == RESOURCE_TYPE('M', 'I', 'D', 'I') || type == RESOURCE_TYPE('t', 'M', 'I', 'D'))
        kind = 1;
    else if (!g_4aa42a)
        reportSoundError(key, type, 0, msgUnknownChunk);
    else
        removeSound(entry);
    (*entry)->type = kind;
}

/* Loads a sound into the engine if it isn't loaded; whether it is. */
/* @zoombi32 0x00411728 */
short loadSound(Entry *entry)
{
    if (fn_4121a5(2))
        return 0;
    if (!entry->handle) {
        if (entry->unknown2) {
            fn_41585f();
            entry->handle = newStreamedSound(entry->unknownA, g_4a0098);
            mainLoopEvents();
        } else
            entry->handle = newSound(fn_46beac(entry->unknownA));
        if (!entry->handle && !g_4aa42a)
            reportSoundError(0, 0, entry, msgUnableToCreate);
    }
    return entry->handle != 0;
}

/* @zoombi32 0x004117a8 */
void fn_4117a8(Entry *entry)
{
    if (entry->handle) {
        disposeSound(entry->handle);
        entry->handle = 0;
    }
}

/*
 * Prepares a sound on a channel. If the device fails, asks whether to retry
 * (Abort is fatal, Ignore stops asking); then the channel is the sound's, and
 * newest. Whether it's prepared. (The ageing loop was presumably meant to age
 * every other channel by one, but it takes the new channel's `started` down
 * by the channel count instead.)
 */
/* @zoombi32 0x004117c7 */
short prepareSound(Entry *entry, short channel)
{
    char message[0x100];
    int answer;
    short type, i;
    char *kind;

    if (fn_4121a5(3))
        return 0;
    do {
        answer = IDOK;
        if (openSound(entry->handle, 0xffff)) {
            if (!g_4aa42a)
                reportSoundError(0, 0, entry, msgUnableToPrepare);
            if (soundErrorsIgnored)
                return 0;
            kind = !entry->type ? textWaveform : textMidi;
            sprintf(message, formatJoin, kind, msgDeviceFailed);
            answer = MessageBox(mainWindow, message, appName, MB_ABORTRETRYIGNORE);
        }
    } while (answer == IDRETRY);
    if (answer == IDABORT)
        fatalError(g_4a07b4);
    else if (answer == IDIGNORE) {
        soundErrorsIgnored = 1;
        return 0;
    }
    type = entry->type;
    soundChannels[type][channel].id = entry->key;
    soundChannels[type][channel].started = 0xffff;
    for (i = 0; i < channelCounts[type]; i++)
        soundChannels[type][channel].started--;
    return 1;
}

/* @zoombi32 0x00411910 */
void fn_411910(Entry *entry, short channel)
{
    closeSound(entry->handle);
    soundChannels[entry->type][channel].id = 0xffff;
}

/* Starts a sound on a channel (unless sound is off); whether it's playing.
   The engine reports on it to fn_411d2c, with its type and channel. */
/* @zoombi32 0x0041193e */
short startSound(Entry *entry, short channel)
{
    short type;

    if (fn_4121a5(4))
        return 0;
    type = entry->type;
    soundChannels[type][channel].playing = 1;
    currentChannel[type] = 0;
    if (playSound(entry->handle, fn_411d2c,
                  ((unsigned long)(unsigned short)type << 16) + (unsigned short)channel)) {
        if (g_4aa42a)
            soundChannels[type][channel].playing = 0;
        else
            reportSoundError(0, 0, entry, msgUnableToStart);
    }
    return soundChannels[type][channel].playing;
}

/* Stops a sound playing on a channel. */
/* @zoombi32 0x004119f3 */
void fn_4119f3(Entry *entry, short channel)
{
    short type = entry->type;

    if (soundChannels[type][channel].playing)
        stopSound(entry->handle);
    soundChannels[type][channel].playing = 0;
    currentChannel[type] = -1;
}

/* Reports a problem with a sound: its kind and id, the engine's error if
   any, and the message. */
/* @zoombi32 0x00411a4c */
void reportSoundError(short id, long type, Entry *entry, const char *message)
{
    char name[0x14];
    char error[0x10];
    short code;
    const char *errorText, *kind;

    kind = errorText = 0;
    if (entry) {
        id = entry->key;
        if (!entry->type)
            kind = textWaveform;
        else
            kind = textMidi;
    } else if (type == RESOURCE_TYPE('t', 'W', 'A', 'V'))
        kind = textWaveform;
    else if (type == RESOURCE_TYPE('t', 'M', 'I', 'D'))
        kind = textMidi;
    if ((code = soundError()) != 0) {
        fn_4150c7(0xe, error, formatErrorNumber, code);
        errorText = error;
    }
    fn_4150c7(0x14, name, formatSoundId, textSound, (unsigned short)id);
    joinText(&g_4aa438, name, errorText);
    joinText(&g_4aa434, kind, g_4aa438);
    joinText(&g_4aa430, message, g_4aa434);
    reportJoinedError(g_4aa430);
}

/*
 * Plays the sound with a key (of a type) on a channel (-1: whichever
 * findChannel picks), stopping what's playing there. Whether it started.
 *
 * Differs only in register choice: the original has entry (and result) in
 * ebx, channel in esi and key (then the sound's type) in edi; this compiles to
 * key in ebx, entry in esi, channel in edi. BCC32 ranks register candidates by
 * use count, so the original presumably used key once less, or channel and
 * entry once more.
 */
/* @zoombi32 0x00411b28 */
short playSoundOn(short key, long type, short channel)
{
    short result = 0;
    Entry *entry;

    waitWhilePaused();
    if (loadSoundByKey(key, type) != 0xffff) {
        entry = fn_4115f5(key, type);
        key = entry->type; /* from here on, the sound's type */
        if (channel == -1)
            channel = findChannel(key);
        if (soundChannels[key][channel].id != 0xffff)
            stopSounds(soundChannels[key][channel].id, type);
        if (!prepareSound(entry, channel))
            return 0;
        if (seekSound(entry->handle, 0)) {
            if (g_4aa42a)
                return 0;
            reportSoundError(0, 0, entry, msgSeekError);
        }
        if (!startSound(entry, channel))
            return 0;
        result = 1;
    }
    return result;
}

/* playSoundOn, finding the sound's resource first (fn_411382). */
/* @zoombi32 0x00411bfe */
short fn_411bfe(short key, long type, short channel)
{
    short result = 0;

    if (fn_411382(key, type) != 0xffff)
        result = playSoundOn(key, type, channel);
    return result;
}

/* The channel for a new sound of a type: a free one, else the idle one
   started longest ago, else the playing one started longest ago. */
/* @zoombi32 0x00411c30 */
short findChannel(short type)
{
    short best, found, idle, i;
    unsigned short oldest;

    found = idle = 0;
    oldest = 0xffff;
    for (i = 0; i < channelCounts[type] && !found; i++) {
        if (soundChannels[type][i].id == 0xffff) {
            found = 1;
            best = i;
        } else if (!soundChannels[type][i].playing) {
            if (!idle || oldest > soundChannels[type][i].started) {
                oldest = soundChannels[type][i].started;
                best = i;
            }
            idle = 1;
        } else if (!idle && oldest > soundChannels[type][i].started) {
            oldest = soundChannels[type][i].started;
            best = i;
        }
    }
    return best;
}

/* The engine's notice about a sound (the cookie holds its type and
   channel). The empty `if` is as in the original (compiled-out debug code?). */
/* @zoombi32 0x00411d2c */
void fn_411d2c(long, SoundNotice *notice, long cookie)
{
    short type = (unsigned long)cookie >> 16;
    short channel = cookie;

    switch (notice->what) {
    case 1:
        soundChannels[type][channel].playing = 0;
        currentChannel[type] = -1;
        if (notice->unknown4 && !g_4aa42a)
            ;
        break;
    case 0:
        currentChannel[type] = *notice->data;
        break;
    }
}

/* Stops a sound (0xffff: all) of a type ('SND': any), freeing its channel. */
/* @zoombi32 0x00411d8f */
void stopSounds(unsigned short id, long type)
{
    Entry *entry;
    short t, channel;
    unsigned short current = id;

    for (t = 0; t < 2; t++) {
        if (type == RESOURCE_TYPE(0, 'S', 'N', 'D') || soundTypes[t] == type) {
            for (channel = 0; channel < 4; channel++) {
                if (id == 0xffff)
                    current = soundChannels[t][channel].id;
                if (soundChannels[t][channel].id != 0xffff && current == soundChannels[t][channel].id) {
                    entry = fn_4115f5(current, soundTypes[t]);
                    fn_4119f3(entry, channel);
                    fn_411910(entry, channel);
                }
            }
        }
    }
}

/* Ends the looping of each matching sound that's playing. */
/* @zoombi32 0x00411e4c */
void fn_411e4c(unsigned short id, long type)
{
    short t, channel;
    unsigned short current;

    waitWhilePaused();
    current = id;
    for (t = 0; t < 2; t++) {
        if (type == RESOURCE_TYPE(0, 'S', 'N', 'D') || soundTypes[t] == type) {
            for (channel = 0; channel < 4; channel++) {
                if (id == 0xffff)
                    current = soundChannels[t][channel].id;
                if (soundChannels[t][channel].id != 0xffff && current == soundChannels[t][channel].id
                    && soundChannels[t][channel].playing)
                    endSoundLoop(fn_4115f5(id, soundTypes[t])->handle);
            }
        }
    }
}

/* Whether a sound (0xffff: any) of a type ('SND': any) is playing. */
/* @zoombi32 0x00411f21 */
short isSoundPlaying(unsigned short id, long type)
{
    short found, t, channel;
    unsigned short current;

    waitWhilePaused();
    found = 0;
    current = id;
    for (t = 0; t < 2 && !found; t++) {
        if (type == RESOURCE_TYPE(0, 'S', 'N', 'D') || soundTypes[t] == type) {
            for (channel = 0; channel < 4 && !found; channel++) {
                if (id == 0xffff)
                    current = soundChannels[t][channel].id;
                found = current == soundChannels[t][channel].id && soundChannels[t][channel].playing;
            }
        }
    }
    return found;
}
/* Frees the error message's parts and unloads every sound. */
/* @zoombi32 0x00411fd3 */
void unloadSounds()
{
    Entry *entry;

    freeText((void **)&g_4aa430);
    freeText((void **)&g_4aa434);
    freeText((void **)&g_4aa438);
    while ((entry = g_4a00a0) != 0)
        fn_41158c(entry->key, RESOURCE_TYPE(0, 'S', 'N', 'D'));
}

/*
 * Plays the sound with a key (of a type) on a channel, unless an event of
 * eventType is already waiting; then waits for it as waitForSound does.
 */
/* @zoombi32 0x0041200c */
short playSound(short key, long type, short channel, short eventType, short discard)
{
    if (!isEventWaiting(eventType, 0))
        playSoundOn(key, type, channel);
    return waitForSound(key, type, eventType, discard);
}


/* Waits for a sound to finish, running the main loop; an input event (of
   eventType) cuts it short. Whether it finished uninterrupted. */
/* @zoombi32 0x00412048 */
short waitForSound(unsigned short id, long type, short eventType, short discard)
{
    unsigned short interrupted;

    do {
        mainLoopEvents();
        interrupted = isEventWaiting(eventType, discard);
    } while (fn_4120a2(id, type, interrupted));
    return !interrupted;
}

/* @zoombi32 0x00412084 */
short fn_412084(unsigned short id, long type, short eventType, short discard)
{
    return waitForSound(id, type, eventType, discard);
}

/* With `stop`, stops the sound (and answers 0); else whether it's playing. */
/* @zoombi32 0x004120a2 */
short fn_4120a2(unsigned short id, long type, short stop)
{
    if (stop) {
        stopSounds(id, type);
        return 0;
    }
    return isSoundPlaying(id, type);
}

/* Whether a sound type has no current value, or one at least `value`. */
/* @zoombi32 0x004120c8 */
short fn_4120c8(char value, long type)
{
    short i;

    waitWhilePaused();
    if (type == RESOURCE_TYPE('t', 'W', 'A', 'V'))
        i = 0;
    else
        i = 1;
    if (currentChannel[i] != -1)
        return currentChannel[i] >= value;
    return 1;
}

/* Waits until a sound type's value reaches `value` (fn_4120c8), running the
   main loop; an input event cuts it short (thrown away with `discard`).
   Whether it wasn't cut short. */
/* @zoombi32 0x0041210a */
short waitForSoundValue(char value, long type, short eventType, short discard)
{
    unsigned short interrupted;

    do {
        mainLoopEvents();
        interrupted = isEventWaiting(eventType, 0);
    } while (!interrupted && !fn_4120c8(value, type));
    if (interrupted && discard)
        discardEvents(eventType);
    return !interrupted;
}

/* @zoombi32 0x00412159 */
short fn_412159(char value, long type, short eventType, short discard)
{
    return waitForSoundValue(value, type, eventType, discard);
}

/* Resets a sound type's current channel, if it has one. */
/* @zoombi32 0x00412176 */
void fn_412176(long type)
{
    short i;

    if (type == RESOURCE_TYPE('t', 'W', 'A', 'V'))
        i = 0;
    else
        i = 1;

    if (currentChannel[i] != -1)
        currentChannel[i] = 0;
}

/* Whether sound is on and at most `level`. */
/* @zoombi32 0x004121a5 */
short fn_4121a5(short level)
{
    if (soundLevel && soundLevel <= level)
        return 1;
    return 0;
}
