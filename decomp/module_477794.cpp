/*
 * module_477794 (Mohawk engine): newSound
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A sound from a Mohawk MIDI or WAVE file in a handle; 0 on error. */
/* @zoombi32 0x00477794 */
long newSound(short data)
{
    unsigned long *header;
    audioObj *object;

    if ((header = (unsigned long *)handleData(data)) == 0) {
        setSoundError(memError());
        return 0;
    }
    if (byteSwapLong(header[0]) != 0x4d48574b || byteSwapLong(header[1]) < 4) {
        setSoundError(0x29d0);
        return 0;
    }
    switch (byteSwapLong(header[2])) {
    case 0x4d494449:
        object = newMidiSound(data);
        break;
    case 0x57415645:
        object = newWaveSound(data);
        break;
    default:
        setSoundError(0x29d0);
        return 0;
    }
    if (object) {
        if ((object->next = sound.objects) != 0)
            object->next->prev = object;
        object->prev = 0;
        sound.objects = object;
    }
    return (long)object;
}
