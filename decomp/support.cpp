/*
 * The support library linked just below the Borland runtime (0x46ce80 to
 * 0x46f7a4, with the fileSpec and threading classes). Unlike the rest of the
 * game it was compiled with standard stack frames (no -k-).
 */
/* @flags -p */

#include <windows.h>
#include <mmsystem.h>

/* The time in milliseconds since Windows started. */
/* @zoombi32 0x0046dda5 */
DWORD fn_46dda5()
{
    return timeGetTime();
}
