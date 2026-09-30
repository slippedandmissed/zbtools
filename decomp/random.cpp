/*
 * random (0x415514-0x41559c): the game's random numbers (seeded from the time)
 */

#include "zoombinis.h"
#include "random.h"

/* @zoombi32 0x00415514 */
void seedRandom()
{
    time_t now;
    randomSeed = time(&now);
}

/* A random number from 0 to `limit`. */
/* @zoombi32 0x0041552a */
unsigned short randomUpTo(unsigned short limit)
{
    unsigned short value;

    if (seedPending) {
        seedRandom();
        seedPending = 0;
    }
    if (!limit)
        return 0;
    randomSeed = randomSeed * 0x343fd + 0x269ec3;
    value = randomSeed >> 16;
    value %= limit + 1;
    return value;
}

/* A random number below `limit`. */
/* @zoombi32 0x00415584 */
short randomBelow(short limit)
{
    return randomUpTo(limit - 1);
}
