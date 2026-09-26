/*
 * platform (0x455530-0x456c00): the Windows layer: the window class and procedure
 * (0x45605e), the message loop (PeekMessageA, GetMessageA), keyboard and cursor
 */

#include <windows.h>
#include <stdlib.h>
#include <dir.h>
#include <dos.h>
#include <stdlib.h>
#include "zoombinis.h"

/* Whether a mouse is installed. */
/* @zoombi32 0x00455903 */
int isMousePresent()
{
    return GetSystemMetrics(SM_MOUSEPRESENT);
}

/* Frees *block if it's allocated, and clears it. */
/* @zoombi32 0x00455c43 */
void freeAndClear(void **block)
{
    if (*block) {
        free(*block);
        *block = 0;
    }
}

/* Writes value in decimal into buffer. */
/* @zoombi32 0x00455c8b */
char *intToDecimal(int value, char *buffer)
{
    return itoa(value, buffer, 10);
}

/* Writes value in decimal into buffer. */
/* @zoombi32 0x00455ca2 */
char *unsignedToDecimal(unsigned long value, char *buffer)
{
    return ultoa(value, buffer, 10);
}

/* @zoombi32 0x00455e26 */
void fn_455e26(long)
{
}

/* @zoombi32 0x00455e2d */
void fn_455e2d(long)
{
}

/* @zoombi32 0x00455e85 */
long fn_455e85(long, long)
{
    return 0;
}

/* @zoombi32 0x00456a2f */
void fn_456a2f(Callback callback)
{
    g_4a4a14 = callback;
}

/* @zoombi32 0x00456a3e */
void fn_456a3e(long first, long second)
{
    g_4a4a18 = first;
    g_4a4a1c = second;
}

/* @zoombi32 0x00456a55 */
void fn_456a55(long value)
{
    g_4a4a00 = value;
}

/* @zoombi32 0x00456bf6 */
short fn_456bf6()
{
    return g_4b2d38;
}

/* Adds the Shift and Ctrl keys' state to `modifiers`. */
/* @zoombi32 0x00455990 */
short addModifierKeys(short modifiers)
{
    if (GetKeyState(VK_SHIFT) < 0)
        modifiers |= 0x800;
    if (GetKeyState(VK_CONTROL) < 0)
        modifiers |= 0x400;
    return modifiers;
}

/* Where the cursor is, in the coordinates of the port g_4aa7a4. */
/* @zoombi32 0x004559c0 */
void getCursorPosition(Point *where)
{
    POINT cursor;
    Point point;

    GetCursorPos(&cursor);
    point.x = cursor.x;
    point.y = cursor.y;
    long saved = getPort();
    setPort(g_4aa7a4);
    globalToLocal(&point);
    setPort(saved);
    *where = point;
}

/* QuickDraw's SetPt. */
inline void setPoint(Point *point, short x, short y)
{
    point->x = x;
    point->y = y;
}

/*
 * Moves the cursor to a point in the coordinates of the port g_4aa7a4.
 * Not exact: the original loads y before x (into ax and dx) when setting the
 * point; direct stores, an initialiser and this inline helper don't.
 */
/* @zoombi32-nonmatching 0x00455a10 */
void setCursorPosition(short x, short y)
{
    Point point;

    setPoint(&point, x, y);
    long saved = getPort();
    setPort(g_4aa7a4);
    localToGlobal(&point);
    setPort(saved);
    SetCursorPos(point.x, point.y);
}

/*
 * Whether mouse button 1-3 is still held down: pressed, with no button-up
 * message waiting. Not exact: the original tests `ah` for 0x80, as it would
 * with GetAsyncKeyState returning int (as in the 16-bit Windows headers);
 * with the Win32 declaration's SHORT, every form tried is a sign test.
 */
/* @zoombi32-nonmatching 0x00455a5b */
short isButtonStillDown(unsigned short button)
{
    MSG message;

    if (!button)
        return 0;
    int key = buttonKeys[button - 1];
    UINT up = buttonUpMessages[button - 1];
    if (GetAsyncKeyState(key) >= 0)
        return 0;
    return !PeekMessage(&message, 0, up, up, PM_NOYIELD);
}

/* Allocates `size` bytes into *block; whether it could. */
/* @zoombi32 0x00455c18 */
short allocateBlock(void **block, unsigned long size)
{
    if (!(*block = malloc(size)))
        allocationFailed = 1;
    return *block != 0;
}

/* The time of day. */
/* @zoombi32 0x00455c60 */
void getClockTime(char *hour, char *minute, char *second)
{
    struct time now;

    gettime(&now);
    *hour = now.ti_hour;
    *minute = now.ti_min;
    *second = now.ti_sec;
}

/* Changes back to the drive and directory saved in savedDisk and
   savedDirectory, if any. */
/* @zoombi32 0x00455d9a */
void restoreDirectory()
{
    if (savedDisk >= 0) {
        chdir(savedDirectory);
        setdisk(savedDisk);
        savedDisk = -1;
    }
}

/* @zoombi32 0x004568d8 */
short fn_4568d8()
{
    if (!g_4b2d38 && g_4aafe8) {
        if (fn_48cab4(g_4aafe8, 1))
            InvalidateRect(mainWindow, 0, 0);
        return 1;
    }
    return 0;
}

/*
 * Brightens `count` palette entries from `first`: each component c becomes
 * c + 31 - c/8 (black stays black). Presumably adjusting colours made for the
 * Mac's lighter display gamma.
 */
/* @zoombi32 0x00455dc5 */
void brightenPalette(PALETTEENTRY *entries, short first, short count)
{
    for (short i = 0; i < count; i++) {
        unsigned char *color = (unsigned char *)&entries[first + i];
        for (short c = 0; c < 3; c++)
            if (color[c])
                color[c] = color[c] + 31 - color[c] / 8;
    }
}

/* Whether input is waiting (without taking it): keys if `which` has bit 0,
   mouse clicks if it has bit 1. */
/* @zoombi32 0x00455b1b */
short isInputWaiting(long which)
{
    MSG message;

    if (which & 1) {
        if (PeekMessage(&message, 0, WM_KEYDOWN, WM_KEYDOWN, PM_NOYIELD)
            || PeekMessage(&message, 0, WM_CHAR, WM_SYSKEYDOWN, PM_NOYIELD)
            || PeekMessage(&message, 0, WM_SYSCHAR, WM_SYSDEADCHAR, PM_NOYIELD))
            return 1;
    }
    if (which & 2) {
        if (PeekMessage(&message, 0, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOYIELD)
            || PeekMessage(&message, 0, WM_RBUTTONDOWN, WM_RBUTTONDOWN, PM_NOYIELD)
            || PeekMessage(&message, 0, WM_MBUTTONDOWN, WM_MBUTTONDOWN, PM_NOYIELD))
            return 1;
    }
    return 0;
}

/* Appends a record to a queue of up to 1024 (in five parallel arrays; the
   window procedure fills it). */
/* @zoombi32 0x004565c8 */
void fn_4565c8(long a, long b, long c, short d, long e)
{
    if (g_4b2d42 != 0x400) {
        g_4b2d44[g_4b2d42] = a;
        g_4b3d44[g_4b2d42] = b;
        g_4b4d44[g_4b2d42] = c;
        g_4b6d44[g_4b2d42] = d;
        g_4b5d44[g_4b2d42] = e;
        g_4b2d42++;
    }
}
