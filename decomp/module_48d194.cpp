/*
 * module_48d194 (Mohawk engine): setClipRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's clip region to a rectangle. */
/* @zoombi32 0x0048d194 */
short setClipRect(const Rect &rect)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (setRectRgn(port->clip, (ShortRect *)&rect))
        return setPortError(regionError());
    port->clipApplied = 0;
    return setPortError(0);
}
