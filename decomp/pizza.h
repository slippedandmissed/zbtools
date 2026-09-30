/*
 * pizza's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PIZZA_H
#define PIZZA_H

/* Pizza Pass (pizza) */
extern SceneButton pizzaButtons[13]; /* @data 0x4a33e4: buttons 1-13 */
extern short pizzaGoReady;
extern short g_4a3d98;
extern short g_4a3d9a;
extern short g_4b166c;
extern short pizzaOpen; /* the scene is open */
extern long pizzaButtonResource;
extern long pizzaFile;
extern short mealView;
extern Point pizzaPlaces[];
extern short g_4b1820;
void updatePizzaButtons(View *, short region);
void closePizza();
void showMealView();
void placeMealToppings(View *view);
void sendFlaggedToPlaces();
extern ImageBank *pizzaButtonImages;
extern short toppingCount; /* how many toppings */
extern short toppingChance;
extern short toppingsWanted;
extern short pickedToppings[8];
/* The toppings each troll wants. */
extern short arnoWants[8]; /* @data 0x4b1686 */
extern short willaWants[8]; /* @data 0x4b1696 */
extern short shylerWants[8]; /* @data 0x4b16a6 */
extern short pizzaToppings[8];
extern short lastTriedPizza;
extern char triedPizzas[];
void drawPizzaButton(short which, short lit, short show);
short randomWantedTopping(short troll);
void pickToppings();
short pizzaTriedBefore();
extern short zoombiniAtPizza;
extern short g_4b1720;
extern short arnoState;
extern short arnoView;
extern short willaView;
extern short shylerView;
extern short g_4b15f8;
extern short g_4b15f2;
extern short g_4b160a;
extern short nextZoombini;
extern short g_4b165a;
void recordPizzaTried();
void restartPizzaView();
void pizzaDoneNotify(View *, short);
/* Toppings shown on the pizza (one each): */
extern short mealShown0;
extern short mealShown1;
extern short mealShown2;
extern short mealShown3;
extern short mealShown4;
extern short mealShown5;
extern short mealShown6;
extern short g_4b1660;
extern short g_4b1600;
extern short g_4b1602;
extern short g_4b1604;
extern short g_4b1608;
extern short g_4b165e;
void startLevelTroll();
void stepTrollTurns();
extern short g_4b15da;
extern short g_4b15d8;
extern Point pizzaSpot; /* where the Zoombini at the pizza stands */
extern short g_4b15fa;
extern short g_4b15ee;
extern short g_4b171e;
void drawPizzaButtonsView(View *);
void bringNextZoombini();
extern short pizzaView;
extern short g_4b1616;
extern short lastShownPizza;
extern ShownPizza shownPizzas[];
extern short mealToppings[8];
extern short toppingViews[8]; /* @data 0x4b1636 */
extern short g_4b1630;
void orderPizzaViews();
void clearToppings();
void placePizzaToppings(View *view);
void placeArnoToppings(View *view);
void placeWillaToppings(View *view);
void placeShylerToppings(View *view);
short judgePizza(short troll);
void trollFidget();
extern short trollTurn; /* whose turn it is */
extern short g_4b1670;
extern short pizzasLeft;
extern short g_4b1606;
extern short g_4b16bc;
extern short g_4b15fe;
extern short g_4b15ec;
void willaNotify(View *view, short event);
void placeTrollToppings(View *view);
void trollReacts();
void trollsEat();
extern short g_4b15f4;
extern short g_4b15f6;
void pizzaZoombiniNotify(View *view, short event);
extern short g_4b170c;
extern short g_4b171a;
extern short g_4b1646;
extern View *g_4b15e0;
extern short g_4b15ea;
extern short g_4b165c;
extern short g_4b1648;
extern short g_4b15fc;
void servePizza();
extern short g_4b170e;
void showFourPizzas(short a, short b, short c, short d);
void drawTrollWants();
extern short g_4b16ea;
extern short g_4b16b6;
extern short g_4b16be;
extern short g_4b16c4;
extern short judgedPizzaPlace;
void trollVerdict(short troll, short verdict);
extern short g_4a3dcc;
extern short g_4a3d38;
extern short arnoPileTop;
extern short willaPileTop;
extern short shylerPileTop;
extern short arnoPile[3]; /* the three piles' top views */
extern short willaPile[3];
extern short shylerPile[3];
extern short g_4b160c;
void showJudgedPizza();
extern short g_4b15f0;
void toppingButton(short button);
extern unsigned long lastKeyTime;
extern short g_4b15e8;
extern short g_4a3d9e;
extern short g_4a3da0;
extern short g_4a3da2;
extern short g_4b1634;
short pizzaKey(unsigned short key);
void shareToppings();
extern short g_4b16b8;
extern short g_4b16ba;
extern short g_4b16c0;
extern short g_4b16c2;
extern short g_4b16c6;
extern short g_4b16c8;
extern short g_4a3d40;
void pizzaServedTo(short troll, short);
short playAndWait(short sound, short keep);
void sayIntroduction(short which);
extern short g_4b1672;
void pizzaButtonClicked(short button);
extern short inPizzaFrame; /* the scene's frame is running */
extern short g_4b1674;
extern short g_4b166e;
extern unsigned long g_4b1814;
extern unsigned long g_4b1818;
extern short g_4b181e;
void pizzaFrame();
/* The pizza scene's buttons at each level (copied into pizzaButtons). */
extern SceneButton pizzaButtonsLevel0[13];
extern SceneButton pizzaButtonsLevel1[13];
extern SceneButton pizzaButtonsLevel2[13];
extern SceneButton pizzaButtonsLevel3[13];
extern GroupList pizzaGroups[1];
extern ChosenSnoids *pizzaChosen;
extern short g_4b1632;
extern short g_4b166a;
extern short g_4b170a;
extern short g_4b162c;
extern short g_4b1622;
extern short g_4b162a;
void openPizza();

#endif
