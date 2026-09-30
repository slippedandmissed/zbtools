/*
 * os_fixed's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_FIXED_H
#define OS_FIXED_H

/* Fixed-point (16.16) arithmetic */
long fixedDiv(long a, long b); /* 0x46d754 */
long fixedMul(long a, long b); /* 0x46d7aa */

#endif
