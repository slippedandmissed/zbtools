# Quirks of the port

Things that look odd and aren't bugs.

- **One host thread, many "threads".** Win32 threads and the engine's fibers are all host fibers switched cooperatively. A thread that waits yields; a thread woken at a *higher priority* runs at once (as Windows would preempt). If the game seems to do things in a surprising order, it is almost always the priority wake-up rule.
- **Time is polled in `service()`.** Nothing runs "in the background"; timers, audio and input arrive when the game calls a wait, a message function or a time function. A tight loop in the game that polls none of those would starve the browser; every blocking loop in the engine does call one.
- **Asyncify.** Anything that can reach `hostYield` unwinds and rewinds the stack, so code that can reach it is slower and bigger. Don't be surprised by the code size.
- **The screen is 8-bit.** Palette animation (fades, colour cycling) is real palette animation on the simulated system palette; the framebuffer is converted to RGB on presentation. The simulated system keeps Windows' 20 static colours (10 at each end), so the game's palette starts at index 10 and `PC_RESERVED` entries animate in place.
- **Windows 95 behaviour is reproduced where relied on**: 16-bit `0xFFFF` counts and device ids, a click that activates the window. Don't "fix" these.
- **Case-insensitive paths.** `D:\Data\Zoombini.mhk` finds `DATA/ZOOMBINI.MHK` on any host.
- **Text** is drawn unhinted and unantialiased with the original font (Cornerstone, free for personal use, included for this non-commercial project: replace it if you use the project commercially).
- **The intro movie is not QuickTime** ([QuickTime in the port](quicktime.md)); `qtim_*` selectors the game never calls aren't implemented.
- **Saved games live in IndexedDB** in the browser (`C:`), and in `build/port/data/c/` for native/headless builds (removed only by `uv run clean port-data`).
- **MIDI** goes to a General MIDI synthesizer, not a Windows MIDI device: the game's MIDI map picks that profile ("unknown device (port)").
- **64-bit targets** need the care described in [Overview](overview.md#64-bit-targets).

- **The decompilation must not rely on where the compiler put data.** In the original's layout objects sit next to each other, so a pointer written as `&array[19]` (one past the end) could name the next object; in the port it names whatever follows, and the wasm build traps on the first call through it ("null function or function signature mismatch"). `netGroupList` did this to Mudball Wall's input group (`g_4a2e22`, which follows `acrossSpots`): name the object instead. `uv run match-data` still checks it sits at the same address. A scene that crashes on opening is quickly found by visiting each in turn with the [debug tools](debug-tools.md): `party 8; scene 3; wait 3000; assert scene 3; scene 4; …`.

## Perfect clears per level

The original (the Windows 95 build, "Version 1.0") raises a group's level on the first perfect clear: `recordParty` (`0x459c84`) adds one to the level at `gameState + 0xc0 + group * 2` whenever a party leaves the group's last puzzle with nobody lost in any of the group's three puzzles (the flag at `gameState + 0x54`), and has no counter or threshold. Players expect the difficulty to rise after three perfect clears, so the port asks for three: CMake's `ZB_PERFECT_CLEARS_PER_LEVEL` (default 3, 1 to 4; 1 is the original's behaviour) defines the macro of that name for `decomp/snoids.cpp`, which uses it only inside `#ifdef`, so `uv run match` and the Windows 98 rebuild are unchanged.

The count is kept in bits 14-15 of the visit counter of the group's last puzzle (Pizza Pass `+0x2e`, Stone Rise `+0x36`, Mudball Wall `+0x3c`, Bubblewonder Abyss `+0x44`): `campHint` only touches bits 0-13, so the saved game keeps its layout and old saves start at 0. The count starts again when the level goes up. Zoombiniville's monument for a group and level is still recorded on the first perfect clear at that level, as in the original. The debug command `clears` reads and sets the count. The gameplay case `perfect-clears` guards it: it forces Pizza Pass's go button (`set pizzaGoReady 1`) so that a party leaves flawlessly, and checks that the level stays at 1 after two such trips (with one losing trip in between, which counts for nothing) and goes up on the third, with pictures of the map before and after.
