/*
 * os_threads (0x46e2a4-0x46f5c0): the threading classes; timeGetTime
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

/* @zoombi32 0x0046e5dc */
long fn_46e5dc()
{
    return fn_46f43a(g_4b9d74);
}

/* @zoombi32 0x0046e5ed */
short threadError()
{
    return g_4b9d4c;
}

/* @zoombi32 0x0046e5f4 */
long fn_46e5f4()
{
    return fn_46f43a(g_4b9d70);
}

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

/* @zoombi32 0x0046f43a */
long __cdecl fn_46f43a(long value)
{
    return value;
}
