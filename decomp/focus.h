/*
 * focus's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef FOCUS_H
#define FOCUS_H

extern GroupList *groupLists; /* @data 0x4a01ac */
extern short keyboardMoved; /* @data 0x4a01b0 */
extern InputItem *highlightedItem; /* @data 0x4aa484 */
extern unsigned short inputFlags; /* @data 0x4aa48a */
extern short groupListCount; /* @data 0x4aa48c: how many lists groupLists has */
extern GroupList *currentList; /* @data 0x4aa490 */
extern Group *currentGroup; /* @data 0x4aa494 */
extern InputItem *currentItem; /* @data 0x4aa498 */
extern Cursor searchCursor; /* @data 0x4aa49c */
extern InputItem *enteredItem; /* @data 0x4aa4a8 */
extern short searchKind; /* @data 0x4aa4ac */
extern Point *searchPoint; /* @data 0x4aa4b0 */
extern InputItem *searchItem; /* @data 0x4aa4b4 */
extern short searchColumn; /* @data 0x4aa4b8 */
extern short searchRow; /* @data 0x4aa4ba */
extern short searchKey; /* @data 0x4aa4bc */
extern unsigned short searchFlags; /* @data 0x4aa4be */
extern short inputMode; /* @data 0x4aa4c0 */
extern short hovering; /* @data 0x4aa4c2 */
short callItemHandler(void (*callback)(InputItem *item));
short itemAvailable(InputItem *item);
short listsAvailable();
void defaultHandler4();
void defaultHandler0();
void defaultHandler10();
void defaultHandler18();
void defaultHandler14();
void defaultHandler24();
short handlersOverridden();
short moveOverridden();
short hitTestFocus(Point *where);
void callHandlerC();
void callHandler8();
void callHandler20();
void callHandler1C();
void callItemHandlers(short alternative);
void callOtherHandlers();
short listSearchable(GroupList *list);
short groupSearchable(Group *group);
void loadInputState(InputState *state, short all);
void saveInputState(InputState *state, short all);
void getHookedMousePosition(Point *where);
short searchGroupForward(Group *group, short start);
short searchGroupBackward(Group *group, short start);
short matchItem(InputItem *item);
short searchListForward(GroupList *list, short first, short start);
short searchListBackward(GroupList *list, short first, short start);
short searchForward(short list, short group, short start);
short searchBackward(short list, short group, short start);
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
short setFocusedOn(short on, short value);
void stepFocus(short direction);
short trackItemPress(InputItem *item, unsigned short button);
short trackPress(unsigned short button);
InputItem *highlightItemAt(short x, short y);
short handleMouse(Point *where, unsigned short button);
InputItem *handleKey(unsigned short *key);
void pressFocusedItem();
void getItemPosition(InputItem *item, Cursor *where);
InputItem *itemAt(short x, short y);
void activateItemAt(short x, short y);
void setMouseHook(void (*hook)(Point *where));

#endif
