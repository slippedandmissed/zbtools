# Modules and scenes

## The scene table

`scenes[22]` (`game.cpp`, `0x4a26e8`) maps a scene number to its `Scene {open, close, frame, key}`. `currentScene` is the running one (`-1` before the first), `pendingScene` the next. The map's hotspots (`picker.cpp:pickHotspot`) are what send the player to a puzzle.

| # | Scene | Player-facing name | Module (`decomp/…`) | Archive (`assets/…`) | Open / frame / click |
| --- | --- | --- | --- | --- | --- |
| 0 | intro | the logo movie, then on | `town.cpp` | (`Data\Logo025.MOV`) | `openIntro` / `introFrame` / `introClicked` |
| 1 | map | the world map | `picker.cpp` | `MAP` | `openMap` / `mapFrame` / `mapClicked` |
| 2 | journey | travelling between places | `xfer.cpp` | `XFER` | `openJourney` / `journeyFrame` / `journeyClicked` |
| 3 | isle | Zoombini Isle (making Zoombinis) | `isle.cpp` | `PICKER` | `openIsle` / `isleFrame` / `featureButtonClicked`, `isleButtonClicked` |
| 4 | camp | Shelter Rock | `basecamp.cpp` | `BASECAMP` | `enterCamp` / `campIdle` / `campButtonClicked` |
| 5 | camp 2 | Shade Tree | `bctwo.cpp` | `BCTWO` | `openCamp2` / `camp2Frame` / `camp2Clicked` |
| 6 | town | Zoombiniville | `town.cpp` | `TOWN` | `openTown` / `townFrame` / `townClicked` |
| 7 | bridge | Allergic Cliffs | `bridge.cpp` | `BRIDGE` | `openBridge` / `bridgeFrame` / `bridgeClicked` |
| 8 | tunnels | Stone Cold Caves | `tunnels.cpp` | `TUNNELS` | `openTunnels` / `tunnelsFrame` / `tunnelsClicked` |
| 9 | pizza | Pizza Pass | `pizza.cpp` | `PIZZA` | `openPizza` / `pizzaFrame` / `pizzaButtonClicked` |
| 10 | ferry | Captain Cajun's Ferryboat | `ferry.cpp` | `FERRY` | `openFerry` / `ferryFrame` / `ferryClicked` |
| 11 | lilly | Titanic Tattooed Toads | `lilly.cpp` | `LILLY` | `openLilly` / `lillyFrame` / `lillyClicked` |
| 12 | slides | Stone Rise | `slides.cpp` | `SLIDES` | `openStoneRise` / `stoneRiseFrame` / `stoneRiseClicked` |
| 13 | fleens | Fleens! | `fleens.cpp` | `FLEENS` | `openFleens` / `fleensFrame` / `fleensClicked` |
| 14 | hotel | Hotel Dimensia | `hotel.cpp` | `HOTEL` | `openHotel` / `hotelFrame` / `hotelClicked` |
| 15 | net | Mudball Wall | `net.cpp` | `NET` | `openNet` / `netFrame` / `netClicked` |
| 16 | caves | The Lion's Lair | `roster.cpp` | `CAVES` | `openCaves` / `cavesFrame` / `cavesClicked` |
| 17 | smoke | Mirror Machine | `game.cpp` | `SMOKE` | `openSmoke` / `smokeFrame` / `smokeClicked` |
| 18 | maze | Bubblewonder Abyss | `maze.cpp` | `MAZE2` | `openMaze` / `mazeFrame` / `mazeButtonClicked` |
| 19, 21 | catch | a hidden throwing mini-game (cheat only) | `picker.cpp` | `PICKER` | `openCatch` / `catchFrame` / `catchClicked` |
| 20 | targets | a hidden target-shooting mini-game (cheat only) | `picker.cpp` | `PICKER` | `openTargets` / `targetsFrame` / `targetsClicked` |

How the table was established: the player-facing names are the strings of `placeNames` (`picker.cpp`), which `pickHotspot` maps to scene numbers (hotspots 2-4 → 7-9, 6-11 → 10-15, 13-15 → 16-18, 5/12/16 → the camps and the town, 1 → the isle). Which module is which puzzle comes from the archive each opens and from its backdrop and its sound set (scenes 16-18 share a sound-pool set, and 13/12 share another). Module names that don't match the puzzle (`net`, `roster`, `game`, `maze`, `picker`) are named after the strings that identified the *module*, not the puzzle it contains.

The module `town` holds scene 0 (the intro) as well as scene 6; `picker` holds the map, the two hidden mini-games, and the practice-mode machinery; `game` holds `WinMain` *and* the Mirror Machine; `roster` holds the saved-game code *and* the Lion's Lair; `isle` was split from `net` by its data. Scene numbers 19-21 are reached only by typing a cheat code on the map.

## Group structure of the journey

| Group | Puzzles | Camp after |
| --- | --- | --- |
| 1 | Allergic Cliffs (7), Stone Cold Caves (8), Pizza Pass (9) | Shelter Rock (4) |
| 2 | Captain Cajun's Ferryboat (10), Titanic Tattooed Toads (11), Stone Rise (12) | Shade Tree (5) |
| 3 | Fleens (13), Hotel Dimensia (14), Mudball Wall (15) | Shade Tree (5) |
| 4 | Lion's Lair (16), Mirror Machine (17), Bubblewonder Abyss (18) | Zoombiniville (6) |

Groups 2 and 3 are alternatives chosen at Shelter Rock (its two set-out buttons go to scenes 10 and 13); finishing either opens Shade Tree. (Taken from each scene's `sceneDue` assignments and `enterNextScene`, which records `puzzleLeft` when the last puzzle of a group sends the party to a camp. The town's monument texts, `featTexts`, are written per group and level.) Each group has four levels (`sceneLevel()` 1-4).

## The 42 game modules

| Address | Module | What it is |
| --- | --- | --- |
| `0x41008c` | `anim` | the opcode animation player (unused by the game) |
| `0x411350` | `sound` | game-level sounds: channels, `'tWAV'`/`'tMID'` by key |
| `0x4121cc` | `buttons` | graphic buttons |
| `0x4124a4` | `focus` | input groups, hover and press tracking |
| `0x413c24` | `jointext` | error-message joining |
| `0x413dc0` | `events` | the 32-entry event ring |
| `0x4144d0` | `graphics` | screen and work ports, palette, saved areas |
| `0x414f30` | `loading` | formatter and "Unable to load" errors, MIDI sound functions |
| `0x415514`–`0x415604` | `random`, `skipstrings`, `nthstring` | RNG (an LCG seeded from `time()`), string helpers |
| `0x415604` | `debug` | breakpoints, starvation warning, main-loop events |
| `0x415a30` | `basecamp` | Shelter Rock, plus wave-sound helpers, wipes, blinds, the cheat tracker |
| `0x418698` | `bctwo` | Shade Tree and its book of waiting Zoombinis |
| `0x41a404` | `bridge` | Allergic Cliffs |
| `0x41c09c` | `roster` | saved players, plus the Lion's Lair |
| `0x41f8cc` | `ferry` | Captain Cajun's Ferryboat |
| `0x42160c` | `fleens` | Fleens! |
| `0x424274` | `hotel` | Hotel Dimensia |
| `0x4281b0` | `lilly` | Titanic Tattooed Toads (lily pads) |
| `0x42f920` | `picker` | the map, practice mode, the hidden mini-games |
| `0x433510` | `maze` | Bubblewonder Abyss |
| `0x439560` | `net` | Mudball Wall; `enterNextScene`; the per-scene remark pools |
| `0x43e620` | `isle` | Zoombini Isle |
| `0x4402c0` | `pizza` | Pizza Pass |
| `0x44695c` | `config` | `Zoombi32.CFG`, `findGameData` |
| `0x446bf8` | `slides` | Stone Rise |
| `0x44b550` | `game` | `WinMain`, the Mirror Machine, cheats, memory statistics |
| `0x455530` | `platform` | the Windows layer: window, procedure, input, cursor |
| `0x456c00` | `snoids` | Zoombini creation, names, drawing, paths, drag |
| `0x45c0f4` | `town` | the intro scene and Zoombiniville |
| `0x45e2d8` | `tunnels` | Stone Cold Caves |
| `0x4623b8` | `mainloop` | one main-loop pass, game keys, cursors, about box |
| `0x463034` | `view` | the view list and scripts |
| `0x465bd0` | `features` | image banks, cels, dialogs, save/load prompts |
| `0x4696f0` | `xfer` | the journey scene |
| `0x46be28` | `e2memory` | the e2 layer over memory and resources |

The OS layer (`os_fixed` … `os_contexts`) and the engine's 149 modules are described in [OS layer, timers and audio](engine-os-audio.md), [Graphics](engine-graphics.md), [Memory and resources](engine-memory-resources.md) and [The engine's modules](engine-modules.md).
