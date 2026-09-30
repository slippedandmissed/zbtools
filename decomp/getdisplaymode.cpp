/*
 * getdisplaymode (Mohawk engine): getDisplayMode
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "graphics.h"

/* The current display mode. */
/* @zoombi32 0x0048b2d8 */
void getDisplayMode(DisplayMode *mode)
{
    HDC ic;
    unsigned short depth;

    ic = CreateIC("DISPLAY", 0, 0, 0);
    mode->width = GetDeviceCaps(ic, HORZRES);
    mode->height = GetDeviceCaps(ic, VERTRES);
    depth = GetDeviceCaps(ic, PLANES) * GetDeviceCaps(ic, BITSPIXEL);
    mode->colors = 1L << (depth < 24 ? depth : 24);
    mode->palettized = depth == 8;
    DeleteDC(ic);
}
