# The port: overview

`port/` builds **the decompiled game, unchanged** (`decomp/` and `glue/`), for a modern system with CMake and SDL2: WebAssembly in a browser (the main target), headless under Node (for testing), and native builds (32- and 64-bit).

```text
   decomp/*.cpp  ─┐                                     ┌─ web      Emscripten + Asyncify  ─▶ zoombinis.html/.js/.wasm
   glue/ or       ├─▶ library "game" ─┐                 │
   port/glue/*   ─┘   (clang, with    ├─▶ zoombinis ────┼─ headless same wasm under Node, no screen or sound
                       prelude.h)     │                 │
   port/miniwin/ ──▶ library "miniwin"┤                 └─ native   SDL2 from the system
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

## Third-party pieces

| Piece | Used for | Licence note |
| --- | --- | --- |
| SDL2 | window, input, audio | zlib |
| stb_truetype | the game's font | public domain |
| [TinySoundFont](https://github.com/schellingb/TinySoundFont) | the MIDI synthesizer | MIT |
| [GeneralUser GS](https://www.schristiancollins.com/generaluser.php) (S. Christian Collins) | the General MIDI SoundFont, downloaded by `port setup` into `build/soundfont/` (32 MB) | free to use and redistribute in software |
| Cornerstone (`CORNER.TTF`) | the game's font, in `assets/zoombi32/installed/` | free for personal use; replace it if you use the project commercially |

All are fetched at pinned versions and checksums (`port.py`, `port/CMakeLists.txt`).

## Limits

- Run so far: the 64-bit Linux and macOS builds (every scene), and the 64-bit Windows build under Wine; the 32-bit Windows build is built but not run. See [Native builds](native.md).
- The game assumes Windows 95 behaviour in places; miniwin reproduces it where relied on (see [Quirks](quirks.md)).

## Status

The game starts and reaches Zoombini Isle (the scene most exercised); music plays through the synthesizer; sound effects are mixed but not yet checked by ear; the intro movie plays from its converted scene with its sound.

## 64-bit targets

The game was compiled for 32-bit Windows and the decompilation keeps that dialect, so a 64-bit build needs care in three places (none is meant to change what BCC32 generates, which `uv run match` checks):

- **Pointers in integers say `LONG_PTR`.** The engine hands out handles that are pointers (files, volumes, timers, threads, sounds, MIDI maps, movies) and passes callback arguments as integers. Those are typed `LONG_PTR` (`UINT_PTR`, `DWORD_PTR` for unsigned ones), as Win32 does; `decomp/zoombinis.h` makes them `long` for Borland C++ 4.5, which lacks the types, and `miniwin/types.h` makes them `long` or `intptr_t` for the port, so a pointer is never cut short and nothing else changes. A handle that is really a number (a resource map's, a memory handle's) stays `long`.
- **The game's `long` stays 32 bits.** On targets where `long` is 64 bits, `miniwin/prelude.h` ends with `#define long int`, after everything the system and miniwin declare has been read, so only the game's code sees it. Its structures keep their sizes, and its `%ld` formats (the text functions in `miniwin/borland.cpp`) read `int`s. Windows and WebAssembly don't need it: `long` is already 32 bits there.
- **No sizes written as numbers.** Structures that hold pointers are bigger, so an allocation of `0xec` bytes or a `memset` of 9 corrupts the heap on a 64-bit target. The code says what it means (`sizeof(View)`, `offsetof(FileName, path) + 1`) with the same value on 32 bits. Valgrind and a sweep of every scene (`scene N; wait 6000`, screenshots) find the rest: a crash or a heap error on opening a scene is usually one of these.

`va_list` isn't a pointer on 64-bit targets either: `va_copy` copies one (the decompilation defines it for Borland, which lacks it), and `formatJoined`, which made a `va_list` out of an array, passes the parts as arguments (functional, not byte-exact).
