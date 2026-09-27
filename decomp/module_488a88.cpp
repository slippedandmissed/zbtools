/*
 * module_488a88 (Mohawk engine): copyPortBits
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Copies a rectangle of one port to a rectangle of another. */
/* @zoombi32 0x00488a88 */
short copyPortBits(basePort *to, basePort *from, const Rect &toRect, const Rect &fromRect,
                   short mode)
{
    basePort *toPort;
    basePort *fromPort;

    if ((toPort = checkPort(to, 8)) == 0 || (fromPort = checkPort(from, 4)) == 0)
        return graphics.error;
    return fromPort->copyBits(toPort, &toRect, &fromRect, mode, 0);
}
