/*
 * module_48d574 (Mohawk engine): setPortPalette, palette handles
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's palette (0: the default one); the previous one,
   or -1 on error. Not exact: the original keeps `port` in eax; BCC32 4.5
   gives it esi. */
/* @zoombi32 0x0048d574 */
Palette *setPortPalette(Palette *handle)
{
    Palette *palette;
    basePort *port;

    palette = handle ? checkPalette(handle, 1) : graphics.defaultPalette;
    if (!palette) {
        setPortError(0x2a70);
        return (Palette *)-1;
    }
    if ((port = portObject(1)) == 0)
        return (Palette *)-1;
    palette = port->setPalette(palette);
    setPortError(0);
    return paletteHandle(palette == graphics.defaultPalette ? 0 : palette);
}

/* @zoombi32 0x0048d5df */
Palette *paletteHandle(Palette *palette)
{
    return palette;
}
