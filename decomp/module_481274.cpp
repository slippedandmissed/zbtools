/*
 * module_481274 (Mohawk engine): newRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A new, empty region (0 if there's no memory). */
/* @zoombi32 0x00481274 */
short newRgn()
{
    short region;
    Region *data;

    if ((region = newHandle(0x90)) != 0) {
        data = (Region *)handleData(region);
        data->tag = RESOURCE_TYPE('r', 'g', 'n', 'R');
        data->capacity = 16;
        data->count = 0;
        setRegionError(0);
    } else
        setRegionError(memError());
    return region;
}
