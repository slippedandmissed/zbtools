/*
 * sound (0x411350-0x4121cc): 'waveform', 'midi', 'unknown chunk type:', 'unable to prepare'
 */

#include "zoombinis.h"

/* Finds the entry with a key whose tag matches ('SND' matches any). */
/* @zoombi32 0x004115f5 */
Entry *fn_4115f5(short key, long tag)
{
    Entry *entry = g_4a00a0;
    while (entry && (key != entry->key || (tag != 0x534e44 && tag != g_4a00dc[entry->index])))
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
