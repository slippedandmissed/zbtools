/*
 * sethandlelocks (Mohawk engine): setHandleLocks, setHandleState
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Sets a handle's lock count (up to 127); the previous one. */
/* @zoombi32 0x0048f360 */
unsigned short setHandleLocks(short handle, unsigned short locks)
{
    HandleEntry *entry;
    unsigned short old;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0xffff;
    }
    if (locks > 0x7f) {
        setMemError(0x27aa);
        return 0xffff;
    }
    entry = handleEntry(handle);
    if (!entry->block) {
        setMemError(0x2775);
        return 0xffff;
    }
    old = entry->locks;
    entry->locks = locks;
    if (locks > 0 && !old)
        GlobalLock(entry->block);
    else if (!locks && old > 0)
        GlobalUnlock(entry->block);
    setMemError(0);
    return old;
}

/* @zoombi32 0x0048f404 */
short setHandleState(short handle, unsigned short state)
{
    if (!validHandle(handle, 0))
        return setMemError(0x27a7);
    handleEntry(handle)->state = state;
    return setMemError(0);
}
