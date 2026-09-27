/*
 * skipstrings (0x41559c-0x4155d0): skipping strings
 */

#include <string.h>
#include "zoombinis.h"

/* Skips `count` NUL-terminated strings. */
/* @zoombi32 0x0041559c */
char *skipStrings(char *text, short count)
{
    while (count) {
        text = (char *)memchr(text, 0, 0xffff) + 1;
        count--;
    }
    return text;
}
