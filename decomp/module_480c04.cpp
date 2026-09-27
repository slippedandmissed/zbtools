/*
 * module_480c04 (0x480c04-0x480c24): one function (Mohawk engine)
 */

/* @flags -p */

#include "zoombinis.h"

/* Moves a rectangle (QuickDraw's OffsetRect). */
/* @zoombi32 0x00480c04 */
void offsetRect(ShortRect *rect, short dx, short dy)
{
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
}
