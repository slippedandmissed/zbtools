/*
 * disposergn (Mohawk engine): disposeRgn
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x004812bc */
void disposeRgn(short region)
{
    Region *data;

    if ((data = getRegion(region)) == 0) {
        setRegionError(0x2937);
        return;
    }
    data->tag = 0;
    if (disposeHandle(region)) {
        data->tag = RESOURCE_TYPE('r', 'g', 'n', 'R');
        setRegionError(memError());
        return;
    }
    setRegionError(0);
}
