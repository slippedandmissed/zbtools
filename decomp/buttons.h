/*
 * buttons's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BUTTONS_H
#define BUTTONS_H

extern short buttonColors[6]; /* @data 0x4a019c: colours buttons are drawn in */
extern ButtonGroup buttonGroups[10]; /* @data 0x4aa440 */
extern char *buttonText; /* @data 0x4aa47c */
extern char *buttonError; /* @data 0x4aa480 */
extern short buttonsOffscreen; /* @data 0x4aa488: drawn buttons aren't copied to the screen */
/* buttons */
void drawButtonOn(InputItem *item);
void fn_4121df(InputItem *item);
void drawButtonInColor0(InputItem *item);
void drawButtonOff(InputItem *item);
void fn_41221b(InputItem *item);
void drawButtonInColor1(InputItem *item);
void fn_412244(InputItem *item);
void fn_412255(InputItem *item);
void drawButtonInColor4(InputItem *item);
void drawButtonPressed(InputItem *item, short mode);
void drawButton(InputItem *item, short on, short mode);
void drawButtonInColor(InputItem *item, short color);
void addButtonGroup(ResourceList **images, short id, short kind, Group *group, const char *name);
void freeButtonGroup(ResourceList **images);
void fn_412482(ResourceList **images);

#endif
