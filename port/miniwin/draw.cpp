/*
 * Filling and outlining: FillRect, FillRgn, InvertRect, Rectangle, Ellipse,
 * Polygon and lines, with the DC's brush, pen and ROP2 mode.
 */

#include <math.h>
#include <stdlib.h>

#include <algorithm>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

void plot(DC *dc, int x, int y, uint8_t value, int rop2);
bool isDrawable(DC *dc);
std::vector<RECT> polygonRows(const POINT *points, int count, int mode);

/* FillRect's brush may be a system colour's index, plus one. */
static Brush *brushOf(HBRUSH handle, Brush &system)
{
    uintptr_t value = (uintptr_t)handle;
    if (value && value <= 31) {
        system.color = GetSysColor((int)value - 1);
        system.pattern = 0;
        system.null = false;
        return &system;
    }
    GdiObject *object = objectOf(handle);
    return object && object->kind == KIND_BRUSH ? (Brush *)object : 0;
}

int FillRect(HDC handle, const RECT *rect, HBRUSH brushHandle)
{
    DC *dc = dcOf(handle);
    Brush system;
    Brush *brush = brushOf(brushHandle, system);

    if (!dc || !brush)
        return 0;
    fillRectDevice(dc, toDeviceRect(dc, rect->left, rect->top, rect->right, rect->bottom), brush,
                   PATCOPY);
    system.pattern = 0;
    return 1;
}

BOOL FillRgn(HDC handle, HRGN rgn, HBRUSH brushHandle)
{
    DC *dc = dcOf(handle);
    Region *region = regionOf(rgn);
    Brush system;
    Brush *brush = brushOf(brushHandle, system);

    if (!dc || !region || !brush)
        return FALSE;
    for (const RECT &r : region->rects)
        fillRectDevice(dc, toDeviceRect(dc, r.left, r.top, r.right, r.bottom), brush, PATCOPY);
    system.pattern = 0;
    return TRUE;
}

int InvertRect(HDC handle, const RECT *rect)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return 0;
    fillRectDevice(dc, toDeviceRect(dc, rect->left, rect->top, rect->right, rect->bottom), 0,
                   DSTINVERT);
    return 1;
}

/* Device spans, filled with the brush through the ROP2 mode. */
static void fillWithBrush(DC *dc, const std::vector<RECT> &spans)
{
    Brush *brush = dc->brush;

    if (!brush || brush->null || !isDrawable(dc))
        return;
    std::vector<RECT> clips = clipRects(dc);
    for (RECT span : spans)
        for (RECT clip : clips) {
            RECT r = span;
            if (!intersect(r, clip))
                continue;
            for (LONG y = r.top; y < r.bottom; y++)
                for (LONG x = r.left; x < r.right; x++)
                    plot(dc, x + dc->origin.x, y + dc->origin.y, brushPixel(dc, brush, x, y), dc->rop2);
        }
    if (dc->surface->device)
        screenChanged();
}

/* Device spans in the pen's colour. */
static void fillWithPen(DC *dc, const std::vector<RECT> &spans)
{
    Pen *pen = dc->pen;

    if (!pen || pen->style == PS_NULL || !isDrawable(dc))
        return;
    uint8_t value = pixelFor(dc, pen->color);
    std::vector<RECT> clips = clipRects(dc);
    for (RECT span : spans)
        for (RECT clip : clips) {
            RECT r = span;
            if (!intersect(r, clip))
                continue;
            for (LONG y = r.top; y < r.bottom; y++)
                for (LONG x = r.left; x < r.right; x++)
                    plot(dc, x + dc->origin.x, y + dc->origin.y, value, dc->rop2);
        }
    if (dc->surface->device)
        screenChanged();
}

static int penWidth(DC *dc)
{
    if (!dc->pen || dc->pen->style == PS_NULL)
        return 0;
    LONG w = dc->pen->width, zero = 0, h = 0;
    toDevice(dc, w, h);
    toDevice(dc, zero, h);
    int width = abs((int)(w - zero));
    return width < 1 ? 1 : width;
}

BOOL Rectangle(HDC handle, int left, int top, int right, int bottom)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return FALSE;
    RECT r = toDeviceRect(dc, left, top, right, bottom);
    int w = penWidth(dc);
    if (!w) {
        fillWithBrush(dc, {{r.left, r.top, r.right - 1, r.bottom - 1}});
        return TRUE;
    }
    fillWithBrush(dc, {{r.left + w, r.top + w, r.right - w, r.bottom - w}});
    fillWithPen(dc, {{r.left, r.top, r.right, r.top + w},
                     {r.left, r.bottom - w, r.right, r.bottom},
                     {r.left, r.top + w, r.left + w, r.bottom - w},
                     {r.right - w, r.top + w, r.right, r.bottom - w}});
    return TRUE;
}

static std::vector<RECT> ellipseRows(RECT r)
{
    std::vector<RECT> rows;
    double a = (r.right - r.left) / 2.0, b = (r.bottom - r.top) / 2.0;
    double cx = r.left + a, cy = r.top + b;

    for (LONG y = r.top; y < r.bottom && a > 0 && b > 0; y++) {
        double dy = (y + 0.5 - cy) / b;
        if (dy * dy > 1)
            continue;
        double dx = a * sqrt(1 - dy * dy);
        LONG x1 = (LONG)lround(cx - dx), x2 = (LONG)lround(cx + dx);
        if (x2 > x1)
            rows.push_back({x1, y, x2, y + 1});
    }
    return rows;
}

BOOL Ellipse(HDC handle, int left, int top, int right, int bottom)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return FALSE;
    RECT r = toDeviceRect(dc, left, top, right, bottom);
    int w = penWidth(dc);
    RECT inner = {r.left + w, r.top + w, r.right - w, r.bottom - w};
    std::vector<RECT> outer = ellipseRows(r), inside = ellipseRows(inner);
    fillWithBrush(dc, w ? inside : outer);
    if (!w)
        return TRUE;
    /* The outline: each row of the outer ellipse less the inner one's. */
    std::vector<RECT> ring;
    for (const RECT &row : outer) {
        const RECT *in = 0;
        for (const RECT &i : inside)
            if (i.top == row.top)
                in = &i;
        if (!in)
            ring.push_back(row);
        else {
            ring.push_back({row.left, row.top, in->left, row.bottom});
            ring.push_back({in->right, row.top, row.right, row.bottom});
        }
    }
    fillWithPen(dc, ring);
    return TRUE;
}

/* A line from (x0, y0) up to (x1, y1), in device coordinates, in the pen. */
static void line(DC *dc, LONG x0, LONG y0, LONG x1, LONG y1)
{
    int w = penWidth(dc);
    if (!w)
        return;
    std::vector<RECT> dots;
    LONG dx = labs(x1 - x0), dy = -labs(y1 - y0);
    int stepX = x0 < x1 ? 1 : -1, stepY = y0 < y1 ? 1 : -1;
    LONG error = dx + dy;
    int half = (w - 1) / 2;
    while (x0 != x1 || y0 != y1) {
        dots.push_back({x0 - half, y0 - half, x0 - half + w, y0 - half + w});
        LONG e2 = 2 * error;
        if (e2 >= dy) {
            error += dy;
            x0 += stepX;
        }
        if (e2 <= dx) {
            error += dx;
            y0 += stepY;
        }
    }
    fillWithPen(dc, dots);
}

BOOL LineTo(HDC handle, int x, int y)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return FALSE;
    LONG x0 = dc->position.x, y0 = dc->position.y, x1 = x, y1 = y;
    toDevice(dc, x0, y0);
    toDevice(dc, x1, y1);
    line(dc, x0, y0, x1, y1);
    dc->position = {x, y};
    return TRUE;
}

BOOL Polygon(HDC handle, const POINT *points, int count)
{
    DC *dc = dcOf(handle);

    if (!dc || count < 2)
        return FALSE;
    std::vector<POINT> device(points, points + count);
    for (POINT &p : device)
        toDevice(dc, p.x, p.y);
    fillWithBrush(dc, polygonRows(device.data(), count, dc->polyFillMode));
    for (int i = 0; i < count; i++) {
        const POINT &p = device[i], &q = device[(i + 1) % count];
        line(dc, p.x, p.y, q.x, q.y);
    }
    return TRUE;
}

} /* namespace miniwin */
