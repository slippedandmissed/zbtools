/*
 * module_488ba8 (Mohawk engine): newPort
 */

/* @flags -p -x- */

#include "zoombinis.h"

/*
 * A new port in memory: a memoryPort in the display's format (depth 0, or
 * 0xffff), an 8-bit DIB, or a DIB of 1, 4, 16 or 24 bits. Depth 1 means
 * 8 on an 8-bit display. A palette of 0 is the default one.
 *
 * Not exact: the original keeps width and height in registers and `pal`
 * and the new objects on the stack; BCC32 4.5 does the opposite.
 */
/* @zoombi32 0x00488ba8 */
basePort *newPort(short width, short height, short depth, Palette *palette)
{
    Palette *pal;
    basePort *port;

    switch ((unsigned short)depth) {
    case 1:
    case 4:
    case 8:
    case 16:
    case 24:
        break;
    case 0xffff:
        depth = 0;
        break;
    case 0:
        if (graphics.depth == 8)
            depth = 8;
        break;
    default:
        setPortError(0x2a62);
        return 0;
    }
    pal = palette ? checkPalette(palette, 1) : graphics.defaultPalette;
    if (!pal) {
        setPortError(0x2a70);
        return 0;
    }
    if (!depth)
        port = new memoryPort(width, height);
    else if (depth == 8)
        port = new DIB8Port(width, height);
    else
        port = new DIBPort(width, height, depth);
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
