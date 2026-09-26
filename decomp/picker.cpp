/*
 * picker (0x42f920-0x433510): 'Picker.MHK', 'New Game', 'Snoids to practice with = '
 */

#include "zoombinis.h"

/* @zoombi32 0x0042fc89 */
void fn_42fc89(Counters *object)
{
    Triple *counters = &object->counters;
    if (object->mode == 2) {
        counters->a--;
        counters->b++;
        counters->c += 2;
    }
}

/* @zoombi32 0x004320da */
long fn_4320da(long)
{
    return 0;
}

/* @zoombi32 0x004334f0 */
void fn_4334f0(long, short value)
{
    if (value == -1 && g_4afb90 < 0)
        g_4afb90 = -g_4afb90;
}
