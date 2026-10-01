# Group 1: the first three puzzles

Reached from Zoombini Isle (`isleButtonClicked` button 6 sets `sceneDue = 7`) and chained: Allergic Cliffs → Stone Cold Caves → Pizza Pass → Shelter Rock.

## Allergic Cliffs (scene 7)

> 📷 **Screenshot: `cliffs-overview`**
> *The cliffs: two bridges, upper and lower, with Zoombinis waiting at the left and the buttons at lower right.*
> *Capture:* start a new game, make a party, click button 6, wait for the journey.

> 📷 **Screenshot: `cliffs-sneeze`**
> *A Zoombini turned back by a sneezing cliff.*
> *Capture:* send a Zoombini across the wrong bridge.

| | |
| --- | --- |
| Scene / module / archive | 7 / `decomp/bridge.cpp` / `BRIDGE` (`bridge.mhk`) |
| Group list | `bridgeGroups` → `bridgeClicked` |
| Level | `bridgeLevel` (0-3), chosen from `sceneLevel()` |

**Entry points**

| Function | Address | Role |
| --- | --- | --- |
| `openBridge` | `0x41a506` | loads `bridge.mhk`, sounds, images and scripts; the two placed spots at the bridges' ends; the views; the party; the level's rule |
| `bridgeFrame` | `0x41aa24` | `updateViews`; leaves when `sceneDue` is set and sound 996 is done; fidgets |
| `bridgeClicked` | `0x41af4c` | button 1: map; button 2 (when `bridgeGoReady`): send on to scene 8; item 3: drag a Zoombini to a bridge (limited to six sent back: `sentBackCount >= 6`) |
| `bridgeKey` | `0x41b203` | |
| `closeBridge` | `0x41a9d7` | |

**Where the rules live**

- `makeBridgeRule` (`0x41b812`) builds the level's rule from the party's actual features (`ChosenSnoids`): a rule is a `FeatureRule` (`FeatureRules`, at `0x4ab804`) saying which side a Zoombini with certain feature values is sent to. Level 0 builds 20 single-value masks; the other levels build pair masks (the table `0x12 … 0x45` lists feature pairs) and count how many chosen Zoombinis match each, then pick one that suits the party.
- `bridgeSnoidNotify` (`0x41b453`) is the logic of a crossing: script event 10 starts the crossing script for the Zoombini by how it's going (`crossingEvent` 1000-1016, upper or lower bridge by `crossingBridge`); events 1-2 and 4-5 set `reactingView`; 3 (lower) and 6 (upper) mean it got across; 20 means it was sent back (`sentBackCount`).
- `turnedBack` (`0x41c00c`) and `bridgeTimer` (`0x41a40f`, started by `startBridgeTimer`): helpers for the sent-back Zoombinis and for timing.

**Things to know**

- The module's string `"Upper bridge accepts:"` (a debug message) is what identified it as the cliffs.
- Zoombinis on the bridge are stacked in `lowerViews`/`upperViews`; a cheer plays when all are over, and more fidgets are allowed as the chosen Zoombinis run out (`bridgeFidgetsAllowed`).

## Stone Cold Caves (scene 8)

> 📷 **Screenshot: `caves-overview`**
> *The caves: four doors in the rock face, the characters at them, and Zoombinis queuing.*
> *Capture:* complete Allergic Cliffs.

> 📷 **Screenshot: `caves-remark`**
> *A guard speaking one of its remarks.*
> *Capture:* wait on the screen.

| | |
| --- | --- |
| Scene / module / archive | 8 / `decomp/tunnels.cpp` / `TUNNELS` (`Tunnels.MHK`) |
| Group list | `tunnelsGroups` → `tunnelsClicked` |
| Level | rules by level; the number of turn-backs allowed (16-22) |

| Function | Address | Role |
| --- | --- | --- |
| `openTunnels` | `0x45e441` | the level's turn-backs allowed and rules, `Tunnels.MHK`, the backdrop, images, scripts; views (four placed at the doors, the characters, the buttons); the party; a first remark |
| `tunnelsFrame` | `0x45ea81` | sequencing and fidgets |
| `tunnelsClicked` | `0x45eff0` | buttons and dragging |
| `makeOneFeatureRule` | `0x460e3d` | level 1: counts chosen Zoombinis having each of the 20 feature values, leaves out one count if others remain, looks for a count about half the party's size and picks that value at random as the rule, which the door accepts or refuses at random |
| `makeOneValueRules`, `makeTwoValueRules`, `makeTwoFeatureRules` | `0x461135`, `0x4612b1`, `0x461bec` | the higher levels' rules (two doors' rules, two features) |
| `sendThroughDoors` | `0x45f9c9` | sends up to four waiting Zoombinis (`tunnelQueue`) off through their doors, freeing the four places by the doors |
| `tunnelsSnoidNotify` | `0x45fb50` | the Zoombini's script events |
| `turnedBackAtDoor` | `0x460c41` | a Zoombini refused at a door goes back; counted against the turn-backs allowed |
| `sayTunnelRemark`, `queueRemark`, `tunnelRemarkNotify` | `0x460571` | the guards' remarks: pools `speaker0BackLines`, `speaker0Replies` (sound ids from `0xfa0`) |

**Things to know**: this scene has the most elaborate remarks in the game (speakers with lines for being turned back, replies, and so on, in the `speaker…` pools); its rules use the same `FeatureRule` type as Allergic Cliffs (`FeatureRules` at `0x4b7f18` for the caves).

## Pizza Pass (scene 9)

> 📷 **Screenshot: `pizza-overview`**
> *Pizza Pass: the pizza being assembled in the middle with the topping buttons to its left, and the trolls waiting.*
> *Capture:* complete Stone Cold Caves.

> 📷 **Screenshot: `pizza-trolls`**
> *The trolls Arno, Willa and Shyler, each with a thought bubble of what they want.*
> *Capture:* higher levels have more trolls.

> 📷 **Screenshot: `pizza-yuck`**
> *A troll reacting to a pizza it dislikes.*
> *Capture:* serve a pizza with a topping the troll doesn't want.

| | |
| --- | --- |
| Scene / module / archive | 9 / `decomp/pizza.cpp` / `PIZZA` (`Pizza.MHK`) |
| Group list | `pizzaGroups` → `pizzaButtonClicked`; 13 items in `pizzaButtons` (the topping buttons are items 3-13) |
| Level | `pizzaButtonsLevel0`-`3` pick which buttons exist |

| Function | Address | Role |
| --- | --- | --- |
| `openPizza` | `0x4402c0` | resets state; the level's buttons; `Pizza.MHK` backdrop, images, features, scripts; the pizza, the topping views and the trolls at the level; the toppings and what each troll wants (`shareToppings`); the party; an introduction |
| `pizzaFrame` | `0x4411f2` | sequencing, troll fidgets (`trollFidget`) |
| `pizzaButtonClicked` | `0x441e78` | buttons and topping clicks (`toppingButton`) |
| `shareToppings` | `0x442ea2` | shares the toppings picked (`pickToppings`) among the trolls at random (from level 2 each troll gets at least one); at levels 1 and 3 shows four pizzas made from two toppings the troll with the fewest wants and one each the others want |
| `judgePizza` | `0x44338b` | what troll *n* (0-2) makes of the pizza: 0 if it has one topping the troll doesn't want, 4 if more; else 2 if it has all the troll wants, 1 if not all (3 is never returned) |
| `servePizza` | `0x445153` | a pizza is served: counts down pizzas left; the troll up takes it and the Zoombini at the pizza gets its notify |
| `trollReacts` | `0x444c62` | records the pizza as tried and has the troll whose turn it is react (8020, 9026 or 10030), placed by `placeTrollToppings`; Willa's turn is skipped while `lastPizzaEaten` |
| `trollsEat`, `trollVerdict`, `showJudgedPizza` | `0x444391` | the eating and verdict animations |
| `bringNextZoombini`, `pizzaZoombiniNotify` | `0x445789`, `0x444e0c` | the Zoombini carrying each pizza |
| `placeArnoToppings`, `placeWillaToppings`, `placeShylerToppings` | | draw each troll's wants |
| `sayIntroduction` | | the introduction |

**Things to know**

- The three trolls are **Arno, Willa and Shyler**; the debug string `"Arno 1 0 …"` (a line per troll of eight 0/1 wants) is built by `sprintf` in the module.
- `pizzaTried`/`recordPizzaTried` keep a record of combinations already tried so the same pizza isn't offered twice.
- Finishing the group sends the party to Shelter Rock (`sceneDue = 4`) and sets bit `1 << level` in `gameState[0x50]` (`enterNextScene`, case 9).
