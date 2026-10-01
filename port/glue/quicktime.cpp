/*
 * The port's QuickTime: plays the game's intro movie itself, behind the
 * calls the game makes of QuickTime for Windows' glue (glue/quicktime.cpp is
 * the real one, which forwards them to QuickTime). The movie's video is
 * Broderbund's QkBk codec, which only its own QuickTime component can decode,
 * so the port plays the movie's scene file (src/zbtools/formats/scene.py,
 * which `uv run port package` writes beside where the .MOV would be, as .SCN):
 * each frame is sprites in a palette on a background colour, and drawing one
 * is what the codec does, so there's no decoder. The sound is one track
 * from time 0, played through waveOut; the video follows the clock.
 *
 * The calls are named by selector (see src/zbtools/quicktime.py). What the
 * game asks of them: open the file and make a movie of it (qtim_2c, qtim_2a,
 * qtim_02), a controller for it over a window (qtim_38; cmgr_0d to reuse one),
 * place it (cmgr_0e), play (cmgr_01 with action 8), let it run (cmgr_09 on
 * each pass of the game's loop), ask whether it still plays (cmgr_05, flag
 * 0x40), and stop and dispose of it (qtim_31, qtim_07, qtim_37, qtim_0c).
 */

#include "../../decomp/zoombinis.h"

#include <stdlib.h>
#include <string.h>

enum
{
    quickTimeVersion = 0x2300, /* the least the game accepts */
    fileNotFound = -43, /* the Mac's fnfErr */
    badMovie = -2048, /* invalidMovie */
    actionPlay = 8, /* MCDoAction's mcActionPlay */
    controllerPlaying = 0x40, /* in what cmgr_05 gives */
    castPalette = 1,
    castBitmap = 2,
    paletteColours = 256
};

struct MovieCast
{
    unsigned char kind;
    unsigned short width, height;
    const unsigned char *data; /* a palette's colours, or a bitmap's rows */
};

/* A loaded scene file. */
struct MovieScene
{
    unsigned char *file;
    int width, height;
    int frameMilliseconds;
    int frameCount;
    int soundRate;
    int soundSamples;
    const unsigned char *sound;
    const unsigned char **frames; /* each frame's record: fill, sprites, palette, sprites */
    MovieCast *casts; /* by ID */
    int castLimit;
};

struct MoviePlayer
{
    MovieScene *scene;
    HWND window;
    int x, y; /* where the movie's top left is in the window */
    bool playing;
    bool started;
    DWORD startTime;
    int drawn; /* the frame on the screen, or -1 */
    int palette; /* the palette in force, or -1 */
    HWAVEOUT wave;
    WAVEHDR header;
    bool headerPrepared;
    unsigned char *pixels;
};

static long lastError = 0;
static MoviePlayer *player = 0;

static unsigned read16(const unsigned char *at)
{
    return at[0] | (at[1] << 8);
}

static unsigned long read32(const unsigned char *at)
{
    return read16(at) | ((unsigned long)read16(at + 2) << 16);
}

static void freeScene(MovieScene *scene)
{
    if (scene) {
        free(scene->file);
        free(scene->frames);
        free(scene->casts);
        free(scene);
    }
}

/* Loads the scene file that goes with a .MOV path. */
static MovieScene *loadScene(const char *path)
{
    char name[300];
    size_t length = strlen(path);
    HANDLE file;
    DWORD size, got;
    MovieScene *scene;
    const unsigned char *at;
    int i;

    if (length < 4 || length >= sizeof name)
        return 0;
    strcpy(name, path);
    strcpy(name + length - 4, ".SCN");
    file = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    size = GetFileSize(file, 0);
    scene = (MovieScene *)calloc(1, sizeof *scene);
    scene->file = (unsigned char *)malloc(size);
    if (!ReadFile(file, scene->file, size, &got, 0) || got != size || size < 36
        || memcmp(scene->file, "ZBSC", 4) != 0 || read32(scene->file + 4) != 1) {
        CloseHandle(file);
        freeScene(scene);
        return 0;
    }
    CloseHandle(file);
    at = scene->file + 8;
    scene->width = read16(at);
    scene->height = read16(at + 2);
    scene->frameMilliseconds = (int)read32(at + 4);
    scene->frameCount = (int)read32(at + 8);
    scene->soundRate = (int)read32(at + 12);
    scene->soundSamples = (int)read32(at + 16);
    int castCount = (int)read32(at + 20);
    at += 24;
    scene->frames = (const unsigned char **)malloc(sizeof(unsigned char *) * scene->frameCount);
    for (i = 0; i < scene->frameCount; i++) {
        scene->frames[i] = at;
        at += 4 + 6 * at[1];
    }
    scene->castLimit = 0;
    const unsigned char *casts = at;
    for (i = 0; i < castCount; i++) {
        int id = read16(at);
        if (id >= scene->castLimit)
            scene->castLimit = id + 1;
        at += 4 + (at[2] == castPalette ? paletteColours * 3 : 8 + read32(at + 8));
    }
    scene->casts = (MovieCast *)calloc(scene->castLimit, sizeof(MovieCast));
    at = casts;
    for (i = 0; i < castCount; i++) {
        MovieCast &cast = scene->casts[read16(at)];
        cast.kind = at[2];
        if (cast.kind == castPalette) {
            cast.data = at + 4;
            at += 4 + paletteColours * 3;
        } else {
            cast.width = (unsigned short)read16(at + 4);
            cast.height = (unsigned short)read16(at + 6);
            cast.data = at + 12;
            at += 12 + read32(at + 8);
        }
    }
    scene->sound = at;
    return scene;
}

/* Draws a bitmap's rows, clipped, over the pixels: a run of colour 0 (and
   any pixel of it) leaves what's there. */
static void drawBitmap(unsigned char *pixels, int width, int height, const MovieCast &cast, int x, int y)
{
    const unsigned char *row = cast.data;
    int line;

    for (line = 0; line < cast.height; line++) {
        unsigned length = read16(row);
        const unsigned char *at = row + 2;
        const unsigned char *end = at + length;
        int column = 0;
        int py = y + line;
        unsigned char *out = pixels + (long)py * width;

        while (column < cast.width && at < end) {
            unsigned control = *at++;
            int count = (control & 0x7f) + 1;
            if (control & 0x80) {
                unsigned char colour = *at++;
                if (colour && py >= 0 && py < height)
                    for (int i = 0; i < count; i++) {
                        int px = x + column + i;
                        if (px >= 0 && px < width)
                            out[px] = colour;
                    }
            } else {
                if (py >= 0 && py < height)
                    for (int i = 0; i < count; i++) {
                        int px = x + column + i;
                        if (px >= 0 && px < width)
                            out[px] = at[i];
                    }
                at += count;
            }
            column += count;
        }
        row += 2 + length;
    }
}

static void drawFrame(MoviePlayer *p, int frame)
{
    MovieScene *scene = p->scene;
    const unsigned char *record = scene->frames[frame];
    int count = record[1];
    int i;
    int paletteId = (int)read16(record + 2);

    memset(p->pixels, record[0], (size_t)scene->width * scene->height);
    for (i = 0; i < count; i++) {
        const unsigned char *sprite = record + 4 + 6 * i;
        unsigned id = read16(sprite);
        if (id < (unsigned)scene->castLimit && scene->casts[id].kind == castBitmap)
            drawBitmap(p->pixels, scene->width, scene->height, scene->casts[id],
                       (short)read16(sprite + 2), (short)read16(sprite + 4));
    }

    HDC dc = GetDC(p->window);
    /* The codec realizes the palette when the frame's palette changes, as
       many colours as the system leaves free: with its 20 static colours, 236
       (the palette's 10 to 245, at the same indices); without, all but the
       first, which is black (its 1 to 255). A pixel then shows as the system
       palette's colour of that value. */
    if (paletteId != p->palette && paletteId < scene->castLimit
        && scene->casts[paletteId].kind == castPalette) {
        struct
        {
            WORD version;
            WORD count;
            PALETTEENTRY entries[paletteColours];
        } logical;
        const unsigned char *colours = scene->casts[paletteId].data;
        bool keepStatic = GetSystemPaletteUse(dc) == SYSPAL_STATIC;
        int first = keepStatic ? 10 : 1;
        int entries = keepStatic ? paletteColours - 20 : paletteColours - 1;
        logical.version = 0x300;
        logical.count = paletteColours;
        memset(logical.entries, 0, sizeof logical.entries);
        for (i = 0; i < entries; i++) {
            logical.entries[i].peRed = colours[(first + i) * 3];
            logical.entries[i].peGreen = colours[(first + i) * 3 + 1];
            logical.entries[i].peBlue = colours[(first + i) * 3 + 2];
            logical.entries[i].peFlags = PC_RESERVED;
        }
        HPALETTE palette = CreatePalette((LOGPALETTE *)&logical);
        HPALETTE before = SelectPalette(dc, palette, 0);
        RealizePalette(dc);
        SelectPalette(dc, before, 0);
        DeleteObject(palette);
        p->palette = paletteId;
    }
    /* The pixels are the system palette's values: a DIB whose colour table
       is the identity (DIB_PAL_COLORS), through a logical palette that's
       never been realized, which maps each entry to the same value. */
    struct
    {
        BITMAPINFOHEADER header;
        WORD indices[256];
    } info;
    WORD *table = info.indices;

    memset(&info, 0, sizeof info);
    info.header.biSize = sizeof info.header;
    info.header.biWidth = scene->width;
    info.header.biHeight = -scene->height; /* top down */
    info.header.biPlanes = 1;
    info.header.biBitCount = 8;
    info.header.biClrUsed = 256;
    for (i = 0; i < 256; i++)
        table[i] = (WORD)i;
    /* (a LOGPALETTE holds one entry; the rest follow it) */
    struct
    {
        WORD version;
        WORD count;
        PALETTEENTRY entries[256];
    } identityPalette;
    memset(&identityPalette, 0, sizeof identityPalette);
    identityPalette.version = 0x300;
    identityPalette.count = 256;
    HPALETTE identityHandle = CreatePalette((LOGPALETTE *)&identityPalette);
    HPALETTE before = SelectPalette(dc, identityHandle, 0);
    StretchDIBits(dc, p->x, p->y, scene->width, scene->height, 0, 0, scene->width, scene->height,
                  p->pixels, (BITMAPINFO *)&info, DIB_PAL_COLORS, SRCCOPY);
    SelectPalette(dc, before, 0);
    DeleteObject(identityHandle);
    ReleaseDC(p->window, dc);
    p->drawn = frame;
}

static void stopSound(MoviePlayer *p)
{
    if (p->wave) {
        waveOutReset(p->wave);
        if (p->headerPrepared)
            waveOutUnprepareHeader(p->wave, &p->header, sizeof p->header);
        p->headerPrepared = false;
        waveOutClose(p->wave);
        p->wave = 0;
    }
}

static void startSound(MoviePlayer *p)
{
    PCMWAVEFORMAT format;
    MovieScene *scene = p->scene;

    if (!scene->soundSamples)
        return;
    memset(&format, 0, sizeof format);
    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    format.wf.nChannels = 1;
    format.wf.nSamplesPerSec = scene->soundRate;
    format.wf.nAvgBytesPerSec = scene->soundRate;
    format.wf.nBlockAlign = 1;
    format.wBitsPerSample = 8;
    if (waveOutOpen(&p->wave, WAVE_MAPPER, (WAVEFORMAT *)&format, 0, 0, CALLBACK_NULL)
        != MMSYSERR_NOERROR) {
        p->wave = 0;
        return;
    }
    memset(&p->header, 0, sizeof p->header);
    p->header.lpData = (LPSTR)scene->sound;
    p->header.dwBufferLength = scene->soundSamples;
    waveOutPrepareHeader(p->wave, &p->header, sizeof p->header);
    p->headerPrepared = true;
    waveOutWrite(p->wave, &p->header, sizeof p->header);
}

static void stopPlaying(MoviePlayer *p)
{
    p->playing = false;
    stopSound(p);
}

static void attach(MoviePlayer *p, MovieScene *scene)
{
    stopPlaying(p);
    free(p->pixels);
    p->pixels = 0;
    p->scene = scene;
    p->started = false;
    p->drawn = -1;
    p->palette = -1;
    if (scene)
        p->pixels = (unsigned char *)malloc((size_t)scene->width * scene->height);
}

long __cdecl QTInitialize(long *version)
{
    *version = quickTimeVersion;
    return 0;
}

void __cdecl QTTerminate()
{
}

/* EnterMovies. */
long qtim_0b()
{
    return 0;
}

/* ExitMovies. */
long __cdecl qtim_0c()
{
    return 0;
}

/* OpenMovieFile: the movie's scene file. */
long __cdecl qtim_2c(const char *path, LONG_PTR *file, long)
{
    MovieScene *scene = loadScene(path);
    *file = (LONG_PTR)scene;
    lastError = scene ? 0 : fileNotFound;
    return lastError;
}

/* GetMoviesError. */
long __cdecl qtim_5e()
{
    long error = lastError;
    lastError = 0;
    return error;
}

/* CloseMovieFile: the movie made from it keeps the scene. */
long __cdecl qtim_02(LONG_PTR)
{
    return 0;
}

/* DisposeMovie. */
long __cdecl qtim_07(LONG_PTR movie)
{
    if (player && player->scene == (MovieScene *)movie)
        attach(player, 0);
    freeScene((MovieScene *)movie);
    return 0;
}

/* GetMovieBox: from 0, 0. */
long __cdecl qtim_0f(LONG_PTR movie, RECT *box)
{
    MovieScene *scene = (MovieScene *)movie;
    box->left = box->top = 0;
    box->right = scene ? scene->width : 0;
    box->bottom = scene ? scene->height : 0;
    return 0;
}

/* NewMovieFromFile: the movie is the scene. */
long __cdecl qtim_2a(LONG_PTR *movie, LONG_PTR file, long *, long, long, long)
{
    *movie = file;
    lastError = file ? 0 : badMovie;
    return lastError;
}

long __cdecl qtim_2f(LONG_PTR, long, long)
{
    return 0;
}

/* SetMovieActive: inactive stops it. */
long __cdecl qtim_31(LONG_PTR movie, long active)
{
    if (!active && player && player->scene == (MovieScene *)movie)
        stopPlaying(player);
    return 0;
}

/* DisposeMovieController. */
long __cdecl qtim_37(LONG_PTR controller)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    if (p) {
        attach(p, 0);
        if (p == player)
            player = 0;
        free(p);
    }
    return 0;
}

/* NewMovieController: drawn in the window at the bounds. */
LONG_PTR __cdecl qtim_38(LONG_PTR movie, RECT *bounds, long, HWND window)
{
    MoviePlayer *p = (MoviePlayer *)calloc(1, sizeof *p);
    p->window = window;
    p->x = bounds->left;
    p->y = bounds->top;
    attach(p, (MovieScene *)movie);
    player = p;
    return (LONG_PTR)p;
}

/* MoviesTask. */
long __cdecl qtim_62(long, long)
{
    return 0;
}

long __cdecl cmgr_00(LONG_PTR controller, HWND window, long)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    if (p)
        p->window = window;
    return 0;
}

/* MCDoAction: play starts the movie (and its sound) from the start. */
long __cdecl cmgr_01(LONG_PTR controller, long action, long parameters)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    if (p && p->scene && action == actionPlay && parameters && !p->started) {
        p->started = true;
        p->playing = true;
        p->startTime = GetTickCount();
        startSound(p);
    }
    return 0;
}

/* MCGetControllerInfo: whether it's playing. */
long __cdecl cmgr_05(LONG_PTR controller, long *flags)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    *flags = p && p->playing ? controllerPlaying : 0;
    return 0;
}

/* MCIdle: draws the frame that's due, and ends the movie after its last. */
long __cdecl cmgr_09(LONG_PTR controller)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    int frame;

    if (!p || !p->scene || !p->scene->frameCount)
        return 0;
    frame = 0;
    if (p->playing) {
        DWORD elapsed = GetTickCount() - p->startTime;
        frame = (int)(elapsed / p->scene->frameMilliseconds);
        if (frame >= p->scene->frameCount) {
            frame = p->scene->frameCount - 1;
            if (p->drawn != frame)
                drawFrame(p, frame);
            stopPlaying(p);
            return 0;
        }
    } else if (p->started) {
        return 0;
    }
    if (frame != p->drawn)
        drawFrame(p, frame);
    return 0;
}

/* MCIsPlayerEvent: no event is the movie's. */
long __cdecl cmgr_0b(LONG_PTR, HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

/* MCSetMovie: a new movie, drawn at `where`. */
long __cdecl cmgr_0d(LONG_PTR controller, LONG_PTR movie, HWND window, POINT where)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    if (p) {
        p->window = window;
        p->x = where.x;
        p->y = where.y;
        attach(p, (MovieScene *)movie);
    }
    return 0;
}

/* MCSetControllerBoundsRect. */
long __cdecl cmgr_0e(LONG_PTR controller, RECT *bounds, long, long)
{
    MoviePlayer *p = (MoviePlayer *)controller;
    if (p) {
        p->x = bounds->left;
        p->y = bounds->top;
    }
    return 0;
}
