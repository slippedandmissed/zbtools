/*
 * The parts of Borland C++'s runtime the game uses that standard C++ lacks
 * (dir.h, dos.h, stdlib.h and string.h extensions). Paths go through
 * miniwin's drives (files.cpp), like the Win32 file functions.
 */

#ifndef MINIWIN_BORLAND_H
#define MINIWIN_BORLAND_H

#include <stdarg.h>
#include <stdio.h>

/* Borland declares these structures globally, where the C library's time()
   can share the name. */
struct time
{
    unsigned char ti_min;
    unsigned char ti_hour;
    unsigned char ti_hund;
    unsigned char ti_sec;
};

struct dosdate_t
{
    unsigned char day;
    unsigned char month;
    unsigned int year;
    unsigned char dayofweek;
};

namespace miniwin {

void gettime(struct time *now);
void _dos_getdate(struct dosdate_t *date);
int getdisk();
int setdisk(int drive);
char *getcwd(char *buffer, int size);
int chdir(const char *path);
FILE *fopen(const char *path, const char *mode);

#ifndef _WIN32 /* (Windows' C library has them all) */
char *itoa(int value, char *buffer, int radix);
char *ltoa(long value, char *buffer, int radix);
#endif
char *ultoa(unsigned long value, char *buffer, int radix);
#ifndef _WIN32
int stricmp(const char *a, const char *b);
int strnicmp(const char *a, const char *b, size_t n);
int memicmp(const void *a, const void *b, size_t n);
char *strupr(char *s);
char *strlwr(char *s);
#endif

/* The text functions with Borland's long (32 bits) in their formats: where
   long is 64 bits, `%ld` is `%d` (prelude.h redirects the game's calls here). */
int bcSprintf(char *buffer, const char *format, ...);
int bcVsprintf(char *buffer, const char *format, va_list args);
int bcFprintf(FILE *file, const char *format, ...);
int bcPrintf(const char *format, ...);
int bcSscanf(const char *text, const char *format, ...);

} /* namespace miniwin */

using namespace miniwin;

#endif
