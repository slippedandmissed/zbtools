/*
 * diffrgnrect (Mohawk engine): diffRgnRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Takes a rectangle out of a region. */
/* @zoombi32 0x00481640 */
short diffRgnRect(short region, ShortRect *rect)
{
    if (emptyRect(rect))
        return setRegionError(0);
    return diffRgnRects(region, 1, rect);
}
