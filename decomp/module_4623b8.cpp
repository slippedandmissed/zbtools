/*
 * module_4623b8 (0x4623b8-0x463034): debug keys: 'ALL in party', 'midi test on'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "events.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

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
            enterNextScene();
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

/* A key pressed: noted for the cheats, then given to the dialog or the
   scene; else the game's own keys (the options, the dialogs, and with
   debugging on, g_4b8803, the debugging keys), showing a toggle's new
   setting. */
/* @zoombi32 0x0046293a */
void fn_46293a(unsigned short key)
{
    short handled = 0;
    short message = 0;

    if (key < 256)
        noteCheatKey(key);
    if (g_4b9684) {
        dialogKey(key);
        handled = 1;
    } else if (currentScene != -1 && scenes[currentScene]->key && scenes[currentScene]->key(key))
        handled = 1;
    else {
        switch (key) {
        case 32:
            if ((short)isCheat(0xa675d204, 0xfd3939a0)) {
                g_4b8803 = 1;
                showNameTag("you got it", 90, 1);
            } else if ((short)isCheat(0xd09804a9, 0xdd3934a0)) {
                midiTest = !midiTest;
                if (midiTest) {
                    midiTestIndex = 0;
                    showNameTag("midi test on", 90, 1);
                } else
                    showNameTag("midi test off", 90, 1);
            } else if (midiTest) {
                if (addModifierKeys(0) == 0x800)
                    midiTestIndex++;
                if (midiTestIndex >= 18 || midiTestIndex < 0)
                    midiTestIndex = 0;
                queueViewSound(midiTests[midiTestIndex], 0);
                noteSoundTest(midiTests[midiTestIndex], 3);
            }
            break;
        case 94:
            if (g_4b8803)
                g_4a48e4 = !g_4a48e4;
            break;
        case 64:
            if (g_4b8803) {
                g_4a4ba0[0x50] |= 1;
                *(short *)(g_4a4ba0 + 0x52) |= 1;
                g_4a4ba0[0x51] |= 1;
            }
            break;
        case 47:
        case 63:
            showDialog(1, 0, 0, 0);
            break;
        case 14:
            askNewGame();
            break;
        case 12:
            askLoadGame();
            break;
        case 19:
            askSaveGame();
            break;
        case 2:
            g_4b87ff = !g_4b87ff;
            if (g_4b87ff) {
                queueViewSound(0, 0);
                message = 1;
            } else {
                stopSounds(g_4a7d42, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                message = 2;
            }
            break;
        case 4:
            g_4b87fe = !g_4b87fe;
            if (!g_4b87fe && lastViewSound) {
                stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            if (g_4b87fe)
                message = 3;
            else
                message = 4;
            break;
        case 7:
            *(unsigned short *)(g_4a4ba0 + 0x20) = !*(unsigned short *)(g_4a4ba0 + 0x20);
            if (*(short *)(g_4a4ba0 + 0x20))
                message = 5;
            else
                message = 6;
            break;
        case 8:
            hideDragCursor = !hideDragCursor;
            if (hideDragCursor)
                message = 7;
            else
                message = 8;
            break;
        case 10:
            clickToDragOption = !clickToDragOption;
            if (clickToDragOption)
                message = 9;
            else
                message = 10;
            break;
        case 17:
            askQuit();
            handled = 1;
            break;
        case 20:
            g_4b0d4a = !g_4b0d4a;
            if (g_4b0d4a)
                message = 12;
            else
                message = 11;
            break;
        case 21:
            dragClicks = !dragClicks;
            if (dragClicks)
                message = 13;
            else
                message = 14;
            break;
        case 22:
            fn_4625b8();
            handled = 1;
            break;
        case 61:
            if (g_4b8803)
                unionRgnRect(removedRgn, &gameRect);
            break;
        case 38:
            if (g_4b8803)
                fn_456a64();
            break;
        case 42:
            if (g_4b8803)
                g_4a4b98 = g_4a7af8 = 0;
            break;
        case 91:
            if (g_4b8803) {
                viewStep = 0;
                viewsPaused = 0;
                debugMessage(-1, "Step-Mode OFF", 0, 0, 0);
            }
            break;
        case 93:
            if (g_4b8803) {
                if (viewsPaused) {
                    if (viewStep) {
                        viewStep++;
                        if ((short)countViews() >= viewStep)
                            drawViewLabels(viewStep);
                        else {
                            viewStep = 0;
                            viewsStep = 1;
                        }
                    } else
                        viewsStep = 1;
                } else {
                    viewStep = 0;
                    viewsPaused = 1;
                    debugMessage(-1, "Step-Mode ON", 0, 0, 0);
                }
            }
            break;
        case 5:
        case 6:
            if (g_4b8803) {
                labelActorsOnly = key == 6;
                if (viewsPaused)
                    viewStep = 1;
                else
                    viewStep = 0;
                labelIds = 0;
                drawViewLabels(viewStep);
            }
            break;
        case 24:
        case 25:
            if (g_4b8803) {
                labelActorsOnly = key == 25;
                if (viewsPaused)
                    viewStep = 1;
                else
                    viewStep = 0;
                labelIds = 1;
                drawViewLabels(viewStep);
            }
            break;
        case 9:
            if (g_4b8803) {
                if (countSnoidViews() > countChosenSnoids()) {
                    chooseSnoids(1, 1);
                    debugMessage(-1, "ALL in party", 0, 0, 0);
                } else
                    debugMessage(-1, "ALL in party", 0, 0, 0);
            }
            break;
        case 78:
            if (g_4b8803)
                drawPaths();
            break;
        case 80:
            if (g_4b8803) {
                fpsTime = clockTime();
                fpsMin = 999;
                fpsMax = 0;
                fpsFrames = 0;
                showFps = !showFps;
                if (!showFps)
                    unionRgnRect(removedRgn, &fpsRect);
            }
            break;
        case 83:
            if (g_4b8803)
                soundTests = !soundTests;
            break;
        case 18:
            if (g_4b8803)
                toggleShowPositions();
            break;
        case 26:
            if (g_4b8803) {
                if (!fillViews)
                    fillViews = 1;
                else {
                    fillViews = 0;
                    if (removedRgn)
                        unionRgnRect(removedRgn, &gameRect);
                }
            }
            break;
        }
        if (message)
            showNameTag(toggleTexts[message], 90, 1);
    }
    if (handled)
        resetViewClock();
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

/* Sets the cursor for mode g_4b80d2: the arrow for 0, else that mode's
   cursor (g_4b80c4). */
/* @zoombi32 0x0046258a */
void fn_46258a()
{
    if (!g_4b80d2) {
        setCursorShape(0);
        return;
    }
    setCursorShape((const MacCursor *)handleData(g_4b80c4[g_4b80d2]));
}

/* Shows the about box (the title, version and copyright, in a framed
   white box in the middle of the game's area) until a key or click. */
/* @zoombi32 0x004625b8 */
void fn_4625b8()
{
    ShortRect rect;
    ShortRect inner;
    Color saved;
    Font *font;
    basePort *port;

    rect.left = (gameRect.right - 340) >> 1;
    rect.right = rect.left + 340;
    rect.top = (gameRect.bottom - 152) >> 1;
    rect.bottom = rect.top + 152;
    port = getPort();
    setPort(screenPort);
    font = setFont(fonts[1]);
    saved = getForeColor();
    setForeColor(Color(RGBColor(0, 0, 0)));
    fillPortRect(Rect(rect), Color(RGBColor(0xff, 0xff, 0xff)), 0);
    frameRect(Rect(rect));
    inner = rect;
    insetRect(&inner, 9, 9);
    drawText(Rect(inner), 0x22, aboutText, 0xffff);
    while (!isEventWaiting(3, 0))
        ;
    discardEvents(3);
    setForeColor(saved);
    setFont(font);
    setPort(port);
    showRect(&rect);
}

/* A mouse button pressed at `where`: goes to the dialog while one is up,
   else to the input items. */
/* @zoombi32 0x004624bd */
void fn_4624bd(Point *where, short button)
{
    g_4b80d0 = button;
    resetViewClock();
    if (g_4b9684)
        dialogClick(*where);
    else
        handleMouse(where, button);
}

/* @zoombi32 0x004624f4 */
void fn_4624f4()
{
    setCursorMode(0);
}

/* @zoombi32 0x004624fc */
void fn_4624fc()
{
    if (setCursorMode(1)) {
        g_4a79c0 = 1;
        g_4b80dc = -7202;
    }
}
