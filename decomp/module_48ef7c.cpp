/*
 * module_48ef7c (Mohawk engine): unlockPtr
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048ef7c */
short unlockPtr(void *pointer)
{
    Chunk *chunk;

    if (!isPointer(pointer))
        return setMemError(0x27af);
    chunk = (Chunk *)pointer - 1;
    return setMemError(osUnlockMemory(chunk, chunk->size + sizeof(Chunk)));
}
