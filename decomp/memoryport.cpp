/*
 * memoryport (Mohawk engine): memoryPort, a port in a bitmap compatible
 * with the display; Rect's constructor and moveTo
 */

/* @flags -p -x- */

#include "zoombinis.h"

static char display[] = "DISPLAY";

/* @zoombi32 0x0048c774 */
__cdecl memoryPort::memoryPort(short width, short height)
    : displayPort(Rect(0, 0, width, height))
{
    kind = 3;
    paletteKind = 1;
}

/* @zoombi32 0x0048c7bd */
short memoryPort::init()
{
    HDC screen;

    if (basePort::init())
        return graphics.error;
    if ((screen = CreateDC(display, 0, 0, 0)) == 0) {
        basePort::release();
        return setPortError(0x2a37);
    }
    bitmap = CreateCompatibleBitmap(screen, frame.right, frame.bottom);
    DeleteDC(screen);
    if (!bitmap) {
        basePort::release();
        return setPortError(0x2a37);
    }
    return setPortError(0);
}

/* @zoombi32 0x0048c844 */
short memoryPort::lock()
{
    if (!locks) {
        if ((dc = CreateCompatibleDC(0)) == 0)
            return setPortError(0x2a37);
        SelectObject(dc, bitmap);
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

/* @zoombi32 0x0048c8b7 */
void memoryPort::release()
{
    DeleteObject(bitmap);
    basePort::release();
}

/* @zoombi32 0x0048c8d4 */
__cdecl Rect::Rect(short left, short top, short right, short bottom)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

/* @zoombi32-implicit 0x0048c94f memoryPort::~memoryPort */

/* Moves the current port's pen. Not exact: the original keeps `port` in eax
   (see the open question in findings.md); BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048c974 */
short moveTo(short x, short y)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    MoveToEx(port->dc, x, y, 0);
    return setPortError(0);
}
