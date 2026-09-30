/*
 * availablevirtualmemory (Mohawk engine): availableVirtualMemory
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048e7ec */
unsigned long availableVirtualMemory()
{
    MEMORYSTATUS status;

    status.dwLength = sizeof(status);
    GlobalMemoryStatus(&status);
    return status.dwAvailVirtual;
}
