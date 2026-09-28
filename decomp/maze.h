/*
 * maze's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef MAZE_H
#define MAZE_H

extern short g_4aff9a[21];
void fn_43595f(View *);
void fn_435966(View *, short);
int fn_43691d(long, short value);
short indexOfLargestExcept(short exclude);
short fn_437acb(short i);
short fn_4381bb();
/* maze */
extern short g_4afc6a;
extern short g_4a25c4;
extern short g_4a25c6;
extern long g_4afc18;
extern short g_4afc20;
extern long g_4afbd8;
extern long g_4afbe0;
extern long g_4afbe4;
extern long g_4afbc4;
extern long g_4afbc8;
extern long g_4afbcc;
extern long g_4afc64;
void loadMazeTable(long *resource, short *handle, short id, short **locked);
void freeMazeTable(long *resource, short *handle);
void drawMazeButton(short which, short lit, short show);
void drawMazeButtons(View *);
void updateMazeButtons(View *, short region);
void closeMaze();
short mazeKey(unsigned short key);
extern short *g_4afbe8; /* hot spots: x */
extern short *g_4afbec; /* and y */
void fn_43583c(View *view, short group, ViewNotify, char unknownF8);
void fn_435882(View *view, short group, ViewNotify, char unknownF8);
void fn_435925(View *view, short group, ViewNotify, char unknownF8);
void fn_436321(View *view);
void fn_436356(View *view);
extern short g_4b0a0a;
extern short g_4b0a0c;
extern short g_4affc4[21];
extern short g_4affee[21];
extern short g_4b0018[21];
extern short g_4b0042[21];
extern short g_4b00c6;
extern short g_4b00c8;
extern short g_4b00ca;
extern short g_4b00cc;
void fn_435f03(short id);
void fn_43824f();
short fn_438280();
void fn_4382df();
short fn_438310();
void fn_43836f();
extern short g_4a263c[21];
extern short g_4a2634[4];
extern short g_4afc36;
short fn_4381da(short kind, short low, short high);
short fn_437ade(short least);
short fn_437b23(short low, short high);
short fn_4373cd(short exclude);
short fn_437331(short which);
extern short g_4afc2e;
void fn_436045(View *, short event);
void fn_43606d(View *, short event);
void fn_43692b(View *view);
void fn_436994(View *view, short region);
void fn_437089();
short fn_4370f8(short id);
extern short g_4b0908[];
extern short g_4b09fc;
extern short g_4a210c;
extern short g_4a210e[4];
void fn_435f3d(View *view, short event);
short fn_4372bf(short which, short ignore);
void fn_435e8a(View *view, short event);
short *fn_436a00(short which);
void fn_435c57(View *view, short event);
void fn_435b9e(View *view, short event);
void fn_435da5(View *view, short event);
extern short g_4afd2c[];
extern short g_4afd48[];
extern short g_4a2308[];
extern short g_4a22d0[];
extern short g_4afc4a[12];
extern short g_4afc60;
extern short g_4b0958[];
extern short g_4b0a00;
void fn_436092(View *view, short event);
void fn_43573e(short n);
extern short g_4b09d0[];
extern short g_4b0a06;
short fn_4371b3(short id);
extern short g_4b0d26;
void fn_4350be(View *view, short pose);
extern short g_4afd26[];
extern short g_4a2324[];
void fn_43596d(View *view, short group, ViewNotify, char unknownF8);
void fn_43638b(View *view, short event);
extern short g_4b00d0; /* how many */
short fn_43780d(short exclude);
short fn_437b7b(short low, short high);
short fn_437ea2(short low, short high);
extern short g_4afc44;
extern short g_4afc2a;
extern short g_4b08b4;
extern short g_4b0cfe;
extern short g_4b0d0e;
extern short g_4b0d10[11]; /* each line's value */
extern short g_4b0096[20]; /* the maze's sequence of values */
extern short g_4b00c2;
void fn_436d39(Snoid *snoid);
short fn_437416(short exclude, short whole);
extern short g_4b0980[];
extern short g_4b0a02;
extern short g_4b00be; /* how many values in g_4b0096 */
extern short g_4b00c0;
extern short g_4a2666[];
void fn_436c71(short count);
void fn_438396();
void fn_438626();
extern short g_4afc32;
void fn_4388d8();
void fn_438d67();
extern short g_4a26aa[];
void fn_439190();
extern short g_4a25ca[11];
extern short g_4a239a[18];
void fn_436abf(short level);
extern short g_4afc3c;
extern short g_4afc3e;
extern short g_4afc40;
extern short g_4afc38;
extern short g_4afc3a;
extern short g_4afc48;
extern short g_4afc46;
extern short g_4afc2c;
extern short g_4b00ce;
extern short g_4b00aa[10];
extern short g_4a2550;
extern short g_4a2552;
extern short g_4b08b2;
extern short g_4afe52;
extern short g_4afe54;
extern short g_4afe56;
extern short g_4afe58;
extern short g_4b09a8[20];
extern short g_4b0a04;
extern short g_4b0a08;
extern short g_4afdac[3];
extern short g_4afc92[14];
extern short g_4b0ccc[25];
extern short g_4a22ec[];
extern Point g_4a21f0[];
extern Point g_4a232a[];
void openMaze();
/* Maze starting places (1-14, from 0): */
extern short g_4a2228[14];
extern short g_4a2260[14];
extern short g_4a227c[14]; /* the direction faced */
extern short g_4a2298[14];
extern Point g_4a2362[14]; /* the square */
extern ShortRect g_4a2554[];
extern unsigned long g_4a2548;
extern unsigned long g_4a254c;
void mazeButtonClicked(short button);
void mazeFrame();

#endif
