/*
 * displaysettings (Mohawk engine): opening and closing the drawing engine,
 * display modes, the system palette, fonts and decompressors
 */

/* @flags -p -x- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "zoombinis.h"
#include "game.h"
#include "graphics.h"
#include "os_localmem.h"
#include "os_manager.h"

/* Windows 95 additions Borland C++ 4.5's headers predate */
#ifndef CDS_TEST
#define CDS_TEST 0x00000002
#define CDS_FULLSCREEN 0x00000004
#endif
#ifndef WM_DISPLAYCHANGE
#define WM_DISPLAYCHANGE 0x007e
#endif

typedef LONG(WINAPI *ChangeDisplaySettingsProc)(DeviceMode *mode, DWORD flags);
typedef BOOL(WINAPI *EnumDisplaySettingsProc)(LPCSTR device, DWORD index, DeviceMode *mode);

GraphicsState graphics;
int sysColorIndices[21] = {
    COLOR_ACTIVEBORDER, COLOR_ACTIVECAPTION, COLOR_APPWORKSPACE, COLOR_BACKGROUND,
    COLOR_BTNFACE, COLOR_BTNHIGHLIGHT, COLOR_BTNSHADOW, COLOR_BTNTEXT, COLOR_CAPTIONTEXT,
    COLOR_GRAYTEXT, COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT, COLOR_INACTIVEBORDER,
    COLOR_INACTIVECAPTION, COLOR_INACTIVECAPTIONTEXT, COLOR_MENU, COLOR_MENUTEXT,
    COLOR_SCROLLBAR, COLOR_WINDOW, COLOR_WINDOWFRAME, COLOR_WINDOWTEXT,
};
COLORREF monoSysColors[21] = {
    0xffffff, 0, 0xffffff, 0, 0xffffff, 0, 0, 0, 0xffffff, 0, 0,
    0xffffff, 0xffffff, 0xffffff, 0, 0xffffff, 0, 0xffffff, 0xffffff, 0, 0,
};
COLORREF savedSysColors[21];

/* ChangeDisplaySettings, where Windows has it (-1 where not). Not exact:
   the original keeps `change` in a saved register (ebx), this in eax. */
/* @zoombi32 0x0048b594 */
long changeDisplaySettings(DeviceMode *mode, unsigned long flags)
{
    ChangeDisplaySettingsProc change;

    flags |= CDS_FULLSCREEN;
    change = (ChangeDisplaySettingsProc)GetProcAddress(GetModuleHandle("USER32"),
                                                       "ChangeDisplaySettingsA");
    if (change)
        return change(mode, flags);
    return -1;
}

/* @zoombi32 0x0048b5cb */
HPALETTE createPalette(PALETTEENTRY *entries)
{
    struct {
        WORD palVersion;
        WORD palNumEntries;
        PALETTEENTRY palPalEntry[256];
    } logical;

    logical.palVersion = 0x300;
    logical.palNumEntries = 256;
    memcpy(logical.palPalEntry, entries, sizeof(logical.palPalEntry));
    return CreatePalette((LOGPALETTE *)&logical);
}

/* EnumDisplaySettings, where Windows has it. Not exact: the original keeps
   `enumerate` in a saved register (ebx), this in eax. */
/* @zoombi32 0x0048b60f */
BOOL enumDisplaySettings(const char *device, unsigned long index, DeviceMode *mode)
{
    EnumDisplaySettingsProc enumerate;

    enumerate = (EnumDisplaySettingsProc)GetProcAddress(GetModuleHandle("USER32"),
                                                        "EnumDisplaySettingsA");
    if (enumerate)
        return enumerate(device, index, mode);
    return 0;
}

/* @zoombi32 0x0048b642 */
BOOL polygon(HDC dc, const Point *points, unsigned short count)
{
    POINT buffer[256];
    POINT *converted;
    int i;
    BOOL result;

    if (count <= 256)
        converted = buffer;
    else if ((converted = (POINT *)malloc(count * sizeof(POINT))) == 0)
        return 0;
    for (i = 0; i < count; i++) {
        converted[i].x = points[i].x;
        converted[i].y = points[i].y;
    }
    result = Polygon(dc, converted, count);
    if (converted != buffer)
        free(converted);
    return result;
}

/*
 * In 8-bit modes, gives the static palette entries to the game
 * (SYSPAL_NOSTATIC, the system colours shown in black and white) or back
 * (SYSPAL_STATIC). The previous setting.
 */
/* @zoombi32 0x0048b6c1 */
UINT setSystemPaletteUse(HDC dc, UINT use)
{
    UINT old;
    int i;
    Palette *palette;

    old = GetSystemPaletteUse(dc);
    if (graphics.depth == 8 && (use != old || use != graphics.systemPaletteUse)) {
        graphics.systemPaletteUse = use;
        SetSystemPaletteUse(dc, use);
        if (use == SYSPAL_STATIC)
            SetSysColors(21, sysColorIndices, savedSysColors);
        else {
            for (i = 0; i < 21; i++)
                savedSysColors[i] = GetSysColor(sysColorIndices[i]);
            SetSysColors(21, sysColorIndices, monoSysColors);
        }
        PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
        if ((palette = graphics.palettes) != 0)
            do {
                palette->realized = 0;
                palette = palette->next;
            } while (palette != graphics.palettes);
        if (!graphics.realizing)
            SendMessage(HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)GetDesktopWindow(), 0);
    } else
        graphics.systemPaletteUse = use;
    return old;
}

/* The activate hook: gives the static colours back while the game is in the
   background. */
/* @zoombi32 0x0048b7a0 */
void graphicsActivate(short active)
{
    HDC dc;

    if (graphics.previousHook)
        graphics.previousHook(active);
    if (active != graphics.active) {
        dc = GetDC(0);
        if (!active && graphics.systemPaletteUse == SYSPAL_NOSTATIC)
            setSystemPaletteUse(dc, SYSPAL_STATIC);
        else if (active && graphics.takeStatic)
            setSystemPaletteUse(dc, SYSPAL_NOSTATIC);
        ReleaseDC(0, dc);
    }
    graphics.active = active;
}

/*
 * Finds the display mode that best suits `want` (the smallest that
 * fits, fewest colours first), else checks whether the current one does.
 * Whether one was found (in `best`).
 */
/* @zoombi32 0x0048b80e */
short findDisplayMode(const DisplayMode *want, DeviceMode *best)
{
    short found;
    unsigned long index;
    DeviceMode mode;

    if (want->width == 0xffff && want->height == 0xffff && want->colors == 0xffffffffUL
        && !want->palettized) {
        currentDisplayMode(best);
        return 1;
    }
    found = 0;
    index = 0;
    memset(&mode, 0, sizeof(mode));
    mode.dmSize = sizeof(mode);
    while (enumDisplaySettings(0, index++, &mode)) {
        if ((want->width == 0xffff || want->width <= mode.dmPelsWidth)
            && (want->height == 0xffff || want->height <= mode.dmPelsHeight)
            && (want->colors == 0xffffffffUL
                || want->colors <= 1UL << (mode.dmBitsPerPel < 24 ? mode.dmBitsPerPel : 24))
            && (!want->palettized || mode.dmBitsPerPel == 8)
            && !changeDisplaySettings(&mode, CDS_TEST)
            && (!found
                || ((want->width == 0xffff || mode.dmPelsWidth <= best->dmPelsWidth)
                    && (want->height == 0xffff || mode.dmPelsHeight <= best->dmPelsHeight)
                    && (!want->palettized || mode.dmBitsPerPel == 0x100)
                    && (want->colors == 0xffffffffUL || mode.dmBitsPerPel <= best->dmBitsPerPel
                        || (want->width != 0xffff && mode.dmPelsWidth < best->dmPelsWidth)
                        || (want->height != 0xffff
                            && mode.dmPelsHeight < best->dmPelsHeight))))) {
            found = 1;
            memcpy(best, &mode, sizeof(mode));
        }
    }
    if (!found) {
        currentDisplayMode(best);
        found = (want->width == 0xffff || want->width <= best->dmPelsWidth)
                && (want->height == 0xffff || want->height <= best->dmPelsHeight)
                && (want->colors == 0xffffffffUL
                    || want->colors
                           <= 1UL << (best->dmBitsPerPel < 24 ? best->dmBitsPerPel : 24))
                && (!want->palettized || best->dmBitsPerPel == 8);
    }
    return found;
}

/* @zoombi32 0x0048b9e7 */
void currentDisplayMode(DeviceMode *mode)
{
    HDC ic;

    memset(mode, 0, sizeof(*mode));
    mode->dmSize = sizeof(*mode);
    mode->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    ic = CreateIC("DISPLAY", 0, 0, 0);
    mode->dmBitsPerPel = GetDeviceCaps(ic, PLANES) * GetDeviceCaps(ic, BITSPIXEL);
    mode->dmPelsWidth = GetDeviceCaps(ic, HORZRES);
    mode->dmPelsHeight = GetDeviceCaps(ic, VERTRES);
    DeleteDC(ic);
}

/*
 * Starts the drawing engine (switching to `mode` if `change`): the system
 * palette, the default palette (the static colours, a 6x6x6 colour cube and
 * 20 greys) and font, the fonts in the program's directory, decompressor
 * DLLs, and a hook on the main window. An error code.
 *
 * A decompressor name longer than 8 characters stops the loop over them
 * from advancing (it never ends), as in the original.
 *
 * Not exact: the original copies `*mode` member by member (BCC32 4.5 copies
 * the block), lays out its frame differently and assigns registers
 * differently.
 */
/* @zoombi32 0x0048ba5a */
short openGraphicsEngine(const DisplayMode *mode, short change)
{
    long red;
    char redByte;
    long green;
    unsigned short length;
    FARPROC proc;
    DisplayMode saved;
    DisplayMode want;
    ColorBytes colors[256];
    char names[256];
    char path[256];
    HMODULE module;
    HDC dc;
    int i;
    unsigned int blue;
    LOGPALETTE *logical;
    HPALETTE hpal;
    HPALETTE old;
    char *name;
    Decompressor *decompressor;
    short error;
    HWND window;

    memset(&graphics, 0, sizeof(graphics));
    initDisplayMode(&saved, 0xffff, 0xffff, -1, 0);
    getDisplayMode(&saved);
    want = *mode;
    if (!canUseDisplayMode(&want, change))
        return setPortError(0x2a36);
    if (change)
        setDisplayMode(&want);
    for (i = 0; i < 21; i++)
        savedSysColors[i] = GetSysColor(sysColorIndices[i]);
    graphics.paletteReserved = 20;
    GetPaletteEntries((HPALETTE)GetStockObject(DEFAULT_PALETTE), 0, 20, graphics.systemColors);
    dc = GetDC(0);
    graphics.depth = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
    graphics.depth = graphics.depth < 24 ? graphics.depth : 24;
    if (graphics.depth == 8) {
        if (!(GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE) || GetDeviceCaps(dc, SIZEPALETTE) != 256
            || GetDeviceCaps(dc, NUMRESERVED) != 20) {
            ReleaseDC(0, dc);
            setDisplayMode(&saved);
            return setPortError(0x2a32);
        }
        setSystemPaletteUse(dc, SYSPAL_STATIC);
        GetSystemPaletteEntries(dc, 0, 10, graphics.systemColors);
        GetSystemPaletteEntries(dc, 246, 10, graphics.systemColors + 10);
        logical = (LOGPALETTE *)localAlloc(sizeof(LOGPALETTE) + 235 * sizeof(PALETTEENTRY));
        if (logical) {
            logical->palVersion = 0x300;
            logical->palNumEntries = 236;
            for (i = 0; i < 236; i++) {
                logical->palPalEntry[i].peRed = 0;
                logical->palPalEntry[i].peGreen = 0;
                logical->palPalEntry[i].peBlue = 0;
                logical->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
            }
            hpal = CreatePalette(logical);
            localFree(logical);
            old = SelectPalette(dc, hpal, TRUE);
            RealizePalette(dc);
            SelectPalette(dc, old, FALSE);
            DeleteObject(hpal);
        } else {
            ReleaseDC(0, dc);
            setDisplayMode(&saved);
            return setPortError(localMemError());
        }
    }
    ReleaseDC(0, dc);

    memset(colors, 0, sizeof(colors));
    i = 10;
    for (red = 1; red <= 6; red++) {
        redByte = (unsigned char)red * 36;
        for (green = 1; green <= 6; green++)
            for (blue = 1; blue <= 6; blue++)
                colors[i++] = RGBColor(redByte, green * 36, (unsigned char)blue * 36, 0).bytes;
    }
    for (blue = 1; blue <= 20; blue++)
        colors[i++] = RGBColor((unsigned char)blue * 12, (unsigned char)blue * 12,
                               (unsigned char)blue * 12, 0).bytes;
    if ((graphics.defaultPalette = newPalette(256, colors)) == 0) {
        error = graphics.error;
        setDisplayMode(&saved);
        graphics.error = error;
        return error;
    }
    if ((graphics.defaultFont = newFont(0, 0, 0)) == 0) {
        error = graphics.error;
        disposePalette(graphics.defaultPalette);
        setDisplayMode(&saved);
        graphics.error = error;
        return error;
    }

    fileSpec current;
    fileSpec directory;
    programDirectory(&directory);
    currentDirectory(&current);
    setCurrentDirectory(&directory);
    forEachFile(addFont, &directory);
    setCurrentDirectory(&current);
    if (graphics.fontFiles)
        SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);

    if (!getIniString(0, "Graphics.Decompressors", 0, names, 256))
        for (name = names; *name;) {
            length = strlen(name);
            if (length <= 8) {
                if (!getIniString(0, "Graphics.Decompressors", name, path, 256)
                    && (UINT_PTR)(module = LoadLibrary(path)) >= 32) {
                    proc = GetProcAddress(module, "GFXXDECPROC");
                    if (proc) {
                        decompressor = (Decompressor *)malloc(sizeof(Decompressor));
                        if (decompressor) {
                            memset(decompressor, 0, sizeof decompressor->next);
                            strcpy(decompressor->name, name);
                            decompressor->module = module;
                            decompressor->proc = proc;
                            decompressor->next = graphics.decompressors;
                            graphics.decompressors = decompressor;
                        } else
                            FreeLibrary(module);
                    } else
                        FreeLibrary(module);
                }
                name += length + 1;
            }
        }

    SetCursor(graphics.cursor = LoadCursor(0, IDC_ARROW));
    graphics.standardCursor = 1;
    getIniBool(0, "Graphics", "fEnableCursorFix", &graphics.cursorFix);
    graphics.previousHook = setActivateHook(graphicsActivate);
    window = appWindowHandle();
    graphics.windowProc = (WNDPROC)GetWindowLong(window, GWL_WNDPROC);
    SetWindowLong(window, GWL_WNDPROC, (LONG_PTR)graphicsWindowProc);
    graphics.ready = 1;
    if (isAppActive())
        graphicsActivate(1);
    return setPortError(0);
}

/*
 * forEachFile's callback for openGraphicsEngine: adds a .FON or .TTF file (in
 * `directory`) as a font resource, a .TTF by way of a .FOT file made for it
 * in the tempDirectory directory.
 *
 * Not exact: the original keeps `extension` in a register (sharing it with
 * `length`) and passes the fileSpec temporaries' addresses from their slots.
 */
/* @zoombi32 0x0048bf9f */
short addFont(const char *name, void *directory)
{
    const char *extension;
    short trueType;
    short created;
    char *end;
    unsigned short length;
    unsigned long i;
    FontFile *font;
    char path[256];
    char fot[256];

    if ((extension = strchr(name, '.')) != 0) {
        fileSpec(*(fileSpec *)directory, name).getPath(path);
        created = 0;
        if ((trueType = !stricmp(extension, ".TTF")) != 0) {
            fileSpec fotDirectory;
            tempDirectory(&fotDirectory);
            fotDirectory.getPath(fot);
            end = fot + strlen(fot);
            if (end[-1] != '\\')
                *end++ = '\\';
            length = extension - name;
            memcpy(end, name, length);
            strcpy(end + length, ".FOT");
            if (!fileMissing(fileSpec(fot)))
                for (i = 0; i <= 0xffff; i++) {
                    sprintf(end, "!GFX%04lX.FOT", i);
                    if (fileMissing(fileSpec(fot)) == 0x2845)
                        break;
                }
            created = CreateScalableFontResource(0, fot, path, 0) != 0;
            if (!created)
                return 0;
            strcpy(path, fot);
        } else if (stricmp(extension, ".FON"))
            return 0;
        if (!AddFontResource(path)) {
            if (created)
                deleteFile(fileSpec(path));
            return 0;
        }
        font = (FontFile *)malloc(offsetof(FontFile, path) + strlen(path) + 1);
        if (font) {
            font->trueType = trueType;
            font->created = created;
            strcpy(font->path, path);
            font->next = graphics.fontFiles;
            graphics.fontFiles = font;
        } else {
            RemoveFontResource(path);
            if (created)
                deleteFile(fileSpec(path));
        }
    }
    return 0;
}

/* The main window's procedure while the engine runs: tells locked ports
   about depth changes, and gives the static colours back when another
   program is about to realize its palette. */
/* @zoombi32 0x0048c25d */
LRESULT CALLBACK graphicsWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    basePort *port;
    unsigned long thread;
    HDC dc;

    switch (message) {
    case WM_DISPLAYCHANGE:
        graphics.depth = wParam;
        for (port = graphics.ports; port; port = port->next)
            if (port->locks > 0)
                port->depthChanged();
        break;
    case WM_PALETTEISCHANGING:
        thread = GetWindowThreadProcessId((HWND)wParam, 0);
        if (graphics.active && thread != appThreadId() && !isMohawkThread(thread)
            && graphics.paletteReserved == 2) {
            dc = GetDC(0);
            setSystemPaletteUse(dc, SYSPAL_STATIC);
            ReleaseDC(0, dc);
        }
    }
    return graphics.windowProc(window, message, wParam, lParam);
}

/* How big a buffer the drawing engine needs (0 before openGraphicsEngine). */
/* @zoombi32 0x0048c300 */
short graphicsBufferSize()
{
    return graphics.ready ? 0x500 : 0;
}

/* Undoes openGraphicsEngine, disposing of every port, palette and font. */
/* @zoombi32 0x0048c314 */
void closeGraphicsEngine()
{
    register GraphicsState *state;
    basePort *port;
    FontFile *font;
    Decompressor *decompressor;
    HDC dc;

    state = &graphics;
    SetWindowLong(appWindowHandle(), GWL_WNDPROC, (LONG_PTR)state->windowProc);
    setActivateHook(state->previousHook);
    state->previousHook = 0;
    graphicsActivate(0);
    setCursorLevel(0);
    setCursorShape(0);
    while ((port = state->ports) != 0) {
        while (port->locks)
            port->unlock();
        port->release();
        delete port;
    }
    while (state->palettes)
        disposePalette(state->palettes);
    while (state->fonts)
        disposeFont(fontHandle(state->fonts));
    if (state->fontFiles) {
        do {
            font = state->fontFiles;
            state->fontFiles = font->next;
            RemoveFontResource(font->path);
            if (font->created) {
                fileSpec file(font->path);
                deleteFile(file);
            }
            free(font);
        } while (state->fontFiles);
        SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    }
    while (state->decompressors) {
        decompressor = state->decompressors;
        state->decompressors = decompressor->next;
        FreeLibrary(decompressor->module);
        free(decompressor);
    }
    dc = GetDC(0);
    setSystemPaletteUse(dc, SYSPAL_STATIC);
    ReleaseDC(0, dc);
    state->ready = 0;
}

/* The port behind a handle, or 0 (error 0x2a73) if it isn't one. */
/* @zoombi32 0x0048c473 */
basePort *checkPort(basePort *port, short kind)
{
    if (!port || port == (basePort *)-1 || port->tag != 0x506f7274L /* 'Port' */) {
        setPortError(0x2a73);
        return 0;
    }
    if (kind & 0xf)
        ;
    setPortError(0);
    return port;
}

/* @zoombi32 0x0048c4ac */
__cdecl RGBColor::RGBColor(unsigned char red, unsigned char green, unsigned char blue,
                           unsigned char kind)
{
    bytes.red = red;
    bytes.green = green;
    bytes.blue = blue;
    bytes.kind = kind;
}
