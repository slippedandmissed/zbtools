/*
 * module_48d480 (Mohawk engine): setDisplayMode, setFont, font handles
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Switches to the display mode that best suits `mode`, if there is one. */
/* @zoombi32 0x0048d480 */
short setDisplayMode(const DisplayMode *mode)
{
    DeviceMode found;

    if (!canUseDisplayMode((DisplayMode *)mode, 1))
        return 0;
    findDisplayMode(mode, &found);
    changeDisplaySettings(&found, 0);
    return 1;
}

/* Sets the current port's font (0: the default one); the previous one, or
   -1 on error. */
/* @zoombi32 0x0048d4c4 */
Font *setFont(Font *handle)
{
    basePort *port;
    Font *font;
    Font *old;

    if ((port = portObject(1)) == 0)
        return (Font *)-1;
    font = handle ? fontObject(handle) : graphics.defaultFont;
    if (!font) {
        setPortError(0x2a67);
        return (Font *)-1;
    }
    if (font != port->font && port->useFont(font))
        return (Font *)-1;
    old = port->font;
    old->users--;
    port->font = font;
    font->users++;
    setPortError(0);
    return fontHandle(old == graphics.defaultFont ? 0 : old);
}

/* @zoombi32 0x0048d54b */
Font *fontHandle(Font *font)
{
    return font;
}

/* The font behind a handle, or 0 if it isn't one. */
/* @zoombi32 0x0048d555 */
Font *fontObject(Font *font)
{
    if (!font || font == (Font *)-1 || font->magic != 0x466f6e74L /* 'Font' */)
        return 0;
    return font;
}
