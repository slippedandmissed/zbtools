/*
 * os_manager (0x46da64-0x46e2a4): 'MOHAWK OS Manager'; SetTimer, window hooks
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include <windows.h>
#include <mmsystem.h>
#include "zoombinis.h"

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
long atomicExchange(long *target, long value)
{
    return InterlockedExchange(target, value);
}

/* Adds one to *value and returns the result. */
/* @zoombi32-functional 0x0046dabb */
long atomicIncrement(long *value)
{
    return InterlockedIncrement(value);
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

/* Adds a reference to everything in the list at g_4a8dcc. */
/* @zoombi32 0x0046dc45 */
void fn_46dc45()
{
    Counted *object = g_4a8dcc;
    while (object) {
        fn_46d827(object);
        object = object->next;
    }
}

/* @zoombi32 0x0046dd21 */
HINSTANCE engineInstanceHandle()
{
    return engineInstance;
}

/* @zoombi32 0x0046dd27 */
unsigned long appThreadId()
{
    return appThread;
}

/* @zoombi32 0x0046dd2d */
HWND appWindowHandle()
{
    return appWindow;
}

/* The time in milliseconds since Windows started. */
/* @zoombi32 0x0046dda5 */
DWORD currentTimeMs()
{
    return timeGetTime();
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
    return engineActive;
}

/* @zoombi32 0x0046dff7 */
short fn_46dff7()
{
    return g_4b9cf6 ? 0x500 : 0;
}

/* Sets the function told when the application is activated or
   deactivated, returning the previous one. */
/* @zoombi32 0x0046e0d7 */
ActivateHook setActivateHook(ActivateHook hook)
{
    ActivateHook old = activateHook;
    activateHook = hook;
    return old;
}

/* @zoombi32 0x0046e0ec */
HINSTANCE fn_46e0ec(long)
{
    return engineInstance;
}

/* @zoombi32 0x0046e1e7 */
short setOsError(short error)
{
    return osError = error;
}

/* @zoombi32 0x0046e1f8 */
long fn_46e1f8(long value)
{
    return value;
}

/* The object if it carries the tag, else null. */
/* @zoombi32 0x0046e202 */
Tagged *fn_46e202(Tagged *object)
{
    if (object && object->tag == 0x4954736bL)
        return object;
    return 0;
}

/* @zoombi32 0x0046e28e */
char __cdecl fn_46e28e(char value)
{
    return value;
}

/* The high byte of a 16-bit value. */
/* @zoombi32 0x0046e296 */
int __cdecl highByte(unsigned short value)
{
    return value >> 8;
}
