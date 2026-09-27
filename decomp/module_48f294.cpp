/*
 * module_48f294 (Mohawk engine): fillMemory
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* @zoombi32 0x0048f294 */
void fillMemory(void *to, unsigned char value, unsigned long size)
{
    memset(to, value, size);
}
