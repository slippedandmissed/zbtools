/*
 * Thin wrappers around Windows functions.
 */

#include <windows.h>

/* Whether a mouse is installed. */
/* @zoombi32 0x00455903 */
int isMousePresent()
{
    return GetSystemMetrics(SM_MOUSEPRESENT);
}

#include <stdlib.h>
#include <time.h>

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

extern time_t g_4a07b8;

/* @zoombi32 0x00415514 */
void fn_415514()
{
    time_t now;
    g_4a07b8 = time(&now);
}
