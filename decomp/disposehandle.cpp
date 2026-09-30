/*
 * disposehandle (Mohawk engine): disposeHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Frees a handle that isn't locked (or in a state). */
/* @zoombi32 0x0048e71c */
short disposeHandle(short handle)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0))
        return setMemError(0x27a7);
    entry = handleEntry(handle);
    if (entry->locks)
        return setMemError(0x27a8);
    if (entry->state)
        return setMemError(0x27a9);
    if (entry->block && freeBlock(entry->block))
        return heap.error;
    entry->nextFree = heap.table->freeList;
    heap.table->freeList = handle;
    entry->used = 0;
    return setMemError(0);
}
