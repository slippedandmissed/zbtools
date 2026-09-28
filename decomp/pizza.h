/*
 * pizza's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PIZZA_H
#define PIZZA_H

/* Pizza Pass (pizza) */
extern SceneButton pizzaButtons[13]; /* @data 0x4a33e4: buttons 1-13 */
extern short g_4b15e6;
extern short g_4a3d98;
extern short g_4a3d9a;
extern short g_4b166c;
extern short g_4b15e4; /* the scene is open */
extern long g_4a3d3c;
extern long g_4b15d0;
extern short g_4b162e;
extern Point g_4a3d54[];
extern short g_4b1820;
void fn_441127(View *, short region);
void closePizza();
void fn_4423d7();
void fn_442443(View *view);
void fn_4468eb();
extern ImageBank *g_4a3d94;
extern short g_4b1624; /* how many toppings */
extern short g_4b1626;
extern short g_4b1628;
extern short g_4b1676[8];
/* The toppings each troll wants. */
extern short arnoWants[8]; /* @data 0x4b1686 */
extern short willaWants[8]; /* @data 0x4b1696 */
extern short shylerWants[8]; /* @data 0x4b16a6 */
extern short g_4b16da[8];
extern short g_4b1708;
extern char g_4b16ec[];
void drawPizzaButton(short which, short lit, short show);
short fn_443316(short troll);
void fn_44410b();
short fn_44460a();
extern short g_4a3d42;
extern short g_4b1720;
extern short g_4b1618;
extern short g_4b160e;
extern short g_4b1610;
extern short g_4b1612;
extern short g_4b15f8;
extern short g_4b15f2;
extern short g_4b160a;
extern short g_4b15d6;
extern short g_4b165a;
void fn_444556();
void fn_44509b();
void fn_445ae1(View *, short);
/* Toppings shown on the pizza (one each): */
extern short g_4b164a;
extern short g_4b164c;
extern short g_4b164e;
extern short g_4b1650;
extern short g_4b1652;
extern short g_4b1654;
extern short g_4b1656;
extern short g_4b1660;
extern short g_4b1600;
extern short g_4b1602;
extern short g_4b1604;
extern short g_4b1608;
extern short g_4b165e;
void fn_4458c3();
void fn_4459b3();
extern short g_4b15da;
extern short g_4b15d8;
extern Point g_4a3d44; /* where the Zoombini at the pizza stands */
extern short g_4b15fa;
extern short g_4b15ee;
extern short g_4b171e;
void drawPizzaButtonsView(View *);
void fn_445789();
extern short g_4b1664;
extern short g_4b1616;
extern short g_4b1712;
extern ShownPizza g_4b1734[];
extern short g_4b16ca[8];
extern short toppingViews[8]; /* @data 0x4b1636 */
extern short g_4b1630;
void fn_446035();
void fn_446198();
void fn_442a9f(View *view);
void fn_44468e(View *view);
void fn_44485d(View *view);
void fn_444a93(View *view);
short fn_44338b(short troll);
void fn_446745();
extern short g_4b1614; /* whose turn it is */
extern short g_4b1670;
extern short g_4b1620;
extern short g_4b1606;
extern short g_4b16bc;
extern short g_4b15fe;
extern short g_4b15ec;
void fn_4441a8(View *view, short event);
void fn_442c6c(View *view);
void fn_444c62();
void fn_444391();
extern short g_4b15f4;
extern short g_4b15f6;
void fn_444e0c(View *view, short event);
extern short g_4b170c;
extern short g_4b171a;
extern short g_4b1646;
extern View *g_4b15e0;
extern short g_4b15ea;
extern short g_4b165c;
extern short g_4b1648;
extern short g_4b15fc;
void fn_445153();
extern short g_4b170e;
void fn_446487(short a, short b, short c, short d);
void fn_443e2e();
extern short g_4b16ea;
extern short g_4b16b6;
extern short g_4b16be;
extern short g_4b16c4;
extern short g_4b1710;
void fn_445b80(short troll, short verdict);
extern short g_4a3dcc;
extern short g_4a3d38;
extern short g_4b1714;
extern short g_4b1716;
extern short g_4b1718;
extern short g_4b1722[3]; /* the three piles' top views */
extern short g_4b1728[3];
extern short g_4b172e[3];
extern short g_4b160c;
void fn_445307();
extern short g_4b15f0;
void fn_442560(short button);
extern unsigned long g_4b1824;
extern short g_4b15e8;
extern short g_4a3d9e;
extern short g_4a3da0;
extern short g_4a3da2;
extern short g_4b1634;
short pizzaKey(unsigned short key);
void fn_442ea2();
extern short g_4b16b8;
extern short g_4b16ba;
extern short g_4b16c0;
extern short g_4b16c2;
extern short g_4b16c6;
extern short g_4b16c8;
extern short g_4a3d40;
void fn_443521(short troll, short);
short fn_445feb(short sound, short keep);
void fn_445eb3(short which);
extern short g_4b1672;
void pizzaButtonClicked(short button);
extern short g_4a3d9c; /* the scene's frame is running */
extern short g_4b1674;
extern short g_4b166e;
extern unsigned long g_4b1814;
extern unsigned long g_4b1818;
extern short g_4b181e;
void pizzaFrame();
/* The pizza scene's buttons at each level (copied into pizzaButtons). */
extern SceneButton g_4a35b8[13];
extern SceneButton g_4a378c[13];
extern SceneButton g_4a3960[13];
extern SceneButton g_4a3b34[13];
extern GroupList g_4a3d18[1];
extern ChosenSnoids *g_4b15dc;
extern short g_4b1632;
extern short g_4b166a;
extern short g_4b170a;
extern short g_4b162c;
extern short g_4b1622;
extern short g_4b162a;
void openPizza();

#endif
