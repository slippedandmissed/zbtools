/*
 * os_46f5c0 (0x46f5c0-0x46f7a4): follows the threading classes (after event::~event)
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

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

/* @zoombi32 0x0046f78e */
void fn_46f78e(short value)
{
    g_4b9d4c = value;
}
