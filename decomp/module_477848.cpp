/*
 * module_477848 (Mohawk engine): newStreamedSound
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A wave sound played from its resource's file as it goes; 0 on error. */
/* Not exact: the original keeps `object` in eax and `resource` in ebx; BCC32
   4.5 gives them ebx and esi. */
/* @zoombi32 0x00477848 */
long newStreamedSound(long resource, long unknown)
{
    long file;
    unsigned long offset;
    unsigned long size;
    audioObj *object;

    if (getResourceInfo(resource, &file, &offset, &size)) {
        setSoundError(resourceError());
        return 0;
    }
    if ((object = newStreamedWave(resource, file, unknown)) != 0) {
        if ((object->next = sound.objects) != 0)
            object->next->prev = object;
        object->prev = 0;
        sound.objects = object;
    }
    return (long)object;
}
