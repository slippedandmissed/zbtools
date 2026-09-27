/*
 * module_48b358 (Mohawk engine): getPortError
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048b358 */
short getPortError()
{
    return graphics.error;
}
