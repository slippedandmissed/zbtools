/*
 * module_4850d4 (Mohawk engine): the file layer's way of asking the user
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x004850d4 */
long setAskUser(long handler)
{
    long previous = (long)files.askUser;

    files.askUser = (short (*)(void *))handler;
    return previous;
}
