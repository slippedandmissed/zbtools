/*
 * getfont (Mohawk engine): getFont, nearestPaletteIndex
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The current port's font (0 for the default one, or with no port). Not exact: the original keeps `port` in eax; BCC32 4.5 gives it a saved
   register. */
/* @zoombi32 0x0048b360 */
Font *getFont()
{
    basePort *port;

    if ((port = portObject(1)) == 0)
        return 0;
    return fontHandle(port->font == graphics.defaultFont ? 0 : port->font);
}

/*
 * The index of the palette entry nearest `color` (the smallest sum of
 * squared differences). The colour's kind picks the candidates: bits 2-3
 * 4 for the static colours only, 8 for all but them, else the static
 * colours and the palette's own; bits 0-1 1 for PC_RESERVED entries only, 2
 * for the others.
 *
 * The original is in assembly (it saves registers in the middle of the
 * function and squares bytes with `mul al`); this does the same in C++.
 */
/* @zoombi32-functional 0x0048b38c */
unsigned short nearestPaletteIndex(Palette *palette, RGBColor color)
{
    unsigned short end;
    unsigned short low;
    unsigned short high;
    unsigned short best;
    unsigned char which;
    unsigned char reserved;
    unsigned long bestDistance;
    unsigned long distance;
    PALETTEENTRY *entry;
    unsigned short i;
    unsigned char difference;

    end = graphics.paletteReserved / 2 + palette->first;
    low = graphics.paletteReserved / 2;
    high = 256 - graphics.paletteReserved / 2;
    best = 0;
    which = color.bytes.kind & 0xc;
    reserved = color.bytes.kind & 3;
    bestDistance = 0xffffffffUL;
    entry = palette->entries;
    for (i = 0; i < 256; i++, entry++) {
        if (which == 4) {
            if (i >= low && i < high)
                continue;
        } else if (which == 8) {
            if (i < low || i >= high)
                continue;
        } else if (i < high && i >= end)
            continue;
        if (reserved == 1 ? !(entry->peFlags & 1) : reserved == 2 && (entry->peFlags & 1))
            continue;
        difference = entry->peRed > color.bytes.red ? entry->peRed - color.bytes.red
                                                    : color.bytes.red - entry->peRed;
        distance = (unsigned short)(difference * difference);
        difference = entry->peGreen > color.bytes.green ? entry->peGreen - color.bytes.green
                                                        : color.bytes.green - entry->peGreen;
        distance += (unsigned short)(difference * difference);
        difference = entry->peBlue > color.bytes.blue ? entry->peBlue - color.bytes.blue
                                                      : color.bytes.blue - entry->peBlue;
        distance += (unsigned short)(difference * difference);
        if (distance < bestDistance) {
            best = i;
            bestDistance = distance;
        }
    }
    return best;
}
