/*
 * Colour matching, raster operations and the blits: BitBlt, StretchBlt,
 * StretchDIBits, PatBlt, ScrollDC and GetPixel.
 *
 * On a palette device GDI works in pixel values: the screen's are system
 * palette indices, an 8-bit DIB section's index its colour table, and raster
 * operations combine the values bit by bit. A source in another format is
 * translated to the destination's values first, through its colour table and
 * the nearest colour (the game's colours match exactly, so that's the
 * identity, found quickly).
 */

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

uint8_t nearestIndex(const RGBQUAD *table, int count, int r, int g, int b)
{
    int best = 0;
    long bestDistance = 1L << 30;

    for (int i = 0; i < count; i++) {
        long dr = r - table[i].rgbRed, dg = g - table[i].rgbGreen, db = b - table[i].rgbBlue;
        long distance = dr * dr + dg * dg + db * db;
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
            if (!distance)
                break;
        }
    }
    return (uint8_t)best;
}

static RGBQUAD quad(const PALETTEENTRY &e)
{
    return {e.peBlue, e.peGreen, e.peRed, 0};
}

static bool same(const RGBQUAD &a, const RGBQUAD &b)
{
    return a.rgbRed == b.rgbRed && a.rgbGreen == b.rgbGreen && a.rgbBlue == b.rgbBlue;
}

/* A destination's colours: its pixel values and what they show. */
struct Target
{
    RGBQUAD colors[256];
    int count;
    bool mono;
};

static void targetOf(DC *dc, Target &target)
{
    Bitmap *surface = dc->surface;

    target.mono = surface->bpp == 1;
    if (surface->device) {
        for (int i = 0; i < 256; i++)
            target.colors[i] = quad(systemPalette[i]);
        target.count = 256;
    } else {
        memcpy(target.colors, surface->table.colors, sizeof target.colors);
        target.count = surface->table.count ? surface->table.count : 1;
    }
}

/* The pixel value for a colour: the logical palette's entry for it on the
   device (by its index, if the colour there matches), else the nearest. */
static uint8_t deviceValueOf(DC *dc, const RGBQUAD &color, int hint)
{
    Palette *palette = dc->palette;

    if (hint >= 0 && hint < palette->count && same(quad(palette->entries[hint]), color))
        return palette->mapping[hint];
    for (int i = 0; i < palette->count; i++)
        if (same(quad(palette->entries[i]), color))
            return palette->mapping[i];
    RGBQUAD entries[256];
    for (int i = 0; i < palette->count; i++)
        entries[i] = quad(palette->entries[i]);
    return palette->mapping[nearestIndex(entries, palette->count, color.rgbRed, color.rgbGreen,
                                         color.rgbBlue)];
}

static uint8_t valueOf(DC *dc, const Target &target, const RGBQUAD &color, int hint)
{
    if (target.mono)
        return color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 >= 12750 ? 1 : 0;
    if (dc->surface->device)
        return deviceValueOf(dc, color, hint);
    if (hint >= 0 && hint < target.count && same(target.colors[hint], color))
        return (uint8_t)hint;
    return nearestIndex(target.colors, target.count, color.rgbRed, color.rgbGreen, color.rgbBlue);
}

uint8_t pixelFor(DC *dc, COLORREF color)
{
    Target target;
    Palette *palette = dc->palette;

    if ((color & 0xffff0000) == 0x10ff0000)
        return (uint8_t)(color & 0xff); /* DIBINDEX */
    targetOf(dc, target);
    if (color >> 24 == 1) {
        WORD index = (WORD)(color & 0xffff);
        if (index >= palette->count)
            index = 0;
        if (dc->surface->device && !target.mono)
            return palette->mapping[index];
        return valueOf(dc, target, quad(palette->entries[index]), index);
    }
    RGBQUAD rgb = {GetBValue(color), GetGValue(color), GetRValue(color), 0};
    if (dc->surface->device && !target.mono && color >> 24 == 0) {
        /* A plain RGB colour on the device: the nearest static colour. */
        RGBQUAD statics[20];
        BYTE indices[20];
        for (int i = 0; i < 10; i++) {
            statics[i] = quad(systemPalette[i]);
            indices[i] = (BYTE)i;
            statics[10 + i] = quad(systemPalette[246 + i]);
            indices[10 + i] = (BYTE)(246 + i);
        }
        return indices[nearestIndex(statics, 20, rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue)];
    }
    return valueOf(dc, target, rgb, -1);
}

RGBQUAD colorOfPixel(DC *dc, uint8_t pixel)
{
    Bitmap *surface = dc->surface;

    if (surface->device)
        return quad(systemPalette[pixel]);
    if (pixel < surface->table.count)
        return surface->table.colors[pixel];
    return {0, 0, 0, 0};
}

/* Translations, cached by what they depend on. */

static uint64_t hashBytes(uint64_t hash, const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < size; i++)
        hash = (hash ^ bytes[i]) * 0x100000001b3ULL;
    return hash;
}

static std::unordered_map<uint64_t, std::vector<uint8_t>> translations;

void translation(DC *to, SourceKind kind, const void *table, int count, uint8_t map[256])
{
    Target target;
    targetOf(to, target);
    Palette *palette = to->palette;

    uint64_t key = 0xcbf29ce484222325ULL;
    key = hashBytes(key, &kind, sizeof kind);
    key = hashBytes(key, &count, sizeof count);
    if (kind != SOURCE_DEVICE)
        key = hashBytes(key, table, (size_t)count * (kind == SOURCE_RGB ? sizeof(RGBQUAD) : sizeof(WORD)));
    key = hashBytes(key, &target.mono, sizeof target.mono);
    key = hashBytes(key, target.colors, (size_t)target.count * sizeof(RGBQUAD));
    if (to->surface->device || kind == SOURCE_PALETTE) {
        key = hashBytes(key, palette->entries, (size_t)palette->count * sizeof(PALETTEENTRY));
        key = hashBytes(key, palette->mapping, sizeof palette->mapping);
    }
    auto found = translations.find(key);
    if (found != translations.end()) {
        memcpy(map, found->second.data(), 256);
        return;
    }

    memset(map, 0, 256);
    for (int v = 0; v < count && v < 256; v++) {
        switch (kind) {
        case SOURCE_DEVICE:
            map[v] = to->surface->device && !target.mono
                         ? (uint8_t)v
                         : valueOf(to, target, quad(systemPalette[v]), v);
            break;
        case SOURCE_RGB:
            map[v] = valueOf(to, target, ((const RGBQUAD *)table)[v], v);
            break;
        case SOURCE_PALETTE: {
            WORD index = ((const WORD *)table)[v];
            if (index >= palette->count)
                index = 0;
            if (to->surface->device && !target.mono)
                map[v] = palette->mapping[index];
            else
                map[v] = valueOf(to, target, quad(palette->entries[index]), index);
            break;
        }
        }
    }
    if (translations.size() > 256)
        translations.clear();
    translations[key].assign(map, map + 256);
}

/* Raster operations: the ROP3's truth table, applied to each bit. */

uint8_t rop3(DWORD rop, uint8_t p, uint8_t s, uint8_t d)
{
    switch (rop) {
    case SRCCOPY:
        return s;
    case PATCOPY:
        return p;
    case SRCAND:
        return s & d;
    case SRCPAINT:
        return s | d;
    case SRCINVERT:
        return s ^ d;
    case PATINVERT:
        return p ^ d;
    case NOTSRCCOPY:
        return (uint8_t)~s;
    case DSTINVERT:
        return (uint8_t)~d;
    case BLACKNESS:
        return 0;
    case WHITENESS:
        return 0xff;
    }
    uint8_t table = (uint8_t)(rop >> 16);
    uint8_t result = 0;
    for (int i = 0; i < 8; i++)
        if (table & (1 << i))
            result |= (uint8_t)((i & 4 ? p : ~p) & (i & 2 ? s : ~s) & (i & 1 ? d : ~d));
    return result;
}

bool ropUsesSource(DWORD rop)
{
    uint8_t t = (uint8_t)(rop >> 16);
    return ((t >> 2) & 0x33) != (t & 0x33);
}

bool ropUsesPattern(DWORD rop)
{
    uint8_t t = (uint8_t)(rop >> 16);
    return ((t >> 4) & 0x0f) != (t & 0x0f);
}

/* A brush as an 8x8 tile of the destination's pixel values, aligned to the
   device's origin. */
static void brushTile(DC *dc, Brush *brush, uint8_t tile[64])
{
    if (!brush || !brush->pattern) {
        memset(tile, brush ? pixelFor(dc, brush->color) : 0, 64);
        return;
    }
    uint8_t map[256];
    if (brush->patternUsage == DIB_PAL_COLORS)
        translation(dc, SOURCE_PALETTE, brush->patternIndices, brush->pattern->table.count, map);
    else
        translation(dc, SOURCE_RGB, brush->pattern->table.colors, brush->pattern->table.count,
                    map);
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            tile[y * 8 + x] = map[brush->pattern->row(y)[x]];
}

uint8_t brushPixel(DC *dc, Brush *brush, int x, int y)
{
    uint8_t tile[64];
    brushTile(dc, brush, tile);
    return tile[(y & 7) * 8 + (x & 7)];
}

/* Surface access, for 8-bit and 1-bit surfaces. */

static inline uint8_t readPixel(const Bitmap *b, int x, int y)
{
    const uint8_t *row = b->row(y);
    switch (b->bpp) {
    case 8:
        return row[x];
    case 1:
        return (uint8_t)(row[x >> 3] >> (7 - (x & 7)) & 1);
    case 4:
        return (uint8_t)(row[x >> 1] >> (x & 1 ? 0 : 4) & 15);
    }
    return 0;
}

static inline void writePixel(Bitmap *b, int x, int y, uint8_t value)
{
    uint8_t *row = b->row(y);
    switch (b->bpp) {
    case 8:
        row[x] = value;
        break;
    case 1: {
        uint8_t bit = (uint8_t)(0x80 >> (x & 7));
        row[x >> 3] = (uint8_t)(value & 1 ? row[x >> 3] | bit : row[x >> 3] & ~bit);
        break;
    }
    case 4: {
        int shift = x & 1 ? 0 : 4;
        row[x >> 1] = (uint8_t)((row[x >> 1] & ~(15 << shift)) | (value & 15) << shift);
        break;
    }
    }
}

static bool drawable(Bitmap *b)
{
    return b && (b->bpp == 8 || b->bpp == 1 || b->bpp == 4);
}

void fillRectDevice(DC *dc, RECT rect, Brush *brush, DWORD rop)
{
    Bitmap *surface = dc->surface;
    uint8_t tile[64];

    if (!drawable(surface) || (brush && brush->null && ropUsesPattern(rop)))
        return;
    brushTile(dc, brush, tile);
    for (RECT clip : clipRects(dc)) {
        if (!intersect(clip, rect))
            continue;
        for (LONG y = clip.top; y < clip.bottom; y++) {
            int sy = y + dc->origin.y;
            for (LONG x = clip.left; x < clip.right; x++) {
                int sx = x + dc->origin.x;
                uint8_t p = tile[(y & 7) * 8 + (x & 7)];
                if (rop == PATCOPY && surface->bpp == 8)
                    surface->row(sy)[sx] = p;
                else
                    writePixel(surface, sx, sy, rop3(rop, p, 0, readPixel(surface, sx, sy)));
            }
        }
    }
    if (surface->device)
        screenChanged();
}

void fillSpans(DC *dc, const std::vector<RECT> &spans, Brush *brush, DWORD rop)
{
    for (const RECT &span : spans)
        fillRectDevice(dc, span, brush, rop);
}

BOOL PatBlt(HDC handle, int x, int y, int width, int height, DWORD rop)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return FALSE;
    fillRectDevice(dc, toDeviceRect(dc, x, y, x + width, y + height), dc->brush, rop);
    return TRUE;
}

/* A source of pixels for a blit: an 8-bit-or-less image and how its
   values translate into the destination's (or, for deeper images, colours
   matched one by one). */
struct Source
{
    int width, height;
    int bpp;
    const uint8_t *top;
    long stride;
    uint8_t map[256];
    /* 16-bit and deeper */
    DWORD masks[3];
};

static inline uint8_t sourceValue(const Source &s, int x, int y)
{
    const uint8_t *row = s.top + y * s.stride;
    switch (s.bpp) {
    case 8:
        return s.map[row[x]];
    case 4:
        return s.map[row[x >> 1] >> (x & 1 ? 0 : 4) & 15];
    case 1:
        return s.map[row[x >> 3] >> (7 - (x & 7)) & 1];
    }
    return 0;
}

static inline RGBQUAD deepColor(const Source &s, int x, int y)
{
    const uint8_t *row = s.top + y * s.stride;
    if (s.bpp == 24)
        return {row[x * 3], row[x * 3 + 1], row[x * 3 + 2], 0};
    if (s.bpp == 32)
        return {row[x * 4], row[x * 4 + 1], row[x * 4 + 2], 0};
    unsigned value = row[x * 2] | row[x * 2 + 1] << 8;
    RGBQUAD color;
    if (s.masks[1] == 0x7e0) { /* 565 */
        color.rgbRed = (BYTE)((value >> 11 & 31) * 255 / 31);
        color.rgbGreen = (BYTE)((value >> 5 & 63) * 255 / 63);
    } else {
        color.rgbRed = (BYTE)((value >> 10 & 31) * 255 / 31);
        color.rgbGreen = (BYTE)((value >> 5 & 31) * 255 / 31);
    }
    color.rgbBlue = (BYTE)((value & 31) * 255 / 31);
    color.rgbReserved = 0;
    return color;
}

/*
 * Copies source rectangle (sx, sy, sw, sh) to the destination's device
 * rectangle from corner (x0, y0) to (x1, y1), which may be reversed (a
 * mirror image), stretching by nearest pixels, through the raster operation.
 */
static void blit(DC *to, LONG x0, LONG y0, LONG x1, LONG y1, const Source &source, int sx, int sy,
                 int sw, int sh, DWORD rop)
{
    Bitmap *surface = to->surface;
    RECT area = {std::min(x0, x1), std::min(y0, y1), std::max(x0, x1), std::max(y0, y1)};
    bool useSource = ropUsesSource(rop);
    bool usePattern = ropUsesPattern(rop);
    uint8_t tile[64];

    if (!drawable(surface) || area.left == area.right || area.top == area.bottom)
        return;
    if (usePattern)
        brushTile(to, to->brush, tile);
    Target target;
    if (source.bpp > 8)
        targetOf(to, target);
    std::vector<int> columns((size_t)(area.right - area.left));
    for (LONG x = area.left; x < area.right; x++) {
        double t = (x - x0 + (x1 > x0 ? 0.5 : -0.5)) / (double)(x1 - x0);
        int c = sx + (int)(t * sw);
        columns[(size_t)(x - area.left)] = std::min(std::max(c, sx), sx + sw - 1);
    }
    for (RECT clip : clipRects(to)) {
        if (!intersect(clip, area))
            continue;
        for (LONG y = clip.top; y < clip.bottom; y++) {
            double t = (y - y0 + (y1 > y0 ? 0.5 : -0.5)) / (double)(y1 - y0);
            int row = std::min(std::max(sy + (int)(t * sh), sy), sy + sh - 1);
            bool rowInside = row >= 0 && row < source.height;
            int dy = y + to->origin.y;
            for (LONG x = clip.left; x < clip.right; x++) {
                int column = columns[(size_t)(x - area.left)];
                int dx = x + to->origin.x;
                uint8_t s = 0;
                if (useSource) {
                    if (!rowInside || column < 0 || column >= source.width)
                        continue; /* outside the source: nothing to copy */
                    if (source.bpp <= 8)
                        s = sourceValue(source, column, row);
                    else {
                        RGBQUAD c = deepColor(source, column, row);
                        s = valueOf(to, target, c, -1);
                    }
                }
                if (rop == SRCCOPY && surface->bpp == 8) {
                    surface->row(dy)[dx] = s;
                    continue;
                }
                uint8_t p = usePattern ? tile[(y & 7) * 8 + (x & 7)] : 0;
                writePixel(surface, dx, dy, rop3(rop, p, s, readPixel(surface, dx, dy)));
            }
        }
    }
    if (surface->device)
        screenChanged();
}

/* A DC's surface as a source for another DC, copied first when they share
   a surface (a blit onto itself, say to mirror it). */
static bool dcSource(DC *from, DC *to, Source &source, std::vector<uint8_t> &copy)
{
    Bitmap *b = from->surface;

    if (!b || !drawable(b))
        return false;
    source.width = b->width;
    source.height = b->height;
    source.bpp = b->bpp;
    source.top = b->top;
    source.stride = b->stride;
    if (b == to->surface) {
        long rowBytes = b->stride < 0 ? -b->stride : b->stride;
        copy.resize((size_t)(rowBytes * b->height));
        for (int y = 0; y < b->height; y++)
            memcpy(&copy[(size_t)(y * rowBytes)], b->row(y), (size_t)rowBytes);
        source.top = copy.data();
        source.stride = rowBytes;
    }
    Target target;
    targetOf(to, target);
    if (b->bpp == 1 && !target.mono) {
        /* Monochrome to colour: 0 is the text colour, 1 the background. */
        source.map[0] = pixelFor(to, to->textColor);
        source.map[1] = pixelFor(to, to->bkColor);
    } else if (target.mono && b->bpp != 1) {
        /* Colour to monochrome: the source's background colour is white. */
        uint8_t background = pixelFor(from, from->bkColor);
        for (int v = 0; v < 256; v++)
            source.map[v] = v == background ? 1 : 0;
    } else if (b->device)
        translation(to, SOURCE_DEVICE, 0, 256, source.map);
    else
        translation(to, SOURCE_RGB, b->table.colors, b->table.count ? b->table.count : 256,
                    source.map);
    return true;
}

BOOL StretchBlt(HDC toHandle, int x, int y, int width, int height, HDC fromHandle, int fromX,
                int fromY, int fromWidth, int fromHeight, DWORD rop)
{
    DC *to = dcOf(toHandle);
    DC *from = dcOf(fromHandle);
    Source source;
    std::vector<uint8_t> copy;

    if (!to)
        return FALSE;
    LONG x0 = x, y0 = y, x1 = x + width, y1 = y + height;
    toDevice(to, x0, y0);
    toDevice(to, x1, y1);
    if (!ropUsesSource(rop)) {
        fillRectDevice(to, {std::min(x0, x1), std::min(y0, y1), std::max(x0, x1), std::max(y0, y1)},
                       to->brush, rop);
        return TRUE;
    }
    if (!from || !dcSource(from, to, source, copy))
        return FALSE;
    LONG sx0 = fromX, sy0 = fromY, sx1 = fromX + fromWidth, sy1 = fromY + fromHeight;
    toDevice(from, sx0, sy0);
    toDevice(from, sx1, sy1);
    sx0 += from->origin.x;
    sx1 += from->origin.x;
    sy0 += from->origin.y;
    sy1 += from->origin.y;
    /* A reversed source mirrors too. */
    if (sx1 < sx0) {
        std::swap(sx0, sx1);
        std::swap(x0, x1);
    }
    if (sy1 < sy0) {
        std::swap(sy0, sy1);
        std::swap(y0, y1);
    }
    blit(to, x0, y0, x1, y1, source, (int)sx0, (int)sy0, (int)(sx1 - sx0), (int)(sy1 - sy0), rop);
    return TRUE;
}

BOOL BitBlt(HDC to, int x, int y, int width, int height, HDC from, int fromX, int fromY, DWORD rop)
{
    return StretchBlt(to, x, y, width, height, from, fromX, fromY, width, height, rop);
}

/* Windows' RLE8: pairs of count and value, or 0 and an escape (end of line,
   end of bitmap, a move, or that many literal bytes, word-aligned). */
static void decodeRle8(const uint8_t *data, size_t size, int width, int height,
                       std::vector<uint8_t> &out, long rowBytes)
{
    out.assign((size_t)(rowBytes * height), 0);
    int x = 0, y = 0;
    size_t at = 0;
    while (at + 1 < size && y < height) {
        uint8_t count = data[at++], value = data[at++];
        if (count) {
            for (int i = 0; i < count && x < width; i++)
                out[(size_t)(y * rowBytes + x++)] = value;
            continue;
        }
        switch (value) {
        case 0:
            x = 0;
            y++;
            break;
        case 1:
            return;
        case 2:
            if (at + 1 >= size)
                return;
            x += data[at++];
            y += data[at++];
            break;
        default:
            for (int i = 0; i < value && at < size; i++, at++)
                if (x < width)
                    out[(size_t)(y * rowBytes + x++)] = data[at];
            if (value & 1)
                at++;
        }
    }
}

int StretchDIBits(HDC handle, int x, int y, int width, int height, int fromX, int fromY,
                  int fromWidth, int fromHeight, const void *bits, const BITMAPINFO *info,
                  UINT usage, DWORD rop)
{
    DC *dc = dcOf(handle);
    const BITMAPINFOHEADER &header = info->bmiHeader;
    Source source;
    std::vector<uint8_t> decoded;

    if (!dc)
        return 0;
    source.bpp = header.biBitCount;
    source.width = header.biWidth;
    source.height = header.biHeight < 0 ? -header.biHeight : header.biHeight;
    long rowBytes = ((long)source.width * source.bpp + 31) / 32 * 4;
    const uint8_t *data = (const uint8_t *)bits;
    const uint8_t *afterHeader = (const uint8_t *)info + header.biSize;
    bool bottomUp = header.biHeight > 0;
    if (header.biCompression == BI_RLE8) {
        decodeRle8(data, header.biSizeImage, source.width, source.height, decoded, rowBytes);
        data = decoded.data();
        bottomUp = true; /* the decoded rows are in the order they came */
    } else if (header.biCompression != BI_RGB && header.biCompression != BI_BITFIELDS) {
        unsupported("StretchDIBits compression");
        return 0;
    }
    if (bottomUp) {
        source.top = data + rowBytes * (source.height - 1);
        source.stride = -rowBytes;
    } else {
        source.top = data;
        source.stride = rowBytes;
    }
    source.masks[0] = 0x7c00;
    source.masks[1] = 0x3e0;
    source.masks[2] = 0x1f;
    if (source.bpp <= 8) {
        int count = header.biClrUsed ? (int)header.biClrUsed : 1 << source.bpp;
        translation(dc, usage == DIB_PAL_COLORS ? SOURCE_PALETTE : SOURCE_RGB, afterHeader,
                    count > 256 ? 256 : count, source.map);
    } else if (header.biCompression == BI_BITFIELDS)
        memcpy(source.masks, afterHeader, sizeof source.masks);

    /* The source rectangle counts rows from the bottom of a bottom-up DIB. */
    int top = bottomUp ? source.height - (fromY + fromHeight) : fromY;
    LONG x0 = x, y0 = y, x1 = x + width, y1 = y + height;
    toDevice(dc, x0, y0);
    toDevice(dc, x1, y1);
    int sw = fromWidth, sh = fromHeight;
    int sx = fromX;
    if (sw < 0) {
        sx += sw;
        sw = -sw;
        std::swap(x0, x1);
    }
    if (sh < 0) {
        top += sh;
        sh = -sh;
        std::swap(y0, y1);
    }
    blit(dc, x0, y0, x1, y1, source, sx, top, sw, sh, rop);
    return sh;
}

BOOL ScrollDC(HDC handle, int dx, int dy, const RECT *scroll, const RECT *clip, HRGN update,
              LPRECT updateRect)
{
    DC *dc = dcOf(handle);

    if (!dc || !drawable(dc->surface))
        return FALSE;
    RECT area = scroll ? toDeviceRect(dc, scroll->left, scroll->top, scroll->right, scroll->bottom)
                       : dc->visible;
    RECT limit = clip ? toDeviceRect(dc, clip->left, clip->top, clip->right, clip->bottom) : area;
    RECT visible = dc->visible;
    intersect(area, visible);
    intersect(limit, visible);
    RECT source = area;
    intersect(source, limit);
    RECT moved = {source.left + dx, source.top + dy, source.right + dx, source.bottom + dy};
    intersect(moved, limit);

    Bitmap *surface = dc->surface;
    if (moved.right > moved.left && moved.bottom > moved.top) {
        int w = moved.right - moved.left, h = moved.bottom - moved.top;
        std::vector<uint8_t> copy((size_t)(w * h));
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                copy[(size_t)(y * w + x)] = readPixel(surface, moved.left - dx + x + dc->origin.x,
                                                      moved.top - dy + y + dc->origin.y);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                writePixel(surface, moved.left + x + dc->origin.x, moved.top + y + dc->origin.y,
                           copy[(size_t)(y * w + x)]);
        if (surface->device)
            screenChanged();
    }
    /* What's uncovered: the clip area less where the pixels went. */
    HRGN uncovered = CreateRectRgn(limit.left, limit.top, limit.right, limit.bottom);
    HRGN filled = CreateRectRgn(moved.left, moved.top, moved.right, moved.bottom);
    CombineRgn(uncovered, uncovered, filled, RGN_DIFF);
    if (update)
        CombineRgn(update, uncovered, 0, RGN_COPY);
    if (updateRect)
        *updateRect = regionOf(uncovered)->bounds();
    DeleteObject(filled);
    DeleteObject(uncovered);
    return TRUE;
}

COLORREF GetPixel(HDC handle, int x, int y)
{
    DC *dc = dcOf(handle);

    if (!dc || !drawable(dc->surface))
        return CLR_INVALID;
    LONG dx = x, dy = y;
    toDevice(dc, dx, dy);
    for (const RECT &clip : clipRects(dc))
        if (dx >= clip.left && dx < clip.right && dy >= clip.top && dy < clip.bottom) {
            RGBQUAD c = colorOfPixel(dc, readPixel(dc->surface, dx + dc->origin.x, dy + dc->origin.y));
            return RGB(c.rgbRed, c.rgbGreen, c.rgbBlue);
        }
    return CLR_INVALID;
}

/* For draw.cpp and text.cpp. */
void plot(DC *dc, int x, int y, uint8_t value, int rop2)
{
    Bitmap *surface = dc->surface;
    uint8_t d = readPixel(surface, x, y);
    uint8_t p = value, r;

    switch (rop2) {
    case R2_BLACK: r = 0; break;
    case R2_NOTMERGEPEN: r = (uint8_t)~(p | d); break;
    case R2_MASKNOTPEN: r = (uint8_t)(~p & d); break;
    case R2_NOTCOPYPEN: r = (uint8_t)~p; break;
    case R2_MASKPENNOT: r = (uint8_t)(p & ~d); break;
    case R2_NOT: r = (uint8_t)~d; break;
    case R2_XORPEN: r = (uint8_t)(p ^ d); break;
    case R2_NOTMASKPEN: r = (uint8_t)~(p & d); break;
    case R2_MASKPEN: r = (uint8_t)(p & d); break;
    case R2_NOTXORPEN: r = (uint8_t)~(p ^ d); break;
    case R2_NOP: r = d; break;
    case R2_MERGENOTPEN: r = (uint8_t)(~p | d); break;
    case R2_MERGEPENNOT: r = (uint8_t)(p | ~d); break;
    case R2_MERGEPEN: r = (uint8_t)(p | d); break;
    case R2_WHITE: r = 0xff; break;
    default: r = p;
    }
    writePixel(surface, x, y, r);
}

bool isDrawable(DC *dc)
{
    return dc && drawable(dc->surface);
}

} /* namespace miniwin */
