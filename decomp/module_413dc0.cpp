/*
 * module_413dc0 (0x413dc0-0x4144d0): no strings; a 32-entry ring buffer (an event queue?): fn_413dc0, nextRingIndex
 */

#include "zoombinis.h"

/* How many entries are queued in a 32-entry ring buffer (g_4aa79a is where
   reading starts, g_4aa79c where writing does). */
/* @zoombi32 0x00413dc0 */
short fn_413dc0()
{
    short count = g_4aa79c - g_4aa79a;
    if (count < 0)
        count += 32;
    return count;
}

/* Advances an index into the 32-entry ring buffer, wrapping to 0. */
/* @zoombi32 0x0041416f */
void __cdecl nextRingIndex(short *index)
{
    if (++*index > 31)
        *index = 0;
}

/* @zoombi32 0x00414358 */
void fn_414358(void **block)
{
    freeAndClear(block);
}
