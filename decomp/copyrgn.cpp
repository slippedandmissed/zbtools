/*
 * copyrgn (Mohawk engine): compacting, copying, conversion to a Windows region
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * Frees the region's unused rectangles.
 *
 * Not exact: the original has data in ebx, count in esi and error in edi.
 */
/* @zoombi32 0x00480fc0 */
void compactRgn(short region)
{
    Region *data;
    short error;
    unsigned short count;

    if ((data = getRegion(region)) == 0) {
        setRegionError(0x2937);
        return;
    }
    error = 0;
    count = data->count;
    if (count < data->capacity) {
        error = setHandleSize(region, count * sizeof(ShortRect) + 0x10);
        if (!error)
            ((Region *)handleData(region))->capacity = count;
    }
    setRegionError(error);
}

/* Not exact: the original tests `from`'s data in eax (no variable) and
   computes the size in 32 bits, straight into edi. */
/* @zoombi32 0x00481024 */
short copyRgn(short to, short from)
{
    Region *data;
    unsigned short size;

    if (!getRegion(to))
        return setRegionError(0x2937);
    if (to == from) {
        return setRegionError(0);
    }
    if ((data = getRegion(from)) == 0)
        return setRegionError(0x2937);
    size = data->capacity * 8 + 0x10;
    if (setHandleSize(to, size)) {
        return setRegionError(memError());
    }
    memcpy(handleData(to), handleData(from), size);
    return setRegionError(0);
}

/* Makes a Windows region from a region, moved by (dx, dy). */
/* @zoombi32 0x004810b8 */
short regionToHrgn(HRGN target, short region, short dx, short dy)
{
    HRGN spare;
    unsigned short swapped;
    HRGN swap;
    Region *data;
    HRGN other;
    long i;

    if (!target)
        return setRegionError(0x2937);
    if ((data = getRegion(region)) == 0)
        return setRegionError(0x2937);
    other = CreateRectRgn(0, 0, 0, 0);
    if (!other)
        return setRegionError(0x2904);
    spare = CreateRectRgn(0, 0, 0, 0);
    if (!spare) {
        DeleteObject(other);
        return setRegionError(0x2904);
    }
    swapped = 0;
    SetRectRgn(target, 0, 0, 0, 0);
    for (i = 0; i < data->count; i++) {
        SetRectRgn(spare, data->rects[i].left + dx, data->rects[i].top + dy,
                   data->rects[i].right + dx, data->rects[i].bottom + dy);
        CombineRgn(other, target, spare, RGN_OR);
        swapped = !swapped;
        swap = other;
        other = target;
        target = swap;
    }
    if (swapped) {
        target = other;
        other = swap;
        CombineRgn(target, other, 0, RGN_COPY);
    }
    DeleteObject(spare);
    DeleteObject(other);
    return setRegionError(0);
}
