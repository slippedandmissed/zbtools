/*
 * module_481670 (Mohawk engine): setEmptyRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00481670 */
void setEmptyRgn(short region)
{
    Region *data;
    ShortRect empty;

    if ((data = getRegion(region)) == 0) {
        setRegionError(0x2937);
        return;
    }
    data->count = 0;
    data->capacity = 16;
    data->bounds = *setRect(&empty, 0, 0, 0, 0);
    setHandleSize(region, 0x90);
}
