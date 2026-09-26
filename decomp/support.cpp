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

extern short g_4b9cf0;
extern short g_4b9cf4;
extern short g_4b9d4c;

/* @zoombi32 0x0046da35 */
void fn_46da35(short value)
{
    g_4b9cf0 = value;
}

/* @zoombi32 0x0046e1e7 */
void fn_46e1e7(short value)
{
    g_4b9cf4 = value;
}

/* @zoombi32 0x0046f78e */
void fn_46f78e(short value)
{
    g_4b9d4c = value;
}

extern long g_4b9cfc;

/* Sets g_4b9cfc, returning its old value. */
/* @zoombi32 0x0046e0d7 */
long fn_46e0d7(long value)
{
    long old = g_4b9cfc;
    g_4b9cfc = value;
    return old;
}

long __cdecl fn_46f43a(long value);
extern long g_4b9d70;
extern long g_4b9d74;

/* @zoombi32 0x0046e5dc */
long fn_46e5dc()
{
    return fn_46f43a(g_4b9d74);
}

/* @zoombi32 0x0046e5f4 */
long fn_46e5f4()
{
    return fn_46f43a(g_4b9d70);
}

extern short g_4b9cf6;

/* @zoombi32 0x0046dff7 */
short fn_46dff7()
{
    return g_4b9cf6 ? 0x500 : 0;
}

/* Something reference-counted, with its count at +8. */
struct Counted
{
    long unknown0;
    long unknown4;
    long references;
};

/* @zoombi32 0x0046d827 */
long __cdecl fn_46d827(Counted *object)
{
    return atomicIncrement(&object->references);
}
