# Debug tools

The port can be built with a small command interpreter (`port/debug/zbdebug.cpp`) for getting into any state quickly: jump to a scene, set the levels, fill the party, edit the saved state, assert on the result. It is port code, not decompiled code, and the game's own cheats stay as they are (their codes are compared by hash, see [Hidden scenes](../gameplay/hidden.md); the tools set the debugging flag directly instead).

## How it is built in

`decomp/game.cpp:gameFrame` calls `zbDebugFrame()` at its start, inside `#ifdef ZB_DEBUG`. Neither the matching build (`uv run match`) nor the Windows 98 rebuild defines `ZB_DEBUG`, so the compiled code is unchanged; `grep -rn ZB_DEBUG decomp/` finds every hook. CMake's `ZB_DEBUG` option (default on) adds `port/debug/` to the game and defines it. `uv run port build` and `run` build it in; `uv run port package` leaves it out of the published site unless given `--debug-tools`.

## Giving commands

Commands are separated by `;` or newlines (`#` starts a comment) and queued to run when the game is at rest: active, no scene change pending, and no dialog open (while a dialog is open only the commands that look, wait or send input run: `screenshot`, `get`, `assert`, `dump`, `wait`, `click X Y`, `key` and `quit`, which is how a dialog is operated; the rest wait for it to close). "At rest" also means that none of a scene's own callbacks (`open`, `close`, `frame`, `key`) is running: they often run the main loop themselves while they wait for a sound or an animation, which calls the frame hook again from inside them, and a command run there would change the scene under them. `zbdebug.cpp` wraps each scene's callbacks to count them (before the game starts). A command that changes the scene holds up the ones after it until the scene it entered is open. The game starts in scene 0 (the intro), so a first command like `scene 1` waits until the intro's frames run.

| Where | How |
| --- | --- |
| command line | `zoombinis --cmd "scene 9; party 8"` (repeatable), `--script commands.txt` |
| `uv run port run` | `--cmd`, `--script`, with `headless_wasm` or this machine's target, `--screenshot`, `--seconds` |
| browser | `?cmd=scene%209` in the URL (repeatable); `zbDebug("scene 9")` in the console (it queues the text and returns; the game picks it up on its next frame) |

```sh
uv run port run headless_wasm --seconds 30 --screenshot build/port/shot.bmp \
    --cmd "debug on; level 1 3; party 8; scene 9; wait 2000; assert scene 9; dump"
```

`assert` makes `quit` (and so the process) exit with status 1 if any assertion or command failed; results print to the console prefixed `[zbdebug]`.

## Concepts

The commands name things from the game's own structure; this is what they are (the gameplay chapters have the detail).

- **Scene.** The game is a set of numbered screens, and only one runs at a time (`currentScene`). The table below lists them. Changing scene is what `scene N` does.
- **Group.** The twelve puzzle scenes (7-18) come in four groups of three, played in order: group 1 is scenes 7-9, group 2 is 10-12, group 3 is 13-15 and group 4 is 16-18. Clearing a group's last puzzle (9, 12, 15 or 18) sends the party to a camp or Zoombiniville. Groups 2 and 3 are alternatives: Shelter Rock offers either, and finishing either opens Shade Tree. See [Modules and scenes](../codebase/modules-and-scenes.md#group-structure-of-the-journey).
- **Level.** Each group has its own level, 1 to 4, and every puzzle in the group reads it to choose its rules (what it asks of the player, the number of choices, and so on), so level 1 is the first and level 4 the last, hardest tier. Each time the player clears a group's last puzzle that group's level goes up by one (to a maximum of 4), so a second journey through the same puzzles plays harder. (The port changes that, see [Perfect clears per level](quirks.md#perfect-clears-per-level).) `level 2 3` makes scenes 10-12 play as level 3. The game stores levels as 0-3 in `gameState`, the commands use 1-4. `clears G [N]` shows or sets (0-3) the group's count of perfect clears towards its next level, and `assert clearsG N` checks it.
- **Practice mode.** A mode the map offers (Ctrl-P, then a key 1-4): any puzzle is played at one chosen level whatever the groups' levels are, with a practice party, and nothing is saved to the journey. While it is on, leaving a puzzle goes back to the map. It overrides `level`. `practice L` turns it on (L 1-4) or off (0).
- **Party.** The Zoombinis that set out on a journey, up to 16 (`party()` in `gameState`). Puzzle scenes make one view per party member when they open. Zoombini Isle and the two camps keep their own lists of waiting Zoombinis, which `party` does not touch, so use `party` before jumping to a puzzle scene (7-18), not to fill a camp.
- **Journey map.** Between most scenes the game shows the party travelling over the map (scene 2). `scene N` skips it unless given `map`.
- **Camps and their "unlock bits".** Shelter Rock (4), Shade Tree (5) and Zoombiniville (6) can be visited from the map only after the group before them has been cleared. The game remembers this as bits in `gameState`: for each group, one bit per level it has been left at. Shelter Rock opens on `gameState[0x50] & 0xf` (group 1), Shade Tree on `0x52` (the low nibble is group 2, the high nibble group 3) and Zoombiniville on `0x51` (group 4). The map reads such a nibble as a *level number* while the group is still at level 1, so only 0 (not cleared) and 1 (cleared) are valid there: a nibble of 15 makes the map index past its four levels' views and crash (`openMap`, in `runViewScript`). `unlock` therefore sets only the lowest bit of each group's nibble (`0x50` and `0x51` to 1, `0x52` to `0x11`), as the game's own debug key `@` does, so every camp can be chosen on the map. It doesn't change the levels or the records.
- **Records.** Zoombiniville's monuments (scene 6) each commemorate one journey the player completed: a date, a group and a level. `gameState` holds up to 16 of them, and the town draws one monument per record. The game adds one when the last puzzle of a group is cleared at a level it hasn't recorded yet. `records N` makes the first N slots (in order: group 1 at levels 1-4, then group 2, and so on) into completed journeys dated 1 January 1996, and clears the rest, so `records 0` empties the town and `records 16` fills it.
- **Waiting.** Commands run in order, but only when the game is at rest (running, no dialog, no scene change under way). `scene N` also holds up the commands after it until the scene it entered has opened: scene N, or with `map` the journey scene (2) on the way, which stays up until its narration has played (about twenty seconds in the headless port); follow it with `wait scene N` to wait for N itself. `wait scene N` holds them until scene N is the current scene, which is for changes that something else makes: a `--click`, a `key`, a puzzle finishing. It continues at once if the game is already in N, and the rest of the queue stalls if N never comes. `wait MS` holds them for that many milliseconds. A `wait` after `scene` is what gives a scene time to start and draw before a `--screenshot` or `assert`.
- **Debug mode.** `debug on` turns on the game's own debugging keys (below), the ones its hashed cheat enables.

## Scenes

| # | Scene | What it is | Notes for `scene N` |
| --- | --- | --- | --- |
| 0 | intro | the logo movie, then on | the game starts here: do not jump to it while it is showing, which opens the intro a second time and corrupts the heap (set the saved scene during the logo and click instead) |
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
| `transitions on\|off` | the options' "transitions" (Ctrl-T): the game starts with it on, and **off** is what shows the journey screen (scene 2) between scenes |
| `scene N [map]` | leave the current scene (the way the game does, closing it) and open scene N (0-21), skipping the journey map unless `map` (which goes by the journey scene, 2, if the game shows one between the two); holds up the commands after it until the scene entered is open |
| `level G L` | group G (1-4) is at level L (1-4): its three puzzles now play at that level |
| `practice L` | practice mode at level L (1-4); 0 leaves it |
| `party N` | the party that sets out: N Zoombinis (up to 16), all on board, each a different kind, with names made the way the game makes them. It is made again on every later `scene`, as leaving a scene can empty it |
| `unlock` | marks every group as cleared (the lowest unlock bit of each), so the map's camp hotspots can be chosen |
| `records N` | the first N of the town's 16 monument records are completed journeys; the rest are cleared |
| `state get OFFSET [SIZE]`, `state set OFFSET VALUE [SIZE]` | read or write 1, 2 or 4 bytes (default 1) of `gameState` at a byte offset, for what has no command ([layout](../codebase/game-state.md)); numbers can be decimal or `0x` hex. Nothing checks the values: one the game doesn't expect (such as a camp nibble above 1) can crash it |
| `cheatcode HASH CODE` | set the game's cheat tracker (`cheatHash`, `cheatCode`) to those values, so that the next `isCheat(HASH, CODE)` test passes. This suits tests made when a hotspot is clicked (the map's hidden scenes, though `scene 19` and `20` are simpler); a key typed afterwards shifts the tracker, so it can't trigger the key-driven ones |
| `click X Y [press\|move\|release]` | click the mouse at a point of the screen now, or only press, move or release the button (a drag is `click X Y press; wait 300; click X2 Y2 move; wait 300; click X2 Y2 release`) |
| `zoombinis` | list the party's Zoombini views in the order they were made (the party's order): where each one is, its features, name and state. The numbers below are these indexes, from 0 |
| `click zoombini N` | click the middle of Zoombini N's picture |
| `drag zoombini N X Y` | drag Zoombini N so that its feet end at (X, Y): the grab is at its middle and the game puts the feet where the pointer is, less that offset, so X, Y is where it should *stand* |
| `drag zoombini N place K` | the same, to the K-th (from 1) of the scene's placed points (`places`): where a puzzle's drop spots are (a bridge's start, a seat, a room's door) |
| `drag X1 Y1 X2 Y2` | a drag from one point to another (press, three moves with a pause each, release) |
| `toads` | in Toads (scene 11): list the pieces at the left (position, attribute and value) and the board's rows (entry point, left pad's attributes): a piece goes into a row where the pad's attribute (1-3) has its value |
| `places` | list the scene's placed points (where a dragged Zoombini can be claimed: `placedViewPoints`, with which are taken) and its standing spots (`viewPlaces`) |
| `key CODE` | gives the game the key CODE as if typed: ASCII for characters (`key 0x4e` is `N`), 1-26 for Ctrl-A to Ctrl-Z. The cheat tracker sees it too: `key 1; key 109; key 105; key 100; key 105; key 32` types the real code `Ctrl-A midi ` (the game's MIDI test) |
| `roster save`, `roster load` | write `gameState` to the current saved game, or read it back |
| `wait MS`, `wait scene N` | hold up the commands after, see [Waiting](#concepts) |
| `set NAME VALUE`, `seed N` | write a game global by name (below); `seed` seeds the game's random numbers (`randomSeed`), which it otherwise takes from the time |
| `wait until NAME OP VALUE` | hold up the commands after until the value passes the test (`==`, `!=`, `<`, `>`, `<=`, `>=`): `wait until crossingUnderway == 1` |
| `screenshot NAME` | write the screen now as `NAME.bmp` beside the `--screenshot` path (with `uv run shots`, a recipe's `also` pictures) |
| `get NAME`, `assert NAME VALUE` | print or check a value; the names are `scene`, `pending` (the scene about to open, -1 if none), `practice`, `party` (the count), `zoombinis` (the Zoombinis on the screen: a puzzle takes the party into its own list, so `party` reads 0 inside one), `debug`, `dialog` (non-zero while a dialog is up), `transitions`, `due` (the scene the current one is about to leave for), `clock` and `viewclock` (the game's clock in 60ths of a second, and the time since the last input), `level1`-`level4` (1-4) and `state:OFFSET[:SIZE]` |
| `paldiff` | print which entries of the scene's own palette (`loadedPalette`) differ from the one the screen fades to (`targetPalette`): none when a scene's colours are right; for finding scenes that don't copy their palette |
| `dump`, `help`, `quit` | print the main state, list the commands, exit (status 1 if any assertion or command failed) |

`click` and `--click` are the mouse: `click X Y` (on the 640×480 screen) clicks now, and `press`, `move` and `release` after it make a drag with `wait`s between; `--click MS:X,Y` does the same at a time after the start.

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

## Game globals by name

`get`, `set`, `assert` and `wait until` also take the name of any integer global (`char`, `short`, `int` or `long`, signed or not, alone or as an array of one or two dimensions: `name[3]`, `name[1][2]`) that `decomp/`'s headers declare and a source defines: `get bridgeLevel`, `get queuedCount`, `wait until sentBackCount >= 1`, `set clickToDragOption 0`. The table is generated from the headers by `zbtools.debug_globals` (`uv run port build` writes `build/port/generated/debug_globals.inc`), so a renamed global keeps working and a new one appears; structs, pointers and anything else aren't in it. Reading a puzzle's own variables is how a script waits for the game rather than for a time, and how to find out what a puzzle thinks is happening.

## Playing a puzzle

A puzzle is played with drags, and a few things about the game's own input matter:

- **Turn "sticky mouse" off** (`set clickToDragOption 0; set dragClicks 0`, the options' Ctrl-J): with it on (the default) the game treats a release as "pick up" and waits for a second click to drop, so a scripted drag would hang.
- **Aim by where the Zoombini stands.** The game claims a drop spot when the Zoombini's *feet* are within `placeSnapRadius` of it, which is why `drag zoombini N place K` and `drag zoombini N X Y` work in feet, not pointer, coordinates.
- **Pause before the release** (the built-in drags do): the game's drag loop has to see the pointer at the end before the button goes up.
- **Wait for the game, not the clock**: `wait until crossingUnderway == 1`, `wait until placeHeld == 0` and the like. While a drag or a click handler runs, the game runs its own loops, and the commands go on in them.

## Oracles: solving a puzzle by its rule

A puzzle is won by working out a hidden rule, which a recorded list of moves can't do reliably (the rule depends on the random numbers, and the list breaks whenever they shift). An *oracle* is a debug command that reads the rule from the game's own state and plays the next correct (or deliberately wrong) move through real mouse input. They are in `port/debug/oracles.cpp` and are not part of the game. An oracle command is turned into plain commands (`drag ...`, `wait until ...`) at the moment it runs, so it sees the state as it is then; each one that plays a move says what it chose (`cliffs: zoombini 3 to bridge 2 (right)`), and an oracle may add values to `get`/`assert`/`wait until` (`cliffsAcross`, `cliffsWaiting`).

| Command | Does |
| --- | --- |
| `cliffs` | Allergic Cliffs (scene 7): lists every Zoombini with its features, the bridge where the cliff lets it across and whether it is waiting |
| `cliffs pick right\|wrong` | drags the first waiting Zoombini to its right bridge (or the other one); fails if six have been sent back (the cliff takes no more) |
| `cliffs send right\|wrong [N]` | N times (once): waits for the queue and the crossing to clear, then `cliffs pick`; waits a moment after each drop for the game to take it |

| `tunnels` | Stone Cold Caves (scene 8): lists every Zoombini with the doors that let it in (the rule `turnedBackAtDoor`, and at level 0 the pair of doors that is shut) and whether it is waiting |
| `tunnels pick right\|wrong` | drags the first waiting Zoombini to a door that lets it in (or turns it back); fails if there are no turn-backs left (the doors take no more drops) |
| `tunnels send right\|wrong [N]` | N times (once): waits for the guards to be done with the last one, then `tunnels pick` |

Values: `cliffsAcross` (the Zoombinis that have crossed), `cliffsWaiting` (those standing among the waiting ones); `tunnelsIn` (let in), `tunnelsWaiting`, `tunnelsQueued` (entries the guards have yet to deal with).

```sh
level 1 2; party 16; scene 7; wait 6000
cliffs send right 16; wait until cliffsAcross == 16
assert sentBackCount 0; click 618 458; wait scene 8
```

The other puzzles get theirs as their cases are written (see the [coverage map](gameplay-coverage.md)).
