# Startup: `WinMain`

`WinMain` (`0x4546f8`, `decomp/game.cpp`) is reached only through the startup code's table at `0x4a0044` (which is why Ghidra misses it). It does, in order:

1. **Single instance.** Registers `shutDownAtExit` with `atexit`; if a window whose class is the program's own file name exists, brings it forward and returns; also returns if there's a previous instance or the global atom `Zoombini` exists. Otherwise adds the atom (deleted once graphics are up, and at exit). A crash leaves the atom behind.
2. **Mode and hooks.** A command line starting with `d` switches on debug mode. Sets `clockInTicks`, installs the frame hook (`gameFrame`), fatal hook (`shutDownGame`), click hook (`refreshCursor`) and about hook (`showAboutBox`).
3. **The engine, step by step, each failure fatal with a message naming the step:** `osStartup` (the OS layer, given a 0x5f50-byte buffer for its thread stacks), `initTimers`, `initMemory`, a free-memory check (about 1.6 MB, 3.7 MB on anything newer than Windows 3.11, plus a physical-memory check of 6 MB), `initFiles`, `initIni`, `initResources`, `initSound`, then checks for at least one wave and one MIDI device.
4. **Game data.** `enterGameDirectory`, reads the saved-games list (`Zoombini.who`), and `findGameData` (the `config` module: `Zoombi32.CFG`, and asks for the CD if `Data\Zoombini.mhk` isn't there).
5. **Display.** `initGraphics` for 640×480, 256 colours (`checkDisplayMode` finds the smallest mode that fits, falling back to 512×384; `createMainWindow` makes a borderless popup covering the screen), realises the palette, loads the *CornerStone* font at 13 and 18 points.
6. **Game state.** Allocates `gameState` (0xae05 bytes), fills the roster header, applies the player's settings, `initViews`, `loadSnoids` (the Zoombinis' images), and loads cursors 1-5 from `'CURS'` resources.
7. **QuickTime.** `QTInitialize` must report version 2.3 (`0x2300`) or later, and the component manager must start, else a fatal message.
8. **Run.** `pendingScene = 0` (the intro) and loop: `while (mainLoopUpdate() && !quitRequested) mainLoopEvents();`, then `quitSilently`.

## Messages are named strings

Startup messages sit in a table of their own (`0x4a4da4`-`0x4a4f7a`, declared `msg…` in `game.h`) alongside Mac-only messages the Windows build never shows ("Virtual Memory must be turned off.", "Requires Sound Manager 3.1 or later to be installed.", "GetVol failed. Boot drive."). The original pushes each message by its own address instead of addressing literals from a pooled base, so they're arrays in the source.

## Shutdown

`shutDownAtExit` → `shutDownGame` closes every scene (`scenes[i]->close()`), saves the roster if asked, stops sound and movies, and releases the engine. A fatal error calls the fatal hook then exits; `showError` prefixes and formats the message into a message box.

## The "d" switch and debug mode

With `d`, debug mode is on, breakpoints requested by the engine really break (`debugBreak`). A second level, `debugMessagesOn`, is unlocked in game by a typed cheat code (the codes are compared by hash, see `isCheat`) and enables the debug keys in `mainloop.cpp:gameKey` (step mode, view labels, FPS, memory statistics, palette chart, "ALL in party").
