/*
 * findresourcebyhandle (Mohawk engine): findResourceByHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The ID of the resource whose data is in `handle`; 0 if none. */
/* @zoombi32 0x0048fd68 */
long findResourceByHandle(short handle)
{
    short mapHandle;
    ResourceMap *map;
    unsigned short index;
    FileTableEntry *entry;

    for (mapHandle = resources.maps; mapHandle; mapHandle = map->next) {
        map = (ResourceMap *)handleData(mapHandle);
        for (index = 1; index <= map->fileTable.count; index++) {
            entry = &map->fileTable.entries[index - 1];
            if (!(entry->flags & RESOURCE_DELETED)
                && (entry->flags & (RESOURCE_LOADED | RESOURCE_PURGED))
                && handle == entry->handle)
                return makeResourceId(mapHandle, index);
        }
    }
    setResourceError(0x28a3);
    return 0;
}
