/*
 * baseport (Mohawk engine): basePort, the ports' base class (drawing
 * through a DC with GDI), and its pen
 */

/* @flags -p -x- */

#include <stdlib.h>
#include <string.h>
#define RECT_OUT_OF_LINE
#include "zoombinis.h"
#include "graphics.h"
#include "os_fixed.h"
#include "os_localmem.h"

DWORD patternRops[8] = {PATCOPY, 0xa000c9, 0xfa0089, PATINVERT,
                        0x0f0001, 0x0a0329, 0xaf0229, 0xa50065};
DWORD copyRops[8] = {SRCCOPY,    SRCAND,   SRCPAINT, SRCINVERT,
                     NOTSRCCOPY, 0x220326, MERGEPAINT, 0x990066};

/* @zoombi32 0x00486318 */
__cdecl basePort::basePort(const Rect &bounds)
{
    tag = 0x506f7274L; /* 'Port' */
    setFrame(&bounds, Pt(0, 0), Pt(0, 0));
    if ((next = graphics.ports) != 0)
        next->prev = this;
    prev = 0;
    graphics.ports = this;
}

/* @zoombi32 0x004863bb */
__cdecl basePort::~basePort()
{
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        graphics.ports = next;
    tag = 0;
}

/* Sets the port's bounds (on its device) and the frame drawn in: from
   `origin`, of `size` (or, where 0, the bounds' size); the frame is scaled
   to the bounds. */
/* @zoombi32 0x00486407 */
short basePort::setFrame(const Rect *bounds, Pt origin, Pt size)
{
    this->bounds = *bounds;
    this->size = size;
    frame.left = origin.x;
    frame.right = (size.x ? size.x : bounds->right - bounds->left) + origin.x;
    frame.top = origin.y;
    frame.bottom = (size.y ? size.y : bounds->bottom - bounds->top) + origin.y;
    offset.x = frame.left - bounds->left;
    offset.y = frame.top - bounds->top;
    scaleX = fixedDiv(bounds->right - bounds->left, frame.right - frame.left);
    scaleY = fixedDiv(bounds->bottom - bounds->top, frame.bottom - frame.top);
    scaled = !(scaleX == 0x10000 && scaleY == 0x10000);
    if (!locks) {
        setPortError(0);
    } else {
        applyMapping();
        clipApplied = 0;
    }
    return graphics.error;
}

/* Copies with StretchBlt between ports that share a palette and depth, else
   through a DIB. Not exact: the original saves edi around the copy of the
   converted rectangle. */
/* @zoombi32 0x00486511 */
short basePort::copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                         unsigned short flags)
{
    DWORD rop;
    unsigned short width;
    unsigned short height;
    unsigned short trueColor;
    HBITMAP bitmap;
    RGBQUAD *bits;
    UINT usage;
    RECT source;

    prepare();
    SetStretchBltMode(port->dc, flags & 1 ? BLACKONWHITE : flags & 2 ? WHITEONBLACK : COLORONCOLOR);
    if (port->palette == palette && port->paletteKind == paletteKind && port->depth == depth) {
        rop = copyRops[mode];
        port->prepare();
        StretchBlt(port->dc, to->left, to->top, to->right - to->left, to->bottom - to->top, dc,
                   flags & 0x10 ? from->right - 1 : from->left,
                   flags & 0x20 ? from->bottom - 1 : from->top,
                   flags & 0x10 ? from->left - from->right : from->right - from->left,
                   flags & 0x20 ? from->top - from->bottom : from->bottom - from->top, rop);
    } else {
        WinRect device(*from);
        HDC memory;
        BITMAPINFO *info;

        source = device;
        LPtoDP(dc, (POINT *)&source, 2);
        OffsetRect(&source, offset.x, offset.y);
        width = source.right - source.left;
        height = source.bottom - source.top;
        trueColor = depth > 8;
        bitmap = CreateCompatibleBitmap(dc, width, height);
        if (!bitmap)
            return setPortError(0x2a37);
        memory = CreateCompatibleDC(dc);
        if (!memory) {
            DeleteObject(bitmap);
            return setPortError(0x2a37);
        }
        SelectObject(memory, bitmap);
        SelectPalette(memory, hpal, TRUE);
        if (mapMode != MM_TEXT)
            SetMapMode(dc, MM_TEXT);
        BitBlt(memory, 0, 0, width, height, dc, source.left, source.top, SRCCOPY);
        if (mapMode != MM_TEXT)
            SetMapMode(dc, mapMode);
        DeleteDC(memory);
        info = (BITMAPINFO *)newPtr(
            trueColor ? ((width * 3 + 3) & ~3) * height + sizeof(BITMAPINFOHEADER)
                      : ((width + 3) & ~3) * height + sizeof(BITMAPINFOHEADER)
                            + 256 * sizeof(RGBQUAD));
        if (!info) {
            DeleteObject(bitmap);
            return setPortError(memError());
        }
        memset(info, 0, sizeof(BITMAPINFOHEADER));
        info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info->bmiHeader.biWidth = width;
        info->bmiHeader.biHeight = height;
        info->bmiHeader.biPlanes = 1;
        info->bmiHeader.biBitCount = trueColor ? 24 : 8;
        info->bmiHeader.biCompression = BI_RGB;
        bits = (RGBQUAD *)((char *)info
                           + ((trueColor ? 0 : 256) * sizeof(RGBQUAD) + sizeof(BITMAPINFOHEADER)));
        usage = port->palette != palette || trueColor ? DIB_RGB_COLORS : DIB_PAL_COLORS;
        GetDIBits(dc, bitmap, 0, height, bits, info, usage);
        DeleteObject(bitmap);
        port->stretchDIBits(flags & 0x10 ? to->right : to->left,
                            flags & 0x20 ? to->bottom - 1 : to->top,
                            flags & 0x10 ? to->left - to->right : to->right - to->left,
                            flags & 0x20 ? to->top - to->bottom : to->bottom - to->top, 0, 0,
                            width, height, bits, info, usage, copyRops[mode]);
        disposePtr(info);
    }
    return setPortError(0);
}

/* @zoombi32 0x004868bd */
void basePort::applyMapping()
{
    SetMapMode(dc, mapMode = scaled ? MM_ANISOTROPIC : MM_TEXT);
    SetViewportOrgEx(dc, bounds.left, bounds.top, 0);
    SetViewportExtEx(dc, bounds.right - bounds.left, bounds.bottom - bounds.top, 0);
    SetWindowOrgEx(dc, frame.left, frame.top, 0);
    SetWindowExtEx(dc, frame.right - frame.left, frame.bottom - frame.top, 0);
}

/* @zoombi32 0x0048694c */
short basePort::clipTo(HRGN rgn)
{
    HRGN combined;

    if (!clipApplied && toHrgn(clipRgn, clip))
        return graphics.error;
    if ((combined = CreateRectRgn(0, 0, 0, 0)) == 0)
        return setPortError(0x2a37);
    if (!CombineRgn(combined, clipRgn, rgn, RGN_AND)) {
        DeleteObject(combined);
        return setPortError(0x2a37);
    }
    DeleteObject(clipRgn);
    clipRgn = combined;
    SelectClipRgn(dc, clipRgn);
    IntersectClipRect(dc, frame.left, frame.top, frame.right, frame.bottom);
    clipApplied = 0;
    return setPortError(0);
}

/* Reads the DC's depth and selects the port's state into it. */
/* @zoombi32 0x00486a08 */
short basePort::setupDC()
{
    rasterCaps = GetDeviceCaps(dc, RASTERCAPS);
    depth = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
    depth = depth < 24 ? depth : 24;
    applyMapping();
    if ((clipRgn = CreateRectRgn(0, 0, 0, 0)) != 0)
        clipApplied = 0;
    else
        return setPortError(0x2a37);
    if (paletteKind == 1) {
        hpal = palette->hpal;
    } else if ((hpal = createPalette(palette->entries)) == 0) {
        DeleteObject(clipRgn);
        return setPortError(0x2a37);
    }
    realized = 0;
    SelectPalette(dc, hpal, TRUE);
    realizePalette();
    basePort *volatile previous = graphics.currentPort; /* (the original keeps it in memory) */
    graphics.currentPort = this;
    locks++;
    SetBkMode(dc, TRANSPARENT);
    setBackColor(backColor);
    SetTextAlign(dc, TA_BASELINE | TA_UPDATECP);
    MoveToEx(dc, pen.position.x, pen.position.y, 0);
    hpen = 0;
    setMode(pen.mode);
    setColor(pen.color, pen.width);
    hfont = 0;
    useFont(font);
    locks--;
    graphics.currentPort = previous;
    return setPortError(0);
}

/* @zoombi32 0x00486b82 */
short basePort::toHrgn(HRGN target, short region)
{
    HRGN other;
    HRGN piece;
    unsigned short swapped;
    HRGN swap;
    Region *data;
    long i;

    if (!scaled) {
        regionToHrgn(target, region, -offset.x, -offset.y);
    } else {
        if ((other = CreateRectRgn(0, 0, 0, 0)) == 0)
            return setPortError(0x2a37);
        piece = CreateRectRgn(0, 0, 0, 0);
        if (!piece) {
            DeleteObject(other);
            return setPortError(0x2a37);
        }
        swapped = 0;
        SetRectRgn(target, 0, 0, 0, 0);
        data = (Region *)handleData(region);
        for (i = 0; i < data->count; i++) {
            WinRect device(Rect(data->rects[i]));

            LPtoDP(dc, (POINT *)&device, 2);
            SetRectRgn(piece, device.left, device.top, device.right, device.bottom);
            CombineRgn(other, target, piece, RGN_OR);
            swapped = !swapped;
            swap = other;
            other = target;
            target = swap;
        }
        if (swapped) {
            target = other;
            other = swap;
            CombineRgn(target, other, 0, RGN_COPY);
        }
        DeleteObject(piece);
        DeleteObject(other);
    }
    return setPortError(0);
}

/* @zoombi32 0x00486a01 */
void basePort::depthChanged()
{
}

/* Takes the port's state out of the DC. */
/* @zoombi32 0x00486cd9 */
void basePort::cleanupDC()
{
    POINT position;

    GetCurrentPositionEx(dc, &position);
    pen.position.x = position.x;
    pen.position.y = position.y;
    SelectObject(dc, GetStockObject(BLACK_PEN));
    DeleteObject(hpen);
    hpen = 0;
    SelectObject(dc, GetStockObject(DEVICE_DEFAULT_FONT));
    DeleteObject(hfont);
    hfont = 0;
    DeleteObject(clipRgn);
    clipRgn = 0;
    SelectPalette(dc, (HPALETTE)GetStockObject(DEFAULT_PALETTE), FALSE);
    if (paletteKind != 1)
        DeleteObject(hpal);
    hpal = 0;
}

/* @zoombi32 0x00486d7e */
void basePort::prepare()
{
    if (!clipApplied && !setClip(clip))
        clipApplied = 1;
}

/* @zoombi32 0x00486da7 */
void basePort::realizePalette()
{
    if (palette != realized || !palette->realized || palette->changed)
        SetPaletteEntries(hpal, 0, 256, palette->entries);
    RealizePalette(dc);
    realized = palette;
}

/* @zoombi32 0x00486df3 */
short basePort::setBackColor(Color color)
{
    COLORREF ref = colorRef(color);

    if (ref == CLR_INVALID)
        return graphics.error;
    SetBkColor(dc, ref);
    return setPortError(0);
}

/* @zoombi32 0x00486e36 */
short basePort::setClip(short region)
{
    if (!toHrgn(clipRgn, region)) {
        SelectClipRgn(dc, clipRgn);
        IntersectClipRect(dc, frame.left, frame.top, frame.right, frame.bottom);
    }
    return graphics.error;
}

/* @zoombi32 0x00486e82 */
short basePort::useFont(Font *font)
{
    LOGFONT logFont;
    HFONT handle;

    memset(&logFont, 0, sizeof(logFont));
    logFont.lfCharSet = DEFAULT_CHARSET;
    strcpy(logFont.lfFaceName, font->name);
    logFont.lfHeight = -font->size;
    logFont.lfWeight = font->style & 1 ? FW_BOLD : FW_NORMAL;
    logFont.lfItalic = (font->style & 2) != 0;
    logFont.lfUnderline = (font->style & 4) != 0;
    if (font->rotated) {
        logFont.lfOutPrecision |= OUT_TT_ONLY_PRECIS;
        logFont.lfEscapement = fixedRound(fixedMul(makeFixed(3600, 0), font->angle));
    }
    if ((handle = CreateFontIndirect(&logFont)) == 0)
        return setPortError(0x2a37);
    if (!SelectObject(dc, handle)) {
        DeleteObject(handle);
        return setPortError(0x2a37);
    }
    if (!GetTextMetrics(dc, &metrics)) {
        SelectObject(dc, hfont);
        DeleteObject(handle);
        return setPortError(0x2a37);
    }
    if (hfont)
        DeleteObject(hfont);
    hfont = handle;
    return setPortError(0);
}

/* @zoombi32 0x00486fae */
void basePort::setUnknown66(short value)
{
    if (value)
        prepare();
    unknown66 = value;
}

/* @zoombi32 0x00486fce */
short basePort::setColor(Color color, unsigned short width)
{
    COLORREF ref;
    HPEN handle;

    if ((ref = colorRef(color)) == CLR_INVALID)
        return graphics.error;
    if ((handle = CreatePen(PS_INSIDEFRAME, width, ref)) == 0)
        return setPortError(0x2a37);
    SelectObject(dc, handle);
    if (hpen)
        DeleteObject(hpen);
    hpen = handle;
    SetTextColor(dc, ref);
    return setPortError(0);
}

/* @zoombi32 0x00487059 */
void basePort::setMode(short mode)
{
    SetROP2(dc, mode == 0   ? R2_COPYPEN
                : mode == 1 ? R2_MASKPEN
                : mode == 2 ? R2_MERGEPEN
                : mode == 3 ? R2_XORPEN
                : mode == 4 ? R2_NOTCOPYPEN
                : mode == 5 ? R2_NOTMASKPEN
                : mode == 6 ? R2_NOTMERGEPEN
                            : R2_NOTXORPEN);
    setPortError(0);
}

/* @zoombi32 0x004870d1 */
void basePort::operator delete(void *block)
{
    localFree(block);
}

/*
 * Draws pixels (bottom-up rows): format's low 4 bits give the depth (1, 4,
 * 8, 16 or 24 bits), its next 4 the packing: 0 none, 0x20 RLE8 (after a
 * big-endian size), 0x10 packed 8-bit (drawPackedPixels's) drawn through a
 * DIB. Mode 8 draws with colour 0 transparent.
 */
/* @zoombi32 0x004870e0 */
short basePort::drawPixels(const Rect &bounds, unsigned short width, unsigned short height,
                           short rowBytes, unsigned short format, void *pixels,
                           unsigned short mode, unsigned short flags)
{
    unsigned short flipX;
    unsigned short depthCode;
    unsigned short packing;
    struct
    {
        BITMAPINFOHEADER header;
        unsigned short colors[256];
    } info;
    unsigned short flipY;

    flipX = (flags & 0x10) != 0;
    flipY = (unsigned short)(flags & 0x20) != (rowBytes >= 0 ? 0 : 0x20);
    SetStretchBltMode(dc, flags & 1 ? BLACKONWHITE : flags & 2 ? WHITEONBLACK : COLORONCOLOR);
    depthCode = format & 0xf;
    packing = format & 0xf0;
    if (packing == 0 || packing == 0x20) {
        flipY = !flipY;
        memset(&info, 0, sizeof(BITMAPINFOHEADER));
        info.header.biSize = sizeof(BITMAPINFOHEADER);
        info.header.biWidth = width;
        info.header.biHeight = height;
        info.header.biPlanes = 1;
        if (!packing) {
            info.header.biCompression = BI_RGB;
        } else {
            info.header.biSizeImage = byteSwapLong(*((unsigned long *&)pixels)++);
            info.header.biCompression = BI_RLE8;
        }
        switch (depthCode) {
        case 0:
            info.header.biBitCount = 1;
            info.header.biClrUsed = 2;
            info.colors[0] = mode == 8 ? 0 : backColor.paletteIndex();
            info.colors[1] = pen.color.paletteIndex();
            break;
        case 1: {
            info.header.biBitCount = 4;
            info.header.biClrUsed = 16;
            for (int i = 0; i < 8; i++) {
                info.colors[i] = i;
                info.colors[15 - i] = 0xff - i;
            }
            break;
        }
        case 2: {
            info.header.biBitCount = 8;
            info.header.biClrUsed = 256;
            for (int i = 0; i < 256; i++)
                info.colors[i] = i;
            break;
        }
        case 3:
            info.header.biBitCount = 16;
            info.header.biCompression = BI_BITFIELDS;
            ((DWORD *)info.colors)[0] = 0x7c00;
            ((DWORD *)info.colors)[1] = 0x3e0;
            ((DWORD *)info.colors)[2] = 0x1f;
            break;
        case 4:
            info.header.biBitCount = 24;
            break;
        default:
            return setPortError(0x2a63);
        }
        if (mode == 8)
            drawMasked(&bounds, pixels, (BITMAPINFO *)&info, flipX, flipY);
        else
            stretchDIBits(flipX ? bounds.right : bounds.left,
                          flipY ? bounds.bottom - 1 : bounds.top,
                          flipX ? bounds.left - bounds.right : bounds.right - bounds.left,
                          flipY ? bounds.top - bounds.bottom : bounds.bottom - bounds.top, 0, 0,
                          width, height, pixels, (BITMAPINFO *)&info, DIB_PAL_COLORS,
                          copyRops[mode]);
        return setPortError(0);
    }
    if (packing == 0x10 && depthCode == 2) {
        DIB dib(width, height, 8);

        if (dib.create(0))
            return graphics.error;
        drawPackedPixels((unsigned char *)dib.bits, dib.lastRow, dib.rowBytes, dib.bounds, 0, 0,
                         dib.bounds, width, height, (const unsigned char *)pixels, 0);
        if (mode == 8) {
            drawMasked(&bounds, dib.bits, dib.indexInfo, flipX, flipY);
        } else {
            DWORD rop = copyRops[mode];
            long usage = dib.usage(paletteKind, rop, flipX, flipY);

            if (usage == DIB_RGB_COLORS)
                dib.setEntries(0, 256, palette->entries);
            dib.draw(this, bounds, dib.bounds, usage, rop, flipX, flipY);
        }
        dib.destroy();
        return setPortError(0);
    }
    return setPortError(0x2a63);
}

/* @zoombi32 0x00487539 */
HBRUSH basePort::brush(Color color)
{
    COLORREF ref = colorRef(color);

    if (ref == CLR_INVALID)
        return 0;
    HBRUSH handle = CreateSolidBrush(ref);
    setPortError(handle ? 0 : 0x2a37);
    return handle;
}

/*
 * A brush from an 8x8 pattern: a big-endian size, then its rows, top first,
 * of 1 bit (drawn in the pen and background colours), 4 or 8 bits (palette
 * indices), 16 bits (big-endian 5-5-5 RGB) or 24 bits a pixel.
 */
/* @zoombi32 0x00487583 */
HBRUSH basePort::patternBrush(const unsigned short *pattern)
{
    struct
    {
        BITMAPINFOHEADER header;
        unsigned short colors[256];
        unsigned char bits[0xc4];
    } info;
    unsigned char *bits;
    unsigned short row;
    UINT usage;
    unsigned short size;
    HGLOBAL memory;
    HBRUSH handle;

    if (!pattern) {
        setPortError(0x2a72);
        return 0;
    }
    memset(&info, 0, sizeof(info));
    info.header.biSize = sizeof(BITMAPINFOHEADER);
    info.header.biWidth = 8;
    info.header.biHeight = 8;
    info.header.biPlanes = 1;
    info.header.biCompression = BI_RGB;
    switch (byteSwapShort(pattern[0])) {
    case 8:
        info.header.biBitCount = 1;
        info.header.biSizeImage = 32;
        info.header.biClrUsed = 2;
        info.colors[0] = pen.color.paletteIndex();
        info.colors[1] = backColor.paletteIndex();
        bits = (unsigned char *)&info.colors[2];
        for (row = 0; row < 8; row++)
            bits[row * 4] = ~((const unsigned char *)(pattern + 1))[7 - row];
        usage = DIB_PAL_COLORS;
        break;
    case 0x20: {
        info.header.biBitCount = 4;
        info.header.biSizeImage = 32;
        info.header.biClrUsed = 16;
        for (unsigned short i = 0; i < 8; i++) {
            info.colors[i] = i;
            info.colors[15 - i] = 0xff - i;
        }
        bits = (unsigned char *)&info.colors[16];
        for (row = 0; row < 8; row++)
            memcpy(bits + row * 4, (const unsigned char *)pattern + (7 - row) * 4 + 2, 4);
        usage = DIB_PAL_COLORS;
        break;
    }
    case 0x40: {
        info.header.biBitCount = 8;
        info.header.biSizeImage = 64;
        info.header.biClrUsed = 256;
        for (unsigned short j = 0; j < 256; j++)
            info.colors[j] = j;
        bits = info.bits;
        for (row = 0; row < 8; row++)
            memcpy(bits + row * 8, (const unsigned char *)pattern + (7 - row) * 8 + 2, 8);
        usage = DIB_PAL_COLORS;
        break;
    }
    case 0x80: {
        info.header.biBitCount = 24;
        info.header.biSizeImage = 192;
        info.header.biClrUsed = 0;
        bits = (unsigned char *)info.colors;
        for (row = 0; row < 8; row++)
            for (unsigned short column = 0; column < 8; column++) {
                unsigned short color = byteSwapShort(
                    ((const unsigned short *)((const unsigned char *)pattern + (7 - row) * 16))[column + 1]);
                unsigned char *pixel = bits + row * 24 + column * 3;

                pixel[2] = (color >> 10) << 3;
                pixel[1] = (color >> 5) << 3;
                pixel[0] = (color >> 0) << 3;
            }
        usage = DIB_RGB_COLORS;
        break;
    }
    case 0xc0:
        info.header.biBitCount = 24;
        info.header.biSizeImage = 192;
        info.header.biClrUsed = 0;
        bits = (unsigned char *)info.colors;
        for (row = 0; row < 8; row++)
            memcpy(bits + row * 24, (const unsigned char *)pattern + (7 - row) * 24 + 2, 24);
        usage = DIB_RGB_COLORS;
        break;
    default:
        setPortError(0x2a72);
        return 0;
    }
    size = bits - (unsigned char *)&info + info.header.biSizeImage;
    if ((memory = GlobalAlloc(GMEM_MOVEABLE, size)) == 0) {
        setPortError(0x2a37);
        return 0;
    }
    memcpy(GlobalLock(memory), &info, size);
    GlobalUnlock(memory);
    handle = CreateDIBPatternBrush(memory, usage);
    GlobalFree(memory);
    setPortError(handle ? 0 : 0x2a37);
    return handle;
}

/* StretchDIBits, flipping (by a negative width or height) through a DIB
   where the device might not. */
/* @zoombi32 0x0048792c */
int basePort::stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                            int fromWidth, int fromHeight, const void *bits, BITMAPINFO *info,
                            UINT usage, DWORD rop)
{
    int result;

    prepare();
    if (toWidth < 0 || toHeight < 0) {
        DIB dib(fromWidth, fromHeight, info->bmiHeader.biBitCount);
        HDC memory;
        HPALETTE copy;

        if (dib.create(0))
            return 0;
        memory = dib.getDC();
        if (!memory) {
            dib.destroy();
            return 0;
        }
        copy = createPalette(palette->entries);
        if (copy) {
            SelectPalette(memory, copy, TRUE);
        } else {
            dib.releaseDC(memory);
            dib.destroy();
            return 0;
        }
        if (dib.depth == 8)
            dib.setEntries(0, 256, palette->entries);
        StretchDIBits(memory, 0, 0, fromWidth, fromHeight, fromX, fromY, fromWidth, fromHeight,
                      bits, info, usage, SRCCOPY);
        StretchBlt(memory, toWidth < 0 ? fromWidth - 1 : 0, toHeight < 0 ? fromHeight - 1 : 0,
                   toWidth < 0 ? -fromWidth : fromWidth, toHeight < 0 ? -fromHeight : fromHeight,
                   memory, 0, 0, fromWidth, fromHeight, SRCCOPY);
        dib.releaseDC(memory);
        DeleteObject(copy);
        result = StretchDIBits(dc, toWidth < 0 ? toX + toWidth : toX,
                               toHeight < 0 ? toY + toHeight : toY,
                               toWidth >= 0 ? toWidth : -toWidth,
                               toHeight >= 0 ? toHeight : -toHeight, 0, 0, fromWidth, fromHeight,
                               dib.bits, usage == DIB_RGB_COLORS ? dib.info : dib.indexInfo,
                               usage, rop);
        dib.destroy();
        return result;
    }
    return StretchDIBits(dc, toX, toY, toWidth, toHeight, fromX, fromY, fromWidth, fromHeight,
                         bits, info, usage, rop);
}

/* @zoombi32 0x00487b35 */
void basePort::getBits(PixMap *)
{
    setPortError(0x2a32);
}

/* @zoombi32 0x00487b46 */
unsigned short basePort::nearestIndex(RGBColor color)
{
    return nearestPaletteIndex(palette, color);
}

/* The colour of a pixel: read back through a bitmap on 8-bit devices, where
   GetPixel gives RGB. Not exact: the original keeps `memory` in edi and
   `bitmap` on the stack; BCC32 4.5 swaps them. */
/* @zoombi32 0x00487b61 */
Color basePort::getPixel(short x, short y)
{
    prepare();
    setPortError(0);
    if (!ptInRect(&frame, Pt(x, y)))
        return Color(RGBColor(0, 0, 0, 0));
    if (depth == 8) {
        POINT device;
        HBITMAP bitmap;
        unsigned char index[4];
        HDC memory;

        device.x = x;
        device.y = y;
        LPtoDP(dc, &device, 1);
        device.x += offset.x;
        device.y += offset.y;
        bitmap = CreateCompatibleBitmap(dc, 1, 1);
        if (!bitmap) {
        failed:
            setPortError(0x2a37);
            return Color(0xffff);
        }
        if ((memory = CreateCompatibleDC(dc)) == 0) {
            DeleteObject(bitmap);
            goto failed;
        }
        SelectObject(memory, bitmap);
        SelectPalette(memory, hpal, TRUE);
        if (mapMode != MM_TEXT)
            SetMapMode(dc, MM_TEXT);
        BitBlt(memory, 0, 0, 1, 1, dc, device.x, device.y, SRCCOPY);
        if (mapMode != MM_TEXT)
            SetMapMode(dc, mapMode);
        GetBitmapBits(bitmap, 1, index);
        DeleteDC(memory);
        DeleteObject(bitmap);
        return Color(index[0]);
    }
    COLORREF pixel = GetPixel(dc, x, y);
    if (pixel == CLR_INVALID) {
        HRGN all = CreateRectRgn(bounds.left, bounds.top, bounds.right, bounds.bottom);

        SelectClipRgn(dc, all);
        pixel = GetPixel(dc, x, y);
        SelectClipRgn(dc, clipRgn);
        IntersectClipRect(dc, frame.left, frame.top, frame.right, frame.bottom);
        DeleteObject(all);
        if (pixel == CLR_INVALID) {
            setPortError(0x2a37);
            return Color(0xffff);
        }
    }
    RGBColor rgb(pixel, pixel >> 8, pixel >> 16, 0);
    return rgb;
}

/* A palette entry's colour; black, marked as none, outside the colours the
   game may use. */
/* @zoombi32 0x00487da8 */
RGBColor basePort::paletteColor(unsigned short index)
{
    if (!(graphics.paletteReserved / 2 + palette->first > index
          || (index < 0x100 && 0x100 - graphics.paletteReserved / 2 <= index))) {
        setPortError(0x2a64);
        return RGBColor(0, 0, 0, 0xff);
    }
    setPortError(0);
    const PALETTEENTRY &entry = palette->entries[index];
    return RGBColor(entry.peRed, entry.peGreen, entry.peBlue, 0);
}

/* The default palette and font, a white background, a black pen, and a
   clip region of the frame. */
/* @zoombi32 0x00487e50 */
short basePort::init()
{
    memset(&palette, 0, 4 * sizeof(void *));
    setPalette(graphics.defaultPalette);
    (font = graphics.defaultFont)->users++;
    backColor = Color(RGBColor(0xff, 0xff, 0xff, 4));
    pen = Pen(Pt(0, 0), RGBColor(0, 0, 0, 4), 0, 1);
    if ((clip = newRgn()) != 0)
        setRectRgn(clip, &frame);
    else
        return setPortError(regionError());
    return setPortError(0);
}

/* @zoombi32 0x00487f56 */
void *basePort::operator new(size_t size)
{
    void *block;

    if ((block = localAlloc(size)) == 0) {
        setPortError(localMemError());
        return 0;
    }
    setPortError(0);
    memset(block, 0, size);
    return block;
}

/* @zoombi32 0x00487f94 */
short basePort::fillRect(const Rect &rect, Color color, short mode)
{
    HBRUSH handle;

    if ((handle = brush(color)) != 0) {
        patBlt(&rect, handle, mode);
        DeleteObject(handle);
    }
    return graphics.error;
}

/* @zoombi32 0x00487fda */
short basePort::fillOval(const Rect *rect, HBRUSH brush, short mode)
{
    HGDIOBJ oldBrush;
    HGDIOBJ oldPen;
    HRGN rgn;

    if (!mode) {
        prepare();
        oldBrush = SelectObject(dc, brush);
        oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
        Ellipse(dc, rect->left, rect->top, rect->right, rect->bottom);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        return setPortError(0);
    }
    WinRect device(*rect);
    LPtoDP(dc, (POINT *)&device, 2);
    if ((rgn = CreateEllipticRgn(device.left, device.top, device.right, device.bottom)) == 0)
        return setPortError(0x2a37);
    clipTo(rgn);
    DeleteObject(rgn);
    clipApplied = 1;
    if (!graphics.error)
        patBlt(rect, brush, mode);
    clipApplied = 0;
    return graphics.error;
}

/* Not exact: the original keeps `this` in esi and `data` in edi (the pens
   and `points` on the stack); BCC32 4.5 allocates them differently. */
/* @zoombi32 0x004880cd */
short basePort::fillPoly(short poly, HBRUSH brush, short mode)
{
    if (!poly)
        return setPortError(0x2ac8);
    PolygonData *data = (PolygonData *)handleData(poly);
    if (!mode) {
        prepare();
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
        SetPolyFillMode(dc, WINDING);
        setPortError(polygon(dc, data->points, data->count) ? 0 : 0x2a37);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        return graphics.error;
    }
    POINT *points = (POINT *)malloc(data->count * sizeof(POINT));
    if (!points)
        return setPortError(0x2a37);
    for (int i = 0; i < data->count; i++) {
        Pt corner(data->points[i]);
        WinPoint point(corner);

        points[i].x = point.x;
        points[i].y = point.y;
        LPtoDP(dc, &points[i], 1);
    }
    HRGN rgn = CreatePolygonRgn(points, data->count, WINDING);
    free(points);
    if (!rgn)
        return setPortError(0x2a37);
    clipTo(rgn);
    DeleteObject(rgn);
    clipApplied = 1;
    if (!graphics.error)
        patBlt(&frame, brush, mode);
    clipApplied = 0;
    return graphics.error;
}

/* @zoombi32 0x00488253 */
short basePort::patBlt(const Rect *rect, HBRUSH brush, unsigned short mode)
{
    HGDIOBJ old;
    BOOL done;

    prepare();
    old = SelectObject(dc, brush);
    done = PatBlt(dc, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                  patternRops[mode]);
    SelectObject(dc, old);
    return setPortError(done ? 0 : 0x2a37);
}

/* @zoombi32 0x004882c6 */
short basePort::fillRgn(short region, HBRUSH brush, short mode)
{
    HRGN rgn;

    if (!region)
        return setPortError(0x2937);
    if ((rgn = CreateRectRgn(0, 0, 0, 0)) == 0)
        return setPortError(0x2a37);
    if (!mode) {
        if (regionToHrgn(rgn, region, 0, 0)) {
            DeleteObject(rgn);
            return setPortError(regionError());
        }
        prepare();
        setPortError(FillRgn(dc, rgn, brush) ? 0 : 0x2a37);
    } else {
        if (toHrgn(rgn, region)) {
            DeleteObject(rgn);
            return setPortError(regionError());
        }
        if (!clipTo(rgn)) {
            clipApplied = 1;
            patBlt(&frame, brush, mode);
        }
        clipApplied = 0;
    }
    DeleteObject(rgn);
    return graphics.error;
}

/* @zoombi32 0x004883b5 */
void basePort::release()
{
    disposeRgn(clip);
    if (nextOnPalette)
        nextOnPalette->prevOnPalette = prevOnPalette;
    if (prevOnPalette)
        prevOnPalette->nextOnPalette = nextOnPalette;
    else
        palette->ports = nextOnPalette;
    font->users--;
}

/* @zoombi32 0x004883fb */
short basePort::scroll(const Rect *rect, short dx, short dy, short region)
{
    Rect moved = *rect;

    offsetRect(&moved, dx, dy);
    if (region && (setRectRgn(region, (ShortRect *)rect) || diffRgnRect(region, &moved)))
        return setPortError(regionError());
    prepare();
    WinRect device(*rect);
    return setPortError(ScrollDC(dc, dx, dy, &device, &device, 0, 0) ? 0 : 0x2a37);
}

/* @zoombi32 0x004884aa */
Palette *basePort::setPalette(Palette *newPalette)
{
    Palette *old = palette;

    if (newPalette != old) {
        if (old) {
            if (nextOnPalette)
                nextOnPalette->prevOnPalette = prevOnPalette;
            if (prevOnPalette)
                prevOnPalette->nextOnPalette = nextOnPalette;
            else
                old->ports = nextOnPalette;
        }
        prevOnPalette = 0;
        if ((nextOnPalette = newPalette->ports) != 0)
            nextOnPalette->prevOnPalette = this;
        newPalette->ports = this;
        palette = newPalette;
        if (locks) {
            if (paletteKind == 1)
                SelectPalette(dc, hpal = newPalette->hpal, TRUE);
            if (newPalette->realized)
                realizePalette();
        }
    }
    return old;
}

/* @zoombi32 0x00488536 */
void basePort::unlock()
{
    if (!--locks) {
        cleanupDC();
        DeleteDC(dc);
    }
}

/* Draws with colour 0 transparent: ANDs in a mask (white where the pixels
   are 0), then ORs in the pixels. */
/* @zoombi32 0x0048855d */
void basePort::drawMasked(const Rect *rect, const void *bits, BITMAPINFO *info, short flipX,
                          short flipY)
{
    struct
    {
        BITMAPINFOHEADER header;
        unsigned short indices[512];
    } mask;
    unsigned short i;
    unsigned short count;

    switch (info->bmiHeader.biBitCount) {
    case 1:
    case 4:
    case 8:
        memcpy(&mask, info, sizeof(BITMAPINFOHEADER));
        count = 1 << mask.header.biBitCount;
        for (i = 0; i < count; i++)
            mask.indices[i] = !((unsigned short *)info->bmiColors)[i] ? 0xff : 0;
        stretchDIBits(flipX ? rect->right : rect->left, flipY ? rect->bottom - 1 : rect->top,
                      flipX ? rect->left - rect->right : rect->right - rect->left,
                      flipY ? rect->top - rect->bottom : rect->bottom - rect->top, 0, 0,
                      info->bmiHeader.biWidth, info->bmiHeader.biHeight, bits,
                      (BITMAPINFO *)&mask, DIB_PAL_COLORS, SRCAND);
        stretchDIBits(flipX ? rect->right : rect->left, flipY ? rect->bottom - 1 : rect->top,
                      flipX ? rect->left - rect->right : rect->right - rect->left,
                      flipY ? rect->top - rect->bottom : rect->bottom - rect->top, 0, 0,
                      info->bmiHeader.biWidth, info->bmiHeader.biHeight, bits, info,
                      DIB_PAL_COLORS, SRCPAINT);
    }
    setPortError(0);
}

/* @zoombi32 0x004886ee */
COLORREF basePort::colorRef(Color color)
{
    switch (color.kind()) {
    case -1:
        setPortError(0x2a64);
        return CLR_INVALID;
    case 2:
        if (depth != 8) {
            RGBColor rgb = color.rgb();

            if (rgb.bytes.kind == 0xff)
                return CLR_INVALID;
            return PALETTERGB(rgb.bytes.red, rgb.bytes.green, rgb.bytes.blue);
        }
        break;
    }
    unsigned short index = color.paletteIndex();
    return index == 0xffff ? CLR_INVALID : PALETTEINDEX(index);
}

/* @zoombi32 0x00488786 */
__cdecl Pt::Pt()
{
}

/* @zoombi32 0x0048878e */
__cdecl Pen::Pen()
{
}

/* @zoombi32 0x004887ab */
void __cdecl Pt::operator=(const Pt &point)
{
    x = point.x;
    y = point.y;
}

/* @zoombi32 0x004887c4 */
__cdecl Pen::Pen(const Pt &position, const Color &color, unsigned short mode, short width)
    : position(position), color(color)
{
    this->mode = mode;
    this->width = width;
}
