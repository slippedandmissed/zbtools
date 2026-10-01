# Architecture

The game is five layers deep. Each calls only the ones beneath it, and the port replaces the bottom two.

```text
 ┌──────────────────────────────────────────────────────────────────────┐
 │ The game      scenes (puzzles, map, camps), Zoombinis, dialogs, save │  decomp/<scene>.cpp, snoids,
 │               files, input groups, views and scripts                 │  features, view, focus, ...
 ├──────────────────────────────────────────────────────────────────────┤
 │ e2 layer      the game's own wrappers over the engine: handles,      │  e2memory, loading, graphics,
 │               resources, shapes, sounds, fonts, fade, errors         │  sound, anim
 ├──────────────────────────────────────────────────────────────────────┤
 │ Mohawk engine QuickDraw-style ports and regions; Mac-style memory    │  0x4764bc-0x494000: baseport,
 │               and resource managers; audio objects, WaveMix; async   │  newhandle, resourcefile, ...
 │               files; INI reader; timers                              │
 ├──────────────────────────────────────────────────────────────────────┤
 │ Mohawk OS     reference counts, local memory, deferred calls,        │  os_*.cpp  (0x46d754-0x46f7a4)
 │ layer         cooperative threads on fibers, timers, window hooks    │
 ├──────────────────────────────────────────────────────────────────────┤
 │ Win32 / C RTL KERNEL32, USER32, GDI32, WINMM, DSOUND (by name),      │  Borland CW32.LIB, the Windows
 │ QuickTime     QuickTime for Windows 2.x (by name, via SDK glue)      │  DLLs; miniwin + SDL2 in the port
 └──────────────────────────────────────────────────────────────────────┘
```

## Why it looks like a Mac game

*Logical Journey of the Zoombinis* was a Mac title first, and Broderbund's Mohawk engine imitates the Mac Toolbox so that game code could move across. The evidence is everywhere: handles and purgeable memory, a resource manager with typed IDs, regions and `Rect`/`Point`, GrafPort-style ports, transfer modes, `'CURS'`/`'tBMP'`/`'tMID'` resource types, Mac message strings, 16-bit-style `short` arithmetic, and big-endian resources byte-swapped on load. The Windows layer below it is thin. This is also why the port is cheap: it implements Win32 for the engine, and the game never notices.

## What each layer owns

| Layer | Owns | Key objects | Read |
| --- | --- | --- | --- |
| Game | the rules of every puzzle; the journey; saved games | `Scene`, `View`, `Snoid`, `gameState`, input `Group`s | [Game layer](game-layer.md), [Entities](entities.md) |
| e2 | loading and lifetime of game resources | resource handles, `ImageBank`, `SoundEntry` | [Game layer](game-layer.md) |
| Engine: graphics | drawing | `basePort` and subclasses, `DIB`, regions, `Palette`, `Color` | [Graphics](engine-graphics.md) |
| Engine: memory/resources | storage and Mohawk archives | handles, `'BM'` blocks, `'RMap'` maps | [Memory and resources](engine-memory-resources.md) |
| Engine: audio | sound | `audioObj`, `wavebuf`, `wmxMixer` | [OS layer, timers, audio](engine-os-audio.md) |
| Engine: files, INI | the file system | `fileSpec`, `asyncAPI`, INI reader | [OS layer, timers, audio](engine-os-audio.md) |
| OS layer | pseudo-threads, timers, deferred calls | `thread`, `sync`, `DeferLock` | [OS layer, timers, audio](engine-os-audio.md) |

## C++ classes

Only the engine and OS layer are polymorphic (46 classes with RTTI); the game's own logic is plain functions and structs, which is why most game structs carry a `/* +0x.. */` comment and no vtable. See [Classes](classes.md).

## Control flow in one picture

```text
 WinMain
   └─ loop:  mainLoopUpdate()           enterNextScene if pendingScene != -1; handle an event or the mouse
             mainLoopEvents()           handleWaitingMessage (the Win32 queue → mainWindowProc → events)
                └─ frameHook = gameFrame   scenes[currentScene]->frame()   ← each scene's logic runs here
```

See [Startup](startup.md) and [The main loop](main-loop.md).
