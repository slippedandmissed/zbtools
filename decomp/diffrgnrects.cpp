/*
 * diffrgnrects (Mohawk engine): diffRgnRects
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * Takes `count` rectangles out of a region: each rectangle it cuts is
 * replaced by the (up to four) pieces left around the cut.
 *
 * Not exact: the original keeps the address of `data` in edi (and p, j in
 * esi, ebx); this reads it from the frame. Going through an explicit
 * `Region **` gets the address into a register, but ranks it first.
 */
/* @zoombi32 0x0048130c */
short diffRgnRects(short handle, long count, ShortRect *rects)
{
    Region *data;
    long start;
    ShortRect cut;
    long i;
    ShortRect piece;
    long pieceCount;
    ShortRect pieces[4];
    long p;
    long j;
    ShortRect *r;

    data = getRegion(handle);
    if (!data)
        return setRegionError(0x2937);
    start = 0;
    while (count--) {
        r = rects++;
        cut = *r;
        for (i = start; data->count > i; i++) {
            r = &data->rects[i];
            if (r->bottom <= cut.top) {
                start = i + 1;
                continue;
            }
            if (r->top >= cut.bottom)
                break;
            if (r->left >= cut.right || r->right <= cut.left)
                continue;
            if (r->top >= cut.top && r->left >= cut.left && r->bottom <= cut.bottom
                && r->right <= cut.right) {
                memcpy(&data->rects[i], &data->rects[i + 1],
                       (--data->count - i) * sizeof(ShortRect));
                i--;
                continue;
            }
            piece = *r;
            pieceCount = 0;
            if (cut.top > piece.top) {
                pieces[0].top = piece.top;
                pieces[0].left = piece.left;
                pieces[0].right = piece.right;
                pieces[0].bottom = piece.top = cut.top;
                pieceCount++;
            }
            if (cut.left > piece.left) {
                pieces[pieceCount].left = piece.left;
                pieces[pieceCount].top = piece.top;
                pieces[pieceCount].right = piece.right < cut.left ? piece.right : cut.left;
                pieces[pieceCount].bottom = cut.bottom < piece.bottom ? cut.bottom : piece.bottom;
                pieceCount++;
            }
            if (cut.right < piece.right) {
                pieces[pieceCount].left = piece.left > cut.right ? piece.left : cut.right;
                pieces[pieceCount].top = piece.top;
                pieces[pieceCount].right = piece.right;
                pieces[pieceCount].bottom = cut.bottom < piece.bottom ? cut.bottom : piece.bottom;
                pieceCount++;
            }
            if (cut.bottom < piece.bottom) {
                pieces[pieceCount].top = cut.bottom;
                pieces[pieceCount].left = piece.left;
                pieces[pieceCount].right = piece.right;
                pieces[pieceCount].bottom = piece.bottom;
                pieceCount++;
            }
            if (r->top == pieces[0].top) {
                *r = pieces[0];
                p = 1;
                j = i + 1;
            } else {
                memcpy(&data->rects[i], &data->rects[i + 1],
                       (--data->count - i) * sizeof(ShortRect));
                p = 0;
                j = i;
            }
            if (p < pieceCount)
                do {
                    for (; j < data->count
                           && (pieces[p].top > data->rects[j].top
                               || (pieces[p].top == data->rects[j].top
                                   && pieces[p].left >= data->rects[j].left));
                         j++)
                        ;
                    insertRgnRect(handle, &data, j++, &pieces[p++]);
                } while (p < pieceCount);
        }
    }
    tidyRgn(data);
    shrinkRgn(handle, &data);
    return setRegionError(0);
}
