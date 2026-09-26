/*
 * module_433510 (0x433510-0x439560): 'Picker.MHK'
 */

#include "zoombinis.h"

/* @zoombi32 0x0043595f */
void fn_43595f(long)
{
}

/* @zoombi32 0x00435966 */
void fn_435966(long, long)
{
}

/*
 * Only an unsigned constant (or `-=` on a local) gives the original's
 * `sub eax, 50`; a signed one becomes `add eax, -50`. Perhaps a sizeof or an
 * unsigned #define.
 */
/* @zoombi32 0x0043691d */
int fn_43691d(long, short value)
{
    return value - 50u;
}

/* The index (1-20) of the largest value, ignoring `exclude`. */
/* @zoombi32 0x00437390 */
short indexOfLargestExcept(short exclude)
{
    short best, bestValue, i;
    for (i = 1, best = 0, bestValue = 0; i < 0x15; i++) {
        if (g_4aff9a[i] > bestValue && exclude != i) {
            bestValue = g_4aff9a[i];
            best = i;
        }
    }
    return best;
}

/* @zoombi32 0x00437acb */
short fn_437acb(short i)
{
    return g_4aff9a[i];
}

/* How many of g_4aff9a[1..20] are non-zero. */
/* @zoombi32 0x004381bb */
short fn_4381bb()
{
    short i, count;
    for (i = 1, count = 0; i < 0x15; i++)
        if (g_4aff9a[i])
            count++;
    return count;
}
