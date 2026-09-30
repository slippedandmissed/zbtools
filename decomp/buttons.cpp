/*
 * buttons (0x4121cc-0x4124a4): 'too many graphic button groups:', 'graphic button'
 *
 * Graphic buttons: items drawn from their group's images. An item's
 * cursor.a.y is its button group (from 1) and cursor.c.y its image pair.
 */

#include "zoombinis.h"
#include "buttons.h"
#include "e2memory.h"
#include "graphics.h"
#include "jointext.h"

short buttonColors[6] = {0};

ButtonGroup buttonGroups[10];
char *buttonText;
char *buttonError;
short buttonsOffscreen;

/* @zoombi32 0x004121cc */
void drawButtonOn(InputItem *item)
{
    drawButton(item, 1, 0);
}

/* @zoombi32 0x004121df */
void drawButtonLit(InputItem *item)
{
    drawButton(item, 1, 8);
}

/* @zoombi32 0x004121f2 */
void drawButtonInColor0(InputItem *item)
{
    drawButtonInColor(item, buttonColors[0]);
}

/* @zoombi32 0x00412208 */
void drawButtonOff(InputItem *item)
{
    drawButton(item, 0, 0);
}

/* @zoombi32 0x0041221b */
void drawButtonUnlit(InputItem *item)
{
    drawButton(item, 0, 8);
}

/* @zoombi32 0x0041222e */
void drawButtonInColor1(InputItem *item)
{
    drawButtonInColor(item, buttonColors[1]);
}

/* @zoombi32 0x00412244 */
void drawButtonUp(InputItem *item)
{
    drawButtonPressed(item, 0);
}

/* @zoombi32 0x00412255 */
void drawButtonDown(InputItem *item)
{
    drawButtonPressed(item, 8);
}

/* @zoombi32 0x00412266 */
void drawButtonInColor4(InputItem *item)
{
    drawButtonInColor(item, buttonColors[4]);
}

/* Draws a button's image offset by twice its group's kind, then shows it
   unless buttonsOffscreen. */
/* @zoombi32 0x0041227c */
void drawButtonPressed(InputItem *item, short mode)
{
    short group = item->cursor.a.y - 1;
    short image = item->cursor.c.y;

    if (buttonGroups[group].kind == 1)
        image = 1;
    drawImage(buttonGroups[group].images, image + buttonGroups[group].kind * 2,
              item->bounds.left, item->bounds.top, mode, 0x11);
    if (!buttonsOffscreen)
        copyBits(screenPort, getPort(), &item->bounds);
}

/* Draws a button's on or off image (in a drawing mode), then shows it unless
   buttonsOffscreen. */
/* @zoombi32 0x004122f8 */
void drawButton(InputItem *item, short on, short mode)
{
    short group = item->cursor.a.y - 1;
    short image = item->cursor.c.y;

    if (buttonGroups[group].kind == 1)
        image = 1;
    drawImage(buttonGroups[group].images, image * 2 - on, item->bounds.left, item->bounds.top,
              mode, 0x11);
    if (!buttonsOffscreen)
        copyBits(screenPort, getPort(), &item->bounds);
}

/* Draws a button's image in a colour, then shows it unless buttonsOffscreen. */
/* @zoombi32 0x00412367 */
void drawButtonInColor(InputItem *item, short color)
{
    short group = item->cursor.a.y - 1;
    short image = item->cursor.c.y;

    if (buttonGroups[group].kind == 1)
        image = 1;
    drawImageInColor(buttonGroups[group].images, image, item->bounds.left, item->bounds.top, 0,
                     color, 0x11);
    if (!buttonsOffscreen)
        copyBits(screenPort, getPort(), &item->bounds);
}

/* Loads a group's images (resource `id`) as the button group its first item
   names. `name` is for error messages. */
/* @zoombi32 0x004123d2 */
void addButtonGroup(ResourceList **images, short id, short kind, Group *group, const char *name)
{
    short index = group->items->cursor.a.y - 1;

    if (index >= 10) {
        joinText(&buttonError, "too many graphic button groups:", name);
        reportJoinedError(buttonError);
    }
    joinText(&buttonText, name, "graphic button");
    loadShapeList(images, id, buttonText, 1);
    freeText((void **)&buttonText);
    buttonGroups[index].images = *images;
    buttonGroups[index].kind = kind;
}

/* @zoombi32 0x0041245f */
void freeButtonGroup(ResourceList **images)
{
    freeText((void **)&buttonText);
    freeText((void **)&buttonError);
    freeShapeList(images);
}

/* freeButtonGroup with setFreeAtOnce's setting at 1. */
/* @zoombi32 0x00412482 */
void freeButtonGroupNow(ResourceList **images)
{
    short saved = setFreeAtOnce(1);

    freeButtonGroup(images);
    setFreeAtOnce(saved);
}
