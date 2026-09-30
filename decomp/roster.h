/*
 * roster's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ROSTER_H
#define ROSTER_H

void fn_41f195(const char *message);
void fn_41f668();
void newSaveFileName(const char *, char *file, short *nextId);
void fn_41f5d0();
void fn_41f551();
int fn_41d3e6(View *, short value);
void fn_41d9e4(long);
void fn_41d9eb(long, long);
extern long g_4a0fd4;
extern ShortRect g_4a11ac;
extern Point g_4ab8e0;
void fn_41dccb();
void fn_41dbce();
void fn_41dbab(View *view, short region);
void fn_41e8f3(short id, short n);
extern char *rosterError; /* @data 0x4aba80 */
extern Point *g_4ab8e4;
extern short g_4ab9f8;
extern short g_4aba78[2];
extern long g_4aba70[2];
void reportRosterError(const char *message);
void fn_41d167(short group, short script, ViewNotify notify, char f8);
void fn_41dd37(volatile short n);
void fn_41d30b(View *view, short event);
void fn_41dce6();
extern short g_4a0fe8;
extern short g_4a120a;
extern short g_4a120c;
extern long g_4aba7c; /* the roster file */
void fn_41d972(View *, short region);
void applyPlayerSettings();
short openRosterFile(const char *path, short mode);
extern short g_4a1000; /* the first of the frames fn_41dd37 shows */
extern short g_4a1002; /* their number */
extern short g_4a1004; /* the frame shown */
extern SceneButton rosterButtons[2]; /* @data 0x4a1024: buttons 1 and 2 */
extern ImageBank *g_4a1020;
extern short g_4a1282[6];
extern short g_4a128e[6];
extern short g_4a129a[6];
extern short g_4a12a6[6];
extern short g_4ab8ec[21];
void drawRosterButton(short which, short lit, short show);
void fn_41ed59(short kind, short n, ShortRect rect);
void fn_41d80e(short x, short y, long interval);
extern short g_4a0fec; /* the roster screen is open */
extern long g_4a0fd0;
extern long g_4ab83c;
extern short g_4ab878;
extern short g_4a0ff4;
extern short rosterFeatures[2]; /* @data 0x4ab87a: the features (0-3) the roster asks about */
extern short g_4a1014;
void drawRosterButtonsView(View *);
void closeRoster();
void fn_41e273();
extern short g_4ab872;
extern short *g_4aba68; /* @data 0x4aba68: REGS 200 */
extern short *g_4aba6c; /* @data 0x4aba6c: REGS 201 */
void fn_41d9f2(short which, short image, long);
extern short g_4ab870;
extern short g_4aba64;
extern short g_4ab86c;
extern short g_4ab86e;
extern short g_4a100e;
extern short g_4a1010;
extern short g_4a1012;
extern short g_4a0ff2;
extern short g_4a101a;
extern short g_4a0ffa;
extern short g_4a0ff8;
extern short g_4a0fe4;
void readWriteRoster(void *data, short read);
void fn_41dfe3(short which);
extern short g_4a0ffe; /* @data 0x4a0ffe */
extern short g_4a1006; /* @data 0x4a1006 */
extern short g_4ab994; /* @data 0x4ab994 */
extern Point g_4aba14[20]; /* @data 0x4aba14: a stack of points, g_4aba64 of them */
void fn_41d1b1(View *view, short event);
extern short g_4a1018; /* @data 0x4a1018 */
void fn_41e0f3();
extern short rosterPlaced[11]; /* @data 0x4ab840: an image is placed at place 1-10 */
extern short rosterPlaceImages[11]; /* @data 0x4ab856: the image placed there */
void fn_41db60(long unused);
void fn_41dadf(View *);
void fn_41eaf1();
extern short g_4a100c; /* @data 0x4a100c */
void fn_41e5e1();
short fn_41e771(short id, short n);
extern unsigned long g_4a78cc; /* @data 0x4a78cc */
void fn_41f6fc(short reset);
void readWriteSavedGames(SavedGameList *list, short mode);
extern short g_4ab9c2; /* @data 0x4ab9c2: the roster's next Zoombini's view */
extern short g_4ab9c4[26]; /* @data 0x4ab9c4: views, by frame */
extern short g_4ab8da; /* @data 0x4ab8da */
extern short g_4ab8dc; /* @data 0x4ab8dc */
extern short g_4a0ffc; /* @data 0x4a0ffc: the first of the walking scripts */
void fn_41cf14(short which);
void fn_41e326(short kind);
void fn_41edf7();
extern Point g_4a115c[20]; /* @data 0x4a115c: the chosen Zoombinis' spots on the roster screen */
extern short g_4a101c; /* @data 0x4a101c */
extern short g_4a1016; /* @data 0x4a1016 */
extern short g_4a0fea; /* @data 0x4a0fea */
extern short g_4ab96a[21]; /* @data 0x4ab96a: views, by place */
void fn_41e0e3();
void fn_41ec69();
void fn_41eb43();
void fn_41e920(short feature);
extern Point g_4a10a8[21]; /* @data 0x4a10a8 */
extern ShortRect g_4a10fc[12]; /* @data 0x4a10fc: the places' areas */
extern short g_4a11b4[21]; /* @data 0x4a11b4 */
extern short g_4a11de[21]; /* @data 0x4a11de */
extern short g_4ab996[20]; /* @data 0x4ab996: g_4ab9be of them */
extern short g_4ab9be; /* @data 0x4ab9be */
extern short g_4aba08; /* @data 0x4aba08 */
extern short g_4ab876; /* @data 0x4ab876 */
extern Point *g_4ab8e8; /* @data 0x4ab8e8 */
void rosterClicked(short which);
extern short g_4a1208; /* @data 0x4a1208: rosterFrame is running */
extern unsigned long g_4ab9fc; /* @data 0x4ab9fc: when a Zoombini last cheered */
extern unsigned long g_4aba00; /* @data 0x4aba00: slots used (allocateSlot) */
extern short g_4aba04; /* @data 0x4aba04: how many cheer */
extern short g_4aba06; /* @data 0x4aba06: how many have */
void rosterFrame();
short rosterKey(unsigned short key);

#endif
