/*
 * graphics (0x4144d0-0x414f30): 'unable to initialize graphics', 'unable to create palette', 'work port', 'back port'
 */

#include "zoombinis.h"

/* @zoombi32 0x00414ce7 */
void fn_414ce7(short *handle)
{
    if (*handle) {
        fn_4812bc(*handle);
        *handle = 0;
    }
}
