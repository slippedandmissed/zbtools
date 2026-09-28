/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "bctwo.h"

/* @zoombi32 0x004196a8 */
long fn_4196a8(long)
{
    return 0;
}

/* The index of the last of 625 entries with a value, or 0. */
/* @zoombi32 0x00419f1a */
short fn_419f1a()
{
    for (short i = 0x270; i >= 0; i--)
        if (g_4ab64c[i].value)
            return i;
    return 0;
}
