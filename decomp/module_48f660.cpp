/*
 * module_48f660 (Mohawk engine): closeResourceFile, findResource
 */

/* @flags -p -x- */

#include <stdlib.h>

#include "zoombinis.h"
#include "e2memory.h"

/* Closes an archive once its last user does: writes back what changed,
   cancels its preloads, frees its resources' data and, if `compact`, first
   squeezes out deleted resources and unused space. Without `force` it stops
   at the first problem (a resource in use or locked); with it, it carries
   on and returns the last error. */
/* @zoombi32 0x0048f660 */
short closeResourceFile(long handle, short compact, short force)
{
    ResourceMap *map;
    short error;
    unsigned short index;
    Directory *copy;
    FileTableEntry *next;
    Directory *directory;
    ResourceTable *copyTable;
    FileTableEntry *entry;
    short result;
    unsigned short deleted;
    unsigned short old;
    unsigned short t;
    ResourceTable *table;
    unsigned short i;
    unsigned long position;
    unsigned long size;

    map = resourceMap(handle);
    if (!map)
        return setResourceError(0x28d4);
    if (map->unknown3A > 0)
        return setResourceError(0x28d2);
    error = 0;
    if (map->users > 1) {
        map->users--;
        goto done;
    }
    lockHandle(handle);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    if (writeResourceMap(handle)) {
        if (force)
            error = resources.error;
        else {
        fail:
            unlockFile(map->file);
            unlockHandle(handle);
            return resources.error;
        }
    }
    while (map->preloads > 0) {
        map->preload->busy = 0;
        disposePreload(map->preload);
    }
    for (index = 1; index <= map->fileTable.count; index++) {
        entry = &map->fileTable.entries[index - 1];
        if (!(entry->flags & RESOURCE_DELETED) && entry->flags & (RESOURCE_LOADED | RESOURCE_PURGED)) {
            if (*resourceCounts(map, index) > 0) {
                if (force)
                    error = 0x28d6;
                else {
                    resources.error = 0x28d6;
                    goto fail;
                }
            }
            unsigned short locks = handleLocks(entry->handle);
            if (locks != 0xffff && locks > 0) {
                if (force) {
                    setHandleLocks(entry->handle, 0);
                    error = 0x27a8;
                } else {
                    resources.error = 0x27a8;
                    goto fail;
                }
            }
            if ((result = disposeResourceHandle(entry->handle)) != 0) {
                if (force)
                    error = result;
                else {
                    resources.error = result;
                    goto fail;
                }
            }
            entry->flags &= ~(RESOURCE_LOADED | RESOURCE_PURGED);
        }
    }
    if (compact && map->compacted) {
        if ((copy = (Directory *)newPtr(((Directory *)handleData(map->directory))->names)) != 0) {
            /* Sort the file table by position, deleted resources last (each
               entry noting its old index), and drop the deleted ones. */
            deleted = 0;
            for (index = 1, next = map->fileTable.entries; index <= map->fileTable.count;
                 index++, next++) {
                next->handle = index;
                if (next->flags & RESOURCE_DELETED)
                    deleted++;
            }
            qsort(map->fileTable.entries, map->fileTable.count, sizeof(FileTableEntry),
                  compareEntries);
            map->fileTable.count -= deleted;
            map->fileTable.countHigh = 0;
            /* Renumber the directory's references (going by a copy, so each
               is renumbered once). */
            directory = (Directory *)handleData(map->directory);
            memcpy(copy, directory, directory->names);
            for (index = 1, next = map->fileTable.entries; index <= map->fileTable.count;
                 index++, next++) {
                old = next->handle;
                next->handle = 0;
                if (old == index)
                    continue;
                for (t = 0; t < directory->count; t++) {
                    table = (ResourceTable *)((char *)directory + directory->types[t].resources);
                    copyTable = (ResourceTable *)((char *)copy + directory->types[t].resources);
                    for (i = 0; i < table->count; i++) {
                        if (old == copyTable->entries[i].index) {
                            table->entries[i].index = index;
                            copyTable->entries[i].index = 0;
                            NameTable *names =
                                (NameTable *)((char *)directory + directory->types[t].names);
                            NameTable *copyNames =
                                (NameTable *)((char *)copy + directory->types[t].names);
                            for (i = 0; i < names->count; i++) {
                                if (old == copyNames->entries[i].index) {
                                    names->entries[i].index = index;
                                    copyNames->entries[i].index = 0;
                                    break;
                                }
                            }
                            t = directory->count;
                            break;
                        }
                    }
                }
            }
            disposePtr(copy);
            /* Move the data down over the gaps. */
            resources.error = 0;
            position = sizeof(MohawkHeader);
            for (index = 1; index <= map->fileTable.count; index++) {
                entry = &map->fileTable.entries[index - 1];
                size = makeLong(entry->sizeLow, entry->sizeHigh);
                if (position < entry->offset && !(entry->flags & RESOURCE_08)) {
                    if (copyFileBytes(map, position, entry->offset, size))
                        break;
                    entry->offset = position;
                }
                position = entry->offset + size;
            }
            if (resources.error)
                error = resources.error;
            else {
                map->fileSize = position;
                map->compacted = 0;
            }
            map->dirty = 1;
            map->directorySize = 0;
            if (writeMapHeader(map)) {
                if (!force)
                    goto fail;
                error = resources.error;
            }
        } else if (force)
            error = memError();
        else {
            setResourceError(memError());
            goto fail;
        }
    }
    unlockFile(map->file);
    if (closeFile(map->file, force)) {
        error = fileError();
        if (!force) {
            unlockHandle(handle);
            return setResourceError(error);
        }
    }
    map->tag = 0;
    if (map->prev)
        ((ResourceMap *)handleData(map->prev))->next = map->next;
    if (map->next)
        ((ResourceMap *)handleData(map->next))->prev = map->prev;
    if (resources.maps == (short)handle)
        resources.maps = map->next;
    if (resources.currentMap == (short)handle)
        resources.currentMap = map->next;
    disposeHandle(map->counters);
    disposeHandle(map->directory);
    setHandleLocks(handle, 0);
    disposeHandle(handle);
done:
    return resources.error = error;
}

/* Orders file-table entries by position, deleted ones last. */
/* Not exact: the original keeps `order` in ecx to a single exit and
   compares through ebx; BCC32 4.5 returns from each branch in eax. */
/* @zoombi32 0x0048fbf0 */
int __cdecl compareEntries(const void *a, const void *b)
{
    const FileTableEntry *x = (const FileTableEntry *)a;
    const FileTableEntry *y = (const FileTableEntry *)b;

    int order;

    if (!(x->flags & RESOURCE_DELETED) && !(y->flags & RESOURCE_DELETED))
        order = x->offset < y->offset ? -1 : x->offset > y->offset ? 1 : 0;
    else if (x->flags & RESOURCE_DELETED)
        order = y->flags & RESOURCE_DELETED ? 0 : 1;
    else
        order = -1;
    return order;
}

/* The ID of resource `id` of type `type` in map `map`, or if that's 0, in
   the current map or those opened before it; 0 if there's none. */
/* @zoombi32 0x0048fc38 */
long findResource(unsigned long type, unsigned short id, long map)
{
    short handle;
    ResourceMap *data;
    unsigned short high;
    unsigned short resourcesHigh;
    Directory *directory;
    unsigned short low;
    unsigned short mid;
    ResourceTable *table;
    unsigned short resourcesLow;
    unsigned short resource;

    for (handle = map ? map : resources.currentMap; handle; handle = map ? 0 : data->next) {
        data = (ResourceMap *)handleData(handle);
        directory = (Directory *)handleData(data->directory);
        for (low = 0, high = directory->count + 1; (mid = (low + high) / 2) != low; ) {
            if (directory->types[mid - 1].type < type)
                low = mid;
            else if (directory->types[mid - 1].type > type)
                high = mid;
            else {
                table = (ResourceTable *)((char *)directory + directory->types[mid - 1].resources);
                for (resourcesLow = 0, resourcesHigh = table->count + 1;
                     (resource = (resourcesLow + resourcesHigh) / 2) != resourcesLow; ) {
                    if (id > table->entries[resource - 1].id)
                        resourcesLow = resource;
                    else if (id < table->entries[resource - 1].id)
                        resourcesHigh = resource;
                    else {
                        setResourceError(0);
                        return makeResourceId(handle, table->entries[resource - 1].index);
                    }
                }
                break;
            }
        }
    }
    setResourceError(0x28a3);
    return 0;
}
