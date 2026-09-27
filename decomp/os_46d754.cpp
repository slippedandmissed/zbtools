/*
 * os_46d754 (0x46d754-0x46d7f8): the start of the Mohawk OS layer: 16.16
 * fixed-point division and multiplication
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"

/* The result of a division by zero or an overflow. */
#define FIXED_ERROR ((long)0x80000000L)

/*
 * a / b in 16.16 fixed point (0x80000000 when b is 0 or the quotient
 * overflows). The original is in assembly (`div` twice, `shld`); this does
 * the same with 32-bit arithmetic, dividing the remainder bit by bit.
 */
/* @zoombi32-functional 0x0046d754 */
long fixedDiv(long a, long b)
{
    unsigned long dividend;
    unsigned long divisor;
    unsigned long quotient;
    unsigned long remainder;
    int negative = 0;
    int i;

    if (!b)
        return FIXED_ERROR;
    divisor = b;
    if (b < 0) {
        negative = !negative;
        divisor = -b;
    }
    dividend = a;
    if (a < 0) {
        negative = !negative;
        dividend = -a;
    }
    if ((quotient = dividend / divisor) >= 0x10000)
        return FIXED_ERROR;
    remainder = dividend % divisor;
    for (i = 0; i < 16; i++) {
        remainder <<= 1; /* remainder < divisor <= 2^31: no overflow */
        quotient <<= 1;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= 1;
        }
    }
    return negative ? -(long)quotient : (long)quotient;
}

/*
 * a * b in 16.16 fixed point (0x80000000 when the product overflows). The
 * original is in assembly (a 64-bit `mul`, `shrd`); this multiplies the
 * 16-bit halves.
 */
/* @zoombi32-functional 0x0046d7aa */
long fixedMul(long a, long b)
{
    unsigned long x = a < 0 ? -a : a;
    unsigned long y = b < 0 ? -b : b;
    int negative = (a < 0) != (b < 0);
    unsigned long high = (x >> 16) * (y >> 16);
    unsigned long result;
    unsigned long part;

    if (high >= 0x10000)
        return FIXED_ERROR;
    result = high << 16;
    part = (x >> 16) * (y & 0xffff);
    if ((result += part) < part)
        return FIXED_ERROR;
    part = (x & 0xffff) * (y >> 16);
    if ((result += part) < part)
        return FIXED_ERROR;
    part = ((x & 0xffff) * (y & 0xffff)) >> 16;
    if ((result += part) < part)
        return FIXED_ERROR;
    return negative ? -(long)result : (long)result;
}
