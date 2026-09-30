/*
 * setforecolor (Mohawk engine): setForeColor
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's pen colour; the previous one (none on error). Not
   exact: the original swaps the registers of `port` and the hidden result
   pointer (ebx and esi). */
/* @zoombi32 0x0048d884 */
Color setForeColor(Color color)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return Color(0xffff);
    if (port->setColor(color, port->pen.width))
        return Color(0xffff);
    Color old = port->pen.color;
    port->pen.color = color;
    return old;
}
