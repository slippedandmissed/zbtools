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
    Counted *next;
    long references;
};

/* @zoombi32 0x0046d827 */
long __cdecl fn_46d827(Counted *object)
{
    return atomicIncrement(&object->references);
}

/* @zoombi32 0x0046dfd4 */
void fn_46dfd4(long, long)
{
    fn_46e1e7(0);
}

/* @zoombi32 0x0046dfe2 */
void fn_46dfe2(long, long)
{
    fn_46e1e7(0);
}

/* An object whose third virtual function takes a flag. */
class Releasable
{
public:
    virtual void __cdecl virtual0();
    virtual void __cdecl virtual1();
    virtual void __cdecl virtual2(int flag);
};

/* @zoombi32 0x0046e842 */
void fn_46e842(Releasable *object)
{
    object->virtual2(0);
}

/* @zoombi32 0x0046eac8 */
void fn_46eac8(Releasable *object)
{
    object->virtual2(1);
}

/* Something starting with the tag 'ksTI' (bytes in memory order). */
struct Tagged
{
    long tag;
};

/* The object if it carries the tag, else null. */
/* @zoombi32 0x0046e202 */
Tagged *fn_46e202(Tagged *object)
{
    if (object && object->tag == 0x4954736bL)
        return object;
    return 0;
}

/* Whether a pointer is non-null and 4-byte aligned. */
/* @zoombi32 0x0046da46 */
int isAlignedPointer(void *pointer)
{
    if (!pointer || ((unsigned long)pointer & 3))
        return 0;
    return 1;
}

extern Counted *g_4a8dcc;

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

/* Something that records a return address at +0x28. */
struct Resume
{
    char unknown0[0x28];
    long address;
};

/*
 * Records in `resume` the return address `depth` frames up the stack. The
 * frame walk was hand-written (`sub 1` / `jb` on the parameter in memory, a
 * loop BCC32 doesn't generate), so it's emitted as bytes.
 */
/* @zoombi32 0x0046f771 */
void fn_46f771(Resume *resume, unsigned short depth)
{
    _EAX = _EBP;
    /* up: sub word ptr [depth], 1 / jb done / mov eax, [eax] / jmp up / done: */
    __emit__(0x66, (unsigned char)0x83, 0x6d, 0x08, 0x01, 0x72, 0x04, (unsigned char)0x8b, 0x00,
             (unsigned char)0xeb, (unsigned char)0xf5);
    _EDX = ((long *)_EAX)[1];
    resume->address = _EDX;
}
