/*
 * resourceerror (Mohawk engine): resourceError
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The error of the last resource call. */
/* @zoombi32 0x0048feb8 */
short resourceError()
{
    return resources.error;
}
