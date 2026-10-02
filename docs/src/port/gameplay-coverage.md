# Gameplay test coverage map

What the [instrumented gameplay tests](gameplay-tests.md) should cover, what they cover now, and in what order to close the gap. The aim is confidence that the port is **fully playable from the isle to Zoombiniville**, including the side branches: the cases need not be one continuous playthrough (they may set up state with debug commands such as `scene N`, `level G L`, `party N`), but together they must exercise every scene, every transition a player can take, and every way of finishing or failing a puzzle.

Sources: the transitions are read from the code (every `sceneDue`/`pendingScene` assignment, `enterNextScene`, `mapClicked`, `sceneToReturnTo`); the puzzle rules are from the handbook's gameplay chapters, which are summaries of the code and have not all been checked by play. Where this page says "to confirm", the first oracle or case for that scene should settle it, and the page should then be corrected.

## Principles

1. **Every scene has an *opens* case**: it asserts the scene and the state that scene is supposed to start with (the party's size, the level, the scene's own counters), and takes **at least one screenshot**, so that a scene that opens into the wrong state, or renders wrongly, fails. Today the `scenes-*` cases only assert `scene N`.
2. **Assert state, not pictures**, for everything after that: the scene reached, a puzzle's counters, the party, the level, the records. Pictures are for the moments only a picture can show (the win animation, a refusal).
3. **Use the game's own rules, not recorded moves.** A puzzle is won by working out a hidden rule; a case can't know it from the seed alone without breaking whenever the random numbers shift. Each puzzle gets an *oracle* in `port/debug/` that reads the rule from the game's globals and plays the next correct (or deliberately wrong) move through real mouse input.
4. **Real transitions where they are the point.** A case about a transition leaves the scene by the player's own click and asserts where it arrives (and that the journey scene was or wasn't shown); cases about a puzzle may enter it with `scene N`.

## The scenes

| # | Scene | Opens case today | State to assert on opening | Screenshot today |
| --- | --- | --- | --- | --- |
| 0 | intro (logo movie) | `intro-logo` (no assert) | `scene 0`, movie playing; ends by itself or by a click | yes |
| 1 | map | `scenes-camps-and-town`, `map-practice` | open hotspots by the groups' left bits; practice level and party | yes |
| 2 | journey | `journeys` | `journeyFrom`, `journeyTo`, `journeyRoute` | yes |
| 3 | isle | `scenes-camps-and-town`, `isle-make-party-and-send` | made count, queue, `enoughToLeaveChosen` | yes |
| 4 | Shelter Rock | `scenes-camps-and-town`, `camp-drag` | the party's return, camp slots, `campEnoughChosen` | yes |
| 5 | Shade Tree | `scenes-camps-and-town` | as 4, and the book | yes |
| 6 | Zoombiniville | `scenes-camps-and-town`, `town-monuments` | records, `townPartySize` | yes |
| 7-18 | the twelve puzzles | `scenes-group-1..4` (open only) | the level, the party's views, the rule built (`bridgeLevel`, ...) | yes |
| 19, 20 | hidden games | `scenes-hidden` | scene, score 0, throws/ships left | yes |
| 21 | catch (second entry) | none | to confirm how it is reached (`openCatch` shares scenes 19 and 21) | no |

Gaps to close first: richer *opens* asserts for all 22, and a case for scene 21.

## Every transition

"Via journey" means the journey scene (2) is shown between the two scenes (`enterNextScene` sets `viaMap` for most moves out of a puzzle or camp, unless `skipJourneyMap`/`transitionsOn`, practice mode, or the move leaves the map, the journey or the town, or goes to the map; the move *to* the town does show it).

| From | Trigger | To | Condition / what changes | Covered now |
| --- | --- | --- | --- | --- |
| start | `WinMain` | 0 | | `intro-logo` |
| 0 | click or movie end | the saved scene (3, 4, 5, 6, 1, or 7-18 if the party is not empty), else 3 | resumes with `skipJourneyMap`; a fresh game goes to the isle | no |
| 0 | movie fails to start | as above | `logoFailed` | no (the port plays a scene file) |
| 3 | panel button 5 | 1 | | no |
| 3 | panel button 6 | 7 (via journey) | needs `enoughToLeaveChosen` (16 or the population of 625); otherwise a remark and no move | `isle-make-party-and-send` (sends; arrival asserted? no) |
| 3 | Ctrl-N / Ctrl-L | 3 | new game / load a saved game | no |
| 1 | hotspot 1 | 3 | leaves practice | no |
| 1 | hotspot 5 / 12 / 16 | 4 / 5 / 6 | only if group 1 / group 2 or 3 / group 4 has been left (`gameState` bits `+0x50`, `+0x52`, `+0x51`); otherwise nothing happens | no |
| 1 | hotspots 2-4, 6-11, 13-15 | 7-9, 10-12 (note 8 and 9: 20 and 19 with the cheat code), 14, 15, 16-18 | **practice mode only** | `map-practice` (opens the list only) |
| 1 | Ctrl-P, 1-4, `+`/`-` | stays | practice mode on, level, party size | `map-practice` |
| 7 | button 2 (`bridgeGoReady`) | 8 (via journey) | | no |
| 8 | button 2 | 9 (via journey) | | no |
| 9 | button 2 | 4 (via journey) | sets `puzzleLeft = 9` and bit `1 << level` in `gameState[0x50]`; may raise group 1's level | `perfect-clears` (state forced) |
| 4 | button 1 | 10 (via journey) | needs enough chosen, else a random remark | no |
| 4 | button 2 | 13 (via journey) | as above | no |
| 4 | button 3 | 1 | | no |
| 10 | button 2 | 11 | `ferryLeaving` once everyone has crossed | no |
| 11 | button 2 | 12 | `padsArrived` | `toads-place-and-go` (to the hopping only) |
| 12 | finish | 5 (via journey) | `puzzleLeft = 12`, bit in `gameState+0x52` | no |
| 13 | button 2 | 14 | | no |
| 14 | finish | 15 | | no |
| 15 | finish | 5 (via journey) | `puzzleLeft = 15`, bit `<< 4` in `gameState+0x52` | no |
| 5 | button 1 | 16 (via journey) | needs enough chosen | no |
| 5 | other button | 1 | | no |
| 16 | button 2 | 17 | `cavesGoReady` | no |
| 17 | finish | 18 | | no |
| 18 | finish | 6 (via journey) | `puzzleLeft = 18`, bit in `gameState+0x51`, `recordParty` adds a monument | no |
| 6 | button 1 | 1 | | no |
| 7-18 | button 1, then KEEP 'EM | stays | `dialog-keep-party` | `dialog-keep-party` |
| 7-18 | button 1, then LOSE 'EM | 1 | the party is dropped | `dialog-lose-party` |
| any puzzle in practice mode | any exit | 1 | no bookkeeping (`enterNextScene`) | no |
| 2 | click or 300 ticks | `journeyTo` | | `journeys` |
| 19, 20 | their buttons | 1 | | no |
| any | Ctrl-S / options / Ctrl-Q | stays / quits | dialogs | `dialog-save-game`, `dialog-options` |

Not a transition but must work with them: the **saved game** (a game saved in one scene resumes in it, or on the isle if it was in a puzzle with no party) and the **population cap** (625 made: the isle's "population full" ending, reachable in the port's debug mode with Ctrl on button 1).

## The puzzles

For each puzzle the cases to have: **O** opens (asserts), **W** won at the lowest and the highest level (and at the levels between where the rule's shape differs), **X** a wrong move (what it does to the counters and the Zoombini), **L** lost or run out (what the game does when the puzzle can't be finished), **E** the exit to the next scene with the state change.

| # | Puzzle | Levels | Rule an oracle must read | Failure / limit to cover | Cases today |
| --- | --- | --- | --- | --- | --- |
| 7 | Allergic Cliffs | 0-3 (`bridgeLevel`) | `makeBridgeRule`: which side a feature value goes to (`FeatureRules`) | sent back 6 times (`sentBackCount >= 6`: what happens then, to confirm) | O (opens), X (`cliffs-right-and-wrong`) |
| 8 | Stone Cold Caves | rules by level, 16-22 turn-backs | `makeOneFeatureRule`...`makeTwoFeatureRules`: what each door accepts (partly random) | turn-backs allowed run out | O, `caves-door` (a guard speaks) |
| 9 | Pizza Pass | `pizzaButtonsLevel0-3` | each troll's wants (`shareToppings`, `judgePizza`) | pizzas left run out; a refused pizza | X (`pizza-wrong-pizza`), `pizza-wants` |
| 10 | Captain Cajun's Ferryboat | 0-4, 16-20 Zoombinis | `layOutFerryLevel`: which seats are paired | wrong seating remarks | `ferry-load-and-go` (mechanics) |
| 11 | Titanic Tattooed Toads | `lillyLevel` | `setUpBoard`: which row takes which piece | wrong piece (goes back) | X (`toads-wrong-piece`), `toads-place-and-go`, `toads-come-back` |
| 12 | Stone Rise | levels 1-4 | `groupInThrees`, `sharedStone`: the intended solution | wrong cells | `stonerise-place` (mechanics) |
| 13 | Fleens! | `fleensLevel` | `addFleens`: which Zoombini goes with which fleen | wrong pick | `fleens-pick` (mechanics) |
| 14 | Hotel Dimensia | `hotelLevel` (2D, 3D) | `setUpHotelPuzzle`, `fitsRoom`/`fitsRoom3d` | a room it doesn't fit | `hotel-rooms`, `high-levels` (L4 opens) |
| 15 | Mudball Wall | `netLevel` 0-3 (5x5, 5x5x5) | `setUpCodes`: the codes the net needs | wrong code | `mudball-codes` (a code is set) |
| 16 | The Lion's Lair | `cavesLevel` 1-4 | `pickCave`: the stone each Zoombini wants | wrong stone (walks to the right one) | `lion-places`, `lion-right-stone` |
| 17 | Mirror Machine | `smokeLevel` 1-4 | `giveSlotFeatures`, `shareFeature`: the features of the picked Zoombini(s) | wrong cell | `high-levels` (L4 opens) |
| 18 | Bubblewonder Abyss | `mazeLevel` 0-4 (3 with under 5 Zoombinis plays as 4) | `chooseSequence1-5`: the sequence of values | wrong square | `high-levels` (L4 opens) |

So **no puzzle is won in any case today**; seven have a wrong-move case; the transitions between puzzles and camps are covered only by forcing state.

## Branches beyond the main line

| Branch | What to cover |
| --- | --- |
| Group 2 versus group 3 | Shelter Rock's two set-out buttons; both end at Shade Tree (5) with different bits (`+0x52` low nibble versus `<< 4`); the camp's hotspot opens after either |
| Levels rising | the perfect-clear rule (`perfect-clears` covers group 1 at Pizza Pass): the same for groups 2-4, a trip that loses a Zoombini counting for nothing, level 4 not rising, the town's monument recorded by group and level |
| The camps | not enough chosen (the three remarks), set out with enough; picking a Zoombini up and dropping it (`camp-drag`); scrolling; Shade Tree's book; the population cap's remarks |
| The isle | making Zoombinis (`isle-make-party-and-send` makes 16), rename, random, remake from the queue, population full |
| Hidden scenes | the cheat codes (typed by hash: the debug command `cheatcode HASH CODE`) from the map to 19 and 20, playing each to its score; how scene 21 is entered |
| Practice mode | every puzzle reachable at a chosen level and party size, leaving returns to the map, nothing saved (bits and records unchanged) |
| The town | monuments (`town-monuments`), the clock, settling the party (`settleTravellers`), scrolling the six screens, a full town (625) |
| Saved games | save in each kind of scene, load (Ctrl-L), resume at start (`sceneToReturnTo`: the saved scene, or the isle if it was in a puzzle with no party), new game (Ctrl-N) |
| Losing Zoombinis | a party that dwindles to nothing mid-group: what the game does next (to confirm) |
| Dialogs | keep/lose party (covered), save/load/new/quit, options toggles (covered: music) |
| Journeys | every route (`journeyRoute` 1-16), clicking to skip, `transitionsOn`/`skipJourneyMap` |

## Order of work

1. **Opens everywhere.** Strengthen every `scenes-*` case with per-scene state asserts and a screenshot each (a case per scene, so a failure names the scene), add scene 21, and add the *real transition* cases for the camps, the map, the town and the journey (no puzzle has to be solved: set the group bits with `state set`).
2. **One oracle, as the template: Allergic Cliffs**, then the rest one per pull request, each with W (low and high level), X and E. Oracle commands live in `port/debug/` (a command per puzzle that queues the next right or wrong move as real input), documented in [Debug tools](debug-tools.md).
3. **The group chains**: group 1 end to end (7 → 8 → 9 → 4, leaving by the buttons), then groups 2, 3 and 4, then the two camps and the town's record.
4. **The branches** above, smallest first (practice, saved games, the isle), then the hidden games.

A table-driven case (one definition played at levels 1-4, or for each of several seeds) will be wanted for step 2: the cases file would need a way to say it. About 12 puzzles x 4 levels of wins is a few dozen cases of a minute or so each, so the CI job will need sharding (the [progress output](gameplay-tests.md#watching-a-run) shows where the time goes).
