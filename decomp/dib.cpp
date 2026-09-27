/*
 * dib (Mohawk engine): DIB (a DIB section ports draw through), and
 * fixed-point helpers
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

RGBQUAD monoColors[2] = {{0, 0, 0, 0}, {0xff, 0xff, 0xff, 0}};
RGBQUAD vgaColors[16] = {
    {0, 0, 0, 0},       {0x80, 0, 0, 0},       {0, 0x80, 0, 0},    {0x80, 0x80, 0, 0},
    {0, 0, 0x80, 0},    {0x80, 0, 0x80, 0},    {0, 0x80, 0x80, 0}, {0xc0, 0xc0, 0xc0, 0},
    {0x80, 0x80, 0x80, 0}, {0xff, 0, 0, 0},    {0, 0xff, 0, 0},    {0xff, 0xff, 0, 0},
    {0, 0, 0xff, 0},    {0xff, 0, 0xff, 0},    {0, 0xff, 0xff, 0}, {0xff, 0xff, 0xff, 0},
};

/* @zoombi32 0x00489148 */
__cdecl DIB::DIB(short width, short height, unsigned short depth)
{
    bounds = Rect(0, 0, width, height);
    this->depth = depth;
    portUsage = DIB_PAL_COLORS;
    indexInfo = 0;
}

/* Draws `from` (clipped to the bitmap) into `to` of a port. A port that
   selects its palette's own HPALETTE takes a StretchBlt from the DIB
   section's DC; others get the bits. */
/* @zoombi32 0x00489198 */
short DIB::draw(basePort *port, Rect to, Rect from, long usage, DWORD rop, unsigned short flipX,
                unsigned short flipY)
{
    short over;
    RECT source;

    if ((over = -from.left) > 0) {
        to.left += fixedRound(fixedMul(makeFixed(over, 0),
                                       fixedDiv(to.right - to.left, from.right - from.left)));
        from.left = 0;
    }
    if ((over = from.right - bounds.right) > 0) {
        to.right -= fixedRound(fixedMul(makeFixed(over, 0),
                                        fixedDiv(to.right - to.left, from.right - from.left)));
        from.right = bounds.right;
    }
    if ((over = -from.top) > 0) {
        to.top += fixedRound(fixedMul(makeFixed(over, 0),
                                      fixedDiv(to.bottom - to.top, from.bottom - from.top)));
        from.top = 0;
    }
    if ((over = from.bottom - bounds.bottom) > 0) {
        to.bottom -= fixedRound(fixedMul(makeFixed(over, 0),
                                         fixedDiv(to.bottom - to.top, from.bottom - from.top)));
        from.bottom = bounds.bottom;
    }
    if (rowBytes < 0)
        flipY = !flipY;
    if (port->paletteKind == 1 && usage == DIB_RGB_COLORS) {
        WinRect device(from);

        source = device;
        DPtoLP(dc, (POINT *)&source, 2);
        port->prepare();
        return setPortError(
            StretchBlt(port->dc, to.left, to.top, to.right - to.left, to.bottom - to.top, dc,
                       flipX ? source.right - 1 : source.left,
                       flipY ? source.bottom - 1 : source.top,
                       flipX ? source.left - source.right : source.right - source.left,
                       flipY ? source.top - source.bottom : source.bottom - source.top, rop)
                ? 0
                : 0x2a37);
    } else {
        unsigned short height = from.bottom - from.top;

        return setPortError(
            port->stretchDIBits(flipX ? to.right : to.left, flipY ? to.bottom - 1 : to.top,
                                flipX ? to.left - to.right : to.right - to.left,
                                flipY ? to.top - to.bottom : to.bottom - to.top, from.left,
                                bounds.bottom - height - from.top, from.right - from.left, height,
                                bits, usage == DIB_PAL_COLORS ? indexInfo : info, usage, rop)
                    > 0
                ? 0
                : 0x2a37);
    }
}

/* @zoombi32 0x0048947d */
HDC DIB::getDC()
{
    setPortError(0);
    return dc;
}

/* @zoombi32 0x00489493 */
void DIB::releaseDC(HDC)
{
}

/* @zoombi32 0x0048949a */
void DIB::getColors(unsigned short first, unsigned short count, RGBQUAD *colors)
{
    memcpy(colors, &info->bmiColors[first], count * sizeof(RGBQUAD));
}

/* Makes the DIB section (cleared, if asked), and two headers for it: one
   with palette indices for colours (identity), one with RGB colours (black
   and white, the VGA colours, or the default palette's). */
/* @zoombi32 0x004894c5 */
short DIB::create(short clear)
{
    unsigned short colors;

    switch (depth) {
    case 1:
        colors = 2;
        rowBytes = (bounds.right + 7) / 8;
        break;
    case 4:
        colors = 16;
        rowBytes = (bounds.right + 1) / 2;
        break;
    case 8:
        colors = 256;
        rowBytes = bounds.right;
        break;
    case 16:
        colors = 0;
        rowBytes = bounds.right * 2;
        break;
    case 24:
        colors = 0;
        rowBytes = bounds.right * 3;
        break;
    default:
        return setPortError(0x2a62);
    }
    rowBytes = (rowBytes + 3) & ~3;
    unsigned short rgbBytes = colors * sizeof(RGBQUAD);
    indexInfo = (BITMAPINFO *)newPtr(sizeof(BITMAPINFOHEADER) + colors * sizeof(short)
                                     + sizeof(BITMAPINFOHEADER) + rgbBytes);
    if (!indexInfo)
        return setPortError(memError());
    memset(indexInfo, 0, sizeof(BITMAPINFOHEADER));
    indexInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    indexInfo->bmiHeader.biWidth = bounds.right;
    indexInfo->bmiHeader.biHeight = bounds.bottom;
    indexInfo->bmiHeader.biPlanes = 1;
    indexInfo->bmiHeader.biBitCount = depth;
    indexInfo->bmiHeader.biCompression = BI_RGB;
    switch (depth) {
    case 1:
        ((unsigned short *)indexInfo->bmiColors)[0] = 0;
        ((unsigned short *)indexInfo->bmiColors)[1] = 0xff;
        break;
    case 4: {
        for (int i = 0; i < 8; i++) {
            ((unsigned short *)indexInfo->bmiColors)[i] = i;
            ((unsigned short *)indexInfo->bmiColors)[15 - i] = 0xff - i;
        }
        break;
    }
    case 8: {
        for (int i = 0; i < 256; i++)
            ((unsigned short *)indexInfo->bmiColors)[i] = i;
        break;
    }
    }
    info = (BITMAPINFO *)((char *)indexInfo + sizeof(BITMAPINFOHEADER) + colors * sizeof(short));
    memcpy(info, indexInfo, sizeof(BITMAPINFOHEADER));
    switch (depth) {
    case 1:
        memcpy(info->bmiColors, monoColors, sizeof(monoColors));
        break;
    case 4:
        memcpy(info->bmiColors, vgaColors, sizeof(vgaColors));
        break;
    case 8: {
        for (int i = 0; i < 256; i++) {
            info->bmiColors[i].rgbRed = graphics.defaultPalette->entries[i].peRed;
            info->bmiColors[i].rgbGreen = graphics.defaultPalette->entries[i].peGreen;
            info->bmiColors[i].rgbBlue = graphics.defaultPalette->entries[i].peBlue;
            info->bmiColors[i].rgbReserved = 0;
        }
        break;
    }
    }
    HDC screen = GetDC(0);
    dc = CreateCompatibleDC(screen);
    ReleaseDC(0, screen);
    if ((bitmap = CreateDIBSection(dc, info, DIB_RGB_COLORS, &bits, 0, 0)) != 0) {
        SelectObject(dc, bitmap);
    } else {
        DeleteDC(dc);
        disposePtr(indexInfo);
        indexInfo = 0;
        return setPortError(0x2a37);
    }
    lastRow = (bounds.bottom - 1) * rowBytes;
    if (clear)
        fillMemory(bits, 0, bounds.bottom * rowBytes);
    return setPortError(0);
}

/* @zoombi32 0x004897a9 */
void DIB::destroy()
{
    GdiFlush();
    DeleteDC(dc);
    DeleteObject(bitmap);
    disposePtr(indexInfo);
}

/* @zoombi32 0x004897d2 */
long DIB::usage(long paletteKind, DWORD, short, short)
{
    return paletteKind == 1 || paletteKind == 2 ? DIB_RGB_COLORS : DIB_PAL_COLORS;
}

/* @zoombi32 0x004897eb */
void DIB::setColors(unsigned short first, unsigned short count, const RGBQUAD *colors)
{
    memcpy(&info->bmiColors[first], colors, count * sizeof(RGBQUAD));
    SetDIBColorTable(dc, 0, 256, info->bmiColors);
}

/* @zoombi32 0x0048982e */
void DIB::setEntries(unsigned short first, short count, const PALETTEENTRY *entries)
{
    RGBQUAD *color = &info->bmiColors[first];

    while (count--) {
        color->rgbRed = entries->peRed;
        color->rgbGreen = entries->peGreen;
        color->rgbBlue = entries->peBlue;
        color++;
        entries++;
    }
    SetDIBColorTable(dc, 0, 256, info->bmiColors);
}

/* @zoombi32 0x0048988a */
long makeFixed(short whole, unsigned short fraction)
{
    return ((long)whole << 16) + fraction;
}

/* @zoombi32 0x0048989e */
short fixedToInt(long value)
{
    return value >> 16;
}

/* @zoombi32 0x004898ab */
unsigned short fixedFraction(long value)
{
    return value;
}

/* To the nearest whole number. */
/* @zoombi32 0x004898b6 */
short fixedRound(long value)
{
    return (fixedFraction(value) >= 0x8000 ? 1 : 0) + fixedToInt(value);
}
