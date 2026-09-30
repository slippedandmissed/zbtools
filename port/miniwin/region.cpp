/*
 * Regions and clipping. A region is a list of rectangles in bands: sorted
 * by top then left, none overlapping, each band's rectangles sharing their
 * top and bottom, and bands that would be alike merged. Combining two goes
 * band by band over every edge either has.
 */

#include <math.h>
#include <stdlib.h>

#include <algorithm>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

void trackObject(GdiObject *object);

RECT Region::bounds() const
{
    if (rects.empty())
        return {0, 0, 0, 0};
    RECT b = rects[0];
    for (const RECT &r : rects) {
        b.left = std::min(b.left, r.left);
        b.top = std::min(b.top, r.top);
        b.right = std::max(b.right, r.right);
        b.bottom = std::max(b.bottom, r.bottom);
    }
    return b;
}

int Region::complexity() const
{
    return rects.empty() ? NULLREGION : rects.size() == 1 ? SIMPLEREGION : COMPLEXREGION;
}

typedef std::vector<std::pair<LONG, LONG>> Spans;

/* The x-spans of a region's rectangles covering the band [top, bottom). */
static Spans spansIn(const std::vector<RECT> &rects, LONG top, LONG bottom)
{
    Spans spans;
    for (const RECT &r : rects)
        if (r.top <= top && r.bottom >= bottom)
            spans.push_back({r.left, r.right});
    std::sort(spans.begin(), spans.end());
    /* Join touching or overlapping spans (the input may not be banded). */
    Spans joined;
    for (const auto &s : spans)
        if (!joined.empty() && s.first <= joined.back().second)
            joined.back().second = std::max(joined.back().second, s.second);
        else
            joined.push_back(s);
    return joined;
}

static bool inside(const Spans &spans, LONG x)
{
    for (const auto &s : spans)
        if (x >= s.first && x < s.second)
            return true;
    return false;
}

static bool apply(int mode, bool a, bool b)
{
    switch (mode) {
    case RGN_AND:
        return a && b;
    case RGN_OR:
        return a || b;
    case RGN_XOR:
        return a != b;
    case RGN_DIFF:
        return a && !b;
    }
    return a;
}

static Spans combineSpans(const Spans &a, const Spans &b, int mode)
{
    std::vector<LONG> edges;
    for (const auto &s : a) {
        edges.push_back(s.first);
        edges.push_back(s.second);
    }
    for (const auto &s : b) {
        edges.push_back(s.first);
        edges.push_back(s.second);
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    Spans result;
    for (size_t i = 0; i + 1 < edges.size(); i++) {
        LONG x = edges[i];
        if (!apply(mode, inside(a, x), inside(b, x)))
            continue;
        if (!result.empty() && result.back().second == x)
            result.back().second = edges[i + 1];
        else
            result.push_back({x, edges[i + 1]});
    }
    return result;
}

static std::vector<RECT> combine(const std::vector<RECT> &a, const std::vector<RECT> &b, int mode)
{
    std::vector<LONG> ys;
    for (const RECT &r : a) {
        ys.push_back(r.top);
        ys.push_back(r.bottom);
    }
    for (const RECT &r : b) {
        ys.push_back(r.top);
        ys.push_back(r.bottom);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    std::vector<RECT> result;
    Spans previous;
    size_t previousStart = 0;
    LONG previousBottom = 0;
    for (size_t i = 0; i + 1 < ys.size(); i++) {
        LONG top = ys[i], bottom = ys[i + 1];
        Spans spans = combineSpans(spansIn(a, top, bottom), spansIn(b, top, bottom), mode);
        if (spans.empty())
            continue;
        if (!previous.empty() && previousBottom == top && spans == previous) {
            for (size_t j = previousStart; j < result.size(); j++)
                result[j].bottom = bottom;
        } else {
            previousStart = result.size();
            for (const auto &s : spans)
                result.push_back({s.first, top, s.second, bottom});
            previous = spans;
        }
        previousBottom = bottom;
    }
    return result;
}

static HRGN newRegion(const std::vector<RECT> &rects)
{
    Region *region = new Region;
    region->rects = combine(rects, std::vector<RECT>(), RGN_OR);
    trackObject(region);
    return (HRGN)region;
}

static RECT normalized(int left, int top, int right, int bottom)
{
    RECT r = {std::min(left, right), std::min(top, bottom), std::max(left, right),
              std::max(top, bottom)};
    return r;
}

HRGN CreateRectRgn(int left, int top, int right, int bottom)
{
    RECT r = normalized(left, top, right, bottom);
    std::vector<RECT> rects;
    if (r.right > r.left && r.bottom > r.top)
        rects.push_back(r);
    return newRegion(rects);
}

BOOL SetRectRgn(HRGN rgn, int left, int top, int right, int bottom)
{
    Region *region = regionOf(rgn);
    if (!region)
        return FALSE;
    RECT r = normalized(left, top, right, bottom);
    region->rects.clear();
    if (r.right > r.left && r.bottom > r.top)
        region->rects.push_back(r);
    return TRUE;
}

HRGN CreateEllipticRgn(int left, int top, int right, int bottom)
{
    RECT r = normalized(left, top, right, bottom);
    std::vector<RECT> rows;
    double a = (r.right - r.left) / 2.0, b = (r.bottom - r.top) / 2.0;
    double cx = r.left + a, cy = r.top + b;

    for (LONG y = r.top; y < r.bottom && b > 0; y++) {
        double dy = (y + 0.5 - cy) / b;
        if (dy * dy > 1)
            continue;
        double dx = a * sqrt(1 - dy * dy);
        LONG x1 = (LONG)lround(cx - dx), x2 = (LONG)lround(cx + dx);
        if (x2 > x1)
            rows.push_back({x1, y, x2, y + 1});
    }
    return newRegion(rows);
}

/* Pixels whose centres the polygon covers, by the fill mode's rule. */
std::vector<RECT> polygonRows(const POINT *points, int count, int mode)
{
    std::vector<RECT> rows;
    if (count < 3)
        return rows;
    LONG top = points[0].y, bottom = points[0].y;
    for (int i = 1; i < count; i++) {
        top = std::min(top, points[i].y);
        bottom = std::max(bottom, points[i].y);
    }
    for (LONG y = top; y < bottom; y++) {
        double yc = y + 0.5;
        std::vector<std::pair<double, int>> crossings;
        for (int i = 0; i < count; i++) {
            POINT p = points[i], q = points[(i + 1) % count];
            if (p.y == q.y)
                continue;
            int direction = q.y > p.y ? 1 : -1;
            if ((yc >= p.y && yc < q.y) || (yc >= q.y && yc < p.y))
                crossings.push_back({p.x + (yc - p.y) * (q.x - p.x) / (double)(q.y - p.y),
                                     direction});
        }
        std::sort(crossings.begin(), crossings.end());
        int winding = 0;
        for (size_t i = 0; i + 1 < crossings.size(); i++) {
            winding += crossings[i].second;
            bool in = mode == WINDING ? winding != 0 : (i % 2) == 0;
            if (!in)
                continue;
            LONG x1 = (LONG)ceil(crossings[i].first - 0.5);
            LONG x2 = (LONG)ceil(crossings[i + 1].first - 0.5);
            if (x2 > x1)
                rows.push_back({x1, y, x2, y + 1});
        }
    }
    return rows;
}

HRGN CreatePolygonRgn(const POINT *points, int count, int mode)
{
    return newRegion(polygonRows(points, count, mode));
}

int CombineRgn(HRGN target, HRGN first, HRGN second, int mode)
{
    Region *to = regionOf(target);
    Region *a = regionOf(first);
    Region *b = regionOf(second);

    if (!to || !a || (mode != RGN_COPY && !b))
        return ERROR;
    if (mode == RGN_COPY)
        to->rects = a->rects;
    else
        to->rects = combine(a->rects, b->rects, mode);
    return to->complexity();
}

/* Clipping, in device coordinates. */

int SelectClipRgn(HDC handle, HRGN rgn)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return ERROR;
    Region *region = regionOf(rgn);
    delete dc->clip;
    dc->clip = 0;
    if (!region)
        return SIMPLEREGION;
    dc->clip = new Region;
    dc->clip->rects = region->rects;
    return dc->clip->complexity();
}

int IntersectClipRect(HDC handle, int left, int top, int right, int bottom)
{
    DC *dc = dcOf(handle);

    if (!dc)
        return ERROR;
    RECT rect = toDeviceRect(dc, left, top, right, bottom);
    std::vector<RECT> clip;
    if (dc->clip)
        clip = dc->clip->rects;
    else
        clip.push_back(dc->visible);
    std::vector<RECT> with(1, rect);
    if (!dc->clip)
        dc->clip = new Region;
    dc->clip->rects = combine(clip, with, RGN_AND);
    return dc->clip->complexity();
}

} /* namespace miniwin */
