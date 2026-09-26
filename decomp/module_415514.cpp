/*
 * module_415514 (0x415514-0x41559c): no strings; calls time()
 */

#include "zoombinis.h"

/* @zoombi32 0x00415514 */
void fn_415514()
{
    time_t now;
    g_4a07b8 = time(&now);
}
