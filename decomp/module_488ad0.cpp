/*
 * module_488ad0 (Mohawk engine): newFont
 */

/* @flags -p -x- */

#include <stdlib.h>
#include <string.h>
#include "zoombinis.h"
#include "game.h"

/* A new font ("SYSTEM" for no name or "default"), in graphics.fonts. */
/* @zoombi32 0x00488ad0 */
Font *newFont(const char *name, unsigned short size, unsigned short style)
{
    Font *font;
    unsigned short length;

    if (!name || !*name || !stricmp(name, "default"))
        name = "SYSTEM";
    length = strlen(name);
    if (length > 31)
        length = 31;
    if ((font = (Font *)malloc(length + 0x39)) == 0) {
        setPortError(0x2a37);
        return 0;
    }
    memset(font, 0, 0x38);
    font->magic = 0x466f6e74L; /* 'Font' */
    memcpy(font->name, name, length);
    font->size = size;
    font->style = style;
    if ((font->next = graphics.fonts) != 0) {
        font->prev = graphics.fonts->prev;
        font->prev->next = font;
        font->next->prev = font;
    } else
        font->next = font->prev = font;
    graphics.fonts = font;
    setPortError(0);
    return fontHandle(font);
}
