/*
 * module_4927dc (Mohawk engine): writeResource
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Writes a modified resource's data back to its file, and the file's
   directory with it. */
/* @zoombi32 0x004927dc */
short writeResource(long id)
{
    ResourceMap *map;
    FileTableEntry *entry;
    unsigned long size;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    lockHandle(id);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    if (entry->flags & RESOURCE_MODIFIED) {
        if (entry->flags & RESOURCE_LOADED) {
            size = handleSize(entry->handle);
            writeResourceData(id, lockHandle(entry->handle), size, 0);
            unlockHandle(entry->handle);
            if (resources.error)
                goto done;
            entry->sizeLow = lowWord(size);
            entry->sizeHigh = highWord(size);
        }
        if (!writeMapHeader(map)) {
            entry->flags &= ~RESOURCE_MODIFIED;
            map->modified--;
            map->dirty = 0;
        }
    }
done:
    unlockFile(map->file);
    unlockHandle(id);
    return resources.error;
}
