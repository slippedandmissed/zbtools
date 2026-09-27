/*
 * module_481640 (Mohawk engine): diffRgnRect
 */

/* @flags -p */

#include "zoombinis.h"

/* Takes a rectangle out of a region. */
/* @zoombi32 0x00481640 */
void diffRgnRect(short region, ShortRect *rect)
{
    if (emptyRect(rect))
        setRegionError(0);
    else
        diffRgnRects(region, 1, rect);
}
