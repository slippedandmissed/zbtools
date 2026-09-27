/*
 * os_manager (0x46da64-0x46e2a4): the Mohawk OS layer's manager: starting
 * and stopping it (its 'MOHAWK OS Manager' window), activation, and timers
 * run from the application thread's messages
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include "zoombinis.h"

OsState os;
SystemState systemState;

static char managerName[] = "MOHAWK OS Manager";

/* The tag of a live timer: 'ksTI' in memory order. */
#define TIMER_TAG 0x4954736bL
/* The WM_TIMER that makes sure timers run while nothing else happens. */
#define TIMER_ID 0x7469

/* The application was activated or deactivated: tells the hook. */
/* @zoombi32 0x0046da64 */
short osSetActive(short active)
{
    active = active != 0;
    if (active != os.active) {
        os.active = active;
        if (os.activateHook)
            os.activateHook(active);
    }
    return setOsError(0);
}

/*
 * Atomic operations on a shared counter, as the threading classes need. The
 * originals are hand-written (`lock inc`, `lock dec`, `xchg`), presumably
 * because Windows 95's InterlockedIncrement/Decrement only return the sign of
 * the result; on the Windows versions that run the game today they return the
 * value, so these use them (functional, not byte-exact: see CLAUDE.md).
 */
/* Subtracts one from *value and returns the result. */
/* @zoombi32-functional 0x0046da9d */
long atomicDecrement(long *value)
{
    return InterlockedDecrement(value);
}

/* Stores value in *target and returns what was there. */
/* @zoombi32-functional 0x0046daac */
void *atomicExchange(void **target, void *value)
{
    return (void *)InterlockedExchange((long *)target, (long)value);
}

/* Adds one to *value and returns the result. */
/* @zoombi32-functional 0x0046dabb */
long atomicIncrement(long *value)
{
    return InterlockedIncrement(value);
}

/* A timer calling `proc` every `interval` ms (-1: at every chance), with
   `data`; 0 on failure. */
/* @zoombi32 0x0046daca */
long newTimer(void (*proc)(long timer, long data), long data, long interval)
{
    OsTimer *timer;

    if ((timer = (OsTimer *)localAlloc(sizeof(OsTimer))) == 0) {
        setOsError(localMemError());
        return 0;
    }
    memset(timer, 0, sizeof(OsTimer));
    timer->tag = TIMER_TAG;
    timer->proc = proc;
    timer->data = data;
    timer->interval = interval;
    if (!os.timerId) {
        os.timerId = SetTimer(os.window, TIMER_ID, 1, 0);
        if (!os.timerId) {
            localFree(timer);
            setOsError(10000);
            return 0;
        }
    }
    if ((timer->next = os.timers) != 0)
        timer->next->prev = timer;
    os.timers = timer;
    setTimerInterval(timerHandle(timer), interval);
    return timerHandle(timer);
}

/*
 * Stops in the debugger, then returns `value`. The original has an `int3`
 * (probably inline assembly); DebugBreak does the same (functional).
 */
/* @zoombi32-functional 0x0046db83 */
short debugBreak(short value)
{
    DebugBreak();
    return value;
}

/* Writes to the debugger's output, printf-style. */
/* @zoombi32 0x0046db93 */
void __cdecl debugPrintf(const char *format, ...)
{
    char text[256];
    va_list args;

    va_start(args, format);
    vsprintf(text, format, args);
    OutputDebugString(text);
}

/* @zoombi32 0x0046dbc2 */
void deleteTimer(long handle)
{
    OsTimer *timer;

    if ((timer = timerOf(handle)) == 0) {
        setOsError(0x2743);
        return;
    }
    if (timer->next)
        timer->next->prev = timer->prev;
    if (timer->prev)
        timer->prev->next = timer->next;
    else
        os.timers = timer->next;
    if (!os.timers) {
        KillTimer(os.window, TIMER_ID);
        os.timerId = 0;
        os.nextTimer = 0;
    }
    timer->tag = 0;
    localFree(timer);
    setOsError(0);
}

/* Takes every listed lock (see DeferLock). */
/* @zoombi32 0x0046dc45 */
void enterAllLocks()
{
    DeferLock *object = locks;
    while (object) {
        enterLock(object);
        object = object->next;
    }
}

/* Runs the timers that are due (or, with `all`, every one), unless they're
   running already. */
/* @zoombi32 0x0046dc64 */
void runTimers(unsigned long now, short all)
{
    OsTimer *timer;
    OsTimer *next;

    if (!os.runningTimers++) {
        os.nextTimer = 0;
        for (timer = os.timers; timer; timer = next) {
            next = timer->next;
            if (all || timer->due <= now) {
                timer->proc(timerHandle(timer), timer->data);
                if (timer->tag == TIMER_TAG && timer->interval) {
                    timer->due = timer->interval == -1 ? now : timer->interval + now;
                    if (!os.nextTimer || os.nextTimer > timer->due)
                        os.nextTimer = timer->due;
                } else {
                    timer->due = 0;
                }
            }
        }
    }
    os.runningTimers--;
}

/* Releases every listed lock (see DeferLock). */
/* @zoombi32 0x0046dd02 */
void leaveAllLocks()
{
    for (DeferLock *lock = locks; lock; lock = lock->next)
        leaveLock(lock);
}

/* @zoombi32 0x0046dd21 */
HINSTANCE engineInstanceHandle()
{
    return os.instance;
}

/* @zoombi32 0x0046dd27 */
unsigned long appThreadId()
{
    return os.thread;
}

/* @zoombi32 0x0046dd2d */
HWND appWindowHandle()
{
    return os.window;
}

/* Watches the application thread's messages (WH_GETMESSAGE): runs every
   timer on our WM_TIMER, the ones due on any other message. */
/* @zoombi32 0x0046dd40 */
LRESULT CALLBACK timerHook(int code, WPARAM wParam, LPARAM lParam)
{
    unsigned long now;
    MSG *message = (MSG *)lParam;

    switch (message->message) {
    case WM_TIMER:
        if (message->wParam == TIMER_ID) {
            runTimers(currentTimeMs(), 1);
            break;
        }
    default:
        if (os.nextTimer > 0 && (now = currentTimeMs()) >= os.nextTimer)
            runTimers(now, 0);
    }
    return CallNextHookEx(os.hook, code, wParam, lParam);
}

/* The time in milliseconds since Windows started. */
/* @zoombi32 0x0046dda5 */
DWORD currentTimeMs()
{
    return timeGetTime();
}

/*
 * Starts the OS layer: checks for Windows 95 or later (not Win32s), notes
 * the processor, makes its window and message hook, and starts the threads
 * (their stacks carved from `stacks`). Not exact: the original's `sub ax, 1`
 * has TASM's encoding (the module holds inline assembly, so it was compiled
 * through TASM32).
 */
/* @zoombi32 0x0046ddaf */
short osStartup(HINSTANCE instance, void *stacks, long size)
{
    DWORD version;
    unsigned short low;
    WNDCLASS windowClass;
    short error;

    memset(&systemState, 0, sizeof(systemState));
    version = GetVersion();
    low = lowWord(version);
    systemState.windowsVersion = bcdVersion(byteSwapShort(low));
    switch (highWord(version)) {
    case 0:
        systemState.windowsNT = 1;
        break;
    case 1:
        return setOsError(0x2744);
    }
    GetSystemInfo(&systemState.info);
    switch (systemState.info.dwProcessorType) {
    case PROCESSOR_INTEL_386:
        systemState.processor = 3;
        break;
    case PROCESSOR_INTEL_486:
        systemState.processor = 4;
        break;
    case PROCESSOR_INTEL_PENTIUM:
        systemState.processor = 5;
        break;
    case 686:
        systemState.processor = 6;
        break;
    case 786:
        systemState.processor = 7;
        break;
    default:
        systemState.processor = 0;
        break;
    }
    if (systemState.windowsVersion < 0x310) {
        setOsError(0x2744);
        return os.error;
    }
    memset(&os, 0, sizeof(os));
    os.instance = instance;
    os.thread = GetCurrentThreadId();
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.lpszClassName = managerName;
    windowClass.lpfnWndProc = managerWindowProc;
    windowClass.hInstance = instance;
    RegisterClass(&windowClass);
    os.window = CreateWindowEx(0, managerName, managerName, WS_POPUP, 0, 0, 0, 0, 0, 0, instance,
                               0);
    if (!os.window) {
        setOsError(10000);
    unregister:
        UnregisterClass(managerName, os.instance);
        return os.error;
    }
    os.hook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)timerHook, engineInstanceHandle(),
                               appThreadId());
    if (!os.hook) {
        setOsError(10000);
    destroy:
        DestroyWindow(os.window);
        goto unregister;
    }
    if (initThreads((char *)stacks, (char *)stacks + size)) {
        setOsError(threadError());
        UnhookWindowsHookEx(os.hook);
        goto destroy;
    }
    os.running = 1;
    if (osSetActive(1)) {
        error = os.error;
        osShutdown();
        os.error = error;
        return error;
    }
    return setOsError(0);
}

/* @zoombi32 0x0046dfd4 */
short osLockMemory(void *, unsigned long)
{
    return setOsError(0);
}

/* @zoombi32 0x0046dfe2 */
short osUnlockMemory(void *, unsigned long)
{
    return setOsError(0);
}

/* @zoombi32 0x0046dff0 */
short isAppActive()
{
    return os.active;
}

/* @zoombi32 0x0046dff7 */
short osVersion()
{
    return os.running ? 0x500 : 0;
}

/* Whether a thread runs the Mohawk OS layer: whether it has a manager
   window, which answers with the layer's version. */
/* @zoombi32 0x0046e00b */
short isMohawkThread(unsigned long thread)
{
    short version = 0;

    EnumThreadWindows(thread, (WNDENUMPROC)findManagerWindow, (LPARAM)&version);
    return version;
}

/* Not exact: the original keeps `version` in esi. */
/* @zoombi32 0x0046e02f */
BOOL CALLBACK findManagerWindow(HWND window, short *version)
{
    char title[64];

    GetWindowText(window, title, sizeof(title));
    if (!strcmp(title, managerName)) {
        *version = (short)SendMessage(window, WM_USER, 0, 0);
        return FALSE;
    }
    return TRUE;
}

/* @zoombi32 0x0046e081 */
void osShutdown()
{
    enterAllLocks();
    osSetActive(0);
    stopThreads();
    while (os.timers)
        deleteTimer(timerHandle(os.timers));
    UnhookWindowsHookEx(os.hook);
    DestroyWindow(os.window);
    UnregisterClass(managerName, os.instance);
    os.running = 0;
}

/* Sets the function told when the application is activated or
   deactivated, returning the previous one. */
/* @zoombi32 0x0046e0d7 */
ActivateHook setActivateHook(ActivateHook hook)
{
    ActivateHook old = os.activateHook;
    os.activateHook = hook;
    return old;
}

/* @zoombi32 0x0046e0ec */
HINSTANCE fn_46e0ec(long)
{
    return os.instance;
}

/* Sets a timer to run `interval` ms from now (-1: at every chance; 0:
   never). Not exact: the original adds with `lea` where BCC32 4.5 moves and
   adds. */
/* @zoombi32 0x0046e0f8 */
void setTimerInterval(long handle, long interval)
{
    OsTimer *timer;

    if ((timer = timerOf(handle)) == 0) {
        setOsError(0x2743);
        return;
    }
    timer->interval = interval;
    if (interval) {
        unsigned long now = currentTimeMs();

        timer->due = interval == -1 ? now : interval + now;
        if (!os.nextTimer || os.nextTimer > timer->due)
            os.nextTimer = timer->due;
    } else {
        timer->due = 0;
    }
    setOsError(0);
}

/* @zoombi32 0x0046e164 */
LRESULT CALLBACK managerWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_ACTIVATEAPP:
        osSetActive(wParam != 0);
        break;
    case WM_DESTROY:
        return 0;
    case WM_USER: /* isMohawkThread's question */
        return 0x500;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

/* No thread can run: runs the timers (if this is the main thread) and
   waits a moment. */
/* @zoombi32 0x0046e1b2 */
void osIdle()
{
    if (currentThread() == mainThread()) {
        disableScheduling();
        runTimers(currentTimeMs(), 1);
        enableScheduling();
    }
    yieldThread(0);
}

/* @zoombi32 0x0046e1e7 */
short setOsError(short error)
{
    return os.error = error;
}

/* @zoombi32 0x0046e1f8 */
long timerHandle(OsTimer *timer)
{
    return (long)timer;
}

/* @zoombi32 0x0046e202 */
OsTimer *timerOf(long handle)
{
    OsTimer *timer = (OsTimer *)handle;

    if (timer && timer->tag == TIMER_TAG)
        return timer;
    return 0;
}

/* A binary-coded decimal version from a version's major and minor bytes
   (3 and 95 give 0x395). */
/* @zoombi32 0x0046e21a */
unsigned short bcdVersion(unsigned short version)
{
    return (unsigned char)lowByte(version) % 10 | (unsigned char)lowByte(version) / 10 << 4
           | (unsigned char)highByte(version) % 10 << 8
           | (unsigned char)highByte(version) / 10 << 12;
}

/* @zoombi32 0x0046e28e */
char __cdecl lowByte(char value)
{
    return value;
}

/* The high byte of a 16-bit value. */
/* @zoombi32 0x0046e296 */
int __cdecl highByte(unsigned short value)
{
    return value >> 8;
}
