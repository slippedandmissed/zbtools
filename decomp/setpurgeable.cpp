/*
 * setpurgeable (Mohawk engine): setPurgeable
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Whether a handle's block may be purged; the previous setting. */
/* @zoombi32 0x0048f464 */
unsigned short setPurgeable(short handle, short purgeable)
{
    HandleEntry *entry;
    unsigned short old;

    if (!validHandle(handle, 0)) {
        setMemError(0x27a7);
        return 0xffff;
    }
    entry = handleEntry(handle);
    old = entry->purgeable;
    entry->purgeable = purgeable;
    setMemError(0);
    return old;
}
