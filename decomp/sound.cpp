/*
 * sound (0x411350-0x4121cc): 'waveform', 'midi', 'unknown chunk type:', 'unable to prepare'
 */

#include <string.h>
#include "zoombinis.h"

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
        reportSoundError(key, type, 0, "unknown chunk type:");
    else
        removeSound(entry);
    (*entry)->type = kind;
}

/* @zoombi32 0x004117a8 */
void fn_4117a8(Entry *entry)
{
    if (entry->handle) {
        fn_476622(entry->handle);
        entry->handle = 0;
    }
}

/* @zoombi32 0x00411910 */
void fn_411910(Entry *entry, short channel)
{
    fn_4771e4(entry->handle);
    soundChannels[entry->type][channel].id = 0xffff;
}

/* Stops a sound playing on a channel. */
/* @zoombi32 0x004119f3 */
void fn_4119f3(Entry *entry, short channel)
{
    short type = entry->type;

    if (soundChannels[type][channel].playing)
        fn_4771a4(entry->handle);
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
            kind = "waveform";
        else
            kind = "midi";
    } else if (type == RESOURCE_TYPE('t', 'W', 'A', 'V'))
        kind = "waveform";
    else if (type == RESOURCE_TYPE('t', 'M', 'I', 'D'))
        kind = "midi";
    if ((code = fn_476bb4()) != 0) {
        fn_4150c7(0xe, error, ":error #%d", code);
        errorText = error;
    }
    fn_4150c7(0x14, name, "%s id #%u", "sound", (unsigned short)id);
    fn_413c24(&g_4aa438, name, errorText);
    fn_413c24(&g_4aa434, kind, g_4aa438);
    fn_413c24(&g_4aa430, message, g_4aa434);
    fn_413d33(g_4aa430);
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
