/*
 * movememory (Mohawk engine): moveMemory, lockPtr
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"
#include "os_manager.h"

/* @zoombi32 0x0048ef24 */
void moveMemory(void *to, const void *from, unsigned long size)
{
    memmove(to, from, size);
}

/* Locks a fixed block's pages in memory (the OS layer does nothing). */
/* @zoombi32 0x0048ef3c */
short lockPtr(void *pointer)
{
    Chunk *chunk;

    if (!isPointer(pointer))
        return setMemError(0x27af);
    chunk = (Chunk *)pointer - 1;
    return setMemError(osLockMemory(chunk, chunk->size + sizeof(Chunk)));
}
