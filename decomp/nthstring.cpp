/*
 * nthstring (0x4155d0-0x415604): one function
 */

#include "zoombinis.h"
#include "nthstring.h"
#include "skipstrings.h"

/*
 * String `n` (from 1) of a table: a count, then that many NUL-terminated
 * strings.
 *
 * Not exact: the original keeps table and n in esi and ebx (saved registers)
 * and computes n - 1 in 16 bits; this compiles to eax and edx.
 */
/* @zoombi32 0x004155d0 */
char *nthString(char *table, unsigned char n)
{
    if (table && n && n <= (unsigned char)*table)
        return skipStrings(table + 1, n - 1);
    return 0;
}
