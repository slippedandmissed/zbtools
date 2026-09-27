/*
 * module_480bbc (Mohawk engine): insetRect
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Shrinks a rectangle by dx and dy on each side (QuickDraw's InsetRect). */
/* @zoombi32 0x00480bbc */
void insetRect(ShortRect *rect, short dx, short dy)
{
    rect->left += dx;
    rect->right -= dx;
    rect->top += dy;
    rect->bottom -= dy;
}
