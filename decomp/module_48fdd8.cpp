/*
 * module_48fdd8 (Mohawk engine): releaseResource
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Releases a resource's data once nothing uses it; `release` counts one
   user less (loadResource counts them). */
/* @zoombi32 0x0048fdd8 */
short releaseResource(long id, short release)
{
    ResourceMap *map;
    FileTableEntry *entry;
    unsigned char *counts;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    counts = resourceCounts(map, resourceIndex(id));
    if (release && !*counts)
        return setResourceError(0x28d6);
    if ((entry->flags & (RESOURCE_LOADED | RESOURCE_PURGED))
        && (!*counts || release && *counts == 1)) {
        lockHandle(id);
        lockHandle(map->counters);
        if (!setResourceError(disposeResourceHandle(entry->handle)))
            entry->flags &= ~(RESOURCE_LOADED | RESOURCE_PURGED);
        unlockHandle(map->counters);
        unlockHandle(id);
    } else
        setResourceError(0);
    if (!resources.error && release)
        (*counts)--;
    return resources.error;
}
