/*
 * wavestream (Mohawk engine): streamed wave sounds (Mohawk WAVE files read
 * from their file as they play, by a thread of their own)
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "basecamp.h"
#include "os_fixed.h"
#include "os_manager.h"
#include "os_refcount.h"
#include "os_threads.h"

/* A buffer for `samples` samples; 0 on error. */
/* @zoombi32 0x0047cb30 */
StreamBuffer *__cdecl wavestreamObj::newBuffer(unsigned long samples)
{
    unsigned long size = blockAlign * samples;
    StreamBuffer *buffer = (StreamBuffer *)newPtr(size + offsetof(StreamBuffer, data));

    if (buffer) {
        memset(buffer, 0, offsetof(StreamBuffer, data) + 4);
        buffer->sound = this;
        buffer->call.proc = streamBufferDone;
        buffer->call.data = buffer;
        buffer->capacity = samples;
        buffer->size = size;
        buffer->header.lpData = (LPSTR)buffer->data;
        buffer->header.dwUser = (DWORD_PTR)buffer;
        setSoundError(0);
    } else
        setSoundError(memError());
    return buffer;
}

/* @zoombi32 0x0047cb97 */
void __cdecl wavestreamObj::freeBuffer(StreamBuffer *buffer)
{
    if (buffer->prepared)
        unprepareBuffer(buffer);
    disposePtr(buffer);
}

/* Prepares the whole buffer (whatever it holds now) for the device. */
/* @zoombi32 0x0047cbbb */
short __cdecl wavestreamObj::prepareBuffer(StreamBuffer *buffer)
{
    DWORD length;

    if (!buffer->prepared) {
        length = buffer->header.dwBufferLength;
        buffer->header.dwBufferLength = buffer->size;
        buffer->prepared = !wavebufPrepareHeader(wave, &buffer->header, sizeof buffer->header);
        buffer->header.dwBufferLength = length;
    }
    return setSoundError(buffer->prepared ? 0 : 0x29cc);
}

/* Reads the buffer's samples, from the preloaded start where it can. */
/* @zoombi32 0x0047cc0f */
short __cdecl wavestreamObj::readBuffer(StreamBuffer *buffer)
{
    unsigned char *to = buffer->data;
    unsigned long size = buffer->header.dwBufferLength;
    unsigned long offset = blockAlign * buffer->start + dataOffset;
    unsigned long count;

    if (buffer->start < preloadSamples) {
        count = (preloadSamples - buffer->start) * blockAlign;
        if (count > size)
            count = size;
        moveMemory(buffer->data, blockAlign * buffer->start + preload, count);
        to += count;
        offset += count;
        size -= count;
    }
    return setSoundError(size > 0 ? readStream(resource, file, to, &size, offset) : 0);
}

/* Writes the buffer to the device, adding it to the ring. */
/* @zoombi32 0x0047cca9 */
short __cdecl wavestreamObj::queueBuffer(StreamBuffer *buffer)
{
    if (wavebufWrite(wave, &buffer->header, sizeof buffer->header))
        return setSoundError(0x29cc);
    queuedSamples += buffer->length;
    if (queued++ == 0) {
        ring = buffer;
        buffer->next = buffer;
        buffer->prev = buffer;
    } else {
        buffer->next = ring;
        buffer->prev = ring->prev;
        ring->prev->next = buffer;
        ring->prev = buffer;
    }
    buffer->queued = 1;
    buffer->done = 0;
    return setSoundError(0);
}

/* @zoombi32 0x0047cd26 */
void __cdecl wavestreamObj::unprepareBuffer(StreamBuffer *buffer)
{
    DWORD length;

    if (buffer->prepared) {
        length = buffer->header.dwBufferLength;
        buffer->header.dwBufferLength = buffer->size;
        wavebufUnprepareHeader(wave, &buffer->header, sizeof buffer->header);
        buffer->header.dwBufferLength = length;
        buffer->prepared = 0;
    }
}

/* A streamed wave sound from a Mohawk WAVE file: the resource `resource`,
   or (resource 0) the open file `file`; the first `preloadMs` ms read in
   now. 0 on error. */
/* @zoombi32 0x0047cd5c */
audioObj *__cdecl newStreamedWave(long resource, LONG_PTR file, long preloadMs)
{
    unsigned long size;
    unsigned long chunk[2];
    unsigned long samples;
    unsigned char *entry;
    unsigned short cue;
    unsigned long header[3];
    struct /* the Data chunk's header */
    {
        unsigned long tag;
        unsigned long size;
        unsigned short sampleRate;
        unsigned long sampleCount;
        unsigned char bitsPerSample;
        unsigned char channels;
        unsigned short encoding;
        unsigned short loops;
        unsigned long loopStart;
        unsigned long loopEnd;
        unsigned long unknown1C;
    } format;
    wavestreamObj *wave;
    unsigned char *cues;
    unsigned long offset;

    size = 0xc;
    offset = 0;
    if (readStream(resource, file, header, &size, offset) == 0)
        offset += size;
    else {
        setSoundError(resourceError());
        return 0;
    }
    if (byteSwapLong(header[0]) != 0x4d48574b || byteSwapLong(header[2]) != 0x57415645) {
        setSoundError(0x29d0);
        return 0;
    }
    wave = 0;
    cues = 0;
    do {
        size = 8;
        if (readStream(resource, file, chunk, &size, offset) == 0) {
            chunk[1] = byteSwapLong(chunk[1]);
            switch (chunk[0] = byteSwapLong(chunk[0])) {
            case 0x43756523:
                if ((cues = (unsigned char *)newPtr(chunk[1] + 8)) == 0) {
                    setSoundError(memError());
                    goto fail;
                }
                size = chunk[1];
                if (readStream(resource, file, cues + 8, &size, offset + 8)) {
                    setSoundError(resourceError());
                    goto fail;
                }
                ((unsigned long *)cues)[0] = chunk[0];
                ((unsigned long *)cues)[1] = chunk[1];
                if ((*(unsigned short *)(cues + 8) = byteSwapShort(*(unsigned short *)(cues + 8))) != 0) {
                    entry = cues + 0xa;
                    for (cue = 0; cue < *(unsigned short *)(cues + 8); cue++) {
                        *(unsigned long *)entry = byteSwapLong(*(unsigned long *)entry);
                        entry += entry[4] + 6 & ~1;
                    }
                } else {
                    disposePtr(cues);
                    cues = 0;
                }
                break;
            case 0x44617461:
                size = 0x14;
                if (readStream(resource, file, &format.sampleRate, &size, offset + 8) == 0)
                    samples = offset + 0x1c;
                else {
                    setSoundError(resourceError());
                    goto fail;
                }
                break;
            }
            offset += chunk[1] + 8;
        } else {
            setSoundError(resourceError());
            goto fail;
        }
    } while (chunk[0] != 0x44617461);
    if ((wave = (wavestreamObj *)newPtr(sizeof(wavestreamObj))) != 0)
        new (wave) wavestreamObj;
    else {
        setSoundError(memError());
        goto fail;
    }
    wave->tag = 0x414f626a;
    wave->kind = 1;
    wave->file = file;
    wave->resource = resource;
    wave->active = sound.active;
    wave->cues = cues;
    wave->dataOffset = samples;
    wave->rate = makeFixed(1, 0);
    wave->volume = makeFixed(1, 0);
    wave->sampleRate = byteSwapShort(format.sampleRate);
    wave->sampleCount = byteSwapLong(format.sampleCount);
    wave->bitsPerSample = format.bitsPerSample;
    wave->channels = format.channels;
    wave->encoding = byteSwapShort(format.encoding);
    if ((wave->loops = byteSwapShort(format.loops)) != 0) {
        wave->loopStart = byteSwapLong(format.loopStart);
        wave->loopEnd = byteSwapLong(format.loopEnd);
        if (wave->loopStart >= wave->loopEnd || wave->loopEnd > wave->sampleCount)
            wave->loops = 0;
    }
    if (wave->bitsPerSample % 8 || !wave->sampleCount || wave->encoding & ~0x8000
        || wave->bitsPerSample > 8 && wave->encoding & 0x8000) {
        setSoundError(0x29d1);
    fail:
        /* The original reads wave->cues here even before wave is allocated. */
        if (wave->cues)
            disposePtr(wave->cues);
        if (wave)
            disposePtr(wave);
        return 0;
    }
    wave->blockAlign = wave->channels * ((wave->bitsPerSample + 7) / 8);
    wave->samplesPerMs = fixedDiv(wave->sampleRate, 1000);
    wave->msPerSample = fixedDiv(1000, wave->sampleRate);
    wave->bufferSamples = 0x1000L / wave->blockAlign;
    wave->maxQueued = wave->sampleRate * 3;
    wave->seek(0);
    wave->duration = fixedMul(wave->sampleCount, wave->msPerSample);
    wave->looping = wave->loops > 0;
    wave->resetLoop();
    if (preloadMs) {
        wave->preloadSamples = fixedMul(preloadMs, wave->samplesPerMs);
        if (wave->preloadSamples < wave->bufferSamples)
            wave->preloadSamples = wave->bufferSamples;
        if (wave->preloadSamples > wave->sampleCount)
            wave->preloadSamples = wave->sampleCount;
        size = wave->blockAlign * wave->preloadSamples;
        if ((wave->preload = (unsigned char *)newPtr(size)) == 0)
            goto fail;
        if (readStream(resource, file, wave->preload, &size, wave->dataOffset)) {
            setSoundError(resourceError());
            disposePtr(wave->preload);
            goto fail;
        }
    }
    return wave;
}

/* @zoombi32 0x0047d243 */
void __cdecl wavestreamObj::release()
{
    if (preload)
        disposePtr(preload);
    if (cues)
        disposePtr(cues);
    if (!resource)
        closeFile(file, 0);
}

/* Opens the wave device for the sound's format, and starts the thread that
   reads for it. */
/* @zoombi32 0x0047d27f */
short __cdecl wavestreamObj::openDevice()
{
    PCMWAVEFORMAT format;

    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    format.wf.nChannels = channels;
    format.wf.nSamplesPerSec = sampleRate;
    format.wf.nAvgBytesPerSec = sampleRate * blockAlign;
    format.wf.nBlockAlign = blockAlign;
    format.wBitsPerSample = bitsPerSample;
    switch (openWaveOutDevice(&wave, device, &format, (LONG_PTR)streamCallback, (LONG_PTR)this,
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
        if (cues && lockPtr(cues)) {
            setSoundError(memError());
        fail:
            wavebufClose(wave);
            wave = 0;
            return sound.error;
        }
        event = newEvent(0);
        if (!event) {
            setSoundError(threadError());
        unlock:
            unlockPtr(cues);
            goto fail;
        }
        thread = createThread(streamThread, (LONG_PTR)this, 0x1000, 1);
        if (!thread) {
            setSoundError(threadError());
            deleteSync(event);
            goto unlock;
        }
        initLock(&lock, 1);
        return setSoundError(0);
    }
}

/* @zoombi32 0x0047d3dd */
short __cdecl wavestreamObj::setDeviceRate(long speed)
{
    return setSoundError(speed == rate ? 0 : wavebufSetPlaybackRate(wave, speed) ? 0x29d2 : 0);
}

/* @zoombi32 0x0047d411 */
short __cdecl wavestreamObj::setDeviceVolume(long level)
{
    return setSoundError(wavebufSetVolume(wave, level) ? 0x29d3 : 0);
}

/* Starts reading from the position, letting the thread run (at the
   caller's priority) until it has queued the first buffers; then starts
   the device unless paused. */
/* @zoombi32 0x0047d43d */
short __cdecl wavestreamObj::startDevice(short paused)
{
    LONG_PTR self;
    short priority;

    playing = paused;
    wavebufReset(wave);
    wavebufPause(wave);
    atEnd = 0;
    readPosition = base = start;
    queued = 0;
    queuedSamples = 0;
    ring = 0;
    loopAdjust = 0;
    loopEndBuffer = 0;
    loopsRead = loopsPlayed;
    self = currentThread();
    priority = threadPriority(self);
    setThreadPriority(self, 3);
    setThreadPriority(thread, 3);
    setEvent(event);
    resumeThread(thread);
    yieldThread(0);
    setThreadPriority(thread, 1);
    setThreadPriority(self, priority);
    if (!playing)
        wavebufRestart(wave);
    streaming = 1;
    return setSoundError(0);
}

/* Stops the device and the thread, keeping the position (and the loops
   played); drops the queued buffers. */
/* @zoombi32 0x0047d50b */
void __cdecl wavestreamObj::haltDevice()
{
    MMTIME time;
    unsigned long at;
    StreamBuffer *buffer;

    if (streaming) {
        streaming = 0;
        resetting = 1;
        lockFile(file, -1);
        time.wType = TIME_SAMPLES;
        wavebufGetPosition(wave, &time, sizeof time);
        at = time.u.sample + base;
        wavebufReset(wave);
        resetEvent(event);
        if (currentThread() != thread)
            suspendThread(thread);
        resetting = 0;
        unlockFile(file);
        if (loops && at >= loopStart && loopEndBuffer && !loopDone && !endingLoop)
            loopsPlayed += (at - loopStart) / (loopEnd - loopStart);
        start = positionAt(at);
    }
    if (queued) {
        do {
            buffer = ring;
            ring = ring->next;
            buffer->queued = 0;
            if (!buffer->keep)
                freeBuffer(buffer);
        } while (--queued);
        ring = 0;
        queuedSamples = 0;
    }
}

/* @zoombi32 0x0047d62e */
void __cdecl wavestreamObj::closeDevice()
{
    haltDevice();
    deleteSync(thread);
    deleteSync(event);
    wavebufReset(wave);
    if (cues)
        unlockPtr(cues);
    if (loopBuffer) {
        freeBuffer(loopBuffer);
        loopBuffer = 0;
    }
    wavebufClose(wave);
    wave = 0;
    removeLock(&lock);
}

/* Reads and queues buffers until three seconds' worth are queued (or it
   reaches the end, or the end of the preloaded start before the device has
   started): recycling the ones played, keeping the loop's first buffer for
   each pass, and marking the loop's last one to loop on the device where
   it can. On error it halts the sound and tells it it finished. */
/* @zoombi32 0x0047d69c */
short __cdecl wavestreamObj::stream()
{
    StreamBuffer *spare = 0;
    short locked;
    SoundNotice notice;
    StreamBuffer *buffer;
    unsigned long end;
    unsigned char *entry;
    unsigned short cue;

    if (!started || playing)
        return setSoundError(0);
    while (queued > 0 && ring->done) {
        buffer = ring;
        buffer->next->prev = buffer->prev;
        buffer->prev->next = buffer->next;
        buffer->queued = 0;
        queued--;
        ring = queued ? buffer->next : 0;
        queuedSamples -= buffer->length;
        if (!buffer->keep) {
            buffer->next = spare;
            spare = buffer;
        }
    }
    locked = 0;
    while (!atEnd && !resetting && (queued < 2 || queuedSamples < maxQueued)) {
        if (loops && readPosition == loopStart && loopBuffer && !loopBuffer->queued) {
            buffer = loopBuffer;
            goto queue;
        }
        if ((buffer = spare) != 0)
            spare = buffer->next;
        else if ((buffer = newBuffer(bufferSamples)) == 0) {
            haltDevice();
            started = 0;
            goto failed;
        }
        buffer->keep = 0;
        end = loops ? readPosition < loopStart ? loopStart
                    : readPosition < loopEnd ? loopEnd
                    : sampleCount
              : sampleCount;
        if (end - readPosition > buffer->capacity)
            end = readPosition + buffer->capacity;
        buffer->cue = 0;
        if (cues) {
            entry = cues + 0xa;
            for (cue = 0; cue < *(unsigned short *)(cues + 8); cue++) {
                if (readPosition < *(unsigned long *)entry && end >= *(unsigned long *)entry) {
                    end = *(unsigned long *)entry;
                    buffer->cue = entry;
                    break;
                }
                entry += entry[4] + 6 & ~1;
            }
        }
        if (!streaming && !readPosition && preload && end > preloadSamples)
            end = preloadSamples;
        buffer->start = readPosition;
        buffer->length = end - readPosition;
        buffer->header.dwBufferLength = blockAlign * buffer->length;
        if (!locked) {
            locked = 1;
            if (lockFile(file, -1)) {
                setSoundError(fileError());
                goto failed;
            }
        }
        if (readBuffer(buffer)) {
            freeBuffer(buffer);
            goto failed;
        }
    queue:
        buffer->header.dwFlags &= ~(WHDR_DONE | WHDR_BEGINLOOP | WHDR_ENDLOOP);
        buffer->header.dwLoops = 0;
        if (loops && readPosition == loopStart && !loopDone && !endingLoop) {
            if (readPosition + buffer->length == loopEnd) {
                buffer->header.dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
                buffer->header.dwLoops = loops == 0xffff ? 0xffffffff : loops - loopsRead + 1;
            } else if (!loopBuffer && !buffer->keep) {
                buffer->keep = 1;
                loopBuffer = buffer;
            }
        }
        enterLock(&lock);
        if (prepareBuffer(buffer) || queueBuffer(buffer)) {
            leaveLock(&lock);
            freeBuffer(buffer);
        failed:
            notice.what = 1;
            notice.value = sound.error;
            haltDevice();
            started = 0;
            notifySound(this, &notice);
            sound.error = notice.value;
            goto done;
        }
        readPosition += buffer->length;
        if (buffer->header.dwFlags & (WHDR_BEGINLOOP | WHDR_ENDLOOP))
            loopEndBuffer = buffer;
        else if (loops && readPosition == loopEnd && !loopDone && !endingLoop
                 && loopsRead < loops) {
            readPosition = loopStart;
            if (loops != 0xffff)
                loopsRead++;
            else {
                loopsPlayed = 0;
                loopsRead = 0;
            }
        }
        if (readPosition >= sampleCount)
            atEnd = 1;
        leaveLock(&lock);
        if (!streaming && preload && preloadSamples == readPosition)
            break;
    }
    setSoundError(0);
done:
    if (locked)
        unlockFile(file);
    while ((buffer = spare) != 0) {
        spare = spare->next;
        freeBuffer(buffer);
    }
    return sound.error;
}

/* @zoombi32 0x0047dab4 */
LONG_PTR __cdecl wavestreamObj::deviceHandle()
{
    return wave;
}

/* The position in samples, given the device's (0xffffffff: ask it),
   allowing for the loop. */
/* @zoombi32 0x0047dabf */
unsigned long __cdecl wavestreamObj::positionAt(unsigned long sample)
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
/* @zoombi32 0x0047db64 */
long __cdecl wavestreamObj::position()
{
    return fixedMul(positionAt(0xffffffff), msPerSample);
}

/* The reading thread: each time its event is set, reads more (halting
   the sound, and itself, when it can't). */
/* @zoombi32 0x0047db85 */
void streamThread(LONG_PTR data)
{
    wavestreamObj *wave = (wavestreamObj *)data;

    while (!waitSync(wave->event, -1)) {
        resetEvent(wave->event);
        if (wave->started && !wave->stream())
            continue;
        wave->haltDevice();
        suspendThread(wave->thread);
    }
}

/* @zoombi32 0x0047dbcc */
void __cdecl wavestreamObj::pause()
{
    SoundNotice notice;

    wavebufPause(wave);
    started = 0;
    playing = 1;
    notice.what = 2;
    notifySound(this, &notice);
}

/* Reads from the resource `resource`, or (resource 0) the file `file`. */
/* @zoombi32 0x0047dc02 */
short __cdecl readStream(long resource, LONG_PTR file, void *buffer, unsigned long *size,
                         unsigned long offset)
{
    if (resource)
        return setSoundError(readResourceBytes(resource, buffer, size, offset));
    if (seekFile(file, offset, 0) == 0xffffffff || readFile(file, buffer, (long *)size))
        return setSoundError(fileError());
    return setSoundError(0);
}

/* Lets the loop play out: breaks it on the device, and notes how much
   looping it did. */
/* @zoombi32 0x0047dc62 */
void __cdecl wavestreamObj::endLoop()
{
    MMTIME time;
    unsigned long at;

    enterLock(&lock);
    if ((playing || started) && loops && !loopDone && !endingLoop) {
        if (loopEndBuffer) {
            wavebufBreakLoop(wave);
            loopEndBuffer->header.dwLoops = 1;
        }
        time.wType = TIME_SAMPLES;
        wavebufGetPosition(wave, &time, sizeof time);
        at = time.u.sample + base;
        loopAdjust = at > loopEnd ? (at - loopStart) / (loopEnd - loopStart) * (loopEnd - loopStart) : 0;
    }
    endingLoop = 1;
    leaveLock(&lock);
}

/* @zoombi32 0x0047dd32 */
void __cdecl wavestreamObj::resetLoop()
{
    if (loops) {
        loopsPlayed = 0;
        loopDone = 0;
        endingLoop = 0;
    }
}

/* @zoombi32 0x0047dd5c */
void __cdecl wavestreamObj::resume()
{
    SoundNotice notice;

    wavebufRestart(wave);
    playing = 0;
    started = 1;
    notice.what = 3;
    notifySound(this, &notice);
}

/* Moves to `ms` (-1: the end). */
/* @zoombi32 0x0047dd92 */
short __cdecl wavestreamObj::seek(long ms)
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
/* @zoombi32 0x0047dde7 */
short __cdecl wavestreamObj::setText(const char *text, unsigned short length)
{
    unsigned short count;
    unsigned char *entry;

    if (cues) {
        entry = cues + 0xa;
        count = *(unsigned short *)(cues + 8);
        while (count--) {
            if (length <= entry[4] && !memicmp(text, entry + 5, entry[4])) {
                start = *(unsigned long *)entry;
                return setSoundError(0);
            }
            entry += entry[4] + 6 & ~1;
        }
    }
    return setSoundError(0x29d4);
}

/* Not exact: the original keeps `this` in ebx; BCC32 4.5 uses eax. */
/* @zoombi32 0x0047de63 */
short __cdecl wavestreamObj::play(SoundNotify proc, LONG_PTR data)
{
    if (start < sampleCount)
        return audioObj::play(proc, data);
    return setSoundError(0x29cf);
}

/* @zoombi32 0x0047de96 */
short setSoundError(short error)
{
    return sound.error = error;
}

/* Placement new that zeroes the object first. */
/* @zoombi32 0x0047dea7 */
void *__cdecl audioObj::operator new(size_t size, void *where)
{
    return memset(where, 0, size);
}

/* The wave device's callback: passes finished buffers to streamBufferDone
   under the sound's lock. */
/* @zoombi32 0x0047df34 */
void CALLBACK streamCallback(LONG_PTR, unsigned short message, DWORD_PTR instance, DWORD_PTR header, DWORD_PTR)
{
    HINSTANCE saved = osInstance(0);
    StreamBuffer *buffer;

    if (message == WOM_DONE) {
        wavestreamObj *wave = (wavestreamObj *)instance;
        WAVEHDR *done = (WAVEHDR *)header;
        if (!wave->resetting && (buffer = (StreamBuffer *)done->dwUser) != 0)
            deferCall(&wave->lock, &buffer->call);
    }
    osInstance((LONG_PTR)saved);
}

/* A buffer has played: wakes the thread, reports its cue point, counts
   loops, and notes the end. */
/* @zoombi32 0x0047df7a */
void streamBufferDone(void *data)
{
    StreamBuffer *buffer = (StreamBuffer *)data;
    wavestreamObj *wave = buffer->sound;
    SoundNotice notice;

    buffer->done = 1;
    wave->start = buffer->start + buffer->length;
    setEvent(wave->event);
    {
        unsigned char *cue = buffer->cue;
        if (cue) {
            notice.what = 0;
            notice.value = cue[4];
            notice.data = (char *)cue + 5;
            notifySound(wave, &notice);
        }
    }
    if (wave->loops && buffer->start + buffer->length == wave->loopEnd) {
        if (buffer == wave->loopEndBuffer) {
            if (!wave->endingLoop)
                wave->loopAdjust = (wave->loopEnd - wave->loopStart) * (buffer->header.dwLoops - 1);
            wave->loopDone = 1;
            wave->loopsPlayed = wave->loops;
            buffer->header.dwLoops = 1;
            wave->loopEndBuffer = 0;
        } else if (wave->loopsPlayed++ >= wave->loops) {
            if (!wave->endingLoop)
                wave->loopAdjust = (wave->loopEnd - wave->loopStart) * (wave->loopsPlayed - 1);
            wave->loopDone = 1;
            wave->loopsPlayed = wave->loops;
        }
    }
    if (buffer->start + buffer->length == wave->sampleCount && wave->atEnd) {
        wave->started = 0;
        wave->start = wave->sampleCount;
        notice.what = 1;
        notice.value = 0;
        notifySound(wave, &notice);
    }
}

/* Tells the sound's owner. */
/* @zoombi32 0x0047e0d3 */
void __cdecl notifySound(audioObj *object, SoundNotice *notice)
{
    if (object->notify)
        object->notify((LONG_PTR)object, notice, object->cookie);
}
