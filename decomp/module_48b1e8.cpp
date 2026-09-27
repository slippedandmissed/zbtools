/*
 * module_48b1e8 (Mohawk engine): eraseRgn, frameRect, getClip
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Fills a region with the current port's background colour. Not exact: the
   original copies the colour through eax, BCC32 4.5 through a saved
   register. */
/* @zoombi32 0x0048b1e8 */
short eraseRgn(short region)
{
    basePort *port;
    HBRUSH brush;
    short result;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    if ((brush = port->brush(port->backColor)) == 0)
        return graphics.error;
    result = port->fillRgn(region, brush, 0);
    DeleteObject(brush);
    return result;
}

/* Outlines a rectangle with the current port's pen. */
/* @zoombi32 0x0048b244 */
short frameRect(const ShortRect &rect)
{
    basePort *port;
    HGDIOBJ old;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    port->prepare();
    old = SelectObject(port->dc, GetStockObject(NULL_BRUSH));
    Rectangle(port->dc, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(port->dc, old);
    return setPortError(0);
}

/* Copies the current port's clip region into `region`. Not exact: the original keeps `port` in eax; BCC32 4.5 gives it a saved
   register. */
/* @zoombi32 0x0048b2ac */
short getClip(short region)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    return copyRgn(region, port->clip);
}
