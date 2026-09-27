/*
 * module_4764bc (Mohawk engine): sounds (the API over MIDI and wave objects,
 * the default devices, and audioObj's own methods)
 */

/* @flags -p -x- */

#include <stdio.h>

#include "zoombinis.h"

SoundState sound;

/* The module's strings, named (their addresses are pushed, not pooled). */
static char audio[] = "Audio";
static char cacheMidiKey[] = "fCacheDefaultMidiDevice";
static char cacheWaveKey[] = "fCacheDefaultWaveDevice";
static char midiDeviceKey[] = "DefaultMidiDevice";
static char waveDeviceKey[] = "DefaultWaveDevice";
static char translateKey[] = "fTranslateWaveRateOnError";
static char rateTranslations[] = "Audio.WaveRateTranslations";
static char soundMapper[] = "Software\\Microsoft\\Multimedia\\Sound Mapper";
static char playback[] = "Playback";
static char versionFormat[] = "%[;]%d%[.]%d%[+-]";
static char rateFormat[] = "%lu";

/* Turns sounds on or off (as the application is activated or not): each
   sound closes or reopens its device, and so do the cached default devices. */
/* @zoombi32 0x004764bc */
short setSoundsActive(short active)
{
    short error = 0;
    audioObj *object;
    PCMWAVEFORMAT format;

    if (sound.active && !active) {
    deactivate:
        for (object = sound.objects; object; object = object->next)
            object->activate(0);
        if (sound.cacheMidiDevice && sound.midiCache) {
            midiMapClose((long)sound.midiCache);
            sound.midiCache = 0;
        }
        if (sound.cacheWaveDevice && sound.waveCache) {
            wavebufClose(sound.waveCache);
            sound.waveCache = 0;
        }
        sound.driverOpen = fn_47a074(0) == 0 ? 0 : 1;
        sound.active = 0;
    } else if (!sound.active && active) {
        if (!sound.driverOpen) {
            if (fn_47a074(1))
                return setSoundError(0x29cd);
            sound.driverOpen = 1;
        }
        if (sound.cacheMidiDevice)
            midiMapOpen(&sound.midiCache, sound.midiDevice, 0, 0, 0x80000000);
        if (sound.cacheWaveDevice) {
            format.wf.wFormatTag = WAVE_FORMAT_PCM;
            format.wf.nChannels = 1;
            format.wf.nSamplesPerSec = 11025;
            format.wf.nAvgBytesPerSec = 11025;
            format.wf.nBlockAlign = 1;
            format.wBitsPerSample = 8;
            openWaveOut(&sound.waveCache, sound.waveDevice, &format, 0, 0, 0x80000000);
        }
        for (object = sound.objects; object; object = object->next)
            if ((error = object->activate(1)) != 0)
                goto deactivate;
        sound.active = 1;
        error = 0;
    }
    return sound.error = error;
}

/* Frees a sound (it must be closed). */
/* @zoombi32 0x00476622 */
short disposeSound(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (object->isOpen)
        return setSoundError(0x2a06);
    object->release();
    if (object->next)
        object->next->prev = object->prev;
    if (object->prev)
        object->prev->next = object->next;
    else
        sound.objects = object->next;
    object->tag = 0;
    disposePtr(object);
    return setSoundError(0);
}

/* Calls `proc` for each open sound of a kind on a device (0xffff: the
   default one) until it returns non-zero. */
/* @zoombi32 0x00476698 */
short forEachSound(long kind, unsigned short device, short (*proc)(audioObj *object, long data),
                   long data)
{
    audioObj *object;

    if (kind == 0) {
        if (device == 0xffff)
            device = sound.midiDevice;
        if (midiOutGetNumDevs() <= device)
            return setSoundError(0x29ce);
    } else if (kind == 1) {
        if (device == 0xffff)
            device = sound.waveDevice;
        if (waveOutGetNumDevs() <= device)
            return setSoundError(0x29ce);
    } else
        return setSoundError(0x29fe);
    for (object = sound.objects; object; object = object->next)
        if (object->isOpen && kind == object->kind && device == object->device
            && proc(object, data))
            break;
    return setSoundError(0);
}

/* Picks the default MIDI device: [Audio] DefaultMidiDevice, by number or
   name; else none (0xffff). */
/* @zoombi32 0x00476742 */
void chooseMidiDevice()
{
    char name[32];
    MIDIOUTCAPS caps;
    unsigned short count;
    unsigned short device;

    sound.midiDevice = 0xffff;
    count = midiOutGetNumDevs();
    if (!getIniString(0, audio, midiDeviceKey, name, sizeof name)
        || iniError() == 0x296c) {
        if (name[0] >= '0' && name[0] <= '9') {
            device = parseNumber(name);
            if (device < count) {
                sound.midiDevice = device;
                return;
            }
        } else
            for (device = 0; device < count; device++)
                if (!midiOutGetDevCaps(device, &caps, sizeof caps) && !stricmp(name, caps.szPname)) {
                    sound.midiDevice = device;
                    return;
                }
    }
    sound.midiDevice = 0xffff;
}

/* Picks the default wave device: [Audio] DefaultWaveDevice, by number or
   name; else the Sound Mapper's playback device; else the first. */
/* @zoombi32 0x004767f9 */
void chooseWaveDevice()
{
    HKEY key;
    DWORD type;
    DWORD size;
    char name[32];
    WAVEOUTCAPS caps;
    unsigned short count;
    unsigned short device;
    long result;

    sound.waveDevice = 0xffff;
    count = waveOutGetNumDevs();
    if (!getIniString(0, audio, waveDeviceKey, name, sizeof name)
        || iniError() == 0x296c) {
        if (name[0] >= '0' && name[0] <= '9') {
            device = parseNumber(name);
            if (device < count) {
                sound.waveDevice = device;
                return;
            }
        } else {
        byName:
            for (device = 0; device < count; device++)
                if (!waveOutGetDevCaps(device, &caps, sizeof caps) && !stricmp(name, caps.szPname)) {
                    sound.waveDevice = device;
                    return;
                }
        }
    }
    if (sound.waveDevice == 0xffff) {
        sound.waveDevice = 0;
        if (!RegOpenKey(HKEY_CURRENT_USER, soundMapper, &key)) {
            size = sizeof name;
            result = RegQueryValueEx(key, playback, 0, &type, (BYTE *)name, &size);
            RegCloseKey(key);
            if (!result && type == REG_SZ)
                goto byName;
        }
    }
}

/* Looks `name` up among the keys of a settings section, copying the key
   into `entry`. A key can be quoted ("..." or '...', the latter matching
   names that start with it), and followed by a version (";major.minor",
   then '+' for that or older or '-' for that or newer) that must match
   `version` unless that's 0. */
/* @zoombi32 0x0047690f */
short findIniEntry(fileSpec *file, const char *section, char *entry, unsigned short size,
                   const char *name, unsigned short version, unsigned short, unsigned short)
{
    unsigned short nameLength;
    unsigned short length;
    char *text;
    char *rest;
    struct
    {
        char sign[24];
        int minor;
        int major;
        char point[24];
        char separator[20];
    } parsed;
    char keys[0x800];
    char *key;
    char *quote;
    unsigned short textLength;

    *entry = 0;
    if (getIniString(file, section, 0, keys, sizeof keys) && iniError() != 0x296c)
        return setSoundError(iniError());
    nameLength = strlen(name);
    for (key = keys; (length = strlen(key)) != 0; key += length + 1) {
        quote = 0;
        if ((*key == '"' || *key == '\'') && (quote = strrchr(key + 1, *key)) != 0) {
            text = key + 1;
            textLength = quote - text;
            rest = quote + 1;
        } else {
            text = key;
            textLength = length;
            rest = text + textLength;
        }
        if (textLength > 0
            && (!stricmp(text, name)
                || quote && *quote == '\'' && textLength <= nameLength
                       && !memicmp(text, name, textLength))) {
            if (version && *rest) {
                if (sscanf(rest, versionFormat, parsed.separator, &parsed.major, parsed.point, &parsed.minor,
                           parsed.sign) < 4)
                    continue;
                unsigned short found = makeWord(parsed.minor, parsed.major);
                if (!(!parsed.sign[0] && found == version
                      || parsed.sign[0] == '+' && found <= version
                      || parsed.sign[0] == '-' && found >= version))
                    continue;
            }
            if (length + 1 > size)
                return setSoundError(0x296c);
            strcpy(entry, key);
            return setSoundError(0);
        }
    }
    return setSoundError(0x296b);
}

/* A sound's device; 0xffff on error (or if it isn't open). */
/* @zoombi32 0x00476ade */
unsigned short soundDevice(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return 0xffff;
    }
    if (!object->isOpen) {
        setSoundError(0x2a03);
        return 0xffff;
    }
    setSoundError(0);
    return object->device;
}

/* @zoombi32 0x00476b27 */
long soundDeviceHandle(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return 0;
    }
    if (!object->active) {
        setSoundError(0x2a00);
        return 0;
    }
    if (!object->isOpen) {
        setSoundError(0x2a03);
        return 0;
    }
    setSoundError(0);
    return object->deviceHandle();
}

/* @zoombi32 0x00476b84 */
long soundDuration(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return -1;
    }
    setSoundError(0);
    return object->duration;
}

/* @zoombi32 0x00476bb4 */
short soundError()
{
    return sound.error;
}

/* @zoombi32 0x00476bc2 */
long soundRate(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return 0;
    }
    setSoundError(0);
    return object->rate;
}

/* @zoombi32 0x00476bf1 */
long soundPosition(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return -1;
    }
    setSoundError(0);
    return object->position();
}

/* A sound's state: 1 open, 2 playing, 4 started, 8 and 0x10 unknown. */
/* @zoombi32 0x00476c25 */
unsigned short soundFlags(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return 0xffff;
    }
    setSoundError(0);
    return (object->isOpen ? 1 : 0) | (object->playing ? 2 : 0) | (object->started ? 4 : 0)
           | (object->endingLoop ? 8 : 0) | (object->unknown20 ? 0x10 : 0);
}

/* @zoombi32 0x00476ca3 */
long soundKind(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return -1;
    }
    setSoundError(0);
    return object->kind;
}

/* @zoombi32 0x00476cd3 */
long soundVolume(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0) {
        setSoundError(0x29ff);
        return -1;
    }
    setSoundError(0);
    return object->volume;
}

/* Starts sound: reads [Audio]'s settings, picks the default devices and
   opens them if they're to be kept open. */
/* @zoombi32 0x00476d0a */
short initSound()
{
    PCMWAVEFORMAT format;

    switch (initMidi()) {
    case 4:
        return setSoundError(0x29cd);
    default:
        return setSoundError(0x2a07);
    case 0:;
    }
    memset(&sound, 0, sizeof sound);
    sound.driverOpen = 1;
    getIniBool(0, audio, cacheMidiKey, &sound.cacheMidiDevice);
    getIniBool(0, audio, cacheWaveKey, &sound.cacheWaveDevice);
    getIniBool(0, audio, translateKey, &sound.translateWaveRate);
    chooseMidiDevice();
    if (sound.cacheMidiDevice)
        midiMapOpen(&sound.midiCache, sound.midiDevice, 0, 0, 0x80000000);
    chooseWaveDevice();
    if (sound.cacheWaveDevice) {
        format.wf.wFormatTag = WAVE_FORMAT_PCM;
        format.wf.nChannels = 1;
        format.wf.nSamplesPerSec = 11025;
        format.wf.nAvgBytesPerSec = 11025;
        format.wf.nBlockAlign = 1;
        format.wBitsPerSample = 8;
        openWaveOut(&sound.waveCache, sound.waveDevice, &format, 0, 0, 0x80000000);
    }
    sound.ready = 1;
    sound.active = 1;
    return setSoundError(0);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00476e1f */
short pauseSound(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->active)
        return setSoundError(0x2a00);
    if (!object->started)
        return setSoundError(0x2a02);
    object->pause();
    return setSoundError(0);
}

/* Opens a sound on a device (0xffff: its kind's default). */
/* Not exact: the original keeps `object` in eax and `device` in ebx; BCC32
   4.5 gives them ebx and esi. */
/* @zoombi32 0x00476e72 */
short openSound(long handle, unsigned short device)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->active)
        return setSoundError(0x2a00);
    if (object->isOpen)
        return setSoundError(0x2a06);
    if (device == 0xffff) {
        if (object->kind == 0)
            device = sound.midiDevice;
        else
            device = sound.waveDevice;
    }
    return object->open(device);
}

/* @zoombi32 0x00476ee3 */
short soundBufferSize()
{
    return sound.ready ? 0x500 : 0;
}

/* Stops, closes and frees every sound, and stops sound. */
/* @zoombi32 0x00476ef7 */
void closeSounds()
{
    audioObj *object;

    while ((object = sound.objects) != 0) {
        object->stop();
        if (object->isOpen)
            object->close();
        disposeSound((long)object);
    }
    if (sound.midiCache)
        midiMapClose((long)sound.midiCache);
    if (sound.waveCache)
        wavebufClose(sound.waveCache);
    sound.ready = 0;
    closeMidi();
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00476f50 */
short endSoundLoop(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->active)
        return setSoundError(0x2a00);
    if (!object->isOpen)
        return setSoundError(0x2a03);
    object->endLoop();
    return setSoundError(0);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00476fa3 */
short resumeSound(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->active)
        return setSoundError(0x2a00);
    if (!object->playing)
        return setSoundError(0x2a01);
    object->resume();
    return setSoundError(0);
}

/* @zoombi32 0x00476ff6 */
short seekSound(long handle, long position)
{
    audioObj *object;
    short error;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (object->playing)
        return setSoundError(0x2a04);
    if (object->started)
        return setSoundError(0x2a05);
    error = object->seek(position);
    if (!error && !position)
        object->resetLoop();
    return error;
}

/* Sets a sound's text (0xffff: NUL-terminated). */
/* @zoombi32 0x00477064 */
short setSoundText(long handle, const char *text, unsigned short length)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (object->playing)
        return setSoundError(0x2a04);
    if (object->started)
        return setSoundError(0x2a05);
    return object->setText(text, length == 0xffff ? strlen(text) : length);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004770d4 */
short setSoundRate(long handle, long rate)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    return object->setRate(rate);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004770ff */
short setSoundVolume(long handle, long volume)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    return object->setVolume(volume);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0047712a */
short playSound(long handle, SoundNotify notify, long cookie)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->active)
        return setSoundError(0x2a00);
    if (!object->isOpen)
        return setSoundError(0x2a03);
    if (object->playing)
        return setSoundError(0x2a04);
    if (object->started)
        return setSoundError(0x2a05);
    return object->play(notify, cookie);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004771a4 */
short stopSound(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->isOpen)
        return setSoundError(0x2a03);
    object->stop();
    return setSoundError(0);
}

/* Not exact: the original keeps `object` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004771e4 */
short closeSound(long handle)
{
    audioObj *object;

    if ((object = audioObject(handle)) == 0)
        return setSoundError(0x29ff);
    if (!object->isOpen)
        return setSoundError(0x2a03);
    if (object->playing)
        return setSoundError(0x2a04);
    if (object->started)
        return setSoundError(0x2a05);
    object->close();
    return setSoundError(0);
}

/* Makes `device` the default MIDI device (reopening it if it's kept open). */
/* @zoombi32 0x0047724a */
unsigned short setMidiDevice(unsigned short device)
{
    unsigned short previous;

    if (midiOutGetNumDevs() <= device && device != 0xffff) {
        setSoundError(0x29ce);
        return 0xffff;
    }
    previous = sound.midiDevice;
    sound.midiDevice = device;
    if (sound.cacheMidiDevice && previous != device) {
        if (sound.midiCache) {
            midiMapClose((long)sound.midiCache);
            sound.midiCache = 0;
        }
        if (sound.active)
            midiMapOpen(&sound.midiCache, device, 0, 0, 0x80000000);
    }
    setSoundError(0);
    return previous;
}

/* Makes `device` the default wave device (reopening it if it's kept open). */
/* @zoombi32 0x004772da */
unsigned short setWaveDevice(unsigned short device)
{
    unsigned short previous;
    PCMWAVEFORMAT format;

    if (waveOutGetNumDevs() <= device) {
        setSoundError(0x29ce);
        return 0xffff;
    }
    previous = sound.waveDevice;
    sound.waveDevice = device;
    if (sound.cacheWaveDevice && previous != device) {
        if (sound.waveCache) {
            wavebufClose(sound.waveCache);
            sound.waveCache = 0;
        }
        if (sound.active) {
            format.wf.wFormatTag = WAVE_FORMAT_PCM;
            format.wf.nChannels = 1;
            format.wf.nSamplesPerSec = 11025;
            format.wf.nAvgBytesPerSec = 11025;
            format.wf.nBlockAlign = 1;
            format.wBitsPerSample = 8;
            openWaveOut(&sound.waveCache, sound.waveDevice, &format, 0, 0, 0x80000000);
        }
    }
    setSoundError(0);
    return previous;
}

/* Opens a wave device for PCM whose rate the device may not take:
   [Audio.WaveRateTranslations] maps a rate to one to use instead. */
/* @zoombi32 0x0047739b */
unsigned short openWaveOutDevice(long *out, unsigned short device, PCMWAVEFORMAT *format, long a, long b,
                        long flags)
{
    char key[8];
    char value[8];
    unsigned short rate;
    struct
    {
        char unknown0[0x34];
        unsigned long rate;
        char unknown38[4];
    } caps;
    short error;

    if (sound.translateWaveRate || format->wf.wFormatTag != WAVE_FORMAT_PCM) {
        error = openWaveOut(out, device, format, a, b, flags);
        if (error != 0x20 || format->wf.wFormatTag != WAVE_FORMAT_PCM)
            return error;
    }
    if ((error = getWaveCaps(device, &caps, sizeof caps)) != 0)
        return error;
    if (format->wf.nSamplesPerSec % caps.rate) {
        sprintf(key, rateFormat, format->wf.nSamplesPerSec);
        if ((!getIniString(0, rateTranslations, key, value, sizeof value)
             || iniError() == 0x296c)
            && (rate = parseNumber(value)) != 0) {
            format->wf.nSamplesPerSec = rate;
            format->wf.nAvgBytesPerSec = rate * format->wf.nBlockAlign;
        } else if (error)
            return error;
    }
    return openWaveOut(out, device, format, a, b, flags);
}

/* Turns a sound on or off: closes its device, or reopens it (and restarts
   it if it was playing). */
/* Not exact: `on` and `error` swap registers (edi and esi). */
/* @zoombi32 0x00477490 */
short __cdecl audioObj::activate(short on)
{
    short error;

    if (active && !on) {
        if (playing || started)
            haltDevice();
        if (isOpen)
            closeDevice();
    } else if (!active && on && isOpen) {
        if ((error = openDevice()) != 0)
            return error;
        if ((error = setDeviceRate(rate)) != 0 || (error = setDeviceVolume(volume)) != 0) {
            closeDevice();
            return sound.error = error;
        }
        if ((playing || started) && (error = startDevice(playing)) != 0) {
            closeDevice();
            return sound.error = error;
        }
    }
    active = on;
    return setSoundError(0);
}

/* Opens the sound's device. */
/* @zoombi32 0x0047757e */
short __cdecl audioObj::open(unsigned short on)
{
    short error;

    device = on;
    if ((error = openDevice()) != 0)
        return error;
    if ((error = setDeviceRate(rate)) != 0 || (error = setDeviceVolume(volume)) != 0) {
        closeDevice();
        return sound.error = error;
    }
    if ((error = lockPtr(this)) != 0) {
        closeDevice();
        return setSoundError(error);
    }
    isOpen = 1;
    return setSoundError(0);
}

/* @zoombi32 0x00477605 */
short __cdecl audioObj::setRate(long level)
{
    if (isOpen && setDeviceRate(level))
        return sound.error;
    rate = level;
    return setSoundError(0);
}

/* @zoombi32 0x0047763c */
short __cdecl audioObj::setVolume(long level)
{
    if (isOpen && setDeviceVolume(level))
        return sound.error;
    volume = level;
    return setSoundError(0);
}

/* Starts the sound, telling `notify`. */
/* @zoombi32 0x00477673 */
short __cdecl audioObj::play(SoundNotify proc, long data)
{
    SoundNotice notice;

    notify = proc;
    cookie = data;
    enterLock(&lock);
    started = 1;
    if (!startDevice(0)) {
        notice.what = 4;
        notifySound(this, &notice);
        setSoundError(0);
    } else
        started = 0;
    leaveLock(&lock);
    return sound.error;
}

/* @zoombi32 0x004776e0 */
void __cdecl audioObj::stop()
{
    SoundNotice notice;
    short wasPlaying = playing || started;

    if (wasPlaying && active)
        haltDevice();
    playing = 0;
    started = 0;
    if (wasPlaying) {
        notice.what = 5;
        notifySound(this, &notice);
    }
}

/* @zoombi32 0x0047773f */
void __cdecl audioObj::close()
{
    unlockPtr(this);
    if (active)
        closeDevice();
    isOpen = 0;
}

/* @zoombi32 0x00477763 */
audioObj *audioObject(long handle)
{
    audioObj *object = (audioObj *)handle;

    if (object && object->tag == 0x414f626a)
        return object;
    return 0;
}

/* @zoombi32 0x0047777c */
unsigned short __cdecl makeWord(unsigned char low, unsigned char high)
{
    return ((unsigned short)high << 8) + low;
}
