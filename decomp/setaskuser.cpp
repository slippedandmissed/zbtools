/*
 * setaskuser (Mohawk engine): the file layer's way of asking the user
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x004850d4 */
LONG_PTR setAskUser(LONG_PTR handler)
{
    LONG_PTR previous = (LONG_PTR)files.askUser;

    files.askUser = (short (*)(void *))handler;
    return previous;
}
