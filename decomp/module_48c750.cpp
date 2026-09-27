/*
 * module_48c750 (Mohawk engine): lockPort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Not exact: the original keeps `port` in eax; BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048c750 */
short lockPort(basePort *handle)
{
    basePort *port;

    if ((port = checkPort(handle, 0)) == 0)
        return graphics.error;
    return port->lock();
}
