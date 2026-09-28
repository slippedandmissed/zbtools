/*
 * focus's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FOCUS_H
#define FOCUS_H

extern GroupList *g_4a01ac;
extern short g_4a01b0;
extern InputItem *highlightedItem; /* @data 0x4aa484 */
extern unsigned short g_4aa48a;
extern unsigned char g_4aa48b;
extern short g_4aa48c; /* how many lists g_4a01ac has */
extern GroupList *g_4aa490;
extern Group *g_4aa494;
extern InputItem *g_4aa498;
extern Cursor g_4aa49c;
extern InputItem *enteredItem; /* @data 0x4aa4a8 */
extern short g_4aa4ac;
extern Point *g_4aa4b0;
extern InputItem *g_4aa4b4;
extern short g_4aa4b8;
extern short g_4aa4ba;
extern short g_4aa4bc;
extern unsigned short g_4aa4be;
extern short g_4aa4c0;
extern short g_4aa4c2;
short fn_412b4d(void (*callback)(InputItem *item));
short fn_412844(InputItem *item);
short fn_412884();
void fn_412cc0();
void fn_412cd0();
void fn_412cdf();
void fn_412d37();
void fn_412d47();
void fn_412d57();
short fn_41336f();
short fn_4133a4();
short hitTestFocus(Point *where);
void fn_412b6b();
void fn_412b8f();
void fn_412cef();
void fn_412d13();
void fn_412bb3(short alternative);
void fn_412c3d();
short fn_4128c6(GroupList *list);
short fn_41295f(Group *group);
void fn_413a4e(InputState *state, short all);
void fn_413afd(InputState *state, short all);
void fn_413bad(Point *where);
short fn_41382a(Group *group, short start);
short fn_4138a2(Group *group, short start);
short fn_41391d(InputItem *item);
short fn_413693(GroupList *list, short first, short start);
short fn_413755(GroupList *list, short first, short start);
short fn_41348b(short list, short group, short start);
short fn_41357a(short list, short group, short start);
short focusItemAtPoint(Point *where);
short focusItemByKey(short key);
short focusItemAt(short x, short y);
short focusItem(InputItem *item);
void visitAllItems();
void numberAllItems();
void setGroupLists(GroupList *lists, short count, short flags);
void leaveEnteredItem();
void enterFocusedItem();
void moveMouseTo(short x, short y);
void moveMouseToFocus();
short moveFocus(short direction);
InputItem *hoverItemAtPoint(Point *where);
void highlightFocus();
void toggleFocusedItem();
void switchOffOthers();
short fn_412722(short on, short value);
void stepFocus(short direction);
short fn_412587(InputItem *item, unsigned short button);
short trackPress(unsigned short button);
InputItem *highlightItemAt(short x, short y);
short handleMouse(Point *where, unsigned short button);
InputItem *handleKey(unsigned short *key);
void pressFocusedItem();
void getItemPosition(InputItem *item, Cursor *where);
InputItem *itemAt(short x, short y);
void activateItemAt(short x, short y);
void fn_413bcf(void (*hook)(Point *where));

#endif
