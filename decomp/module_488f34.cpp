/*
 * module_488f34 (Mohawk engine): newWindowPort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* A new port drawing into a window. A palette of 0 is the default one. */
/* @zoombi32 0x00488f34 */
basePort *newWindowPort(const Rect &bounds, HWND window, Palette *palette)
{
    Palette *pal;
    basePort *port;

    pal = palette ? checkPalette(palette, 1) : graphics.defaultPalette;
    if (!pal) {
        setPortError(0x2a70);
        return 0;
    }
    if (!IsWindow(window)) {
        setPortError(0x2a62);
        return 0;
    }
    port = new windowPort(bounds, window);
    if (!port) {
        setPortError(0x2a37);
        return 0;
    }
    if (port->init()) {
        delete port;
        return 0;
    }
    port->setPalette(pal);
    setPortError(0);
    return portHandle(port);
}
