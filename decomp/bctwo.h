/*
 * bctwo's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef BCTWO_H
#define BCTWO_H

/* One of the Zoombinis waiting at the camp (22 bytes). */
struct CampEntry
{
    long zoombini; /* 0: none */
    ShortRect rect; /* +4: where it is drawn */
    char name[10]; /* +0xc */
};

/* The Zoombinis waiting at the camp, shown in rows of 5. */
struct CampEntries
{
    short row; /* the first shown (bookRow) */
    short count; /* bookCount */
    CampEntry entries[625];
};

extern short bookRow; /* @data 0x4ab640 */
extern short bookRows; /* @data 0x4ab642 */
extern short bookSlots; /* @data 0x4ab644 */
extern short bookCount; /* @data 0x4ab646 */
extern short bookHighest; /* @data 0x4ab648 */
extern short chosenCamp2Button; /* @data 0x4ab64a */
extern CampEntries *bookEntries; /* @data 0x4ab64c */
extern short bookView; /* @data 0x4ab650 */
extern short camp2HoverButton; /* @data 0x4ab652 */
extern long unusedCamp2Long; /* @data 0x4ab654 */
extern long camp2File; /* @data 0x4ab658: BaseCamp.MHK */
extern short camp2Open; /* @data 0x4ab660: the scene is open */
extern short enoughChosen; /* @data 0x4ab65c */
extern short camp2EnoughDrawn; /* @data 0x4ab65e */
extern short camp2ClicksOff; /* @data 0x4ab662 */
extern short camp2Dragging; /* @data 0x4ab664 */
extern short view6002Next; /* @data 0x4ab666 */
extern short view6000Running; /* @data 0x4ab668 */
extern short campThingViews[10]; /* @data 0x4ab66a: their views */
extern short view6000; /* @data 0x4ab67c */
extern short populationFull; /* @data 0x4ab67e */
extern Snoid bookSnoid; /* @data 0x4ab680: the Zoombini taken out of the book */
extern short scrollPressed; /* @data 0x4a0abc: the scroll button pressed (1-4) */
extern SceneButton camp2Buttons[7]; /* @data 0x4a0adc: drawCamp2Button draws them */
extern GroupList campGroups[2]; /* @data 0x4a0c40 */
extern short bookHalfLine; /* @data 0x4a0abe: the book shows half a line more */
extern ResourceList *bookImages; /* @data 0x4a0ac0: the book's images */
extern ResourceList *camp2Images; /* @data 0x4a0ac4: the camp's images */
extern short inCamp2Frame; /* @data 0x4a0ce8: in camp2Frame */
extern ShortRect camp2ButtonsRect; /* @data 0x4a0cea */
extern ShortRect bookArea; /* @data 0x4a0be8: the book's area */
extern ShortRect campThingRects[10]; /* @data 0x4a0c58: the camp's things to click */
extern ShortRect camp2EmptyRect; /* @data 0x4a0d76 */
extern short cellX[11]; /* @data 0x4a0cf2: the book's cells' x, by half line */
extern short cellY[11][5]; /* @data 0x4a0d08: their y, by half line and column */
void resetCamp2();
void updateCamp2Button0(View *view, short region);
void countBookEntry(short n);
long camp2Key(long);
void refreshBook();
void openCamp2();
void closeCamp2();
void camp2Frame();
void drawBook(View *view);
void camp2Clicked(short which);
void campDragged(short event);
void scrollBook(View *view, short region);
void makeBookRoom();
short addPartyToBook();
void dropEmptyBookRows();
short bookEntryAt(short row, ShortRect rect, short taken);
void drawCamp2Button(short button, short lit, short group, short show);
void drawCamp2Buttons1(View *view);
void drawCamp2Buttons2(View *view);
void lightScrollButton(short quiet, short);
short lastBookEntry();

#endif
