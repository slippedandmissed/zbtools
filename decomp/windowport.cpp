/*
 * windowport (Mohawk engine): windowPort, a port on a window, and WinRect's
 * constructor
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048db64 */
__cdecl windowPort::windowPort(const Rect &bounds, HWND window) : displayPort(bounds)
{
    this->window = window;
    kind = 5;
}

/* Clips to what the window needs painted, and validates it; the whole port
   if nothing does. */
/* @zoombi32 0x0048db92 */
short windowPort::beginUpdate()
{
    if (update)
        return setPortError(0x2a78);
    if ((update = CreateRectRgn(0, 0, 0, 0)) == 0)
        return setPortError(0x2a37);
    switch (GetUpdateRgn(window, update, FALSE)) {
    case ERROR:
        DeleteObject(update);
        update = 0;
        return setPortError(0x2a37);
    case NULLREGION:
        SetRectRgn(update, bounds.left, bounds.top, bounds.right, bounds.bottom);
        break;
    default: {
        WinRect all(bounds);

        ValidateRect(window, &all);
        break;
    }
    }
    clipApplied = 0;
    return setPortError(0);
}

/* @zoombi32 0x0048dc69 */
short windowPort::setClip(short region)
{
    HRGN combined;

    if (toHrgn(clipRgn, region))
        return graphics.error;
    if (update) {
        if ((combined = CreateRectRgn(0, 0, 0, 0)) == 0)
            return setPortError(0x2a37);
        if (!CombineRgn(combined, clipRgn, update, RGN_AND)) {
            DeleteObject(combined);
            return setPortError(0x2a37);
        }
        DeleteObject(clipRgn);
        clipRgn = combined;
    }
    SelectClipRgn(dc, clipRgn);
    IntersectClipRect(dc, frame.left, frame.top, frame.right, frame.bottom);
    return setPortError(0);
}

/* @zoombi32 0x0048dd1d */
short windowPort::endUpdate()
{
    if (!update)
        return setPortError(0x2a77);
    DeleteObject(update);
    update = 0;
    clipApplied = 0;
    return setPortError(0);
}

/* @zoombi32 0x0048dd5e */
short windowPort::lock()
{
    if (!locks) {
        if ((dc = GetDC(window)) == 0)
            return setPortError(0x2a37);
        if (setupDC()) {
            ReleaseDC(window, dc);
            return graphics.error;
        }
        unknown66 = 0;
        unknown68 = 1;
        unknown6A = 1;
    }
    locks++;
    return setPortError(0);
}

/* @zoombi32 0x0048ddcd */
void windowPort::release()
{
    if (update)
        DeleteObject(update);
    basePort::release();
}

/* Scrolls, and has the window repaint what's uncovered (unless it goes into
   `region`). */
/* @zoombi32 0x0048ddf3 */
short windowPort::scroll(const Rect *rect, short dx, short dy, short region)
{
    Rect moved = *rect;
    HRGN uncovered;

    offsetRect(&moved, dx, dy);
    if (region && (setRectRgn(region, (ShortRect *)rect) || diffRgnRect(region, &moved)))
        return setPortError(regionError());
    if ((uncovered = CreateRectRgn(0, 0, 0, 0)) == 0)
        return setPortError(0x2a37);
    prepare();
    WinRect device(*rect);
    if (ScrollDC(dc, dx, dy, &device, &device, uncovered, 0)) {
        InvalidateRgn(window, uncovered, FALSE);
        if (region) {
            toHrgn(uncovered, region);
            ValidateRgn(window, uncovered);
        }
        setPortError(0);
    } else {
        setPortError(0x2a37);
    }
    DeleteObject(uncovered);
    return graphics.error;
}

/* @zoombi32 0x0048def9 */
void windowPort::unlock()
{
    if (!--locks) {
        cleanupDC();
        ReleaseDC(window, dc);
    }
}

/* @zoombi32 0x0048df26 */
__cdecl WinRect::WinRect(const Rect &rect)
{
    RECT converted;

    converted.left = rect.left;
    converted.top = rect.top;
    converted.right = rect.right;
    converted.bottom = rect.bottom;
    *(RECT *)this = converted;
}

/* @zoombi32-implicit 0x0048dfff displayPort::~displayPort */

/* @zoombi32-implicit 0x0048e078 windowPort::~windowPort */
