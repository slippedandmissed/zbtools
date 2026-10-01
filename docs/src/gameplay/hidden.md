# Hidden scenes and debug keys

## Two hidden mini-games (scenes 19-21)

`picker.cpp` contains two scenes that the game never reaches by playing it:

- **Scene 19 / 21 — catching** (`openCatch`, `catchFrame`, `catchClicked`; `Picker.MHK`): set up for "9 throws of 99" with Zoombinis crossing (`catchCrossers`, `nextCatchSendTime`) and a score of how many are caught (`caughtNotify`, `placeCatchScore`).
- **Scene 20 — targets** (`openTargets`, `targetsFrame`, `targetsClicked`): targets that burst when hit (`fireShot`, `placeShot`, `startTarget`, `burstNotify`, `bigTargetOut`, `placeTargetScore`, `driftView`).

> 📷 **Screenshot: `hidden-catch`**
> *The hidden catching game.*
> *Capture:* on the map, type the cheat code that `isCheat(0x469110d3, 0x1e1c32f2)` tests for, then click hotspot 9 (or, in the port's [debug tools](../port/debug-tools.md), `scene 19`).

> 📷 **Screenshot: `hidden-targets`**
> *The hidden targets game.*
> *Capture:* on the map, enter the code that `isCheat(0xc07a877d, 0xedfa7273)` tests for, then click hotspot 8 (or `scene 20`).

`mapClicked` sends hotspot 8 (Stone Rise) to scene 20 and hotspot 9 (Fleens) to scene 19 *when the matching cheat code has just been typed*. Cheat codes are not stored: `noteCheatKey` (`basecamp.cpp`) hashes the last keys typed into two words, and `isCheat(hash, code)` compares against constants, so the code words themselves appear nowhere in the decompilation.

## Debug keys

Typing a code (compared by hash, again) at a space key switches on `debugMessagesOn`, with the name tag "you got it". Then (`mainloop.cpp:gameKey`):

| Key | Does |
| --- | --- |
| Space | with another code: toggles the MIDI test (`midiTest`); in the test, Space with a modifier held steps through the 18 tracks |
| `^` | toggle memory statistics |
| `@` | mark every puzzle group as left at level 1 (unlocks the camps) |
| `=` | redraw the whole game area |
| `&` | draw the palette as a chart of squares |
| `*` | no idle delay (Zoombinis fidget at once) |
| `[`, `]` | step mode off / on; `]` steps one view at a time |
| Ctrl-E, Ctrl-F, Ctrl-X, Ctrl-Y | label views (all / actors only; by position / by id) |
| Tab | put every Zoombini in the party ("ALL in party") |
| `N` | draw the walking paths |
| `P` | toggle the frames-per-second display |
| `S` | sound tests |
| Ctrl-R | show positions |
| Ctrl-Z | fill the views |

On the map, `+`/`-` change the practice party size and `T` then `a`-`p` previews a journey transition. The memory-statistics display (`drawMemoryStats`) and the message log (`dumpMessages` writes `msgNNN.txt`) are the other debug aids.

The `d` switch on the command line is separate: it turns on the engine's debug mode ([Startup](../codebase/startup.md)).
