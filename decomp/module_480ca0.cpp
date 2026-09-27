/*
 * module_480ca0 (Mohawk engine): unionRect
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* The smallest rectangle holding both (QuickDraw's UnionRect), into
   `into`; an empty rectangle adds nothing. */
/* @zoombi32 0x00480ca0 */
ShortRect *unionRect(ShortRect *into, ShortRect *add)
{
    if (into->left >= into->right || into->top >= into->bottom) {
        if (add->left >= add->right || add->top >= add->bottom)
            memset(into, 0, sizeof(ShortRect));
        else
            memcpy(into, add, sizeof(ShortRect));
    } else if (add->left < add->right && add->top < add->bottom) {
        into->left = into->left < add->left ? into->left : add->left;
        into->top = into->top < add->top ? into->top : add->top;
        into->right = into->right > add->right ? into->right : add->right;
        into->bottom = into->bottom > add->bottom ? into->bottom : add->bottom;
    }
    return into;
}
