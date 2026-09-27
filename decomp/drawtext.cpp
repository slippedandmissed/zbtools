/*
 * drawtext (Mohawk engine): drawText
 */

/* @flags -p -x- */

#include "zoombinis.h"

/*
 * Draws text in a rectangle of the current port, word-wrapped: flags 1
 * align it left (the default), 2 centre it and 4 right-align it; 0x10 put it
 * at the top (the default), 0x20 centre it vertically and 0x40 put it at the
 * bottom. In pen modes other than copying, the text is drawn white
 * on black into a bitmap first, which is then drawn with the mode's raster
 * operation.
 */
/* @zoombi32 0x0048aed8 */
short drawText(const Rect &rect, unsigned short flags, const char *text, unsigned short length)
{
    basePort *port;
    unsigned short oldAlign;
    unsigned short format;
    unsigned short height;
    unsigned short textHeight;

    if (!(flags & 7))
        flags |= 1;
    if (!(flags & 0x70))
        flags |= 0x10;
    if ((port = portObject(8)) == 0)
        return graphics.error;
    oldAlign = SetTextAlign(port->dc, TA_LEFT | TA_TOP);
    format = (flags & 4 ? DT_RIGHT : flags & 2 ? DT_CENTER : DT_LEFT) | DT_WORDBREAK | DT_NOCLIP
             | DT_EXTERNALLEADING | DT_NOPREFIX;
    WinRect bounds(rect);
    if (flags & 0x20 || flags & 0x40) {
        height = bounds.bottom - bounds.top;
        textHeight = DrawText(port->dc, text, length, &bounds, format | DT_CALCRECT);
        WinRect again(rect);

        bounds = again;
        bounds.top = flags & 0x40 ? bounds.bottom - textHeight
                                  : height / 2 + bounds.top - textHeight / 2;
        bounds.bottom = bounds.top + height;
    }
    port->prepare();
    HDC target;
    HBITMAP bitmap;
    HDC memory;
    RECT area;
    if (!port->pen.mode) {
        target = port->dc;
        memory = 0;
    } else {
        WinRect converted(rect);

        area = converted;
        RECT device = area;
        LPtoDP(port->dc, (POINT *)&device, 2);
        if ((bitmap = CreateCompatibleBitmap(port->dc, device.right - device.left,
                                             device.bottom - device.top)) == 0)
            return setPortError(0x2a37);
        if ((memory = CreateCompatibleDC(port->dc)) == 0) {
            DeleteObject(bitmap);
            return setPortError(0x2a37);
        }
        bitmap = (HBITMAP)SelectObject(memory, bitmap);
        SetMapMode(memory, MM_ANISOTROPIC);
        SetViewportExtEx(memory, device.right - device.left, device.bottom - device.top, 0);
        SetWindowExtEx(memory, area.right - area.left, area.bottom - area.top, 0);
        SetWindowOrgEx(memory, area.left, area.top, 0);
        SetBkMode(memory, TRANSPARENT);
        SetTextAlign(memory, TA_LEFT | TA_TOP);
        SetTextColor(memory, RGB(0xff, 0xff, 0xff));
        SelectObject(memory, port->hfont);
        FillRect(memory, &area, (HBRUSH)GetStockObject(BLACK_BRUSH));
        target = memory;
    }
    IntersectClipRect(target, rect.left, rect.top, rect.right, rect.bottom);
    DrawText(target, text, length, &bounds, format);
    if (!memory) {
        port->clipApplied = 0;
        SetTextAlign(port->dc, oldAlign);
    } else {
        BitBlt(port->dc, area.left, area.top, area.right - area.left, area.bottom - area.top,
               memory, area.left, area.top, copyRops[port->pen.mode]);
        bitmap = (HBITMAP)SelectObject(memory, bitmap);
        DeleteObject(bitmap);
        DeleteDC(memory);
    }
    return setPortError(0);
}
