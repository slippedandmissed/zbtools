/*
 * graphics (0x4144d0-0x414f30): 'unable to initialize graphics', 'unable to create palette', 'work port', 'back port'
 *
 * The game's drawing layer over the Mohawk engine: the screen and work ports,
 * the palette, images, saved areas of the screen (e2MapSave) and clip regions.
 */

#include <string.h>
#include "zoombinis.h"
#include "e2memory.h"
#include "graphics.h"
#include "jointext.h"
#include "loading.h"
#include "platform.h"

/*
 * Sets up graphics in a display mode: the engine, the main window, the
 * palette (all reserved entries) and the work port the game draws into.
 * msgNoScreenPort is a named string (placeGamePort uses it too).
 */
/* @zoombi32 0x004144d0 */
void initGraphics(DisplayMode *mode, short depth)
{
    short width, height;
    short i;

    g_4aa7ce = depth;
    width = mode->width;
    height = mode->height;
    checkDisplayMode(mode);
    displayMode = *mode;
    g_4ab404 = mode->palettized;
    if (openGraphicsEngine(mode, 0))
        fatalError("unable to initialize graphics");
    setMinimalReserve(depth);
    gameRect.right = width;
    gameRect.bottom = height;
    if (!createMainWindow(width, height))
        fatalError(msgNoScreenPort);
    g_4aa7b8 = gameRect;
    sectRect(&g_4aa7b8, &screenRect);
    if (!allocateBlock((void **)&g_4ab3f0, 0x400))
        notEnoughNearMemory("initial RGB's");
    memset(g_4ab3f0, 0, 4);
    for (i = 0; i < 0x100; i++)
        g_4ab3f0[i].peFlags = PC_RESERVED;
    if ((palette = newPalette(0x100, (ColorBytes *)g_4ab3f0)) == 0)
        fatalError("unable to create palette");
    freeAndClear((void **)&g_4ab3f0);
    setPort(screenPort);
    setPortPalette(palette);
    createPort(&workPort, &gameRect, 1, "work port");
    setPort(workPort);
    setColors(0, 0, 0x100);
    if (!(short)isMousePresent()) /* the original tests only ax */
        showCursor();
}

/* Frees what initGraphics set up, and the main window. */
/* @zoombi32 0x00414653 */
void closeGraphics()
{
    freeText((void **)&g_4ab3f4);
    freeText((void **)&g_4ab3f8);
    freeText((void **)&g_4ab3fc);
    freeText((void **)&g_4ab400);
    destroyPort(&workPort, 1);
    if (palette) {
        setPort(screenPort);
        setPortPalette(0);
        deletePalette(palette);
        palette = 0;
    }
    freeAndClear((void **)&g_4ab3f0);
    destroyMainWindow();
    if (graphicsBufferSize()) {
        setCursorLevel(isMousePresent() - 1);
        closeGraphicsEngine();
    }
}

/*
 * Draws image `index` (from 1) of a list at (x, y), in a drawing mode.
 * `anchor` places it: 0x11 by its top left; else horizontally by its left
 * (1) or centre (2) or right, and vertically by its top (0x10) or centre
 * (0x20) or bottom. Image data starts with its size, big-endian.
 */
/* @zoombi32 0x004146df */
void drawImage(ResourceList *images, short index, short x, short y, short mode, short anchor)
{
    short handle;
    unsigned short *image;
    short width, height;

    if (index > 0) {
        handle = fn_46beac(images->resources[index - 1]);
        image = (unsigned short *)lockHandle(handle);
        if (anchor != 0x11) {
            width = swapShort(image[0]);
            height = swapShort(image[1]);
            if (anchor & 1)
                width = 0;
            else if (anchor & 2)
                width >>= 1;
            if (anchor & 0x10)
                height = 0;
            else if (anchor & 0x20)
                height >>= 1;
            x -= width;
            y -= height;
        }
        drawImageData(image, x, y, mode);
        unlockHandle(handle);
    }
}

/* drawImage in a colour (a palette index). */
/* @zoombi32 0x0041479d */
void drawImageInColor(ResourceList *images, short index, short x, short y, short mode, short color,
                      short anchor)
{
    Color saved;

    saved = getForeColor();
    setForeColor(Color(color));
    drawImage(images, index, x, y, mode, anchor);
    setForeColor(saved);
}

/* Copies `count` of the palette's colours, from `first`. */
/* @zoombi32 0x0041481d */
void getColors(PALETTEENTRY *to, short first, short count)
{
    memcpy(to, &colors[first], count * sizeof(PALETTEENTRY));
}

/* Sets `count` of the palette's colours from `first` (to black without
   `from`), and shows the result unless g_4ab404. */
/* @zoombi32 0x00414845 */
void setColors(PALETTEENTRY *from, short first, short count)
{
    Palette *current;

    if (count > 0) {
        current = getPortPalette();
        if (!from)
            memset(&colors[first], 0, count * sizeof(PALETTEENTRY));
        else
            memcpy(&colors[first], from, count * sizeof(PALETTEENTRY));
        setPaletteColors(current, first, count, (ColorBytes *)&colors[first]);
        realizePalette(current, 1);
        if (!g_4ab404)
            showRect(&gameRect);
    }
}

/* Copies `count` entries of g_4aa7e8 to g_4aabe8, from `first`. */
/* @zoombi32 0x004148da */
void fn_4148da(short first, short count)
{
    memcpy(&g_4aabe8[first], &g_4aa7e8[first], count * sizeof(PALETTEENTRY));
}

/*
 * Creates an off-screen port the size of `bounds`, in the palette, with its
 * origin at the bounds' corner; unless `keep`, it's released again
 * (unlockPort). `name` is for error messages.
 */
/* @zoombi32 0x0041490e */
void createPort(basePort **port, ShortRect *bounds, short keep, const char *name)
{
    short width;
    short height;
    basePort *saved;
    Palette *current;

    joinText(&g_4ab3f4, name, "back port");
    if (*port) {
        joinText(&g_4ab3f8, "e2GetBackPort error:", g_4ab3f4);
        reportJoinedError(g_4ab3f8);
    }
    width = bounds->right - bounds->left;
    height = bounds->bottom - bounds->top;
    if ((*port = newPort(width, height, bitsPerPixel, 0)) == 0)
        reportJoinedError(g_4ab3f4);
    saved = getPort();
    current = getPortPalette();
    lockPortOrFail(*port);
    setPort(*port);
    setPortPalette(current);
    fn_414a2e(*port, bounds);
    if (!keep)
        unlockPort(*port);
    setPort(saved);
    freeText((void **)&g_4ab3f4);
}

/* Destroys a port (releasing it first, if asked), leaving no current port if
   it was current. */
/* @zoombi32 0x004149e0 */
void destroyPort(basePort **port, short release)
{
    freeText((void **)&g_4ab3f4);
    freeText((void **)&g_4ab3f8);
    if (*port) {
        if (getPort() == *port)
            setPort(0);
        if (release)
            unlockPort(*port);
        deletePort(*port);
        *port = 0;
    }
}

/* Gives a port its origin and clipping from `bounds`. */
/* @zoombi32 0x00414a2e */
void fn_414a2e(basePort *port, ShortRect *bounds)
{
    basePort *saved = getPort();

    setPort(port);
    setOrigin(bounds->left, bounds->top);
    setClipRect(*bounds);
    setPort(saved);
}

/*
 * Saves an area of the current port into a new MapSave (which must be
 * empty), locked once more if `locked`. `name` is for error messages.
 */
/* @zoombi32 0x00414a80 */
void saveRect(MapSave **save, ShortRect *rect, short locked, const char *name)
{
    if (*save) {
        joinText(&g_4ab400, "e2SaveRect error:", name);
        reportJoinedError(g_4ab400);
    }
    if (!allocateBlock((void **)save, sizeof(MapSave))) {
        joinText(&g_4ab3fc, name, "e2MapSave structure");
        reportJoinedError(g_4ab3fc);
    }
    (*save)->rect = *rect;
    (*save)->port = 0;
    (*save)->locks = 0;
    createPort(&(*save)->port, rect, 0, name);
    if (locked)
        lockSave(*save);
    lockSave(*save);
    copyBits((*save)->port, getPort(), &(*save)->rect);
    unlockSave(*save);
}

/* Puts a saved area back into the current port, then frees it if asked. */
/* @zoombi32 0x00414b35 */
void restoreRect(MapSave **save, short free)
{
    if (*save) {
        lockSave(*save);
        copyBits(getPort(), (*save)->port, &(*save)->rect);
        unlockSave(*save);
        if (free)
            freeSave(save);
    }
}

/* Frees a saved area, unlocking it fully. */
/* @zoombi32 0x00414b76 */
void freeSave(MapSave **save)
{
    freeText((void **)&g_4ab3fc);
    freeText((void **)&g_4ab400);
    if (*save) {
        while ((*save)->locks)
            unlockSave(*save);
        destroyPort(&(*save)->port, 0);
        freeAndClear((void **)save);
    }
}

/* Clips drawing to a rectangle, keeping the old clip region in *region
   (which, with `keep`, must be empty; else it must hold one). */
/* @zoombi32 0x00414bbc */
void clipRect(short *region, ShortRect *rect, short keep)
{
    if (keep) {
        if (*region)
            fatalError("e2ClipRect error -- region must equal 0");
    } else if (!emptyRgn(*region))
        fatalError("e2ClipRect error -- region must be empty");
    getClipRegion(region, keep);
    clipPortToRect(*rect);
}

/* Restores a clip region kept by clipRect, freeing it if asked. */
/* @zoombi32 0x00414c25 */
void fn_414c25(short *region, short free)
{
    if (*region && !emptyRgn(*region)) {
        setClip(*region);
        setEmptyRgn(*region);
        if (free)
            freeRegion(region);
    }
}

/* Gets the clip region into *region, creating the region if asked. */
/* @zoombi32 0x00414c64 */
void getClipRegion(short *region, short create)
{
    if (create) {
        if (*region)
            fatalError("e2GetClipRgn error -- region must equal 0");
        createRegion(region);
    } else if (!emptyRgn(*region))
        fatalError("e2GetClipRgn error -- region must be empty");
    getClip(*region);
}

/* Creates a region into *region (which must be empty). */
/* @zoombi32 0x00414cb2 */
void createRegion(short *region)
{
    if (*region)
        fatalError("e2CreateClipRgn error -- region must equal 0");
    if ((*region = newRgn()) == 0)
        fatalError("unable to allocate region");
}

/* @zoombi32 0x00414ce7 */
void freeRegion(short *region)
{
    if (*region) {
        disposeRgn(*region);
        *region = 0;
    }
}

/* Copies a rectangle of port `from` to the same place in port `to`. */
/* @zoombi32 0x00414d07 */
void copyBits(basePort *to, basePort *from, ShortRect *rect)
{
    copyPortBits(to, from, *rect, *rect, 0);
}

/* Copies a rectangle of the work port to the screen. */
/* @zoombi32 0x00414d53 */
void showRect(ShortRect *rect)
{
    copyPortBits(screenPort, workPort, *rect, *rect, 0);
}

/* @zoombi32 0x00414da5 */
void lockPortOrFail(basePort *port)
{
    if (lockPort(port))
        fatalError(msgUnableToLockPort);
}

/* @zoombi32 0x00414dc4 */
void lockSave(MapSave *save)
{
    lockPortOrFail(save->port);
    if (!++save->locks)
        fatalError("MapSave lock count overflow");
}

/* @zoombi32 0x00414def */
void unlockSave(MapSave *save)
{
    unlockPort(save->port);
    if (!save->locks)
        fatalError("MapSave lock count underflow");
    save->locks--;
}

/* Moves a rectangle to (x, y), placed as drawImage places images. */
/* @zoombi32 0x00414e18 */
void alignRect(ShortRect *rect, short x, short y, short how)
{
    short width, height;

    if (how != 0x11) {
        width = rect->right - rect->left;
        height = rect->bottom - rect->top;
        if (how & 1)
            width = 0;
        else if (how & 2)
            width >>= 1;
        if (how & 0x10)
            height = 0;
        else if (how & 0x20)
            height >>= 1;
        x -= width;
        y -= height;
    }
    offsetRect(rect, x - rect->left, y - rect->top);
}

/* @zoombi32 0x00414e7d */
void fn_414e7d()
{
    initDisplayMode(&displayMode, 0xffff, 0xffff, -1, 0);
    initDisplayMode(&g_4aa7dc, 0xffff, 0xffff, -1, 0);
}

/* Redraws a rectangle of the work port (invertRect) and shows it. */
/* @zoombi32 0x00414eb4 */
void redrawRect(ShortRect *rect)
{
    basePort *saved = getPort();

    setPort(workPort);
    invertRect(*rect);
    showRect(rect);
    setPort(saved);
}

/* Redraws an item, if its flag 4 is set. */
/* @zoombi32 0x00414f01 */
void fn_414f01(InputItem *item)
{
    if (item->flags & 4)
        redrawRect(&item->bounds);
}

/* Redraws an item, unless its flag 4 is set. */
/* @zoombi32 0x00414f17 */
void fn_414f17(InputItem *item)
{
    if (!(item->flags & 4))
        redrawRect(&item->bounds);
}
