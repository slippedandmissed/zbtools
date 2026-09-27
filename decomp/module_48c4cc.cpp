/*
 * module_48c4cc (Mohawk engine): globalToLocal
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Converts a point from the screen's (device) coordinates to the current
   port's. Not exact: the original swaps the
   registers of `port` and `point` (ebx and esi). */
/* @zoombi32 0x0048c4cc */
short globalToLocal(Point *point)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    WinPoint converted = Pt(*point);
    DPtoLP(port->dc, &converted, 1);
    *point = Pt(converted);
    return setPortError(0);
}
