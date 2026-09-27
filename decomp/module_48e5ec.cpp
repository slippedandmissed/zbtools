/*
 * module_48e5ec (Mohawk engine): newHandle, newPtr
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A new handle, with a block of `size` bytes (none if 0); 0 on error. */
/* Not exact: BCC32 4.5 caches `heap`'s address in edi; the
   original addresses each field directly (it caches in bigger functions). */
/* @zoombi32 0x0048e5ec */
short newHandle(unsigned long size)
{
    Block block;
    HandleEntry *entry;
    short error;

    if (!size)
        block = 0;
    else if ((block = allocBlock(size, 1)) == 0)
        return 0;
    for (;;) {
        if (heap.table->freeList || !growHandleTable(0x100))
            break;
        if ((error = heap.error) == 0x2776 && growZone(0, error))
            continue;
        if (block)
            freeBlock(block);
        setMemError(error);
        return 0;
    }
    entry = handleEntry(heap.table->freeList);
    heap.table->freeList = entry->nextFree;
    entry->used = 1;
    entry->block = block;
    if (block)
        (*block)->handle = entryIndex(entry);
    entry->state = 0;
    entry->locks = 0;
    entry->age = 15;
    entry->keep = 0;
    entry->purgeable = 0;
    setMemError(0);
    return entryIndex(entry);
}

/* A new fixed block of `size` bytes; 0 if 0 or on error. */
/* @zoombi32 0x0048e6b4 */
void *newPtr(unsigned long size)
{
    Block block;

    if (!size) {
        setMemError(0);
        return 0;
    }
    if ((block = allocBlock(size, 0)) == 0)
        return 0;
    setMemError(0);
    return (char *)*block + sizeof(Chunk);
}
