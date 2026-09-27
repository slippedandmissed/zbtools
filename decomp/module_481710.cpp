/*
 * module_481710 (Mohawk engine): tidyRgn
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* Merges each rectangle with the ones below it that it exactly continues,
   and recomputes the bounds. The number of rectangles. */
/* @zoombi32 0x00481710 */
short tidyRgn(Region *region)
{
    long i;
    ShortRect empty;
    ShortRect *rect;
    long j;

    if (region->count) {
        for (i = 0; region->count > i; i++) {
            rect = &region->rects[i];
            for (j = i + 1; j < region->count;) {
                if (rect->bottom == region->rects[j].top && rect->left == region->rects[j].left
                    && rect->right == region->rects[j].right) {
                    rect->bottom = region->rects[j].bottom;
                    removeRgnRect(region, j);
                } else if (rect->bottom < region->rects[j].top)
                    break;
                else
                    j++;
            }
            if (!i)
                region->bounds = *rect;
            else {
                region->bounds.left = rect->left < region->bounds.left ? rect->left : region->bounds.left;
                region->bounds.right =
                    rect->right > region->bounds.right ? rect->right : region->bounds.right;
                region->bounds.bottom =
                    rect->bottom > region->bounds.bottom ? rect->bottom : region->bounds.bottom;
            }
        }
    } else
        region->bounds = *setRect(&empty, 0, 0, 0, 0);
    return region->count;
}
