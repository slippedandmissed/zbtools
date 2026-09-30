/*
 * setport (Mohawk engine): setPort, portHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Makes a locked port (or none) current; the previous one, or -1 on
   error. */
/* @zoombi32 0x0048d960 */
basePort *setPort(basePort *handle)
{
    basePort *port;
    basePort *old;

    if (!handle)
        port = 0;
    else {
        if ((port = checkPort(handle, 0)) == 0)
            return (basePort *)-1;
        if (!port->locks) {
            setPortError(0x2a75);
            return (basePort *)-1;
        }
    }
    setPortError(0);
    old = graphics.currentPort;
    graphics.currentPort = port;
    return portHandle(old ? old : 0);
}

/* @zoombi32 0x0048d9bd */
basePort *portHandle(basePort *port)
{
    return port;
}
