# Debug tools

The port can be built with a small command interpreter (`port/debug/zbdebug.cpp`) for getting into any state quickly: jump to a scene, set the levels, fill the party, edit the saved state, assert on the result. It is port code, not decompiled code, and the game's own cheats stay as they are (their codes are compared by hash, see [Hidden scenes](../gameplay/hidden.md); the tools set the debugging flag directly instead).

## How it is built in

`decomp/game.cpp:gameFrame` calls `zbDebugFrame()` at its start, inside `#ifdef ZB_DEBUG`. Neither the matching build (`uv run match`) nor the Windows 98 rebuild defines `ZB_DEBUG`, so the compiled code is unchanged; `grep -rn ZB_DEBUG decomp/` finds every hook. CMake's `ZB_DEBUG` option (default on) adds `port/debug/` to the game and defines it. `uv run port build` and `run` build it in; `uv run port package` leaves it out of the published site unless given `--debug-tools`.

## Giving commands

Commands are separated by `;` or newlines (`#` starts a comment) and queued to run when the game is at rest: active, no dialog, no scene change pending. A command that changes the scene holds up the ones after it until the new scene is open. The game starts in scene 0 (the intro), so a first command like `scene 1` waits until the intro's frames run.

| Where | How |
| --- | --- |
| command line | `zoombinis --cmd "scene 9; party 8"` (repeatable), `--script commands.txt` |
| `uv run port run` | `--cmd`, `--script`, with `--headless`, `--screenshot`, `--seconds` |
| browser | `?cmd=scene%209` in the URL (repeatable); `zbDebug("scene 9")` in the console |

```sh
uv run port run --headless --seconds 30 --screenshot build/port/shot.bmp \
    --cmd "debug on; level 1 3; party 8; scene 9; wait 2000; assert scene 9; dump"
```

`assert` makes `quit` (and so the process) exit with status 1 if any assertion or command failed; results print to the console prefixed `[zbdebug]`.

## Concepts

The commands name things from the game's own structure; this is what they are (the gameplay chapters have the detail).

- **Scene.** The game is a set of numbered screens, and only one runs at a time (`currentScene`). The table below lists them. Changing scene is what `scene N` does.
- **Group.** The twelve puzzle scenes (7-18) come in four groups of three, played in order: group 1 is scenes 7-9, group 2 is 10-12, group 3 is 13-15 and group 4 is 16-18. Clearing a group's last puzzle (9, 12, 15 or 18) sends the party to a camp or Zoombiniville. Groups 2 and 3 are alternatives: Shelter Rock offers either, and finishing either opens Shade Tree. See [Modules and scenes](../codebase/modules-and-scenes.md#group-structure-of-the-journey).
- **Level.** Each group has its own level, 1 to 4, and every puzzle in the group reads it to choose its rules (what it asks of the player, the number of choices, and so on), so level 1 is the first and level 4 the last, hardest tier. Each time the player clears a group's last puzzle that group's level goes up by one (to a maximum of 4), so a second journey through the same puzzles plays harder. `level 2 3` makes scenes 10-12 play as level 3. The game stores levels as 0-3 in `gameState`, the commands use 1-4.
- **Practice mode.** A mode the map offers (Ctrl-P, then a key 1-4): any puzzle is played at one chosen level whatever the groups' levels are, with a practice party, and nothing is saved to the journey. While it is on, leaving a puzzle goes back to the map. It overrides `level`. `practice L` turns it on (L 1-4) or off (0).
- **Party.** The Zoombinis that set out on a journey, up to 16 (`party()` in `gameState`). Puzzle scenes make one view per party member when they open. Zoombini Isle and the two camps keep their own lists of waiting Zoombinis, which `party` does not touch, so use `party` before jumping to a puzzle scene (7-18), not to fill a camp.
- **Journey map.** Between most scenes the game shows the party travelling over the map (scene 2). `scene N` skips it unless given `map`.
- **Camps and their "unlock bits".** Shelter Rock (4), Shade Tree (5) and Zoombiniville (6) can be visited from the map only after the group before them has been cleared. The game remembers this as bits in `gameState`: for each group, one bit per level it has been left at. Shelter Rock opens on `gameState[0x50] & 0xf` (group 1), Shade Tree on `0x52` (the low nibble is group 2, the high nibble group 3) and Zoombiniville on `0x51` (group 4). `unlock` sets all of these bits, so every camp can be chosen on the map. It doesn't change the levels or the records.
- **Records.** Zoombiniville's monuments (scene 6) each commemorate one journey the player completed: a date, a group and a level. `gameState` holds up to 16 of them, and the town draws one monument per record. The game adds one when the last puzzle of a group is cleared at a level it hasn't recorded yet. `records N` makes the first N slots (in order: group 1 at levels 1-4, then group 2, and so on) into completed journeys dated 1 January 1996, and clears the rest, so `records 0` empties the town and `records 16` fills it.
- **Waiting.** Commands run in order, but only when the game is at rest (running, no dialog, no scene change under way). `scene N` also holds up the commands after it until scene N has opened (it may pass through the journey map first). `wait scene N` holds them until scene N is the current scene, which is for changes that something else makes: a `--click`, a `key`, a puzzle finishing. It continues at once if the game is already in N, and the rest of the queue stalls if N never comes. `wait MS` holds them for that many milliseconds. A `wait` after `scene` is what gives a scene time to start and draw before a `--screenshot` or `assert`.
- **Debug mode.** `debug on` turns on the game's own debugging keys (below), the ones its hashed cheat enables.

## Scenes

| # | Scene | What it is | Notes for `scene N` |
| --- | --- | --- | --- |
| 0 | intro | the logo movie, then on | the game starts here |
| 1 | map | the world map | |
| 2 | journey | the party travelling between places | entered with a route chosen by the game; not useful to jump to |
| 3 | isle | Zoombini Isle: making the Zoombinis | |
| 4 | camp | Shelter Rock, the first camp | |
| 5 | camp 2 | Shade Tree, the second camp | |
| 6 | town | Zoombiniville, with the monuments | draws the records |
| 7 | bridge | Allergic Cliffs (group 1) | |
| 8 | tunnels | Stone Cold Caves (group 1) | |
| 9 | pizza | Pizza Pass (group 1, the last) | |
| 10 | ferry | Captain Cajun's Ferryboat (group 2) | |
| 11 | lilly | Titanic Tattooed Toads (group 2) | |
| 12 | slides | Stone Rise (group 2, the last) | |
| 13 | fleens | Fleens! (group 3) | |
| 14 | hotel | Hotel Dimensia (group 3) | |
| 15 | net | Mudball Wall (group 3, the last) | |
| 16 | caves | The Lion's Lair (group 4) | |
| 17 | smoke | Mirror Machine (group 4) | |
| 18 | maze | Bubblewonder Abyss (group 4, the last) | |
| 19, 21 | catch | a hidden throwing mini-game | see [Hidden scenes](../gameplay/hidden.md) |
| 20 | targets | a hidden target-shooting mini-game | |

The puzzles are described in [Gameplay and the code](../gameplay/index.md), and the table's source, with the module and archive of each, is in [Modules and scenes](../codebase/modules-and-scenes.md).

## Commands

| Command | Does |
| --- | --- |
| `debug on\|off` | turns the game's debugging keys on or off ([below](#debug-keys)) |
| `scene N [map]` | go to scene N (0-21), skipping the journey map unless `map`; holds up the commands after it until N is open |
| `level G L` | group G (1-4) is at level L (1-4): its three puzzles now play at that level |
| `practice L` | practice mode at level L (1-4); 0 leaves it |
| `party N` | the party that sets out: N Zoombinis (up to 16), all on board, each a different kind, named `Debug0`, `Debug1`, … |
| `unlock` | sets the camps' unlock bits, so the map's camp hotspots can be chosen |
| `records N` | the first N of the town's 16 monument records are completed journeys; the rest are cleared |
| `state get OFFSET [SIZE]`, `state set OFFSET VALUE [SIZE]` | read or write 1, 2 or 4 bytes (default 1) of `gameState` at a byte offset, for what has no command ([layout](../codebase/game-state.md)); numbers can be decimal or `0x` hex |
| `cheatcode HASH CODE` | set the game's cheat tracker (`cheatHash`, `cheatCode`) to those values, so that the next `isCheat(HASH, CODE)` test passes. This suits tests made when a hotspot is clicked (the map's hidden scenes, though `scene 19` and `20` are simpler); a key typed afterwards shifts the tracker, so it can't trigger the key-driven ones |
| `key CODE` | gives the game the key CODE as if typed: ASCII for characters (`key 0x4e` is `N`), 1-26 for Ctrl-A to Ctrl-Z. The cheat tracker sees it too: `key 1; key 109; key 105; key 100; key 105; key 32` types the real code `Ctrl-A midi ` (the game's MIDI test) |
| `roster save`, `roster load` | write `gameState` to the current saved game, or read it back |
| `wait MS`, `wait scene N` | hold up the commands after, see [Waiting](#concepts) |
| `get NAME`, `assert NAME VALUE` | print or check a value; the names are `scene`, `pending` (the scene about to open, -1 if none), `practice`, `party` (the count), `debug`, `dialog` (non-zero while a dialog is up), `level1`-`level4` (1-4) and `state:OFFSET[:SIZE]` |
| `dump`, `help`, `quit` | print the main state, list the commands, exit (status 1 if any assertion or command failed) |

Mouse input stays with `--click`.

### Examples

```text
# Allergic Cliffs at level 3, with a party of eight
level 1 3; party 8; scene 7

# The town with a full set of monuments
records 16; scene 6; wait 1000; dump

# Practice mode in Pizza Pass at level 2
practice 2; party 8; scene 9

# A test: it must end in the Mirror Machine
party 8; scene 17; wait 500; assert scene 17; quit
```

### Debug keys

With `debug on` the game's own debugging keys work (`mainloop.cpp:gameKey`), and `key CODE` sends them: `N` (0x4e) draws the paths, `P` (0x50) shows the frame rate, `S` (0x53) toggles sound tests, `^` (0x5e) shows memory statistics, `&` (0x26) draws the palette chart, `]` (0x5d) and `[` (0x5b) turn step mode on and off, Tab (9) chooses every Zoombini on Zoombini Isle, Ctrl-R (18) shows the dragged Zoombini's position, Ctrl-Z (26) fills the views, `@` (0x40) sets the lowest camp bits. In the map `+` and `-` change the practice party size.

## Cautions

- On the web, C: is persisted in IndexedDB: a command that changes the state, and any scene the game then enters, can end up saved over the player's roster. Use a private window for experiments.
- `scene N` skips the game's own setup of that scene; give it a party (`party`) and levels (`level`) first, and note that `#` comments run to the end of a `;`-separated command, not the line.
- The commands were written without a runtime to hand: if one misbehaves, fix it here and in this table.
