/*
 * module_48d9c8 (Mohawk engine): setOrigin, portObject
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Moves the current port's origin. */
/* @zoombi32 0x0048d9c8 */
short setOrigin(short left, short top)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    return port->setFrame(&port->unknown18, Pt(left, top), port->unknown2c);
}

/* @zoombi32 0x0048da17 */
__cdecl Pt::Pt(short x, short y)
{
    this->x = x;
    this->y = y;
}

/* The current port, if it is of a kind; else 0 (the error in
   graphics.error). */
/* @zoombi32 0x0048da2e */
basePort *portObject(short kind)
{
    return checkPort(graphics.currentPort, kind);
}
