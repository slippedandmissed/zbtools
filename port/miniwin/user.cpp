/*
 * USER32: windows, the message queue, timers, hooks, painting, input state,
 * cursors and the system's metrics and colours.
 *
 * There is one screen and one message queue (one thread has windows). A
 * message comes, as in Windows, from what was posted (input included), else
 * a WM_PAINT for a window with something to repaint, else a WM_TIMER that's
 * due. Every message retrieved passes the WH_GETMESSAGE hooks first.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <deque>
#include <string>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

BYTE keyState[256];
POINT cursorPosition = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

struct WindowClass
{
    std::string name;
    WNDCLASS info;
};

struct Window
{
    uint32_t magic;
    std::string className;
    std::string title;
    WNDPROC proc;
    HINSTANCE instance;
    DWORD style;
    RECT rect; /* on the screen */
    RECT client; /* on the screen */
    bool visible;
    bool iconic;
    bool desktop;
    LONG_PTR userData;
    Region update; /* client coordinates */
    bool erase;
};

static std::vector<WindowClass *> classes;
static std::vector<Window *> windows;
static Window desktop;
static HWND mainHwnd;
static HWND activeHwnd;
static bool appActive;

struct Timer
{
    HWND window;
    UINT_PTR id;
    UINT elapse;
    TIMERPROC proc;
    DWORD due;
};

static std::vector<Timer> timers;
static std::deque<MSG> queue;
static std::vector<HOOKPROC> hooks;
static int quitCode = -1;

static const uint32_t WINDOW_MAGIC = 0x4d575744;

Window *windowOf(HWND window)
{
    Window *w = (Window *)window;

    if (!window)
        return 0;
    if (w == &desktop)
        return &desktop;
    for (Window *candidate : windows)
        if (candidate == w)
            return w->magic == WINDOW_MAGIC ? w : 0;
    return 0;
}

HWND mainWindow()
{
    return mainHwnd;
}

static WindowClass *classOf(LPCSTR name)
{
    for (WindowClass *c : classes)
        if (!stricmp(c->name.c_str(), name))
            return c;
    return 0;
}

ATOM RegisterClass(const WNDCLASS *info)
{
    if (!info->lpszClassName || classOf(info->lpszClassName))
        return 0;
    WindowClass *c = new WindowClass;
    c->name = info->lpszClassName;
    c->info = *info;
    c->info.lpszClassName = c->name.c_str();
    classes.push_back(c);
    return (ATOM)(0xC000 + classes.size());
}

BOOL UnregisterClass(LPCSTR name, HINSTANCE)
{
    for (size_t i = 0; i < classes.size(); i++)
        if (!stricmp(classes[i]->name.c_str(), name)) {
            delete classes[i];
            classes.erase(classes.begin() + i);
            return TRUE;
        }
    return FALSE;
}

static void invalidateAll(Window *w, bool erase)
{
    w->update.rects.clear();
    RECT all = {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
    if (all.right > all.left && all.bottom > all.top)
        w->update.rects.push_back(all);
    w->erase = w->erase || erase;
}

HWND CreateWindowEx(DWORD, LPCSTR className, LPCSTR title, DWORD style, int x, int y, int width,
                    int height, HWND, HMENU, HINSTANCE instance, LPVOID)
{
    WindowClass *c = classOf(className);

    if (!c) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    Window *w = new Window();
    w->magic = WINDOW_MAGIC;
    w->className = c->name;
    w->title = title ? title : "";
    w->proc = c->info.lpfnWndProc;
    w->instance = instance;
    w->style = style;
    if (x == CW_USEDEFAULT) {
        x = y = 0;
        width = SCREEN_WIDTH;
        height = SCREEN_HEIGHT;
    }
    w->rect = {x, y, x + width, y + height};
    w->client = w->rect;
    if (style & WS_BORDER) {
        w->client.left++;
        w->client.top++;
        w->client.right--;
        w->client.bottom--;
    }
    if (w->client.right < w->client.left)
        w->client.right = w->client.left;
    if (w->client.bottom < w->client.top)
        w->client.bottom = w->client.top;
    windows.push_back(w);
    HWND hwnd = (HWND)w;
    if (!mainHwnd && w->client.right - w->client.left >= SCREEN_WIDTH)
        mainHwnd = hwnd;
    if (!SendMessage(hwnd, WM_NCCREATE, 0, 0) || SendMessage(hwnd, WM_CREATE, 0, 0) == -1) {
        DestroyWindow(hwnd);
        return 0;
    }
    if (style & WS_VISIBLE) {
        w->style &= ~WS_VISIBLE;
        ShowWindow(hwnd, SW_SHOW);
    }
    return hwnd;
}

BOOL DestroyWindow(HWND hwnd)
{
    Window *w = windowOf(hwnd);

    if (!w || w == &desktop)
        return FALSE;
    SendMessage(hwnd, WM_DESTROY, 0, 0);
    SendMessage(hwnd, WM_NCDESTROY, 0, 0);
    for (size_t i = 0; i < windows.size(); i++)
        if (windows[i] == w)
            windows.erase(windows.begin() + i);
    for (size_t i = 0; i < timers.size();)
        if (timers[i].window == hwnd)
            timers.erase(timers.begin() + i);
        else
            i++;
    if (mainHwnd == hwnd)
        mainHwnd = 0;
    if (activeHwnd == hwnd)
        activeHwnd = 0;
    w->magic = 0;
    delete w;
    return TRUE;
}

BOOL IsWindow(HWND hwnd)
{
    return windowOf(hwnd) != 0;
}

HWND FindWindow(LPCSTR className, LPCSTR title)
{
    for (Window *w : windows)
        if ((!className || !stricmp(w->className.c_str(), className))
            && (!title || w->title == title))
            return (HWND)w;
    return 0;
}

bool appIsActive()
{
    return appActive;
}

/* Tells every top-level window the application is (in)active, as Windows
   does when the user switches to or away from it. */
void setActive(bool active)
{
    HWND main = mainHwnd;

    /* Until the program has a window, there's nothing to activate. */
    if (active == appActive || !main)
        return;
    appActive = active;
    std::vector<Window *> all = windows;
    if (!active && main)
        SendMessage(main, WM_ACTIVATE, WA_INACTIVE, 0);
    for (Window *w : all)
        if (windowOf((HWND)w))
            SendMessage((HWND)w, WM_ACTIVATEAPP, active, 0);
    if (!main)
        return;
    if (active) {
        activeHwnd = main;
        SendMessage(main, WM_NCACTIVATE, TRUE, 0);
        SendMessage(main, WM_ACTIVATE, WA_ACTIVE, 0);
        SendMessage(main, WM_SETFOCUS, 0, 0);
        InvalidateRect(main, 0, TRUE);
    } else {
        activeHwnd = 0;
        SendMessage(main, WM_NCACTIVATE, FALSE, 0);
        SendMessage(main, WM_KILLFOCUS, 0, 0);
    }
}

BOOL ShowWindow(HWND hwnd, int command)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return FALSE;
    bool wasVisible = w->visible;
    switch (command) {
    case SW_HIDE:
        w->visible = false;
        break;
    case SW_MINIMIZE:
    case SW_SHOWMINIMIZED:
    case SW_SHOWMINNOACTIVE:
        w->visible = true;
        if (!w->iconic) {
            w->iconic = true;
            if (hwnd == mainHwnd)
                setActive(false);
        }
        break;
    default:
        w->visible = true;
        if (w->iconic) {
            w->iconic = false;
            invalidateAll(w, true);
        }
        if (!wasVisible) {
            SendMessage(hwnd, WM_SHOWWINDOW, TRUE, 0);
            invalidateAll(w, true);
        }
        if (hwnd == mainHwnd && command != SW_SHOWNOACTIVATE && command != SW_SHOWNA)
            setActive(true);
    }
    return wasVisible;
}

BOOL UpdateWindow(HWND hwnd)
{
    Window *w = windowOf(hwnd);

    if (w && w->visible && !w->update.rects.empty())
        SendMessage(hwnd, WM_PAINT, 0, 0);
    return w != 0;
}

BOOL SetForegroundWindow(HWND hwnd)
{
    if (hwnd == mainHwnd)
        setActive(true);
    return windowOf(hwnd) != 0;
}

HWND GetActiveWindow()
{
    return activeHwnd;
}

HWND GetDesktopWindow()
{
    desktop.client = desktop.rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    desktop.visible = true;
    desktop.desktop = true;
    return (HWND)&desktop;
}

BOOL IsIconic(HWND hwnd)
{
    Window *w = windowOf(hwnd);
    return w && w->iconic;
}

BOOL GetClientRect(HWND hwnd, LPRECT rect)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return FALSE;
    *rect = {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
    return TRUE;
}

BOOL GetWindowRect(HWND hwnd, LPRECT rect)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return FALSE;
    *rect = w->rect;
    return TRUE;
}

/* The client area's origin on the screen (for its DCs). */
POINT clientOrigin(HWND hwnd)
{
    Window *w = windowOf(hwnd);
    POINT origin = {0, 0};

    if (w) {
        origin.x = w->client.left;
        origin.y = w->client.top;
    }
    return origin;
}

RECT clientRect(HWND hwnd)
{
    RECT rect = {0, 0, 0, 0};
    GetClientRect(hwnd, &rect);
    return rect;
}

LONG_PTR GetWindowLongPtr(HWND hwnd, int index)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return 0;
    switch (index) {
    case GWL_WNDPROC:
        return (LONG_PTR)w->proc;
    case GWL_HINSTANCE:
        return (LONG_PTR)w->instance;
    case GWL_STYLE:
        return (LONG_PTR)(w->style | (w->visible ? WS_VISIBLE : 0) | (w->iconic ? WS_MINIMIZE : 0));
    case GWL_USERDATA:
        return w->userData;
    }
    return 0;
}

LONG_PTR SetWindowLongPtr(HWND hwnd, int index, LONG_PTR value)
{
    Window *w = windowOf(hwnd);
    LONG_PTR previous = GetWindowLongPtr(hwnd, index);

    if (!w)
        return 0;
    switch (index) {
    case GWL_WNDPROC:
        w->proc = (WNDPROC)value;
        break;
    case GWL_STYLE:
        w->style = (DWORD)value;
        break;
    case GWL_USERDATA:
        w->userData = value;
        break;
    }
    return previous;
}

LONG_PTR GetWindowLong(HWND hwnd, int index)
{
    return GetWindowLongPtr(hwnd, index);
}

LONG_PTR SetWindowLong(HWND hwnd, int index, LONG_PTR value)
{
    return SetWindowLongPtr(hwnd, index, value);
}

int GetWindowText(HWND hwnd, LPSTR text, int size)
{
    Window *w = windowOf(hwnd);

    if (!w || size <= 0)
        return 0;
    strncpy(text, w->title.c_str(), size - 1);
    text[size - 1] = 0;
    return (int)strlen(text);
}

DWORD GetWindowThreadProcessId(HWND hwnd, LPDWORD process)
{
    if (process)
        *process = 1;
    return windowOf(hwnd) ? 0x100 : 0; /* the main thread's id (threads.cpp) */
}

BOOL EnumThreadWindows(DWORD, WNDENUMPROC proc, LPARAM data)
{
    std::vector<Window *> all = windows;
    for (Window *w : all)
        if (windowOf((HWND)w) && !proc((HWND)w, data))
            return FALSE;
    return TRUE;
}

LRESULT DefWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window *w = windowOf(hwnd);

    switch (message) {
    case WM_NCCREATE:
        return TRUE;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_ERASEBKGND: {
        WindowClass *c = w ? classOf(w->className.c_str()) : 0;
        RECT rect;
        if (!c || !c->info.hbrBackground)
            return 0;
        GetClientRect(hwnd, &rect);
        FillRect((HDC)wParam, &rect, c->info.hbrBackground);
        return 1;
    }
    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_MINIMIZE:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        case SC_RESTORE:
            ShowWindow(hwnd, SW_RESTORE);
            break;
        case SC_CLOSE:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && w) {
            WindowClass *c = classOf(w->className.c_str());
            if (c && c->info.hCursor)
                SetCursor(c->info.hCursor);
        }
        return 0;
    case WM_NCHITTEST:
        return HTCLIENT;
    case WM_NCACTIVATE:
        return TRUE;
    case WM_QUERYENDSESSION:
        return TRUE;
    }
    (void)lParam;
    return 0;
}

LRESULT CallWindowProc(WNDPROC proc, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return proc ? proc(hwnd, message, wParam, lParam) : 0;
}

LRESULT SendMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hwnd == HWND_BROADCAST) {
        std::vector<Window *> all = windows;
        for (Window *w : all)
            if (windowOf((HWND)w) && w->proc)
                w->proc((HWND)w, message, wParam, lParam);
        return 0;
    }
    Window *w = windowOf(hwnd);
    if (!w || !w->proc)
        return w == &desktop ? DefWindowProc(hwnd, message, wParam, lParam) : 0;
    return w->proc(hwnd, message, wParam, lParam);
}

bool postToQueue(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;

    msg.hwnd = hwnd;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.time = now();
    msg.pt = cursorPosition;
    queue.push_back(msg);
    return true;
}

BOOL PostMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hwnd == HWND_BROADCAST) {
        for (Window *w : windows)
            postToQueue((HWND)w, message, wParam, lParam);
        return TRUE;
    }
    if (hwnd && !windowOf(hwnd))
        return FALSE;
    return postToQueue(hwnd, message, wParam, lParam);
}

void PostQuitMessage(int code)
{
    quitCode = code;
}

UINT_PTR SetTimer(HWND hwnd, UINT_PTR id, UINT elapse, TIMERPROC proc)
{
    /* Windows won't time less than 10 ms. */
    if (elapse < 10)
        elapse = 10;
    for (Timer &timer : timers)
        if (timer.window == hwnd && timer.id == id && hwnd) {
            timer.elapse = elapse;
            timer.proc = proc;
            timer.due = now() + elapse;
            return id;
        }
    static UINT_PTR nextId = 0x7fff0000;
    if (!hwnd)
        id = nextId++;
    timers.push_back({hwnd, id, elapse, proc, now() + elapse});
    return id ? id : 1;
}

BOOL KillTimer(HWND hwnd, UINT_PTR id)
{
    for (size_t i = 0; i < timers.size(); i++)
        if (timers[i].window == hwnd && timers[i].id == id) {
            timers.erase(timers.begin() + i);
            return TRUE;
        }
    return FALSE;
}

HHOOK SetWindowsHookEx(int kind, HOOKPROC proc, HINSTANCE, DWORD)
{
    if (kind != WH_GETMESSAGE) {
        unsupported("SetWindowsHookEx (other than WH_GETMESSAGE)");
        return 0;
    }
    hooks.insert(hooks.begin(), proc);
    return (HHOOK)proc;
}

BOOL UnhookWindowsHookEx(HHOOK hook)
{
    for (size_t i = 0; i < hooks.size(); i++)
        if ((HHOOK)hooks[i] == hook) {
            hooks.erase(hooks.begin() + i);
            return TRUE;
        }
    return FALSE;
}

LRESULT CallNextHookEx(HHOOK hook, int code, WPARAM wParam, LPARAM lParam)
{
    for (size_t i = 0; i + 1 < hooks.size(); i++)
        if ((HHOOK)hooks[i] == hook)
            return hooks[i + 1](code, wParam, lParam);
    return 0;
}

static void callHooks(MSG *message, bool removed)
{
    if (!hooks.empty())
        hooks[0](HC_ACTION, removed ? PM_REMOVE : PM_NOREMOVE, (LPARAM)message);
}

static bool inRange(const MSG &message, HWND hwnd, UINT first, UINT last)
{
    if (hwnd && message.hwnd != hwnd)
        return false;
    return (!first && !last) || (message.message >= first && message.message <= last);
}

static bool rangeHas(UINT message, UINT first, UINT last)
{
    return (!first && !last) || (message >= first && message <= last);
}

/* The next message for the filter, without calling the hooks. */
static bool nextMessage(MSG *message, HWND hwnd, UINT first, UINT last, bool remove)
{
    for (auto i = queue.begin(); i != queue.end(); ++i)
        if (inRange(*i, hwnd, first, last)) {
            *message = *i;
            if (remove)
                queue.erase(i);
            return true;
        }
    if (quitCode >= 0 && rangeHas(WM_QUIT, first, last)) {
        message->hwnd = 0;
        message->message = WM_QUIT;
        message->wParam = (WPARAM)quitCode;
        message->lParam = 0;
        if (remove)
            quitCode = -1;
        return true;
    }
    if (rangeHas(WM_PAINT, first, last))
        for (Window *w : windows)
            if ((!hwnd || (HWND)w == hwnd) && w->visible && !w->iconic && !w->update.rects.empty()) {
                message->hwnd = (HWND)w;
                message->message = WM_PAINT;
                message->wParam = 0;
                message->lParam = 0;
                message->time = now();
                message->pt = cursorPosition;
                return true;
            }
    if (rangeHas(WM_TIMER, first, last)) {
        DWORD time = now();
        for (Timer &timer : timers)
            if ((!hwnd || timer.window == hwnd) && (LONG)(time - timer.due) >= 0) {
                message->hwnd = timer.window;
                message->message = WM_TIMER;
                message->wParam = timer.id;
                message->lParam = (LPARAM)timer.proc;
                message->time = time;
                message->pt = cursorPosition;
                if (remove)
                    timer.due = time + timer.elapse;
                return true;
            }
    }
    return false;
}

BOOL PeekMessage(LPMSG message, HWND hwnd, UINT first, UINT last, UINT flags)
{
    service();
    if (!nextMessage(message, hwnd, first, last, (flags & PM_REMOVE) != 0)) {
        if (!(flags & PM_NOYIELD))
            yieldThreads();
        return FALSE;
    }
    callHooks(message, (flags & PM_REMOVE) != 0);
    return TRUE;
}

struct Filter
{
    HWND hwnd;
    UINT first;
    UINT last;
};

static bool messageWaiting(void *data)
{
    Filter *filter = (Filter *)data;
    MSG message;
    return nextMessage(&message, filter->hwnd, filter->first, filter->last, false);
}

BOOL GetMessage(LPMSG message, HWND hwnd, UINT first, UINT last)
{
    Filter filter = {hwnd, first, last};

    service();
    while (!nextMessage(message, hwnd, first, last, true))
        waitFor(messageWaiting, &filter, INFINITE);
    callHooks(message, true);
    return message->message != WM_QUIT;
}

void noteInput()
{
}

/* Keys that make characters, with Shift or without (a US keyboard). */
static char characterOf(WPARAM key, bool shift)
{
    static const char shifted[] = ")!@#$%^&*(";

    if (key >= 'A' && key <= 'Z')
        return (char)(shift != ((keyState[VK_CAPITAL] & 1) != 0) ? key : tolower((int)key));
    if (key >= '0' && key <= '9')
        return shift ? shifted[key - '0'] : (char)key;
    switch (key) {
    case VK_SPACE:
        return ' ';
    case VK_RETURN:
        return '\r';
    case VK_BACK:
        return '\b';
    case VK_TAB:
        return '\t';
    case VK_ESCAPE:
        return 27;
    case 0xBA:
        return shift ? ':' : ';';
    case 0xBB:
        return shift ? '+' : '=';
    case 0xBC:
        return shift ? '<' : ',';
    case 0xBD:
        return shift ? '_' : '-';
    case 0xBE:
        return shift ? '>' : '.';
    case 0xBF:
        return shift ? '?' : '/';
    case 0xC0:
        return shift ? '~' : '`';
    case 0xDB:
        return shift ? '{' : '[';
    case 0xDC:
        return shift ? '|' : '\\';
    case 0xDD:
        return shift ? '}' : ']';
    case 0xDE:
        return shift ? '"' : '\'';
    }
    return 0;
}

BOOL TranslateMessage(const MSG *message)
{
    if (message->message != WM_KEYDOWN && message->message != WM_SYSKEYDOWN)
        return FALSE;
    bool control = (keyState[VK_CONTROL] & 0x80) != 0;
    char c = characterOf(message->wParam, (keyState[VK_SHIFT] & 0x80) != 0);
    if (control && message->wParam >= 'A' && message->wParam <= 'Z')
        c = (char)(message->wParam - 'A' + 1);
    if (!c)
        return FALSE;
    queue.push_front({message->hwnd, message->message == WM_SYSKEYDOWN ? (UINT)WM_SYSCHAR : (UINT)WM_CHAR,
                      (WPARAM)(unsigned char)c, message->lParam, message->time, message->pt});
    return TRUE;
}

LRESULT DispatchMessage(const MSG *message)
{
    if (message->message == WM_TIMER && message->lParam) {
        ((TIMERPROC)message->lParam)(message->hwnd, WM_TIMER, message->wParam, now());
        return 0;
    }
    if (!message->hwnd)
        return 0;
    return SendMessage(message->hwnd, message->message, message->wParam, message->lParam);
}

/* Painting */

static void addToUpdate(Window *w, const RECT &rect, bool erase)
{
    RECT all = {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
    RECT r = rect;
    if (!intersect(r, all))
        return;
    Region add;
    add.rects.push_back(r);
    CombineRgn((HRGN)&w->update, (HRGN)&w->update, (HRGN)&add, RGN_OR);
    w->erase = w->erase || erase;
}

BOOL InvalidateRect(HWND hwnd, LPCRECT rect, BOOL erase)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return FALSE;
    if (!rect)
        invalidateAll(w, erase != 0);
    else
        addToUpdate(w, *rect, erase != 0);
    return TRUE;
}

BOOL InvalidateRgn(HWND hwnd, HRGN rgn, BOOL erase)
{
    Window *w = windowOf(hwnd);
    Region *region = regionOf(rgn);

    if (!w)
        return FALSE;
    if (!region)
        invalidateAll(w, erase != 0);
    else
        for (const RECT &r : region->rects)
            addToUpdate(w, r, erase != 0);
    return TRUE;
}

BOOL ValidateRect(HWND hwnd, LPCRECT rect)
{
    Window *w = windowOf(hwnd);

    if (!w)
        return FALSE;
    if (!rect) {
        w->update.rects.clear();
        w->erase = false;
        return TRUE;
    }
    Region remove;
    remove.rects.push_back(*rect);
    CombineRgn((HRGN)&w->update, (HRGN)&w->update, (HRGN)&remove, RGN_DIFF);
    return TRUE;
}

BOOL ValidateRgn(HWND hwnd, HRGN rgn)
{
    Window *w = windowOf(hwnd);
    Region *region = regionOf(rgn);

    if (!w)
        return FALSE;
    if (!region)
        return ValidateRect(hwnd, 0);
    CombineRgn((HRGN)&w->update, (HRGN)&w->update, rgn, RGN_DIFF);
    return TRUE;
}

int GetUpdateRgn(HWND hwnd, HRGN rgn, BOOL)
{
    Window *w = windowOf(hwnd);
    Region *region = regionOf(rgn);

    if (!w || !region)
        return ERROR;
    region->rects = w->update.rects;
    return region->complexity();
}

HDC dcForPaint(HWND hwnd, const Region &update);

HDC BeginPaint(HWND hwnd, LPPAINTSTRUCT paint)
{
    Window *w = windowOf(hwnd);

    memset(paint, 0, sizeof *paint);
    if (!w)
        return 0;
    HDC dc = dcForPaint(hwnd, w->update);
    paint->hdc = dc;
    paint->rcPaint = w->update.bounds();
    bool erase = w->erase;
    w->update.rects.clear();
    w->erase = false;
    if (erase)
        paint->fErase = !SendMessage(hwnd, WM_ERASEBKGND, (WPARAM)dc, 0);
    return dc;
}

BOOL EndPaint(HWND hwnd, const PAINTSTRUCT *paint)
{
    ReleaseDC(hwnd, paint->hdc);
    return TRUE;
}

int MessageBox(HWND, LPCSTR text, LPCSTR caption, UINT type)
{
    static const struct
    {
        UINT type;
        int count;
        const char *labels[3];
        int results[3];
    } kinds[] = {
        {MB_OK, 1, {"OK"}, {IDOK}},
        {MB_OKCANCEL, 2, {"OK", "Cancel"}, {IDOK, IDCANCEL}},
        {MB_ABORTRETRYIGNORE, 3, {"Abort", "Retry", "Ignore"}, {IDABORT, IDRETRY, IDIGNORE}},
        {MB_YESNOCANCEL, 3, {"Yes", "No", "Cancel"}, {IDYES, IDNO, IDCANCEL}},
        {MB_YESNO, 2, {"Yes", "No"}, {IDYES, IDNO}},
        {MB_RETRYCANCEL, 2, {"Retry", "Cancel"}, {IDRETRY, IDCANCEL}},
    };
    UINT kind = type & 0xf;
    for (const auto &k : kinds)
        if (k.type == kind) {
            trace("MessageBox \"%s\": %s", caption ? caption : "", text ? text : "");
            int choice = hostMessageBox(caption ? caption : "", text ? text : "", k.labels, k.count);
            return k.results[choice >= 0 && choice < k.count ? choice : k.count - 1];
        }
    return IDOK;
}

/* Input state */

SHORT GetKeyState(int key)
{
    BYTE state = keyState[key & 0xff];
    return (SHORT)((state & 0x80 ? 0x8000 : 0) | (state & 1));
}

SHORT GetAsyncKeyState(int key)
{
    pumpEvents();
    return (SHORT)(keyState[key & 0xff] & 0x80 ? 0x8000 : 0);
}

BOOL GetKeyboardState(BYTE *state)
{
    memcpy(state, keyState, 256);
    return TRUE;
}

BOOL SetKeyboardState(BYTE *state)
{
    memcpy(keyState, state, 256);
    return TRUE;
}

BOOL GetCursorPos(LPPOINT point)
{
    *point = cursorPosition;
    return TRUE;
}

BOOL SetCursorPos(int x, int y)
{
    cursorPosition.x = x < 0 ? 0 : x >= SCREEN_WIDTH ? SCREEN_WIDTH - 1 : x;
    cursorPosition.y = y < 0 ? 0 : y >= SCREEN_HEIGHT ? SCREEN_HEIGHT - 1 : y;
    return TRUE;
}

/* Cursors: 32x32, an AND mask and an XOR mask of a bit each. */

struct Cursor
{
    uint32_t magic;
    int hotX, hotY;
    uint8_t andMask[128];
    uint8_t xorMask[128];
};

static Cursor *currentCursor;
static int cursorCount;

static Cursor *makeCursor(int hotX, int hotY, int width, int height, const void *andPlane,
                          const void *xorPlane)
{
    Cursor *cursor = new Cursor;
    int rowBytes = ((width + 15) / 16) * 2;

    cursor->magic = 0x4d574355;
    cursor->hotX = hotX;
    cursor->hotY = hotY;
    memset(cursor->andMask, 0xff, sizeof cursor->andMask);
    memset(cursor->xorMask, 0, sizeof cursor->xorMask);
    for (int y = 0; y < height && y < 32; y++)
        for (int x = 0; x < rowBytes && x < 4; x++) {
            cursor->andMask[y * 4 + x] = ((const uint8_t *)andPlane)[y * rowBytes + x];
            cursor->xorMask[y * 4 + x] = ((const uint8_t *)xorPlane)[y * rowBytes + x];
        }
    return cursor;
}

HCURSOR CreateCursor(HINSTANCE, int hotX, int hotY, int width, int height, const void *andPlane,
                     const void *xorPlane)
{
    return (HCURSOR)makeCursor(hotX, hotY, width, height, andPlane, xorPlane);
}

/* The standard arrow and hourglass, drawn from text. */
static Cursor *standardCursor(const char *const *rows, int hotX, int hotY)
{
    uint8_t andPlane[128], xorPlane[128];

    memset(andPlane, 0xff, sizeof andPlane);
    memset(xorPlane, 0, sizeof xorPlane);
    for (int y = 0; rows[y]; y++)
        for (int x = 0; rows[y][x]; x++) {
            uint8_t bit = (uint8_t)(0x80 >> (x & 7));
            if (rows[y][x] == 'X')
                andPlane[y * 4 + x / 8] &= (uint8_t)~bit;
            else if (rows[y][x] == '.') {
                andPlane[y * 4 + x / 8] &= (uint8_t)~bit;
                xorPlane[y * 4 + x / 8] |= bit;
            }
        }
    return makeCursor(hotX, hotY, 32, 32, andPlane, xorPlane);
}

HCURSOR LoadCursor(HINSTANCE instance, LPCSTR name)
{
    static const char *const arrow[] = {
        "X", "XX", "X.X", "X..X", "X...X", "X....X", "X.....X", "X......X", "X.......X",
        "X........X", "X.....XXXXX", "X..X..X", "X.X X..X", "XX  X..X", "X    X..X",
        "     X..X", "      X..X", "      XX", 0,
    };
    static Cursor *standard;

    if (instance) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return 0;
    }
    (void)name;
    if (!standard)
        standard = standardCursor(arrow, 0, 0);
    return (HCURSOR)standard;
}

BOOL DestroyCursor(HCURSOR cursor)
{
    Cursor *c = (Cursor *)cursor;

    if (!c || c->magic != 0x4d574355 || c == (Cursor *)LoadCursor(0, IDC_ARROW))
        return FALSE;
    if (currentCursor == c)
        SetCursor(0);
    c->magic = 0;
    delete c;
    return TRUE;
}

HCURSOR SetCursor(HCURSOR cursor)
{
    Cursor *previous = currentCursor;

    currentCursor = (Cursor *)cursor;
    if (currentCursor)
        setCursorImage(currentCursor->andMask, currentCursor->xorMask, currentCursor->hotX,
                       currentCursor->hotY);
    setCursorShown(currentCursor && cursorCount >= 0);
    return (HCURSOR)previous;
}

int ShowCursor(BOOL show)
{
    cursorCount += show ? 1 : -1;
    setCursorShown(currentCursor && cursorCount >= 0);
    return cursorCount;
}

HICON LoadIcon(HINSTANCE, LPCSTR)
{
    static char icon;
    return (HICON)&icon;
}

/* Metrics and colours */

int GetSystemMetrics(int index)
{
    switch (index) {
    case SM_CXSCREEN:
        return SCREEN_WIDTH;
    case SM_CYSCREEN:
        return SCREEN_HEIGHT;
    case SM_CXBORDER:
    case SM_CYBORDER:
        return 1;
    case SM_CXCURSOR:
    case SM_CYCURSOR:
        return 32;
    case SM_MOUSEPRESENT:
        return 1;
    case SM_CMOUSEBUTTONS:
        return 3;
    }
    return 0;
}

/* Windows 95's standard scheme. */
static COLORREF sysColors[21] = {
    RGB(192, 192, 192), RGB(0, 128, 128), RGB(0, 0, 128), RGB(128, 128, 128),
    RGB(192, 192, 192), RGB(255, 255, 255), RGB(0, 0, 0), RGB(0, 0, 0),
    RGB(0, 0, 0), RGB(255, 255, 255), RGB(192, 192, 192), RGB(192, 192, 192),
    RGB(128, 128, 128), RGB(0, 0, 128), RGB(255, 255, 255), RGB(192, 192, 192),
    RGB(128, 128, 128), RGB(128, 128, 128), RGB(0, 0, 0), RGB(192, 192, 192),
    RGB(255, 255, 255),
};

DWORD GetSysColor(int index)
{
    return index >= 0 && index < 21 ? sysColors[index] : 0;
}

BOOL SetSysColors(int count, const int *elements, const COLORREF *colors)
{
    for (int i = 0; i < count; i++)
        if (elements[i] >= 0 && elements[i] < 21)
            sysColors[elements[i]] = colors[i];
    return TRUE;
}

BOOL OffsetRect(LPRECT rect, int dx, int dy)
{
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
    return TRUE;
}

BOOL SetRect(LPRECT rect, int left, int top, int right, int bottom)
{
    *rect = {left, top, right, bottom};
    return TRUE;
}

/* One display mode: 640x480 in 256 colours. */

static void describeMode(DEVMODE *mode)
{
    WORD size = mode->dmSize ? mode->dmSize : (WORD)sizeof(DEVMODE);
    memset(mode, 0, size < sizeof(DEVMODE) ? size : sizeof(DEVMODE));
    mode->dmSize = size;
    mode->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    mode->dmBitsPerPel = 8;
    mode->dmPelsWidth = SCREEN_WIDTH;
    mode->dmPelsHeight = SCREEN_HEIGHT;
    mode->dmDisplayFrequency = 60;
}

LONG ChangeDisplaySettings(DEVMODE *mode, DWORD)
{
    if (!mode)
        return DISP_CHANGE_SUCCESSFUL;
    if ((mode->dmFields & DM_PELSWIDTH && mode->dmPelsWidth != SCREEN_WIDTH)
        || (mode->dmFields & DM_PELSHEIGHT && mode->dmPelsHeight != SCREEN_HEIGHT)
        || (mode->dmFields & DM_BITSPERPEL && mode->dmBitsPerPel != 8))
        return DISP_CHANGE_BADMODE;
    return DISP_CHANGE_SUCCESSFUL;
}

BOOL EnumDisplaySettings(LPCSTR, DWORD index, DEVMODE *mode)
{
    if (index != 0 && index != ENUM_CURRENT_SETTINGS)
        return FALSE;
    describeMode(mode);
    return TRUE;
}

} /* namespace miniwin */
