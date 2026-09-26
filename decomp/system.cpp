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

/* Lower-cases an ASCII letter. */
/* @zoombi32 0x004159dd */
unsigned short toLowerAscii(unsigned short c)
{
    if (c >= 'A' && c <= 'Z')
        c |= 0x20;
    return c;
}

/* Upper-cases an ASCII letter. */
/* @zoombi32 0x004159f7 */
unsigned short toUpperAscii(unsigned short c)
{
    if (c >= 'a' && c <= 'z')
        c &= 0xdf;
    return c;
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

/* @zoombi32 0x00413c6d */
void fn_413c6d(void **block)
{
    freeAndClear(block);
}

/* @zoombi32 0x00414358 */
void fn_414358(void **block)
{
    freeAndClear(block);
}
