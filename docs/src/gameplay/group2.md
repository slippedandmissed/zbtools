# Group 2: ferry, toads and stone

Chosen at Shelter Rock (button 1 → scene 10). Chained: Captain Cajun's Ferryboat → Titanic Tattooed Toads → Stone Rise → Shade Tree.

## Captain Cajun's Ferryboat (scene 10)

> 📷 **Screenshot: `ferry-overview`**
> *The river with Captain Cajun's ferry, the landing places and the Zoombinis waiting on the bank.*
> *Capture:* from Shelter Rock, set out with button 1.

> 📷 **Screenshot: `ferry-crossing`**
> *The ferry mid-river carrying Zoombinis, with Captain Cajun at the helm.*
> *Capture:* place some Zoombinis on the ferry's seats and let it cross.

| | |
| --- | --- |
| Scene / module / archive | 10 / `decomp/ferry.cpp` / `FERRY` (`Ferry.MHK`) |
| Group list | `ferryGroups` → `ferryClicked` |
| Level | `ferryLevel` (0-4), with 16-20 Zoombinis (`forcedFerryCount` overrides) |

| Function | Address | Role |
| --- | --- | --- |
| `openFerry` | `0x41f97c` | loads `Ferry.MHK`; Captain Cajun (the first time sound 1803, then one of `cajunGreetings`); the views; the places for the level (`layOutFerryLevel`) and the party; a hint or greeting |
| `ferryFrame` | `0x41ff89` | once everyone has crossed (`ferryLeaving`), Captain Cajun speaks and the group ends (`sceneDue = 11`) |
| `ferryClicked` | `0x4203b3` | buttons and dragging a Zoombini to a place |
| `layOutFerryLevel` | `0x4211a3` | the level's places and the number of Zoombinis: `SCRB` scripts 1510-1529 |
| `layOutFerry` | `0x420cce` | lays out the places from a script: its first two frames are two lists of parts; each part is a view at a point, 1-3 places to stand, 4-10 scenery |
| `findFerryPlace` | `0x4214ab` | finds a free waiting place |
| `linkFerryPlaces` | `0x42121c` | links places (which seats are paired) |
| `startNextCrosser`, `crosserNotify`, `moveFerryOn` | `0x420f85`, `0x420a60`, `0x4209b8` | each crossing: the ferry's views play scripts 1604-1607 and the Zoombini's script by route |
| `slideFerryViews`, `ferryHelperNotify` | | the ferry sliding across |

**Things to know**: Captain Cajun has pools of lines for greetings, idle remarks and for good and bad placing (`cajunGreetings`, `goodPlacingRemarks`, `badPlacingRemarks`, sound ids `0x708`-`0x723`). The ferry's routes use slots allocated with `returnRoutesUsed`.

## Titanic Tattooed Toads (scene 11)

> 📷 **Screenshot: `toads-overview`**
> *The river with the grid of lily pads and toads.*
> *Capture:* complete the ferry.

> 📷 **Screenshot: `toads-hop`**
> *A Zoombini hopping across lily pads.*
> *Capture:* start the crossing once the board is set.

| | |
| --- | --- |
| Scene / module / archive | 11 / `decomp/lilly.cpp` / `LILLY` (`Lilly.MHK`) |
| Group list | `lillyGroups` → `lillyClicked` |
| Level | `lillyLevel` (set when the scene opens, from the level reached); `LillyActor` is its large view body |

The module is 46 KB, the biggest puzzle (its boundary with `hotel` is found from the data, not from padding).

| Function | Address | Role |
| --- | --- | --- |
| `openLilly` | `0x4281b0` | opens `Lilly.MHK` at the level reached |
| `lillyFrame` | `0x428d84` | per-frame |
| `lillyClicked` | `0x429943` | buttons; drag pieces |
| `setUpBoard` | `0x42cc75` | turns and mirrors the level's grids, leaves out some pieces, deals the squares and fills them in, noting starting squares on the first row (`lillyStarts`) at levels 3 and 4 |
| `dealSquares` | `0x42ca39` | deals out the twelve squares' contents from three sets (3, 4, 5 entries: `squareSetA/B/C`) |
| `turnGrid`, `mirrorGrid` | `0x42d6e9`, `0x42d875` | rotate and reflect a grid |
| `swapSquares` | `0x42c6dc` | swaps the attributes of two squares and replans every actor whose kind either now has |
| `planWay`, `searchLayer`, `searchStep` | `0x42ec2a` | plans an actor's way across (kind 0) or down (1): from each square, the least visited neighbour that's free and of its kind, at most 200 steps |
| `dragLillyPiece` | `0x42d9c5` | dragging a square |
| `addLillyActors`, `lillyNotify30/44/49/54/60/70`, `hopperNotify`, `hopNotify` | `0x42b857`, `0x42f49d` | the hoppers and their script events |
| `checkLillyArrivals` | `0x42e6b5` | have they got across |

## Stone Rise (scene 12)

> 📷 **Screenshot: `stonerise-overview`**
> *The cliff of stones with the Zoombinis waiting at the bottom and the cells above.*
> *Capture:* complete the toads.

> 📷 **Screenshot: `stonerise-lit-path`**
> *A path of lit stones between Zoombinis that share a feature.*
> *Capture:* place Zoombinis in adjacent cells.

| | |
| --- | --- |
| Scene / module / archive | 12 / `decomp/slides.cpp` / `SLIDES` (`Slides.MHK`) |
| Group list | `slidesGroups` → `stoneRiseClicked` |
| Cheat | with a typed code, map hotspot 8 opens scene 20 instead ([Hidden scenes](hidden.md)) |

| Function | Address | Role |
| --- | --- | --- |
| `openStoneRise` | `0x446bf8` | opens `Slides.MHK`, builds the board (all cells empty), the party |
| `stoneRiseFrame` | `0x447171` | per-frame; after finishing goes to **Shade Tree** (`sceneDue = 5`) |
| `stoneRiseClicked` | `0x447528` | buttons and dragging a Zoombini onto a cell |
| `lightPath` | `0x448f02` | lights the path: from the listed cell, walks back along the links: Zoombinis' cells go to 508; a feature stone (510-513) lights when the Zoombinis on either side share its feature; a plain stone lights on level 1 after a Zoombini's cell; the walk ends at an empty or blocked cell or a stone that doesn't light |
| `sharedStone` | `0x449f96` | whether two cells' Zoombinis share a feature, checking from a random feature on (510-513 for hair, eyes, nose, feet; 0 if none) |
| `groupInThrees` | `0x44986f` | groups the party in threes, each sharing a feature with the last where one can (the intended solution) |
| `pairByFeatures`, `seatPair`, `tryMove`, `tryMovesAround`, `followRoutes`, `lightFromStarts` | `0x4494b3`, `0x44abce`, `0x44a674`, `0x44accc` | the board-building and checking helpers |
| `markCellAt`, `linkCells` | `0x44b0fc` | the board's cells |

**Things to know**: this scene and Fleens share a remark set (`stoneRiseSounds` equals `fleensSounds`); finishing sets bit `1 << level` in `gameState+0x52` and sends the party to Shade Tree.
