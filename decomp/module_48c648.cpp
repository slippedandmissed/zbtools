/*
 * module_48c648 (Mohawk engine): lineTo
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048c648 */
short lineTo(short x, short y)
{
    basePort *port;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    port->prepare();
    LineTo(port->dc, x, y);
    return setPortError(0);
}
