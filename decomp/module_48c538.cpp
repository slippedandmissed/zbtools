/*
 * module_48c538 (Mohawk engine): hideCursor
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"
#include "os_manager.h"

/* ShowCursor(FALSE); with the cursor fix, the first hide swaps in a blank
   cursor instead. The new display count. */
/* @zoombi32 0x0048c538 */
short hideCursor()
{
    POINT position;
    char xorMask[128];
    char andMask[128];

    if (!graphics.cursorFix)
        return ShowCursor(FALSE);
    if (!graphics.cursorLevel) {
        graphics.savedCursor = graphics.cursor;
        memset(xorMask, 0, sizeof(xorMask));
        memset(andMask, 0xff, sizeof(andMask));
        graphics.cursor = CreateCursor(engineInstanceHandle(), 0, 0, 32, 32, andMask, xorMask);
        SetCursor(graphics.cursor);
        GetCursorPos(&position);
        SetCursorPos(position.x, position.y);
        ShowCursor(FALSE);
    }
    return --graphics.cursorLevel;
}
