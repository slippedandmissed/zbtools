/*
 * getinibool (Mohawk engine): reading settings as booleans and numbers; ptInRect
 */

/* @flags -p -x- */

#include <stdlib.h>
#include <string.h>
#include "zoombinis.h"

/*
 * Reads a setting as a boolean: true, yes, on or 1, or false, no, off or 0
 * (any case).
 *
 * Not exact: the original keeps `value` in ebx and the literals' base in esi
 * (see docs/src/concepts/compiler.md on the engine's register allocation).
 */
/* @zoombi32 0x004809f8 */
short getIniBool(fileSpec *file, const char *section, const char *key, short *value)
{
    char buffer[0x40];
    short error;

    error = getIniString(file, section, key, buffer, sizeof(buffer));
    if (!error || error == 0x296c) {
        if (!memicmp(buffer, "true", 4) || !memicmp(buffer, "yes", 3)
            || !memicmp(buffer, "on", 2) || !memicmp(buffer, "1", 1)) {
            *value = 1;
            return setIniError(0);
        }
        if (!memicmp(buffer, "false", 5) || !memicmp(buffer, "no", 2)
            || !memicmp(buffer, "off", 3) || !memicmp(buffer, "0", 1)) {
            *value = 0;
            return setIniError(0);
        }
        setIniError(0x296a);
    }
    return iniState.error;
}

/* Reads a setting as a number (decimal, or hex or octal as C writes them). */
/* @zoombi32 0x00480b0c */
short getIniLong(fileSpec *file, const char *section, const char *key, long *value)
{
    char *end;
    char buffer[0x40];
    short error;
    long number;

    error = getIniString(file, section, key, buffer, sizeof(buffer));
    if (!error || error == 0x296c) {
        number = strtol(buffer, &end, 0);
        if (buffer == end)
            setIniError(0x296a);
        else {
            *value = number;
            setIniError(0);
        }
    }
    return iniState.error;
}

/* @zoombi32 0x00480b6f */
short setIniError(short error)
{
    return iniState.error = error;
}

/* Whether a point is in a rectangle (QuickDraw's PtInRect). */
/* @zoombi32 0x00480b80 */
short ptInRect(ShortRect *rect, const Point &point)
{
    return point.x >= rect->left && point.x < rect->right && point.y >= rect->top
           && point.y < rect->bottom;
}
