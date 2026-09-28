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

/*
 * Shows a debugging message, if they're on (g_4b8803) and `value` isn't 0:
 * `before`, *number, `after` and `value` (if positive), in debugRect; if
 * `wait`, waits for a key or click.
 */
/* @zoombi32 0x00462749 */
void debugMessage(short value, const char *after, short *number, const char *before, short wait)
{
    char line[256] = "";
    Color saved;
    short i;

    if (g_4b8803) {
        if (value) {
            line[255] = 0;
            if (before)
                for (i = strlen(before) + 1; i >= 0; i--)
                    line[i] = before[i];
            if (number) {
                i = strlen(line);
                line[i++] = ' ';
                intToDecimal(*number, line + i);
            }
            if (after) {
                short start;

                if (before) {
                    start = strlen(line);
                    line[start++] = ' ';
                } else {
                    start = 0;
                }
                for (i = strlen(after) + 1; i >= 0; i--)
                    line[i + start] = after[i];
            }
            if (value > 0) {
                i = strlen(line);
                line[i++] = ' ';
                intToDecimal(value, line + i);
            }
            fillPortRect(Rect(debugRect), Color(0xff), 0);
            saved = setForeColor(Color(0));
            drawText(Rect(debugRect), 0x22, line, 0xffff);
            setForeColor(saved);
            showRect(&debugRect);
        }
        if (wait)
            while (!isInputWaiting(3))
                ;
    }
}

/* Sets the cursor mode (0: the arrow; else cursor g_4b80c4[mode]):
   whether it changed. */
/* @zoombi32 0x0046251c */
short setCursorMode(long mode)
{
    short changed = mode != g_4b80d2;

    if (changed) {
        if (g_4b80d2 == 1)
            g_4b80d4 = g_4b80dc = clockTime();
        discardEvents(3);
        if (!mode)
            setCursorShape(0);
        else
            setCursorShape((const MacCursor *)handleData(g_4b80c4[mode]));
        g_4b80d2 = mode;
    }
    return changed;
}

/* @zoombi32 0x004624fc */
void fn_4624fc()
{
    if (setCursorMode(1)) {
        g_4a79c0 = 1;
        g_4b80dc = -7202;
    }
}
