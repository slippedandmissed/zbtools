/*
 * module_4928e4 (Mohawk engine): writeResourceMap, makeResourceId
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Writes a map's modified resources back to its file, then its directory. */
/* @zoombi32 0x004928e4 */
short writeResourceMap(long handle)
{
    ResourceMap *map;
    unsigned short index;
    FileTableEntry *entry;
    unsigned long size;
    short error;

    if ((map = resourceMap(handle)) == 0)
        return setResourceError(0x28d4);
    lockHandle(handle);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    resources.error = 0;
    if (map->dirty || map->modified) {
        for (index = 1; map->modified && index <= map->fileTable.count; index++) {
            entry = &map->fileTable.entries[index - 1];
            if (!(entry->flags & RESOURCE_MODIFIED) || !(entry->flags & RESOURCE_LOADED))
                continue;
            size = handleSize(entry->handle);
            writeResourceData(makeResourceId(handle, index), lockHandle(entry->handle), size, 0);
            unlockHandle(entry->handle);
            if (resources.error)
                break;
            entry->sizeLow = lowWord(size);
            entry->sizeHigh = highWord(size);
            entry->flags &= ~RESOURCE_MODIFIED;
            map->modified--;
        }
        error = resources.error;
        if (!writeMapHeader(map)) {
            map->dirty = 0;
            resources.error = error;
        }
    }
    unlockFile(map->file);
    unlockHandle(handle);
    return resources.error;
}

/* @zoombi32 0x00492a25 */
long makeResourceId(short map, unsigned short index)
{
    return ((unsigned long)index << 16) + (unsigned short)map;
}
