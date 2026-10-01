/*
 * The Borland C++ runtime's extras the game uses (miniwin/borland.h): number
 * formatting, case-blind comparisons, and dir.h and dos.h's current drive
 * and directory (miniwin's, which are Win32's) and the date and time.
 */

#include <ctype.h>
#include <stdarg.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "miniwin/internal.h"
#include <miniwin/borland.h>

namespace miniwin {

static char *unsignedToText(unsigned long value, char *buffer, int radix, bool negative)
{
    char digits[40];
    int n = 0;

    do {
        int digit = (int)(value % radix);
        digits[n++] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
        value /= radix;
    } while (value);
    char *out = buffer;
    if (negative)
        *out++ = '-';
    while (n)
        *out++ = digits[--n];
    *out = 0;
    return buffer;
}

#ifndef _WIN32 /* (Windows' C library has them all) */
char *itoa(int value, char *buffer, int radix)
{
    return ltoa(value, buffer, radix);
}

char *ltoa(long value, char *buffer, int radix)
{
    if (radix == 10 && value < 0)
        return unsignedToText(0 - (unsigned long)value, buffer, radix, true);
    return unsignedToText((unsigned long)value, buffer, radix, false);
}

#endif

char *ultoa(unsigned long value, char *buffer, int radix)
{
    return unsignedToText(value, buffer, radix, false);
}

#ifndef _WIN32

int strnicmp(const char *a, const char *b, size_t n)
{
    for (; n; n--, a++, b++) {
        int ca = tolower((unsigned char)*a), cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        if (!ca)
            break;
    }
    return 0;
}

int stricmp(const char *a, const char *b)
{
    return strnicmp(a, b, (size_t)-1);
}

int memicmp(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = (const unsigned char *)a, *pb = (const unsigned char *)b;

    for (; n; n--, pa++, pb++) {
        int ca = tolower(*pa), cb = tolower(*pb);
        if (ca != cb)
            return ca - cb;
    }
    return 0;
}

char *strupr(char *s)
{
    for (char *p = s; *p; p++)
        *p = (char)toupper((unsigned char)*p);
    return s;
}

char *strlwr(char *s)
{
    for (char *p = s; *p; p++)
        *p = (char)tolower((unsigned char)*p);
    return s;
}
#endif

void gettime(struct ::time *now)
{
    SYSTEMTIME local;

    GetLocalTime(&local);
    now->ti_hour = (unsigned char)local.wHour;
    now->ti_min = (unsigned char)local.wMinute;
    now->ti_sec = (unsigned char)local.wSecond;
    now->ti_hund = (unsigned char)(local.wMilliseconds / 10);
}

void _dos_getdate(struct ::dosdate_t *date)
{
    SYSTEMTIME local;

    GetLocalTime(&local);
    date->year = local.wYear;
    date->month = (unsigned char)local.wMonth;
    date->day = (unsigned char)local.wDay;
    date->dayofweek = (unsigned char)local.wDayOfWeek;
}

/* The current drive, 0 for A:. */
int getdisk()
{
    char path[MAX_PATH];

    GetCurrentDirectory(sizeof path, path);
    return toupper((unsigned char)path[0]) - 'A';
}

/* Makes `drive` current (its root, as miniwin keeps one directory for all
   drives); the number of drives. */
int setdisk(int drive)
{
    char root[4] = {(char)('A' + drive), ':', '\\', 0};

    if (getdisk() != drive)
        SetCurrentDirectory(root);
    return 26;
}

char *getcwd(char *buffer, int size)
{
    if (!buffer && (buffer = (char *)malloc(size)) == 0)
        return 0;
    DWORD length = GetCurrentDirectory(size, buffer);
    return length && (int)length < size ? buffer : 0;
}

int chdir(const char *path)
{
    return SetCurrentDirectory(path) ? 0 : -1;
}

FILE *fopen(const char *path, const char *mode)
{
    std::string host;

    if (!hostPath(path, host))
        return 0;
    return ::fopen(host.c_str(), mode);
}

/* A format with Borland's long taken as an int: the `l` before an integer
   conversion goes. */
static std::string intFormat(const char *format)
{
    std::string out;
    for (const char *p = format; *p; p++) {
        out += *p;
        if (*p != '%')
            continue;
        if (p[1] == '%') {
            out += *++p;
            continue;
        }
        for (p++; *p && !isalpha((unsigned char)*p) && *p != '['; p++)
            out += *p;
        if (*p == 'l' && p[1] && strchr("diouxX", p[1]))
            p++;
        if (!*p)
            break;
        out += *p;
        if (*p == '[') { /* a scan set, to its closing bracket */
            if (p[1] == '^')
                out += *++p;
            if (p[1] == ']')
                out += *++p;
            while (p[1] && p[1] != ']')
                out += *++p;
            if (p[1])
                out += *++p;
        }
    }
    return out;
}

int bcVsprintf(char *buffer, const char *format, va_list args)
{
    return vsprintf(buffer, intFormat(format).c_str(), args);
}

int bcSprintf(char *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = bcVsprintf(buffer, format, args);
    va_end(args);
    return n;
}

int bcFprintf(FILE *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vfprintf(file, intFormat(format).c_str(), args);
    va_end(args);
    return n;
}

int bcPrintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vprintf(intFormat(format).c_str(), args);
    va_end(args);
    return n;
}

int bcSscanf(const char *text, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsscanf(text, intFormat(format).c_str(), args);
    va_end(args);
    return n;
}

} /* namespace miniwin */
