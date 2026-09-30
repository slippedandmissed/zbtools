/*
 * beginportupdate (Mohawk engine): beginPortUpdate (before a window port is painted)
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Takes the current window port's update region as a clip. Not exact: the
   original keeps `port` in eax (no call intervenes where it is
   used); BCC32 4.5 gives it ebx. */
/* @zoombi32 0x004887f4 */
short beginPortUpdate()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (port->kind != 5)
        return setPortError(0x2a73);
    return ((windowPort *)port)->beginUpdate();
}
