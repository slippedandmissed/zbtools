/*
 * graphics's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

extern char msgUnableToLockPort[]; /* @data 0x4a0710 */
extern PALETTEENTRY colors[256]; /* @data 0x4aafee: the palette's colours */
extern PALETTEENTRY *g_4ab3f0;
extern char *g_4ab3f4;
extern char *g_4ab3f8;
extern char *g_4ab3fc;
extern char *g_4ab400;
extern short g_4ab404; /* displayMode.unknown8 */
void fn_4144d0(DisplayMode *mode, long);
/* graphics */
void initGraphics(DisplayMode *mode, short depth);
void closeGraphics();
void drawImage(ResourceList *images, short index, short x, short y, short mode, short anchor);
void drawImageInColor(ResourceList *images, short index, short x, short y, short mode, short color,
                      short anchor);
void getColors(PALETTEENTRY *to, short first, short count);
void setColors(PALETTEENTRY *from, short first, short count);
void fn_4148da(short first, short count);
void createPort(basePort **port, ShortRect *bounds, short keep, const char *name);
void destroyPort(basePort **port, short release);
void fn_414a2e(basePort *port, ShortRect *bounds);
void saveRect(MapSave **save, ShortRect *rect, short locked, const char *name);
void restoreRect(MapSave **save, short free);
void freeSave(MapSave **save);
void clipRect(short *region, ShortRect *rect, short keep);
void fn_414c25(short *region, short free);
void getClipRegion(short *region, short create);
void createRegion(short *region);
void freeRegion(short *region);
void copyBits(basePort *to, basePort *from, ShortRect *rect);
void showRect(ShortRect *rect);
void lockPortOrFail(basePort *port);
void lockSave(MapSave *save);
void unlockSave(MapSave *save);
void alignRect(ShortRect *rect, short x, short y, short how);
void fn_414e7d();
void redrawRect(ShortRect *rect);
void fn_414f01(InputItem *item);
void fn_414f17(InputItem *item);

#endif
