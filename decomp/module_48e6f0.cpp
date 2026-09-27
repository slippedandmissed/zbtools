/*
 * module_48e6f0 (Mohawk engine): availableMemory
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* How much can be allocated (the available virtual memory, less a block's
   header). */
/* @zoombi32 0x0048e6f0 */
unsigned long availableMemory(unsigned long)
{
    MEMORYSTATUS status;
    unsigned long available;

    status.dwLength = sizeof(status);
    GlobalMemoryStatus(&status);
    available = status.dwAvailVirtual;
    return available > sizeof(Chunk) ? available - sizeof(Chunk) : 0;
}
