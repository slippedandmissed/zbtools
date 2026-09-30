/*
 * Palettes, on a 256-colour display: the system palette holds what the
 * screen shows; realizing a logical palette in the foreground puts its
 * colours there as Windows does (static colours matched exactly where they
 * may collapse, the rest in free entries from the first), and maps each
 * logical entry to the system entry it got. The game builds its palettes so
 * that this is the identity, and animates PC_RESERVED entries in place.
 */

#include <string.h>

#include "miniwin/internal.h"

namespace miniwin {

void trackObject(GdiObject *object);

PALETTEENTRY systemPalette[256];
unsigned systemPaletteVersion = 1;
UINT systemPaletteUse = SYSPAL_STATIC;

/* Windows' 20 static colours: 10 at each end. */
static const PALETTEENTRY staticColors[20] = {
    {0, 0, 0, 0}, {128, 0, 0, 0}, {0, 128, 0, 0}, {128, 128, 0, 0},
    {0, 0, 128, 0}, {128, 0, 128, 0}, {0, 128, 128, 0}, {192, 192, 192, 0},
    {192, 220, 192, 0}, {166, 202, 240, 0}, {255, 251, 240, 0}, {160, 160, 164, 0},
    {128, 128, 128, 0}, {255, 0, 0, 0}, {0, 255, 0, 0}, {255, 255, 0, 0},
    {0, 0, 255, 0}, {255, 0, 255, 0}, {0, 255, 255, 0}, {255, 255, 255, 0},
};

static bool isStatic(int index)
{
    if (systemPaletteUse == SYSPAL_NOSTATIC)
        return index == 0 || index == 255;
    return index < 10 || index >= 246;
}

static void restoreStatic()
{
    for (int i = 0; i < 10; i++) {
        systemPalette[i] = staticColors[i];
        systemPalette[246 + i] = staticColors[10 + i];
    }
}

void initPalette()
{
    memset(systemPalette, 0, sizeof systemPalette);
    restoreStatic();
}

Palette *defaultPalette()
{
    static Palette *palette;

    if (!palette) {
        palette = new Palette;
        palette->count = 20;
        memcpy(palette->entries, staticColors, sizeof staticColors);
        for (int i = 0; i < 10; i++) {
            palette->mapping[i] = (BYTE)i;
            palette->mapping[10 + i] = (BYTE)(246 + i);
        }
        palette->realized = true;
    }
    return palette;
}

static bool sameColor(const PALETTEENTRY &a, const PALETTEENTRY &b)
{
    return a.peRed == b.peRed && a.peGreen == b.peGreen && a.peBlue == b.peBlue;
}

static int nearestSystem(const PALETTEENTRY &e)
{
    int best = 0;
    long bestDistance = 1 << 30;

    for (int i = 0; i < 256; i++) {
        long dr = e.peRed - systemPalette[i].peRed, dg = e.peGreen - systemPalette[i].peGreen,
             db = e.peBlue - systemPalette[i].peBlue;
        long distance = dr * dr + dg * dg + db * db;
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
        }
    }
    return best;
}

static void paletteChanged()
{
    systemPaletteVersion++;
    screenChanged();
}

HPALETTE CreatePalette(const LOGPALETTE *logical)
{
    Palette *palette = new Palette;

    palette->count = logical->palNumEntries > 256 ? 256 : logical->palNumEntries;
    memcpy(palette->entries, logical->palPalEntry, palette->count * sizeof(PALETTEENTRY));
    for (int i = 0; i < 256; i++)
        palette->mapping[i] = (BYTE)i;
    trackObject(palette);
    return (HPALETTE)palette;
}

HPALETTE SelectPalette(HDC handle, HPALETTE paletteHandle, BOOL)
{
    DC *dc = dcOf(handle);
    Palette *palette = paletteOf(paletteHandle);

    if (!dc || !palette)
        return 0;
    Palette *previous = dc->palette;
    dc->palette = palette;
    return (HPALETTE)previous;
}

/* Realizes the DC's palette in the foreground: the number of system entries
   it changed. */
UINT realize(DC *dc)
{
    Palette *palette = dc->palette;

    if (!dc->surface || !dc->surface->device || palette == defaultPalette())
        return 0;
    PALETTEENTRY before[256];
    memcpy(before, systemPalette, sizeof before);
    bool assigned[256];
    for (int i = 0; i < 256; i++)
        assigned[i] = isStatic(i);
    int next = 0;
    for (int i = 0; i < palette->count; i++) {
        PALETTEENTRY e = palette->entries[i];
        if (e.peFlags & PC_EXPLICIT) {
            palette->mapping[i] = (BYTE)(e.peRed | e.peGreen << 8);
            continue;
        }
        int slot = -1;
        if (!(e.peFlags & (PC_NOCOLLAPSE | PC_RESERVED)))
            for (int j = 0; j < 256; j++)
                if (assigned[j] && sameColor(systemPalette[j], e)) {
                    slot = j;
                    break;
                }
        if (slot < 0) {
            while (next < 256 && assigned[next])
                next++;
            if (next < 256) {
                slot = next;
                systemPalette[slot] = e;
                systemPalette[slot].peFlags = 0;
                assigned[slot] = true;
            } else
                slot = nearestSystem(e);
        }
        palette->mapping[i] = (BYTE)slot;
    }
    palette->realized = true;
    UINT changed = 0;
    for (int i = 0; i < 256; i++)
        if (!sameColor(before[i], systemPalette[i]))
            changed++;
    if (changed) {
        paletteChanged();
        static bool telling;
        if (!telling) {
            telling = true;
            SendMessage(HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)(dc->window ? dc->window : mainWindow()), 0);
            telling = false;
        }
    }
    return changed;
}

UINT RealizePalette(HDC handle)
{
    DC *dc = dcOf(handle);
    return dc ? realize(dc) : GDI_ERROR;
}

BOOL AnimatePalette(HPALETTE handle, UINT start, UINT count, const PALETTEENTRY *entries)
{
    Palette *palette = paletteOf(handle);
    bool changed = false;

    if (!palette)
        return FALSE;
    for (UINT i = start; i < start + count && (int)i < palette->count; i++) {
        if (!(palette->entries[i].peFlags & PC_RESERVED))
            continue;
        BYTE flags = palette->entries[i].peFlags;
        palette->entries[i] = entries[i - start];
        palette->entries[i].peFlags = flags;
        if (palette->realized && !isStatic(palette->mapping[i])) {
            PALETTEENTRY &system = systemPalette[palette->mapping[i]];
            if (!sameColor(system, entries[i - start])) {
                system = entries[i - start];
                system.peFlags = 0;
                changed = true;
            }
        }
    }
    if (changed)
        paletteChanged();
    return TRUE;
}

UINT SetPaletteEntries(HPALETTE handle, UINT start, UINT count, const PALETTEENTRY *entries)
{
    Palette *palette = paletteOf(handle);

    if (!palette || palette->stock)
        return 0;
    UINT set = 0;
    for (UINT i = start; i < start + count && (int)i < palette->count; i++, set++)
        palette->entries[i] = entries[i - start];
    return set;
}

UINT GetPaletteEntries(HPALETTE handle, UINT start, UINT count, LPPALETTEENTRY entries)
{
    Palette *palette = paletteOf(handle);

    if (!palette)
        return 0;
    if (!entries)
        return (UINT)palette->count;
    UINT got = 0;
    for (UINT i = start; i < start + count && (int)i < palette->count; i++, got++)
        entries[i - start] = palette->entries[i];
    return got;
}

UINT GetSystemPaletteEntries(HDC, UINT start, UINT count, LPPALETTEENTRY entries)
{
    if (!entries)
        return 256;
    UINT got = 0;
    for (UINT i = start; i < start + count && i < 256; i++, got++)
        entries[i - start] = systemPalette[i];
    return got;
}

UINT SetSystemPaletteUse(HDC, UINT use)
{
    UINT previous = systemPaletteUse;

    if (use != SYSPAL_STATIC && use != SYSPAL_NOSTATIC)
        return SYSPAL_ERROR;
    systemPaletteUse = use;
    if (use != previous) {
        if (use == SYSPAL_STATIC)
            restoreStatic();
        else {
            systemPalette[0] = staticColors[0];
            systemPalette[255] = staticColors[19];
        }
        paletteChanged();
    }
    return previous;
}

UINT GetSystemPaletteUse(HDC)
{
    return systemPaletteUse;
}

} /* namespace miniwin */
