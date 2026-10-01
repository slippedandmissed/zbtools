# Introduction

This handbook documents the decompilation of **Logical Journey of the Zoombinis** (Broderbund, 1996), and everything built around it: the tools, the file formats, the decompiled source, and the port that runs it in a browser. It is written for someone joining the project who wants to find their way around, not for someone who only wants to play.

## What this project is

The disc ships two builds of the game. The project targets the **32-bit Windows 95 build**, `zoombi32.exe`: a PE32 executable compiled with Borland C++ 4.5 (about 634 KB), with its assets in Mohawk archives (`DATA/*.MHK`, the container format of Myst and Living Books) and one intro movie in QuickTime.

| Piece | Where | Status |
| --- | --- | --- |
| Decompiled game and engine | `decomp/` | Every function (about 2,100) is written as C++. 1,915 compile with Borland C++ 4.5 to the original's exact bytes; 176 differ in register choice; 37 are portable stand-ins for hand-written assembly. |
| Game data | `assets/` | Every resource converted to a modern format (PNG, WAV, MIDI, TOML) and packed back into archives identical byte for byte to the disc's. |
| Tooling | `src/zbtools/` | Python tools for extraction, a scripted Windows 98 VM, Ghidra, a function matcher, asset and movie converters, a rebuild and the port. |
| Rebuilt executable | `uv run build` | Linked with the original's linker; runs in the VM. |
| The port | `port/` | The same `decomp/`, compiled unchanged for WebAssembly (and 32-bit native/headless) over SDL2, with a Win32 subset called *miniwin*. |

The long-term goal is source code that builds and runs on modern systems. The game's logic is done; the work that remains is mostly the last near-misses, playing the rebuilt game all the way through, and making the code 64-bit clean.

## How to read this book

- **New here?** Start with [Prerequisites](getting-started/prerequisites.md) and [Setup](getting-started/setup.md), then [How the pieces fit](concepts/overview.md).
- **Decompiling or matching functions?** Read [The workflow](concepts/workflow.md), [Matching](concepts/matching.md), and keep [Matching BCC32](concepts/bcc32-quirks.md) open.
- **Looking for the code behind something you saw in the game?** Go to [Gameplay and the code](gameplay/index.md): it walks through the game in the order a player meets it, with a code map for every screen and puzzle.
- **Learning the engine?** Read [Architecture](codebase/architecture.md) and the chapters after it.
- **Working on the web port?** Read [The port](port/overview.md).
- **Editing this book?** See [About this book](reference/book.md).
- **Poking at assets?** See [Formats](formats/mohawk.md).

## Conventions

- Addresses (`0x46be2e`) are virtual addresses in `zoombi32.exe`. Every decompiled function carries one in a marker comment, so `grep -rn 0x46be2e decomp/` finds it.
- Commands are written `uv run <tool>`; the tools are listed in the [tool reference](reference/tools.md).
- Names are camelCase for functions, variables and fields and PascalCase for types. Names recovered from RTTI are used exactly (`displayPort`, `DIB8Port`). Names that aren't understood yet are after their address (`fn_46be2e`, `g_4a7f58`).
- Block quotes that begin with a camera emoji and `Screenshot: <id>` are **screenshot placeholders**: places where an image of the running game belongs (the first is in [Gameplay and the code](gameplay/index.md)). See [Screenshots](appendix/screenshots.md) for how to capture and add them.

## Legal

The repository doesn't distribute the original game's binaries or its disc. It holds source reconstructed from them (the decompiled code, and the resources converted to modern formats, from which the tools rebuild the original archives), for preservation and interoperability. Building anything needs your own copy of the game. The Cornerstone font and the GeneralUser GS SoundFont have their own licences, noted in the README. The tools' generated report contains the game's disassembly and must never be published.
