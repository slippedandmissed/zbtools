/*
 * module_480f90 (Mohawk engine): sectRgnWithRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Intersects a region with a rectangle. Not exact: the original has region
   in ebx and rect in esi. */
/* @zoombi32 0x00480f90 */
void sectRgnWithRect(short region, ShortRect *rect)
{
    if (emptyRect(rect))
        setEmptyRgn(region);
    else
        sectRgnRects(region, 1, rect);
}
