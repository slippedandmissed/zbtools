/*
 * module_48c688 (Mohawk engine): localToGlobal and point conversions
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Converts a point from the current port's coordinates to the screen's. Not exact: the original swaps the
   registers of `port` and `point` (ebx and esi). */
/* @zoombi32 0x0048c688 */
short localToGlobal(Point *point)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    WinPoint converted = Pt(*point);
    LPtoDP(port->dc, &converted, 1);
    *point = Pt(converted);
    return setPortError(0);
}

/* @zoombi32 0x0048c6f3 */
__cdecl Pt::Pt(const Point &point)
{
    x = point.x;
    y = point.y;
}

/* Not exact: the original has `point` in eax and `this` in edx, and returns
   `this` explicitly. */
/* @zoombi32 0x0048c70c */
__cdecl WinPoint::WinPoint(const Pt &point)
{
    POINT converted;

    converted.x = point.x;
    converted.y = point.y;
    x = converted.x;
    y = converted.y;
}

/* @zoombi32 0x0048c736 */
__cdecl Pt::Pt(const WinPoint &point)
{
    x = (short)point.x;
    y = (short)point.y;
}
