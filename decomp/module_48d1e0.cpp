/*
 * module_48d1e0 (Mohawk engine): setClip, setCursorLevel
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets the current port's clip region to a copy of `region`. */
/* @zoombi32 0x0048d1e0 */
short setClip(short region)
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return graphics.error;
    if (copyRgn(port->clip, region))
        return setPortError(regionError());
    port->clipApplied = 0;
    return setPortError(0);
}

/* Shows or hides the cursor until its display count is `level`; the count
   it had. */
/* @zoombi32 0x0048d22c */
short setCursorLevel(short level)
{
    short current;
    short previous;

    if (level < 0) {
        current = hideCursor();
        previous = current;
        previous++;
    } else {
        current = showCursor();
        previous = current;
        previous--;
    }
    while (current != level)
        current = current < level ? showCursor() : hideCursor();
    return previous;
}
