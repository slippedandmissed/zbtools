/*
 * module_480bdc (0x480bdc-0x480c04): one function (Mohawk engine)
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Whether a rectangle is empty (QuickDraw's EmptyRect). */
/* @zoombi32 0x00480bdc */
short emptyRect(ShortRect *rect)
{
    return rect->left >= rect->right || rect->top >= rect->bottom;
}
