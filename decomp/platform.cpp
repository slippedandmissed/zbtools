/*
 * platform (0x455530-0x456c00): the Windows layer: the window class and procedure
 * (0x45605e), the message loop (PeekMessageA, GetMessageA), keyboard and cursor
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <dos.h>
#include <stdlib.h>
#include "zoombinis.h"

/*
 * Checks the display mode asked for, and switches to it: the smallest of
 * 640x480 to 1280x1024 that fits it, at 256 colours (or 16/24-bit), falling
 * back to 512x384. If that fails, the message says what the game needs.
 */
/* @zoombi32 0x00455530 */
void checkDisplayMode(DisplayMode *mode)
{
    char message[0x100];
    const char *minimum = emptyString;
    const char *depth;
    unsigned short width, height;
    short i, found;

    if (!mode->palettized) {
        if (mode->colors <= 0x10000)
            minimum = minimumOfText;
    } else if (mode->colors > 0x100)
        fatalError("Invalid display mode.");
    if (mode->colors <= 0x100) {
        depth = colors256Text;
        bitsPerPixel = 8;
        mode->colors = 0x100;
    } else if (mode->colors <= 0x10000) {
        depth = color16Text;
        bitsPerPixel = 16;
    } else {
        depth = color24Text;
        bitsPerPixel = 24;
    }
    for (i = 0, found = 0; i < 4 && !found; i++) {
        if (resolutionWidths[i] >= mode->width && resolutionHeights[i] >= mode->height) {
            found = 1;
            mode->width = resolutionWidths[i];
            mode->height = resolutionHeights[i];
            if (g_4aa7cc) {
                width = 512;
                height = 384;
            } else {
                width = resolutionWidths[i];
                height = resolutionHeights[i];
            }
        }
    }
    fn_4150c7(0x100, message, svgaRequiredFormat, minimum, depth, width, height);
    if (!canUseDisplayMode(mode, 1)) {
        mode->width = 512;
        mode->height = 384;
        if (!g_4aa7cc || !canUseDisplayMode(mode, 1))
            fatalError(message);
    }
}

/*
 * Creates the main window: a borderless popup covering the screen (its thin
 * border just off it), of a class named after the program, shown maximised.
 * Whether the screen's port exists afterwards.
 */
/* @zoombi32 0x004556aa */
short createMainWindow(short, short)
{
    int screenWidth, screenHeight, borderWidth, borderHeight;

    if (screenPort)
        return 0;
    if (!appPreviousInstance) {
        windowClass.style = 0;
        windowClass.lpfnWndProc = mainWindowProc;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = appInstance;
        windowClass.hIcon = LoadIcon(appInstance, "AppIcon");
        windowClass.hCursor = 0;
        windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        windowClass.lpszMenuName = 0;
        windowClass.lpszClassName = programPath;
        if (!RegisterClass(&windowClass))
            return 0;
        classRegistered = 1;
    }
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    borderWidth = GetSystemMetrics(SM_CXBORDER);
    borderHeight = GetSystemMetrics(SM_CYBORDER);
    if (!(mainWindow = CreateWindowEx(0, programPath, appName, WS_POPUP | WS_BORDER | WS_SYSMENU,
                                      -borderWidth, -borderHeight, screenWidth + borderWidth * 2,
                                      screenHeight + borderHeight * 2, 0, 0, appInstance, 0)))
        return 0;
    if (appShowCommand == SW_SHOWNORMAL || appShowCommand == SW_SHOWDEFAULT)
        appShowCommand = SW_SHOWMAXIMIZED;
    windowed = appShowCommand != SW_SHOWMAXIMIZED;
    ShowWindow(mainWindow, appShowCommand);
    UpdateWindow(mainWindow);
    fn_456914();
    return screenPort != 0;
}

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
short fn_455e85(Point *, short)
{
    return 0;
}

/*
 * Handles the next waiting message, if any: 0 if there was none, 2 if it was
 * a mouse button going down, 1 if a key going down, else 0 or whatever an
 * earlier test found.
 */
/* @zoombi32 0x0045590b */
short handleNextMessage()
{
    MSG message;
    short kind;

    dispatchingEvents = 1;
    kind = 0;
    if (!pumpMessage(&message, 0, 0, 0)) {
        dispatchingEvents = 0;
        return 0;
    }
    if (message.message >= WM_KEYFIRST && message.message <= WM_KEYLAST
        && message.message != WM_KEYUP && message.message != WM_SYSKEYUP)
        kind = 1;
    switch (message.message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        kind = 2;
    }
    dispatchingEvents = 0;
    return kind;
}

/* Throws away waiting input: keys (which & 1; system keys are handled
   first) and mouse buttons (which & 2). */
/* @zoombi32 0x00455ab0 */
void flushInput(short which)
{
    MSG message;

    if (which & 1) {
        while (pumpMessage(&message, WM_SYSKEYDOWN, WM_KEYLAST, PM_NOYIELD))
            ;
        while (PeekMessage(&message, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE | PM_NOYIELD))
            ;
    }
    if (which & 2)
        while (PeekMessage(&message, 0, WM_LBUTTONDOWN, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD))
            ;
}

/* A mouse button went down at `where` (a message's lParam, in the window):
   posts it, in the game port's coordinates, unless input is ignored. */
/* @zoombi32 0x00455ef9 */
void mouseButtonDown(short button, long keys, long where)
{
    POINT position;
    Point point;
    basePort *saved;

    position.x = (short)LOWORD(where);
    position.y = (short)HIWORD(where);
    point.x = position.x;
    point.y = position.y;
    saved = getPort();
    setPort(screenPort);
    globalToLocal(&point);
    setPort(saved);
    if (!inputIgnored)
        postMouseEvent(&point, button);
}

/* Handles every waiting message, dropping keys and clicks. */
/* @zoombi32 0x00455f66 */
void handleMessagesIgnoringInput()
{
    MSG message;

    inputIgnored = 1;
    while (pumpMessage(&message, 0, 0, 0))
        ;
    inputIgnored = 0;
}

/* Destroys the main window (the app is deactivated first) and unregisters
   its class. */
/* @zoombi32 0x0045581b */
void destroyMainWindow()
{
    destroyPort(&screenPort, 1);
    if (mainWindow) {
        g_4b2d32 = 1;
        activateApp(0);
        DestroyWindow(mainWindow);
        mainWindow = 0;
    }
    if (!appPreviousInstance && classRegistered) {
        UnregisterClass(programPath, appInstance);
        classRegistered = 0;
    }
}

/* Shows an error: `prefix`, then `format` filled in from `args`. When
   debugging, it's also printed and the debugger stops. */
/* @zoombi32 0x0045587f */
void showError(const char *prefix, const char *format, va_list args)
{
    char *text;
    HGLOBAL block;

    if ((block = GlobalAlloc(GMEM_FIXED, 0x400)) != 0) {
        text = (char *)GlobalLock(block);
        sprintf(text, prefix);
        vsprintf(text + strlen(text), format, args);
        if (debugging) {
            fn_46db93(text);
            debugBreak(0);
        }
        MessageBox(mainWindow, text, appName, MB_SYSTEMMODAL | MB_ICONEXCLAMATION);
        GlobalUnlock(block);
        GlobalFree(block);
    }
}

/* Marks Ctrl and Alt as up in the keyboard state, then handles the waiting
   messages, ignoring input. */
/* @zoombi32 0x00455e4d */
void releaseControlKeys()
{
    char unused[12]; /* the original's frame has 12 unused bytes above keys */
    BYTE keys[256];

    GetKeyboardState(keys);
    keys[VK_CONTROL] &= 0x7f;
    keys[VK_MENU] &= 0x7f;
    SetKeyboardState(keys);
    handleMessagesIgnoringInput();
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
void fn_456a55(void (*callback)(short active))
{
    g_4a4a00 = callback;
}

/* @zoombi32 0x00456bf6 */
short fn_456bf6()
{
    return windowed;
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

/* Where the cursor is, in the coordinates of the port screenPort. */
/* @zoombi32 0x004559c0 */
void getCursorPosition(Point *where)
{
    POINT cursor;
    Point point;

    GetCursorPos(&cursor);
    point.x = cursor.x;
    point.y = cursor.y;
    basePort *saved = getPort();
    setPort(screenPort);
    globalToLocal(&point);
    setPort(saved);
    *where = point;
}

/* Sets a point, vertical coordinate first (the original evaluates y before x). */
inline void setPoint(Point *point, short y, short x)
{
    point->x = x;
    point->y = y;
}

/* Moves the cursor to a point in the coordinates of the port screenPort. */
/* @zoombi32 0x00455a10 */
void setCursorPosition(short x, short y)
{
    Point point;

    setPoint(&point, y, x);
    basePort *saved = getPort();
    setPort(screenPort);
    localToGlobal(&point);
    setPort(saved);
    SetCursorPos(point.x, point.y);
}

/*
 * Whether mouse button 1-3 is still held down: pressed, with no button-up
 * message waiting. Not exact: the original tests `ah` for 0x80, as it would
 * with GetAsyncKeyState returning int (as in the 16-bit Windows headers);
 * with the Win32 declaration's SHORT, `>= 0` is a sign test and `& 0x8000`
 * (with any cast tried) sign-extends first.
 */
/* @zoombi32 0x00455a5b */
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

/*
 * Changes to the program's own drive and directory, saving the current ones
 * (restoreDirectory goes back), and names the program after its file if it
 * has no name yet.
 */
/* @zoombi32 0x00455cb9 */
void enterProgramDirectory()
{
    char directory[0x100];
    char *first;
    char *last;

    g_4b2b00 = 0;
    GetModuleFileName(appInstance, programPath, 0x100);
    strcpy(directory, programPath);
    if (!getcwd(savedDirectory, 0x100))
        fatalError("path too long: limit %d characters", 0x100);
    savedDisk = getdisk();
    if (appName == emptyString)
        appName = programPath;
    setdisk((programPath[0] & ~0x20) - 'A');
    first = strchr(directory, '\\');
    last = strrchr(directory, '\\');
    if (first == last)
        last[1] = 0;
    else
        *last = 0;
    chdir(directory);
    fn_455f66();
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
    if (!windowed && palette) {
        if (fn_48cab4(palette, 1))
            InvalidateRect(mainWindow, 0, 0);
        return 1;
    }
    return 0;
}

/* Handles a message, if one is waiting. */
/* @zoombi32 0x00455e34 */
void handleWaitingMessage()
{
    MSG message;

    pumpMessage(&message, 0, 0, 0);
}

/* While the game is paused (and not stopping), keep handling messages. */
/* @zoombi32 0x00455e8e */
void waitWhilePaused()
{
    MSG message;

    if (g_4b2d34 && !g_4b2d3c && !g_4b2d32) {
        inputIgnored = g_4b2d3c = 1;
        while (g_4b2d34 && !g_4b2d32)
            pumpMessage(&message, 0, 0, 0);
        inputIgnored = g_4b2d3c = 0;
    }
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
short isInputWaiting(short which)
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
/*
 * The main window's procedure. QuickTime's component manager sees each
 * message first when g_4b2ad4 is set. Messages other than timer, mouse and
 * cursor ones are logged before and after (logMessage), and clear g_4b2b00
 * unless they're keys. Keys and clicks become game events; closing the
 * window, or the session ending, is a fatal error (it quits); the window is
 * repainted by g_4a07ec (or showRect) and blacked out around it.
 */
/* @zoombi32 0x0045605e */
LRESULT CALLBACK mainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    short i;
    long command;
    LRESULT result;
    PAINTSTRUCT paint;
    RECT rect;
    basePort *port;
    HDC dc;
    char key;
    UINT hitTest;
    BOOL active;

    if (g_4b2ad4 && cmgr_0b(g_4b2adc, window, message, wParam, lParam))
        return 0;
    if (message != WM_TIMER && (message < WM_KEYFIRST || message > WM_KEYLAST)
        && (message < WM_MOUSEFIRST || message > WM_MOUSELAST) && message != WM_NCHITTEST
        && message != WM_SETCURSOR)
        g_4b2b00 = 0;
    if (message != WM_TIMER && (message < WM_MOUSEFIRST || message > WM_MOUSELAST)
        && message != WM_NCHITTEST && message != WM_SETCURSOR)
        logMessage(message, wParam, lParam, 0, 0);
    switch (message) {
    case WM_SETCURSOR:
        hitTest = LOWORD(lParam);
        if (hitTest == HTCLIENT && g_4a07e8) {
            g_4a07e8();
            return 1;
        }
        break;
    case WM_CHAR:
    case WM_DEADCHAR:
        key = wParam;
        /* (Shift+Tab was presumably meant to become something else.) */
        switch (key) {
        case '\t':
            if (GetKeyState(VK_SHIFT) < 0)
                key = '\t';
        }
        if (!inputIgnored)
            postKeyEvent(key);
        return 0;
    case WM_KEYDOWN:
        if (!inputIgnored)
            postKeyEvent(addModifierKeys(wParam + 0xff));
        return 0;
    case WM_LBUTTONDOWN:
        mouseButtonDown(1, wParam, lParam);
        return 0;
    case WM_RBUTTONDOWN:
        mouseButtonDown(2, wParam, lParam);
        return 0;
    case WM_MBUTTONDOWN:
        mouseButtonDown(3, wParam, lParam);
        return 0;
    case WM_NCACTIVATE:
        active = wParam;
        if (g_4a4ad6 && !active)
            activateApp(active);
        g_4a4ad6 = 0;
        break;
    case WM_ACTIVATEAPP:
        activateApp(wParam);
        break;
    case WM_SETFOCUS:
        windowed = 0;
        setTakeStatic(g_4aa7ce);
        fn_4568d8();
        activateApp(1);
        g_4b2d3a = 0;
        if (g_4b2d40) {
            if (g_4b2d3e) {
                if (g_4b2d3e > 0)
                    for (i = 0; i < g_4b2d3e; i++)
                        showCursor();
                else
                    for (i = 0; i < -g_4b2d3e; i++)
                        hideCursor();
            }
            g_4b2d40 = 0;
        }
        break;
    case WM_KILLFOCUS:
        setTakeStatic(0);
        if (!g_4b2d40) {
            g_4b2d3e = setCursorLevel(isMousePresent() - 1);
            g_4b2d3e -= isMousePresent() - 1;
            g_4b2d40 = 1;
        }
        break;
    case WM_ACTIVATE:
        if (!g_4b2d34) {
            active = LOWORD(wParam);
            if (g_4a4a00)
                g_4a4a00(active != WA_INACTIVE);
        }
        break;
    case WM_SYSCOMMAND:
        command = wParam & 0xfff0;
        if (command == SC_SCREENSAVE) {
            if (g_4b2b02)
                return 1;
            g_4b2d3a = 1;
        }
        if (command == SC_TASKLIST) {
            g_4b2d3e = setCursorLevel(isMousePresent() - 1);
            g_4b2d3e -= isMousePresent() - 1;
            g_4b2d40 = 1;
        }
        if (command != SC_CLOSE)
            break;
        /* fall through */
    case WM_DESTROY:
    case WM_CLOSE:
    case WM_QUIT:
    case WM_ENDSESSION:
        if (!g_4b2d32) {
            g_4b2d32 = 1;
            fatalError(g_4a07b4);
        }
        return 0;
    case WM_PALETTECHANGED:
        if ((HWND)wParam != mainWindow)
            fn_4568d8();
        break;
    case WM_QUERYNEWPALETTE:
        return 1;
    case WM_PAINT:
        if (windowed)
            return 0;
        port = getPort();
        if (screenPort && workPort) {
            setPort(screenPort);
            fn_4887f4();
            if (g_4a07ec)
                g_4a07ec();
            else
                showRect(&gameRect);
            fn_48b1b4();
        }
        setPort(port);
        dc = BeginPaint(window, &paint);
        GetClientRect(window, &rect);
        FillRect(dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(window, &paint);
        return 0;
    }
    result = DefWindowProc(window, message, wParam, lParam);
    if (message != WM_TIMER && (message < WM_KEYFIRST || message > WM_KEYLAST)
        && (message < WM_MOUSEFIRST || message > WM_MOUSELAST) && message != WM_NCHITTEST
        && message != WM_SETCURSOR)
        g_4b2b00 = 0;
    if (message != WM_TIMER && (message < WM_MOUSEFIRST || message > WM_MOUSELAST)
        && message != WM_NCHITTEST && message != WM_SETCURSOR)
        logMessage(message, wParam, lParam, 1, result);
    return result;
}


/* Appends a record to a queue of up to 1024 (in five parallel arrays; the
   window procedure fills it). */
/* @zoombi32 0x004565c8 */
void logMessage(long message, long wParam, long lParam, short after, long result)
{
    if (g_4b2d42 != 0x400) {
        g_4b2d44[g_4b2d42] = message;
        g_4b3d44[g_4b2d42] = wParam;
        g_4b4d44[g_4b2d42] = lParam;
        g_4b6d44[g_4b2d42] = after;
        g_4b5d44[g_4b2d42] = result;
        g_4b2d42++;
    }
}

/*
 * Takes a waiting message in the range first-last (all, if both are 0) and
 * handles it; whether there was one. Paint messages are taken with GetMessage
 * too, and system keys are also handled as plain keys.
 */
/* @zoombi32 0x00455f96 */
short pumpMessage(MSG *message, unsigned short first, unsigned short last, unsigned short flags)
{
    waitWhilePaused();
    if (!PeekMessage(message, 0, first, last, flags | PM_REMOVE))
        return 0;
    UINT kind = message->message;
    if (kind == WM_PAINT)
        GetMessage(message, 0, WM_PAINT, WM_PAINT);
    if (kind >= WM_SYSKEYDOWN && kind <= WM_KEYLAST)
        handleSystemKey(message);
    handleMessage(message);
    return 1;
}

/* Handles a system key message (Alt held) as the plain key message. */
/* @zoombi32 0x00455fff */
void handleSystemKey(MSG *message)
{
    short saved = dispatchingEvents;
    MSG key = *message;

    key.message -= WM_SYSKEYDOWN - WM_KEYDOWN;
    /* Clears the Alt context bit (29) in lParam's high word; the original works on
       that word, as here (which assumes little-endian, like every current target). */
    ((WORD *)&key.lParam)[1] &= ~0x2000;
    handleMessage(&key);
    dispatchingEvents = saved;
}

/* @zoombi32 0x00456041 */
void handleMessage(MSG *message)
{
    TranslateMessage(message);
    DispatchMessage(message);
    waitWhilePaused();
}

/*
 * Writes the queued messages (logMessage) to the first unused msgNNN.txt, then
 * empties the queue.
 *
 * Not exact yet, but only because its literals are addressed from the
 * module's literal pool, which in the original starts with 0x41 bytes of
 * literals from earlier functions ("Invalid display mode.", ...) that aren't
 * decompiled yet; it should match once they are.
 */
/* @zoombi32 0x00456638 */
void dumpMessages()
{
    FILE *file;
    short i = -1;

    do {
        if (++i >= 1000)
            return;
        sprintf(messageLogName, "%s%03d%s", "msg", i, ".txt");
        file = fopen(messageLogName, "r");
        if (file)
            fclose(file);
    } while (file);
    file = fopen(messageLogName, "wt");
    if (!file)
        return;
    for (i = 0; i < g_4b2d42; i++) {
        if (g_4b6d44[i])
            fprintf(file, "%3d %4x %8lx %8lx %08lx\n", i, g_4b2d44[i], g_4b3d44[i], g_4b4d44[i],
                    g_4b5d44[i]);
        else
            fprintf(file, "%3d %4x %8lx %8lx\n", i, g_4b2d44[i], g_4b3d44[i], g_4b4d44[i]);
    }
    fclose(file);
    g_4b2d42 = 0;
}

/*
 * The app is activated or deactivated (unless the window is minimised).
 * Activating sets up the screen port and the sound driver (asking to retry
 * while it's missing); deactivating shuts them down and, in some setups,
 * minimises the window.
 */
/* @zoombi32 0x00456747 */
void activateApp(long active)
{
    if (!windowed && active != appActive) {
        appActive = active;
        if (active) {
            if (!g_4b2d3a && g_4b9d22 >= 0x395 && !g_4b2b04) {
                fn_48b2d8(&g_4aa7dc);
                fn_48d480(&displayMode);
            }
            placeGamePort();
            fn_46da64(1);
            while (fn_4764bc(1))
                if (MessageBox(mainWindow, "Sound driver missing or unavailable.", appName,
                               MB_RETRYCANCEL)
                    == IDCANCEL)
                    fatalError(g_4a07b4);
            runClock(1);
            g_4b2d34 = 0;
            g_4b2d30 = 1;
            g_4a4a10 = 1;
            if (g_4a4a10)
                fn_456b2e(1);
        } else {
            g_4a4a10 = 0;
            if (!g_4a4a10)
                fn_456b2e(0);
            g_4b2d34 = 1;
            runClock(0);
            fn_4764bc(0);
            fn_46da64(0);
            if (!g_4b2d3a && g_4b9d22 >= 0x395 && !g_4b2b04)
                fn_48d480(&g_4aa7dc);
            if (!g_4b2d32 && g_4b9d22 >= 0x395 && g_4a4a0c && !g_4b2d3a) {
                windowed = 1;
                SendMessage(mainWindow, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            }
        }
    }
}

/*
 * Measures the screen, centres the game's area on it (alignRect) and moves
 * it to match, then creates the screen port there if there isn't one yet.
 */
/* @zoombi32 0x00456914 */
void placeGamePort()
{
    ShortRect centred;
    ShortRect *screen;
    basePort *saved;

    screen = &screenRect;
    screen->left = screen->top = 0;
    screen->right = GetSystemMetrics(SM_CXSCREEN);
    screen->bottom = GetSystemMetrics(SM_CYSCREEN);
    centred = g_4aa7b8 = gameRect;
    alignRect(&centred, (screen->right + screen->left) >> 1, (screen->bottom + screen->top) >> 1,
              0x22);
    offsetRect(screen, -centred.left, -centred.top);
    sectRect(&g_4aa7b8, screen);
    if (!screenPort) {
        screenPort = newWindowPort(centred, mainWindow, 0);
        if (screenPort)
            lockPortOrFail(screenPort);
        else
            fatalError(msgNoScreenPort);
        if (palette) {
            saved = getPort();
            setPort(screenPort);
            setPortPalette(palette);
            setPort(saved);
        }
    }
}

/* Draws the palette as a chart of 8-pixel squares, 32 to a row, keeping the
   current colour. */
/* @zoombi32 0x00456a64 */
void fn_456a64()
{
    ShortRect cell;
    ShortRect saved;
    short i;

    saved = g_4a4ae6;
    Color color;
    color = getForeColor();
    for (i = 0; i <= 0xff; i++) {
        cell.left = (i & 0x1f) << 3;
        cell.top = ((i & 0xe0) >> 5) << 3;
        cell.right = cell.left + 8;
        cell.bottom = cell.top + 8;
        fillPortRect(cell, Color(i), 0);
    }
    setForeColor(color);
    showRect(&saved);
}

/* With a screen port: activating clears the game's area (and fills it via
   fillPortRect) when g_4b2ad4 and g_4b2ad8 are set; deactivating calls
   fn_455273 then, and clears the area. */
/* @zoombi32 0x00456b2e */
void fn_456b2e(short active)
{
    if (screenPort) {
        if (active) {
            if (g_4b2ad4 && g_4b2ad8) {
                setClipRect(gameRect);
                fillPortRect(gameRect, Color(0), 0);
            }
        } else if (g_4b2ad4 && g_4b2ad8) {
            g_4b7cf8 = 1;
            fn_455273(1);
        }
        if (!active) {
            setClipRect(gameRect);
        }
    }
}
