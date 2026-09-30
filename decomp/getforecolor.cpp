/*
 * getforecolor (Mohawk engine): getForeColor, getPort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The current port's foreground colour (none with no port). Not exact: the original keeps `port` in eax; BCC32 4.5 gives it a saved
   register. */
/* @zoombi32 0x0048b4d8 */
Color getForeColor()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return Color(0xffff);
    return port->pen.color;
}

/* @zoombi32 0x0048b510 */
basePort *getPort()
{
    return portHandle(graphics.currentPort ? graphics.currentPort : 0);
}
