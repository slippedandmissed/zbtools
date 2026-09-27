/*
 * module_48fec0 (Mohawk engine): getResourceInfo
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Where resource `id`'s data is: its file, offset and size. */
/* @zoombi32 0x0048fec0 */
short getResourceInfo(long id, long *file, unsigned long *offset, unsigned long *size)
{
    ResourceMap *map;
    FileTableEntry *entry;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    *file = map->file;
    *offset = entry->offset;
    *size = makeLong(entry->sizeLow, entry->sizeHigh);
    return setResourceError(0);
}
