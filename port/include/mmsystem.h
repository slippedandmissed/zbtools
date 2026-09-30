/*
 * miniwin's mmsystem.h: the multimedia API the game uses (waveOut, midiOut,
 * multimedia timers). Implemented in port/miniwin/mmsystem.cpp.
 */

#ifndef MINIWIN_MMSYSTEM_H
#define MINIWIN_MMSYSTEM_H

#include "windows.h"

#pragma pack(push, 1)

namespace miniwin {

#define MAXPNAMELEN 32

#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR 1
#define MMSYSERR_BADDEVICEID 2
#define MMSYSERR_NOTENABLED 3
#define MMSYSERR_ALLOCATED 4
#define MMSYSERR_INVALHANDLE 5
#define MMSYSERR_NODRIVER 6
#define MMSYSERR_NOMEM 7
#define MMSYSERR_NOTSUPPORTED 8
#define MMSYSERR_INVALFLAG 10
#define MMSYSERR_INVALPARAM 11
#define WAVERR_BADFORMAT 32
#define WAVERR_STILLPLAYING 33
#define WAVERR_UNPREPARED 34
#define MIDIERR_UNPREPARED 64
#define MIDIERR_STILLPLAYING 65
#define MIDIERR_NOTREADY 67

#define CALLBACK_TYPEMASK 0x00070000L
#define CALLBACK_NULL 0x00000000L
#define CALLBACK_WINDOW 0x00010000L
#define CALLBACK_TASK 0x00020000L
#define CALLBACK_FUNCTION 0x00030000L
#define WAVE_FORMAT_QUERY 0x0001
#define WAVE_ALLOWSYNC 0x0002
#define WAVE_MAPPER ((UINT)-1)
#define MIDI_MAPPER ((UINT)-1)

#define WOM_OPEN 0x3BB
#define WOM_CLOSE 0x3BC
#define WOM_DONE 0x3BD
#define MOM_OPEN 0x3C7
#define MOM_CLOSE 0x3C8
#define MOM_DONE 0x3C9
#define MM_WOM_OPEN WOM_OPEN
#define MM_WOM_CLOSE WOM_CLOSE
#define MM_WOM_DONE WOM_DONE

#define WAVE_FORMAT_PCM 1
#define WAVE_INVALIDFORMAT 0x00000000
#define WAVE_FORMAT_1M08 0x00000001
#define WAVE_FORMAT_1S08 0x00000002
#define WAVE_FORMAT_1M16 0x00000004
#define WAVE_FORMAT_1S16 0x00000008
#define WAVE_FORMAT_2M08 0x00000010
#define WAVE_FORMAT_2S08 0x00000020
#define WAVE_FORMAT_2M16 0x00000040
#define WAVE_FORMAT_2S16 0x00000080
#define WAVE_FORMAT_4M08 0x00000100
#define WAVE_FORMAT_4S08 0x00000200
#define WAVE_FORMAT_4M16 0x00000400
#define WAVE_FORMAT_4S16 0x00000800

#define WAVECAPS_PITCH 0x0001
#define WAVECAPS_PLAYBACKRATE 0x0002
#define WAVECAPS_VOLUME 0x0004
#define WAVECAPS_LRVOLUME 0x0008
#define WAVECAPS_SYNC 0x0010

#define WHDR_DONE 0x00000001
#define WHDR_PREPARED 0x00000002
#define WHDR_BEGINLOOP 0x00000004
#define WHDR_ENDLOOP 0x00000008
#define WHDR_INQUEUE 0x00000010
#define MHDR_DONE 0x00000001
#define MHDR_PREPARED 0x00000002
#define MHDR_INQUEUE 0x00000004

#define MOD_MIDIPORT 1
#define MOD_SYNTH 2
#define MOD_SQSYNTH 3
#define MOD_FMSYNTH 4
#define MOD_MAPPER 5
#define MIDICAPS_VOLUME 0x0001
#define MIDICAPS_LRVOLUME 0x0002
#define MIDICAPS_CACHE 0x0004
#define MIDI_CACHE_ALL 1
#define MIDI_CACHE_BESTFIT 2
#define MIDI_CACHE_QUERY 3
#define MIDI_UNCACHE 4

#define TIME_MS 0x0001
#define TIME_SAMPLES 0x0002
#define TIME_BYTES 0x0004
#define TIME_ONESHOT 0x0000
#define TIME_PERIODIC 0x0001
#define TIMERR_NOERROR 0
#define TIMERR_NOCANDO 97

typedef struct waveformat_tag
{
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
} WAVEFORMAT, *LPWAVEFORMAT;

typedef struct pcmwaveformat_tag
{
    WAVEFORMAT wf;
    WORD wBitsPerSample;
} PCMWAVEFORMAT, *LPPCMWAVEFORMAT;

typedef struct tWAVEFORMATEX
{
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct wavehdr_tag
{
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD_PTR dwUser;
    DWORD dwFlags;
    DWORD dwLoops;
    struct wavehdr_tag *lpNext;
    DWORD_PTR reserved;
} WAVEHDR, *LPWAVEHDR;

typedef struct midihdr_tag
{
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD_PTR dwUser;
    DWORD dwFlags;
    struct midihdr_tag *lpNext;
    DWORD_PTR reserved;
} MIDIHDR, *LPMIDIHDR;

typedef struct tagWAVEOUTCAPS
{
    WORD wMid;
    WORD wPid;
    MMVERSION vDriverVersion;
    CHAR szPname[MAXPNAMELEN];
    DWORD dwFormats;
    WORD wChannels;
    DWORD dwSupport;
} WAVEOUTCAPS, *LPWAVEOUTCAPS;

typedef struct tagMIDIOUTCAPS
{
    WORD wMid;
    WORD wPid;
    MMVERSION vDriverVersion;
    CHAR szPname[MAXPNAMELEN];
    WORD wTechnology;
    WORD wVoices;
    WORD wNotes;
    WORD wChannelMask;
    DWORD dwSupport;
} MIDIOUTCAPS, *LPMIDIOUTCAPS;

typedef struct mmtime_tag
{
    UINT wType;
    union
    {
        DWORD ms;
        DWORD sample;
        DWORD cb;
        DWORD ticks;
    } u;
} MMTIME, *LPMMTIME;

typedef struct timecaps_tag
{
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *LPTIMECAPS;

typedef void(CALLBACK *LPTIMECALLBACK)(UINT id, UINT message, DWORD_PTR user, DWORD_PTR reserved1,
                                       DWORD_PTR reserved2);

/* waveOut */
UINT waveOutGetNumDevs();
MMRESULT waveOutGetDevCaps(UINT_PTR device, LPWAVEOUTCAPS caps, UINT size);
MMRESULT waveOutGetID(HWAVEOUT wave, UINT *device);
MMRESULT waveOutOpen(LPHWAVEOUT wave, UINT device, const WAVEFORMAT *format,
                     DWORD_PTR callback, DWORD_PTR instance, DWORD flags);
MMRESULT waveOutClose(HWAVEOUT wave);
MMRESULT waveOutPrepareHeader(HWAVEOUT wave, LPWAVEHDR header, UINT size);
MMRESULT waveOutUnprepareHeader(HWAVEOUT wave, LPWAVEHDR header, UINT size);
MMRESULT waveOutWrite(HWAVEOUT wave, LPWAVEHDR header, UINT size);
MMRESULT waveOutPause(HWAVEOUT wave);
MMRESULT waveOutRestart(HWAVEOUT wave);
MMRESULT waveOutReset(HWAVEOUT wave);
MMRESULT waveOutBreakLoop(HWAVEOUT wave);
MMRESULT waveOutGetPosition(HWAVEOUT wave, LPMMTIME time, UINT size);
MMRESULT waveOutGetVolume(HWAVEOUT wave, LPDWORD volume);
MMRESULT waveOutSetVolume(HWAVEOUT wave, DWORD volume);
MMRESULT waveOutGetPitch(HWAVEOUT wave, LPDWORD pitch);
MMRESULT waveOutSetPitch(HWAVEOUT wave, DWORD pitch);
MMRESULT waveOutGetPlaybackRate(HWAVEOUT wave, LPDWORD rate);
MMRESULT waveOutSetPlaybackRate(HWAVEOUT wave, DWORD rate);

/* midiOut */
UINT midiOutGetNumDevs();
MMRESULT midiOutGetDevCaps(UINT_PTR device, LPMIDIOUTCAPS caps, UINT size);
MMRESULT midiOutOpen(LPHMIDIOUT midi, UINT device, DWORD_PTR callback, DWORD_PTR instance,
                     DWORD flags);
MMRESULT midiOutClose(HMIDIOUT midi);
MMRESULT midiOutShortMsg(HMIDIOUT midi, DWORD message);
MMRESULT midiOutLongMsg(HMIDIOUT midi, LPMIDIHDR header, UINT size);
MMRESULT midiOutPrepareHeader(HMIDIOUT midi, LPMIDIHDR header, UINT size);
MMRESULT midiOutUnprepareHeader(HMIDIOUT midi, LPMIDIHDR header, UINT size);
MMRESULT midiOutReset(HMIDIOUT midi);
MMRESULT midiOutGetVolume(HMIDIOUT midi, LPDWORD volume);
MMRESULT midiOutSetVolume(HMIDIOUT midi, DWORD volume);
MMRESULT midiOutCachePatches(HMIDIOUT midi, UINT bank, WORD *patches, UINT flags);
MMRESULT midiOutCacheDrumPatches(HMIDIOUT midi, UINT patch, WORD *keys, UINT flags);

/* Timers */
DWORD timeGetTime();
MMRESULT timeGetDevCaps(LPTIMECAPS caps, UINT size);
MMRESULT timeBeginPeriod(UINT period);
MMRESULT timeEndPeriod(UINT period);
MMRESULT timeSetEvent(UINT delay, UINT resolution, LPTIMECALLBACK proc, DWORD_PTR user,
                      UINT flags);
MMRESULT timeKillEvent(UINT id);

} /* namespace miniwin */

#pragma pack(pop)

#endif
