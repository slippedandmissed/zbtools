/*
 * handlesize (Mohawk engine): handleSize, handleLocks
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The size of a handle's block (0 if empty or purged; -1 on error). */
/* @zoombi32 0x0048e814 */
unsigned long handleSize(short handle)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return (unsigned long)-1;
    }
    setMemError(0);
    entry = handleEntry(handle);
    return entry->block ? (*entry->block)->size : 0;
}

/* @zoombi32 0x0048e860 */
unsigned short handleLocks(short handle)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0xffff;
    }
    entry = handleEntry(handle);
    if (!entry->block) {
        setMemError(0x2775);
        return 0xffff;
    }
    setMemError(0);
    return entry->locks;
}
