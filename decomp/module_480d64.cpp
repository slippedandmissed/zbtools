/*
 * module_480d64 (Mohawk engine): sectRgnRects
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Intersects a region with the union of `count` rectangles, building the
   result in a new handle and then moving it into the region. */
/* @zoombi32 0x00480d64 */
short sectRgnRects(short handle, long count, ShortRect *rects)
{
    Region *data;
    short result;
    Region *out;
    long start;
    ShortRect cut;
    ShortRect piece;
    long i;
    long k;
    short error;
    ShortRect *r;

    data = getRegion(handle);
    if (!data)
        return setRegionError(0x2937);
    lockHandle(handle);
    if ((result = newHandle(0x90)) == 0) {
        error = memError();
        unlockHandle(handle);
        return setRegionError(error);
    }
    out = (Region *)handleData(result);
    out->tag = RESOURCE_TYPE('r', 'g', 'n', 'R');
    out->capacity = 16;
    out->count = 0;
    start = 0;
    while (count--) {
        r = rects++;
        cut = *r;
        for (i = start; i < data->count; i++) {
            r = &data->rects[i];
            if (cut.top >= r->bottom) {
                start = i + 1;
                continue;
            }
            if (cut.bottom <= r->top)
                break;
            if (cut.right <= r->left || cut.left >= r->right)
                continue;
            setRect(&piece, cut.left > r->left ? cut.left : r->left,
                    cut.top > r->top ? cut.top : r->top,
                    cut.right < r->right ? cut.right : r->right,
                    cut.bottom < r->bottom ? cut.bottom : r->bottom);
            if (!out->count)
                out->rects[out->count++] = piece;
            else {
                for (k = out->count; k; k--)
                    if (out->rects[k - 1].top < piece.top
                        || (out->rects[k - 1].top == piece.top && out->rects[k - 1].left <= piece.left))
                        break;
                insertRgnRect(result, &out, k, &piece);
            }
        }
    }
    tidyRgn(out);
    shrinkRgn(result, &out);
    unlockHandle(handle);
    setRegionError(fn_48f4bc(handle, result));
    disposeHandle(result);
    return regionErrorCode;
}
