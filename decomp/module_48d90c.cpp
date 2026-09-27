/*
 * module_48d90c (Mohawk engine): setPenMode
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's pen mode; the previous one (0xffff on error). Not
   exact: the original keeps `mode` on the stack; BCC32 4.5 gives it edi. */
/* @zoombi32 0x0048d90c */
unsigned short setPenMode(short mode)
{
    basePort *port;
    unsigned short old;

    if ((port = portObject(1)) == 0)
        return 0xffff;
    if (port->setColor(port->foreColor, mode))
        return 0xffff;
    old = port->mode;
    port->mode = mode;
    return old;
}
