/*
 * module_481200 (Mohawk engine): setRectRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Makes a region the rectangle `rect`. */
/* @zoombi32 0x00481200 */
void setRectRgn(short region, ShortRect *rect)
{
    Region *data;
    short error;

    if (!getRegion(region)) {
        setRegionError(0x2937);
        return;
    }
    if ((error = setHandleSize(region, 0x90)) != 0) {
        setRegionError(error);
        return;
    }
    data = (Region *)handleData(region);
    data->capacity = 16;
    data->count = 1;
    data->bounds = *rect;
    data->rects[0] = *rect;
    setRegionError(0);
}
