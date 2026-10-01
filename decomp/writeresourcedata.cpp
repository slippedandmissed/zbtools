/*
 * writeresourcedata (Mohawk engine): writing resource data; resource IDs
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Writes `length` bytes of `buffer` into resource `id` at `offset` (0xffffffff:
   its end), growing it as needed; if it grows and isn't at the file's end, it
   moves there first. */
/* @zoombi32 0x00492a3c */
short writeResourceData(long id, const void *buffer, unsigned long length, unsigned long offset)
{
    ResourceMap *map;
    FileTableEntry *entry;
    unsigned long grow;
    unsigned long position;
    unsigned long size;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    lockHandle(id);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    position = entry->offset;
    size = makeLong(entry->sizeLow, entry->sizeHigh);
    if (offset == 0xffffffff)
        offset = size;
    grow = size < length + offset ? length + offset - size : 0;
    if (grow + size > 0xffffff) {
        setResourceError(0x28a0);
        goto done;
    }
    if (map->readOnly) {
        setResourceError(0x28a6);
        goto done;
    }
    if (entry->flags & RESOURCE_08) {
        setResourceError(0x28a2);
        goto done;
    }
    if (entry->flags & RESOURCE_LOCKED) {
        setResourceError(0x28a5);
        goto done;
    }
    if (grow || offset >= size) {
        if (entry->offset + size == map->fileSize)
            position = entry->offset;
        else if (map->directoryOffset + map->directorySize == map->fileSize) {
            position = map->directoryOffset;
            if (setFileSize(map->file, position)) {
                setResourceError(fileError());
                goto done;
            }
            map->fileSize = position;
            map->dirty = 1;
            map->directorySize = 0;
        } else
            position = map->fileSize;
        if (position != entry->offset
            && copyFileBytes(map, position, entry->offset, size < offset ? size : offset))
            goto done;
        if (position + offset > map->fileSize) {
            if (setFileSize(map->file, position + offset)) {
                setResourceError(fileError());
                goto done;
            }
            map->fileSize = position + offset;
            map->dirty = 1;
        }
    }
    if (length > 0) {
        if (seekFile(map->file, position + offset, 0) == 0xffffffff
            || writeFile(map->file, buffer, (long *)&length)) {
            setResourceError(fileError());
            goto done;
        }
        if (map->fileSize < position + offset + length) {
            offset += position;
            offset += length;
            map->fileSize = offset;
            map->dirty = 1;
        }
    }
    if (grow) {
        size += grow;
        entry->sizeLow = lowWord(size);
        entry->sizeHigh = highWord(size);
        map->dirty = 1;
    }
    if (position != entry->offset) {
        entry->offset = position;
        map->compacted = 1;
        map->dirty = 1;
    }
    setResourceError(0);
done:
    unlockFile(map->file);
    unlockHandle(id);
    return resources.error;
}

/* @zoombi32 0x00492cfb */
unsigned short __cdecl lowWord(unsigned long value)
{
    return value;
}

/* @zoombi32 0x00492d04 */
unsigned short __cdecl highWord(unsigned long value)
{
    return value >> 16;
}

/* @zoombi32 0x00492d0f */
short setResourceError(short error)
{
    return resources.error = error;
}

/* Finds resource `id`'s map and file-table entry; 0 if there's no such
   resource (or it's been deleted). */
/* @zoombi32 0x00492d20 */
short findEntry(long id, ResourceMap **map, FileTableEntry **entry)
{
    unsigned short index = resourceIndex(id);

    return (*map = resourceMap(resourceMapHandle(id))) != 0
           && !((*entry = &(*map)->fileTable.entries[index - 1])->flags & RESOURCE_DELETED);
}

/* @zoombi32 0x00492d6f */
unsigned long resourceIndex(long id)
{
    return (unsigned long)id >> 16;
}

/* @zoombi32 0x00492d7c */
long resourceMapHandle(long id)
{
    return (unsigned short)id;
}

/* A map's data, from its handle; 0 if it isn't one. */
/* @zoombi32 0x00492d87 */
ResourceMap *resourceMap(LONG_PTR handle)
{
    ResourceMap *map;

    if (handle && (map = (ResourceMap *)handleData(handle))->tag == 0x524d6170)
        return map;
    return 0;
}

/* @zoombi32 0x00492daf */
unsigned long __cdecl makeLong(unsigned short low, unsigned short high)
{
    return ((unsigned long)high << 16) + low;
}
