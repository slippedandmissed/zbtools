/*
 * module_48da48 (Mohawk engine): setTakeStatic
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Whether to take over the static colours while active; the previous
   setting. */
/* @zoombi32 0x0048da48 */
short setTakeStatic(short take)
{
    short old;
    HDC dc;
    Palette *palette;

    old = graphics.takeStatic;
    if (take != old) {
        graphics.takeStatic = take;
        dc = GetDC(0);
        setSystemPaletteUse(dc, take ? SYSPAL_NOSTATIC : SYSPAL_STATIC);
        ReleaseDC(0, dc);
        palette = graphics.palettes;
        do
            palette->realized = 0;
        while ((palette = palette->next) != graphics.palettes);
    }
    return old;
}
