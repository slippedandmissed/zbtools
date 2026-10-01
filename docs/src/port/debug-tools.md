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

## Commands

| Command | Does |
| --- | --- |
| `debug on\|off` | `debugMessagesOn` and `debugMode`: the debug keys in `gameKey` |
| `scene N [map]` | go to scene N (0-21) by `pendingScene`, skipping the journey map unless `map` |
| `level G L` | group G (1-4) is at level L (1-4) |
| `practice L` | practice mode at level L (1-4); 0 leaves it |
| `party N` | the party: N Zoombinis (up to 16), all on board, of different kinds |
| `unlock` | the camps' unlock bits (`gameState` +0x50, +0x51, +0x52) |
| `records N` | N of the 16 Zoombiniville records |
| `state get OFFSET [SIZE]`, `state set OFFSET VALUE [SIZE]` | read or write 1, 2 or 4 bytes of `gameState` ([layout](../codebase/game-state.md)) |
| `cheatcode HASH CODE` | set `cheatHash` and `cheatCode`, as typing a cheat would |
| `key CODE` | the game's `gameKey` with that key (what `noteCheatKey` also sees) |
| `roster save`, `roster load` | `saveRoster`, `readRoster` |
| `wait MS`, `wait scene N` | hold up the commands after |
| `get NAME`, `assert NAME VALUE` | print or check `scene`, `pending`, `practice`, `party`, `debug`, `dialog`, `level1`-`level4` or `state:OFFSET[:SIZE]` |
| `dump`, `help`, `quit` | print the main state, list the commands, exit |

Mouse input stays with `--click`.

## Cautions

- On the web, C: is persisted in IndexedDB: a command that changes the state, and any scene the game then enters, can end up saved over the player's roster. Use a private window for experiments.
- `scene N` skips the game's own setup of that scene; give it a party (`party`) and levels (`level`) first.
- Levels are stored 0-3 in `gameState` (`puzzleLevels()`); the commands use 1-4, as on the map.
- The commands were written without a runtime to hand: if one misbehaves, fix it here and in this table.
