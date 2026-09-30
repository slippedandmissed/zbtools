/*
 * resourcesize (Mohawk engine): resourceSize
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A resource's size (its data's, once loaded); 0xffffffff on error. */
/* @zoombi32 0x0048ff28 */
unsigned long resourceSize(long id)
{
    ResourceMap *map;
    FileTableEntry *entry;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0xffffffff;
    }
    setResourceError(0);
    if (entry->flags & RESOURCE_LOADED)
        return handleSize(entry->handle);
    return makeLong(entry->sizeLow, entry->sizeHigh);
}
