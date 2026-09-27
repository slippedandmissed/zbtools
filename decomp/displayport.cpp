/*
 * displayport (Mohawk engine): displayPort, a port on the display
 */

/* @flags -p -x- */

#include "zoombinis.h"

static char display[] = "DISPLAY";

/* @zoombi32 0x0048ac90 */
__cdecl displayPort::displayPort(const Rect &bounds) : basePort(bounds)
{
    kind = 2;
    paletteKind = 1;
}

/* @zoombi32 0x0048acbc */
void displayPort::depthChanged()
{
    basePort::depthChanged();
    rasterCaps = GetDeviceCaps(dc, RASTERCAPS);
    depth = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
    depth = depth < 24 ? depth : 24;
}

/* @zoombi32 0x0048ad10 */
void displayPort::prepare()
{
    realizePalette();
    basePort::prepare();
}

/* @zoombi32 0x0048ad28 */
void displayPort::realizePalette()
{
    RealizePalette(dc);
    realized = palette;
}

/* @zoombi32 0x0048ad42 */
int displayPort::stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                               int fromWidth, int fromHeight, const void *bits, BITMAPINFO *info,
                               UINT usage, DWORD rop)
{
    prepare();
    return StretchDIBits(dc, toX, toY, toWidth, toHeight, fromX, fromY, fromWidth, fromHeight,
                         bits, info, usage, rop);
}

/* @zoombi32 0x0048ad80 */
short displayPort::lock()
{
    if (!locks) {
        if ((dc = CreateDC(display, 0, 0, 0)) == 0)
            return setPortError(0x2a37);
        if (setupDC()) {
            DeleteDC(dc);
            return graphics.error;
        }
        unknown66 = 0;
        unknown68 = 1;
        unknown6A = 1;
    }
    locks++;
    return setPortError(0);
}
