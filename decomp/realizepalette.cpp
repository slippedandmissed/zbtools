/*
 * realizepalette (Mohawk engine): realizing palettes; packed (run-length
 * encoded) pixels
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"
#include "os_manager.h"

/*
 * Realizes a palette in the main window's DC (the foreground window's if
 * the game's), taking over or giving back the static colours as need be.
 * When only reserved entries changed since it was last realized in the
 * foreground, animates them instead. Whether other palettes need
 * realizing again (0xffff on error).
 *
 * The test that would let a realization count as unchanged compares
 * `!graphics.depth` with 8, so it never passes, as in the original.
 *
 * Not exact: the original assigns registers and stack slots differently
 * (the active window in edi, the window and our thread on the stack).
 */
/* @zoombi32 0x0048cab4 */
unsigned short realizePalette(Palette *handle, short foreground)
{
    unsigned long ourThread;
    HWND window;
    HDC dc;
    HPALETTE old;
    short front;
    unsigned short reservedCount;
    unsigned short changedCount;
    unsigned short lowest;
    unsigned short highest;
    unsigned short half;
    PALETTEENTRY entries[256];
    Palette *palette;
    HWND active;
    unsigned long thread;
    short changed;
    int i;
    int j;
    UINT use;
    basePort *port;
    Palette *other;

    palette = handle ? checkPalette(handle, 1) : graphics.defaultPalette;
    if (!palette) {
        setPortError(0x2a70);
        return 0xffff;
    }
    if (graphics.realizing > 0) {
        setPortError(0x2a62);
        return 0xffff;
    }
    graphics.realizing++;
    if ((active = GetActiveWindow()) != 0)
        thread = GetWindowThreadProcessId(active, 0);
    else
        thread = 0;
    ourThread = appThreadId();
    if (thread != ourThread)
        active = appWindowHandle();
    window = active;
    dc = GetDC(window);
    old = SelectPalette(dc, palette->hpal, !foreground);
    front = foreground && thread == ourThread;
    if (front) {
        use = graphics.takeStatic ? SYSPAL_NOSTATIC : SYSPAL_STATIC;
        if (use != setSystemPaletteUse(dc, use))
            palette->realized = 0;
    } else if (thread != ourThread && (!thread || !isMohawkThread(thread))
               && graphics.systemPaletteUse == SYSPAL_NOSTATIC
               && GetSystemPaletteUse(dc) == SYSPAL_NOSTATIC)
        setSystemPaletteUse(dc, SYSPAL_STATIC);
    changed = 0;
    reservedCount = 0;
    changedCount = 0;
    lowest = 0xff;
    highest = 0;
    if (!palette->realized || palette->changed || front != palette->foreground || !front) {
        half = graphics.paletteReserved / 2;
        memcpy(entries, graphics.systemColors, half * sizeof(PALETTEENTRY));
        memcpy(&entries[256 - half], &graphics.systemColors[20 - half], half * sizeof(PALETTEENTRY));
        for (i = half; i < 256 - half; i++) {
            if (palette->entries[i].peFlags & 2) {
                changedCount++;
                palette->entries[i].peFlags &= ~2;
                if (palette->entries[i].peFlags & PC_RESERVED)
                    reservedCount++;
                if (i < lowest)
                    lowest = i;
                if (i > highest)
                    highest = i;
            }
            entries[i] = palette->entries[i];
            if (front && graphics.minimalReserve == graphics.takeStatic) {
                entries[i].peFlags |= PC_NOCOLLAPSE;
                if (entries[i].peFlags & PC_RESERVED)
                    continue;
                for (j = 256 - half; j < 256; j++)
                    if (entries[i].peRed == entries[j].peRed
                        && entries[i].peGreen == entries[j].peGreen
                        && entries[i].peBlue == entries[j].peBlue) {
                        entries[i].peFlags |= PC_RESERVED;
                        palette->realized = 0;
                        break;
                    }
            } else
                entries[i].peFlags &= ~PC_RESERVED;
        }
        if (reservedCount > 0 && reservedCount == changedCount && palette->realized
            && palette->foreground && front && graphics.minimalReserve == graphics.takeStatic
            && graphics.depth == 8 && lowest >= 10 && highest < 246) {
            AnimatePalette(palette->hpal, lowest, highest - lowest + 1, &entries[lowest]);
            changed = 0;
        } else {
            if (!palette->realized)
                UnrealizeObject(palette->hpal);
            SetPaletteEntries(palette->hpal, 0, 256, entries);
            RealizePalette(dc);
            if (palette->realized && palette->foreground && front
                && graphics.minimalReserve == graphics.takeStatic && !graphics.depth == 8)
                changed = 0;
            else
                changed = 1;
        }
    } else
        RealizePalette(dc);
    SelectPalette(dc, old, TRUE);
    ReleaseDC(window, dc);
    for (port = palette->ports; port; port = port->nextOnPalette)
        if (port->locks)
            port->realizePalette();
    if (foreground && (changed || changedCount != reservedCount))
        for (other = palette->next; palette != other; other = other->next)
            other->realized = 0;
    palette->realized = 1;
    palette->changed = 0;
    palette->foreground = front;
    graphics.realizing--;
    setPortError(0);
    return changed;
}

/* A big-endian word. */
static unsigned short packedWord(const unsigned char *data)
{
    return (unsigned short)(data[0] << 8 | data[1]);
}

/*
 * Draws packed pixels into an 8-bit bottom-up bitmap (`bits` + `offset` is
 * its top row) at x, y, clipped to `bounds` and `*clip`. Each row is a big-endian
 * length and then packets: a header byte, bit 7 set for a run of the next
 * byte, else that many literal bytes, the count being the low bits plus 1.
 * With `transparent`, runs of 0 are skipped.
 *
 * The original is in assembly (its inner loops, at 0x48cfc9 and 0x48d03f,
 * are jump targets rather than functions); this does the same in C++.
 */
/* @zoombi32-functional 0x0048cef4 */
void drawPackedPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x,
                      short y, const Rect &clip, unsigned short width, unsigned short height,
                      const unsigned char *data, short transparent)
{
    short over;
    unsigned short skipX;
    unsigned short skipY;
    unsigned char *row;
    unsigned char *dest;
    const unsigned char *p;
    const unsigned char *next;
    unsigned short skip;
    unsigned short left;
    unsigned short count;
    unsigned char header;
    unsigned char value;

    if (!sectRect(&bounds, (ShortRect *)&clip))
        return;
    skipX = 0;
    if ((over = x - bounds.left) < 0) {
        skipX = -over;
        x += skipX;
        if (width <= skipX)
            return;
        width -= skipX;
    }
    skipY = 0;
    if ((over = y - bounds.top) < 0) {
        skipY = -over;
        y += skipY;
        if (height <= skipY)
            return;
        height -= skipY;
    }
    if ((unsigned short)(x + width) > (unsigned short)bounds.right) {
        over = x + width - bounds.right;
        if (width <= (unsigned short)over)
            return;
        width -= over;
    }
    if ((unsigned short)(y + height) > (unsigned short)bounds.bottom) {
        over = y + height - bounds.bottom;
        if (height <= (unsigned short)over)
            return;
        height -= over;
    }
    row = bits - rowBytes * (unsigned short)y + (unsigned short)x + offset;
    while (skipY--)
        data += 2 + packedWord(data);
    for (;;) {
        next = data + 2 + packedWord(data);
        p = data + 2;
        skip = skipX;
        for (;;) {
            header = *p++;
            count = (header & 0x7f) + 1;
            if (skip < count)
                break;
            skip -= count;
            p += header & 0x80 ? 1 : count;
        }
        count -= skip;
        if (!(header & 0x80))
            p += skip;
        dest = row;
        left = width;
        for (;;) {
            if (count > left)
                count = left;
            if (header & 0x80) {
                value = *p++;
                if (value || !transparent)
                    memset(dest, value, count);
            } else {
                memcpy(dest, p, count);
                p += count;
            }
            dest += count;
            left -= count;
            if (!left)
                break;
            header = *p++;
            count = (header & 0x7f) + 1;
        }
        if (!--height)
            return;
        data = next;
        row -= rowBytes;
    }
}

/*
 * The pixel at x, y of packed pixels (as drawPackedPixels takes them).
 *
 * The original is in assembly; this does the same in C++.
 */
/* @zoombi32-functional 0x0048d13c */
unsigned char packedPixel(const unsigned char *data, unsigned short x, unsigned short y)
{
    const unsigned char *p;
    unsigned char header;
    unsigned short count;

    while (y--)
        data += 2 + packedWord(data);
    p = data + 2;
    for (;;) {
        header = *p++;
        count = (header & 0x7f) + 1;
        if (x < count)
            break;
        x -= count;
        p += header & 0x80 ? 1 : count;
    }
    if (!(header & 0x80))
        p += x;
    return *p;
}
