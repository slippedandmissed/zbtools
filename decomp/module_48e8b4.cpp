/*
 * module_48e8b4 (Mohawk engine): handleState
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048e8b4 */
unsigned short handleState(short handle)
{
    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0xffff;
    }
    setMemError(0);
    return handleEntry(handle)->state;
}
