/*
 * memerror (Mohawk engine): memError
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048e80c */
short memError()
{
    return heap.error;
}
