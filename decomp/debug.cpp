/*
 * debug (0x415604-0x415a30): 'generic breakpoint', 'System starvation warning!', 'e2GetPoolValue Error'
 */

#include "zoombinis.h"

/* @zoombi32 0x00415604 */
void fn_415604(Callback callback)
{
    g_4a07c4 = callback;
}

/* Returns whether either flag was set, and clears both. */
/* @zoombi32 0x004157f3 */
short fn_4157f3()
{
    short either = g_4ab49c | g_4ab49e;
    g_4ab49c = g_4ab49e = 0;
    return either;
}

/* @zoombi32 0x00415811 */
void fn_415811()
{
    g_4ab49e = 1;
}

/*
 * The main loop's other half (see mainLoopUpdate): handles a waiting
 * message, calls the game's registered callback (g_4a07c4, set by
 * fn_415604), and in debug mode stops at a requested breakpoint.
 */
/* @zoombi32 0x00415613 */
void mainLoopEvents()
{
    handleWaitingMessage();
    if (!g_4ab480 && g_4a07c4)
        g_4a07c4();
    if (debugMode && g_4aa5d8) {
        g_4aa5d8 = 0;
        fn_46db93("generic breakpoint");
        debugBreak(0);
    }
    fn_415880();
}

/* @zoombi32 0x0041581b */
void fn_41581b(short flag)
{
    if (flag)
        fn_415811();
}

/* Lower-cases an ASCII letter. */
/* @zoombi32 0x004159dd */
unsigned short toLowerAscii(unsigned short c)
{
    if (c >= 'A' && c <= 'Z')
        c |= 0x20;
    return c;
}

/* Upper-cases an ASCII letter. */
/* @zoombi32 0x004159f7 */
unsigned short toUpperAscii(unsigned short c)
{
    if (c >= 'a' && c <= 'z')
        c &= 0xdf;
    return c;
}

/* @zoombi32 0x00415a11 */
void fn_415a11(Callback callback)
{
    g_4a07e8 = callback;
}

/* @zoombi32 0x00415a20 */
void fn_415a20(long value)
{
    g_4a07ec = value;
}
