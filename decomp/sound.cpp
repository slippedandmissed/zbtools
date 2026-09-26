/*
 * sound (0x411350-0x4121cc): 'waveform', 'midi', 'unknown chunk type:', 'unable to prepare'
 */

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

/* @zoombi32 0x004117a8 */
void fn_4117a8(HasHandle *object)
{
    if (object->handle) {
        fn_476622(object->handle);
        object->handle = 0;
    }
}

/* @zoombi32 0x00411910 */
void fn_411910(Entry *entry, short channel)
{
    fn_4771e4(entry->handle);
    soundChannels[entry->type][channel].unknown0 = -1;
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
