# Starting the game

## On screen

> 📷 **Screenshot: `start-logo`**
> *The intro logo movie playing full screen (a frame from `Logo025.MOV`), at 640×480.*
> *Capture:* the first seconds after pressing Play in the port; or `uv run assets frames LOGO025 1 701` for stills from the converted movie.

After the logo the game goes to Zoombini Isle (a fresh game) or straight back to the scene the saved game was in.

## Scene facts

| | |
| --- | --- |
| Scene | 0 (the intro) |
| Module | `decomp/town.cpp` (shares the module with Zoombiniville) |
| Movie | `Data\Logo025.MOV` (QuickTime, video in Broderbund's `QkBk` codec; see [Movies](../formats/movies.md)) |
| Before it | [`WinMain`](../codebase/startup.md) sets `pendingScene = 0` |

## Entry points

| Function | Address | Role |
| --- | --- | --- |
| `openIntro` | `0x45c12e` | resets the intro state, installs the whole-screen click group |
| `introFrame` | `0x45c212` | step 0: builds `installDir + "Data\\Logo025.MOV"` and calls `playMovie`; then pumps it with `idleMovie`; when the movie ends, or on a click (`introClicked`), sets `sceneDue = sceneToReturnTo()` |
| `introClicked` | `0x45c391` | any click skips the logo (`introSkip`) |
| `closeIntro` | `0x45c175` | stops the movie, realises the game palette, reloads the Zoombinis (`loadSnoids(0)`) and the dialogs (`loadDialogs`), sets `rosterReady`, shows the cursor |
| `playMovie`, `idleMovie`, `stopMovie`, `loadMovie` | `0x45537f`, `0x455229`, `0x455273`, `0x4552fd` | the movie player over QuickTime (`game.cpp`) |
| `sceneToReturnTo` | `0x454c10` | where to go next |

`sceneToReturnTo` returns the saved scene (and sets `skipJourneyMap`) if the saved game was on the isle, the camps, the town, the map, or in a puzzle with a party; otherwise it returns scene 3, Zoombini Isle.

## Things to know

- **The movie is the only QuickTime use.** If QuickTime isn't installed the game refuses to start (`WinMain` requires 2.3 or later); if the movie *fails* to start (`logoFailed`) the cursor is hidden and the next frame goes straight on to `sceneToReturnTo()`. In the port, `port/glue/quicktime.cpp` plays a scene file instead; see [QuickTime in the port](../port/quicktime.md).
- While a movie shows (`movieShowing`), QuickTime's component manager sees every window message first (`mainWindowProc`).

## Dialogs and saved games

> 📷 **Screenshot: `dialog-keep-party`**
> *The dialog "the current party of zoombinis will be lost if you go to the map" with its LOSE 'EM / KEEP 'EM buttons.*
> *Capture:* from a puzzle, click the map button while Zoombinis are in the party.

> 📷 **Screenshot: `dialog-games`**
> *The saved-games dialog (LOAD / SAVE) listing games.*
> *Capture:* press Ctrl-L (or Ctrl-S) at any time.

> 📷 **Screenshot: `dialog-options`**
> *The options/help dialog with the ON/OFF toggles (music, sound, less/more action, hide cursor, sticky mouse…).*
> *Capture:* press `?` or `/`.

| Function | Address | Role |
| --- | --- | --- |
| `showDialog` | `0x466d7e` | builds a modal dialog out of views and scripts from `ZOOMBINI.MHK` |
| `dialogClick` | `0x46879e` | which of the dialog's 17 hot spots was hit |
| `askNewGame`, `askLoadGame`, `askSaveGame`, `askQuit` | `0x469556`, `0x4695e5`, `0x469627`, `0x469669` | Ctrl-N, Ctrl-L, Ctrl-S, Ctrl-Q |
| `askKeepParty` | `0x466d3d` | the keep-the-party question |

The dialogs' text is `dialogTexts[289]` in `features.cpp` and the toggle names are `toggleTexts` in `town.cpp`. The game keeps one default file (`ZBUser.txt`) and one file per named saved game (`ZOOMnnnn.txt`); see [Game state](../codebase/game-state.md). While a dialog is open `dialogFlags` is non-zero and `gameKey` sends keys to `dialogKey`.
