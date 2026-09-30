/*
 * wavebuf (Mohawk engine): WaveMix's output buffers (through waveOut or
 * DirectSound), and its waveOut-like API over WaveMix objects
 */

/* @flags -p -x- */

#include <stdio.h>

#include "zoombinis.h"
#include "os_localmem.h"
#include "os_manager.h"
#include "os_refcount.h"
#include "os_threads.h"

/* [WaveMix]'s strings, stored apart from the pooled literals. */
static char waveMixSection[] = "WaveMix";
static char frameRateKey[] = "ulFrameRate";
static char frameSizeKey[] = "ulFrameSize";
static char stereoKey[] = "fStereo";
static char enableKey[] = "fEnable";

short useDirectSound = 0;
short wavebufCache = 0;
HINSTANCE directSoundLibrary = 0;

/* @zoombi32 0x0047af1c */
__cdecl wavebuf::~wavebuf()
{
}

/* An output buffer for the wave device `device`: DirectSound's when it's
   loaded and knows the device, else waveOut's (if the device isn't "not
   supported"). */
/* @zoombi32 0x0047af3b */
short __cdecl newWavebuf(unsigned short device, wavebuf **buffer)
{
    wavebufDS *ds;

    *buffer = 0;
    if (waveOutGetNumDevs() <= device)
        return MMSYSERR_BADDEVICEID;
    if (useDirectSound && (ds = new wavebufDS(device)) != 0) {
        if (ds->available) {
            *buffer = ds;
            return 0;
        }
        delete ds;
    }
    {
        wavebufWO *wo = new wavebufWO(device);
        if (!wo)
            return MMSYSERR_NOMEM;
        if (!wo->supported) {
            delete wo;
            return MMSYSERR_NOTSUPPORTED;
        }
        *buffer = wo;
    }
    return 0;
}

/* @zoombi32 0x0047b000 */
short __cdecl initWavebuf()
{
    forgetWavebufCache();
    return useDirectSound = loadDirectSound();
}

/* @zoombi32 0x0047b015 */
void __cdecl closeWavebuf()
{
    if (useDirectSound)
        freeDirectSound();
    freeWavebufCache();
}

/* @zoombi32 0x0047b02e */
void *__cdecl wavebufDS::operator new(size_t size)
{
    return localAlloc(size);
}

/* @zoombi32 0x0047b03d */
void *__cdecl wavebufWO::operator new(size_t size)
{
    return localAlloc(size);
}

/* Reads the device's settings from [WaveMix.DeviceInfo]: under its name
   and driver version, else "default". */
/* @zoombi32 0x0047b04c */
__cdecl wavebufWO::wavebufWO(unsigned short device)
{
    char entry[0x100];
    char value[0x40];
    short found;

    isOpen = 0;
    this->device = device;
    waveOutGetDevCaps(device, &caps, sizeof caps);
    strcpy(name, caps.szPname);
    memset(&call, 0, sizeof call);
    call.proc = wavebufWONotify;
    call.data = this;
    initLock(&lockState, 0);
    supported = 1;
    blockCount = 0x10;
    blockSamples = 0x400;
    ahead = 0x1000;
    prime = 0;
    found = 0;
    if (!findIniEntry(0, "WaveMix.DeviceInfo", entry, sizeof entry, caps.szPname,
                      caps.vDriverVersion, caps.wMid, caps.wPid)) {
        getIniString(0, "WaveMix.DeviceInfo", entry, value, sizeof value);
        if (!stricmp(value, "not supported")) {
            supported = 0;
            found = 1;
        } else
            found = readDeviceInfo(entry);
    }
    if (!found)
        readDeviceInfo("default");
}

/* @zoombi32 0x0047b18e */
__cdecl wavebufWO::~wavebufWO()
{
    if (isOpen)
        close();
    removeLock(&lockState);
}

/* @zoombi32 0x0047b1d5 */
void wavebufWONotify(void *data)
{
    wavebufWO *wo = (wavebufWO *)data;

    if (wo->notify)
        wo->notify(wo->data, wo->played, wo->written);
}

/* @zoombi32 0x0047b200 */
short __cdecl wavebufWO::close()
{
    unsigned long i;

    if (!isOpen)
        return MMSYSERR_NODRIVER;
    waveOutReset(wave);
    for (i = 0; i < blockCount; i++)
        waveOutUnprepareHeader(wave, &headers[i], sizeof(WAVEHDR));
    waveOutClose(wave);
    disposePtr(headers);
    if (wavebufCache && handleData(wavebufCache) == buffer)
        unlockHandle(wavebufCache);
    else
        disposePtr(buffer);
    deleteSync(thread);
    isOpen = 0;
    return 0;
}

/* Reads "blocks, block samples, samples ahead[, samples to prime]" from
   [WaveMix.DeviceInfo]; 0 if it's not there. */
/* @zoombi32 0x0047b2b3 */
short __cdecl wavebufWO::readDeviceInfo(const char *key)
{
    long blocks;
    long samples;
    long ahead;
    long prime;
    struct
    {
        char value[0x42];
        char separator[0xee];
    } text;

    if (getIniString(0, "WaveMix.DeviceInfo", key, text.value, 0x40) && iniError() != 0x296c)
        return 0;
    {
        int count = sscanf(text.value, "%ld %[,;:] %ld %[,;:] %ld %[,;:] %ld", &blocks, text.separator,
                           &samples, text.separator, &ahead, text.separator, &prime);
        if (count < 5)
            return 0;
        blockCount = blocks;
        blockSamples = samples;
        this->ahead = ahead;
        if (count >= 7)
            this->prime = prime;
    }
    return 1;
}

/* @zoombi32 0x0047b360 */
void CALLBACK wavebufWOCallback(HWAVEOUT, UINT message, DWORD instance, DWORD header, DWORD)
{
    if (message == WOM_DONE) {
        wavebufWO *wo = (wavebufWO *)instance;
        WAVEHDR *done = (WAVEHDR *)header;
        if (wo->done < done->dwBytesRecorded)
            wo->done = done->dwBytesRecorded;
    }
}

/* @zoombi32 0x0047b38a */
void __cdecl wavebufWO::lock()
{
    enterLock(&lockState);
}

/* @zoombi32 0x0047b39c */
void __cdecl wavebufWO::unlock()
{
    leaveLock(&lockState);
}

/* The device's position, in samples. */
/* @zoombi32 0x0047b3ae */
unsigned long __cdecl wavebufWO::devicePosition()
{
    MMTIME time;

    time.wType = TIME_SAMPLES;
    waveOutGetPosition(wave, &time, sizeof time);
    switch (time.wType) {
    case TIME_BYTES:
        return time.u.cb / format.wf.nBlockAlign;
    case TIME_MS:
        return time.u.ms * format.wf.nSamplesPerSec / 1000;
    case TIME_SAMPLES:
        return time.u.sample;
    default:
        return 0;
    }
}

/* @zoombi32 0x0047b412 */
short __cdecl wavebufWO::position(unsigned long *played, unsigned long *written)
{
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    if (started) {
        unsigned long at = devicePosition();
        if (at > this->played)
            this->played = at;
        fill(0);
    }
    *played = this->played;
    *written = this->written;
    return 0;
}

/* @zoombi32 0x0047b46f */
void __fastcall forgetWavebufCache()
{
    wavebufCache = 0;
}

/* @zoombi32 0x0047b479 */
short __cdecl wavebufWO::lockBuffer(unsigned long at, void **first, unsigned long *firstLength,
                                    void **second, unsigned long *secondLength)
{
    unsigned long *length;
    unsigned long block;
    unsigned long offset;

    *first = *second = 0;
    *firstLength = *secondLength = 0;
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    if (locked > 0 || at < written || at >= written + totalSamples)
        return MMSYSERR_ERROR;
    block = at % totalSamples / blockSamples;
    offset = at % blockSamples;
    *first = headers[block].lpData + format.wf.nBlockAlign * offset;
    length = firstLength;
    while (headers[block].dwFlags & WHDR_DONE && locked < blockCount) {
        *length += blockSamples - offset;
        offset = 0;
        if (++block >= blockCount) {
            block = 0;
            length = secondLength;
            *second = headers->lpData;
        }
        locked++;
    }
    return locked > 0 ? 0 : WAVERR_STILLPLAYING;
}

/* @zoombi32 0x0047b580 */
short __cdecl wavebufWO::open(PCMWAVEFORMAT *format, WavebufNotify notify, long data)
{
    unsigned long i;
    short error;
    unsigned long size;
    WAVEHDR *header;

    if (isOpen)
        return MMSYSERR_ERROR;
    if ((error = waveOutOpen(&wave, device, (WAVEFORMAT *)format, (DWORD)wavebufWOCallback,
                             (DWORD)this, CALLBACK_FUNCTION)) != 0)
        return error;
    if ((thread = createThread(wavebufWOThread, (long)this, 0x1000, 1)) == 0) {
        error = MMSYSERR_ERROR;
    closeWave:
        waveOutClose(wave);
        return error;
    }
    headers = (WAVEHDR *)newPtr(blockCount * sizeof(WAVEHDR));
    if (!headers) {
        error = MMSYSERR_NOMEM;
    killThread:
        deleteSync(thread);
        goto closeWave;
    }
    totalSamples = blockCount * blockSamples;
    size = format->wf.nBlockAlign * totalSamples;
    if (!wavebufCache && (wavebufCache = newHandle(size)) != 0)
        setPurgeable(wavebufCache, 1);
    if (wavebufCache && !handleLocks(wavebufCache) && !setHandleSize(wavebufCache, size))
        buffer = (unsigned char *)lockHandleAlias(wavebufCache);
    else if ((buffer = (unsigned char *)newPtr(size)) == 0) {
        error = MMSYSERR_NOMEM;
        disposePtr(headers);
        goto killThread;
    }
    for (i = 0; i < blockCount; i++) {
        header = &headers[i];
        memset(header, 0, sizeof *header);
        header->lpData = (LPSTR)(format->wf.nBlockAlign * blockSamples * i + buffer);
        header->dwBufferLength = format->wf.nBlockAlign * blockSamples;
        waveOutPrepareHeader(wave, header, sizeof *header);
        header->dwFlags |= WHDR_DONE;
    }
    fillMemory(buffer, format->wBitsPerSample == 8 ? 0x80 : 0, size);
    isOpen = 1;
    started = 0;
    this->format = *format;
    this->notify = notify;
    this->data = data;
    locked = 0;
    done = 0;
    played = 0;
    written = 0;
    return 0;
}

/* @zoombi32 0x0047b7a9 */
void __cdecl wavebufWO::formats(unsigned long *rate, unsigned long *formats)
{
    *rate = 11025;
    *formats = caps.dwFormats;
}

/* @zoombi32 0x0047b7c2 */
void __cdecl freeWavebufCache()
{
    if (wavebufCache) {
        disposeHandle(wavebufCache);
        wavebufCache = 0;
    }
}

/* @zoombi32 0x0047b7e6 */
short __cdecl wavebufWO::start()
{
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    if (!started) {
        waveOutPause(wave);
        started = 1;
        resumeThread(thread);
        deferCall(&lockState, &call);
        fill(prime);
        waveOutRestart(wave);
    }
    return 0;
}

/* Keeps the ring written ahead of the device. */
/* @zoombi32 0x0047b856 */
void wavebufWOThread(long data)
{
    wavebufWO *wo = (wavebufWO *)data;

    for (;;) {
        unsigned long at = wo->devicePosition();
        if (at > wo->played) {
            wo->played = at;
            wo->fill(0);
        }
        yieldThread(0);
    }
}

/* @zoombi32 0x0047b88b */
short __cdecl wavebufWO::unlockBuffer()
{
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    if (!locked)
        return MMSYSERR_ERROR;
    locked = 0;
    return 0;
}

/* Queues free blocks until `ahead` samples are written past the played
   position (and at least `minimum` more); tells the owner if it queued
   any. */
/* @zoombi32 0x0047b8bc */
short __cdecl wavebufWO::fill(unsigned long minimum)
{
    unsigned long from = written;

    while (written - played < ahead || minimum > written - from) {
        WAVEHDR *header = &headers[written % totalSamples / blockSamples];
        if (!(header->dwFlags & WHDR_DONE))
            break;
        header->dwFlags &= ~WHDR_DONE;
        header->dwBytesRecorded = written + blockSamples;
        header->dwUser = (DWORD)this;
        waveOutWrite(wave, header, sizeof *header);
        written += blockSamples;
    }
    if (from != written) {
        deferCall(&lockState, &call);
        return 1;
    }
    return 0;
}

/* @zoombi32 0x0047b96c */
void __cdecl wavebufWO::operator delete(void *block)
{
    localFree(block);
}

/* Finds the DirectSound device with the wave device's number. */
/* @zoombi32 0x0047b9d0 */
BOOL CALLBACK enumerateDirectSound(GUID *guid, const char *description, const char *, void *context)
{
    if (!guid)
        return 1;
    wavebufDS *ds = (wavebufDS *)context;
    if (ds->enumerated == ds->device) {
        ds->available = 1;
        memcpy(&ds->guid, guid, sizeof(GUID));
        strncpy(ds->name, description, sizeof ds->name);
        ds->name[sizeof ds->name - 1] = 0;
        return 0;
    }
    ds->enumerated++;
    return 1;
}

/* @zoombi32 0x0047ba2b */
__cdecl wavebufDS::wavebufDS(unsigned short device)
{
    available = 0;
    enumerated = 0;
    this->device = device;
    directSoundEnumerate(enumerateDirectSound, this);
    if (available) {
        if (directSoundCreate(&guid, &directSound, 0))
            available = 0;
        else {
            memset(&caps, 0, sizeof caps);
            caps.dwSize = sizeof caps;
            if (directSound->GetCaps(&caps) || caps.dwMaxSecondarySampleRate < 11025
                || caps.dwMinSecondarySampleRate > 44100) {
                directSound->Release();
                available = 0;
            } else {
                directSound->SetCooperativeLevel(appWindowHandle(), DSSCL_NORMAL);
                InitializeCriticalSection(&section);
                memset(&call, 0, sizeof call);
                call.proc = wavebufDSNotify;
                call.data = this;
                initLock(&lockState, 0);
            }
        }
    }
    isOpen = 0;
}

/* @zoombi32 0x0047bb2f */
__cdecl wavebufDS::~wavebufDS()
{
    if (available) {
        directSound->Release();
        DeleteCriticalSection(&section);
        removeLock(&lockState);
    }
}

/* @zoombi32 0x0047bb80 */
void wavebufDSNotify(void *data)
{
    wavebufDS *ds = (wavebufDS *)data;

    if (ds->notify)
        ds->notify(ds->data, ds->played, ds->written);
}

/* @zoombi32 0x0047bbab */
short __cdecl wavebufDS::close()
{
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    TerminateThread(thread, 0);
    CloseHandle(thread);
    buffer->Release();
    isOpen = 0;
    return 0;
}

/* @zoombi32 0x0047bbfa */
void __cdecl wavebufDS::lock()
{
    EnterCriticalSection(&section);
    enterLock(&lockState);
}

/* @zoombi32 0x0047bc17 */
void __cdecl wavebufDS::unlock()
{
    leaveLock(&lockState);
    LeaveCriticalSection(&section);
}

/* @zoombi32 0x0047bc34 */
short __cdecl wavebufDS::position(unsigned long *played, unsigned long *written)
{
    DWORD play;
    DWORD write;

    if (!isOpen)
        return MMSYSERR_NODRIVER;
    EnterCriticalSection(&section);
    if (started) {
        buffer->GetCurrentPosition(&play, &write);
        if (play != playCursor) {
            if (play < playCursor)
                playWraps++;
            playCursor = play;
            this->played = playCursor / format.nBlockAlign + bufferSamples * playWraps;
        }
        if (write != writeCursor) {
            if (write < writeCursor)
                writeWraps++;
            writeCursor = write;
            this->written = writeCursor / format.nBlockAlign + bufferSamples * writeWraps;
        }
    }
    *played = this->played;
    *written = this->written;
    LeaveCriticalSection(&section);
    return 0;
}

/* Loads DSOUND.DLL if [WaveMix] fEnableDirectSound is set. */
/* @zoombi32 0x0047bd45 */
short __cdecl loadDirectSound()
{
    short enable = 0;

    getIniBool(0, waveMixSection, "fEnableDirectSound", &enable);
    if (enable && (directSoundLibrary = LoadLibrary("DSOUND.DLL")) != 0) {
        directSoundCreate = (long(WINAPI *)(GUID *, IDirectSound **, void *))GetProcAddress(
            directSoundLibrary, "DirectSoundCreate");
        directSoundEnumerate = (long(WINAPI *)(DSENUMCALLBACK, void *))GetProcAddress(
            directSoundLibrary, "DirectSoundEnumerateA");
    }
    return directSoundLibrary != 0;
}

/* Not exact: BCC32 4.5 computes the byte count into esi (imul esi, ecx);
   the original multiplies in ecx and copies it. */
/* @zoombi32 0x0047bdb8 */
short __cdecl wavebufDS::lockBuffer(unsigned long at, void **first, unsigned long *firstLength,
                                    void **second, unsigned long *secondLength)
{
    unsigned long offset;

    *first = *second = 0;
    *firstLength = *secondLength = 0;
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    EnterCriticalSection(&section);
    {
        unsigned long free = bufferSamples - (written - played);
        if (locked || at < written || at >= written + free) {
            LeaveCriticalSection(&section);
            return MMSYSERR_ERROR;
        }
        offset = at % bufferSamples * format.nBlockAlign;
        at -= written;
        free -= at;
        at = free *= format.nBlockAlign;
    }
    if (buffer->Lock(offset, at, first, firstLength, second, secondLength, 0)) {
        LeaveCriticalSection(&section);
        return MMSYSERR_ERROR;
    }
    lockFirst = *first;
    lockFirstBytes = *firstLength;
    lockSecond = *second;
    lockSecondBytes = *secondLength;
    *firstLength /= format.nBlockAlign;
    *secondLength /= format.nBlockAlign;
    locked = 1;
    LeaveCriticalSection(&section);
    return 0;
}

/* @zoombi32 0x0047bf02 */
short __cdecl wavebufDS::open(PCMWAVEFORMAT *format, WavebufNotify notify, long data)
{
    DWORD id;
    DSBUFFERDESC desc;

    bufferSamples = format->wf.nSamplesPerSec;
    memset(&this->format, 0, sizeof this->format);
    memcpy(&this->format, format, sizeof *format);
    memset(&desc, 0, sizeof desc);
    desc.dwSize = sizeof desc;
    desc.dwFlags = 8;
    desc.dwBufferBytes = format->wf.nBlockAlign * bufferSamples;
    desc.lpwfxFormat = &this->format;
    switch (directSound->CreateSoundBuffer(&desc, &buffer, 0)) {
    case DSERR_ALLOCATED:
        return MMSYSERR_ALLOCATED;
    case DSERR_OUTOFMEMORY:
        return MMSYSERR_NOMEM;
    case DSERR_BADFORMAT:
        return WAVERR_BADFORMAT;
    default:
        return MMSYSERR_ERROR;
    case 0:
        break;
    }
    thread = CreateThread(0, 0, wavebufDSThread, this, CREATE_SUSPENDED, &id);
    if (thread)
        SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
    else {
        buffer->Release();
        return MMSYSERR_ERROR;
    }
    buffer->GetCurrentPosition(&playCursor, &writeCursor);
    played = playCursor / this->format.nBlockAlign;
    playWraps = 0;
    written = writeCursor / this->format.nBlockAlign;
    writeWraps = 0;
    this->notify = notify;
    this->data = data;
    locked = 0;
    started = 0;
    isOpen = 1;
    return 0;
}

/* The rates and formats the device can play (the WAVE_FORMAT_ bits). */
/* @zoombi32 0x0047c0b1 */
void __cdecl wavebufDS::formats(unsigned long *rate, unsigned long *formats)
{
    unsigned long at11 = 0;
    unsigned long at22 = 0;
    unsigned long at44 = 0;

    if (caps.dwFlags & DSCAPS_SECONDARY8BIT && caps.dwFlags & DSCAPS_SECONDARYMONO) {
        at11 |= WAVE_FORMAT_1M08;
        at22 |= WAVE_FORMAT_2M08;
        at44 |= WAVE_FORMAT_4M08;
    }
    if (caps.dwFlags & DSCAPS_SECONDARY8BIT && caps.dwFlags & DSCAPS_SECONDARYSTEREO) {
        at11 |= WAVE_FORMAT_1S08;
        at22 |= WAVE_FORMAT_2S08;
        at44 |= WAVE_FORMAT_4S08;
    }
    if (caps.dwFlags & DSCAPS_SECONDARY16BIT && caps.dwFlags & DSCAPS_SECONDARYMONO) {
        at11 |= WAVE_FORMAT_1M16;
        at22 |= WAVE_FORMAT_2M16;
        at44 |= WAVE_FORMAT_4M16;
    }
    if (caps.dwFlags & DSCAPS_SECONDARY16BIT && caps.dwFlags & DSCAPS_SECONDARYSTEREO) {
        at11 |= WAVE_FORMAT_1S16;
        at22 |= WAVE_FORMAT_2S16;
        at44 |= WAVE_FORMAT_4S16;
    }
    *rate = 11025;
    *formats = 0;
    if (caps.dwMinSecondarySampleRate <= 11025 && caps.dwMaxSecondarySampleRate >= 11025)
        *formats |= at11;
    if (caps.dwMinSecondarySampleRate <= 22050 && caps.dwMaxSecondarySampleRate >= 22050)
        *formats |= at22;
    if (caps.dwMinSecondarySampleRate <= 44100 && caps.dwMaxSecondarySampleRate >= 44100)
        *formats |= at44;
}

/* @zoombi32 0x0047c1a7 */
void __cdecl freeDirectSound()
{
    if (directSoundLibrary) {
        FreeLibrary(directSoundLibrary);
        directSoundLibrary = 0;
    }
}

/* @zoombi32 0x0047c1c7 */
short __cdecl wavebufDS::start()
{
    if (!isOpen)
        return MMSYSERR_NODRIVER;
    if (!started) {
        started = 1;
        deferCall(&lockState, &call);
        buffer->Play(0, 0, DSBPLAY_LOOPING);
        ResumeThread(thread);
    }
    return 0;
}

/* Every 100 ms, tells the owner if the buffer has moved on. */
/* @zoombi32 0x0047c229 */
DWORD WINAPI wavebufDSThread(void *data)
{
    wavebufDS *ds = (wavebufDS *)data;
    unsigned long played;
    unsigned long written;

    for (;;) {
        Sleep(100);
        EnterCriticalSection(&ds->section);
        played = ds->played;
        written = ds->written;
        ds->position(&ds->played, &ds->written);
        if (played < ds->played || written < ds->written)
            deferCall(&ds->lockState, &ds->call);
        LeaveCriticalSection(&ds->section);
    }
}

/* @zoombi32 0x0047c297 */
short __cdecl wavebufDS::unlockBuffer()
{
    short error;

    if (!isOpen)
        return MMSYSERR_NODRIVER;
    EnterCriticalSection(&section);
    if (!locked || buffer->Unlock(lockFirst, lockFirstBytes, lockSecond, lockSecondBytes))
        error = MMSYSERR_ERROR;
    else {
        locked = 0;
        error = 0;
    }
    LeaveCriticalSection(&section);
    return error;
}

/* @zoombi32 0x0047c353 */
void __cdecl wavebufDS::operator delete(void *block)
{
    localFree(block);
}

/* @zoombi32 0x0047c3b4 */
unsigned short wavebufBreakLoop(long handle)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->breakLoop();
}

/* @zoombi32 0x0047c3d4 */
unsigned short wavebufClose(long handle)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    {
        unsigned short error = object->close();
        if (error)
            return error;
    }
    delete object;
    return 0;
}

/* @zoombi32 0x0047c40d */
unsigned short wavebufGetLevels(long handle, unsigned long *levels)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getLevels(levels);
}

/* waveOutGetDevCaps; asked for sizeof(WmxCaps), also the rates and formats
   the mixer supports. */
/* @zoombi32 0x0047c432 */
unsigned short wavebufGetDevCaps(unsigned short device, WmxCaps *caps, unsigned short size)
{
    unsigned short error;
    short full = size == sizeof(WmxCaps);
    wavebuf *buffer;
    wmxDevice *mixer;

    memset(caps, 0, size);
    if (!wmx.enabled) {
        if ((error = waveOutGetDevCaps(device, &caps->caps, full ? sizeof(WAVEOUTCAPS) : size)) != 0)
            return error;
        if (full) {
            caps->rate = 11025;
            caps->formats = caps->caps.dwFormats;
        }
    } else {
        for (mixer = wmx.devices; mixer && device != mixer->device; mixer = mixer->next)
            ;
        if (mixer)
            buffer = mixer->buffer;
        else if ((error = newWavebuf(device, &buffer)) != 0)
            return error;
        caps->caps.wMid = 0;
        caps->caps.wPid = 0;
        caps->caps.vDriverVersion = 0x500;
        strcpy(caps->caps.szPname, buffer->name);
        caps->caps.wChannels = 2;
        caps->caps.dwSupport = WAVECAPS_PLAYBACKRATE | WAVECAPS_VOLUME | WAVECAPS_LRVOLUME;
        caps->caps.dwFormats = 0xfff;
        if (full) {
            caps->caps.dwSupport |= 0x80000000;
            wmxDeviceFormats(buffer, &caps->rate, &caps->formats);
        }
        if (!mixer)
            delete buffer;
    }
    return 0;
}

/* @zoombi32 0x0047c56e */
unsigned short wavebufGetID(long handle, unsigned short *id)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getID(id);
}

/* @zoombi32 0x0047c593 */
unsigned short wavebufGetPitch(long handle, unsigned long *pitch)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getPitch(pitch);
}

/* @zoombi32 0x0047c5b8 */
unsigned short wavebufGetPlaybackRate(long handle, unsigned long *rate)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getPlaybackRate(rate);
}

/* @zoombi32 0x0047c5dd */
unsigned short wavebufGetPosition(long handle, MMTIME *time, unsigned short size)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getPosition(time, size);
}

/* @zoombi32 0x0047c607 */
unsigned short wavebufGetVolume(long handle, unsigned long *volume)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->getVolume(volume);
}

/* Reads [WaveMix]: ulFrameRate (11025, 22050 or 44100), ulFrameSize (8 or
   16), fStereo and fEnable. 1 if already initialized or misconfigured. */
/* @zoombi32 0x0047c62c */
short __cdecl initWaveMix()
{
    WmxState *state = &wmx;
    long rate;
    long size;

    if (state->initialized)
        return 1;
    memset(state, 0, sizeof *state);
    rate = 22050;
    getIniLong(0, waveMixSection, frameRateKey, &rate);
    switch (rate) {
    case 11025:
    case 22050:
    case 44100:
        state->rate = rate;
        break;
    default:
        return 1;
    }
    size = 16;
    getIniLong(0, waveMixSection, frameSizeKey, &size);
    switch (size) {
    case 8:
    case 16:
        state->frameSize = size;
        break;
    default:
        return 1;
    }
    getIniBool(0, waveMixSection, stereoKey, &state->stereo);
    state->enabled = 1;
    getIniBool(0, waveMixSection, enableKey, &state->enabled);
    initWavebuf();
    state->initialized = 1;
    return 0;
}

/* waveOutOpen: through WaveMix when it's enabled and the format is 8- or
   16-bit PCM, else straight to waveOut. */
/* @zoombi32 0x0047c712 */
unsigned short wavebufOpen(long *handle, unsigned short device, PCMWAVEFORMAT *format,
                           long callback, long instance, unsigned long flags)
{
    short error;
    wmxDevice *mixer;
    wmxObject *object;

    if (wmx.enabled)
        goto mix;
direct:
    if (flags & WAVE_FORMAT_QUERY)
        return waveOutOpen(0, device, (WAVEFORMAT *)format, callback, instance, flags);
    if (flags & 0x80000000)
        return MMSYSERR_NOTSUPPORTED;
    *handle = 0;
    {
        unsigned short result;
        wmxWaveOut *wave = new wmxWaveOut(format, callback, instance, flags);
        if (wave) {
            if ((result = wave->open(device, flags)) != 0)
                delete wave;
            else
                *handle = (long)wave;
        } else
            result = MMSYSERR_NOMEM;
        return result;
    }
mix:
    if (!format || format->wf.wFormatTag != WAVE_FORMAT_PCM || !format->wf.nChannels
        || format->wf.nChannels > 2 || format->wBitsPerSample != 8 && format->wBitsPerSample != 16
        || format->wf.nChannels * format->wBitsPerSample / 8 != format->wf.nBlockAlign)
        return WAVERR_BADFORMAT;
    if (flags & WAVE_FORMAT_QUERY)
        return 0;
    {
        unsigned long type = flags & CALLBACK_TYPEMASK;
        if (type && type != CALLBACK_FUNCTION)
            return MMSYSERR_INVALPARAM;
    }
    for (mixer = wmx.devices; mixer && mixer->device != device; mixer = mixer->next)
        ;
    if (!mixer) {
        if ((mixer = new wmxDevice) != 0) {
            if ((error = mixer->open(device)) != 0) {
                delete mixer;
                if (error == MMSYSERR_NOTSUPPORTED)
                    goto direct;
                return error;
            }
        } else
            return MMSYSERR_NOMEM;
    }
    object = flags & 0x80000000 ? new wmxObject(mixer, format, callback, instance, flags)
                                : new wmxMixer(mixer, format, callback, instance, flags);
    if (object)
        object->notify(WOM_OPEN, (long)object, 0);
    else {
        delete mixer;
        return MMSYSERR_NOMEM;
    }
    *handle = (long)object;
    return 0;
}

/* @zoombi32 0x0047c94b */
unsigned short wavebufPause(long handle)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->pause();
}

/* @zoombi32 0x0047c96b */
unsigned short wavebufPrepareHeader(long handle, WAVEHDR *header, unsigned short size)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->prepareHeader(header, size);
}

/* @zoombi32 0x0047c995 */
void __cdecl closeWaveMix()
{
    WmxState *state = &wmx;

    while (state->objects)
        delete state->objects;
    closeWavebuf();
    state->initialized = 0;
}

/* @zoombi32 0x0047c9c8 */
unsigned short wavebufReset(long handle)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->reset();
}

/* @zoombi32 0x0047c9e8 */
unsigned short wavebufRestart(long handle)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->restart();
}

/* @zoombi32 0x0047ca08 */
unsigned short wavebufSetLevels(long handle, unsigned long levels)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->setLevels(levels);
}

/* @zoombi32 0x0047ca2d */
unsigned short wavebufSetPitch(long handle, unsigned long pitch)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->setPitch(pitch);
}

/* @zoombi32 0x0047ca52 */
unsigned short wavebufSetPlaybackRate(long handle, unsigned long rate)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->setPlaybackRate(rate);
}

/* @zoombi32 0x0047ca77 */
unsigned short wavebufSetVolume(long handle, unsigned long volume)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->setVolume(volume);
}

/* @zoombi32 0x0047ca9c */
unsigned short wavebufUnprepareHeader(long handle, WAVEHDR *header, unsigned short size)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->unprepareHeader(header, size);
}

/* @zoombi32 0x0047cac6 */
unsigned short wavebufWrite(long handle, WAVEHDR *header, unsigned short size)
{
    wmxObject *object = wmxObjectOf(handle);

    if (!object)
        return MMSYSERR_INVALHANDLE;
    return object->write(header, size);
}

/* The WaveMix object a handle stands for; 0 if it isn't one. */
/* @zoombi32 0x0047caf0 */
wmxObject *wmxObjectOf(long handle)
{
    wmxObject *object = (wmxObject *)handle;

    if (!object || object->tag != 0x574d6978)
        object = 0;
    return object;
}

/* @zoombi32 0x0047cb09 */
void *__cdecl wmxObject::operator new(size_t size)
{
    return localAlloc(size);
}

/* @zoombi32 0x0047cb18 */
void *__cdecl wmxDevice::operator new(size_t size)
{
    void *block = localAlloc(size);

    if (!block)
        block = 0;
    return block;
}
