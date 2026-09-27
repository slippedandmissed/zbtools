/*
 * module_48206c (Mohawk engine): region errors, inserting rectangles
 */

/* @flags -p */

#include <string.h>
#include "zoombinis.h"

/* @zoombi32 0x0048206c */
short regionError()
{
    return regionErrorCode;
}

/* Inserts a rectangle into a region's list at `index`, growing it (16 at a
   time) if needed. */
/* @zoombi32 0x00482073 */
short insertRgnRect(short handle, Region **region, long index, ShortRect *rect)
{
    if ((*region)->capacity < (*region)->count + 1) {
        if (setHandleSize(handle, ((*region)->capacity + 16) * sizeof(ShortRect) + 0x10)) {
            return setRegionError(memError());
        }
        *region = (Region *)handleData(handle);
        (*region)->capacity += 16;
    }
    if (index < (*region)->count)
        memmove(&(*region)->rects[index + 1], &(*region)->rects[index],
                ((*region)->count - index) * sizeof(ShortRect));
    memcpy(&(*region)->rects[index], rect, sizeof(ShortRect));
    (*region)->count++;
    return setRegionError(0);
}

/* @zoombi32 0x00482129 */
short setRegionError(short error)
{
    return regionErrorCode = error;
}
