/*
 * canusedisplaymode (Mohawk engine): canUseDisplayMode
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "graphics.h"

/*
 * Whether there is a display mode suiting `mode` (switching to it if
 * `change`, else only the current one); if so, fills `mode` in with it.
 */
/* @zoombi32 0x0048c9e8 */
short canUseDisplayMode(DisplayMode *mode, short change)
{
    DeviceMode found;
    short ok;

    if (change)
        ok = findDisplayMode(mode, &found);
    else {
        currentDisplayMode(&found);
        ok = (mode->width == 0xffff || mode->width <= found.dmPelsWidth)
             && (mode->height == 0xffff || mode->height <= found.dmPelsHeight)
             && (mode->colors == 0xffffffffUL
                 || mode->colors
                        <= 1UL << (found.dmBitsPerPel < 24 ? found.dmBitsPerPel : 24))
             && (!mode->palettized || found.dmBitsPerPel == 8);
    }
    if (ok) {
        mode->width = found.dmPelsWidth;
        mode->height = found.dmPelsHeight;
        mode->colors = 1L << (found.dmBitsPerPel < 24 ? found.dmBitsPerPel : 24);
        mode->palettized = found.dmBitsPerPel == 8;
    }
    return ok;
}
