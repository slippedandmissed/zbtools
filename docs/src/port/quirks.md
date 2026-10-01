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
- **32-bit only.** See [Overview](overview.md#limits).

- **The decompilation must not rely on where the compiler put data.** In the original's layout objects sit next to each other, so a pointer written as `&array[19]` (one past the end) could name the next object; in the port it names whatever follows, and the wasm build traps on the first call through it ("null function or function signature mismatch"). `netGroupList` did this to Mudball Wall's input group (`g_4a2e22`, which follows `acrossSpots`): name the object instead. `uv run match-data` still checks it sits at the same address. A scene that crashes on opening is quickly found by visiting each in turn with the [debug tools](debug-tools.md): `party 8; scene 3; wait 3000; assert scene 3; scene 4; …`.
