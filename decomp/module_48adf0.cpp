/*
 * module_48adf0 (Mohawk engine): drawing images
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Draws an image resource (a header of four fields, then its pixels) at
   x, y in the current port. */
/* @zoombi32 0x0048adf0 */
short drawImageData(unsigned short *image, short x, short y, short mode)
{
    if (!image)
        return setPortError(0x2a63);
    return drawPixelData(fn_492730(image[0]), fn_492730(image[1]), fn_492730(image[2]),
                         fn_492730(image[3]), image + 4, x, y, mode);
}

/* @zoombi32 0x0048ae53 */
short drawPixelData(short width, short height, short unknown, unsigned short flags, void *pixels,
                    short x, short y, short mode)
{
    basePort *port;

    if ((port = portObject(8)) == 0)
        return graphics.error;
    if (flags & 0xf00)
        return setPortError(0x2a63);
    return port->drawPixels(Rect(x, y, x + width, y + height), width, height, unknown, flags,
                            pixels, mode, 0);
}
