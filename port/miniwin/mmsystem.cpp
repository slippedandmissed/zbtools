/*
 * WINMM: waveOut, midiOut and the multimedia timers.
 *
 * Sound plays by the wall clock: service() mixes as many frames as time has
 * passed from every open waveOut device (each through an SDL audio stream,
 * which converts its format and rate) and queues them for SDL, so the game
 * sees buffers finish in real time even where the host isn't playing yet (a
 * browser waits for the user's first click). Finished headers are reported
 * through the device's callback, as a driver would from its interrupt.
 *
 * midiOut takes the game's MIDI and hands it to a synthesizer (midi.cpp).
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

/* midi.cpp */
void synthMessage(DWORD message);
void synthSysex(const uint8_t *data, size_t size);
void synthReset();
void synthRender(float *stereo, int frames, int rate);
bool synthAvailable();

static const int OUTPUT_RATE = 44100;
/* How far ahead of the host's playback sound is queued. */
static const int LATENCY_MS = 120;

static SDL_AudioDeviceID audioDevice;
static double pendingFrames;
static DWORD lastMix;
/* midiOut's volume: the left channel's in the low word, the right's in the
   high word. */
static DWORD midiVolume = 0xffffffff;

struct WaveOut
{
    uint32_t magic;
    WAVEFORMAT format;
    int bits;
    DWORD_PTR callback;
    DWORD_PTR instance;
    DWORD flags;
    SDL_AudioStream *stream;
    std::deque<WAVEHDR *> queue;
    size_t at; /* the header playing */
    DWORD offset; /* into it */
    WAVEHDR *loopStart;
    DWORD loopsLeft;
    bool breakLoop;
    bool paused;
    DWORD samples; /* played since opened or reset */
    float left, right;
};

static std::vector<WaveOut *> waveOuts;
static const uint32_t WAVE_MAGIC = 0x4d57574f;

bool openAudio()
{
    SDL_AudioSpec want, have;

    SDL_zero(want);
    want.freq = OUTPUT_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 1024;
    audioDevice = SDL_OpenAudioDevice(0, 0, &want, &have, 0);
    if (!audioDevice) {
        trace("no audio: %s", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    lastMix = now();
    return true;
}

void closeAudio()
{
    if (audioDevice)
        SDL_CloseAudioDevice(audioDevice);
    audioDevice = 0;
}

static WaveOut *waveOf(HWAVEOUT handle)
{
    WaveOut *wave = (WaveOut *)handle;
    for (WaveOut *w : waveOuts)
        if (w == wave && w->magic == WAVE_MAGIC)
            return w;
    return 0;
}

static void notify(WaveOut *wave, UINT message, WAVEHDR *header)
{
    switch (wave->flags & CALLBACK_TYPEMASK) {
    case CALLBACK_FUNCTION:
        ((void (*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR))wave->callback)(
            (HWAVEOUT)wave, message, wave->instance, (DWORD_PTR)header, 0);
        break;
    case CALLBACK_WINDOW:
        PostMessage((HWND)wave->callback, message, (WPARAM)wave, (LPARAM)header);
        break;
    case CALLBACK_TASK:
        unsupported("waveOut CALLBACK_TASK");
        break;
    }
}

/* Feeds the device's stream until it holds `frames` output frames or runs
   out of headers; headers used up go in `done`. A loop's headers stay
   queued until its last pass. */
static void feed(WaveOut *wave, int frames, std::vector<WAVEHDR *> &done)
{
    int frameBytes = (int)sizeof(float) * 2;

    while (!wave->paused && wave->at < wave->queue.size()
           && SDL_AudioStreamAvailable(wave->stream) < frames * frameBytes) {
        WAVEHDR *header = wave->queue[wave->at];
        if (header->dwFlags & WHDR_BEGINLOOP && !wave->loopStart && wave->offset == 0) {
            wave->loopStart = header;
            wave->loopsLeft = header->dwLoops;
        }
        DWORD length = header->dwBufferLength - wave->offset;
        DWORD chunk = std::min<DWORD>(length, 4096);
        chunk -= chunk % wave->format.nBlockAlign;
        if (chunk)
            SDL_AudioStreamPut(wave->stream, header->lpData + wave->offset, (int)chunk);
        wave->offset += chunk;
        wave->samples += chunk / wave->format.nBlockAlign;
        if (wave->offset + wave->format.nBlockAlign <= header->dwBufferLength && chunk)
            continue;
        wave->offset = 0;
        if (header->dwFlags & WHDR_ENDLOOP && wave->loopStart) {
            if (wave->loopsLeft > 1 && !wave->breakLoop) {
                wave->loopsLeft--;
                wave->at = (size_t)(std::find(wave->queue.begin(), wave->queue.end(),
                                              wave->loopStart)
                                    - wave->queue.begin());
                continue;
            }
            wave->loopStart = 0;
            wave->breakLoop = false;
        }
        if (wave->loopStart) {
            wave->at++;
            continue;
        }
        for (size_t i = 0; i <= wave->at; i++) {
            WAVEHDR *finished = wave->queue.front();
            wave->queue.pop_front();
            finished->dwFlags &= ~WHDR_INQUEUE;
            finished->dwFlags |= WHDR_DONE;
            done.push_back(finished);
        }
        wave->at = 0;
    }
}

/* --record: what's played, as a 16-bit stereo WAV file (its header
   rewritten after each block, so it's complete whenever the program
   stops). */
static FILE *recording;
static uint32_t recordedBytes;

void setRecordPath(const char *path)
{
    recording = ::fopen(path, "wb");
    if (!recording)
        trace("can't record to %s", path);
}

static void put32(uint8_t *p, uint32_t value)
{
    for (int i = 0; i < 4; i++)
        p[i] = (uint8_t)(value >> (8 * i));
}

static void record(const std::vector<float> &mix)
{
    if (!recording)
        return;
    std::vector<int16_t> samples(mix.size());
    for (size_t i = 0; i < mix.size(); i++)
        samples[i] = (int16_t)(mix[i] * 32767.0f);
    ::fseek(recording, 44 + recordedBytes, SEEK_SET);
    ::fwrite(samples.data(), sizeof(int16_t), samples.size(), recording);
    recordedBytes += (uint32_t)(samples.size() * sizeof(int16_t));
    uint8_t header[44] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
                          16, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 16, 0,
                          'd', 'a', 't', 'a', 0, 0, 0, 0};
    put32(header + 4, 36 + recordedBytes);
    put32(header + 24, OUTPUT_RATE);
    put32(header + 28, OUTPUT_RATE * 4);
    put32(header + 40, recordedBytes);
    ::fseek(recording, 0, SEEK_SET);
    ::fwrite(header, 1, sizeof header, recording);
    ::fflush(recording);
}

void serviceAudio()
{
    DWORD time = now();

    if (time == lastMix)
        return;
    pendingFrames += (double)(time - lastMix) * OUTPUT_RATE / 1000;
    lastMix = time;
    int frames = (int)pendingFrames;
    if (frames < 256)
        return;
    pendingFrames -= frames;
    if (frames > OUTPUT_RATE / 4)
        frames = OUTPUT_RATE / 4; /* after a long stall, don't catch up */

    std::vector<float> mix((size_t)frames * 2, 0.0f), part((size_t)frames * 2);
    std::vector<std::pair<WaveOut *, WAVEHDR *>> finished;
    std::vector<WaveOut *> open = waveOuts;
    for (WaveOut *wave : open) {
        std::vector<WAVEHDR *> done;
        feed(wave, frames, done);
        int got = SDL_AudioStreamGet(wave->stream, part.data(), frames * (int)sizeof(float) * 2);
        int gotFrames = got > 0 ? got / (int)(sizeof(float) * 2) : 0;
        for (int i = 0; i < gotFrames; i++) {
            mix[(size_t)i * 2] += part[(size_t)i * 2] * wave->left;
            mix[(size_t)i * 2 + 1] += part[(size_t)i * 2 + 1] * wave->right;
        }
        for (WAVEHDR *header : done)
            finished.push_back({wave, header});
    }
    if (synthAvailable()) {
        std::fill(part.begin(), part.end(), 0.0f);
        synthRender(part.data(), frames, OUTPUT_RATE);
        float left = (midiVolume & 0xffff) / 65535.0f, right = (midiVolume >> 16) / 65535.0f;
        for (size_t i = 0; i < mix.size(); i += 2) {
            mix[i] += part[i] * left;
            mix[i + 1] += part[i + 1] * right;
        }
    }
    for (float &sample : mix)
        sample = sample > 1.0f ? 1.0f : sample < -1.0f ? -1.0f : sample;
    record(mix);
    if (audioDevice) {
        Uint32 queued = SDL_GetQueuedAudioSize(audioDevice);
        Uint32 limit = (Uint32)(OUTPUT_RATE * LATENCY_MS / 1000 * sizeof(float) * 2);
        if (queued > limit * 3)
            SDL_ClearQueuedAudio(audioDevice); /* the host isn't playing: drop it */
        SDL_QueueAudio(audioDevice, mix.data(), (Uint32)(mix.size() * sizeof(float)));
    }
    for (auto &f : finished)
        if (waveOf((HWAVEOUT)f.first))
            notify(f.first, WOM_DONE, f.second);
}

UINT waveOutGetNumDevs()
{
    return 1;
}

MMRESULT waveOutGetDevCaps(UINT_PTR device, LPWAVEOUTCAPS caps, UINT size)
{
    if (device != 0 && device != (UINT_PTR)WAVE_MAPPER && device != 0xffff
        && !waveOf((HWAVEOUT)device))
        return MMSYSERR_BADDEVICEID;
    WAVEOUTCAPS c;
    memset(&c, 0, sizeof c);
    c.wMid = 1;
    c.wPid = 1;
    c.vDriverVersion = 0x400;
    strcpy(c.szPname, "miniwin Wave Out");
    c.dwFormats = 0xfff;
    c.wChannels = 2;
    c.dwSupport = WAVECAPS_VOLUME | WAVECAPS_LRVOLUME;
    memcpy(caps, &c, std::min<size_t>(size, sizeof c));
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutGetID(HWAVEOUT handle, UINT *device)
{
    if (!waveOf(handle))
        return MMSYSERR_INVALHANDLE;
    *device = 0;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutOpen(LPHWAVEOUT result, UINT device, const WAVEFORMAT *format,
                     DWORD_PTR callback, DWORD_PTR instance, DWORD flags)
{
    if (device != 0 && device != WAVE_MAPPER && device != 0xffff)
        return MMSYSERR_BADDEVICEID;
    if (format->wFormatTag != WAVE_FORMAT_PCM || format->nChannels < 1 || format->nChannels > 2)
        return WAVERR_BADFORMAT;
    int bits = ((const PCMWAVEFORMAT *)format)->wBitsPerSample;
    if (bits != 8 && bits != 16)
        return WAVERR_BADFORMAT;
    if (flags & WAVE_FORMAT_QUERY)
        return MMSYSERR_NOERROR;
    WaveOut *wave = new WaveOut();
    wave->magic = WAVE_MAGIC;
    wave->format = *format;
    wave->bits = bits;
    wave->callback = callback;
    wave->instance = instance;
    wave->flags = flags;
    wave->left = wave->right = 1.0f;
    wave->stream = SDL_NewAudioStream(bits == 8 ? AUDIO_U8 : AUDIO_S16LSB, (Uint8)format->nChannels,
                                      (int)format->nSamplesPerSec, AUDIO_F32SYS, 2, OUTPUT_RATE);
    if (!wave->stream) {
        delete wave;
        return MMSYSERR_NOMEM;
    }
    waveOuts.push_back(wave);
    if (result)
        *result = (HWAVEOUT)wave;
    notify(wave, WOM_OPEN, 0);
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutClose(HWAVEOUT handle)
{
    WaveOut *wave = waveOf(handle);

    if (!wave)
        return MMSYSERR_INVALHANDLE;
    if (!wave->queue.empty())
        return WAVERR_STILLPLAYING;
    notify(wave, WOM_CLOSE, 0);
    waveOuts.erase(std::find(waveOuts.begin(), waveOuts.end(), wave));
    SDL_FreeAudioStream(wave->stream);
    wave->magic = 0;
    delete wave;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutPrepareHeader(HWAVEOUT handle, LPWAVEHDR header, UINT)
{
    if (!waveOf(handle))
        return MMSYSERR_INVALHANDLE;
    header->dwFlags |= WHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutUnprepareHeader(HWAVEOUT handle, LPWAVEHDR header, UINT)
{
    if (!waveOf(handle))
        return MMSYSERR_INVALHANDLE;
    if (header->dwFlags & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    header->dwFlags &= ~WHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutWrite(HWAVEOUT handle, LPWAVEHDR header, UINT)
{
    WaveOut *wave = waveOf(handle);

    if (!wave)
        return MMSYSERR_INVALHANDLE;
    if (!(header->dwFlags & WHDR_PREPARED))
        return WAVERR_UNPREPARED;
    if (header->dwFlags & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    header->dwFlags &= ~WHDR_DONE;
    header->dwFlags |= WHDR_INQUEUE;
    header->lpNext = 0;
    wave->queue.push_back(header);
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutPause(HWAVEOUT handle)
{
    WaveOut *wave = waveOf(handle);
    if (!wave)
        return MMSYSERR_INVALHANDLE;
    wave->paused = true;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutRestart(HWAVEOUT handle)
{
    WaveOut *wave = waveOf(handle);
    if (!wave)
        return MMSYSERR_INVALHANDLE;
    wave->paused = false;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutReset(HWAVEOUT handle)
{
    WaveOut *wave = waveOf(handle);

    if (!wave)
        return MMSYSERR_INVALHANDLE;
    std::deque<WAVEHDR *> returned;
    returned.swap(wave->queue);
    wave->at = 0;
    wave->offset = 0;
    wave->loopStart = 0;
    wave->breakLoop = false;
    wave->samples = 0;
    wave->paused = false;
    SDL_AudioStreamClear(wave->stream);
    for (WAVEHDR *header : returned) {
        header->dwFlags &= ~WHDR_INQUEUE;
        header->dwFlags |= WHDR_DONE;
        notify(wave, WOM_DONE, header);
    }
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutBreakLoop(HWAVEOUT handle)
{
    WaveOut *wave = waveOf(handle);
    if (!wave)
        return MMSYSERR_INVALHANDLE;
    wave->breakLoop = true;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutGetPosition(HWAVEOUT handle, LPMMTIME time, UINT)
{
    WaveOut *wave = waveOf(handle);

    if (!wave)
        return MMSYSERR_INVALHANDLE;
    /* What's in the stream hasn't been heard yet. */
    int buffered = SDL_AudioStreamAvailable(wave->stream) / (int)(sizeof(float) * 2);
    DWORD behind = (DWORD)((double)buffered * wave->format.nSamplesPerSec / OUTPUT_RATE);
    DWORD samples = wave->samples > behind ? wave->samples - behind : 0;
    switch (time->wType) {
    case TIME_BYTES:
        time->u.cb = samples * wave->format.nBlockAlign;
        break;
    case TIME_MS:
        time->u.ms = (DWORD)((double)samples * 1000 / wave->format.nSamplesPerSec);
        break;
    default:
        time->wType = TIME_SAMPLES;
        time->u.sample = samples;
    }
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutGetVolume(HWAVEOUT handle, LPDWORD volume)
{
    WaveOut *wave = waveOf(handle);
    if (!wave) {
        *volume = 0xffffffff;
        return handle ? MMSYSERR_INVALHANDLE : MMSYSERR_NOERROR;
    }
    *volume = (DWORD)(wave->left * 0xffff) | (DWORD)(wave->right * 0xffff) << 16;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutSetVolume(HWAVEOUT handle, DWORD volume)
{
    WaveOut *wave = waveOf(handle);
    if (!wave)
        return handle ? MMSYSERR_INVALHANDLE : MMSYSERR_NOERROR;
    wave->left = (volume & 0xffff) / 65535.0f;
    wave->right = (volume >> 16) / 65535.0f;
    return MMSYSERR_NOERROR;
}

MMRESULT waveOutGetPitch(HWAVEOUT, LPDWORD)
{
    return MMSYSERR_NOTSUPPORTED;
}

MMRESULT waveOutSetPitch(HWAVEOUT, DWORD)
{
    return MMSYSERR_NOTSUPPORTED;
}

MMRESULT waveOutGetPlaybackRate(HWAVEOUT, LPDWORD)
{
    return MMSYSERR_NOTSUPPORTED;
}

MMRESULT waveOutSetPlaybackRate(HWAVEOUT, DWORD)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* midiOut: one device, a MIDI port as far as the game can tell (which its
   settings treat as a General MIDI device). */

static int midiOpenCount;

UINT midiOutGetNumDevs()
{
    return 1;
}

MMRESULT midiOutGetDevCaps(UINT_PTR device, LPMIDIOUTCAPS caps, UINT size)
{
    if (device != 0 && device != (UINT_PTR)MIDI_MAPPER && device != (UINT_PTR)0xffff)
        return MMSYSERR_BADDEVICEID;
    MIDIOUTCAPS c;
    memset(&c, 0, sizeof c);
    c.wMid = 1;
    c.wPid = 2;
    c.vDriverVersion = 0x400;
    strcpy(c.szPname, "miniwin MIDI");
    c.wTechnology = MOD_MIDIPORT;
    c.wVoices = 0;
    c.wNotes = 0;
    c.wChannelMask = 0xffff;
    c.dwSupport = MIDICAPS_VOLUME | MIDICAPS_LRVOLUME;
    memcpy(caps, &c, std::min<size_t>(size, sizeof c));
    return MMSYSERR_NOERROR;
}

static char midiDevice;

MMRESULT midiOutOpen(LPHMIDIOUT result, UINT device, DWORD_PTR, DWORD_PTR, DWORD)
{
    /* The mapper is 0xffff too: Windows 95's multimedia API is 16-bit
       underneath (the game opens it so). */
    if (device != 0 && device != MIDI_MAPPER && device != 0xffff)
        return MMSYSERR_BADDEVICEID;
    midiOpenCount++;
    *result = (HMIDIOUT)&midiDevice;
    return MMSYSERR_NOERROR;
}

static bool isMidi(HMIDIOUT handle)
{
    return handle == (HMIDIOUT)&midiDevice && midiOpenCount > 0;
}

MMRESULT midiOutClose(HMIDIOUT handle)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    if (!--midiOpenCount)
        synthReset();
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutShortMsg(HMIDIOUT handle, DWORD message)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    synthMessage(message);
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutLongMsg(HMIDIOUT handle, LPMIDIHDR header, UINT)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    if (!(header->dwFlags & MHDR_PREPARED))
        return MIDIERR_UNPREPARED;
    synthSysex((const uint8_t *)header->lpData, header->dwBufferLength);
    header->dwFlags |= MHDR_DONE;
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutPrepareHeader(HMIDIOUT handle, LPMIDIHDR header, UINT)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    header->dwFlags |= MHDR_PREPARED;
    header->dwFlags &= ~MHDR_DONE;
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutUnprepareHeader(HMIDIOUT handle, LPMIDIHDR header, UINT)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    header->dwFlags &= ~MHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutReset(HMIDIOUT handle)
{
    if (!isMidi(handle))
        return MMSYSERR_INVALHANDLE;
    synthReset();
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutGetVolume(HMIDIOUT, LPDWORD volume)
{
    *volume = midiVolume;
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutSetVolume(HMIDIOUT, DWORD volume)
{
    midiVolume = volume;
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutCachePatches(HMIDIOUT, UINT, WORD *, UINT)
{
    return MMSYSERR_NOTSUPPORTED;
}

MMRESULT midiOutCacheDrumPatches(HMIDIOUT, UINT, WORD *, UINT)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* Multimedia timers, run from service(), in order of their times. */

struct MmTimer
{
    UINT id;
    DWORD due;
    UINT period;
    LPTIMECALLBACK proc;
    DWORD_PTR user;
    bool periodic;
};

static std::vector<MmTimer> mmTimers;
static UINT nextTimerId = 1;

DWORD timeGetTime()
{
    service();
    return now();
}

MMRESULT timeGetDevCaps(LPTIMECAPS caps, UINT)
{
    caps->wPeriodMin = 1;
    caps->wPeriodMax = 1000000;
    return TIMERR_NOERROR;
}

MMRESULT timeBeginPeriod(UINT)
{
    return TIMERR_NOERROR;
}

MMRESULT timeEndPeriod(UINT)
{
    return TIMERR_NOERROR;
}

MMRESULT timeSetEvent(UINT delay, UINT, LPTIMECALLBACK proc, DWORD_PTR user, UINT flags)
{
    MmTimer timer;

    timer.id = nextTimerId++;
    timer.due = now() + delay;
    timer.period = delay ? delay : 1;
    timer.proc = proc;
    timer.user = user;
    timer.periodic = (flags & TIME_PERIODIC) != 0;
    mmTimers.push_back(timer);
    return timer.id;
}

MMRESULT timeKillEvent(UINT id)
{
    for (size_t i = 0; i < mmTimers.size(); i++)
        if (mmTimers[i].id == id) {
            mmTimers.erase(mmTimers.begin() + i);
            return TIMERR_NOERROR;
        }
    return MMSYSERR_INVALPARAM;
}

void serviceTimers()
{
    for (;;) {
        DWORD time = now();
        size_t next = mmTimers.size();
        for (size_t i = 0; i < mmTimers.size(); i++)
            if ((LONG)(time - mmTimers[i].due) >= 0
                && (next == mmTimers.size() || (LONG)(mmTimers[i].due - mmTimers[next].due) < 0))
                next = i;
        if (next == mmTimers.size())
            return;
        MmTimer timer = mmTimers[next];
        if (timer.periodic) {
            mmTimers[next].due += timer.period;
            if ((LONG)(time - mmTimers[next].due) > 100)
                mmTimers[next].due = time + timer.period; /* far behind: skip */
        } else
            mmTimers.erase(mmTimers.begin() + next);
        timer.proc(timer.id, 0, timer.user, 0, 0);
    }
}

} /* namespace miniwin */
