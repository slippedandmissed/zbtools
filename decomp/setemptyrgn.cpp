/*
 * setemptyrgn (Mohawk engine): setEmptyRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00481670 */
short setEmptyRgn(short region)
{
    Region *data;

    if ((data = getRegion(region)) == 0) {
        return setRegionError(0x2937);
    }
    data->count = 0;
    data->capacity = 16;
    data->bounds = Rect(0, 0, 0, 0);
    return setHandleSize(region, 0x90);
}
