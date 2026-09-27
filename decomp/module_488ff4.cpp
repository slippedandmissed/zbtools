/*
 * module_488ff4 (Mohawk engine): disposeFont
 */

/* @flags -p -x- */

#include <stdlib.h>
#include "zoombinis.h"

/* Disposes of a font nothing uses. Not exact: the original keeps `font` in eax (no call intervenes where it is
   used); BCC32 4.5 gives it ebx. */
/* @zoombi32 0x00488ff4 */
short disposeFont(Font *handle)
{
    Font *font;

    if ((font = fontObject(handle)) == 0)
        return setPortError(0x2a67);
    if (font->users > 0)
        return setPortError(0x2a66);
    if (font == font->next)
        graphics.fonts = 0;
    else {
        font->next->prev = font->prev;
        font->prev->next = font->next;
        if (font == graphics.fonts)
            graphics.fonts = font->next;
    }
    font->magic = 0;
    free(font);
    return setPortError(0);
}
