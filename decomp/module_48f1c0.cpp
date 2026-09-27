/*
 * module_48f1c0 (Mohawk engine): setPurgeEnabled, resizePtr, isPointer, fn_48f260
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Whether purgeMemory may purge; the previous setting. */
/* @zoombi32 0x0048f1c0 */
short setPurgeEnabled(short enabled)
{
    short old;

    old = heap.purgeEnabled;
    heap.purgeEnabled = enabled;
    return old;
}

/* Resizes a fixed block (which may move): a new one if 0, freed if `size`
   is 0. */
/* Not exact: the original keeps `block` in eax; BCC32 4.5 gives it
   edi. */
/* @zoombi32 0x0048f1d8 */
void *resizePtr(void *pointer, unsigned long size)
{
    Block block;

    if (!pointer)
        return newPtr(size);
    if (!isPointer(pointer)) {
        setMemError(0x27af);
        return 0;
    }
    if (!size) {
        freeBlock(blockOf((Chunk *)pointer - 1));
        return 0;
    }
    if ((block = resizeBlock(blockOf((Chunk *)pointer - 1), size)) == 0)
        return 0;
    return (char *)*block + sizeof(Chunk);
}

/* Whether a pointer is to a fixed block's data. */
/* @zoombi32 0x0048f242 */
short isPointer(void *pointer)
{
    if (pointer)
        return ((Chunk *)pointer - 1)->magic == 0x4d42 /* 'BM' */;
    return 0;
}

/* Counts heap.unknown4 up, to at most 3. */
/* @zoombi32 0x0048f260 */
short fn_48f260()
{
    if (heap.unknown4 >= 3) {
        setMemError(0x27ab);
        return 0;
    }
    setMemError(0);
    return ++heap.unknown4;
}
