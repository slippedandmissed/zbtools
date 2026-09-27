/*
 * module_48187c (Mohawk engine): adding rectangles to regions
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * Adds `count` rectangles to a region, keeping its list of rectangles sorted
 * (by top, then left) and without overlaps: a rectangle that overlaps others
 * is split, the pieces still to add kept on a stack (at most 64).
 */
/* @zoombi32 0x0048187c */
short unionRgnRects(short handle, long count, ShortRect *rects)
{
    Region *data;
    long first;
    ShortRect cur;
    long start;
    long i;
    long insertAt;
    short bottom;
    ShortRect pending[64];
    long pendingCount;
    long j;
    long k;
    ShortRect *r;

    data = getRegion(handle);
    if (!data)
        return setRegionError(0x2937);
    first = -1;
    while (count--) {
        r = rects++;
        cur = *r;
        if (!data->count)
            data->bounds = cur;
        else {
            data->bounds.left = data->bounds.left > cur.left ? cur.left : data->bounds.left;
            data->bounds.top = data->bounds.top > cur.top ? cur.top : data->bounds.top;
            data->bounds.right = data->bounds.right < cur.right ? cur.right : data->bounds.right;
            data->bounds.bottom = data->bounds.bottom < cur.bottom ? cur.bottom : data->bounds.bottom;
        }
        pendingCount = 0;
        start = first >= 0 ? first : 0;
        i = start;
        first = -1;
        for (;;) {
            insertAt = -1;
            while (data->count > i) {
                r = &data->rects[i];
                if (cur.top >= r->top && cur.left >= r->left && cur.right <= r->right
                    && cur.bottom <= r->bottom)
                    goto next;
                if (cur.top <= r->top && cur.left <= r->left && cur.right >= r->right
                    && cur.bottom >= r->bottom) {
                    removeRgnRect(data, i);
                    if (insertAt < 0)
                        insertAt = i;
                    continue;
                }
                if (cur.bottom < r->top)
                    break;
                if (cur.bottom == r->top) {
                    if (cur.left != r->left || cur.right != r->right)
                        break;
                    r->top = cur.top;
                    if (first < 0)
                        first = i;
                    goto next;
                }
                if ((cur.top < r->top || (cur.top == r->top && cur.right < r->left)) && insertAt < 0)
                    insertAt = i;
                if (cur.top <= r->bottom && cur.left <= r->right && cur.right >= r->left) {
                    if (cur.top == r->bottom) {
                        if (cur.left == r->left && cur.right == r->right) {
                            cur.top = r->top;
                            removeRgnRect(data, insertAt = i);
                            continue;
                        }
                    } else {
                        if (pendingCount + 2 > 64)
                            return setRegionError(0x2936);
                        if (cur.top < r->top) {
                            if (cur.left >= r->left && cur.right <= r->right && cur.bottom <= r->bottom)
                                pending[pendingCount++] = *r;
                            else {
                                if (cur.bottom < r->bottom) {
                                    pending[pendingCount].top = cur.bottom;
                                    pending[pendingCount].bottom = r->bottom;
                                    pending[pendingCount].left = r->left;
                                    pending[pendingCount].right = r->right;
                                    pendingCount++;
                                } else if (cur.bottom > r->bottom) {
                                    pending[pendingCount].top = r->bottom;
                                    pending[pendingCount].bottom = cur.bottom;
                                    pending[pendingCount].left = cur.left;
                                    pending[pendingCount].right = cur.right;
                                    pendingCount++;
                                }
                                pending[pendingCount].top = r->top;
                                pending[pendingCount].bottom = cur.bottom < r->bottom ? cur.bottom : r->bottom;
                                pending[pendingCount].left = cur.left < r->left ? cur.left : r->left;
                                pending[pendingCount].right = cur.right > r->right ? cur.right : r->right;
                                pendingCount++;
                            }
                            bottom = r->top;
                            j = insertAt >= 0 ? insertAt : i;
                            if (j < i)
                                memmove(&data->rects[j + 1], &data->rects[j], (i - j) * sizeof(ShortRect));
                            data->rects[j].top = cur.top;
                            data->rects[j].bottom = bottom;
                            data->rects[j].left = cur.left;
                            data->rects[j].right = cur.right;
                            i = j;
                            cur = pending[--pendingCount];
                            insertAt = -1;
                            continue;
                        }
                        if (cur.top == r->top) {
                            if (cur.bottom < r->bottom) {
                                pending[pendingCount].top = cur.bottom;
                                pending[pendingCount].bottom = r->bottom;
                                pending[pendingCount].left = r->left;
                                pending[pendingCount].right = r->right;
                                pendingCount++;
                            } else if (cur.bottom > r->bottom) {
                                pending[pendingCount].top = r->bottom;
                                pending[pendingCount].bottom = cur.bottom;
                                pending[pendingCount].left = cur.left;
                                pending[pendingCount].right = cur.right;
                                pendingCount++;
                            }
                            cur.left = cur.left < r->left ? cur.left : r->left;
                            cur.right = cur.right > r->right ? cur.right : r->right;
                            cur.bottom = cur.bottom < r->bottom ? cur.bottom : r->bottom;
                            removeRgnRect(data, i);
                            k = i;
                            while (k--) {
                                if (data->rects[k].right < cur.left || data->rects[k].left > cur.right)
                                    continue;
                                if (data->rects[k].left == cur.left && data->rects[k].right == cur.right
                                    && data->rects[k].bottom == cur.top)
                                    i = k;
                                break;
                            }
                            insertAt = -1;
                            continue;
                        }
                        if (cur.left >= r->left && cur.right <= r->right)
                            cur.top = r->bottom;
                        else {
                            if (cur.left > r->left || cur.right < r->right || cur.bottom < r->bottom) {
                                if (cur.bottom < r->bottom) {
                                    pending[pendingCount].top = cur.bottom;
                                    pending[pendingCount].bottom = r->bottom;
                                    pending[pendingCount].left = r->left;
                                    pending[pendingCount].right = r->right;
                                    pendingCount++;
                                } else if (cur.bottom > r->bottom) {
                                    pending[pendingCount].top = r->bottom;
                                    pending[pendingCount].bottom = cur.bottom;
                                    pending[pendingCount].left = cur.left;
                                    pending[pendingCount].right = cur.right;
                                    pendingCount++;
                                }
                                cur.left = cur.left < r->left ? cur.left : r->left;
                                cur.right = cur.right > r->right ? cur.right : r->right;
                                cur.bottom = cur.bottom < r->bottom ? cur.bottom : r->bottom;
                            }
                            r->bottom = cur.top;
                        }
                        insertAt = -1;
                    }
                }
                i++;
            }
            if (insertRgnRect(handle, &data, insertAt >= 0 ? insertAt : i, &cur))
                return regionErrorCode;
            if (first < 0)
                first = i;
        next:
            if (!pendingCount--)
                break;
            cur = pending[pendingCount];
            i = start;
        }
    }
    shrinkRgn(handle, &data);
    return setRegionError(0);
}

/* A region's data, or 0. */
/* @zoombi32 0x00481f8e */
Region *getRegion(short region)
{
    return region ? (Region *)handleData(region) : 0;
}

/* Gives back unused room in a region, to the next multiple of 16 rectangles
   past what it holds. */
/* @zoombi32 0x00481fa9 */
void shrinkRgn(short handle, Region **region)
{
    unsigned short capacity = ((*region)->count / 16 + 1) * 16;

    if (capacity < (*region)->capacity) {
        setHandleSize(handle, ((*region)->capacity = capacity) * sizeof(ShortRect) + 0x10);
        *region = (Region *)handleData(handle);
    }
}

/* @zoombi32 0x00481ff4 */
void removeRgnRect(Region *region, long index)
{
    if (index + 1 < region->count--)
        memcpy(&region->rects[index], &region->rects[index + 1],
               (region->count - index) * sizeof(ShortRect));
}
