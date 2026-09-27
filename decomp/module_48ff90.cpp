/*
 * module_48ff90 (Mohawk engine): loadResource
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A handle holding resource `id`'s data, reading it if it isn't loaded;
   0 on error. `use` counts a user (releaseResource(id, 1) uncounts it). */
/* Not exact: `id` and `handle` swap registers (ebx and esi). */
/* @zoombi32 0x0048ff90 */
short loadResource(long id, short use)
{
    ResourceMap *map;
    FileTableEntry *entry;
    unsigned long size;
    short handle;
    unsigned char *counts;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0;
    }
    if (entry->flags & RESOURCE_LOADED)
        handle = entry->handle;
    else {
        lockHandle(id);
        if (lockFile(map->file, -1))
            return setResourceError(fileError());
        size = makeLong(entry->sizeLow, entry->sizeHigh);
        if (entry->flags & RESOURCE_PURGED) {
            handle = entry->handle;
            if (setHandleSize(handle, size)) {
                setResourceError(memError());
            fail:
                unlockFile(map->file);
                unlockHandle(id);
                return 0;
            }
        } else if ((handle = newHandle(size)) == 0) {
            setResourceError(memError());
            goto fail;
        }
        if (size) {
            readResourceBytes(id, lockHandle(handle), &size, 0);
            unlockHandle(handle);
            if (resources.error) {
                if (entry->flags & RESOURCE_PURGED)
                    setHandleSize(handle, 0);
                else
                    disposeResourceHandle(handle);
                goto fail;
            }
        }
        attachHandle(entry, handle);
        if (resourceCounts(map, resourceIndex(id))[1] > 0)
            finishPreloads(id);
        unlockFile(map->file);
        unlockHandle(id);
    }
    if (use) {
        counts = resourceCounts(map, resourceIndex(id));
        if (*counts < 0xff) {
            ++*counts;
            setResourceError(0);
        } else
            setResourceError(0x28d6);
    } else
        setResourceError(0);
    return resources.error ? 0 : handle;
}
