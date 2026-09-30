/*
 * pizza's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PIZZA_H
#define PIZZA_H

/* Pizza Pass (pizza) */
extern SceneButton pizzaButtons[13]; /* @data 0x4a33e4: buttons 1-13 */
extern short pizzaGoReady; /* @data 0x4b15e6 */
extern short pizzaButton2Lit; /* @data 0x4a3d98 */
extern short pizzaButton1Drawn; /* @data 0x4a3d9a */
extern short paceBeforePizza; /* @data 0x4b166c */
extern short pizzaOpen; /* @data 0x4b15e4: the scene is open */
extern long pizzaButtonResource; /* @data 0x4a3d3c */
extern long pizzaFile; /* @data 0x4b15d0 */
extern short mealView; /* @data 0x4b162e */
extern Point pizzaPlaces[]; /* @data 0x4a3d54 */
extern short pizzaFidgeting; /* @data 0x4b1820 */
void updatePizzaButtons(View *, short region);
void closePizza();
void showMealView();
void placeMealToppings(View *view);
void sendFlaggedToPlaces();
extern ImageBank *pizzaButtonImages; /* @data 0x4a3d94 */
extern short toppingCount; /* @data 0x4b1624: how many toppings */
extern short toppingChance; /* @data 0x4b1626 */
extern short toppingsWanted; /* @data 0x4b1628 */
extern short pickedToppings[8]; /* @data 0x4b1676 */
/* The toppings each troll wants. */
extern short arnoWants[8]; /* @data 0x4b1686 */
extern short willaWants[8]; /* @data 0x4b1696 */
extern short shylerWants[8]; /* @data 0x4b16a6 */
extern short pizzaToppings[8]; /* @data 0x4b16da */
extern short lastTriedPizza; /* @data 0x4b1708 */
extern char triedPizzas[]; /* @data 0x4b16ec */
void drawPizzaButton(short which, short lit, short show);
short randomWantedTopping(short troll);
void pickToppings();
short pizzaTriedBefore();
extern short zoombiniAtPizza; /* @data 0x4a3d42 */
extern short skipPizzaReorder; /* @data 0x4b1720 */
extern short arnoState; /* @data 0x4b1618 */
extern short arnoView; /* @data 0x4b160e */
extern short willaView; /* @data 0x4b1610 */
extern short shylerView; /* @data 0x4b1612 */
extern short zoombiniRestartGroup; /* @data 0x4b15f8 */
extern short pizzaView7000Group; /* @data 0x4b15f2 */
extern short nextZoombiniGroup; /* @data 0x4b160a */
extern short nextZoombini; /* @data 0x4b15d6 */
extern short partyThrough; /* @data 0x4b165a */
void recordPizzaTried();
void restartPizzaView();
void pizzaDoneNotify(View *, short);
/* Toppings shown on the pizza (one each): */
extern short mealShown0; /* @data 0x4b164a */
extern short mealShown1; /* @data 0x4b164c */
extern short mealShown2; /* @data 0x4b164e */
extern short mealShown3; /* @data 0x4b1650 */
extern short mealShown4; /* @data 0x4b1652 */
extern short mealShown5; /* @data 0x4b1654 */
extern short mealShown6; /* @data 0x4b1656 */
extern short levelTrollStarted; /* @data 0x4b1660 */
extern short arnoGroup; /* @data 0x4b1600 */
extern short willaGroup; /* @data 0x4b1602 */
extern short shylerGroup; /* @data 0x4b1604 */
extern short trollTurnsGroup; /* @data 0x4b1608 */
extern short trollTurnStep; /* @data 0x4b165e */
void startLevelTroll();
void stepTrollTurns();
extern short zoombiniDone; /* @data 0x4b15da */
extern short zoombiniSettled; /* @data 0x4b15d8 */
extern Point pizzaSpot; /* @data 0x4a3d44: where the Zoombini at the pizza stands */
extern short zoombiniWalkGroup; /* @data 0x4b15fa */
extern short zoombinisSent; /* @data 0x4b15ee */
extern short zoombiniComing; /* @data 0x4b171e */
void drawPizzaButtonsView(View *);
void bringNextZoombini();
extern short pizzaView; /* @data 0x4b1664 */
extern short pizzaAnchorView; /* @data 0x4b1616 */
extern short lastShownPizza; /* @data 0x4b1712 */
extern ShownPizza shownPizzas[]; /* @data 0x4b1734 */
extern short mealToppings[8]; /* @data 0x4b16ca */
extern short toppingViews[8]; /* @data 0x4b1636 */
extern short toppingsSliding; /* @data 0x4b1630 */
void orderPizzaViews();
void clearToppings();
void placePizzaToppings(View *view);
void placeArnoToppings(View *view);
void placeWillaToppings(View *view);
void placeShylerToppings(View *view);
short judgePizza(short troll);
void trollFidget();
extern short trollTurn; /* @data 0x4b1614: whose turn it is */
extern short pickedTrollTurn; /* @data 0x4b1670 */
extern short pizzasLeft; /* @data 0x4b1620 */
extern short reactGroup; /* @data 0x4b1606 */
extern short lastPizzaEaten; /* @data 0x4b16bc */
extern short trollTurnDue; /* @data 0x4b15fe */
extern short cheerGroup; /* @data 0x4b15ec */
void willaNotify(View *view, short event);
void placeTrollToppings(View *view);
void trollReacts();
void trollsEat();
extern short pizzaViewGroup; /* @data 0x4b15f4 */
extern short judgeGroup; /* @data 0x4b15f6 */
void pizzaZoombiniNotify(View *view, short event);
extern short pendingPizzaFacing; /* @data 0x4b170c */
extern short noPathWalk; /* @data 0x4b171a */
extern short pizzasRemain; /* @data 0x4b1646 */
extern View *departingZoombini; /* @data 0x4b15e0 */
extern short zoombiniBack; /* @data 0x4b15ea */
extern short satisfiedThisPizza; /* @data 0x4b165c */
extern short outOfPizzas; /* @data 0x4b1648 */
extern short serveGroup; /* @data 0x4b15fc */
void servePizza();
extern short pizzaScriptStep; /* @data 0x4b170e */
void showFourPizzas(short a, short b, short c, short d);
void drawTrollWants();
extern short satisfiedInARow; /* @data 0x4b16ea */
extern short arnoMoreScript; /* @data 0x4b16b6 */
extern short willaMoreScript; /* @data 0x4b16be */
extern short shylerMoreScript; /* @data 0x4b16c4 */
extern short judgedPizzaPlace; /* @data 0x4b1710 */
void trollVerdict(short troll, short verdict);
extern short thrownScriptsUsedUp; /* @data 0x4a3dcc */
extern short pizzaButtonsView; /* @data 0x4a3d38 */
extern short arnoPileTop; /* @data 0x4b1714 */
extern short willaPileTop; /* @data 0x4b1716 */
extern short shylerPileTop; /* @data 0x4b1718 */
extern short arnoPile[3]; /* @data 0x4b1722: the three piles' top views */
extern short willaPile[3]; /* @data 0x4b1728 */
extern short shylerPile[3]; /* @data 0x4b172e */
extern short pileGroup; /* @data 0x4b160c */
void showJudgedPizza();
extern short pizzaView7000; /* @data 0x4b15f0 */
void toppingButton(short button);
extern unsigned long lastKeyTime; /* @data 0x4b1824 */
extern short cheatArmed; /* @data 0x4b15e8 */
extern short debugArnoScript; /* @data 0x4a3d9e */
extern short debugWillaScript; /* @data 0x4a3da0 */
extern short debugShylerScript; /* @data 0x4a3da2 */
extern short debugPizzasLeft; /* @data 0x4b1634 */
short pizzaKey(unsigned short key);
void shareToppings();
extern short arnoRejectScript; /* @data 0x4b16b8 */
extern short arnoRejectManyScript; /* @data 0x4b16ba */
extern short willaRejectScript; /* @data 0x4b16c0 */
extern short willaRejectManyScript; /* @data 0x4b16c2 */
extern short shylerRejectScript; /* @data 0x4b16c6 */
extern short shylerRejectManyScript; /* @data 0x4b16c8 */
extern short lastPilingTroll; /* @data 0x4a3d40 */
void pizzaServedTo(short troll, short);
short playAndWait(short sound, short keep);
void sayIntroduction(short which);
extern short debugDraggedView; /* @data 0x4b1672 */
void pizzaButtonClicked(short button);
extern short inPizzaFrame; /* @data 0x4a3d9c: the scene's frame is running */
extern short pizzaWasTried; /* @data 0x4b1674 */
extern short zoombiniJustBrought; /* @data 0x4b166e */
extern unsigned long lastPizzaFidgetTime; /* @data 0x4b1814 */
extern unsigned long pizzaFidgetersUsed; /* @data 0x4b1818 */
extern short pizzaFidgets; /* @data 0x4b181e */
void pizzaFrame();
/* The pizza scene's buttons at each level (copied into pizzaButtons). */
extern SceneButton pizzaButtonsLevel0[13]; /* @data 0x4a35b8 */
extern SceneButton pizzaButtonsLevel1[13]; /* @data 0x4a378c */
extern SceneButton pizzaButtonsLevel2[13]; /* @data 0x4a3960 */
extern SceneButton pizzaButtonsLevel3[13]; /* @data 0x4a3b34 */
extern GroupList pizzaGroups[1]; /* @data 0x4a3d18 */
extern ChosenSnoids *pizzaChosen; /* @data 0x4b15dc */
extern short unusedPizza1; /* @data 0x4b1632 */
extern short unusedPizza2; /* @data 0x4b166a */
extern short unusedPizza3; /* @data 0x4b170a */
extern short unusedPizzaLevel5; /* @data 0x4b162c */
extern short trollsAtLevel; /* @data 0x4b1622 */
extern short pizzaLevelFrom2; /* @data 0x4b162a */
void openPizza();

#endif
