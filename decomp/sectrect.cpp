/*
 * sectrect (0x480c24-0x480ca0): one function (Mohawk engine)
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* Intersects a rectangle with another, emptying it (all zeros) if they don't
   overlap; whether they do. */
/* @zoombi32 0x00480c24 */
short sectRect(ShortRect *rect, ShortRect *with)
{
    if (rect->left < with->left)
        rect->left = with->left;
    if (rect->top < with->top)
        rect->top = with->top;
    if (rect->right > with->right)
        rect->right = with->right;
    if (rect->bottom > with->bottom)
        rect->bottom = with->bottom;
    if (rect->left >= rect->right || rect->top >= rect->bottom) {
        memset(rect, 0, sizeof(ShortRect));
        return 0;
    }
    return 1;
}
