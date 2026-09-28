/*
 * module_47e0ec (Mohawk engine): WaveMix's mixer: the wave devices it mixes
 * for (wmxDevice), their mixed channels (wmxMixer) and the WaveMix object
 * base class (wmxObject)
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_46d754.h"
#include "os_localmem.h"
#include "os_manager.h"

/* @zoombi32 0x0047e0ec */
__cdecl wmxDevice::wmxDevice()
{
    isOpen = 0;
}

/* @zoombi32 0x0047e0f9 */
__cdecl wmxDevice::~wmxDevice()
{
    if (isOpen) {
        buffer->close();
        delete buffer;
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;
        else
            wmx.devices = next;
    }
}

/* The output buffer's notice that it has played to `played` and can be
   written from `written`: the channels note what's played, and the device
   mixes on. */
/* @zoombi32 0x0047e163 */
void wmxDeviceNotify(long data, unsigned long played, unsigned long written)
{
    wmxDevice *device = (wmxDevice *)data;
    wmxObject *object;

    if (written > device->written)
        device->written = written;
    if ((object = device->objects) != 0)
        do
            object->played(played);
        while ((object = object->deviceNext) != device->objects);
    device->mix(device->written);
}

/* Opens the output buffer for the wave device in the best format it
   supports within [WaveMix]'s. */
/* @zoombi32 0x0047e1a0 */
unsigned short wmxDevice::open(unsigned short device)
{
    long rate;
    unsigned long formats;
    unsigned short error;

    if (isOpen)
        return MMSYSERR_ERROR;
    if ((error = newWavebuf(device, &buffer)) != 0)
        return error;
    this->device = device;
    objectCount = 0;
    objects = 0;
    written = 0;
    memset(&format, 0, sizeof format);
    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    wmxDeviceFormats(buffer, (unsigned long *)&rate, &formats);
    switch (formats) {
    case WAVE_FORMAT_4S16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 4;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 4;
        break;
    case WAVE_FORMAT_4M16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 4;
        break;
    case WAVE_FORMAT_4S08:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 4;
        break;
    case WAVE_FORMAT_4M08:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 1;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 4;
        break;
    case WAVE_FORMAT_2S16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 4;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 2;
        break;
    case WAVE_FORMAT_2M16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 2;
        break;
    case WAVE_FORMAT_2S08:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 2;
        break;
    case WAVE_FORMAT_2M08:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 1;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 2;
        break;
    case WAVE_FORMAT_1S16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 4;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 1;
        break;
    case WAVE_FORMAT_1M16:
        format.wBitsPerSample = 16;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 1;
        break;
    case WAVE_FORMAT_1S08:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 2;
        format.wf.nChannels = 2;
        format.wf.nSamplesPerSec = 1;
        break;
    default:
        format.wBitsPerSample = 8;
        format.wf.nBlockAlign = 1;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 1;
        break;
    }
    format.wf.nSamplesPerSec *= rate;
    format.wf.nAvgBytesPerSec = format.wf.nBlockAlign * format.wf.nSamplesPerSec;
    if ((error = buffer->open(&format, wmxDeviceNotify, (long)this)) != 0) {
        delete buffer;
        return error;
    }
    prev = 0;
    if ((next = wmx.devices) != 0)
        next->prev = 0;
    wmx.devices = this;
    isOpen = 1;
    return 0;
}

/* Mixes into the output buffer from `at`, as far as it lets. */
/* @zoombi32 0x0047e45e */
void wmxDevice::mix(unsigned long at)
{
    void *first;
    void *second;
    unsigned long firstLength;
    unsigned long secondLength;

    if (buffer->lockBuffer(at, &first, &firstLength, &second, &secondLength))
        return;
    mixInto(at, first, firstLength);
    if (secondLength)
        mixInto(at + firstLength, second, secondLength);
    at += firstLength;
    at += secondLength;
    written = at;
    buffer->unlockBuffer();
}

/* Mixes `count` samples of every channel into `out`: the first to play
   copies, the rest add; silence if none plays. */
/* @zoombi32 0x0047e4d2 */
void wmxDevice::mixInto(unsigned long at, void *out, unsigned long count)
{
    unsigned short mixed = 0;
    wmxObject *object = objects;
    unsigned short i;

    for (i = 0; i < objectCount; i++, object = object->deviceNext)
        mixed |= object->mix(at, out, count, !mixed);
    if (!mixed)
        silence(out, count);
}

/* The best format the buffer supports within [WaveMix]'s rate, sample size
   and channels (one WAVE_FORMAT_ bit). */
/* @zoombi32 0x0047e52b */
void wmxDeviceFormats(wavebuf *buffer, unsigned long *rate, unsigned long *formats)
{
    buffer->formats(rate, formats);
    if (wmx.rate < 44100 && *formats & 0xf00)
        *formats &= ~0xf00;
    if (wmx.rate < 22050 && *formats & 0xf0)
        *formats &= ~0xf0;
    if (wmx.frameSize < 16 && *formats & ~0xcc)
        *formats &= ~0xcc;
    if (!wmx.stereo && *formats & ~0xaa)
        *formats &= ~0xaa;
    {
        unsigned long bit = 0x80000000;
        do {
            if (*formats & bit) {
                *formats &= bit;
                break;
            }
            bit >>= 1;
        } while (bit);
    }
}

/* @zoombi32 0x0047e5b8 */
void wmxDevice::silence(void *out, unsigned long count)
{
    switch (format.wBitsPerSample) {
    case 8:
        fillBytes(out, 0x80, count);
        break;
    case 16:
        fillWords(out, 0, count);
        break;
    }
}

/* @zoombi32 0x0047e5f3 */
void __cdecl wmxDevice::operator delete(void *block)
{
    localFree(block);
}

/* @zoombi32 0x0047e600 */
__cdecl wmxMixer::wmxMixer(wmxDevice *device, PCMWAVEFORMAT *format, long callback,
                           long instance, unsigned long flags)
    : wmxObject(device, format, callback, instance, flags)
{
    queue = 0;
    queueTail = 0;
    rate = 0;
    volume = 0;
    levels = 0;
    channels = format->wf.nChannels > device->format.wf.nChannels ? format->wf.nChannels
                                                                  : device->format.wf.nChannels;
    reset();
    setPlaybackRate(makeFixed(1, 0));
    setLevels(0xffffffff);
    setVolume(makeFixed(1, 0));
}

/* @zoombi32 0x0047e699 */
__cdecl wmxMixer::~wmxMixer()
{
    if (queue) {
        reset();
        close();
    }
}

/* Fills `table` with 8-bit samples scaled by `volume`; true if that
   changes nothing. */
/* Not exact: the original keeps `table` in edi and `changed` on the stack;
   BCC32 4.5 gives the register to `changed`. */
/* @zoombi32 0x0047e6d9 */
short wmxMixer::buildTable(long volume, unsigned char *table)
{
    long i;
    unsigned short changed;

    changed = 0;
    i = 0;
    do {
        switch (format.wBitsPerSample) {
        case 8: {
            long sample = fixedMul(i - 0x80, volume);
            if (sample > 0x7f)
                sample = 0x7f;
            else if (sample < -0x80)
                sample = -0x80;
            table[i] = (unsigned char)(sample + 0x80);
            break;
        }
        case 16: {
            long sample = fixedMul(i, volume);
            if (sample > 0x7f)
                sample = 0x7f;
            else if (sample < -0x80)
                sample = -0x80;
            switch (device->format.wBitsPerSample) {
            case 8:
                table[i] = (unsigned char)(sample + 0x80);
                break;
            case 16:
                table[i] = (unsigned char)sample;
                break;
            }
            break;
        }
        }
        changed |= (unsigned char)i != table[i];
    } while (++i < 0x100);
    return !changed;
}

/* Takes a played block off the queue and returns it to the owner. */
/* @zoombi32 0x0047e794 */
void wmxMixer::retire(WmxBlock *block)
{
    block->header->dwFlags &= ~WHDR_INQUEUE;
    block->header->dwFlags |= WHDR_DONE;
    if (!block->next)
        queueTail = block->prev;
    if (block->prev)
        block->prev->next = block->next;
    else if ((queue = block->next) != 0)
        queue->prev = 0;
    notify(WOM_DONE, (long)block->header, 0);
}

/* The block (from *block on) playing at `at`, and where the pass through it
   that `at` falls in starts. */
/* @zoombi32 0x0047e7ed */
void wmxMixer::findBlock(unsigned long at, WmxBlock **block, unsigned long *start)
{
    while (*block) {
        if ((long)(at - (*block)->start) >= 0 && (long)(at - (*block)->end) < 0) {
            if ((*block)->loops) {
                unsigned long pass = (at - (*block)->loopStart) / (*block)->loopLength;
                *start = (*block)->start + (*block)->loopLength * pass;
            } else
                *start = (*block)->start;
            if ((long)(at - *start) >= 0 && (long)(at - ((*block)->length + *start)) < 0)
                return;
        }
        *block = (*block)->next;
    }
}

/* Mixes (or with `first`, copies) `count` samples from `at` into `out`; 0
   if it has nothing to play there. */
/* Not exact: the original keeps `at` in edi and the offset and channel
   index on the stack; BCC32 4.5 puts those in edi and `at` on the stack. */
/* @zoombi32 0x0047e873 */
short __cdecl wmxMixer::mix(unsigned long at, void *out, unsigned long count, short first)
{
    WmxBlock *block;
    unsigned long start;
    unsigned long offset;
    unsigned char *in;
    long i;
    unsigned long n;

    if (paused || (block = queue) == 0)
        return 0;
    if ((long)(at - block->start) < 0) {
        n = block->start - at > count ? count : block->start - at;
        if (n == count)
            return 0;
        if (first)
            device->silence(out, n);
        out = (unsigned char *)out + device->format.wf.nBlockAlign * n;
        count -= n;
        at += n;
    }
    findBlock(at, &block, &start);
    if (!block)
        return 0;
    while (count) {
        offset = at - start;
        n = block->length - offset;
        in = (unsigned char *)fixedDiv(offset, step) + (long)block->header->lpData;
        if ((n = n < count ? n : count) != 0)
            for (i = 0; i < channels; i++) {
                WmxChannel *channel = &this->channel[i];
                if (first)
                    channel->copy((unsigned char *)out + channel->copyOffset, in + channel->inOffset,
                                  n, device->format.wf.nBlockAlign, format.wf.nBlockAlign, step,
                                  channel->volume, channel->identity, channel->table);
                else if (channel->volume)
                    channel->mix((unsigned char *)out + channel->mixOffset, in + channel->inOffset,
                                 n, device->format.wf.nBlockAlign, format.wf.nBlockAlign, step,
                                 channel->volume, channel->identity, channel->table);
            }
        out = (unsigned char *)out + device->format.wf.nBlockAlign * n;
        count -= n;
        at += n;
        if (at == block->length + start) {
            start += block->length;
            if (block->header->dwFlags & WHDR_ENDLOOP && (long)(at - block->end) < 0) {
                while (block && !(block->header->dwFlags & WHDR_BEGINLOOP))
                    block = block->prev;
            } else if ((block = block->next) == 0) {
                if (first)
                    device->silence(out, count);
                break;
            }
        }
    }
    return 1;
}

/* Works out the blocks' places in the mix again from `at` (after the rate
   changes). */
/* Not exact: the original keeps `loop` in edi and reads `at` from the
   stack; BCC32 4.5 gives edi to `at`. */
/* @zoombi32 0x0047ea78 */
void wmxMixer::retime(unsigned long at)
{
    WmxBlock *found;
    WmxBlock *&block = found;
    unsigned long start;
    long fraction;
    WmxBlock *loop;

    block = queue;
    findBlock(at, &block, &start);
    if (!block)
        return;
    fraction = fixedDiv(at - start, block->length);
    start = at - fixedMul(fixedMul(block->samples, fraction), step);
    if (block->loops && block->loopsDone)
        start -= fixedMul(block->loopSamples * block->loopsDone, step);
    while (block->prev) {
        block = block->prev;
        start -= fixedMul(block->samples, step);
    }
    if (block)
        do {
            block->length = fixedMul(block->samples, step);
            block->start = start;
            block->end = block->length + start;
            if (block->loops) {
                if (block->header->dwFlags & WHDR_BEGINLOOP) {
                    block->loopLength = fixedMul(block->loopSamples, step);
                    block->loopStart = block->start;
                    block->loops = block->header->dwLoops ? block->header->dwLoops - 1 : 0;
                    {
                        unsigned long most = 0x40000000 / block->loopLength;
                        if (block->loops > most - 1)
                            block->loops = most - 1;
                    }
                    block->loopTotal = block->loopLength * (block->loops + 1);
                    loop = block;
                }
                block->loopLength = loop->loopLength;
                block->loopStart = loop->loopStart;
                block->loopTotal = loop->loopTotal;
                block->end += block->loopLength * block->loops;
            }
            start = block->header->dwFlags & WHDR_ENDLOOP ? block->end : block->length + start;
            block = block->next;
        } while (block);
}

/* @zoombi32 0x0047ec2c */
void wmxMixer::chooseMixers()
{
    long i = 0;

    do {
        WmxChannel *channel = &this->channel[i];
        switch (device->format.wBitsPerSample) {
        case 16:
            switch (format.wBitsPerSample) {
            case 16:
                if (channel->volume != 0x10000)
                    goto byteSource;
                if (format.wf.nChannels == 1) {
                    channel->mix = mixWordIntoWord;
                    channel->copy = copyWordToWord;
                    channel->mixOffset = i * 2;
                    channel->copyOffset = i * 2;
                    channel->inOffset = 0;
                } else if (device->format.wf.nChannels == 1) {
                    channel->mix = mixWordIntoWord;
                    channel->copy = i == 0 ? copyWordToWord : mixWordIntoWord;
                    channel->mixOffset = 0;
                    channel->copyOffset = 0;
                    channel->inOffset = i * 2;
                } else {
                    channel->mix = mixWordIntoWord;
                    channel->copy = copyWordToWord;
                    channel->mixOffset = i * 2;
                    channel->copyOffset = i * 2;
                    channel->inOffset = i * 2;
                }
                break;
            case 8:
            byteSource:
                if (format.wf.nChannels == 1) {
                    channel->mix = mixByteIntoWord;
                    channel->copy = copyByteToWord;
                    channel->mixOffset = i * 2;
                    channel->copyOffset = i * 2;
                    channel->inOffset = 0;
                } else if (device->format.wf.nChannels == 1) {
                    channel->mix = mixByteIntoByte;
                    channel->copy = i == 0 ? copyByteToWord : mixByteIntoByte;
                    channel->mixOffset = i * 2 + 1;
                    channel->copyOffset = i * 3;
                    channel->inOffset = format.wBitsPerSample * i / 8;
                } else {
                    channel->mix = mixByteIntoByte;
                    channel->copy = copyByteToWord;
                    channel->mixOffset = i * 2 + 1;
                    channel->copyOffset = i * 2;
                    channel->inOffset = format.wBitsPerSample * i / 8;
                }
                break;
            }
            break;
        case 8:
            if (format.wf.nChannels == 1) {
                channel->mix = mixByteIntoByte;
                channel->copy = copyByteToByte;
                channel->mixOffset = i;
                channel->copyOffset = i;
                channel->inOffset = 0;
            } else if (device->format.wf.nChannels == 1) {
                channel->mix = mixByteIntoByte;
                channel->copy = i == 0 ? copyByteToByte : mixByteIntoByte;
                channel->mixOffset = 0;
                channel->copyOffset = 0;
                channel->inOffset = format.wBitsPerSample * i / 8;
            } else {
                channel->mix = mixByteIntoByte;
                channel->copy = copyByteToByte;
                channel->mixOffset = i;
                channel->copyOffset = i;
                channel->inOffset = format.wBitsPerSample * i / 8;
            }
            break;
        }
    } while (++i < channels);
}

/* The device has played to `at`: returns the blocks played out, counts
   loops, and notes the position. */
/* @zoombi32 0x0047ee5c */
void __cdecl wmxMixer::played(unsigned long at)
{
    WmxBlock *block;
    unsigned long start;

    if (paused)
        return;
    for (block = queue; block; block = block->next) {
        if ((long)(at - block->start) < 0)
            return;
        if ((long)(at - block->end) >= 0) {
            retire(block);
            donePosition += block->samples;
            position = donePosition;
            continue;
        }
        if (block->loops && (long)(at - block->start) >= 0) {
            unsigned long pass = (at - block->loopStart) / block->loopLength;
            short whole;
            start = block->loopLength * pass + block->start;
            whole = block->header->dwFlags & WHDR_BEGINLOOP && block->header->dwFlags & WHDR_ENDLOOP;
            if (whole || (long)(at - (start + block->length)) >= 0) {
                if (whole && pass > block->loopsDone || !whole && pass == block->loopsDone) {
                    block->loopsDone++;
                    if (!(flags & 0x40000000))
                        notify(0x8000, (long)block->header, 0);
                    donePosition += block->samples;
                    position = donePosition;
                }
                continue;
            }
        } else
            start = block->start;
        position = fixedDiv(at - start, step) + donePosition;
        return;
    }
}

/* Stops looping: the rest of the queue plays through once. */
/* @zoombi32 0x0047ef78 */
unsigned short __cdecl wmxMixer::breakLoop()
{
    unsigned long played;
    unsigned long written;
    WmxBlock *found;
    unsigned long start;
    WmxBlock *&block = found;
    short changed;

    device->buffer->lock();
    device->buffer->position(&played, &written);
    block = queue;
    findBlock(written, &block, &start);
    changed = 0;
    if (block->loops && block->loopsDone < block->loops)
        while (!(block->header->dwFlags & WHDR_BEGINLOOP) && block->prev) {
            block = block->prev;
            start -= block->length;
        }
    if (block)
        do {
            changed |= block->loops > 0;
            block->loops = 0;
            block->start = start;
            block->end = block->length + start;
            if ((long)(block->end - played) <= 0 && !(block->header->dwFlags & WHDR_DONE))
                retire(block);
            start += block->length;
            block = block->next;
        } while (block);
    if (changed)
        device->mix(written);
    device->buffer->unlock();
    return 0;
}

/* @zoombi32 0x0047f08e */
unsigned short __cdecl wmxMixer::close()
{
    if (queue)
        return WAVERR_STILLPLAYING;
    notify(WOM_CLOSE, 0, 0);
    return 0;
}

/* @zoombi32 0x0047f0b3 */
unsigned short __cdecl wmxMixer::getLevels(unsigned long *levels)
{
    *levels = this->levels;
    return 0;
}

/* @zoombi32 0x0047f0c5 */
unsigned short __cdecl wmxMixer::getID(unsigned short *id)
{
    *id = device->device;
    return 0;
}

/* @zoombi32 0x0047f0dc */
unsigned short __cdecl wmxMixer::getPitch(unsigned long *pitch)
{
    *pitch = 0x10000;
    return 0;
}

/* @zoombi32 0x0047f0ec */
unsigned short __cdecl wmxMixer::getPlaybackRate(unsigned long *rate)
{
    *rate = this->rate;
    return 0;
}

/* @zoombi32 0x0047f0fe */
unsigned short __cdecl wmxMixer::getPosition(MMTIME *time, unsigned short size)
{
    if (size != sizeof(MMTIME))
        return MMSYSERR_INVALPARAM;
    switch (time->wType) {
    case TIME_BYTES:
        time->u.cb = format.wf.nBlockAlign * position;
        break;
    case TIME_MS:
        time->u.ms = fixedMul(position, fixedDiv(1000, format.wf.nSamplesPerSec));
        break;
    case TIME_SAMPLES:
        time->u.sample = position;
        break;
    default:
        return MMSYSERR_INVALPARAM;
    }
    return 0;
}

/* @zoombi32 0x0047f15e */
unsigned short __cdecl wmxMixer::getVolume(unsigned long *volume)
{
    *volume = this->volume;
    return 0;
}

/* @zoombi32 0x0047f170 */
unsigned short __cdecl wmxMixer::pause()
{
    unsigned long played;
    unsigned long written;

    if (paused)
        return 0;
    paused = 1;
    if (queue) {
        device->buffer->lock();
        device->buffer->position(&played, &written);
        if ((long)(written - queue->start) > 0)
            pauseOffset = written - queue->start;
        else
            pauseOffset = 0;
        device->mix(queue->start);
        device->buffer->unlock();
    } else
        pauseOffset = 0;
    return 0;
}

/* @zoombi32 0x0047f210 */
unsigned short __cdecl wmxMixer::prepareHeader(WAVEHDR *header, unsigned short size)
{
    WmxBlock *block;

    if (!header || size != sizeof(WAVEHDR))
        return MMSYSERR_INVALPARAM;
    if (header->dwFlags & WHDR_PREPARED)
        return MMSYSERR_ERROR;
    if ((block = (WmxBlock *)newPtr(sizeof(WmxBlock))) == 0)
        return MMSYSERR_NOMEM;
    memset(block, 0, sizeof(WmxBlock));
    block->header = header;
    header->lpNext = (WAVEHDR *)block;
    lockPtr(block);
    osLockMemory(header, sizeof(WAVEHDR));
    osLockMemory(header->lpData, header->dwBufferLength);
    header->reserved = 0x574d6978;
    header->dwFlags |= WHDR_PREPARED;
    return 0;
}

/* @zoombi32 0x0047f283 */
unsigned short __cdecl wmxMixer::reset()
{
    unsigned long start;
    unsigned long played;
    unsigned long written;

    if (queue) {
        start = queue->start;
        do
            retire(queue);
        while (queue);
        if ((long)(device->written - start) > 0) {
            device->buffer->lock();
            device->buffer->position(&played, &written);
            device->mix(written);
            device->buffer->unlock();
        }
    }
    donePosition = position = 0;
    paused = 0;
    return 0;
}

/* @zoombi32 0x0047f316 */
unsigned short __cdecl wmxMixer::restart()
{
    unsigned long played;
    unsigned long written;

    if (paused) {
        device->buffer->lock();
        device->buffer->position(&played, &written);
        paused = 0;
        if (queue) {
            WmxBlock *block = queue;
            long shift = written - (block->start + pauseOffset);
            do {
                block->start += shift;
                block->end += shift;
                block->loopStart += shift;
            } while ((block = block->next) != 0);
            device->mix(written);
        }
        device->buffer->unlock();
        device->buffer->start();
    }
    return 0;
}

/* @zoombi32 0x0047f3bb */
unsigned short __cdecl wmxMixer::setLevels(unsigned long levels)
{
    if (levels != this->levels) {
        long current;
        this->levels = levels;
        current = volume;
        volume = 0;
        setVolume(current);
    }
    return 0;
}

/* @zoombi32 0x0047f3e2 */
unsigned short __cdecl wmxMixer::setPitch(unsigned long)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047f3eb */
unsigned short __cdecl wmxMixer::setPlaybackRate(unsigned long rate)
{
    unsigned long played;
    unsigned long written;

    if ((long)rate < 0)
        return MMSYSERR_INVALPARAM;
    if (rate != this->rate) {
        this->rate = rate;
        step = fixedDiv(device->format.wf.nSamplesPerSec,
                        fixedMul(format.wf.nSamplesPerSec, this->rate));
        chooseMixers();
        if (queue) {
            device->buffer->lock();
            device->buffer->position(&played, &written);
            retime(written);
            device->mix(written);
            device->buffer->unlock();
        }
    }
    return 0;
}

/* @zoombi32 0x0047f48e */
unsigned short __cdecl wmxMixer::setVolume(unsigned long volume)
{
    unsigned long played;
    unsigned long written;

    if ((long)volume < 0)
        return MMSYSERR_INVALPARAM;
    if (volume != this->volume) {
        this->volume = volume;
        if (channels == 1) {
            channel[0].volume = this->volume;
            channel[0].identity = buildTable(channel[0].volume, channel[0].table);
        } else {
            channel[0].volume = fixedMul(this->volume, fixedDiv(lowWord(levels), 0xffff));
            channel[0].identity = buildTable(channel[0].volume, channel[0].table);
            channel[1].volume = fixedMul(this->volume, fixedDiv(highWord(levels), 0xffff));
            channel[1].identity = buildTable(channel[1].volume, channel[1].table);
        }
        chooseMixers();
        if (queue) {
            device->buffer->lock();
            device->buffer->position(&played, &written);
            device->mix(written);
            device->buffer->unlock();
        }
    }
    return 0;
}

/* @zoombi32 0x0047f5ac */
unsigned short __cdecl wmxMixer::unprepareHeader(WAVEHDR *header, unsigned short size)
{
    WmxBlock *block;

    if (!header || size != sizeof(WAVEHDR) || header->reserved != 0x574d6978)
        return MMSYSERR_INVALPARAM;
    if (!(header->dwFlags & WHDR_PREPARED))
        return WAVERR_UNPREPARED;
    if (header->dwFlags & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    block = (WmxBlock *)header->lpNext;
    osUnlockMemory(header->lpData, header->dwBufferLength);
    osUnlockMemory(header, sizeof(WAVEHDR));
    unlockPtr(block);
    disposePtr(block);
    header->dwFlags &= ~WHDR_PREPARED;
    return 0;
}

/* Queues a block: at the end of the queue, with its place in the mix and
   its loop worked out. */
/* @zoombi32 0x0047f611 */
unsigned short __cdecl wmxMixer::write(WAVEHDR *header, unsigned short size)
{
    WmxBlock *block;
    WmxBlock *loop;
    unsigned long played;
    unsigned long written;

    if (!header || size != sizeof(WAVEHDR) || header->reserved != 0x574d6978)
        return MMSYSERR_INVALPARAM;
    if (!(header->dwFlags & WHDR_PREPARED))
        return WAVERR_UNPREPARED;
    if (header->dwFlags & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    block = (WmxBlock *)header->lpNext;
    block->samples = header->dwBufferLength / format.wf.nBlockAlign;
    block->length = fixedMul(block->samples, step);
    device->buffer->lock();
    device->buffer->position(&played, &written);
    if (header->dwFlags & WHDR_ENDLOOP) {
        block->loopSamples = block->samples;
        block->loopLength = block->length;
        if (header->dwFlags & WHDR_BEGINLOOP) {
            loop = block;
            block->loopStart = queueTail ? queueTail->end : written;
        } else {
            /* The original returns here with the buffer still locked. */
            for (loop = queueTail;; loop = loop->prev) {
                if (!loop || loop->header->dwFlags & WHDR_ENDLOOP)
                    return MMSYSERR_INVALPARAM;
                block->loopSamples += loop->samples;
                block->loopLength += loop->length;
                if (loop->header->dwFlags & WHDR_BEGINLOOP)
                    break;
            }
            block->loopStart = loop->loopStart;
        }
        block->loops = loop->header->dwLoops ? loop->header->dwLoops - 1 : 0;
        {
            unsigned long most = 0x40000000 / block->loopLength;
            if (block->loops > most - 1)
                block->loops = most - 1;
        }
        if (block->loops > 0) {
            block->loopsDone = 0;
            block->loopTotal = (block->loops + 1) * block->loopLength;
            if (loop != block && loop)
                do {
                    loop->loops = block->loops;
                    loop->loopsDone = 0;
                    loop->loopSamples = block->loopSamples;
                    loop->loopLength = block->loopLength;
                    loop->loopStart = block->loopStart;
                    loop->loopTotal = block->loopTotal;
                    loop->end += loop->loopLength * loop->loops;
                } while ((loop = loop->next) != 0);
        }
    } else
        block->loops = 0;
    header->dwFlags |= WHDR_INQUEUE;
    header->dwFlags &= ~WHDR_DONE;
    if (queueTail) {
        block->next = 0;
        block->prev = queueTail;
        queueTail->next = block;
        queueTail = block;
        block->start = block->loops && !(block->header->dwFlags & WHDR_BEGINLOOP)
            ? block->prev->start + block->prev->length
            : block->prev->end;
    } else {
        block->next = 0;
        block->prev = 0;
        queueTail = block;
        queue = block;
        block->start = written;
    }
    if (block->loops)
        block->end = block->loopStart + block->loopTotal;
    else
        block->end = block->start + block->length;
    if (paused)
        device->buffer->unlock();
    else {
        device->mix(block->start);
        device->buffer->unlock();
        device->buffer->start();
    }
    return 0;
}

/* @zoombi32 0x0047f8b8 */
__cdecl wmxObject::wmxObject(wmxDevice *device, PCMWAVEFORMAT *format, long callback,
                             long instance, unsigned long flags)
{
    tag = 0x574d6978;
    this->format = *format;
    this->callback = callback;
    this->instance = instance;
    this->flags = flags;
    if ((next = wmx.objects) != 0) {
        prev = next->prev;
        prev->next = this;
        next->prev = this;
    } else
        next = prev = this;
    if ((this->device = device) != 0) {
        if ((deviceNext = device->objects) != 0) {
            devicePrev = deviceNext->devicePrev;
            devicePrev->deviceNext = this;
            deviceNext->devicePrev = this;
        } else
            deviceNext = devicePrev = this;
        device->objects = this;
        device->objectCount++;
    }
}

/* @zoombi32 0x0047f954 */
__cdecl wmxObject::~wmxObject()
{
    tag = 0;
    if (this == next)
        wmx.objects = 0;
    else {
        next->prev = prev;
        prev->next = next;
        if (this == wmx.objects)
            wmx.objects = next;
    }
    if (device) {
        if (--device->objectCount == 0)
            delete device;
        else {
            deviceNext->devicePrev = devicePrev;
            devicePrev->deviceNext = deviceNext;
            if (this == device->objects)
                device->objects = deviceNext;
        }
    }
}

/* Calls the owner's callback (a function: messages are words unless bit
   30 of the flags is set). */
/* @zoombi32 0x0047f9f6 */
void wmxObject::notify(unsigned short message, long param1, long param2)
{
    switch (flags & CALLBACK_TYPEMASK) {
    case CALLBACK_FUNCTION:
        if (flags & 0x40000000)
            ((WmxCallback)callback)((long)this, message, instance, param1, param2);
        else
            ((WmxShortCallback)callback)((long)this, message, instance, param1, param2);
        break;
    }
}

/* @zoombi32 0x0047fa3d */
short __cdecl wmxObject::mix(unsigned long, void *, unsigned long, short)
{
    return 0;
}

/* @zoombi32 0x0047fa44 */
void __cdecl wmxObject::played(unsigned long)
{
}

/* @zoombi32 0x0047fa49 */
unsigned short __cdecl wmxObject::breakLoop()
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa52 */
unsigned short __cdecl wmxObject::close()
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa5b */
unsigned short __cdecl wmxObject::getLevels(unsigned long *)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa64 */
unsigned short __cdecl wmxObject::getID(unsigned short *)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa6d */
unsigned short __cdecl wmxObject::getPitch(unsigned long *)
{
    return 0;
}

/* @zoombi32 0x0047fa74 */
unsigned short __cdecl wmxObject::getPlaybackRate(unsigned long *)
{
    return 0;
}

/* @zoombi32 0x0047fa7b */
unsigned short __cdecl wmxObject::getPosition(MMTIME *, unsigned short)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa84 */
unsigned short __cdecl wmxObject::getVolume(unsigned long *)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa8d */
unsigned short __cdecl wmxObject::pause()
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa96 */
unsigned short __cdecl wmxObject::prepareHeader(WAVEHDR *, unsigned short)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fa9f */
unsigned short __cdecl wmxObject::reset()
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047faa8 */
unsigned short __cdecl wmxObject::restart()
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fab1 */
unsigned short __cdecl wmxObject::setLevels(unsigned long)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047faba */
unsigned short __cdecl wmxObject::setPitch(unsigned long)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fac3 */
unsigned short __cdecl wmxObject::setPlaybackRate(unsigned long)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047facc */
unsigned short __cdecl wmxObject::setVolume(unsigned long)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fad5 */
unsigned short __cdecl wmxObject::unprepareHeader(WAVEHDR *, unsigned short)
{
    return MMSYSERR_NOTSUPPORTED;
}

/* @zoombi32 0x0047fade */
unsigned short __cdecl wmxObject::write(WAVEHDR *, unsigned short)
{
    return MMSYSERR_NOTSUPPORTED;
}
