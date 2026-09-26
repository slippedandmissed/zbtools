/*
 * e2memory (0x46be28-0x46cca0): 'e2AllocHandle error: memHandle already in use', 'e2GetShapes error'
 */

#include "zoombinis.h"

/* @zoombi32 0x0046be2e */
void fn_46be2e(long value)
{
    g_4a7f58 = value;
}

/* @zoombi32 0x0046bee2 */
short fn_46bee2()
{
    return g_4b99d4;
}

/* Sets g_4b99d4, returning its old value. */
/* @zoombi32 0x0046bee9 */
short fn_46bee9(short value)
{
    short old = g_4b99d4;
    g_4b99d4 = value;
    return old;
}

/* @zoombi32 0x0046ca9c */
void fn_46ca9c(long *handle)
{
    if (*handle) {
        fn_48f660(*handle, 0, 0);
        *handle = 0;
    }
}
