/*
 * openresourcefile (Mohawk engine): resourceHandle, openResourceFile
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "e2memory.h"

/* The handle holding a resource's data; 0 if it isn't loaded. */
/* @zoombi32 0x00490140 */
unsigned short resourceHandle(long id)
{
    ResourceMap *map;
    FileTableEntry *entry;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0xffff;
    }
    setResourceError(0);
    if (entry->flags & RESOURCE_LOADED)
        return entry->handle;
    return 0;
}

/* Opens a Mohawk archive (again, if it's open already), reading its
   directory and file table and starting its preloads; its map's handle, or
   0 on error. If it can't be opened for writing, it's opened read-only. */
/* @zoombi32 0x0049018c */
long openResourceFile(const fileSpec &file, short readOnly)
{
    short handle;
    ResourceMap *map;
    short error;
    short directory;
    unsigned long size;
    short async;
    unsigned short index;
    unsigned short countsSize;
    short mode;

    for (handle = resources.maps; handle; ) {
        fileSpec spec;
        map = (ResourceMap *)lockHandle(handle);
        fileSpecOf(map->file, &spec);
        if (!file.compare(spec)) {
            map->users++;
            unlockHandle(handle);
            setResourceError(0);
            return (unsigned short)handle;
        }
        unlockHandle(handle);
        handle = map->next;
    }
    long f;
    long volume;
    MohawkHeader header;
    VolumeInfo info;
    if (readOnly) {
    openReadOnly:
        mode = resources.shareReadOnly ? 0x10 : 0x40;
        mode |= 1;
        if ((f = openFile((fileSpec *)&file, mode)) != 0)
            readOnly = 1;
        else {
            setResourceError(fileError());
            return 0;
        }
    } else if ((f = openFile((fileSpec *)&file, 0x43)) == 0) {
        switch (error = fileError()) {
        case 0x283c:
            goto openReadOnly;
        }
        setResourceError(error);
        return 0;
    }
    if ((error = file.volume(&volume)) != 0 || (error = volumeInfo(volume, &info)) != 0) {
        setResourceError(error);
        goto close;
    }
    async = info.async;
    readOnly = readOnly || info.readOnly;
    size = sizeof(MohawkHeader);
    if ((error = readFile(f, &header, (long *)&size)) != 0) {
        setResourceError(error == 0x283f ? 0x28a4 : error);
        goto close;
    }
    byteSwapHeader(&header);
    if (header.tag != 0x4d48574b || header.type != 0x52535243 || header.version != 0x100
        || fileLength(f) != header.fileSize) {
        setResourceError(0x28a4);
        goto close;
    }
    if ((directory = newHandle(header.fileTableOffset)) == 0) {
        setResourceError(memError());
        goto close;
    }
    seekFile(f, header.directoryOffset, 0);
    size = header.fileTableOffset;
    error = readFile(f, lockHandle(directory), (long *)&size);
    unlockHandle(directory);
    if (error) {
        setResourceError(error);
        goto disposeDirectory;
    }
    if ((handle = newHandle(header.fileTableSize + 0x3c)) == 0) {
        setResourceError(memError());
        goto disposeDirectory;
    }
    map = (ResourceMap *)lockHandle(handle);
    size = header.fileTableSize;
    if ((error = readFile(f, &map->fileTable, (long *)&size)) != 0) {
        setResourceError(error);
    disposeMap:
        unlockHandle(handle);
        disposeHandle(handle);
    disposeDirectory:
        disposeHandle(directory);
    close:
        closeFile(f, 0);
        return 0;
    }
    memset(map, 0, 0x3c);
    map->tag = 0x524d6170;
    map->users = 1;
    map->file = f;
    map->async = async;
    map->readOnly = readOnly;
    map->directory = directory;
    map->compacted = header.compacted;
    map->fileSize = header.fileSize;
    map->directoryOffset = header.directoryOffset;
    map->directorySize = (unsigned long)header.fileTableOffset + header.fileTableSize;
    byteSwapDirectory((Directory *)handleData(map->directory), 1);
    byteSwapFileTable(&map->fileTable, 1);
    countsSize = map->fileTable.count * 2;
    if ((map->counters = newHandle(countsSize)) != 0)
        memset(handleData(map->counters), 0, countsSize);
    else {
        setResourceError(memError());
        goto disposeMap;
    }
    map->prev = 0;
    if ((map->next = resources.maps) != 0)
        ((ResourceMap *)handleData(map->next))->prev = handle;
    resources.maps = handle;
    resources.currentMap = handle;
    for (index = 1; index <= map->fileTable.count; index++) {
        FileTableEntry *entry = &map->fileTable.entries[index - 1];
        entry->flags &= ~(RESOURCE_MODIFIED | RESOURCE_LOADED | RESOURCE_PURGED);
        if (entry->flags & RESOURCE_PRELOAD)
            startPreload(makeResourceId(handle, index), 0, 0);
    }
    runMapPreloads((unsigned short)handle, 0xffffffff);
    unlockHandle(handle);
    setResourceError(0);
    return (unsigned short)handle;
}
