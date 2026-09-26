/*
 * os_46f5c0 (0x46f5c0-0x46f7a4): follows the threading classes (after event::~event)
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

/*
 * Records in `resume` the return address `depth` frames up the stack: a
 * hand-written loop follows the saved frame pointers (`mov eax, [eax]`), then
 * reads the return address above the frame reached. That depends on the x86
 * stack layout, so it has no portable equivalent; it's part of the Mohawk OS
 * layer's stack switching (with 0x46f6f9 and 0x46f74f), which a port replaces
 * (e.g. with Windows fibers). Left unimplemented.
 */
/* @zoombi32 0x0046f771 */
void fn_46f771(Resume *resume, unsigned short depth)
{
}

/* @zoombi32 0x0046f78e */
void fn_46f78e(short value)
{
    g_4b9d4c = value;
}
