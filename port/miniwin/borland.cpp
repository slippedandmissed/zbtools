/*
 * The Borland C++ runtime's extras the game uses (miniwin/borland.h): number
 * formatting, case-blind comparisons, and dir.h and dos.h's current drive
 * and directory (miniwin's, which are Win32's) and the date and time.
 */

#include <ctype.h>
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

char *ultoa(unsigned long value, char *buffer, int radix)
{
    return unsignedToText(value, buffer, radix, false);
}

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

} /* namespace miniwin */
