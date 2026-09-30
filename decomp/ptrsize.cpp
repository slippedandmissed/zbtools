/*
 * ptrsize (Mohawk engine): ptrSize, getMemoryInfo, lockHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048e8f4 */
unsigned long ptrSize(void *pointer)
{
    if (!isPointer(pointer)) {
        setMemError(0x27af);
        return 0;
    }
    setMemError(0);
    return ((Chunk *)pointer - 1)->size;
}

/* @zoombi32 0x0048e928 */
void getMemoryInfo(MemoryInfo *info)
{
    MEMORYSTATUS status;

    status.dwLength = sizeof(status);
    GlobalMemoryStatus(&status);
    info->availableVirtual = status.dwAvailVirtual;
    info->totalVirtual = status.dwTotalVirtual;
    info->availablePhysical = status.dwAvailPhys;
    info->totalPhysical = status.dwTotalPhys;
    info->availablePageFile = status.dwAvailPageFile;
    info->totalPageFile = status.dwTotalPageFile;
}

/* Locks a handle's block (up to 127 times), returning its address. */
/* @zoombi32 0x0048e96c */
void *lockHandle(short handle)
{
    HandleEntry *entry;
    unsigned short locks;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0;
    }
    entry = handleEntry(handle);
    if (!entry->block) {
        setMemError(0x2775);
        return 0;
    }
    if (entry->locks == 0x7f) {
        setMemError(0x27aa);
        return 0;
    }
    locks = entry->locks;
    entry->locks++;
    if (!locks)
        GlobalLock(entry->block);
    entry->age = 15;
    setMemError(0);
    return (char *)*entry->block + sizeof(Chunk);
}
