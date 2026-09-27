/*
 * module_48203c (Mohawk engine): unionRgnRect
 */

/* @flags -p */

#include "zoombinis.h"

/* Adds a rectangle to a region. */
/* @zoombi32 0x0048203c */
void unionRgnRect(short region, ShortRect *rect)
{
    if (emptyRect(rect))
        setRegionError(0);
    else
        unionRgnRects(region, 1, rect);
}
