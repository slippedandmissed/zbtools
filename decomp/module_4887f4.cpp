/*
 * module_4887f4 (Mohawk engine): fn_4887f4 (before a window port is painted)
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Calls the current port's slot 37, if it is a window's port. Not exact: the original keeps `port` in eax (no call intervenes where it is
   used); BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004887f4 */
short fn_4887f4()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (port->kind != 5)
        return setPortError(0x2a73);
    return ((windowPort *)port)->v37();
}
