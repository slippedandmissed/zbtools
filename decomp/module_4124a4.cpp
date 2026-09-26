/*
 * module_4124a4 (0x4124a4-0x413c24): no strings; uses isMousePresent and toUpperAscii (input?)
 */

#include "zoombinis.h"

/* Calls callback with g_4aa498 if there is one; returns whether it did. */
/* @zoombi32 0x00412b4d */
short fn_412b4d(void (*callback)(long))
{
    if (!callback)
        return 0;
    callback(g_4aa498);
    return 1;
}

/* @zoombi32 0x00413bcf */
void fn_413bcf(long value)
{
    g_4aa4c4 = value;
}
