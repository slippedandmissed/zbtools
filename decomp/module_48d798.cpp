/*
 * module_48d798 (Mohawk engine): setMinimalReserve
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* Keeps only black and white of the static colours (giving palettes 18
   more), or all 20; the previous setting. */
/* @zoombi32 0x0048d798 */
short setMinimalReserve(short minimal)
{
    short old;
    Palette *palette;
    unsigned short half;

    old = graphics.minimalReserve;
    if (minimal != old) {
        graphics.minimalReserve = minimal;
        graphics.paletteReserved = minimal ? 2 : 20;
        palette = graphics.palettes;
        do {
            if (graphics.paletteReserved == 2) {
                if (palette->first == 236)
                    palette->first = 254;
                else
                    palette->first += 9;
            } else {
                half = graphics.paletteReserved / 2;
                memcpy(palette->entries, graphics.systemColors, half * sizeof(PALETTEENTRY));
                memcpy(&palette->entries[256 - half], &graphics.systemColors[20 - half],
                       half * sizeof(PALETTEENTRY));
                if (palette->first == 254)
                    palette->first = 236;
                else
                    palette->first -= 9;
            }
            palette->realized = 0;
        } while ((palette = palette->next) != graphics.palettes);
    }
    return old;
}
