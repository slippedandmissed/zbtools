/*
 * dibport (Mohawk engine): DIBPort, a port drawing into a DIB section, and
 * the out-of-line inline functions of Rect and DIB it uses
 */

/* @flags -p -x- */

#include <string.h>
#define RECT_OUT_OF_LINE
#include "zoombinis.h"

/* @zoombi32 0x0048a720 */
__cdecl DIBPort::DIBPort(short width, short height, unsigned short depth)
    : basePort(Rect(0, 0, width, height))
{
    dib = 0;
    this->depth = depth;
    paletteKind = 0;
    kind = 0;
}

/* Copies through the DIB, unless within the port and overlapping. Not
   exact: the original keeps `this` in ebx and `flags` in edi; BCC32 4.5 swaps
   them. */
/* @zoombi32 0x0048a775 */
short DIBPort::copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                        unsigned char flags)
{
    Rect clipped;

    if (port != this || !sectRect(&(clipped = *from), (ShortRect *)to)) {
        SetStretchBltMode(port->dc,
                          flags & 1 ? BLACKONWHITE : flags & 2 ? WHITEONBLACK : COLORONCOLOR);
        Rect source = *from;
        if (scaled) {
            WinRect device(source);

            LPtoDP(dc, (POINT *)&device, 2);
            source = device;
        } else {
            offsetRect(&source, -offset.x, -offset.y);
        }
        DWORD rop = copyRops[mode];
        unsigned short flipX = (flags & 0x10) != 0;
        unsigned short flipY = (flags & 0x20) != 0;
        long usage = depth == 8 && port->palette == palette
                         ? dib->usage(port->paletteKind, rop, flipX, flipY)
                         : 0;
        return dib->draw(port, *to, source, usage, rop, flipX, flipY);
    }
    return basePort::copyBits(port, to, from, mode, flags);
}

/* @zoombi32 0x0048a8fe */
void DIBPort::prepare()
{
    drawn = 1;
    basePort::prepare();
}

/* @zoombi32 0x0048a917 */
void DIBPort::realizePalette()
{
    if (palette != realized || !palette->realized || palette->changed) {
        SetPaletteEntries(hpal, 0, 256, palette->entries);
        if (depth == 8)
            dib->setEntries(0, 256, palette->entries);
    }
    RealizePalette(dc);
    realized = palette;
}

/* @zoombi32 0x0048a989 */
void DIBPort::getBits(PixMap *map)
{
    map->bits = dib->bits;
    map->rowBytes = -dib->rowBytes;
    map->depth = depth;
    map->dibBounds = dib->bounds;
    map->bounds = bounds;
    map->frame = frame;
    setPortError(0);
}

/* @zoombi32 0x0048a9f8 */
short DIBPort::init()
{
    short error;

    if ((error = basePort::init()) != 0)
        return error;
    dib = new DIB(frame.right, frame.bottom, depth);
    if (!dib) {
        basePort::release();
        return setPortError(0x2a37);
    }
    if ((error = dib->create(1)) != 0) {
        delete dib;
        basePort::release();
        return graphics.error = error;
    }
    paletteKind = dib->portUsage;
    return graphics.error;
}

/* @zoombi32 0x0048aaae */
short DIBPort::lock()
{
    if (!locks) {
        if ((dc = dib->getDC()) == 0)
            return graphics.error;
        if (setupDC()) {
            DeleteDC(dc);
            return graphics.error;
        }
        depth = dib->depth;
        unknown66 = 0;
        unknown68 = 1;
        unknown6A = 1;
    }
    locks++;
    return setPortError(0);
}

/* @zoombi32 0x0048ab27 */
void DIBPort::unlock()
{
    if (!--locks) {
        cleanupDC();
        dib->releaseDC(dc);
    }
}

/* @zoombi32 0x0048ab5a */
void DIBPort::release()
{
    dib->destroy();
    delete dib;
    basePort::release();
}

/* @zoombi32 0x0048ab89 */
__cdecl Rect::Rect()
{
}

/* @zoombi32 0x0048ab91 */
Rect &__cdecl Rect::operator=(const ShortRect &rect)
{
    return *(Rect *)memcpy(this, &rect, sizeof(Rect));
}

/* @zoombi32 0x0048aba7 */
void __cdecl DIB::operator delete(void *block)
{
    localFree(block);
}

/* @zoombi32 0x0048ac29 */
void __cdecl Rect::operator=(const tagRECT &rect)
{
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;
}

/* @zoombi32 0x0048ac52 */
void *__cdecl DIB::operator new(size_t size)
{
    void *block = localAlloc(size);

    return block ? block : 0;
}
