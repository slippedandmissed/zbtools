/*
 * module_481820 (Mohawk engine): unionRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Adds a region to another. */
/* @zoombi32 0x00481820 */
short unionRgn(short to, short from)
{
    Region *data;

    if ((data = getRegion(from)) == 0) {
        return setRegionError(0x2937);
    }
    if (from == to) {
        return setRegionError(0);
    }
    lockHandle(from);
    unionRgnRects(to, data->count, data->rects);
    unlockHandle(from);
    return regionErrorCode;
}
