/*
 * loading (0x414f30-0x415514): 'Unable to load ', 'Not enough memory for ', 'Unable to allocate port for '
 */

#include "zoombinis.h"

/* @zoombi32 0x004153b0 */
void fn_4153b0(Callback callback)
{
    g_4a07ac = callback;
}

/* @zoombi32 0x004153bf */
void fn_4153bf(long value)
{
    g_4a07b0 = value;
}

/* @zoombi32 0x004153ce */
void fn_4153ce(const char *message)
{
    g_4a07b4 = message;
}
