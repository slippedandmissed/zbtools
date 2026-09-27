/*
 * module_4890f8 (Mohawk engine): deletePort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Disposes of a port that isn't locked. */
/* @zoombi32 0x004890f8 */
short deletePort(basePort *handle)
{
    basePort *port;

    if ((port = checkPort(handle, 0)) == 0)
        return graphics.error;
    if (port->locks)
        return setPortError(0x2a74);
    port->release();
    delete port;
    return setPortError(0);
}
