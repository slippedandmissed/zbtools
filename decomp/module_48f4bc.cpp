/*
 * module_48f4bc (Mohawk engine): swapHandleData, unlockHandle
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Swaps two unlocked handles' blocks. */
/* Not exact: the original keeps `entryB` in eax and `block` in
   edx; BCC32 4.5 gives `entryB` a saved register and `block` a stack slot. */
/* @zoombi32 0x0048f4bc */
short swapHandleData(short a, short b)
{
    HandleEntry *entryA;
    HandleEntry *entryB;
    Block block;

    if (!validHandle(a, 0) || !validHandle(b, 0))
        return setMemError(0x27a7);
    entryA = handleEntry(a);
    entryB = handleEntry(b);
    if (entryA->locks || entryB->locks)
        return setMemError(0x27a8);
    block = entryA->block;
    if ((entryA->block = entryB->block) != 0)
        (*entryA->block)->handle = a;
    if ((entryB->block = block) != 0)
        (*entryB->block)->handle = b;
    return setMemError(0);
}

/* @zoombi32 0x0048f550 */
short unlockHandle(short handle)
{
    HandleEntry *entry;

    if (!validHandle(handle, 0))
        return setMemError(0x27a7);
    entry = handleEntry(handle);
    if (!entry->locks)
        return setMemError(0x27aa);
    entry->locks--;
    if (!entry->locks)
        GlobalUnlock(entry->block);
    return setMemError(0);
}
