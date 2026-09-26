/*
 * platform (0x455530-0x456c00): the Windows layer: the window class and procedure
 * (0x45605e), the message loop (PeekMessageA, GetMessageA), keyboard and cursor
 */

#include <windows.h>
#include <stdlib.h>
#include "zoombinis.h"

/* Whether a mouse is installed. */
/* @zoombi32 0x00455903 */
int isMousePresent()
{
    return GetSystemMetrics(SM_MOUSEPRESENT);
}

/* Frees *block if it's allocated, and clears it. */
/* @zoombi32 0x00455c43 */
void freeAndClear(void **block)
{
    if (*block) {
        free(*block);
        *block = 0;
    }
}

/* Writes value in decimal into buffer. */
/* @zoombi32 0x00455c8b */
char *intToDecimal(int value, char *buffer)
{
    return itoa(value, buffer, 10);
}

/* Writes value in decimal into buffer. */
/* @zoombi32 0x00455ca2 */
char *unsignedToDecimal(unsigned long value, char *buffer)
{
    return ultoa(value, buffer, 10);
}

/* @zoombi32 0x00455e26 */
void fn_455e26(long)
{
}

/* @zoombi32 0x00455e2d */
void fn_455e2d(long)
{
}

/* @zoombi32 0x00455e85 */
long fn_455e85(long, long)
{
    return 0;
}

/* @zoombi32 0x00456a2f */
void fn_456a2f(Callback callback)
{
    g_4a4a14 = callback;
}

/* @zoombi32 0x00456a3e */
void fn_456a3e(long first, long second)
{
    g_4a4a18 = first;
    g_4a4a1c = second;
}

/* @zoombi32 0x00456a55 */
void fn_456a55(long value)
{
    g_4a4a00 = value;
}

/* @zoombi32 0x00456bf6 */
short fn_456bf6()
{
    return g_4b2d38;
}
