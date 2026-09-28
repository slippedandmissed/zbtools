/*
 * net's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef NET_H
#define NET_H

void enterNextScene();
short sceneGroup(short *last);
void spliceList(Link *other, Link *list);
void layOutMazeCels(Snoid *snoid);
/* net */
void fn_439560(Snoid *snoid);
View *fn_43a69a(View *list);
void fn_43a5f6(View *after, unsigned long flags);
extern short g_4b12a8; /* the scene is open */
extern long g_4a2e54;
extern long g_4b12a4;
void fn_43d6e2(View *view);
void closeNet();
extern short g_4b0e78[125];
extern short g_4b0f72[125];
extern short g_4b106c[125];
extern short g_4b12aa;
extern ImageBank *g_4a2e60;
extern short g_4a2ea4;
extern short g_4a2ea6;
void fn_43c8e2(short a, short b, short n);
void fn_43c94f(short a, short b, short c, short n);
void drawNetButton(short which, short lit, short show);
void drawNetButtons(View *);
void updateNetButtons(View *, short region);
extern short g_4b1182[20];
extern short g_4b11aa[];
extern short g_4b0e76;
extern short g_4b0e66;
void fn_43e370();
void fn_43a510(View *view, short other);
extern short g_4b0e68;
extern short g_4b1438[3];
extern short g_4b144a;
extern short g_4b145c;
extern short g_4b0d5c;
extern short g_4b119a;
extern short g_4b1462;
extern short g_4b11a0;
void fn_43cfc3();
void fn_439fc3(View *view, short);
void fn_439e55(short id);
extern short g_4b1456;
extern short g_4b119e;
extern short g_4b12ac;
extern Point g_4a2b7e[];
extern Point g_4a2be2[];
extern short g_4b1452;
extern short g_4b1454;
extern short g_4b117e;
extern short g_4b1180;
extern short g_4b13ca;
extern short g_4b13cc[];
extern short g_4a2e58;
extern short g_4b1450;
extern short g_4b12b6;
void markerPlaced(View *view);
void fn_43da30(short n);
void fn_43e5a7(View *view, short region);
void fn_43e435(short n);
extern short g_4b12ba[5];
extern short g_4b144e;
extern short g_4b145a;
extern short g_4b1466;
extern short g_4b147c;
extern short g_4b0e6c;
void fn_439cb4(View *view);
void fn_43a0e8(View *view, short other);
extern short g_4b1178;
extern short g_4b117a;
extern short g_4b117c;
void fn_43d524();
extern short g_4b12b8;
extern short g_4b12ca;
extern short g_4b141e;
extern short g_4b13fe;
extern short g_4b1400;
extern short g_4b1402;
extern short g_4b1404;
extern short g_4b143e;
extern short g_4b1442;
extern short g_4b1446;
extern short g_4b13c6;
extern short g_4b142a;
void fn_43c6df();
extern short g_4b140a;
extern short g_4b140c;
extern short g_4b140e;
extern short g_4b1410;
extern short g_4b142c;
short netKey(unsigned short key);
void fn_43a2c8(View *view, short other);
extern short g_4b142e;
extern short g_4b1468;
short findCodeEntry();
/* Each scene's ambient sounds, and which have played (a bit each). */
extern short scene7Sounds[9]; /* @data 0x4a2740 */
extern unsigned long scene7SoundsUsed; /* @data 0x4a2754 */
extern short scene8Sounds[9]; /* @data 0x4a2758 */
extern unsigned long scene8SoundsUsed; /* @data 0x4a276c */
extern short scene9Sounds[12]; /* @data 0x4a2770 */
extern unsigned long scene9SoundsUsed; /* @data 0x4a2788 */
extern short scene4Sounds[15]; /* @data 0x4a278c */
extern unsigned long scene4SoundsUsed; /* @data 0x4a27ac */
extern short scene10Sounds[19]; /* @data 0x4a27b0 */
extern unsigned long scene10SoundsUsed; /* @data 0x4a27d8 */
extern short scene11Sounds[20]; /* @data 0x4a27dc */
extern unsigned long scene11SoundsUsed; /* @data 0x4a2804 */
extern short scene12Sounds[13]; /* @data 0x4a2808 */
extern unsigned long scene12SoundsUsed; /* @data 0x4a2824 */
extern short scene5Sounds[10]; /* @data 0x4a2828 */
extern unsigned long scene5SoundsUsed; /* @data 0x4a283c */
extern short scene13Sounds[13]; /* @data 0x4a2840 */
extern unsigned long scene13SoundsUsed; /* @data 0x4a285c */
extern short scene15Sounds[17]; /* @data 0x4a2860 */
extern unsigned long scene15SoundsUsed; /* @data 0x4a2884 */
extern short scene16Sounds[10]; /* @data 0x4a2888 */
extern unsigned long scene16SoundsUsed; /* @data 0x4a289c */
extern short scene18Sounds[10]; /* @data 0x4a28a0 */
extern unsigned long scene18SoundsUsed; /* @data 0x4a28b4 */
extern short scene17Sounds[10]; /* @data 0x4a28b8 */
extern unsigned long scene17SoundsUsed; /* @data 0x4a28cc */
extern unsigned long ambientSoundTime; /* @data 0x4b0d44: when to try the next */
void playAmbientSound();
extern short g_4b1440;
extern short g_4b1444;
extern short g_4b1448;
extern Point g_4a2dd6[];
extern short g_4b12b0[3];
extern short g_4b11a2;
extern short g_4b0e6a;
extern short g_4b1412;
extern short g_4b1414;
extern short g_4b147e;
extern short g_4b11a6;
extern short g_4b11a8;
extern short g_4b1478;
extern short g_4b144c;
extern short g_4a28d0;
void fn_43de4d(View *view, short event);
extern short g_4b1166[5];
extern short g_4b12cc[];
void fn_43c9e2();
void mazeZoombinisMeet(View *a, View *b);
extern short g_4b1416;
extern short g_4b1464;
void fn_43d0b4(short which, short value);
extern short g_4b11a4;
extern unsigned long g_4b1470;
void fn_43c48b(short button);
extern short g_4b141a;
extern short g_4b141c;
extern short g_4b1420;
extern short g_4b1422;
extern short g_4b1424;
extern short g_4b1426;
extern short g_4b1428;
extern short g_4b1408;
extern short g_4b13c8;
extern short g_4b12ae;
extern short g_4b1418;
extern short g_4b1430;
extern short g_4b1432;
extern short g_4b1434;
extern short g_4b1436;
extern short g_4b119c;
extern short g_4b145e;
extern short g_4b1460;
extern short g_4b1458;
extern short g_4b1406;
extern short g_4b147a;
extern ChosenSnoids *g_4b0d68;
extern GroupList g_4a2e32[1];
void openNet();
extern short g_4a2ea8;
extern short g_4b1480;
extern unsigned long g_4b146c;
extern unsigned long g_4b1474;
extern View *g_4b0d60;
void netFrame();

#endif
