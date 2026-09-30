/*
 * setrectrgn (Mohawk engine): setRectRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Makes a region the rectangle `rect`. */
/* @zoombi32 0x00481200 */
short setRectRgn(short region, ShortRect *rect)
{
    Region *data;
    short error;

    if (!getRegion(region)) {
        return setRegionError(0x2937);
    }
    if ((error = setHandleSize(region, 0x90)) != 0) {
        return setRegionError(error);
    }
    data = (Region *)handleData(region);
    data->capacity = 16;
    data->count = 1;
    data->bounds = *rect;
    data->rects[0] = *rect;
    return setRegionError(0);
}
