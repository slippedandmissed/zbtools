# The port: overview

`port/` builds **the decompiled game, unchanged** (`decomp/` and `glue/`), for a modern system with CMake and SDL2: WebAssembly in a browser (the main target), headless under Node (for testing), and 32-bit native.

```text
   decomp/*.cpp  ─┐                                     ┌─ web      Emscripten + Asyncify  ─▶ zoombinis.html/.js/.wasm
   glue/ or       ├─▶ library "game" ─┐                 │
   port/glue/*   ─┘   (clang, with    ├─▶ zoombinis ────┼─ headless same wasm under Node, no screen or sound
                       prelude.h)     │                 │
   port/miniwin/ ──▶ library "miniwin"┤                 └─ native   SDL2 from the system (32-bit only)
   port/host/    ──▶ library "host" ──┘
   port/main.cpp ──▶ the entry point: sets up the drives, then calls the game's WinMain
```

## The three layers

| Layer | Directory | Role |
| --- | --- | --- |
| The game | `decomp/`, `glue/` | compiled as is, in Borland's dialect (via `prelude.h`) |
| **miniwin** | `port/miniwin/`, `port/include/` | the Win32 subset the game uses, implemented over SDL ([miniwin](miniwin.md)) |
| the **host** layer | `port/host/` | the few things that differ per target: fibers, yielding to the browser, message boxes ([host layer](host.md)) |

A source in `port/decomp/` or `port/glue/` *replaces* the same-named file in `decomp/`/`glue/`. `port/glue/quicktime.cpp` does ([QuickTime](quicktime.md)); no `port/decomp/` file exists yet, because every decompiled module compiles as it is.

## What runs where

- **Windows and messages**: the game's own message loop, window procedure, timers and hooks run as on Windows.
- **GDI**: software drawing into a 640×480 8-bit screen shown through a simulated system palette.
- **Sound**: waveOut devices mixed into SDL audio; midiOut sent to a General MIDI synth (TinySoundFont + GeneralUser GS).
- **Threads**: Win32 threads and the engine's own fibers switch cooperatively on one host thread.
- **Files**: Windows paths map to drives `C:` (installed game and saves) and `D:` (the CD).

## Commands

```sh
uv run port setup                 # the pinned Emscripten SDK (~1.8 GB) into build/emsdk/, and the SoundFont
uv run port build [web|headless|native] [--debug]
uv run port package               # the site: build/port/site/
uv run port serve [--port 8000]   # then http://127.0.0.1:8000/
uv run port run [--headless] [--seconds N] [--screenshot F.bmp] [--click MS:X,Y]… [--record F.wav]
```

`build` defaults to `web`. Plain CMake works too: `emcmake cmake -S port -B build/port/web && cmake --build build/port/web`.

## Limits

- **32-bit only.** The decompiled code assumes 4-byte `long`s and pointers (it keeps pointers in `long`s in places), so CMake refuses a 64-bit native target unless `-DZB_ALLOW_64BIT=ON` (the game then won't work). WebAssembly is 32-bit. Untangling this is a roadmap item.
- The game assumes Windows 95 behaviour in places; miniwin reproduces it where relied on (see [Quirks](quirks.md)).

## Status

The game starts and reaches Zoombini Isle (the scene most exercised); music plays through the synthesizer; sound effects are mixed but not yet checked by ear; the intro movie plays from its converted scene with its sound.
