/*
 * module_48b530 (Mohawk engine): textWidth
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/* How wide text is in the current port's font (0xffff with no port). */
/* @zoombi32 0x0048b530 */
unsigned short textWidth(const char *text, unsigned short length)
{
    basePort *port;
    SIZE size;

    if ((port = portObject(1)) == 0)
        return 0xffff;
    GetTextExtentPoint(port->dc, text, length == 0xffff ? strlen(text) : length, &size);
    return size.cx ? size.cx - port->metrics.tmOverhang : 0;
}
