/*
 * module_488874 (Mohawk engine): colours
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00488874 */
__cdecl Color::Color()
{
    *(RGBColor *)this = RGBColor(0, 0, 0, 0xff);
}

/* @zoombi32 0x0048889d */
__cdecl Color::Color(unsigned short index)
{
    *(RGBColor *)this = RGBColor(0, 0, 0, 0xff);
    if (index != 0xffff) {
        bytes.kind = 0x80;
        this->index = index;
    }
}

/* Takes another colour's value, as RGB (dropping the index flag). */
/* @zoombi32 0x004888d9 */
Color &__cdecl Color::setRgb(const Color &color)
{
    value = color.value;
    if (bytes.kind != 0xff)
        bytes.kind &= 0x7f;
    return *this;
}

/* The colour's index in the current port's palette (0xffff if there's
   none). */
/* @zoombi32 0x004888f2 */
unsigned short __cdecl Color::paletteIndex() const
{
    basePort *port;

    if (bytes.kind == 0xff) {
        setPortError(0x2a64);
        return 0xffff;
    }
    if ((port = portObject(1)) == 0)
        return 0xffff;
    if (bytes.kind == 0x80) {
        if (!(graphics.paletteReserved / 2 + port->palette->first > index
              || (index < 0x100 && 0x100 - graphics.paletteReserved / 2 <= index))) {
            setPortError(0x2a64);
            return 0xffff;
        }
        setPortError(0);
        return index;
    }
    return port->nearestIndex(RGBColor(*this));
}

/* The colour as RGB (a palette index looked up in the current port). */
/* @zoombi32 0x004889a4 */
RGBColor __cdecl Color::rgb() const
{
    basePort *port;

    if (bytes.kind == 0xff) {
        setPortError(0x2a64);
        return RGBColor(0, 0, 0, 0xff);
    }
    if ((port = portObject(1)) == 0)
        return RGBColor(0, 0, 0, 0xff);
    return bytes.kind == 0x80 ? port->paletteColor(index) : RGBColor(*this);
}

/* -1 none, 0 a palette index, 1 or 2 RGB (1 with flag 0x10). */
/* @zoombi32 0x00488a39 */
long __cdecl Color::kind() const
{
    long kind;

    if (bytes.kind == 0xff)
        kind = -1;
    else if (bytes.kind == 0x80)
        kind = 0;
    else {
        kind = 1;
        if (!(bytes.kind & 0x10))
            kind++;
    }
    return kind;
}

/* @zoombi32 0x00488a64 */
__cdecl RGBColor::RGBColor(const Color &color)
{
    bytes.red = color.bytes.red;
    bytes.green = color.bytes.green;
    bytes.blue = color.bytes.blue;
    bytes.kind = color.bytes.kind;
}
