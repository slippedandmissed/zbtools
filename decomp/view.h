/*
 * view's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef VIEW_H
#define VIEW_H

/* view */
void initViews(); /* 0x46310c */
void closeViews();
void clearViews();
void removeDeadViews();
void updateViews();
void initView(View *view, View *prev, View *next, short id);
void drawBackdropList(ResourceList *images);
short addView(unsigned long flags, ViewDraw draw, ViewUpdate update, short kind, long interval,
              void *data, short after, short target);
View *viewAt(Point where, unsigned long mask, short backwards);
View *nextActorView(short again);
void setViewPlaces(short count, Point *places, short apply);
void loadDragCursors(short id);
void setDragCursor(short cursor);
void freeDragCursors();
void trackDragCursor(View *view, short region);
void drawDragCursor(View *view);
View *viewListEnd(short head);
View *findView(short id);
View *removeView(short id, short dispose);
void insertViewAtEnd(View *view);
void insertViewBeforeCursor(View *view);
long countViews();
void drawBackdrop(short id);
void loadScripts(short first, short count);
void addScripts(short first, short count, short limit);
void findScript(short id, short *group, short *index);
void freeScripts();
void loadTerrain(short id);
void freeTerrain();
View *startView(short id, short script, ViewNotify notify, char notifyEnd);
void setViewScript(View *view, short script, short running);
short freeViewGroup();
short groupViews(short a, short b, short c, short d, short e, short f);
void pairViews(short a, short b);
void deleteView(short id);
long scriptFrameOffset(short *script, short *frame, short second);
unsigned long resetViewClock();
unsigned long viewClock();
void markViewTime();
unsigned long viewTimeSinceMark();
void sortViews();
View *sortViewList(View *list);
View *mergeViewList(View *into, View *list);
void setViewsLocked(short locked);
void moveView(short moving, short after, short anchor);
ImageBank *loadImageBank(short id, long *resource);
short *loadSwappedResource(long *resource, short id, long type);
short playViewSounds(SoundChannels *channels, short played, short pick);
void viewSoundList(View *view, short *count, short *sounds);
void fadeInViews();
void fadeOutViews();
short soundRangeFor(short sound, short *rank);
void addSoundRange(short low, short high, short value);
void pickViewSounds(SoundChannels *channels);
void loadViewSounds(short id, short now);
void noteSoundTest(short sound, short kind);
void drawViewLabels(short only);
extern View *views; /* @data 0x4a7ba8: viewHead, once set up */
extern long terrainResource; /* @data 0x4a7ba0 */
extern View *lastActorView; /* @data 0x4a7bac */
extern short viewsBusy; /* @data 0x4a7bb0 */
extern ShortRect g_4a7bb2;
extern long viewUnlocks; /* @data 0x4b80f0 */
extern long dragCursorResource; /* @data 0x4b87d0 */
extern ImageBank *dragCursors; /* @data 0x4b87d4 */
extern long dragHotXResource; /* @data 0x4b87d8 */
extern long dragHotYResource; /* @data 0x4b87dc */
extern short *dragHotX; /* @data 0x4b87e0 */
extern short *dragHotY; /* @data 0x4b87e4 */
extern short dragWidth; /* @data 0x4b87e8 */
extern short dragHeight; /* @data 0x4b87ea */
extern ShortRect dragRect; /* @data 0x4b87ec */
extern unsigned short *dragImage; /* @data 0x4b87f4 */
extern Point dragWhere; /* @data 0x4b87f8 */
extern unsigned long viewClockStart; /* @data 0x4b8804 */
extern unsigned long viewClockMark; /* @data 0x4b8808 */
extern unsigned short dragCursor; /* @data 0x4b89d8 */
extern ResourceList *backdropImages; /* @data 0x4b89e4 */
extern short scriptGroupFirst[8]; /* @data 0x4b89e8 */
extern short scriptGroupCount[8]; /* @data 0x4b89f8 */
extern short scriptGroups; /* @data 0x4b8a08 */
extern short g_4b8a0a;
extern char fpsText[]; /* @data 0x4b957a */
extern short soundRangeLow[32]; /* @data 0x4b94ba */
extern short soundRangeHigh[32]; /* @data 0x4b94fa */
extern short soundRangeValue[32]; /* @data 0x4b953a */
extern View viewHead; /* @data 0x4b880c: the list's ends (plain views) */
extern View viewTail; /* @data 0x4b88f8 */
extern long scriptResources[300]; /* @data 0x4b8b58 */
void requestViewSort();

#endif
