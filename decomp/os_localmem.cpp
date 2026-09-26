/*
 * os_localmem (0x46d95c-0x46da64): LocalAlloc, LocalReAlloc, LocalFree
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

/* @zoombi32 0x0046d9c8 */
short fn_46d9c8()
{
    return g_4b9cf0;
}

/* @zoombi32 0x0046da35 */
void fn_46da35(short value)
{
    g_4b9cf0 = value;
}

/* Whether a pointer is non-null and 4-byte aligned. */
/* @zoombi32 0x0046da46 */
int isAlignedPointer(void *pointer)
{
    if (!pointer || ((unsigned long)pointer & 3))
        return 0;
    return 1;
}
