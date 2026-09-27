/*
 * module_48d278 (Mohawk engine): setCursorShape
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

/*
 * Sets the cursor: a standard one (0 arrow, 1 cross, 2 I-beam, 3 wait) or
 * one made from a Mac cursor. With the cursor fix, while the cursor is
 * hidden, the new one waits in graphics.savedCursor.
 *
 * Not exact: the original keeps `standard` and the hot spot on the stack and
 * assigns registers differently.
 */
/* @zoombi32 0x0048d278 */
short setCursorShape(const MacCursor *cursor)
{
    short standard;
    unsigned short hotH;
    unsigned short hotV;
    unsigned short xorMask[64];
    unsigned short andMask[64];
    HCURSOR handle;
    int i;

    standard = 1;
    if (cursor == 0)
        handle = LoadCursor(0, IDC_ARROW);
    else if (cursor == (MacCursor *)1)
        handle = LoadCursor(0, IDC_CROSS);
    else if (cursor == (MacCursor *)2)
        handle = LoadCursor(0, IDC_IBEAM);
    else if (cursor == (MacCursor *)3)
        handle = LoadCursor(0, IDC_WAIT);
    else {
        standard = 0;
        if (!graphics.standardCursor && !memcmp(graphics.cursorData, cursor, sizeof(MacCursor)))
            handle = graphics.cursor;
        else {
            if ((hotH = fn_492730(cursor->hotH)) > 16 || (hotV = fn_492730(cursor->hotV)) > 16)
                return setPortError(0x2a65);
            memset(xorMask, 0, sizeof(xorMask));
            memset(andMask, 0xff, sizeof(andMask));
            for (i = 0; i < 16; i++) {
                xorMask[i * 2] = ~cursor->data[i] & cursor->mask[i];
                andMask[i * 2] = ~cursor->mask[i];
            }
            if ((handle = CreateCursor(engineInstanceHandle(), hotH, hotV, 32, 32, andMask,
                                       xorMask)) == 0)
                return setPortError(0x2a37);
            memcpy(graphics.cursorData, cursor, sizeof(MacCursor));
        }
    }
    if (!graphics.cursorFix || graphics.cursorLevel >= 0) {
        if (handle != graphics.cursor) {
            if (!graphics.standardCursor) {
                SetCursor(handle);
                DestroyCursor(graphics.cursor);
            }
            graphics.cursor = handle;
        }
    } else if (handle != graphics.savedCursor) {
        if (!graphics.standardCursor)
            DestroyCursor(graphics.savedCursor);
        graphics.savedCursor = handle;
    }
    SetCursor(graphics.cursor);
    graphics.standardCursor = standard;
    return setPortError(0);
}
