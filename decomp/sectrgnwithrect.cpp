/*
 * sectrgnwithrect (Mohawk engine): sectRgnWithRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Intersects a region with a rectangle; an error code. */
/* @zoombi32 0x00480f90 */
short sectRgnWithRect(short region, ShortRect *rect)
{
    return emptyRect(rect) ? setEmptyRgn(region) : sectRgnRects(region, 1, rect);
}
