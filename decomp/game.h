/*
 * game's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef GAME_H
#define GAME_H

extern short aboveWindows311; /* @data 0x4a494a */
extern short g_4a4976[12];
extern char msgRequiresQuickTime[]; /* @data 0x4a4dc7 */
extern char msgInitOs[]; /* @data 0x4a4e28 */
extern char msgInitTimer[]; /* @data 0x4a4e40 */
extern char msgInitHeap[]; /* @data 0x4a4e5b */
extern char msgNotEnoughMemory[]; /* @data 0x4a4e75 */
extern char msgNotEnoughPhysicalMemory[]; /* @data 0x4a4e8c */
extern char msgInitFileManager[]; /* @data 0x4a4eac */
extern char msgInitResourceManager[]; /* @data 0x4a4ece */
extern char msgInitConfiguration[]; /* @data 0x4a4ef4 */
extern char msgInitSound[]; /* @data 0x4a4f24 */
extern char msgNoWaveDevices[]; /* @data 0x4a4f3f */
extern char msgNoMidiDevices[]; /* @data 0x4a4f5e */
extern Font *fonts[3]; /* @data 0x4b28c8 */
extern unsigned short instanceAtom; /* @data 0x4b2ae0 */
extern short quickTimeReady; /* @data 0x4b2ae8 */
extern short g_4b2aec;
extern short g_4b2aee;
extern char *appCommandLine; /* @data 0x4b2af8 */
extern long cursors[6]; /* @data 0x4b80ac */
void fn_454c8e();
void fn_454caa();
void fn_455023(short clear);
extern unsigned long g_4a48e0; /* the least free memory seen */
extern ShortRect g_4a498e; /* where the memory statistics go */
void __cdecl fn_454ca4();
void gameFrame();
short noteOutOfMemory(unsigned long size, short error);
extern short g_4b26a6[];
extern short g_4b26ac[2];
extern short g_4b26ba[9];
void fn_45170a(short id, short script, short group, ViewNotify notify, char unknownF8);
void fn_450d00(short id, short n);
void fn_454f03();
void fn_4511c1(short n);
void fn_4512ac();
extern short g_4b2790; /* the scene is open */
extern short g_4b2792;
extern short g_4a483e;
extern short g_4a4840;
extern SceneButton g_4a4708[3];
extern long g_4a47c8;
extern long g_4b2638;
extern long g_4b2650;
extern long g_4b2654;
extern long g_4b278c;
extern ImageBank *g_4b2634;
void fn_44f180(View *, short region);
void fn_44f1f2();
void fn_4541bf(View *view);
short fn_454c10();
extern PlacedSnoid g_4b2544[];
extern basePort *g_4b2ae4;
void fn_44e0e2();
void fn_44e161();
long fn_4552fd(const char *path);
void fn_455273(short shutdown);
extern short g_4b2630;
extern short g_4b2776[7];
void fn_450c24(short id, short n);
void fn_450d5d();
void fn_450df2();
extern short g_4b2734;
extern short g_4b262e;
extern short g_4b2604[21];
extern char g_4b263c[8];
extern short g_4b25ac;
extern short g_4b25ae;
extern short g_4b26b2;
extern Point g_4a44ac;
short fn_452035();
void fn_45162e(short);
short fn_44cd71(short first, short second);
short fn_451f4e();
extern ShortRect g_4a447a;
void fn_44dcdc();
extern short g_4b2788;
extern short g_4b2742;
extern short g_4b25a4;
extern short g_4b25a6;
extern short g_4b2724[3];
extern short g_4b258c;
extern short g_4b258e;
extern short g_4b2714[4];
extern short g_4b273c;
short fn_4513ac();
void fn_4514f6();
extern short g_4b2672[8];
extern Point g_4a44cc[8];
void fn_4520ec(short count);
void fn_450e87();
void fn_451020();
extern ShortRect g_4a48b2;
extern short g_4b2798; /* cheating */
short fn_450a58(unsigned short key);
short fn_44d3b8(short cell, volatile short direction);
extern short g_4b2740;
extern short g_4b2590;
extern short g_4b2592;
void fn_44f163(View *);
short fn_455229();
void fn_451315();
void fn_44d5f5();
void fn_44e21a(short first, short second, short middle);
short fn_45537f(const char *path);
void fn_44ddc9();
void fn_44d127();
extern short g_4b26b0;
extern short g_4b2704;
extern Point g_4a4528;
extern ShortRect g_4a4534;
short fn_453e8c(View *view, Point where);
void fn_44d974(short neighbour, short index, short cell);
extern short *g_4b2658; /* the scene's Zoombini images' hot spots: x */
extern short *g_4b265c; /* and y */
void fn_454374(Snoid *snoid);
extern short g_4b27ac[5];
extern short g_4b27b6[6];
extern short g_4b27c2;
extern short g_4b27c4;
extern short g_4b27c6;
extern short g_4b27c8;
extern short g_4b279c[4];
extern short g_4b27a4[4];
extern Point g_4a4514[4];
void fn_452258(Snoid *snoid, short n);
extern short g_4b269a[4];
void fn_45222d();
void fn_454228(View *view, short region);
short fn_454165(Snoid *snoid);
extern short g_4b2644;
extern short g_4b274a;
extern short g_4b271c[2];
extern Point g_4a46a4[];
extern Point g_4a46e8[];
extern Point g_4a4634[];
extern Point g_4a466c[];
extern Point g_4a44b0;
extern short g_4b272a;
extern short g_4b272c;
extern short g_4b272e;
extern short g_4b2730;
extern short g_4b266e;
extern short g_4b2744;
extern short g_4b2594;
extern short g_4a483c;
extern short g_4b273a;
void fn_451e5d(View *, short event);
extern short g_4b28a2[4];
extern short g_4b28aa[4];
extern short g_4b28b2[4];
extern short g_4b28ba[4];
void fn_452d5d(Snoid *snoid, short n);
extern short g_4b2768[8]; /* where each of views 1-6 stands (g_4a44f0) */
extern short g_4b2662;
extern Point g_4a44f0[8];
extern Point g_4a4530;
void fn_4508db();
extern Point g_4a4524[2];
extern Point g_4a462c[2];
extern short g_4b2660;
extern short g_4b2664;
extern short g_4b2666;
extern short g_4b2668;
void fn_452857(short kind, short count);
extern short g_4b2748;
extern short g_4b2732;
void fn_4527be(short level);
short fn_4507e0();
extern short g_4b264c;
extern short g_4b2670;
extern short g_4b2738;
extern short g_4b274c;
extern short g_4b2750;
extern short g_4b2752;
extern short g_4b274e;
extern long g_4b2758;
extern long g_4b275c;
extern short g_4b2760;
extern short g_4b2762;
extern long g_4b2764;
extern long g_4b2794;
extern long g_4a47cc;
extern short g_4b258a;
extern short g_4b25b4[20];
extern short g_4b25dc[20];
extern short g_4b273e;
extern short g_4b2598;
extern Point g_4a44b4;
extern GroupList g_4a47a8;
void fn_44e494();
extern ShortRect g_4a4614;
extern ShortRect g_4a47d0;
extern short g_4a47e0[5];
void fn_44fa57(short action);
void fn_44f066(short which, short lit, short show);
short fn_44d102();
void fn_44d5ad(short cell);
void fn_44dca0(short cell, short direction, short bit);
void fn_44e092();
void fn_44e314(short cell);
void fn_451238(short n);
void fn_451276();
extern Point g_4a4846;
extern Point g_4a47ec[20];
extern short g_4b25b0;
extern short g_4b25a8;
extern short g_4b25aa;
extern short g_4b2588;
extern short g_4b26b4;
extern short g_4b26b6;
void fn_45174e(View *view, short event);
void fn_450540(short *result);
void fn_45062d(short script);
void fn_450658(short script, short running);
void fn_4506a9(short script);
void fn_4506f0();
void fn_45074d();
void fn_450796();
void fn_4507bb();

#endif
