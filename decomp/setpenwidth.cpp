/*
 * setpenwidth (Mohawk engine): setPenWidth
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's pen width; the previous one (0xffff on error). Not
   exact: the original keeps `width` on the stack; BCC32 4.5 gives it edi. */
/* @zoombi32 0x0048d90c */
unsigned short setPenWidth(short width)
{
    basePort *port;
    unsigned short old;

    if ((port = portObject(1)) == 0)
        return 0xffff;
    if (port->setColor(port->pen.color, width))
        return 0xffff;
    old = port->pen.width;
    port->pen.width = width;
    return old;
}
