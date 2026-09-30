/*
 * setpurgeproc (Mohawk engine): setPurgeProc
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048f44c */
PurgeProc setPurgeProc(PurgeProc proc)
{
    PurgeProc old;

    old = heap.purgeProc;
    heap.purgeProc = proc;
    return old;
}
