/*
 * What miniwin's parts share. Sources include the C++ library and SDL
 * first, then this (windows.h's macros, such as `far` or ERROR, would
 * otherwise reach those headers).
 *
 * The model: one program on one screen, as the game expects of Windows 95
 * on a 640x480, 256-colour display.
 *
 * - Everything runs on one host thread. Win32 threads are fibers scheduled
 *   cooperatively (threads.cpp), as on a uniprocessor: a thread runs until
 *   it waits, or wakes a thread of higher priority.
 * - What Windows does behind the program's back (playing sound, multimedia
 *   timers, input arriving, the screen refreshing) happens in service(),
 *   which the functions a program waits or polls in call: the message
 *   functions, Sleep and the waits, and the time functions.
 * - The screen is an 8-bit framebuffer shown through a system palette
 *   (screen.cpp, palette.cpp); GDI draws into it or into bitmaps in software
 *   (gdi.cpp, region.cpp, blit.cpp, draw.cpp, text.cpp).
 */

#ifndef MINIWIN_INTERNAL_H
#define MINIWIN_INTERNAL_H

#include <stdint.h>
#include <string>
#include <vector>

#include "host/host.h"
#include <miniwin/borland.h>
#include <mmsystem.h>
#include <windows.h>

namespace miniwin {

/* Startup (called by the port's main before the game's WinMain). */
void addDrive(char letter, const char *path, const char *label, DWORD serial, bool cdrom);
void setProgramPath(const char *windowsPath);
bool initialize(const char *title);
void shutdown();
HINSTANCE programInstance();

/* A diagnostic on stderr (the host console). */
void trace(const char *format, ...) __attribute__((format(printf, 1, 2)));
/* A Win32 function the game called that miniwin doesn't do (once each). */
void unsupported(const char *what);

/* service(): what happens in the background, at most once a millisecond or
   so. `waiting`: the caller has nothing to do until something happens. */
void service(bool waiting = false);
/* Milliseconds since "Windows started" (timeGetTime's clock). */
DWORD now();

/* Kernel objects: what a HANDLE points at (CloseHandle frees any). */
enum ObjectType
{
    OBJECT_FILE = 1,
    OBJECT_FIND,
    OBJECT_EVENT,
    OBJECT_THREAD
};

struct KernelObject
{
    uint32_t magic;
    int type;
    explicit KernelObject(int type) : magic(0x4d574b4f), type(type) {}
    virtual ~KernelObject() { magic = 0; }
};

KernelObject *kernelObjectOf(HANDLE handle, int type = 0);
bool closeFileObject(KernelObject *object); /* files.cpp */
bool closeThreadObject(KernelObject *object); /* threads.cpp */

/* Files (files.cpp): a Windows path (relative to the current directory, or
   not) to the host's, false if its drive doesn't exist. Each component that
   exists is matched without regard to case, as Windows does. */
bool hostPath(const char *path, std::string &host);
bool isCdrom(char drive);

/* Threads (threads.cpp) */
struct Waitable;
void initThreads();
/* Lets threads of at least `priority` (the current one's, by default) run
   if any can; returns once this thread is running again. */
void yieldThreads(bool all = false);
/* Blocks the current thread until `ready` says so or `timeout` ms pass (0:
   polls once). Other threads run meanwhile; with none, the host waits. */
bool waitFor(bool (*ready)(void *), void *data, DWORD timeout);
bool isMainThread();

/* Windows and messages (user.cpp) */
struct Window;
Window *windowOf(HWND window);
HWND mainWindow();
POINT clientOrigin(HWND window);
RECT clientRect(HWND window);
bool postToQueue(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void noteInput(); /* input arrived: wake GetMessage */
void setActive(bool active);
bool appIsActive();

/* Input state (screen.cpp and user.cpp) */
extern BYTE keyState[256];
extern POINT cursorPosition; /* in screen pixels */

/* The screen (screen.cpp) */
enum
{
    SCREEN_WIDTH = 640,
    SCREEN_HEIGHT = 480
};
bool openScreen(const char *title);
void closeScreen();
void pumpEvents();
void presentIfDue(bool force = false);
void setScreenshotPath(const char *path);
/* A click at screen (x, y), `at` ms after the first one was scripted (for tests). */
void scriptClick(DWORD at, int x, int y, int action = 0);
/* Quits after `ms` milliseconds (for tests). */
void setRunFor(DWORD ms);
void screenChanged();
void setCursorImage(const uint8_t *andMask, const uint8_t *xorMask, int hotX, int hotY);
void setCursorShown(bool shown);
void showMessage(const char *title, const char *text);

/* GDI (gdi.cpp): objects and device contexts */
enum ObjectKind
{
    KIND_BITMAP = 1,
    KIND_BRUSH,
    KIND_PEN,
    KIND_FONT,
    KIND_PALETTE,
    KIND_REGION,
    KIND_DC
};

struct GdiObject
{
    int kind;
    bool stock;
    explicit GdiObject(int kind) : kind(kind), stock(false) {}
    virtual ~GdiObject() {}
};

/* A palette of colours, as a key for colour translation (blit.cpp). */
struct ColorTable
{
    RGBQUAD colors[256];
    int count;
};

struct Bitmap : GdiObject
{
    int width;
    int height;
    int bpp; /* 1, 4, 8, 16, 24 or 32; only 1 and 8 are drawn into */
    long stride; /* bytes from one row (top-down) to the next: negative for bottom-up DIBs */
    uint8_t *top; /* the top row */
    uint8_t *allocation;
    bool device; /* in the display's format (its pixels are system palette indices) */
    ColorTable table; /* a DIB section's */
    int selectedIn; /* DCs holding it */
    Bitmap() : GdiObject(KIND_BITMAP) {}
    ~Bitmap() override;
    uint8_t *row(int y) const { return top + y * stride; }
};

struct Brush : GdiObject
{
    bool null;
    COLORREF color;
    Bitmap *pattern; /* 8x8, or 0 for a solid brush */
    UINT patternUsage;
    WORD patternIndices[256];
    Brush() : GdiObject(KIND_BRUSH), null(false), color(0), pattern(0), patternUsage(0) {}
    ~Brush() override { delete pattern; }
};

struct Pen : GdiObject
{
    int style;
    int width;
    COLORREF color;
    Pen() : GdiObject(KIND_PEN), style(PS_SOLID), width(1), color(0) {}
};

struct FontFace;
struct Font : GdiObject
{
    LOGFONT logical;
    FontFace *face;
    Font() : GdiObject(KIND_FONT), face(0) {}
};

struct Palette : GdiObject
{
    int count;
    PALETTEENTRY entries[256];
    BYTE mapping[256]; /* logical entry to system palette index, once realized */
    bool realized;
    Palette() : GdiObject(KIND_PALETTE), count(0), realized(false) {}
};

/* A region: rectangles in y-x bands (no two overlapping). */
struct Region : GdiObject
{
    std::vector<RECT> rects;
    Region() : GdiObject(KIND_REGION) {}
    RECT bounds() const;
    int complexity() const;
};

struct DC : GdiObject
{
    HWND window; /* a window's DC (the screen), or 0 */
    bool memory;
    bool information; /* CreateIC's: no drawing */
    Bitmap *surface; /* what it draws into */
    Bitmap *defaultBitmap; /* a memory DC's own 1x1 monochrome bitmap */
    Brush *brush;
    Pen *pen;
    Font *font;
    Palette *palette;
    Region *clip; /* device coordinates; 0: none */
    int mapMode;
    POINT windowOrg, viewportOrg;
    SIZE windowExt, viewportExt;
    COLORREF textColor, bkColor;
    int bkMode, rop2, polyFillMode, stretchMode;
    UINT textAlign;
    POINT position; /* the pen's, logical */
    POINT origin; /* device (0, 0) on its surface: a window's client area */
    RECT visible; /* where it may draw, in device coordinates */
    DC();
    ~DC() override;
};

GdiObject *objectOf(HGDIOBJ handle);
DC *dcOf(HDC dc);
Region *regionOf(HRGN rgn);
Bitmap *bitmapOf(HBITMAP bitmap);
Palette *paletteOf(HPALETTE palette);

/* The screen's pixels, the system palette and its version (bumped on every
   change), and the static colours. */
extern Bitmap *screenBitmap;
extern PALETTEENTRY systemPalette[256];
extern unsigned systemPaletteVersion;
extern UINT systemPaletteUse;
void initPalette();
Palette *defaultPalette();

/* Coordinates */
void toDevice(DC *dc, LONG &x, LONG &y);
void toLogical(DC *dc, LONG &x, LONG &y);
RECT toDeviceRect(DC *dc, int left, int top, int right, int bottom); /* normalised */
/* The rectangles a DC may draw in (device coordinates): its surface, less
   what the clip region excludes. */
std::vector<RECT> clipRects(DC *dc);
bool intersect(RECT &a, const RECT &b);

/* Colours (blit.cpp) */
/* The pixel value a COLORREF draws with in this DC. */
uint8_t pixelFor(DC *dc, COLORREF color);
/* The colour of a pixel value in this DC's surface. */
RGBQUAD colorOfPixel(DC *dc, uint8_t pixel);
/* How a source's pixel values map to a destination DC's, for 8-bit and
   smaller sources: `kind` says what the source's colour table holds. */
enum SourceKind
{
    SOURCE_DEVICE, /* system palette indices */
    SOURCE_RGB, /* a colour table */
    SOURCE_PALETTE /* WORD indices into the destination DC's palette */
};
void translation(DC *to, SourceKind kind, const void *table, int count, uint8_t map[256]);
uint8_t nearestIndex(const RGBQUAD *table, int count, int r, int g, int b);

/* Drawing primitives (blit.cpp, draw.cpp) */
uint8_t rop3(DWORD rop, uint8_t pattern, uint8_t source, uint8_t dest);
bool ropUsesSource(DWORD rop);
bool ropUsesPattern(DWORD rop);
/* The brush's pixel at device (x, y). */
uint8_t brushPixel(DC *dc, Brush *brush, int x, int y);
void fillRectDevice(DC *dc, RECT rect, Brush *brush, DWORD rop);
void fillSpans(DC *dc, const std::vector<RECT> &spans, Brush *brush, DWORD rop);

/* Fonts (text.cpp) */
bool addFontFile(const char *path);
void removeFontFile(const char *path);
FontFace *findFace(const char *name);
void loadSystemFonts();

/* Palettes (palette.cpp) */
UINT realize(DC *dc);

/* Sound (mmsystem.cpp) */
bool openAudio();
/* Writes everything played to a WAV file as well (for tests). */
void setRecordPath(const char *path);
void closeAudio();
void serviceAudio();
void serviceTimers();
/* Music (midi.cpp): the SoundFont midiOut plays with (a host path). */
bool loadSoundFont(const char *path);

} /* namespace miniwin */

#endif
