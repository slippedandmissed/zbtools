/*
 * Stand-in for QuickTime for Windows' SDK glue (0x46cca0-0x46d754 in the
 * original, src/zbtools/quicktime.py), until there's a real one: the game is
 * linked with Apple's glue library, which we don't have, and whose stubs
 * pass a selector in a register (bx), which portable C++ can't.
 *
 * QuickTime reports itself present (the game refuses to start without 2.3
 * or later), but no movie file opens, so loadMovie fails and the game goes
 * on without its movies; everything else does nothing.
 */

#include "../../decomp/zoombinis.h"

enum
{
    quickTimeVersion = 0x2300, /* the least the game accepts */
    fileNotFound = -43 /* the Mac's fnfErr */
};

/* InitializeQTML-like: 0, and the version. */
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

/* OpenMovieFile: no movie file opens. */
long __cdecl qtim_2c(const char *, long *file, long)
{
    *file = 0;
    return fileNotFound;
}

/* GetMoviesError. */
long __cdecl qtim_5e()
{
    return 0;
}

long __cdecl qtim_02(long)
{
    return 0;
}

long __cdecl qtim_07(long)
{
    return 0;
}

/* GetMovieBox: an empty box. */
long __cdecl qtim_0f(long, RECT *box)
{
    box->left = box->top = box->right = box->bottom = 0;
    return 0;
}

long __cdecl qtim_2a(long *movie, long, long *, long, long, long)
{
    *movie = 0;
    return fileNotFound;
}

long __cdecl qtim_2f(long, long, long)
{
    return 0;
}

long __cdecl qtim_31(long, long)
{
    return 0;
}

long __cdecl qtim_37(long)
{
    return 0;
}

long __cdecl qtim_38(long, RECT *, long, HWND)
{
    return 0;
}

long __cdecl qtim_62(long, long)
{
    return 0;
}

long __cdecl cmgr_00(long, HWND, long)
{
    return 0;
}

long __cdecl cmgr_01(long, long, long)
{
    return 0;
}

long __cdecl cmgr_05(long, long *flags)
{
    *flags = 0;
    return 0;
}

long __cdecl cmgr_09(long)
{
    return 0;
}

/* MCIsPlayerEvent: no event is the movie's. */
long __cdecl cmgr_0b(long, HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

long __cdecl cmgr_0d(long, long, HWND, POINT)
{
    return 0;
}

long __cdecl cmgr_0e(long, RECT *, long, long)
{
    return 0;
}
