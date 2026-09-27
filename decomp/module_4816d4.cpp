/*
 * module_4816d4 (Mohawk engine): emptyRgn
 */

/* @flags -p */

#include "zoombinis.h"

/* Whether a region is empty (-1 if it isn't a region). */
/* @zoombi32 0x004816d4 */
unsigned short emptyRgn(short region)
{
    Region *data;

    if ((data = getRegion(region)) == 0) {
        setRegionError(0x2937);
        return 0xffff;
    }
    setRegionError(0);
    return data->count == 0;
}
