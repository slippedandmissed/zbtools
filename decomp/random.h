/*
 * random's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef RANDOM_H
#define RANDOM_H

extern unsigned long randomSeed; /* @data 0x4a07b8 */
extern short seedPending; /* @data 0x4a07bc: seed from the time first */
short randomBelow(short limit);
unsigned short randomUpTo(unsigned short limit);
void seedRandom();

#endif
