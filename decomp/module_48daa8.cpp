/*
 * module_48daa8 (Mohawk engine): showCursor
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* ShowCursor(TRUE); with the cursor fix, the last show puts the real
   cursor back. The new display count. */
/* @zoombi32 0x0048daa8 */
short showCursor()
{
    if (!graphics.cursorFix)
        return ShowCursor(TRUE);
    if (graphics.cursorLevel == -1) {
        SetCursor(graphics.savedCursor);
        ShowCursor(TRUE);
        DeleteObject(graphics.cursor);
        graphics.cursor = graphics.savedCursor;
        graphics.savedCursor = 0;
    }
    return ++graphics.cursorLevel;
}
