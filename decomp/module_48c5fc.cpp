/*
 * module_48c5fc (Mohawk engine): invertRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048c5fc */
short invertRect(const Rect &rect)
{
    basePort *port;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    port->prepare();
    WinRect converted(rect);
    InvertRect(port->dc, &converted);
    return setPortError(0);
}
