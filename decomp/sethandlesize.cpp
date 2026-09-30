/*
 * sethandlesize (Mohawk engine): setHandleSize
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Resizes an unlocked handle's block (freeing it for 0, making one if it
   has none). */
/* @zoombi32 0x0048f2c8 */
short setHandleSize(short handle, unsigned long size)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0))
        return setMemError(0x27a7);
    entry = handleEntry(handle);
    if (entry->locks)
        return setMemError(0x27a8);
    if (!size) {
        if (entry->block)
            freeBlock(entry->block);
        else
            setMemError(0);
    } else if (!entry->block) {
        if ((entry->block = allocBlock(size, 1)) != 0)
            (*entry->block)->handle = handle;
    } else
        resizeBlock(entry->block, size);
    return heap.error;
}
