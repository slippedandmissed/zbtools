/*
 * module_48ea00 (Mohawk engine): lockHandleAlias
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* lockHandle under another name. */
/* @zoombi32 0x0048ea00 */
void *lockHandleAlias(short handle)
{
    return lockHandle(handle);
}
