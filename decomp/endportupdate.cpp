/*
 * endportupdate (Mohawk engine): endPortUpdate (after a window port is painted)
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Ends a window port's update. Not exact:
   the original keeps `port` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048b1b4 */
short endPortUpdate()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (port->kind != 5)
        return setPortError(0x2a73);
    return ((windowPort *)port)->endUpdate();
}
