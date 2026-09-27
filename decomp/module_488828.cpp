/*
 * module_488828 (Mohawk engine): clipPortToRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Intersects the current port's clip region with a rectangle. */
/* @zoombi32 0x00488828 */
short clipPortToRect(const Rect &rect)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (sectRgnWithRect(port->clip, (ShortRect *)&rect))
        return setPortError(regionError());
    port->clipChanged = 0;
    return setPortError(0);
}
