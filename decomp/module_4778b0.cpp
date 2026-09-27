/*
 * module_4778b0 (Mohawk engine): the MIDI mapper's devices and maps
 */

/* @flags -p -x- */

#include <stdio.h>

#include "zoombinis.h"

MidiMapState midiMapState;

/* The module's data, named (their addresses are pushed, not pooled). */
static long sysexType = RESOURCE_TYPE('S', 'Y', 'S', 'X');
static char midiMapSection[] = "MidiMap";
static char hardResetKey[] = "fEnableHardReset";
static char targetDevices[] = "MidiMap.TargetDevices";
static char targetDeviceInfo[] = "MidiMap.TargetDeviceInfo";
static char voicesKey[] = "unknown device (%d+ voices)";
static char portKey[] = "unknown device (port)";
static short capsCached;
static unsigned short maxNotes;
static unsigned short maxVoices;
static char numberFormat[] = "%d";
static char deviceInfoFormat[] = "%16s %[,;:] %d %[,;:] %d %[,;:] %d";

/* midiOutCacheDrumPatches for a map (bank 0 only), keeping track of which
   drum patches it has cached; uncaching leaves those other maps use. */
/* @zoombi32 0x004778b0 */
short midiMapCacheDrumPatches(long handle, unsigned short patch, WORD *keys, unsigned short flags)
{
    WORD cached[128];
    MidiDevice *device;
    MidiMap *map;
    MidiMap *other;
    short error;
    int i;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    if (patch)
        return MMSYSERR_INVALPARAM;
    device = map->device;
    if (!(device->caps.dwSupport & MIDICAPS_CACHE))
        return MMSYSERR_NOTSUPPORTED;
    switch (flags) {
    case MIDI_CACHE_ALL:
    case MIDI_CACHE_BESTFIT:
        error = midiOutCacheDrumPatches(device->out, patch, keys, flags);
        midiOutCacheDrumPatches(device->out, 0, cached, MIDI_CACHE_QUERY);
        for (i = 0; i < 128; i++) {
            map->drumCache[i] |= keys[i];
            map->drumCache[i] &= cached[i];
        }
        return error;
    case MIDI_CACHE_QUERY:
        memcpy(keys, map->patchCache, sizeof map->patchCache);
        return 0;
    case MIDI_UNCACHE:
        for (i = 0; i < 128; i++)
            map->drumCache[i] &= ~keys[i];
        for (other = map->next; other != map; other = other->next)
            if (!other->minimal)
                for (i = 0; i < 128; i++)
                    keys[i] &= ~other->drumCache[i];
        midiOutCacheDrumPatches(device->out, 0, keys, MIDI_UNCACHE);
        return 0;
    default:
        return MMSYSERR_INVALPARAM;
    }
}

/* midiOutCachePatches for a map, likewise. */
/* @zoombi32 0x00477a07 */
short midiMapCachePatches(long handle, unsigned short bank, WORD *patches, unsigned short flags)
{
    WORD cached[128];
    MidiDevice *device;
    MidiMap *map;
    MidiMap *other;
    short error;
    int i;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    if (bank)
        return MMSYSERR_INVALPARAM;
    device = map->device;
    if (!(device->caps.dwSupport & MIDICAPS_CACHE))
        return MMSYSERR_NOTSUPPORTED;
    switch (flags) {
    case MIDI_CACHE_ALL:
    case MIDI_CACHE_BESTFIT:
        error = midiOutCachePatches(device->out, bank, patches, flags);
        midiOutCachePatches(device->out, 0, cached, MIDI_CACHE_QUERY);
        for (i = 0; i < 128; i++) {
            map->patchCache[i] |= patches[i];
            map->patchCache[i] &= cached[i];
        }
        return error;
    case MIDI_CACHE_QUERY:
        memcpy(patches, map->patchCache, sizeof map->patchCache);
        return 0;
    case MIDI_UNCACHE:
        for (i = 0; i < 128; i++)
            map->patchCache[i] &= ~patches[i];
        for (other = map->next; other != map; other = other->next)
            if (!other->minimal)
                for (i = 0; i < 128; i++)
                    patches[i] &= ~other->patchCache[i];
        midiOutCachePatches(device->out, 0, patches, MIDI_UNCACHE);
        return 0;
    default:
        return MMSYSERR_INVALPARAM;
    }
}

/* Fills a map's velocity curve from a table (up to 127). */
/* Not exact: the original keeps `table` in edi, `map` in esi and the velocity in
   eax; BCC32 4.5 keeps `map` in edi, `table` on the stack and the velocity in esi. */
/* @zoombi32 0x00477b5e */
void fillVelocities(MidiMap *map, long table)
{
    int i;

    for (i = 0; i < 128; i++) {
        unsigned long velocity;
        if ((velocity = fixedMul(i, table)) > 0x7f)
            velocity = 0x7f;
        map->velocities[i] = velocity;
    }
}

/* midiOutClose for a map: resets its channels, and closes the device when
   it's the last map on it (sending the device's closing sysex first). */
/* Not exact: `map` and `channel` swap registers (ebx and esi). */
/* @zoombi32 0x00477b91 */
short midiMapClose(long handle)
{
    MidiHeader header;
    MidiMap *map;
    MidiDevice *device;
    short sysex;
    int channel;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (!map->minimal)
        for (channel = 0; channel < 16; channel++)
            resetChannel(map, channel);
    device = map->device;
    if (device->users == 1) {
        if (device->closeSysex && (sysex = loadResource(device->closeSysex, 0)) != 0) {
            memset(&header, 0, sizeof header);
            header.lpData = (LPSTR)lockHandle(sysex);
            header.dwBufferLength = handleSize(sysex);
            if (!midiOutPrepareHeader(device->out, &header, sizeof header)) {
                if (!midiOutLongMsg(device->out, &header, sizeof header))
                    while (!(header.dwFlags & MHDR_DONE))
                        continue;
                midiOutUnprepareHeader(device->out, &header, sizeof header);
            }
            unlockHandle(sysex);
        }
        midiOutReset(device->out);
        midiOutClose(device->out);
        unlockPtr(device);
    }
    device->users--;
    map->prev->next = map->next;
    map->next->prev = map->prev;
    if (!device->users)
        device->maps = 0;
    else if (map == device->maps)
        device->maps = map->next;
    map->tag = 0;
    unlockPtr(map);
    disposePtr(map);
    return 0;
}

/* @zoombi32 0x00477cd5 */
short getChannelMap(unsigned short id, unsigned short *channels)
{
    MidiDevice *device;
    short error;

    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    memcpy(channels, device->channels, sizeof device->channels);
    return 0;
}

/* @zoombi32 0x00477d0f */
short getMutedChannels(unsigned short id, short *muted)
{
    MidiDevice *device;
    short error;

    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    memcpy(muted, device->muted, sizeof device->muted);
    return 0;
}

/* midiOutGetDevCaps; for the MIDI mapper, with the most notes and voices
   of any device. */
/* @zoombi32 0x00477d49 */
short getMidiDevCaps(unsigned short id, MIDIOUTCAPS *caps, unsigned short size)
{
    MIDIOUTCAPS other;
    short error;
    unsigned short count;
    unsigned short i;

    if ((error = midiOutGetDevCaps(id, caps, size)) != 0)
        return error;
    if (id == 0xffff) {
        if (!capsCached) {
            count = midiOutGetNumDevs();
            for (i = 0; i < count; i++) {
                midiOutGetDevCaps(i, &other, sizeof other);
                if (maxNotes < other.wNotes)
                    maxNotes = other.wNotes;
                if (maxVoices < other.wVoices)
                    maxVoices = other.wVoices;
            }
        }
        caps->wNotes = maxNotes;
        caps->wVoices = maxVoices;
    }
    return 0;
}

/* A device's record, made (with its settings) the first time it's asked
   for: [MidiMap.TargetDevices] names its entry in
   [MidiMap.TargetDeviceInfo] by the device's name and version, else by
   "unknown device (port)" or the most voices it has. */
/* @zoombi32 0x00477df1 */
short findMidiDevice(unsigned short id, MidiDevice **device)
{
    char entry[0x100];
    char number[16];
    char key[32];
    short error;
    int i;
    int voices;

    if (midiOutGetNumDevs() <= id && id != 0xffff)
        return MMSYSERR_BADDEVICEID;
    for (*device = midiMapState.devices; *device && id != (*device)->id; *device = (*device)->next)
        ;
    if (!*device) {
        if ((*device = (MidiDevice *)newPtr(sizeof(MidiDevice))) != 0) {
            memset(*device, 0, sizeof(MidiDevice));
            (*device)->id = id;
            (*device)->drumChannel = 9;
            for (i = 0; i < 16; i++)
                (*device)->channels[i] = i;
            if ((error = getMidiDevCaps(id, &(*device)->caps, sizeof(MIDIOUTCAPS))) != 0) {
                disposePtr(*device);
                return error;
            }
            (*device)->next = midiMapState.devices;
            midiMapState.devices = *device;
            if (!findIniEntry(0, targetDevices, entry, sizeof entry, (*device)->caps.szPname,
                              (*device)->caps.vDriverVersion, (*device)->caps.wMid,
                              (*device)->caps.wPid)
                && (!getIniString(0, targetDevices, entry, number, sizeof number)
                    || iniError() == 0x296c)
                && loadTargetDevice(*device, parseNumber(number)))
                ;
            else if ((*device)->caps.wTechnology == MOD_MIDIPORT
                     && (!getIniString(0, targetDevices, portKey, number, sizeof number)
                         || iniError() == 0x296c))
                loadTargetDevice(*device, parseNumber(number));
            else
                for (voices = (*device)->caps.wNotes; voices > 0; voices--) {
                    sprintf(key, voicesKey, voices);
                    if ((!getIniString(0, targetDevices, key, number, sizeof number)
                         || iniError() == 0x296c)
                        && loadTargetDevice(*device, parseNumber(number)))
                        break;
                }
        } else
            return MMSYSERR_NOMEM;
    }
    return 0;
}

/* @zoombi32 0x00478003 */
short getDrumChannel(unsigned short id, unsigned short *channel)
{
    MidiDevice *device;
    short error;

    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    *channel = device->drumChannel;
    return 0;
}

/* Not exact: the original keeps `map` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00478036 */
short midiMapDevice(long handle, unsigned short *id)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    *id = map->device->id;
    return 0;
}

/* Not exact: the original keeps `map` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0047805e */
short midiMapTarget(long handle, short *target)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    *target = map->device->target;
    return 0;
}

/* Not exact: the original keeps `map` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00478086 */
short midiMapTable(long handle, long *table)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    *table = map->table;
    return 0;
}

/* @zoombi32 0x004780b6 */
short initMidiMap()
{
    if (midiMapState.ready)
        return 1;
    memset(&midiMapState, 0, sizeof midiMapState);
    getIniBool(0, midiMapSection, hardResetKey, &midiMapState.hardReset);
    midiMapState.ready = 1;
    return 0;
}

/* Mutes a channel of a device (resetting it on every map using it). */
/* @zoombi32 0x004780fd */
short setChannelMuted(unsigned short id, unsigned short channel, short muted)
{
    MidiDevice *device;
    MidiMap *map;
    short error;

    if (channel >= 16)
        return MMSYSERR_INVALPARAM;
    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    if (muted != device->muted[channel] && muted && device->users > 0) {
        map = device->maps;
        do {
            if (!map->minimal)
                resetChannel(map, channel);
            map = map->next;
        } while (map != device->maps);
    }
    device->muted[channel] = muted;
    return 0;
}

/* midiOutOpen for a map (without callbacks). The device is opened for its
   first map, and sent its reset sysex. */
/* @zoombi32 0x0047818b */
short midiMapOpen(MidiMap **out, unsigned short id, long callback, long instance, long flags)
{
    MidiDevice *device;
    unsigned short size;
    MidiHeader header;
    MidiMap *map;
    short error;
    short sysex;

    *out = 0;
    if (callback || instance || flags & CALLBACK_TYPEMASK)
        return MMSYSERR_INVALPARAM;
    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    size = flags & 0x80000000 ? 0x98 : 0xab8;
    if ((map = (MidiMap *)newPtr(size)) == 0)
        return MMSYSERR_NOMEM;
    if (lockPtr(map)) {
        disposePtr(map);
        return MMSYSERR_NOMEM;
    }
    if (!device->users && (error = midiOutOpen(&device->out, id, 0, 0, 0)) != 0) {
        unlockPtr(map);
        disposePtr(map);
        return error;
    }
    memset(map, 0, size);
    map->tag = 0x4d4d6170;
    map->device = device;
    map->minimal = (flags & 0x80000000) != 0;
    if (!device->users++) {
        if (lockPtr(device)) {
            midiOutClose(device->out);
            device->users = 0;
            unlockPtr(map);
            disposePtr(map);
            return MMSYSERR_NOMEM;
        }
        if (device->resetSysex && (sysex = loadResource(device->resetSysex, 0)) != 0) {
            memset(&header, 0, sizeof header);
            header.lpData = (LPSTR)lockHandle(sysex);
            header.dwBufferLength = handleSize(sysex);
            if (!midiOutPrepareHeader(device->out, &header, sizeof header)) {
                if (!midiOutLongMsg(device->out, &header, sizeof header))
                    while (!(header.dwFlags & MHDR_DONE))
                        continue;
                midiOutUnprepareHeader(device->out, &header, sizeof header);
            }
            unlockHandle(sysex);
        }
    }
    if (!map->minimal) {
        map->table = makeFixed(1, 0);
        fillVelocities(map, map->table);
    }
    if ((map->next = device->maps) != 0) {
        map->prev = device->maps->prev;
        map->prev->next = map;
        device->maps->prev = map;
    } else {
        map->prev = map;
        map->next = map;
    }
    device->maps = map;
    *out = map;
    return 0;
}

/* @zoombi32 0x004783e1 */
short midiMapPrepareHeader(long handle, MidiHeader *header, unsigned short size)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    return midiOutPrepareHeader(map->device->out, header, size);
}

/* Closes every map and forgets the devices. */
/* @zoombi32 0x00478421 */
void closeMidiMaps()
{
    MidiMapState *state = &midiMapState;
    MidiDevice *device;

    if (state->ready) {
        while ((device = state->devices) != 0) {
            while (device->users) {
                midiMapReset((long)device->maps);
                midiMapClose((long)device->maps);
            }
            state->devices = device->next;
            disposePtr(device);
        }
        state->ready = 0;
    }
}

/* Sends a device's channel `channel` to channel `to` (resetting it on every
   map using it). */
/* @zoombi32 0x00478470 */
short setChannelMap(unsigned short id, unsigned short channel, unsigned short to)
{
    MidiDevice *device;
    MidiMap *map;
    short error;

    if (channel >= 16 || to >= 16)
        return MMSYSERR_INVALPARAM;
    if ((error = findMidiDevice(id, &device)) != 0)
        return error;
    if (to != device->channels[channel] && device->users > 0) {
        map = device->maps;
        do {
            if (!map->minimal)
                resetChannel(map, channel);
            map = map->next;
        } while (map != device->maps);
    }
    device->channels[channel] = to;
    return 0;
}

/* midiOutReset for a map: resets its channels, and the device (if
   [MidiMap] fEnableHardReset) once that leaves nothing playing. */
/* @zoombi32 0x004784ff */
short midiMapReset(long handle)
{
    MidiMap *map;
    unsigned short playing;
    int channel;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    playing = map->device->unknown90;
    for (channel = 0; channel < 16; channel++)
        resetChannel(map, channel);
    if (playing > 0 && !map->device->unknown90 && midiMapState.hardReset)
        midiOutReset(map->device->out);
    return 0;
}

/* Reads a device's settings from its entry in [MidiMap.TargetDeviceInfo]:
   its 16 channels' targets (hex digits, X for muted), its drum channel (from
   1), and the IDs of the SYSX resources to send on opening and closing it;
   non-zero if it could. */
/* Not exact: BCC32 4.5 leaves 32 unused bytes in the frame between `closing` and
   `separator` (the original has none), so every local below them is off. */
/* @zoombi32 0x00478572 */
short loadTargetDevice(MidiDevice *device, short target)
{
    char key[8];
    int drums;
    int opening;
    int closing;
    char separator[2];
    int fields;
    char info[64];
    char channels[20];
    int i;

    if (target < -64 || !target || target >= 64)
        return 0;
    sprintf(key, numberFormat, target);
    if (getIniString(0, targetDeviceInfo, key, info, sizeof info) && iniError() != 0x296c)
        return 0;
    opening = 0;
    closing = 0;
    fields = sscanf(info, deviceInfoFormat, channels, separator, &drums, separator, &opening,
                    separator, &closing);
    if (fields < 3 || drums <= 0 && drums > 16)
        return 0;
    for (i = 0; i < 16; i++) {
        char c = toupper(channels[i]);
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'F')
            c -= 'A' - 10;
        else if (c == 'X')
            c = -1;
        else
            return 0;
        channels[i] = c;
    }
    device->target = target;
    device->drumChannel = drums - 1;
    for (i = 0; i < 16; i++) {
        if (channels[i] < 0)
            device->muted[i] = 1;
        else
            device->channels[i] = channels[i];
    }
    if (fields >= 5)
        device->resetSysex = opening ? findResource(sysexType, opening, 0) : 0;
    if (fields >= 7)
        device->closeSysex = closing ? findResource(sysexType, closing, 0) : 0;
    return 1;
}

/* @zoombi32 0x00478708 */
short setChannelMaps(unsigned short id, unsigned short *channels)
{
    int i;
    short error;

    for (i = 0; i < 16; i++)
        if ((error = setChannelMap(id, i, channels[i])) != 0)
            return error;
    return 0;
}

/* @zoombi32 0x0047873c */
short setMutedChannels(unsigned short id, short *muted)
{
    int i;
    short error;

    for (i = 0; i < 16; i++)
        if ((error = setChannelMuted(id, i, muted[i])) != 0)
            return error;
    return 0;
}

/* Changes a map's velocity curve (0: none, resetting its channels). */
/* @zoombi32 0x00478770 */
short setMidiMapTable(long handle, long table)
{
    MidiMap *map;
    int channel;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    if (table < 0)
        return MMSYSERR_INVALPARAM;
    if (table != map->table) {
        if (!table)
            for (channel = 0; channel < 16; channel++)
                resetChannel(map, channel);
        map->table = table;
        fillVelocities(map, map->table);
    }
    return 0;
}

/* @zoombi32 0x004787d1 */
short midiMapUnprepareHeader(long handle, MidiHeader *header, unsigned short size)
{
    MidiMap *map;

    if ((map = midiMap(handle)) == 0)
        return MMSYSERR_INVALHANDLE;
    if (map->minimal)
        return MMSYSERR_NOTSUPPORTED;
    return midiOutUnprepareHeader(map->device->out, header, size);
}
