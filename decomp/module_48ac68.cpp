/*
 * module_48ac68 (Mohawk engine): initDisplayMode
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048ac68 */
void __cdecl initDisplayMode(DisplayMode *mode, unsigned short width, unsigned short height, unsigned long colors,
                             short palettized)
{
    mode->width = width;
    mode->height = height;
    mode->colors = colors;
    mode->palettized = palettized;
}
