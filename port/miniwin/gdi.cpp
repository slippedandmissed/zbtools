/*
 * GDI's objects and device contexts: creating, selecting and deleting
 * objects, bitmaps and DIB sections, and a DC's state and coordinate
 * mapping. Drawing is in blit.cpp, draw.cpp and text.cpp; palettes in
 * palette.cpp; regions in region.cpp.
 *
 * Handles are the objects' addresses. Every surface drawn into is 8-bit (the
 * screen, compatible bitmaps and 8-bit DIB sections) or monochrome.
 */

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <set>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

static std::set<GdiObject *> objects;
static std::set<DC *> dcs;
Bitmap *screenBitmap;


/* Objects carry no magic of their own: they're known by being in `objects`. */
GdiObject *objectOf(HGDIOBJ handle)
{
    GdiObject *object = (GdiObject *)handle;
    return handle && objects.count(object) ? object : 0;
}

static void track(GdiObject *object)
{
    objects.insert(object);
}

DC *dcOf(HDC handle)
{
    DC *dc = (DC *)handle;
    return handle && dcs.count(dc) ? dc : 0;
}

Region *regionOf(HRGN handle)
{
    GdiObject *object = objectOf(handle);
    if (object && object->kind == KIND_REGION)
        return (Region *)object;
    /* Regions miniwin keeps itself (a window's update region). */
    Region *region = (Region *)handle;
    return handle && !object && region->kind == KIND_REGION ? region : 0;
}

Bitmap *bitmapOf(HBITMAP handle)
{
    GdiObject *object = objectOf(handle);
    return object && object->kind == KIND_BITMAP ? (Bitmap *)object : 0;
}

Palette *paletteOf(HPALETTE handle)
{
    GdiObject *object = objectOf(handle);
    return object && object->kind == KIND_PALETTE ? (Palette *)object : 0;
}

Bitmap::~Bitmap()
{
    free(allocation);
}

static long rowBytesOf(int width, int bpp)
{
    return ((long)width * bpp + 31) / 32 * 4;
}

/* A bitmap of its own memory, top-down or (a DIB) bottom-up. */
static Bitmap *newBitmap(int width, int height, int bpp, bool bottomUp, bool device)
{
    Bitmap *bitmap = new Bitmap;
    long rowBytes = rowBytesOf(width, bpp);

    bitmap->width = width;
    bitmap->height = height;
    bitmap->bpp = bpp;
    bitmap->allocation = (uint8_t *)calloc(1, (size_t)(rowBytes * (height ? height : 1)));
    bitmap->stride = bottomUp ? -rowBytes : rowBytes;
    bitmap->top = bottomUp ? bitmap->allocation + rowBytes * (height - 1) : bitmap->allocation;
    bitmap->device = device;
    bitmap->table.count = 0;
    bitmap->selectedIn = 0;
    if (bpp == 1) {
        bitmap->table.count = 2;
        bitmap->table.colors[0] = {0, 0, 0, 0};
        bitmap->table.colors[1] = {255, 255, 255, 0};
    }
    return bitmap;
}

/* Stock objects */

static Brush *stockBrushes[6];
static Pen *stockPens[3];
static Font *stockFonts[7];

static Brush *solidBrush(COLORREF color, bool null = false)
{
    Brush *brush = new Brush;
    brush->color = color;
    brush->null = null;
    return brush;
}

static void makeStock()
{
    static bool made;
    static const COLORREF brushColors[] = {RGB(255, 255, 255), RGB(192, 192, 192),
                                           RGB(128, 128, 128), RGB(64, 64, 64), RGB(0, 0, 0)};

    if (made)
        return;
    made = true;
    for (int i = 0; i < 6; i++) {
        stockBrushes[i] = solidBrush(i < 5 ? brushColors[i] : 0, i == 5);
        stockBrushes[i]->stock = true;
        track(stockBrushes[i]);
    }
    for (int i = 0; i < 3; i++) {
        stockPens[i] = new Pen;
        stockPens[i]->color = i == 0 ? RGB(255, 255, 255) : 0;
        stockPens[i]->style = i == 2 ? PS_NULL : PS_SOLID;
        stockPens[i]->stock = true;
        track(stockPens[i]);
    }
    for (int i = 0; i < 7; i++) {
        stockFonts[i] = new Font;
        memset(&stockFonts[i]->logical, 0, sizeof(LOGFONT));
        stockFonts[i]->logical.lfHeight = 16;
        stockFonts[i]->logical.lfWeight = FW_BOLD;
        strcpy(stockFonts[i]->logical.lfFaceName, "System");
        stockFonts[i]->stock = true;
        track(stockFonts[i]);
    }
    Palette *palette = defaultPalette();
    palette->stock = true;
    track(palette);
}

HGDIOBJ GetStockObject(int index)
{
    makeStock();
    if (index >= WHITE_BRUSH && index <= NULL_BRUSH)
        return stockBrushes[index];
    if (index >= WHITE_PEN && index <= NULL_PEN)
        return stockPens[index - WHITE_PEN];
    if (index >= OEM_FIXED_FONT && index <= SYSTEM_FIXED_FONT && index != DEFAULT_PALETTE)
        return stockFonts[index - OEM_FIXED_FONT];
    if (index == DEFAULT_PALETTE)
        return defaultPalette();
    return 0;
}

/* Device contexts */

DC::DC()
    : GdiObject(KIND_DC), window(0), memory(false), information(false), surface(0),
      defaultBitmap(0), brush(0), pen(0), font(0), palette(0), clip(0), mapMode(MM_TEXT),
      textColor(0), bkColor(RGB(255, 255, 255)), bkMode(OPAQUE), rop2(R2_COPYPEN),
      polyFillMode(ALTERNATE), stretchMode(BLACKONWHITE), textAlign(0)
{
    makeStock();
    windowOrg = viewportOrg = position = origin = {0, 0};
    windowExt = viewportExt = {1, 1};
    brush = stockBrushes[WHITE_BRUSH];
    pen = stockPens[BLACK_PEN - WHITE_PEN];
    font = stockFonts[SYSTEM_FONT - OEM_FIXED_FONT];
    palette = defaultPalette();
    visible = {0, 0, 0, 0};
}

DC::~DC()
{
    if (surface)
        surface->selectedIn--;
    delete defaultBitmap;
    delete clip;
}

static HDC windowDC(HWND hwnd)
{
    DC *dc = new DC;
    Window *w = windowOf(hwnd);

    dc->window = hwnd ? hwnd : GetDesktopWindow();
    dc->surface = screenBitmap;
    screenBitmap->selectedIn++;
    if (w) {
        dc->origin = clientOrigin(hwnd);
        dc->visible = clientRect(hwnd);
    } else
        dc->visible = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    dcs.insert(dc);
    return (HDC)dc;
}

HDC GetDC(HWND hwnd)
{
    if (hwnd && !windowOf(hwnd))
        return 0;
    return windowDC(hwnd);
}

HDC dcForPaint(HWND hwnd, const Region &update)
{
    HDC handle = windowDC(hwnd);
    DC *dc = dcOf(handle);

    dc->clip = new Region;
    dc->clip->rects = update.rects;
    return handle;
}

int ReleaseDC(HWND, HDC handle)
{
    DC *dc = dcOf(handle);

    if (!dc || dc->memory)
        return 0;
    dcs.erase(dc);
    delete dc;
    return 1;
}

HDC CreateDC(LPCSTR driver, LPCSTR, LPCSTR, const DEVMODE *)
{
    if (driver && stricmp(driver, "DISPLAY"))
        return 0;
    return windowDC(0);
}

HDC CreateIC(LPCSTR driver, LPCSTR device, LPCSTR output, const DEVMODE *mode)
{
    HDC handle = CreateDC(driver, device, output, mode);
    DC *dc = dcOf(handle);

    if (dc)
        dc->information = true;
    return handle;
}

static void selectSurface(DC *dc, Bitmap *bitmap)
{
    if (dc->surface)
        dc->surface->selectedIn--;
    dc->surface = bitmap;
    bitmap->selectedIn++;
    dc->visible = {0, 0, bitmap->width, bitmap->height};
}

HDC CreateCompatibleDC(HDC)
{
    DC *dc = new DC;

    dc->memory = true;
    dc->defaultBitmap = newBitmap(1, 1, 1, false, false);
    selectSurface(dc, dc->defaultBitmap);
    dcs.insert(dc);
    return (HDC)dc;
}

BOOL DeleteDC(HDC handle)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return FALSE;
    dcs.erase(dc);
    delete dc;
    return TRUE;
}

int GetDeviceCaps(HDC, int index)
{
    switch (index) {
    case HORZRES:
        return SCREEN_WIDTH;
    case VERTRES:
        return SCREEN_HEIGHT;
    case BITSPIXEL:
        return 8;
    case PLANES:
        return 1;
    case NUMCOLORS:
        return 20;
    case RASTERCAPS:
        return RC_BITBLT | RC_PALETTE | RC_DIBTODEV | RC_STRETCHBLT | RC_STRETCHDIB;
    case SIZEPALETTE:
        return 256;
    case NUMRESERVED:
        return 20;
    case COLORRES:
        return 18;
    }
    return 0;
}

int SelectClipRgn(HDC handle, HRGN rgn);

HGDIOBJ SelectObject(HDC handle, HGDIOBJ handleObject)
{
    DC *dc = dcOf(handle);
    GdiObject *object = objectOf(handleObject);
    HGDIOBJ previous = 0;

    if (!dc || !object)
        return 0;
    switch (object->kind) {
    case KIND_BRUSH:
        previous = dc->brush;
        dc->brush = (Brush *)object;
        break;
    case KIND_PEN:
        previous = dc->pen;
        dc->pen = (Pen *)object;
        break;
    case KIND_FONT:
        previous = dc->font;
        dc->font = (Font *)object;
        break;
    case KIND_BITMAP: {
        Bitmap *bitmap = (Bitmap *)object;
        if (!dc->memory || (bitmap->selectedIn && bitmap != dc->surface))
            return 0;
        previous = dc->surface;
        selectSurface(dc, bitmap);
        break;
    }
    case KIND_REGION:
        return (HGDIOBJ)(intptr_t)SelectClipRgn(handle, (HRGN)object);
    default:
        return 0;
    }
    return previous;
}

/* An object going: DCs holding it fall back to the stock ones. */
static void unselect(GdiObject *object)
{
    for (DC *dc : dcs) {
        if (dc->brush == object)
            dc->brush = stockBrushes[WHITE_BRUSH];
        if (dc->pen == object)
            dc->pen = stockPens[BLACK_PEN - WHITE_PEN];
        if (dc->font == object)
            dc->font = stockFonts[SYSTEM_FONT - OEM_FIXED_FONT];
        if (dc->palette == object)
            dc->palette = defaultPalette();
    }
}

BOOL DeleteObject(HGDIOBJ handle)
{
    GdiObject *object = objectOf(handle);

    if (!object)
        return FALSE;
    if (object->stock)
        return TRUE;
    if (object->kind == KIND_BITMAP && ((Bitmap *)object)->selectedIn)
        return FALSE;
    unselect(object);
    objects.erase(object);
    delete object;
    return TRUE;
}

BOOL UnrealizeObject(HGDIOBJ handle)
{
    Palette *palette = paletteOf((HPALETTE)handle);
    if (palette)
        palette->realized = false;
    return objectOf(handle) != 0;
}

BOOL GdiFlush()
{
    return TRUE;
}

/* Bitmaps */

HBITMAP CreateCompatibleBitmap(HDC handle, int width, int height)
{
    DC *dc = dcOf(handle);

    if (!dc || width <= 0 || height <= 0)
        return 0;
    /* A memory DC still holding its own bitmap is monochrome, and so is what
       it's compatible with, as in Windows. */
    bool mono = dc->memory && dc->surface->bpp == 1;
    Bitmap *bitmap = newBitmap(width, height, mono ? 1 : 8, false, !mono);
    track(bitmap);
    return (HBITMAP)bitmap;
}

HBITMAP CreateDIBSection(HDC handle, const BITMAPINFO *info, UINT usage, void **bits, HANDLE,
                         DWORD)
{
    const BITMAPINFOHEADER &header = info->bmiHeader;
    DC *dc = dcOf(handle);
    int bpp = header.biBitCount;

    if (bits)
        *bits = 0;
    if (header.biWidth <= 0 || !header.biHeight
        || (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (bpp != 8 && bpp != 1)
        trace("CreateDIBSection: a %d-bit DIB section (only 8 and 1 bits are drawn into)", bpp);
    int height = header.biHeight < 0 ? -header.biHeight : header.biHeight;
    Bitmap *bitmap = newBitmap(header.biWidth, height, bpp, header.biHeight > 0, false);
    if (bpp <= 8) {
        int count = header.biClrUsed ? (int)header.biClrUsed : 1 << bpp;
        if (count > 256)
            count = 256;
        bitmap->table.count = count;
        for (int i = 0; i < count; i++) {
            if (usage == DIB_PAL_COLORS) {
                /* Indices into the DC's palette: the colours they are now. */
                WORD index = ((const WORD *)info->bmiColors)[i];
                Palette *palette = dc ? dc->palette : defaultPalette();
                PALETTEENTRY entry = palette->entries[index < palette->count ? index : 0];
                bitmap->table.colors[i] = {entry.peBlue, entry.peGreen, entry.peRed, 0};
            } else
                bitmap->table.colors[i] = info->bmiColors[i];
        }
    }
    track(bitmap);
    if (bits)
        *bits = bitmap->allocation;
    return (HBITMAP)bitmap;
}

UINT SetDIBColorTable(HDC handle, UINT start, UINT count, const RGBQUAD *colors)
{
    DC *dc = dcOf(handle);

    if (!dc || !dc->surface || dc->surface->device)
        return 0;
    ColorTable &table = dc->surface->table;
    UINT set = 0;
    for (UINT i = start; i < start + count && (int)i < table.count; i++, set++)
        table.colors[i] = colors[i - start];
    return set;
}

LONG GetBitmapBits(HBITMAP handle, LONG size, LPVOID bits)
{
    Bitmap *bitmap = bitmapOf(handle);

    if (!bitmap)
        return 0;
    long rowBytes = (bitmap->width * bitmap->bpp + 15) / 16 * 2; /* word-aligned rows */
    long copied = 0;
    for (int y = 0; y < bitmap->height && copied + rowBytes <= size; y++) {
        memcpy((uint8_t *)bits + copied, bitmap->row(y), (size_t)rowBytes);
        copied += rowBytes;
    }
    return copied;
}

int GetDIBits(HDC handle, HBITMAP bitmapHandle, UINT start, UINT lines, LPVOID bits,
              LPBITMAPINFO info, UINT usage)
{
    DC *dc = dcOf(handle);
    Bitmap *bitmap = bitmapOf(bitmapHandle);

    if (!dc || !bitmap)
        return 0;
    BITMAPINFOHEADER &header = info->bmiHeader;
    if (!header.biBitCount || !bits) {
        /* Tell the bitmap's format. */
        header.biWidth = bitmap->width;
        header.biHeight = bitmap->height;
        header.biPlanes = 1;
        header.biBitCount = (WORD)bitmap->bpp;
        header.biCompression = BI_RGB;
        header.biSizeImage = (DWORD)(rowBytesOf(bitmap->width, bitmap->bpp) * bitmap->height);
        if (!bits)
            return bitmap->height;
    }
    if (header.biBitCount != bitmap->bpp) {
        unsupported("GetDIBits in another depth");
        return 0;
    }
    int count = bitmap->bpp <= 8 ? 1 << bitmap->bpp : 0;
    for (int i = 0; i < count; i++) {
        RGBQUAD color = bitmap->device ? RGBQUAD{systemPalette[i].peBlue, systemPalette[i].peGreen,
                                                 systemPalette[i].peRed, 0}
                                       : bitmap->table.colors[i];
        if (usage == DIB_PAL_COLORS)
            ((WORD *)info->bmiColors)[i] = (WORD)i;
        else
            info->bmiColors[i] = color;
    }
    long rowBytes = rowBytesOf(bitmap->width, bitmap->bpp);
    bool bottomUp = header.biHeight > 0;
    UINT copied = 0;
    for (UINT line = start; line < start + lines && (int)line < bitmap->height; line++, copied++) {
        int y = bottomUp ? bitmap->height - 1 - (int)line : (int)line;
        memcpy((uint8_t *)bits + (line - start) * rowBytes, bitmap->row(y), (size_t)rowBytes);
    }
    return (int)copied;
}

/* Brushes, pens, fonts */

HBRUSH CreateSolidBrush(COLORREF color)
{
    Brush *brush = solidBrush(color);
    track(brush);
    return (HBRUSH)brush;
}

/* A packed DIB (a BITMAPINFO, then its bits), 8x8 of it used. */
HBRUSH CreateDIBPatternBrush(HGLOBAL packed, UINT usage)
{
    const BITMAPINFO *info = (const BITMAPINFO *)GlobalLock(packed);
    const BITMAPINFOHEADER &header = info->bmiHeader;
    int bpp = header.biBitCount;

    if (bpp != 1 && bpp != 4 && bpp != 8) {
        GlobalUnlock(packed);
        unsupported("CreateDIBPatternBrush of more than 8 bits");
        return 0;
    }
    int count = header.biClrUsed ? (int)header.biClrUsed : 1 << bpp;
    size_t tableSize = usage == DIB_PAL_COLORS ? count * sizeof(WORD) : count * sizeof(RGBQUAD);
    const uint8_t *bits = (const uint8_t *)info + header.biSize + tableSize;
    int height = header.biHeight < 0 ? -header.biHeight : header.biHeight;
    long rowBytes = rowBytesOf(header.biWidth, bpp);
    Brush *brush = new Brush;
    brush->pattern = newBitmap(8, 8, 8, false, false);
    brush->patternUsage = usage;
    brush->pattern->table.count = count;
    for (int i = 0; i < count; i++) {
        if (usage == DIB_PAL_COLORS)
            brush->patternIndices[i] = ((const WORD *)info->bmiColors)[i];
        else
            brush->pattern->table.colors[i] = info->bmiColors[i];
    }
    for (int y = 0; y < 8; y++) {
        int sourceY = y % height;
        const uint8_t *row = bits + (header.biHeight > 0 ? height - 1 - sourceY : sourceY) * rowBytes;
        for (int x = 0; x < 8; x++) {
            int sourceX = x % header.biWidth;
            uint8_t value;
            if (bpp == 8)
                value = row[sourceX];
            else if (bpp == 4)
                value = (uint8_t)(row[sourceX / 2] >> (sourceX & 1 ? 0 : 4) & 15);
            else
                value = (uint8_t)(row[sourceX / 8] >> (7 - (sourceX & 7)) & 1);
            brush->pattern->row(y)[x] = value;
        }
    }
    GlobalUnlock(packed);
    track(brush);
    return (HBRUSH)brush;
}

HPEN CreatePen(int style, int width, COLORREF color)
{
    Pen *pen = new Pen;
    pen->style = style;
    pen->width = width < 1 ? 1 : width;
    pen->color = color;
    track(pen);
    return (HPEN)pen;
}

HFONT CreateFontIndirect(const LOGFONT *logical)
{
    Font *font = new Font;
    font->logical = *logical;
    track(font);
    return (HFONT)font;
}

void trackObject(GdiObject *object)
{
    track(object);
}

/* The DC's state */

int SetMapMode(HDC handle, int mode)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return 0;
    int previous = dc->mapMode;
    dc->mapMode = mode;
    if (mode == MM_TEXT)
        dc->windowExt = dc->viewportExt = {1, 1};
    return previous;
}

BOOL SetWindowOrgEx(HDC handle, int x, int y, LPPOINT previous)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    if (previous)
        *previous = dc->windowOrg;
    dc->windowOrg = {x, y};
    return TRUE;
}

BOOL SetWindowExtEx(HDC handle, int x, int y, LPSIZE previous)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    if (previous)
        *previous = dc->windowExt;
    if (dc->mapMode == MM_ANISOTROPIC || dc->mapMode == MM_ISOTROPIC)
        dc->windowExt = {x ? x : 1, y ? y : 1};
    return TRUE;
}

BOOL SetViewportOrgEx(HDC handle, int x, int y, LPPOINT previous)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    if (previous)
        *previous = dc->viewportOrg;
    dc->viewportOrg = {x, y};
    return TRUE;
}

BOOL SetViewportExtEx(HDC handle, int x, int y, LPSIZE previous)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    if (previous)
        *previous = dc->viewportExt;
    if (dc->mapMode == MM_ANISOTROPIC || dc->mapMode == MM_ISOTROPIC)
        dc->viewportExt = {x ? x : 1, y ? y : 1};
    return TRUE;
}

/* a * b / c, rounded to nearest, as Windows' MulDiv. */
static LONG mulDiv(LONG a, LONG b, LONG c)
{
    long long product = (long long)a * b;
    if ((product < 0) != (c < 0))
        return (LONG)((product - c / 2) / c);
    return (LONG)((product + c / 2) / c);
}

void toDevice(DC *dc, LONG &x, LONG &y)
{
    x = mulDiv(x - dc->windowOrg.x, dc->viewportExt.cx, dc->windowExt.cx) + dc->viewportOrg.x;
    y = mulDiv(y - dc->windowOrg.y, dc->viewportExt.cy, dc->windowExt.cy) + dc->viewportOrg.y;
}

void toLogical(DC *dc, LONG &x, LONG &y)
{
    x = mulDiv(x - dc->viewportOrg.x, dc->windowExt.cx, dc->viewportExt.cx) + dc->windowOrg.x;
    y = mulDiv(y - dc->viewportOrg.y, dc->windowExt.cy, dc->viewportExt.cy) + dc->windowOrg.y;
}

RECT toDeviceRect(DC *dc, int left, int top, int right, int bottom)
{
    LONG x1 = left, y1 = top, x2 = right, y2 = bottom;
    toDevice(dc, x1, y1);
    toDevice(dc, x2, y2);
    RECT rect = {std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2)};
    return rect;
}

BOOL LPtoDP(HDC handle, LPPOINT points, int count)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    for (int i = 0; i < count; i++)
        toDevice(dc, points[i].x, points[i].y);
    return TRUE;
}

BOOL DPtoLP(HDC handle, LPPOINT points, int count)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    for (int i = 0; i < count; i++)
        toLogical(dc, points[i].x, points[i].y);
    return TRUE;
}

bool intersect(RECT &a, const RECT &b)
{
    a.left = std::max(a.left, b.left);
    a.top = std::max(a.top, b.top);
    a.right = std::min(a.right, b.right);
    a.bottom = std::min(a.bottom, b.bottom);
    if (a.left >= a.right || a.top >= a.bottom) {
        a = {0, 0, 0, 0};
        return false;
    }
    return true;
}

std::vector<RECT> clipRects(DC *dc)
{
    std::vector<RECT> rects;

    if (dc->information || !dc->surface)
        return rects;
    RECT visible = dc->visible;
    RECT surface = {-dc->origin.x, -dc->origin.y, dc->surface->width - dc->origin.x,
                    dc->surface->height - dc->origin.y};
    if (!intersect(visible, surface))
        return rects;
    if (!dc->clip) {
        rects.push_back(visible);
        return rects;
    }
    for (RECT r : dc->clip->rects)
        if (intersect(r, visible))
            rects.push_back(r);
    return rects;
}

#define DC_SETTER(name, field, type) \
    type name(HDC handle, type value) \
    { \
        DC *dc = dcOf(handle); \
        if (!dc) \
            return 0; \
        type previous = (type)dc->field; \
        dc->field = value; \
        return previous; \
    }

DC_SETTER(SetBkMode, bkMode, int)
DC_SETTER(SetBkColor, bkColor, COLORREF)
DC_SETTER(SetTextColor, textColor, COLORREF)
DC_SETTER(SetTextAlign, textAlign, UINT)
DC_SETTER(SetROP2, rop2, int)
DC_SETTER(SetPolyFillMode, polyFillMode, int)
DC_SETTER(SetStretchBltMode, stretchMode, int)

BOOL MoveToEx(HDC handle, int x, int y, LPPOINT previous)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    if (previous)
        *previous = dc->position;
    dc->position = {x, y};
    return TRUE;
}

BOOL GetCurrentPositionEx(HDC handle, LPPOINT position)
{
    DC *dc = dcOf(handle);
    if (!dc)
        return FALSE;
    *position = dc->position;
    return TRUE;
}

/* The screen */

void initGdi()
{
    makeStock();
    screenBitmap = newBitmap(SCREEN_WIDTH, SCREEN_HEIGHT, 8, false, true);
    screenBitmap->stock = true;
    track(screenBitmap);
}

} /* namespace miniwin */
