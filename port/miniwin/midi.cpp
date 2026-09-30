/*
 * The synthesizer behind midiOut: a General MIDI synth, TinySoundFont
 * playing a SoundFont (loadSoundFont; `uv run port` fetches GeneralUser GS).
 * Without one, the game's MIDI is accepted and dropped, and the music is
 * silent. mmsystem.cpp mixes what synthRender makes.
 *
 * The engine does its own sequencing (midisound.cpp), so all this sees is
 * what a MIDI port would: channel messages as they're due, played at once.
 * Its MIDI map treats our device as a General MIDI port ("unknown device
 * (port)" in mohawk.w32): drums on channel 10, channels 6, 7 and 16 muted.
 */

#include <stddef.h>
#include <stdint.h>

#define TSF_IMPLEMENTATION
#include <tsf.h>

#include "miniwin/internal.h"

namespace miniwin {

static tsf *synth;
static int synthRate;
static BYTE runningStatus;

enum
{
    CHANNELS = 16,
    DRUM_CHANNEL = 9, /* channel 10 */
    MAX_VOICES = 128
};

/* Each channel as a GM device starts: piano (standard kit on channel 10),
   controllers at their defaults, pitch wheel centred. */
static void resetChannels()
{
    for (int channel = 0; channel < CHANNELS; channel++) {
        tsf_channel_midi_control(synth, channel, 121, 0);
        tsf_channel_set_presetnumber(synth, channel, 0, channel == DRUM_CHANNEL);
        tsf_channel_set_pitchwheel(synth, channel, 8192);
    }
}

bool loadSoundFont(const char *path)
{
    tsf *loaded = tsf_load_filename(path);
    if (!loaded) {
        trace("no music: can't load the SoundFont %s", path);
        return false;
    }
    if (synth)
        tsf_close(synth);
    synth = loaded;
    synthRate = 0;
    /* Allocated now, so rendering never has to. */
    tsf_set_max_voices(synth, MAX_VOICES);
    resetChannels();
    return true;
}

void synthMessage(DWORD message)
{
    if (!synth)
        return;
    BYTE status = message & 0xff;
    int first = message >> 8 & 0x7f, second = message >> 16 & 0x7f;
    if (status < 0x80) { /* running status: the data bytes move up one */
        second = first;
        first = status;
        status = runningStatus;
    } else if (status < 0xf0)
        runningStatus = status;
    int channel = status & 0x0f;
    switch (status & 0xf0) {
    case 0x80:
        tsf_channel_note_off(synth, channel, first);
        break;
    case 0x90: /* velocity 0 turns the note off */
        tsf_channel_note_on(synth, channel, first, second / 127.0f);
        break;
    case 0xb0:
        tsf_channel_midi_control(synth, channel, first, second);
        break;
    case 0xc0:
        tsf_channel_set_presetnumber(synth, channel, first, channel == DRUM_CHANNEL);
        break;
    case 0xe0:
        tsf_channel_set_pitchwheel(synth, channel, first | second << 7);
        break;
    }
    /* Aftertouch (0xa0, 0xd0) isn't in a SoundFont's model; system
       messages don't reach a synth this way. */
}

/* A long message: any MIDI bytes (the MIDI map sends its SYSX resources,
   runs of controller messages, this way). System exclusive messages are
   skipped: the game sends no device-specific ones. */
void synthSysex(const uint8_t *data, size_t size)
{
    static const int dataBytes[8] = {2, 2, 2, 2, 1, 1, 2, 0}; /* by status 0x8n-0xFn */
    BYTE status = 0;

    for (size_t i = 0; i < size;) {
        BYTE byte = data[i];
        if (byte == 0xf0) {
            while (i < size && data[i] != 0xf7)
                i++;
            i++;
            status = 0;
            continue;
        }
        if (byte & 0x80) {
            status = byte;
            i++;
        }
        if (status < 0x80 || status >= 0xf0) { /* nothing to run on */
            i++;
            continue;
        }
        int count = dataBytes[(status >> 4) - 8];
        if (i + count > size)
            break;
        DWORD message = status;
        for (int k = 0; k < count; k++)
            message |= (DWORD)(data[i + k] & 0x7f) << (8 * (k + 1));
        synthMessage(message);
        i += count;
    }
}

/* midiOutReset: every note off, as a driver sends note-offs (so they
   release as usual); programs and controllers stay. */
void synthReset()
{
    if (!synth)
        return;
    for (int channel = 0; channel < CHANNELS; channel++) {
        tsf_channel_set_sustain(synth, channel, 0);
        tsf_channel_note_off_all(synth, channel);
    }
}

void synthRender(float *stereo, int frames, int rate)
{
    if (!synth)
        return;
    if (rate != synthRate) {
        tsf_set_output(synth, TSF_STEREO_INTERLEAVED, rate, 0.0f);
        synthRate = rate;
    }
    tsf_render_float(synth, stereo, frames, 0);
}

bool synthAvailable()
{
    return synth != 0;
}

} /* namespace miniwin */
