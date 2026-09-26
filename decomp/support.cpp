/*
 * The support library linked just below the Borland runtime (0x46d754 to
 * 0x46f7a4, with the fileSpec and threading classes). Unlike the rest of the
 * game it was compiled with standard stack frames (no -k-).
 */
/* @flags -p */

#include <windows.h>
#include <mmsystem.h>

/* The time in milliseconds since Windows started. */
/* @zoombi32 0x0046dda5 */
DWORD currentTimeMs()
{
    return timeGetTime();
}

/*
 * Atomic operations on a shared counter, as the threading classes need.
 * Windows 95's InterlockedIncrement/Decrement only return the sign of the
 * result, so these do it themselves; BCC32 can't assemble inline without
 * TASM32, so the locked instructions are emitted as bytes (0xff needs a cast
 * to unsigned char, or BCC32 emits it as two bytes).
 */

/* Adds one to *value and returns the result. */
/* @zoombi32 0x0046dabb */
long atomicIncrement(long *value)
{
    _EAX = (long)value;
    __emit__(0xf0, (unsigned char)0xff, 0x00); /* lock inc dword ptr [eax] */
    return *(long *)_EAX;
}

/* Subtracts one from *value and returns the result. */
/* @zoombi32 0x0046da9d */
long atomicDecrement(long *value)
{
    _EAX = (long)value;
    __emit__(0xf0, (unsigned char)0xff, 0x08); /* lock dec dword ptr [eax] */
    return *(long *)_EAX;
}

/* Stores value in *target and returns what was there. */
/* @zoombi32 0x0046daac */
long atomicExchange(long *target, long value)
{
    _EAX = value;
    _EDX = (long)target;
    __emit__(0x87, 0x02); /* xchg dword ptr [edx], eax */
    return _EAX;
}

extern long g_4b9d00;

/* @zoombi32 0x0046e0ec */
long fn_46e0ec(long)
{
    return g_4b9d00;
}

/* The high byte of a 16-bit value. */
/* @zoombi32 0x0046e296 */
int __cdecl highByte(unsigned short value)
{
    return value >> 8;
}
