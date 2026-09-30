/*
 * disposeptr (Mohawk engine): disposePtr
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048e7b8 */
short disposePtr(void *pointer)
{
    if (!isPointer(pointer))
        return setMemError(0x27af);
    return freeBlock(blockOf((Chunk *)pointer - 1));
}
