/*
 * module_48b4a8 (Mohawk engine): getPortPalette
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The current port's palette (0 for the default one, -1 with no port).
   Not exact: the original keeps `port` in eax; BCC32 4.5 gives it a saved
   register. */
/* @zoombi32 0x0048b4a8 */
Palette *getPortPalette()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return (Palette *)-1;
    return paletteHandle(port->palette == graphics.defaultPalette ? 0 : port->palette);
}
