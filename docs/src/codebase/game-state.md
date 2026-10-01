# Game state, saved games and the roster

## `gameState`

All persistent state is one block, `gameState` (`char *`, **0xae05 bytes**, allocated in `WinMain`, `snoids.cpp`). It's written to disk as a saved game and read back whole, so its layout *is* the file format. The decompilation reads it through accessor functions and a few raw offsets (the offsets still marked `*(short *)(gameState + 0xca)` are fields not yet given names):

| Offset | Accessor / meaning |
| --- | --- |
| `+0x20` | the "less action / more action" setting (fidgeting) |
| `+0x46` | a counter the town uses to pick its script (`townScript`) |
| `+0x50`, `+0x51`, `+0x52` | which puzzle *groups* have been left at each level, as bits: the camp's unlock conditions (`gameState[0x50] & 0xf`, …) |
| `+0x54` | "a new puzzle group has started" (set on entering scenes 7, 10, 13, 16) |
| `+0x56` | `sceneFlags()`: per puzzle scene, bits 0-3 left at level 0-3, bits 4-7 passed at level 0-3 |
| `+0x62`…`+0xb2` | the **records**: sixteen groups passed (year, month, day, group, level), shown in Zoombiniville's monuments |
| `+0xc0` | `puzzleLevels()`: the level reached in each group (1-4) |
| `+0xca`, `+0xcc` | the scene the player came from; `savedScene()`: the scene to resume in |
| `+0xa1fc` | `waitingParties()`: parties waiting in scenes 3, 4 and 5 |
| `+0xa462` | `savedParty()`: the camp's party |
| `+0xa92e` | `party()`: the Zoombinis setting out (a `Party`: count and 32 `Traveller`s) |
| `+0xa934` | `travellers()` |
| `+0xab94` | `zoombiniCounts()`: how many Zoombinis of each of the 5⁴ = 625 kinds exist (hair, eyes, nose, feet, each 0-4) |

## Zoombinis

A Zoombini is four feature values, packed in a `long` (`Traveller::zoombini`, `Snoid::features[4]`): **hair, eyes, nose, feet**, each 1-5 on screen (0-4 in the counts), so there are 5⁴ = **625 possible Zoombinis**, which is why Zoombiniville fills at 625 (`townFull`) and the camp has 625 `CampSlot`s. A `Traveller` also records where it stands (`place`), whether it's on board and its name (a 10-character name built from vowel sounds, consonants and endings, `snoids.cpp`).

## The player roster and saved games (`roster.cpp`)

- `ZBUser.txt` is the **default user file**, used until the player saves a game under a name; a saved game gets its own file named `ZOOM` plus the next four-digit number (`newSaveFileName`), the name the player typed being kept only in the list. Every file holds `gameState` (checked for version 107 on load).
- `Zoombini.who` (next to the program) is the **saved-game list**: a little-endian version (107), next id, count and 50 slots of a 23-byte name and 9-byte file name. `assets/zoombi32/installed/Zoombini.who` is the installer's initial one, stored as TOML (see [Formats](../formats/sound-images-scripts.md)).
- `readRoster`, `saveRoster`, `readWriteRoster`, `readWriteSavedGames` and `fillRosterHeader` read and write them; `rosterChanged` marks unsaved changes (`enterNextScene` sets it, and `saveRoster` writes it unless the default file is in use). Failures say "Could not Open/Create Roster file."
- The `roster` module also holds Lion's Lair (scene 16): the two share an original source file. See [Modules and scenes](modules-and-scenes.md).

## Parties

The **party** is the set of Zoombinis currently on the journey. Zoombini Isle makes them, the camps store the ones that wait, and most puzzle scenes begin by taking `party()` and creating one view per traveller (`addSmokeSnoids`, `addMazeSnoids`, …). When a journey ends in a scene (`strandParty`), the party **waits there** if the scene is Zoombini Isle, the first camp or the second camp (their `waitingParties()` / `savedParty()` slots), and is otherwise emptied. Leaving a puzzle with Zoombinis still in it first asks whether to keep them (`askKeepParty`: "the current party of zoombinis will be lost if you go to the map").

## Levels

Each puzzle scene reads `sceneLevel()` (1-4: the level of its group, from `puzzleLevels()`) and picks its rules from it; "practice mode" (`practiceLevel`, from the map's practice hotspot) plays a puzzle at a chosen level without touching the saved journey.
