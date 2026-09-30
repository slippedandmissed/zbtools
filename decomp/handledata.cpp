/*
 * handledata (Mohawk engine): handleData, setMemError, handleEntry, validHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A handle's block's address (not locking it). */
/* @zoombi32 0x0048f5bc */
void *handleData(short handle)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0;
    }
    entry = handleEntry(handle);
    if (!entry->block) {
        setMemError(0x2775);
        return 0;
    }
    entry->age = 15;
    setMemError(0);
    return (char *)*entry->block + sizeof(Chunk);
}

/* @zoombi32 0x0048f613 */
short setMemError(short error)
{
    return heap.error = error;
}

/* @zoombi32 0x0048f624 */
HandleEntry *handleEntry(unsigned short handle)
{
    return &heap.table->entries[handle - 1];
}

/* @zoombi32 0x0048f63c */
short validHandle(unsigned short handle, short)
{
    if (handle && handle <= heap.table->count)
        return 1;
    return 0;
}
