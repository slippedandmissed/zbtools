/*
 * snoids (0x456c00-0x45c0f4): 'Too many snoid NORMAL scripts', 'Zoombini.MHK', name syllables
 */

#include "zoombinis.h"

/* @zoombi32 0x004572bf */
short fn_4572bf()
{
    int count = 0;
    for (short i = 0; i < *(short *)(g_4a4ba0 + 0xa92e); i++)
        if (*(g_4a4ba0 + 0xa93c + i * 0x13) != 0)
            count++;
    return count;
}

/* @zoombi32 0x00457fbb */
short fn_457fbb()
{
    if (g_4b7b3a)
        return g_4b7b38 + 1;
    return 0;
}

/* @zoombi32 0x0045b39a */
void fn_45b39a(short value)
{
    g_4a4ce6 = value & 3;
}

/* @zoombi32 0x0045bfc0 */
void fn_45bfc0(long value)
{
    g_4b7b68 = value;
}
