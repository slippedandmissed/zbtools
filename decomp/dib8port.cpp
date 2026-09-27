/*
 * dib8port (Mohawk engine): DIB8Port, an 8-bit DIB port that draws into its
 * bits directly (through the clip region's rectangles) where it can, and
 * its blitters
 */

/* @flags -p -x- */

#include <string.h>
#define RECT_OUT_OF_LINE
#include "zoombinis.h"

/*
 * Clips a copy of 8-bit pixels to `bounds` and `clip`: x and y (the pixels'
 * top left) move in and width and height shrink; skipX and skipY say by how
 * much. 0 if nothing is left.
 */
static int clipPixels(Rect &bounds, const Rect &clip, short &x, short &y,
                       unsigned short &width, unsigned short &height, unsigned short &skipX,
                       unsigned short &skipY)
{
    short over;

    if (!sectRect(&bounds, (ShortRect *)&clip))
        return 0;
    skipX = 0;
    if ((over = x - bounds.left) < 0) {
        skipX = -over;
        x += skipX;
        if (width <= skipX)
            return 0;
        width -= skipX;
    }
    skipY = 0;
    if ((over = y - bounds.top) < 0) {
        skipY = -over;
        y += skipY;
        if (height <= skipY)
            return 0;
        height -= skipY;
    }
    if ((unsigned short)(x + width) > (unsigned short)bounds.right) {
        over = x + width - bounds.right;
        if (width <= (unsigned short)over)
            return 0;
        width -= over;
    }
    if ((unsigned short)(y + height) > (unsigned short)bounds.bottom) {
        over = y + height - bounds.bottom;
        if (height <= (unsigned short)over)
            return 0;
        height -= over;
    }
    return 1;
}

/*
 * Copies 8-bit pixels (`fromRowBytes` apart, negative for bottom-up rows)
 * into an 8-bit bottom-up bitmap (`bits` + `offset` is its top row) at x, y,
 * clipped to `bounds` and `clip`; with `transparent`, pixels of 0 are
 * skipped.
 *
 * The original is in assembly (it picks its inner loop, `rep movs` or a
 * byte loop at 0x4899ed, by jumping through ebx); this does the same in C++.
 */
/* @zoombi32-functional 0x0048990c */
void copyPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x, short y,
                const Rect &clip, unsigned short width, unsigned short height, long fromRowBytes,
                const unsigned char *from, short transparent)
{
    unsigned short rows = height;
    unsigned short skipX;
    unsigned short skipY;
    unsigned char *row;
    unsigned short i;

    if (!clipPixels(bounds, clip, x, y, width, height, skipX, skipY))
        return;
    from += skipX + (long)(short)((fromRowBytes < 0 ? 1 - rows : 0) + skipY) * fromRowBytes;
    row = bits + offset - rowBytes * (unsigned short)y + (unsigned short)x;
    for (;;) {
        if (transparent) {
            for (i = 0; i < width; i++)
                if (from[i])
                    row[i] = from[i];
        } else {
            memcpy(row, from, width);
        }
        if (!--height)
            return;
        row -= rowBytes;
        from += fromRowBytes;
    }
}

/*
 * Fills `area`, clipped to `bounds`, of an 8-bit bottom-up bitmap with a
 * colour. The original is in assembly (`rep stos`).
 */
/* @zoombi32-functional 0x00489a2d */
void fillPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, const Rect &area,
                unsigned char value)
{
    unsigned char *row;
    unsigned short rows;

    if (!sectRect(&bounds, (ShortRect *)&area))
        return;
    row = bits + offset - rowBytes * (unsigned short)bounds.top + (unsigned short)bounds.left;
    rows = bounds.bottom - bounds.top;
    do {
        memset(row, value, (unsigned short)(bounds.right - bounds.left));
        row -= rowBytes;
    } while (--rows);
}

/*
 * Draws a 1-bit image (rows `fromRowBytes` apart, most significant bit
 * first) into an 8-bit bottom-up bitmap as copyPixels does: set bits in
 * `color`, clear ones left alone. The original is in assembly (it reads the
 * bits a big-endian dword at a time).
 */
/* @zoombi32-functional 0x00489aad */
void expandBits(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x, short y,
                const Rect &clip, unsigned short width, unsigned short height, long fromRowBytes,
                const unsigned char *from, unsigned char color)
{
    unsigned short rows = height;
    unsigned short skipX;
    unsigned short skipY;
    unsigned char *row;
    unsigned short i;
    unsigned long bit;

    if (!clipPixels(bounds, clip, x, y, width, height, skipX, skipY))
        return;
    row = bits + offset - rowBytes * (unsigned short)y + (unsigned short)x;
    from += (long)(short)((fromRowBytes < 0 ? 1 - rows : 0) + skipY) * fromRowBytes;
    do {
        for (i = 0; i < width; i++) {
            bit = skipX + i;
            if (from[bit >> 3] & (0x80 >> (bit & 7)))
                row[i] = color;
        }
        row -= rowBytes;
        from += fromRowBytes;
    } while (--height);
}

/* @zoombi32 0x00489be4 */
__cdecl DIB8Port::DIB8Port(short width, short height) : DIBPort(width, height, 8)
{
    kind = 1;
}

/* Copies straight between the bits of two 8-bit DIB ports sharing a palette
   (unscaled, unflipped, the same size, mode 0). */
/* @zoombi32 0x00489c12 */
short DIB8Port::copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                         unsigned short flags)
{
    Rect clipped;

    if (!(port == this && sectRect(&(clipped = *from), (ShortRect *)to)) && !mode && !scaled
        && !port->scaled && port->kind == 1 && port->palette == palette && !(flags & 0x30)
        && to->right - to->left == from->right - from->left
        && to->bottom - to->top == from->bottom - from->top) {
        Rect dest = *to;
        Rect source = *from;

        offsetRect(&source, -offset.x, -offset.y);
        if (source.left < 0) {
            dest.left -= source.left;
            source.left = 0;
        }
        if (dest.right - dest.left + source.left > dib->bounds.right)
            dest.right -= dest.right - dest.left + source.left - dib->bounds.right;
        if (source.top < 0) {
            dest.top -= source.top;
            source.top = 0;
        }
        if (dest.bottom - dest.top + source.top > dib->bounds.bottom)
            dest.bottom -= dest.bottom - dest.top + source.top - dib->bounds.bottom;
        if (dest.left < dest.right && dest.top < dest.bottom)
            ((DIB8Port *)port)
                ->drawBits(dest.left - source.left, dest.top - source.top, dib->bounds.right,
                           dib->bounds.bottom, -dib->rowBytes, dib->bits, &dest);
        return setPortError(0);
    }
    return DIBPort::copyBits(port, to, from, mode, flags);
}

/* @zoombi32 0x00489e27 */
void DIB8Port::drawBits(short x, short y, unsigned short width, unsigned short height,
                        long rowBytes, const void *bits, const Rect *clip)
{
    Region *data;
    long i;

    x -= offset.x;
    y -= offset.y;
    flush();
    Rect part;
    data = (Region *)handleData(this->clip);
    for (i = 0; i < data->count; i++)
        if (sectRect(&(part = *clip), &data->rects[i]))
            copyPixels((unsigned char *)dib->bits, dib->lastRow, dib->rowBytes, dib->bounds, x, y,
                       *offsetRect(&part, -offset.x, -offset.y), width, height, rowBytes,
                       (const unsigned char *)bits, 0);
}

/* @zoombi32 0x00489f1f */
void DIB8Port::realizePalette()
{
    if (palette != realized || !palette->realized || palette->changed) {
        dib->setEntries(0, 256, palette->entries);
        if (paletteKind != 1) {
            SetPaletteEntries(hpal, 0, 256, palette->entries);
            RealizePalette(dc);
        }
    }
    realized = palette;
}

/* Draws 1-bit pixels in mode 8, and 8-bit pixels (plain or packed) in modes
   0 and 8, into the bits; the rest as any port does. */
/* @zoombi32 0x00489f90 */
short DIB8Port::drawPixels(const Rect &bounds, unsigned short width, unsigned short height,
                           short rowBytes, unsigned short format, void *pixels,
                           unsigned short mode, unsigned short flags)
{
    if (!scaled && bounds.right - bounds.left == width && bounds.bottom - bounds.top == height
        && !(flags & 0x30)) {
        short x = bounds.left - offset.x;
        short y = bounds.top - offset.y;
        Rect part;
        Region *data;
        long i;

        switch (format & 0xf) {
        case 0:
            if (!(format & 0xf0) && mode == 8) {
                flush();
                for (i = 0, data = (Region *)handleData(clip); i < data->count; i++)
                    expandBits((unsigned char *)dib->bits, dib->lastRow, dib->rowBytes,
                               dib->bounds, x, y,
                               *offsetRect(&(part = data->rects[i]), -offset.x, -offset.y), width,
                               height, rowBytes, (const unsigned char *)pixels,
                               pen.color.paletteIndex());
                return setPortError(0);
            }
            break;
        case 2:
            if (mode == 0 || mode == 8) {
                switch (format & 0xf0) {
                case 0:
                    flush();
                    for (i = 0, data = (Region *)handleData(clip); i < data->count; i++)
                        copyPixels((unsigned char *)dib->bits, dib->lastRow, dib->rowBytes,
                                   dib->bounds, x, y,
                                   *offsetRect(&(part = data->rects[i]), -offset.x, -offset.y),
                                   width, height, rowBytes, (const unsigned char *)pixels,
                                   mode == 8);
                    return setPortError(0);
                case 0x10:
                    if (rowBytes < 0)
                        break;
                    flush();
                    for (i = 0, data = (Region *)handleData(clip); i < data->count; i++)
                        drawPackedPixels((unsigned char *)dib->bits, dib->lastRow, dib->rowBytes,
                                         dib->bounds, x, y,
                                         *offsetRect(&(part = data->rects[i]), -offset.x,
                                                     -offset.y),
                                         width, height, (const unsigned char *)pixels,
                                         mode == 8);
                    return setPortError(0);
                }
            }
            break;
        }
    }
    return basePort::drawPixels(bounds, width, height, rowBytes, format, pixels, mode, flags);
}

/* With palette indices, draws through an identity palette. */
/* @zoombi32 0x0048a2f2 */
int DIB8Port::stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                            int fromWidth, int fromHeight, const void *bits, BITMAPINFO *info,
                            UINT usage, DWORD rop)
{
    if (usage == DIB_PAL_COLORS) {
        PALETTEENTRY identity[256];
        PALETTEENTRY *entry = identity;
        HPALETTE copy;
        int result;

        for (int i = 0; i < 256; i++, entry++) {
            entry->peRed = i;
            entry->peGreen = i;
            entry->peBlue = i;
            entry->peFlags = 0;
        }
        copy = createPalette(identity);
        SelectPalette(dc, copy, TRUE);
        dib->setEntries(0, 256, identity);
        result = basePort::stretchDIBits(toX, toY, toWidth, toHeight, fromX, fromY, fromWidth,
                                         fromHeight, bits, info, usage, rop);
        dib->setEntries(0, 256, palette->entries);
        SelectPalette(dc, hpal, TRUE);
        DeleteObject(copy);
        return result;
    }
    return basePort::stretchDIBits(toX, toY, toWidth, toHeight, fromX, fromY, fromWidth,
                                   fromHeight, bits, info, usage, rop);
}

/* @zoombi32 0x0048a3f6 */
Color DIB8Port::getPixel(short x, short y)
{
    long row;

    setPortError(0);
    if (!ptInRect(&frame, Pt(x, y)))
        return Color(RGBColor(0, 0, 0, 0));
    POINT device;
    device.x = x;
    device.y = y;
    LPtoDP(dc, &device, 1);
    if (device.x < 0 || device.x >= dib->bounds.right || device.y < 0
        || device.y >= dib->bounds.bottom)
        return Color(RGBColor(0, 0, 0, 0));
    if (dib->rowBytes < 0)
        row = -dib->rowBytes * device.y;
    else
        row = (dib->bounds.bottom - (device.y + 1)) * dib->rowBytes;
    return Color(((unsigned char *)dib->bits)[row + device.x]);
}

/* @zoombi32 0x0048a51a */
short DIB8Port::fillRect(const Rect &rect, Color color, short mode)
{
    if (!mode && !scaled) {
        Rect part;
        Rect area = rect;
        Region *data;
        long i;

        offsetRect(&area, -offset.x, -offset.y);
        flush();
        for (i = 0, data = (Region *)handleData(clip); i < data->count; i++)
            if (sectRect(offsetRect(&(part = data->rects[i]), -offset.x, -offset.y), &area))
                fillPixels((unsigned char *)dib->bits, dib->lastRow, dib->rowBytes, dib->bounds,
                           part, color.paletteIndex());
        return setPortError(0);
    }
    return basePort::fillRect(rect, color, mode);
}

/* Colours are indices into the DIB's colour table. */
/* @zoombi32 0x0048a640 */
COLORREF DIB8Port::colorRef(Color color)
{
    unsigned short index = color.paletteIndex();

    return index == 0xffff ? CLR_INVALID : 0x10ff0000 | index; /* DIBINDEX */
}

/* @zoombi32 0x0048a667 */
__cdecl Rect::Rect(const ShortRect &rect)
{
    memcpy(this, &rect, sizeof(Rect));
}

/* @zoombi32 0x0048a681 */
void DIB8Port::flush()
{
    if (gdiPending) {
        GdiFlush();
        gdiPending = 0;
    }
}

/* @zoombi32-implicit 0x0048a6f9 DIB8Port::~DIB8Port */
