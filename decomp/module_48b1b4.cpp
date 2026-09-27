/*
 * module_48b1b4 (Mohawk engine): fn_48b1b4 (after a window port is painted)
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Calls the current port's slot 38, if it is a window's port. Not exact:
   the original keeps `port` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048b1b4 */
short fn_48b1b4()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (port->kind != 5)
        return setPortError(0x2a73);
    return ((windowPort *)port)->v38();
}
