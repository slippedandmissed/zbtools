/*
 * The synthesizer behind midiOut. There is none yet: the game's MIDI is
 * accepted and dropped, so the music is silent. A General MIDI synthesizer
 * (a SoundFont player) goes here; mmsystem.cpp mixes what synthRender makes.
 */

#include <stddef.h>
#include <stdint.h>

#include "miniwin/internal.h"

namespace miniwin {

void synthMessage(DWORD)
{
}

void synthSysex(const uint8_t *, size_t)
{
}

void synthReset()
{
}

void synthRender(float *, int, int)
{
}

bool synthAvailable()
{
    return false;
}

} /* namespace miniwin */
