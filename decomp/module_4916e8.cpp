/*
 * module_4916e8 (Mohawk engine): the resource manager's set-up, its files'
 * directories and headers, and byte-swapping
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_threads.h"

ResourceState resources;

/* Records `handle` as the resource's data (0: none). */
/* @zoombi32 0x004916e8 */
void attachHandle(FileTableEntry *entry, short handle)
{
    if ((entry->handle = handle) != 0) {
        entry->flags |= RESOURCE_LOADED;
        entry->flags &= ~RESOURCE_PURGED;
        setHandleState(handle, resources.handleState);
        setPurgeable(handle, (entry->flags & RESOURCE_PURGEABLE) != 0);
    } else
        entry->flags &= ~(RESOURCE_LOADED | RESOURCE_PURGED);
}

/* @zoombi32 0x0049172e */
void byteSwapHeader(MohawkHeader *header)
{
    header->tag = byteSwapLong(header->tag);
    header->size = byteSwapLong(header->size);
    header->type = byteSwapLong(header->type);
    header->version = byteSwapShort(header->version);
    header->compacted = byteSwapShort(header->compacted);
    header->fileSize = byteSwapLong(header->fileSize);
    header->directoryOffset = byteSwapLong(header->directoryOffset);
    header->fileTableOffset = byteSwapShort(header->fileTableOffset);
    header->fileTableSize = byteSwapShort(header->fileTableSize);
}

/* Copies `length` bytes of a map's file from `from` to `to`, through the
   resource manager's buffer (or, if that can't grow, one on the stack). */
/* Not exact: the original keeps `purgeEnabled` in esi (later `done`'s
   register); BCC32 4.5 puts it on the stack. */
/* @zoombi32 0x004917b0 */
short copyFileBytes(ResourceMap *map, unsigned long to, unsigned long from, unsigned long length)
{
    short error = 0;
    char *buffer;
    unsigned long bufferSize;
    GrowProc growProc;
    unsigned long chunk;
    char local[0x400];
    short purgeEnabled;
    unsigned long done;

    if (length) {
        if (lockFile(map->file, -1))
            return setResourceError(fileError());
        bufferSize = handleSize(resources.buffer);
        purgeEnabled = setPurgeEnabled(0);
        growProc = setGrowProc(0);
        if (length <= bufferSize || !handleLocks(resources.buffer)
            || !setHandleSize(resources.buffer, length) || bufferSize > 0x400) {
            buffer = (char *)lockHandle(resources.buffer);
            bufferSize = handleSize(resources.buffer);
        } else {
            buffer = local;
            bufferSize = 0x400;
        }
        setGrowProc(growProc);
        setPurgeEnabled(purgeEnabled);
        for (done = 0; length; ) {
            chunk = bufferSize > length ? length : bufferSize;
            if (seekFile(map->file, from + done, 0) == 0xffffffff
                || readFile(map->file, buffer, (long *)&chunk)
                || seekFile(map->file, to + done, 0) == 0xffffffff
                || writeFile(map->file, buffer, (long *)&chunk)) {
                error = fileError();
                break;
            }
            done += chunk;
            length -= chunk;
            if (to + done > map->fileSize) {
                map->fileSize = to + done;
                map->dirty = 1;
            }
        }
        if (buffer != local)
            unlockHandle(resources.buffer);
        unlockFile(map->file);
    }
    return setResourceError(error);
}

/* Removes the resource at file-table `index` from a directory: its entries in
   its type's resource and name tables, its name, and the type if that leaves
   it empty. */
/* @zoombi32 0x0049194e */
short removeFromDirectory(short handle, unsigned short index)
{
    Directory *directory = (Directory *)handleData(handle);
    unsigned short type;
    unsigned short size;
    unsigned short name;
    unsigned short nameLength;
    ResourceTable *table;
    NameTable *names;
    unsigned short i;
    unsigned short t;
    unsigned short start;

    for (type = 0; type < directory->count; type++) {
        table = (ResourceTable *)((char *)directory + directory->types[type].resources);
        for (i = 0; i < table->count; i++)
            if (table->entries[i].index == index)
                goto found;
    }
    return setResourceError(0x28d2);

found:
    size = handleSize(handle);
    memcpy(&table->entries[i], &table->entries[i + 1],
           size - (unsigned short)((char *)&table->entries[i + 1] - (char *)directory));
    size -= sizeof(ResourceRef);
    directory->names -= sizeof(ResourceRef);
    table->count--;
    for (t = 0; t < directory->count; t++) {
        if (directory->types[t].resources > directory->types[type].resources) {
            directory->types[t].resources -= sizeof(ResourceRef);
            directory->types[t].names -= sizeof(ResourceRef);
        } else if (directory->types[t].resources == directory->types[type].resources)
            directory->types[t].names -= sizeof(ResourceRef);
    }
    /* The loops over each type's names below reuse `names` and `i`, so the
       search goes on from wherever they left off. */
    names = (NameTable *)((char *)directory + directory->types[type].names);
    for (i = 0; i < names->count; i++) {
        if (names->entries[i].index != index)
            continue;
        name = names->entries[i].name;
        nameLength = strlen((char *)directory + directory->names + name) + 1;
        memcpy((char *)directory + directory->names + name,
               (char *)directory + directory->names + name + nameLength,
               size - (directory->names + name + nameLength));
        size -= nameLength;
        memcpy(&names->entries[i], &names->entries[i + 1],
               size - (unsigned short)((char *)&names->entries[i + 1] - (char *)directory));
        size -= sizeof(ResourceRef);
        directory->names -= sizeof(ResourceRef);
        names->count--;
        for (t = 0; t < directory->count; t++) {
            if (directory->types[t].resources > directory->types[type].names) {
                directory->types[t].resources -= sizeof(ResourceRef);
                directory->types[t].names -= sizeof(ResourceRef);
            }
            names = (NameTable *)((char *)directory + directory->types[t].names);
            for (i = 0; i < names->count; i++)
                if (names->entries[i].name > name)
                    names->entries[i].name -= nameLength;
        }
    }
    if (!((ResourceTable *)((char *)directory + directory->types[type].resources))->count) {
        directory->names -= 12u;
        start = directory->types[type].resources;
        memcpy((char *)directory + start, (char *)directory + start + 4, size - (start + 4));
        size -= sizeof(ResourceRef);
        directory->count--;
        memcpy(&directory->types[type], &directory->types[type + 1],
               size - (unsigned short)((char *)&directory->types[type + 1] - (char *)directory));
        size -= sizeof(TypeEntry);
        for (t = 0; t < directory->count; t++) {
            if (start < directory->types[t].resources) {
                directory->types[t].resources -= sizeof(ResourceRef);
                directory->types[t].names -= sizeof(ResourceRef);
            }
            directory->types[t].resources -= sizeof(TypeEntry);
            directory->types[t].names -= sizeof(TypeEntry);
        }
    }
    return setHandleSize(handle, size);
}

/* Adds resource `id` of type `type`, at file-table `index`, to a directory,
   named `name` (up to 31 characters; none if 0). The type table and each
   type's tables are sorted, and searched by bisection. */
/* @zoombi32 0x00491c64 */
short addToDirectory(short handle, unsigned short index, unsigned long type, unsigned short id,
                     const char *name)
{
    ResourceTable *table;
    NameTable *names;
    unsigned short resourcesHigh;
    unsigned short namesLow;
    unsigned short namesHigh;
    unsigned short mid;
    unsigned short low;
    unsigned short nameLength;
    unsigned short size;
    Directory *directory;
    unsigned short resource;
    unsigned short nameIndex;
    unsigned short high;
    unsigned short resourcesLow;
    unsigned short t;
    int order;

    nameLength = name ? (strlen(name) < 31 ? strlen(name) : 31) : 0;
    directory = (Directory *)handleData(handle);
    for (low = 0, high = directory->count + 1; (mid = (low + high) / 2) != low; ) {
        if (directory->types[mid - 1].type < type)
            low = mid;
        else if (directory->types[mid - 1].type > type)
            high = mid;
        else {
            table = (ResourceTable *)((char *)directory + directory->types[mid - 1].resources);
            for (resourcesLow = 0, resourcesHigh = table->count + 1;
                 (resource = (resourcesLow + resourcesHigh) / 2) != resourcesLow; ) {
                if (table->entries[resource - 1].id < id)
                    resourcesLow = resource;
                else if (table->entries[resource - 1].id > id)
                    resourcesHigh = resource;
                else
                    return setResourceError(0x28a1);
            }
            if (nameLength) {
                names = (NameTable *)((char *)directory + directory->types[mid - 1].names);
                for (namesLow = 0, namesHigh = names->count + 1;
                     (nameIndex = (namesLow + namesHigh) / 2) != namesLow; ) {
                    order = memicmp((char *)directory + directory->names
                                        + names->entries[nameIndex - 1].name,
                                    name, nameLength + 1);
                    if (order < 0)
                        namesLow = nameIndex;
                    else if (order > 0)
                        namesHigh = nameIndex;
                    else
                        return setResourceError(0x28a1);
                }
            }
            break;
        }
    }
    size = handleSize(handle);
    if (setHandleSize(handle, size + (mid == low ? 12 : 0) + (nameLength ? nameLength + 5 : 0) + 4))
        return setResourceError(memError());
    directory = (Directory *)handleData(handle);
    if (mid == low) {
        memmove(&directory->types[mid + 1], &directory->types[mid],
                size - (unsigned short)((char *)&directory->types[mid] - (char *)directory));
        size += 8;
        directory->names += 8;
        directory->count++;
        directory->types[mid].type = type;
        for (t = 0; t < directory->count; t++) {
            directory->types[t].resources += 8;
            directory->types[t].names += 8;
        }
        memmove((char *)directory + directory->names + 4, (char *)directory + directory->names,
                size - directory->names);
        size += 4;
        directory->types[mid].resources = directory->names;
        directory->types[mid].names = directory->names + 2;
        directory->names += 4;
        table = (ResourceTable *)((char *)directory + directory->types[mid].resources);
        table->count = 0;
        resource = 0;
        names = (NameTable *)((char *)directory + directory->types[mid].names);
        names->count = 0;
        nameIndex = 0;
    } else {
        mid--;
        table = (ResourceTable *)((char *)directory + directory->types[mid].resources);
    }
    memmove(&table->entries[resource + 1], &table->entries[resource],
            size - (unsigned short)((char *)&table->entries[resource] - (char *)directory));
    size += 4;
    directory->names += 4;
    table->count++;
    table->entries[resource].id = id;
    table->entries[resource].index = index;
    for (t = 0; t < directory->count; t++) {
        if (directory->types[t].resources > directory->types[mid].resources) {
            directory->types[t].resources += 4;
            directory->types[t].names += 4;
        } else if (directory->types[t].resources == directory->types[mid].resources)
            directory->types[t].names += 4;
    }
    if (nameLength) {
        names = (NameTable *)((char *)directory + directory->types[mid].names);
        memmove(&names->entries[nameIndex + 1], &names->entries[nameIndex],
                size - (unsigned short)((char *)&names->entries[nameIndex] - (char *)directory));
        size += 4;
        directory->names += 4;
        names->count++;
        names->entries[nameIndex].index = index;
        names->entries[nameIndex].name = size - directory->names;
        for (t = 0; t < directory->count; t++) {
            if (directory->types[t].resources > directory->types[mid].names) {
                directory->types[t].resources += 4;
                directory->types[t].names += 4;
            }
        }
        memcpy((char *)directory + size, name, nameLength);
        ((char *)directory + size)[nameLength] = 0;
    }
    return setResourceError(0);
}

/* Byte-swaps a directory, to native order (`fromFile`) or back. */
/* @zoombi32 0x0049210b */
void byteSwapDirectory(Directory *directory, short fromFile)
{
    unsigned short count;
    unsigned short t;
    NameTable *names;
    unsigned short resourceCount;
    TypeEntry *entry;
    ResourceTable *table;
    unsigned short i;
    ResourceRef *ref;
    unsigned short nameCount;
    unsigned short j;
    NameRef *nameRef;

    count = fromFile ? byteSwapShort(directory->count) : directory->count;
    directory->names = byteSwapShort(directory->names);
    directory->count = byteSwapShort(directory->count);
    for (t = 0; t < count; t++) {
        entry = &directory->types[t];
        table = (ResourceTable *)((char *)directory
                                  + (fromFile ? byteSwapShort(entry->resources) : entry->resources));
        names = (NameTable *)((char *)directory
                              + (fromFile ? byteSwapShort(entry->names) : entry->names));
        entry->type = byteSwapLong(entry->type);
        entry->resources = byteSwapShort(entry->resources);
        entry->names = byteSwapShort(entry->names);
        resourceCount = fromFile ? byteSwapShort(table->count) : table->count;
        table->count = byteSwapShort(table->count);
        for (i = 0; i < resourceCount; i++) {
            ref = &table->entries[i];
            ref->id = byteSwapShort(ref->id);
            ref->index = byteSwapShort(ref->index);
        }
        nameCount = fromFile ? byteSwapShort(names->count) : names->count;
        names->count = byteSwapShort(names->count);
        for (j = 0; j < nameCount; j++) {
            nameRef = &names->entries[j];
            nameRef->name = byteSwapShort(nameRef->name);
            nameRef->index = byteSwapShort(nameRef->index);
        }
    }
}

/* Starts the resource manager: its buffer, its purge procedure and the
   system archive (SYSTEM.W32, else SYSTEM.MHK, in the program's
   directory). */
/* @zoombi32 0x004922c6 */
short initResources()
{
    memset(&resources, 0, sizeof resources);
    getIniBool(0, "Resource", "fShareReadOnly", &resources.shareReadOnly);
    resources.buffer = newHandle(0);
    if (!resources.buffer)
        return setResourceError(memError());
    setPurgeable(resources.buffer, 1);
    resources.handleState = countHeapUp();
    if (!resources.handleState) {
        setResourceError(memError());
        disposeHandle(resources.buffer);
        return resources.error;
    }
    resources.previousPurgeProc = setPurgeProc(purgeResource);
    resources.ready = 1;
    fileSpec directory;
    programDirectory(&directory);
    resources.systemMap = openResourceFile(fileSpec(directory, "SYSTEM.W32"), 0);
    if (!resources.systemMap)
        resources.systemMap = openResourceFile(fileSpec(directory, "SYSTEM.MHK"), 0);
    return setResourceError(0);
}

/* Byte-swaps a file table, to native order (`fromFile`) or back. */
/* @zoombi32 0x004923f3 */
void byteSwapFileTable(FileTable *table, short fromFile)
{
    unsigned short count;
    unsigned short i;
    FileTableEntry *entry;

    count = fromFile ? byteSwapShort(table->count) : table->count;
    table->countHigh = byteSwapShort(table->countHigh);
    table->count = byteSwapShort(table->count);
    for (i = 0; i < count; i++) {
        entry = &table->entries[i];
        entry->offset = byteSwapLong(entry->offset);
        entry->sizeLow = byteSwapShort(entry->sizeLow);
        entry->handle = byteSwapShort(entry->handle);
    }
}

/* Writes a map's header, directory and file table to its file: in place if
   they fit, else at the end of the file. */
/* @zoombi32 0x00492483 */
short writeMapHeader(ResourceMap *map)
{
    unsigned short directorySize;
    unsigned short tableSize;
    unsigned short compacted;
    unsigned long total;
    unsigned long size;
    long saved;
    Directory *directory;
    MohawkHeader header;
    unsigned long end;
    unsigned long offset;
    short error;

    if (map->readOnly)
        return setResourceError(0x28a6);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    directorySize = handleSize(map->directory);
    tableSize = (char *)&map->fileTable.entries[map->fileTable.count] - (char *)&map->fileTable;
    compacted = map->compacted;
    end = map->fileSize;
    offset = map->directoryOffset;
    total = directorySize + tableSize;
    if (total > map->directorySize) {
        if (!map->directorySize || map->directoryOffset + map->directorySize != map->fileSize) {
            offset = end;
            compacted = 1;
        }
        end = total + offset;
    }
    header.tag = 0x4d48574b;
    header.size = end - 8;
    header.type = 0x52535243;
    header.version = 0x100;
    header.compacted = compacted;
    header.fileSize = end;
    header.directoryOffset = offset;
    header.fileTableOffset = directorySize;
    header.fileTableSize = tableSize;
    byteSwapHeader(&header);
    size = sizeof header;
    if (setFileSize(map->file, end) || seekFile(map->file, 0, 0) == 0xffffffff
        || writeFile(map->file, &header, (long *)&size)) {
        unlockFile(map->file);
        return fileError();
    }
    saved = setAskUser(0);
    directory = (Directory *)lockHandle(map->directory);
    byteSwapDirectory(directory, 0);
    byteSwapFileTable(&map->fileTable, 0);
    if (seekFile(map->file, offset, 0) == 0xffffffff
        || writeFile(map->file, directory, (size = directorySize, (long *)&size))
        || writeFile(map->file, &map->fileTable, (size = tableSize, (long *)&size)))
        error = fileError();
    else {
        map->compacted = compacted;
        map->fileSize = end;
        map->directoryOffset = offset;
        map->directorySize = total;
        error = 0;
    }
    byteSwapFileTable(&map->fileTable, 1);
    byteSwapDirectory(directory, 1);
    unlockHandle(map->directory);
    setAskUser(saved);
    unlockFile(map->file);
    return setResourceError(error);
}

/* @zoombi32 0x0049268a */
short resourceBufferSize()
{
    return resources.ready ? 0x500 : 0;
}

/* Closes every archive and stops the resource manager. */
/* @zoombi32 0x0049269e */
void closeResources()
{
    disableScheduling();
    if (resources.systemMap)
        closeResourceFile(resources.systemMap, 0, 1);
    while (resources.maps) {
        setResourceError(0x28d2);
        closeResourceFile((unsigned short)resources.maps, 0, 1);
    }
    enableScheduling();
    setPurgeProc(resources.previousPurgeProc);
    disposeHandle(resources.buffer);
    resources.ready = 0;
}

/* @zoombi32 0x004926ff */
unsigned long __cdecl byteSwapLong(unsigned long value)
{
    unsigned char *bytes = (unsigned char *)&value;
    return (bytes[3] | (unsigned short)bytes[2] << 8)
           | (unsigned long)(bytes[1] | (unsigned short)bytes[0] << 8) << 16;
}

/* @zoombi32 0x00492730 */
unsigned short __cdecl byteSwapShort(unsigned short value)
{
    unsigned char *bytes = (unsigned char *)&value;
    return (unsigned short)(bytes[1] | bytes[0] << 8);
}

/* Makes a resource purgeable or not; whether it was, or 0xffffffff on
   error (a locked resource can't be changed). */
/* @zoombi32 0x00492748 */
unsigned short setResourcePurgeable(long id, short purgeable)
{
    ResourceMap *map;
    FileTableEntry *entry;
    short was;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0xffff;
    }
    if (entry->flags & RESOURCE_LOCKED) {
        setResourceError(0x28a5);
        return 0xffff;
    }
    was = (entry->flags & RESOURCE_PURGEABLE) != 0;
    if (purgeable)
        entry->flags |= RESOURCE_PURGEABLE;
    else
        entry->flags &= ~RESOURCE_PURGEABLE;
    if (entry->flags & (RESOURCE_LOADED | RESOURCE_PURGED))
        setPurgeable(entry->handle, purgeable);
    setResourceError(0);
    return was;
}
