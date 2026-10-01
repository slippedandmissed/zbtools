/*
 * wavesound (Mohawk engine): wave sounds (Mohawk WAVE files played in
 * blocks through wavebuf)
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_fixed.h"
#include "os_manager.h"
#include "os_refcount.h"

/* Splits the samples into blocks: at each cue point, and at the loop's
   start and end. */
/* @zoombi32 0x0047a0ec */
void __cdecl waveObj::buildBlocks()
{
    unsigned long nextLoopStart;
    unsigned long nextLoopEnd;
    unsigned long nextCue;
    unsigned short cue;
    unsigned short cueCount;
    unsigned char *entry;
    unsigned long at;
    WaveBlock *block;
    unsigned long end;

    blockCount = 0;
    nextLoopStart = loops ? loopStart : 0xffffffff;
    nextLoopEnd = 0xffffffff;
    nextCue = 0xffffffff;
    if (cues && (cueCount = byteSwapShort(*(unsigned short *)(cues + 8))) != 0) {
        cue = 0;
        entry = cues + 0xa;
        nextCue = byteSwapLong(*(unsigned long *)entry);
    }
    at = 0;
    do {
        block = &blocks[blockCount];
        end = sampleCount;
        memset(block, 0, sizeof(WaveBlock));
        block->sound = this;
        block->call.proc = waveBlockDone;
        block->call.data = block;
        block->start = at;
        if (at == nextCue) {
            end = at;
            block->length = 0;
            block->cue = entry;
            entry += entry[4] + 6 & ~1;
            nextCue = ++cue < cueCount ? byteSwapLong(*(unsigned long *)entry) : 0xffffffff;
        } else {
            if (at == nextLoopStart) {
                loopBlock = block;
                end = nextLoopEnd = loopEnd;
                nextLoopStart = 0xffffffff;
                loopBlock = block;
            }
            if (end >= nextLoopStart)
                end = nextLoopStart;
            if (end >= nextCue) {
                end = nextCue;
                block->cue = entry;
                entry += entry[4] + 6 & ~1;
                nextCue = ++cue < cueCount ? byteSwapLong(*(unsigned long *)entry) : 0xffffffff;
            }
            if (end >= nextLoopEnd) {
                afterLoop = block;
                end = nextLoopEnd;
                nextLoopEnd = 0xffffffff;
            }
        }
        block->length = end - at;
        at = end;
        blockCount++;
    } while (at < sampleCount);
}

/* A wave sound from a Mohawk WAVE file in a handle; 0 on error. */
/* @zoombi32 0x0047a28d */
audioObj *__cdecl newWaveSound(short data)
{
    unsigned char *end;
    waveObj *wave;
    unsigned long *file;
    unsigned char *chunk;
    unsigned short count;

    file = (unsigned long *)lockHandleAlias(data);
    end = (unsigned char *)file + (byteSwapLong(file[1]) + 9 & ~1);
    if (byteSwapLong(file[0]) != 0x4d48574b || byteSwapLong(file[2]) != 0x57415645) {
        unlockHandle(data);
        setSoundError(0x29d0);
        return 0;
    }
    if ((wave = (waveObj *)newPtr(sizeof(waveObj))) == 0) {
        unlockHandle(data);
        setSoundError(memError());
        return 0;
    }
    new (wave) waveObj;
    wave->tag = 0x414f626a;
    wave->kind = 1;
    wave->rate = makeFixed(1, 0);
    wave->volume = makeFixed(1, 0);
    wave->active = sound.active;
    wave->data = data;
    wave->file = file;
    chunk = (unsigned char *)(file + 3);
    do {
        switch (byteSwapLong(*(unsigned long *)chunk)) {
        case 0x43756523:
            if (wave->cues) {
                setSoundError(0x29d1);
                goto fail;
            }
            wave->cues = chunk;
            break;
        case 0x44617461: {
            if (wave->samples) {
                setSoundError(0x29d1);
                goto fail;
            }
            unsigned char *dataChunk = chunk;
            wave->samples = dataChunk + 0x1c;
            wave->sampleRate = byteSwapShort(*(unsigned short *)(dataChunk + 8));
            wave->sampleCount = byteSwapLong(*(unsigned long *)(dataChunk + 0xa));
            wave->bitsPerSample = dataChunk[0xe];
            wave->channels = dataChunk[0xf];
            wave->encoding = byteSwapShort(*(unsigned short *)(dataChunk + 0x10));
            if ((wave->loops = byteSwapShort(*(unsigned short *)(dataChunk + 0x12))) != 0) {
                wave->loopStart = byteSwapLong(*(unsigned long *)(dataChunk + 0x14));
                wave->loopEnd = byteSwapLong(*(unsigned long *)(dataChunk + 0x18));
                if (wave->loopStart >= wave->loopEnd || wave->loopEnd > wave->sampleCount)
                    wave->loops = 0;
            }
            if (wave->bitsPerSample % 8 || !wave->sampleCount || wave->encoding & ~0x8000
                || wave->bitsPerSample > 8 && wave->encoding & 0x8000) {
                setSoundError(0x29d1);
                goto fail;
            }
            wave->blockAlign = wave->bitsPerSample / 8 * wave->channels;
            wave->samplesPerMs = fixedDiv(wave->sampleRate, 1000);
            wave->msPerSample = fixedDiv(1000, wave->sampleRate);
            break;
        }
        }
        chunk = chunk + (byteSwapLong(*(unsigned long *)(chunk + 4)) + 1 & ~1) + 8;
    } while (chunk < end);
    if (chunk > end || !wave->samples) {
        setSoundError(0x29d0);
    fail:
        disposePtr(wave);
        unlockHandle(data);
        return 0;
    }
    count = wave->cues ? byteSwapShort(*(unsigned short *)(wave->cues + 8)) : 0;
    count += wave->loops ? 3 : 1;
    if ((wave = (waveObj *)resizePtr(wave, count * sizeof(WaveBlock) + offsetof(waveObj, blocks))) == 0) {
        setSoundError(memError());
        goto fail;
    }
    wave->buildBlocks();
    resizePtr(wave, wave->blockCount * sizeof(WaveBlock) + offsetof(waveObj, blocks));
    wave->seek(0);
    wave->duration = fixedMul(wave->sampleCount, wave->msPerSample);
    wave->looping = wave->loops > 0;
    wave->resetLoop();
    return wave;
}

/* @zoombi32 0x0047a621 */
void __cdecl waveObj::release()
{
    unlockHandle(data);
}

/* Opens the wave device for the sound's format. */
/* @zoombi32 0x0047a633 */
short __cdecl waveObj::openDevice()
{
    PCMWAVEFORMAT format;

    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    format.wf.nChannels = channels;
    format.wf.nSamplesPerSec = sampleRate;
    format.wf.nAvgBytesPerSec = sampleRate * blockAlign;
    format.wf.nBlockAlign = blockAlign;
    format.wBitsPerSample = bitsPerSample;
    switch (openWaveOutDevice(&wave, device, &format, (LONG_PTR)waveCallback, (LONG_PTR)this,
                              CALLBACK_FUNCTION)) {
    case MMSYSERR_ALLOCATED:
        return setSoundError(0x29cd);
    case MMSYSERR_BADDEVICEID:
        return setSoundError(0x29ce);
    case WAVERR_BADFORMAT:
        return setSoundError(0x29d1);
    default:
        return setSoundError(0x29cc);
    case 0:
        if (lockPtr(file)) {
            wavebufClose(wave);
            wave = 0;
            return setSoundError(memError());
        }
        initLock(&lock, 1);
        return setSoundError(0);
    }
}

/* @zoombi32 0x0047a72a */
short __cdecl waveObj::setDeviceRate(long speed)
{
    return setSoundError(speed == rate ? 0 : wavebufSetPlaybackRate(wave, speed) ? 0x29d2 : 0);
}

/* @zoombi32 0x0047a75e */
short __cdecl waveObj::setDeviceVolume(long level)
{
    return setSoundError(wavebufSetVolume(wave, level) ? 0x29d3 : 0);
}

/* Queues the blocks from the position on (the first time round a loop
   from the middle of it, as a buffer of its own), and starts them unless
   paused. */
/* @zoombi32 0x0047a78a */
short __cdecl waveObj::startDevice(short paused)
{
    unsigned short i;
    unsigned long at;
    WaveBlock *block;

    wavebufReset(wave);
    wavebufPause(wave);
    unprepare();
    at = start;
    base = start;
    if (loops) {
        loopDone = at >= loopEnd;
        loopAdjust = 0;
        if (at > loopStart && at < loopEnd && loopBlock->header.dwLoops > 1) {
            memset(&loopHeader, 0, sizeof loopHeader);
            loopHeader.lpData = (LPSTR)(blockAlign * at + samples);
            loopHeader.dwBufferLength = blockAlign * loopEnd + samples - (unsigned char *)loopHeader.lpData;
            wavebufPrepareHeader(wave, &loopHeader, sizeof loopHeader);
            loopHeaderPrepared = 1;
            wavebufWrite(wave, &loopHeader, sizeof loopHeader);
            loopBlock->header.dwLoops--;
            at = loopStart;
        }
    }
    for (i = 0; i < blockCount; i++) {
        block = &blocks[i];
        if (at > block->start && at >= block->start + block->length) {
            block->header.dwFlags |= WHDR_DONE;
            continue;
        }
        block->header.dwUser = (DWORD_PTR)block;
        block->header.dwFlags = 0;
        if (loopBlock && loopBlock->header.dwLoops > 1) {
            if (block == loopBlock)
                block->header.dwFlags |= WHDR_BEGINLOOP;
            if (block == afterLoop)
                block->header.dwFlags |= WHDR_ENDLOOP;
        }
        if (at >= block->start) {
            block->header.lpData = (LPSTR)(blockAlign * at + samples);
            block->header.dwBufferLength = (block->length - (at - block->start)) * blockAlign;
        } else {
            block->header.lpData = (LPSTR)(blockAlign * block->start + samples);
            block->header.dwBufferLength = blockAlign * block->length;
        }
        wavebufPrepareHeader(wave, &block->header, sizeof block->header);
        block->prepared = 1;
        wavebufWrite(wave, &block->header, sizeof block->header);
    }
    if (!paused)
        wavebufRestart(wave);
    return setSoundError(0);
}

/* Stops the device, keeping the position (and the loops left). */
/* @zoombi32 0x0047a95f */
void __cdecl waveObj::haltDevice()
{
    MMTIME time;
    unsigned long at;

    time.wType = TIME_SAMPLES;
    wavebufGetPosition(wave, &time, sizeof time);
    at = time.u.sample + base;
    resetting = 1;
    wavebufReset(wave);
    resetting = 0;
    if (loops && !loopDone && !endingLoop && at >= loopStart) {
        if (loops != 0xffff)
            loopBlock->header.dwLoops -= (at - loopStart) / (loopEnd - loopStart);
        if (loopHeaderPrepared)
            loopBlock->header.dwLoops++;
    }
    start = positionAt(at);
}

/* @zoombi32 0x0047aa07 */
void __cdecl waveObj::closeDevice()
{
    unlockPtr(file);
    unprepare();
    wavebufReset(wave);
    wavebufClose(wave);
    wave = 0;
    removeLock(&lock);
}

/* @zoombi32 0x0047aa3f */
void __cdecl waveObj::unprepare()
{
    unsigned short i;

    if (loopHeaderPrepared) {
        wavebufUnprepareHeader(wave, &loopHeader, sizeof loopHeader);
        loopHeaderPrepared = 0;
    }
    for (i = 0; i < blockCount; i++)
        if (blocks[i].prepared) {
            wavebufUnprepareHeader(wave, &blocks[i].header, sizeof blocks[i].header);
            blocks[i].prepared = 0;
        }
}

/* @zoombi32 0x0047aab6 */
LONG_PTR __cdecl waveObj::deviceHandle()
{
    return wave;
}

/* The position in samples, given the device's (0xffffffff: ask it),
   allowing for the loop. */
/* @zoombi32 0x0047aac1 */
unsigned long __cdecl waveObj::positionAt(unsigned long sample)
{
    MMTIME time;
    unsigned long at;

    if (!active || !playing && !started)
        at = start;
    else {
        if (sample == 0xffffffff) {
            time.wType = TIME_SAMPLES;
            wavebufGetPosition(wave, &time, sizeof time);
            at = time.u.sample + base;
        } else
            at = sample;
        if (loops && at > loopEnd) {
            if (loopDone || endingLoop)
                at -= loopAdjust;
            else
                at = (at - loopStart) % (loopEnd - loopStart) + loopStart;
        }
    }
    return at;
}

/* The position in ms. */
/* @zoombi32 0x0047ab5a */
long __cdecl waveObj::position()
{
    return fixedMul(positionAt(0xffffffff), msPerSample);
}

/* @zoombi32 0x0047ab78 */
void __cdecl waveObj::pause()
{
    SoundNotice notice;

    wavebufPause(wave);
    started = 0;
    playing = 1;
    notice.what = 2;
    notifySound(this, &notice);
}

/* Lets the loop play out: breaks it on the device, and notes how much
   looping it did. */
/* @zoombi32 0x0047abae */
void __cdecl waveObj::endLoop()
{
    MMTIME time;
    unsigned long at;

    enterLock(&lock);
    if ((playing || started) && loops && !loopDone && !endingLoop) {
        wavebufBreakLoop(wave);
        loopBlock->header.dwLoops = 1;
        time.wType = TIME_SAMPLES;
        wavebufGetPosition(wave, &time, sizeof time);
        at = time.u.sample + base;
        loopAdjust = at > loopEnd ? (at - loopStart) / (loopEnd - loopStart) * (loopEnd - loopStart) : 0;
    }
    endingLoop = 1;
    leaveLock(&lock);
}

/* @zoombi32 0x0047ac61 */
void __cdecl waveObj::resetLoop()
{
    if (loops) {
        endingLoop = 0;
        loopBlock->header.dwLoops = loops == 0xffff ? 0xffffffff : loops + 1;
    }
}

/* @zoombi32 0x0047ac97 */
void __cdecl waveObj::resume()
{
    SoundNotice notice;

    wavebufRestart(wave);
    playing = 0;
    started = 1;
    notice.what = 3;
    notifySound(this, &notice);
}

/* Moves to `ms` (-1: the end). */
/* @zoombi32 0x0047accd */
short __cdecl waveObj::seek(long ms)
{
    unsigned long sample = ms == -1 ? sampleCount : fixedMul(ms, samplesPerMs);

    if (sample > sampleCount) {
        start = sampleCount;
        return setSoundError(0x29cf);
    }
    start = sample;
    return setSoundError(0);
}

/* Moves to the cue point named `text`. */
/* @zoombi32 0x0047ad19 */
short __cdecl waveObj::setText(const char *text, unsigned short length)
{
    unsigned short count;
    unsigned char *entry;

    if (cues) {
        entry = cues + 0xa;
        count = byteSwapShort(*(unsigned short *)(cues + 8));
        while (count--) {
            if (length <= entry[4] && !memicmp(text, entry + 5, length)) {
                start = byteSwapLong(*(unsigned long *)entry);
                return setSoundError(0);
            }
            entry += entry[4] + 6 & ~1;
        }
    }
    return setSoundError(0x29d4);
}

/* Not exact: the original keeps `this` in ebx; BCC32 4.5 uses eax. */
/* @zoombi32 0x0047ada8 */
short __cdecl waveObj::play(SoundNotify proc, LONG_PTR data)
{
    if (start < sampleCount)
        return audioObj::play(proc, data);
    return setSoundError(0x29cf);
}

/* The wave device's callback: passes finished blocks to waveBlockDone under
   the sound's lock. */
/* @zoombi32 0x0047ae14 */
void CALLBACK waveCallback(LONG_PTR, unsigned short message, DWORD_PTR instance, DWORD_PTR header, DWORD_PTR)
{
    HINSTANCE saved = osInstance(0);
    WaveBlock *block;

    if (message == WOM_DONE || message == 0x8000) {
        waveObj *wave = (waveObj *)instance;
        WAVEHDR *done = (WAVEHDR *)header;
        if (!wave->resetting && (block = (WaveBlock *)done->dwUser) != 0)
            deferCall(&wave->lock, &block->call);
    }
    osInstance((LONG_PTR)saved);
}

/* A block has played: reports its cue point, notes the end of the loop,
   and the end. */
/* @zoombi32 0x0047ae62 */
void waveBlockDone(void *data)
{
    WaveBlock *block = (WaveBlock *)data;
    waveObj *wave = block->sound;
    SoundNotice notice;

    {
        unsigned char *cue = block->cue;
        if (cue) {
            notice.what = 0;
            notice.value = cue[4];
            notice.data = (char *)cue + 5;
            notifySound(wave, &notice);
        }
    }
    if (block->header.dwFlags & WHDR_DONE) {
        if (block == wave->afterLoop) {
            wave->loopDone = 1;
            if (!wave->endingLoop) {
                wave->loopAdjust = (wave->loopEnd - wave->loopStart) * (wave->loopBlock->header.dwLoops - 1);
                if (wave->loops != 0xffff)
                    wave->loopBlock->header.dwLoops = 1;
            }
        }
        if (block->start + block->length == wave->sampleCount) {
            wave->started = 0;
            wave->start = wave->sampleCount;
            notice.what = 1;
            notice.value = 0;
            notifySound(wave, &notice);
        }
    }
}
