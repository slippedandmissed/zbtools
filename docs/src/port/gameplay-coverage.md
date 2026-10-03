# Gameplay test coverage map

What the [instrumented gameplay tests](gameplay-tests.md) should cover, what they cover now, and in what order to close the gap. The aim is confidence that the port is **fully playable from the isle to Zoombiniville**, including the side branches: the cases need not be one continuous playthrough (they may set up state with debug commands such as `scene N`, `level G L`, `party N`), but together they must exercise every scene, every transition a player can take, and every way of finishing or failing a puzzle.

Sources: the transitions are read from the code (every `sceneDue`/`pendingScene` assignment, `enterNextScene`, `mapClicked`, `sceneToReturnTo`); the puzzle rules are from the handbook's gameplay chapters, which are summaries of the code and have not all been checked by play. Where this page says "to confirm", the first oracle or case for that scene should settle it, and the page should then be corrected.

## Principles

1. **Every scene has an *opens* case**: it asserts the scene and the state that scene is supposed to start with (the party's size, the level, the scene's own counters), and takes **at least one screenshot**, so that a scene that opens into the wrong state, or renders wrongly, fails. (Done: the `open-*` cases.)
2. **Assert state, not pictures**, for everything after that: the scene reached, a puzzle's counters, the party, the level, the records. Pictures are for the moments only a picture can show (the win animation, a refusal).
3. **Use the game's own rules, not recorded moves.** A puzzle is won by working out a hidden rule; a case can't know it from the seed alone without breaking whenever the random numbers shift. Each puzzle gets an *oracle* in `port/debug/` that reads the rule from the game's globals and plays the next correct (or deliberately wrong) move through real mouse input.
4. **Real transitions where they are the point.** A case about a transition leaves the scene by the player's own click and asserts where it arrives (and that the journey scene was or wasn't shown); cases about a puzzle may enter it with `scene N`.

## The scenes

| # | Scene | Opens case | State asserted on opening |
| --- | --- | --- | --- |
| 0 | intro (logo movie) | `intro-logo` | `scene 0`, `movieShowing 1` |
| 1 | map | `open-map` | no practice mode, no hotspot picked |
| 2 | journey | `journeys` (entered by a real move, not `scene 2`) | `journeyFrom`, `journeyTo` |
| 3 | isle | `open-isle` | nothing made or waiting, 16 needed to leave |
| 4 | Shelter Rock | `open-shelter-rock` | the party's Zoombinis on screen, `campEnoughChosen` (with 16) |
| 5 | Shade Tree | `open-shade-tree` | the party on screen, `bookCount` |
| 6 | Zoombiniville | `open-zoombiniville` | `townPartySize`, `recordHotspotCount` |
| 7-18 | the twelve puzzles | `open-allergic-cliffs`, `open-stone-cold-caves`, `open-pizza-pass`, `open-ferry`, `open-toads`, `open-stone-rise`, `open-fleens`, `open-hotel`, `open-mudball-wall`, `open-lions-lair`, `open-mirror-machine`, `open-bubblewonder-abyss` | the level of the puzzle's group (groups 1-4 set to levels 1-4, so a mix-up between groups shows), the party's Zoombinis on screen, its counters at zero, go-ready off |
| 19, 20 | hidden games | `open-hidden-catch`, `open-hidden-targets` | scene, score 0, ships left (targets) |
| 21 | catch (second entry) | `open-hidden-catch-21` | scene 21 opens as the catching game (how the game itself reaches it is still to confirm) |

Every scene now has an *opens* case with a picture. Each puzzle's own level number runs on a different base (`bridgeLevel` 0 at group level 1, `lillyLevel` 2 at group level 2, `cavesLevel` 4 at group level 4, ...); the cases assert what the game gives, which is what the puzzle's code expects.


## Every transition

"Via journey" means the journey scene (2) is shown between the two scenes (`enterNextScene` sets `viaMap` for most moves out of a puzzle or camp, unless `skipJourneyMap`/`transitionsOn`, practice mode, or the move leaves the map, the journey or the town, or goes to the map; the move *to* the town does show it).

| From | Trigger | To | Condition / what changes | Covered now |
| --- | --- | --- | --- | --- |
| start | `WinMain` | 0 | | `intro-logo` |
| 0 | click or movie end | the saved scene (3, 4, 5, 6, 1, or 7-18 if the party is not empty), else 3 | resumes with `skipJourneyMap`; a fresh game goes to the isle | `transition-intro-fresh-game`, `transition-intro-resumes-*` (a puzzle with a party, a puzzle without, a camp, the town, the map) |
| 0 | movie fails to start | as above | `logoFailed` | no (the port plays a scene file) |
| 3 | panel button 5 | 1 | | `transition-isle-to-map` |
| 3 | panel button 6 | 7 (via journey) | needs `enoughToLeaveChosen` (16 or the population of 625); otherwise a remark and no move | `isle-make-party-and-send` (to scene 7 through the journey) |
| 3 | Ctrl-N / Ctrl-L | 3 | new game / load a saved game | no |
| 1 | hotspot 1 | 3 | leaves practice | `transition-map-walk` |
| 1 | hotspot 5 / 12 / 16 | 4 / 5 / 6 | only if group 1 / group 2 or 3 / group 4 has been left (`gameState` bits `+0x50`, `+0x52`, `+0x51`); otherwise nothing happens | `transition-map-walk` (locked, then unlocked) |
| 1 | hotspots 2-4, 6-11, 13-15 | 7-9, 10-12 (note 8 and 9: 20 and 19 with the cheat code), 14, 15, 16-18 | **practice mode only** | `transition-practice-mode` (hotspot 2, and a locked camp stays shut) |
| 1 | Ctrl-P, 1-4, `+`/`-` | stays | practice mode on, level, party size | `map-practice`, `transition-practice-mode` |
| 7 | button 2 (`bridgeGoReady`) | 8 (via journey) | | `cliffs-win-level-1..4`, `cliffs-leave-with-one` |
| 8 | button 2 | 9 (via journey) | needs someone let in (`tunnelsGoReady`) and the guards' closing remark done | `tunnels-win-level-1..4`, `tunnels-leave-with-one` |
| 9 | button 2 | 4 (via journey) | needs every troll satisfied (`pizzaGoReady`); sets `puzzleLeft = 9` and bit `1 << level` in `gameState[0x50]`; may raise group 1's level | `pizza-win-level-1..4` (the bit asserted), `perfect-clears` (the level rise, state forced) |
| 4 | button 1 | 10 (via journey) | needs enough chosen, else a random remark | `transition-shelter-rock-set-out-group-2`, `transition-shelter-rock-not-enough` |
| 4 | button 2 | 13 (via journey) | as above | `transition-shelter-rock-set-out-group-3`, `transition-shelter-rock-not-enough` |
| 4 | button 3 | 1 | | `transition-shelter-rock-to-map`, `transition-map-walk` |
| 10 | button 2 | 11 (via journey) | needs at least one aboard (`ferryHasPassengers`); the passengers cross and the rest stay behind | `ferry-win-level-1..4`, `ferry-leave-with-three` |
| 11 | button 2 | 12 (**no journey scene**: the toads and Stone Rise are one place, and the scene sets `skipJourneyMap` itself) | needs one toad across (`lillyGoReady`); leaves with those that arrived | `toads-win-level-1..3`, `toads-dead-end-strands-one`, `toads-place-and-go` |
| 12 | button 2 (once something is lit) | 5 (via journey) | the lit Zoombinis climb and go on; `puzzleLeft = 12`, bit `1 << level` in `gameState+0x52` | `stone-rise-win-level-1..2` (the bit asserted), `stone-rise-misfit-stays-dark` (leaves with the lit one) |
| 13 | button 2 | 14 (via journey) | needs the three picked fleens found (`fleensGoReady`) and the scene played out (`fleensEntered`) | `fleens-win-level-1..4` |
| 14 | button 2 (`hotelGoReady`) | 15 (**no journey scene**, as from the toads) | leaves with those in rooms | `hotel-win-level-1..4`, `hotel-out-of-tries` |
| 15 | button 2 (`netGoAllowed`, once one has crossed) | 5 (via journey) | leaves with those across; `puzzleLeft = 15`, bit `1 << level` `<< 4` in `gameState+0x52` | `mudball-win-level-1..4` (the bit asserted), `mudball-out-of-shots` |
| 5 | button 1 | 16 (via journey) | needs enough chosen | `transition-shade-tree-set-out`, `transition-shade-tree-not-enough` |
| 5 | other button | 1 | | `transition-shade-tree-to-map`, `transition-map-walk` |
| 16 | button 2 | 17 (via journey) | `cavesGoReady` (once one is placed) | `lion-win-level-1..4` |
| 17 | finish | 18 | | no |
| 18 | finish | 6 (via journey) | `puzzleLeft = 18`, bit in `gameState+0x51`, `recordParty` adds a monument | no |
| 6 | button 1 | 1 | | `transition-map-walk` |
| 7-18 | button 1, then KEEP 'EM | stays | `dialog-keep-party` | `dialog-keep-party` |
| 7-18 | button 1, then LOSE 'EM | 1 | the party is dropped | `dialog-lose-party` |
| any puzzle in practice mode | any exit | 1 | no bookkeeping (`enterNextScene`) | no |
| 2 | click or 300 ticks | `journeyTo` | | `journeys`, `transition-journey-click-skips` (a click) |
| 19, 20 | their buttons | 1 | | `transition-hidden-games-leave` (19, 20 and 21) |
| any | Ctrl-S / options / Ctrl-Q | stays / quits | dialogs | `dialog-save-game`, `dialog-options` |

Not a transition but must work with them: the **saved game** (a game saved in one scene resumes in it, or on the isle if it was in a puzzle with no party) and the **population cap** (625 made: the isle's "population full" ending, reachable in the port's debug mode with Ctrl on button 1).

## The puzzles

For each puzzle the cases to have: **O** opens (asserts), **W** won at the lowest and the highest level (and at the levels between where the rule's shape differs), **X** a wrong move (what it does to the counters and the Zoombini), **L** lost or run out (what the game does when the puzzle can't be finished), **E** the exit to the next scene with the state change.

| # | Puzzle | Levels | Rule an oracle must read | Failure / limit to cover | Cases today |
| --- | --- | --- | --- | --- | --- |
| 7 | Allergic Cliffs | 0-3 (`bridgeLevel`) | `makeBridgeRule`: which side a feature value goes to (`FeatureRules`) | sent back 6 times: the cliff takes no more, and with nobody across the go button stays dead, so the party can only leave by the map button (what the code does; to confirm against the original) | O, W at levels 1-4 with 16 Zoombinis (`cliffs-win-level-1..4`), X (`cliffs-wrong-then-right`, `cliffs-right-and-wrong`), L (`cliffs-six-sent-back`), E (`cliffs-win-level-*`, `cliffs-leave-with-one`: leaves with those across) |
| 8 | Stone Cold Caves | rules by level (`tunnelsLevel` 0-3), 16, 18, 20 or 22 turn-backs | `turnedBackAtDoor` over `tunnelRules`, and at level 0 the shut pair of doors (`closedDoorPair`) | out of turn-backs: the doors take no more drops; with nobody let in the go button stays dead, so the party can only leave by the map button (what the code does; to confirm against the original) | O, W at levels 1-4 (16 Zoombinis at levels 1 and 4, 8 at 2 and 3: `tunnels-win-level-1..4`), X (`tunnels-wrong-then-right`), L (`tunnels-out-of-turn-backs`, with the turn-backs set to two), E (`tunnels-win-level-*`, `tunnels-leave-with-one`), `caves-door` (a guard speaks) |
| 9 | Pizza Pass | `pizzaLevel` 0-3: 1, 2, 3, 3 trolls | each troll's wants (`arnoWants`, `willaWants`, `shylerWants`): a pizza of exactly them satisfies it (`judgePizza`) | a refused pizza costs one of the pizzas left (6 or 7), but running out doesn't end the puzzle: the right pizza still satisfies the troll (what the code does; to confirm against the original), so there is no way to fail it, only to leave by the map button | O, W at levels 1-4 (16 Zoombinis at levels 1 and 4, 8 at 2 and 3: `pizza-win-level-1..4`), X (`pizza-wrong-and-partial-then-right`, `pizza-wrong-pizza`), L (`pizza-out-of-pizzas`, the pizzas set to one), E (`pizza-win-level-*`), `pizza-wants` |
| 10 | Captain Cajun's Ferryboat | `ferryLevel` 0-3 here (16 seats in four layouts: a chain of 16, and grids up to six touching places each) | each place's `ferryLinks`: touching places must hold Zoombinis sharing a feature (found by search) | none: a misfit is sent back to wait; the go button works with any passengers, and those left on the bank do not cross (what the code does; to confirm against the original) | O, W at levels 1-4 (16 Zoombinis at levels 1 and 4, 8 at 2 and 3: `ferry-win-level-1..4`), X (`ferry-wrong-then-right`), E (`ferry-win-level-*`, `ferry-leave-with-three`), `ferry-load-and-go` (the crossing mid-way) |
| 11 | Titanic Tattooed Toads | `lillyLevel` 1-4 (level 3 and 4 add pieces that hop down, and a swapping wand) | which rows a toad can cross from: a path of squares with its value to the far column (`setUpBoard`) | a toad put where it can't cross stays on the board for good and its Zoombini is stranded, the rest cross (what the code does; to confirm against the original); at level 3 and 4 toads can also block each other, which the wand is for | O, W at levels 1-3 (`toads-win-level-1..3`; level 3 on a seed whose toads don't block each other), X (`toads-wrong-piece`), L (`toads-dead-end-strands-one`), E, `toads-place-and-go`, `toads-come-back`. **Level 4 is not won**: matching toads to rows is not enough on any of the 14 seeds tried (the swapping wand is needed), nor is level 3 on most seeds |
| 12 | Stone Rise | levels 1-4 (`stoneRiseLevel` 0-3): rows of cells for 1 and 2, a hex lattice with winding routes for 3 and 4 | each feature stone wants the two Zoombinis either side of it to share its feature (the oracle searches for an arrangement); levels 3-4 light by `tryMove` jumps over stones | a Zoombini that misfits leaves its cell unlit; the go button works once anything is lit, and only the lit ones climb (what the code does; to confirm against the original) | O, W at levels 1-2 (16 Zoombinis at level 1, 8 at 2: `stone-rise-win-level-1..2`), X/L (`stone-rise-misfit-stays-dark`), E, `stonerise-place`. **Levels 3 and 4 are not won** (the lighting is by winding routes over a hex lattice, not yet modelled) |
| 13 | Fleens! | `fleensLevel` 0-3 (the hidden rule shifts each feature of a Zoombini by a per-feature amount to make its fleen; levels 2-4 also swap which feature) | which three fleens were picked to stand apart (`pickedFleens`): the right Zoombinis are those they were made from (`fleenViews`) | a Zoombini whose fleen wasn't picked goes in the line and counts for nothing; there is no limit and no way to fail: any Zoombini can be tried until the three are found (what the code does; to confirm against the original) | O, W at levels 1-4 (16 Zoombinis at levels 1 and 4, 8 at 2 and 3: `fleens-win-level-1..4`), X (`fleens-wrong-then-right`), E, `fleens-pick` |
| 14 | Hotel Dimensia | `hotelLevel` 0-3 (floors; 5x5 rooms; 5x5 with rooms boarded up; 5x5x5) | `rowSortFeature`/`columnSortFeature`/`layerSortFeature` and `fitsRoom`/`fitsRoom3d`: the first Zoombini goes anywhere, then each must fit the rows, columns and layers set so far | eleven misplacements (the guide's steps) and the hotel gives up: drops do nothing, and those in can leave (what the code does; to confirm against the original) | O, W at levels 1-4 (`hotel-win-level-1..4`; level 4 on seed 7), X (`hotel-wrong-then-right`), L (`hotel-out-of-tries`), E, `hotel-rooms`, `high-levels` |
| 15 | Mudball Wall | `netLevel` 0-3 (5x5 tiles with two codes; 5x5x5 with three from level 2) | which tile each group of Zoombinis stands behind (`placeGroups`), and the codes that name it (`findCodeEntry`: `codeColumns`, `codeRows`, `codeLayers` in the order `codeOrder1`/`codeOrderHigh` says) | the shots are limited (`revealSteps`: groups + 7, one more at level 4); out of them nothing more can cross, and the go button leaves with those who did (what the code does; to confirm against the original) | O, W at levels 1-4 (`mudball-win-level-1..4`), X (`mudball-miss-costs-a-shot`), L (`mudball-out-of-shots`, the count set to one), E, `mudball-codes` |
| 16 | The Lion's Lair | `cavesLevel` 1-4 (one roster feature at levels 1 and 2, two at 3 and 4) | `pickCave`: the stones that want a Zoombini's values (`cavePlaceValues`) | none: a Zoombini put on a stone that doesn't want it walks to one that does, so the puzzle cannot be failed | O, W at levels 1-4 (16 Zoombinis at levels 1 and 4, 8 at 2 and 3: `lion-win-level-1..4`), X (`lion-wrong-stone-walks-to-right-one`, `lion-places`), E, `lion-right-stone` (the cheer once crashed here) |
| 17 | Mirror Machine | `smokeLevel` 1-4 | `giveSlotFeatures`, `shareFeature`: the features of the picked Zoombini(s) | wrong cell | `high-levels` (L4 opens) |
| 18 | Bubblewonder Abyss | `mazeLevel` 0-4 (3 with under 5 Zoombinis plays as 4) | `chooseSequence1-5`: the sequence of values | wrong square | `high-levels` (L4 opens) |

So **four puzzles (Allergic Cliffs, Stone Cold Caves and Pizza Pass of group 1, and the ferry) are won in cases at every level**, with their wrong-move, limit and exit paths, **the toads at levels 1-3**, **Stone Rise at levels 1-2**, **Fleens**, **Hotel Dimensia** and **Mudball Wall** at every level, so all of group 3, and **the Lion's Lair** at every level; the Mirror Machine and Bubblewonder Abyss are not won yet. The `cliffs`, `tunnels`, `pizza`, `ferry`, `toads`, `stone`, `fleens`, `hotel`, `mud` and `lion` oracles (in [Debug tools](debug-tools.md)) are the template for the rest.

## Branches beyond the main line

| Branch | What to cover |
| --- | --- |
| Group 2 versus group 3 | Shelter Rock's two set-out buttons; both end at Shade Tree (5) with different bits (`+0x52` low nibble versus `<< 4`); the camp's hotspot opens after either |
| Levels rising | the perfect-clear rule: group 1 at Pizza Pass (`perfect-clears`, state forced), groups 2 and 3 through the real exits of Stone Rise and Mudball Wall with their oracles (`perfect-clears-group-2`, `perfect-clears-group-3`: three flawless trips raise the level, and the first records the group at its level for Zoombiniville). Still to cover: group 4 (needs Bubblewonder Abyss), level 4 not rising |
| The camps | not enough chosen (the three remarks), set out with enough; picking a Zoombini up and dropping it (`camp-drag`); scrolling; Shade Tree's book; the population cap's remarks |
| The isle | making Zoombinis (`isle-make-party-and-send` makes 16), rename, random, remake from the queue, population full |
| Hidden scenes | the ships and shots of the targets game (`hidden-targets-controls`) and the exits of both (`transition-hidden-games-leave`). Still to cover: the cheat codes (typed by hash: the debug command `cheatcode HASH CODE`) from the map, a score in either game, how scene 21 is entered |
| Practice mode | `transition-practice-mode` (the map, a locked camp, leaving) and `practice-puzzle-played-through` (a whole puzzle with 16 practice Zoombinis at level 3; leaving returns to the map still in practice with the bits, level and clears untouched) |
| The town | monuments (`town-monuments`), the clock, settling the party (`settleTravellers`), scrolling the six screens, a full town (625) |
| Saved games | `saved-game-round-trip` (save by name from the camp, change the state, load it back from the town), the resumes at start (`transition-intro-resumes-*`), `new-game-resets` and `new-game-cancelled` (Ctrl-N). Still to cover: saving in the other kinds of scene, a game with several saves, the replace and too-many-games prompts. **A `level` debug command right after a save hangs the headless build** (found while writing these; it is the command, not a player action), so the cases change state with `state set` instead |
| Losing Zoombinis | a party that dwindles to nothing mid-group: what the game does next (to confirm) |
| Dialogs | keep/lose party, save, load, new game, quit (`dialog-quit-prompt`: only the prompt, since both answers leave the game) and the options toggle are covered |
| Journeys | every route (`journeyRoute` 1-16), clicking to skip, `transitionsOn`/`skipJourneyMap` |

## Order of work

1. **Opens everywhere** (done: the `open-*` cases, one per scene with state asserts and a picture, scene 21 included) and **the transitions that need no puzzle solved** (done: the `transition-*` cases: the intro's routing, the isle, the map's locked and unlocked hotspots, both camps' set-out buttons and their "not enough" remarks, the journey's click, practice mode, the hidden games' exits; the group bits are set with `state set`). What is left of the table is every puzzle's own exit, which belongs to the puzzle's win case.
2. **One oracle, as the template: Allergic Cliffs**, then the rest one per pull request, each with W (low and high level), X and E. Oracle commands live in `port/debug/` (a command per puzzle that queues the next right or wrong move as real input), documented in [Debug tools](debug-tools.md).
3. **The group chains**: group 1 end to end (7 → 8 → 9 → 4, leaving by the buttons), then groups 2, 3 and 4, then the two camps and the town's record.
4. **The branches** above, smallest first (practice, saved games, the isle), then the hidden games.

A table-driven case (one definition played at levels 1-4, or for each of several seeds) will be wanted for step 2: the cases file would need a way to say it. About 12 puzzles x 4 levels of wins is a few dozen cases of a minute or so each, so the CI job will need sharding (the [progress output](gameplay-tests.md#watching-a-run) shows where the time goes).
