/*
 * setpalettecolors (Mohawk engine): setPaletteColors, checkPalette
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "graphics.h"

/*
 * Sets `count` of a palette's colours from `first`, leaving the static
 * colours alone. A colour's kind bit 7 says to set PC_RESERVED from bit 0.
 *
 * Not exact: the original loads `first` into esi before `colors` into ebx.
 */
/* @zoombi32 0x0048d5ec */
short setPaletteColors(Palette *handle, unsigned short first, unsigned short count,
                       ColorBytes *colors)
{
    Palette *palette;
    short excess;
    PALETTEENTRY *entry;
    unsigned short i;
    unsigned char flags;

    palette = checkPalette(handle, 1);
    if (!palette)
        return setPortError(0x2a70);
    if (first >= 256 || count > 256 || first + count > 256)
        return setPortError(0x2a62);
    if ((excess = first + count - (graphics.paletteReserved / 2 + palette->first)) > 0) {
        if (256 - graphics.paletteReserved / 2 <= first)
            return setPortError(0);
        count -= excess;
    }
    if ((excess = graphics.paletteReserved / 2 - first) > 0) {
        if (excess >= count)
            return setPortError(0);
        first = graphics.paletteReserved / 2;
        count -= excess;
        colors += excess;
    }
    for (entry = &palette->entries[first], i = first; i < first + count; colors++, entry++, i++) {
        if (colors->kind == 0xff)
            return setPortError(0x2a62);
        entry->peFlags |= 2;
        entry->peRed = colors->red;
        entry->peGreen = colors->green;
        entry->peBlue = colors->blue;
        if (colors->kind & 0x80) {
            flags = entry->peFlags;
            entry->peFlags &= ~1;
            entry->peFlags |= colors->kind & 1 ? 1 : 0;
            if (entry->peFlags != flags)
                palette->realized = 0;
        }
    }
    palette->changed = 1;
    return setPortError(0);
}

/* The palette behind a handle, or 0 if it isn't one. */
/* @zoombi32 0x0048d779 */
Palette *checkPalette(Palette *palette, short kind)
{
    if (!palette || palette == (Palette *)-1 || palette->magic != 0x50616c74L /* 'Palt' */)
        return 0;
    return palette;
}
