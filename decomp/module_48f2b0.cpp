/*
 * module_48f2b0 (Mohawk engine): setGrowProc
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048f2b0 */
GrowProc setGrowProc(GrowProc proc)
{
    GrowProc old;

    old = heap.growProc;
    heap.growProc = proc;
    return old;
}
