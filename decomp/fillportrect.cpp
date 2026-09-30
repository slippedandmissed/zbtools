/*
 * fillportrect (Mohawk engine): fillPortRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Fills a rectangle of the current port with a colour. Not exact: the
   original keeps `port` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048c9ac */
short fillPortRect(const Rect &rect, Color color, short unknown)
{
    basePort *port;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    return port->fillRect(rect, color, unknown);
}
