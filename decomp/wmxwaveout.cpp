/*
 * wmxwaveout (Mohawk engine): wmxWaveOut, the WaveMix object that passes
 * everything straight to waveOut
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_localmem.h"

/* Swaps a 16-bit block's samples to little-endian (if it's prepared and
   not queued: the engine keeps them big-endian). */
static void swapSamples(WAVEHDR *header)
{
    unsigned short *sample = (unsigned short *)header->lpData;
    unsigned long count;

    for (count = header->dwBufferLength >> 1; count; count--, sample++)
        *sample = (unsigned short)(*sample << 8 | *sample >> 8);
}

/* @zoombi32 0x0047fd60 */
__cdecl wmxWaveOut::wmxWaveOut(PCMWAVEFORMAT *format, LONG_PTR callback, LONG_PTR instance,
                               unsigned long flags)
    : wmxObject(0, format, callback, instance, flags)
{
}

/* @zoombi32 0x0047fd89 */
__cdecl wmxWaveOut::~wmxWaveOut()
{
    if (wave)
        close();
}

/* Passes waveOut's messages on to the object's callback. */
/* @zoombi32 0x0047fdc2 */
void CALLBACK wmxWaveOutCallback(HWAVEOUT, UINT message, DWORD_PTR instance, DWORD_PTR param1,
                                 DWORD_PTR param2)
{
    ((wmxObject *)instance)->notify((unsigned short)message, param1, param2);
}

/* @zoombi32 0x0047fddc */
unsigned short wmxWaveOut::open(unsigned short device, unsigned long flags)
{
    unsigned short error = waveOutOpen(&wave, device, (WAVEFORMAT *)&format,
                                       (DWORD_PTR)wmxWaveOutCallback, (DWORD_PTR)this, flags);

    if (!error)
        waveOutGetDevCaps(device, &caps, sizeof caps);
    return error;
}

/* @zoombi32 0x0047fe21 */
unsigned short __cdecl wmxWaveOut::breakLoop()
{
    return waveOutBreakLoop(wave);
}

/* @zoombi32 0x0047fe33 */
unsigned short __cdecl wmxWaveOut::close()
{
    unsigned short error = waveOutClose(wave);

    wave = 0;
    return error;
}

/* @zoombi32 0x0047fe4a */
unsigned short __cdecl wmxWaveOut::getID(unsigned short *id)
{
    UINT device;
    unsigned short error = waveOutGetID(wave, &device);

    *id = (unsigned short)device;
    return error;
}

/* @zoombi32 0x0047fe6c */
unsigned short __cdecl wmxWaveOut::getPitch(unsigned long *pitch)
{
    return waveOutGetPitch(wave, pitch);
}

/* @zoombi32 0x0047fe81 */
unsigned short __cdecl wmxWaveOut::getPlaybackRate(unsigned long *rate)
{
    return waveOutGetPlaybackRate(wave, rate);
}

/* @zoombi32 0x0047fe96 */
unsigned short __cdecl wmxWaveOut::getPosition(MMTIME *time, unsigned short size)
{
    return waveOutGetPosition(wave, time, size);
}

/* @zoombi32 0x0047feb0 */
unsigned short __cdecl wmxWaveOut::getVolume(unsigned long *volume)
{
    *volume = 0x10000;
    return 0;
}

/* @zoombi32 0x0047fec0 */
unsigned short __cdecl wmxWaveOut::pause()
{
    return waveOutPause(wave);
}

/* The original swaps the bytes with inline assembly (xchg ah, al). */
/* @zoombi32-functional 0x0047fed2 */
unsigned short __cdecl wmxWaveOut::prepareHeader(WAVEHDR *header, unsigned short size)
{
    if (format.wBitsPerSample == 16 && header->dwFlags & WHDR_PREPARED
        && !(header->dwFlags & WHDR_INQUEUE))
        swapSamples(header);
    return waveOutPrepareHeader(wave, header, size);
}

/* @zoombi32 0x0047ff21 */
unsigned short __cdecl wmxWaveOut::reset()
{
    return waveOutReset(wave);
}

/* @zoombi32 0x0047ff33 */
unsigned short __cdecl wmxWaveOut::restart()
{
    return waveOutRestart(wave);
}

/* @zoombi32 0x0047ff45 */
unsigned short __cdecl wmxWaveOut::setPitch(unsigned long pitch)
{
    if (caps.dwSupport & WAVECAPS_PITCH)
        return waveOutSetPitch(wave, pitch);
    return pitch == 0x10000 ? 0 : MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047ff72 */
unsigned short __cdecl wmxWaveOut::setPlaybackRate(unsigned long rate)
{
    if (caps.dwSupport & WAVECAPS_PLAYBACKRATE)
        return waveOutSetPlaybackRate(wave, rate);
    return rate == 0x10000 ? 0 : MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047ff9f */
unsigned short __cdecl wmxWaveOut::setVolume(unsigned long volume)
{
    return volume == 0x10000 ? 0 : MMSYSERR_NOTSUPPORTED;
}

/* The original swaps the bytes with inline assembly (xchg ah, al). */
/* @zoombi32-functional 0x0047ffb2 */
unsigned short __cdecl wmxWaveOut::unprepareHeader(WAVEHDR *header, unsigned short size)
{
    if (format.wBitsPerSample == 16 && header->dwFlags & WHDR_PREPARED
        && !(header->dwFlags & WHDR_INQUEUE))
        swapSamples(header);
    return waveOutUnprepareHeader(wave, header, size);
}

/* @zoombi32 0x00480001 */
unsigned short __cdecl wmxWaveOut::write(WAVEHDR *header, unsigned short size)
{
    return waveOutWrite(wave, header, size);
}

/* @zoombi32 0x0048001b */
void __cdecl wmxObject::operator delete(void *block)
{
    localFree(block);
}
