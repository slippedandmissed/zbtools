/*
 * getinistring (Mohawk engine): reading a setting as text
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * Reads a setting as text into `buffer` (`size` bytes), as Windows'
 * GetPrivateProfileString: without a section, the names of all the sections,
 * and without a key, the names of all the section's keys (each ending with a
 * NUL, and a NUL after the last). Surrounding spaces, and quotes around the
 * value, are dropped.
 *
 * Not exact: the original shares registers differently (handle and ini in
 * esi, section, n and end in ebx).
 */
/* @zoombi32 0x00480790 */
short getIniString(fileSpec *file, const char *section, const char *key, char *buffer,
                   unsigned short size)
{
    unsigned short length;
    short handle;
    IniFile *ini;
    char *text;
    unsigned short n;
    char *end;
    short error;

    *buffer = 0;
    if ((handle = openIni(file)) == 0)
        return iniState.error;
    ini = (IniFile *)handleData(handle);
    text = (char *)handleData(ini->text);
    if (!section) {
        ini->pos = n = 0;
        while ((length = nextSection(ini, text)) != 0) {
            if (n + length + 2 > size)
                return setIniError(0x296c);
            memcpy(buffer + n, text + ini->pos, length);
            n += length;
            buffer[n] = 0;
            n++;
            buffer[n] = 0;
            ini->pos = nextLine(ini, text);
        }
        return setIniError(0);
    }
    if (!key) {
        if (!findSection(ini, text, section))
            return setIniError(0x296b);
        n = 0;
        while ((length = nextKey(ini, text)) != 0) {
            if (n + length + 2 > size)
                return setIniError(0x296c);
            memcpy(buffer + n, text + ini->pos, length);
            n += length;
            buffer[n] = 0;
            n++;
            buffer[n] = 0;
            ini->pos = nextLine(ini, text);
        }
        return setIniError(0);
    }
    if (!findKey(ini, text, section, key))
        return setIniError(0x296b);
    ini->pos = skipSpaces(ini, text);
    text += ini->pos;
    end = (char *)memchr(text, '\r', ini->size - ini->pos);
    if (!end)
        end = strchr(text, 0);
    while (text < end && isSpace(end[-1]))
        end--;
    if (end >= text + 2 && (*text == '\'' || *text == '"') && *text == end[-1]) {
        text++;
        end--;
    }
    length = end - text;
    if (length >= size) {
        length = size - 1;
        error = 0x296c;
    } else
        error = 0;
    memcpy(buffer, text, length);
    buffer[length] = 0;
    return setIniError(error);
}

/* @zoombi32 0x004809d3 */
short isSpace(char c)
{
    return c == ' ' || c == '\t';
}
