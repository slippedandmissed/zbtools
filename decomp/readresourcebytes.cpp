/*
 * readresourcebytes (Mohawk engine): readResourceBytes
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Reads up to *size bytes of resource `id`, from `offset`, into `buffer`
   (from its handle if it's loaded, else from its file); *size is set to
   the number read. */
/* @zoombi32 0x00491598 */
short readResourceBytes(long id, void *buffer, unsigned long *size, unsigned long offset)
{
    unsigned long wanted = *size;
    ResourceMap *map;
    FileTableEntry *entry;
    short error;

    *size = 0;
    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    lockHandle(id);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    error = 0;
    if (entry->flags & RESOURCE_LOADED) {
        unsigned long available = handleSize(entry->handle);
        if (wanted + offset <= available)
            *size = wanted;
        else {
            error = 0x28a0;
            *size = offset > available ? 0 : available - offset;
        }
        moveMemory(buffer, (char *)handleData(entry->handle) + offset, *size);
    } else {
        unsigned long available = makeLong(entry->sizeLow, entry->sizeHigh);
        if (wanted + offset <= available)
            *size = wanted;
        else {
            error = 0x28a0;
            *size = offset > available ? 0 : available - offset;
        }
        if (seekFile(map->file, offset += entry->offset, 0) == 0xffffffff
            || readFile(map->file, buffer, (long *)size))
            error = fileError();
    }
    unlockFile(map->file);
    unlockHandle(id);
    return setResourceError(error);
}
