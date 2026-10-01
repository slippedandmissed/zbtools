/*
 * os_localmem (0x46d95c-0x46da64): LocalAlloc, LocalReAlloc, LocalFree
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"
#include "os_localmem.h"

short localMemErrorCode;

/* Memory from the process's heap (LocalAlloc); 0 for 0 bytes. */
/* @zoombi32 0x0046d95c */
void *localAlloc(unsigned short size)
{
    void *block;

    if (!size) {
        setLocalMemError(0);
        return 0;
    }
    block = (void *)LocalAlloc(LMEM_FIXED, size);
    setLocalMemError(block ? 0 : 200);
    return block;
}

/* (The callers test only the low word of isAlignedPointer's result, as if
   it were declared `short`.) */
/* @zoombi32 0x0046d998 */
void localFree(void *block)
{
    if (!(short)isAlignedPointer(block)) {
        setLocalMemError(0xfb);
        return;
    }
    LocalFree((HLOCAL)block);
    setLocalMemError(0);
}

/* The error of the last call. */
/* @zoombi32 0x0046d9c8 */
short localMemError()
{
    return localMemErrorCode;
}

/* Resizes a block (allocating one, from 0, or freeing it, to 0 bytes). */
/* @zoombi32 0x0046d9cf */
void *localReAlloc(void *block, unsigned short size)
{
    if (!block)
        return localAlloc(size);
    if (!(short)isAlignedPointer(block)) {
        setLocalMemError(0xfb);
        return 0;
    }
    if (!size) {
        localFree(block);
        return 0;
    }
    block = (void *)LocalReAlloc((HLOCAL)block, size, LMEM_MOVEABLE);
    setLocalMemError(block ? 0 : 200);
    return block;
}

/* @zoombi32 0x0046da35 */
void setLocalMemError(short value)
{
    localMemErrorCode = value;
}

/* Whether a pointer is non-null and 4-byte aligned. */
/* @zoombi32 0x0046da46 */
int isAlignedPointer(void *pointer)
{
    if (!pointer || ((UINT_PTR)pointer & 3))
        return 0;
    return 1;
}
