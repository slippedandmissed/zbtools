/*
 * disposepalette (Mohawk engine): deletePalette, disposePalette
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_localmem.h"

/* Disposes of a palette no port uses. Not exact: the original keeps `palette` in eax (no call intervenes where it is
   used); BCC32 4.5 gives it ebx. */
/* @zoombi32 0x0048906c */
short deletePalette(Palette *handle)
{
    Palette *palette;

    if ((palette = checkPalette(handle, 1)) == 0)
        return setPortError(0x2a70);
    if (palette->ports)
        return setPortError(0x2a6f);
    disposePalette(palette);
    return setPortError(0);
}

/* @zoombi32 0x004890ac */
void disposePalette(Palette *palette)
{
    palette->prev->next = palette->next;
    palette->next->prev = palette->prev;
    if (palette == graphics.palettes)
        graphics.palettes = palette == palette->next ? 0 : palette->next;
    DeleteObject(palette->hpal);
    palette->magic = 0;
    localFree(palette);
}
