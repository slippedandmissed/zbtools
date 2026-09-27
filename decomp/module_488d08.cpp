/*
 * module_488d08 (Mohawk engine): newPalette
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * A new palette, in graphics.palettes: the static colours at each end and
 * `colors` (all 256 of them, or 0 for the default palette's) between.
 */
/* @zoombi32 0x00488d08 */
Palette *newPalette(unsigned short count, ColorBytes *colors)
{
    Palette *palette;
    unsigned short half;
    unsigned short i;

    if (count > 256) {
        setPortError(0x2a62);
        return 0;
    }
    if ((palette = (Palette *)localAlloc(sizeof(Palette))) == 0) {
        setPortError(fn_46d9c8());
        return 0;
    }
    memset(palette, 0, sizeof(Palette));
    palette->magic = 0x50616c74L; /* 'Palt' */
    palette->first = count <= graphics.paletteReserved / 2 ? 0
                     : 256 - graphics.paletteReserved / 2 >= count
                         ? count - graphics.paletteReserved / 2
                         : 256 - graphics.paletteReserved;
    half = graphics.paletteReserved / 2;
    memcpy(palette->entries, graphics.systemColors, half * sizeof(PALETTEENTRY));
    memcpy(&palette->entries[256 - half], &graphics.systemColors[20 - half],
           half * sizeof(PALETTEENTRY));
    if (colors) {
        for (i = half; i < half + palette->first; i++) {
            if (colors[i].kind == 0xff) {
                localFree(palette);
                setPortError(0x2a62);
                return 0;
            }
            palette->entries[i].peRed = colors[i].red;
            palette->entries[i].peGreen = colors[i].green;
            palette->entries[i].peBlue = colors[i].blue;
            palette->entries[i].peFlags = colors[i].kind & 1 ? 1 : 0;
        }
    } else
        memcpy(&palette->entries[half], &graphics.defaultPalette->entries[half],
               palette->first * sizeof(PALETTEENTRY));
    palette->hpal = createPalette(palette->entries);
    if (!palette->hpal) {
        localFree(palette);
        setPortError(0x2a37);
        return 0;
    }
    if ((palette->next = graphics.palettes) != 0) {
        palette->prev = graphics.palettes->prev;
        graphics.palettes->prev->next = palette;
        graphics.palettes->prev = palette;
    } else
        palette->next = palette->prev = palette;
    graphics.palettes = palette;
    setPortError(0);
    return paletteHandle(palette);
}
