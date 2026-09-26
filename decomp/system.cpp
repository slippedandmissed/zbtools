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
