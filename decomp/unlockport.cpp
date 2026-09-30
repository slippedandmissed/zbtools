/*
 * unlockport (Mohawk engine): unlockPort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Unlocks a port; once unlocked, it is no longer current. */
/* @zoombi32 0x0048db08 */
short unlockPort(basePort *handle)
{
    basePort *port;

    if ((port = checkPort(handle, 0)) == 0)
        return graphics.error;
    if (!port->locks)
        return setPortError(0x2a75);
    port->unlock();
    if (!port->locks && port == graphics.currentPort)
        graphics.currentPort = 0;
    return setPortError(0);
}
