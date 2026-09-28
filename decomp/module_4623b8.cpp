/*
 * module_4623b8 (0x4623b8-0x463034): debug keys: 'ALL in party', 'midi test on'
 */

#include "zoombinis.h"

/*
 * One pass of the main loop (WinMain runs it and mainLoopEvents until it's
 * told to quit); always carries on. While g_4a4a10 is set it runs the game's
 * timed work, and if g_4b80d2 is 1 that's all; otherwise it handles a pending
 * event (handleNextEvent/discardEvents), or passes on where the cursor is (handleMouse).
 */
/* @zoombi32 0x004623b8 */
short mainLoopUpdate()
{
    Point cursor;

    if (g_4a4a10) {
        if (g_4b0d50 != -1)
            fn_43ac20();
        if (g_4a4b98 && fn_464d88() > 3600) {
            fn_464d7d();
            g_4a4b98 /= 2;
            if (!g_4a4b98)
                g_4a4b98 = 1;
        }
        if (g_4b80d2 == 1) {
            if (g_4a79c0) {
                g_4b80d8 = fn_41571f();
                g_4a79c0 = 0;
            } else if (fn_41571f() > g_4b80d8 + 3)
                fn_4624f4();
            return 1;
        }
    }
    if (handleNextEvent()) {
        discardEvents(2);
        g_4a79c0 = 1;
        g_4b80dc = fn_41571f();
    } else {
        getCursorPosition(&cursor);
        g_4a79c8 = fn_41571f() - lastClickTime;
        lastClickTime = fn_41571f();
        handleMouse(&cursor, 0);
    }
    return 1;
}
