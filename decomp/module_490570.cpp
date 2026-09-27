/*
 * module_490570 (Mohawk engine): purgeResource
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The memory manager's purge procedure while resources are open: a
   resource's handle can be purged (its data is read again when needed)
   once any changes are written back. Other handles go to the previous
   procedure. */
/* @zoombi32 0x00490570 */
short purgeResource(short handle, short purpose)
{
    ResourceMap *map;
    FileTableEntry *entry;
    long id;

    if (handleState(handle) == resources.handleState && (id = findResourceByHandle(handle)) != 0) {
        findEntry(id, &map, &entry);
        lockHandle(id);
        if (entry->flags & RESOURCE_MODIFIED && (purpose || writeResource(id))) {
            unlockHandle(id);
            return 0;
        }
        entry->flags &= ~RESOURCE_LOADED;
        entry->flags |= RESOURCE_PURGED;
        unlockHandle(id);
        return 1;
    }
    if (resources.previousPurgeProc)
        return resources.previousPurgeProc(handle, purpose);
    return 1;
}
