/*
 * os_refcount (0x46d7f8-0x46d95c): calls atomicIncrement/Decrement/Exchange
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

/* @zoombi32 0x0046d827 */
long __cdecl fn_46d827(Counted *object)
{
    return atomicIncrement(&object->references);
}
