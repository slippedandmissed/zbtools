# How the pieces fit

```text
                       ┌────────────────────────────────────────────┐
  game CD (data/) ───▶ │ extract-game ─▶ build/disc, build/zoombi32 │
                       └───────┬──────────────────────┬─────────────┘
                               │ zoombi32.exe         │ DATA/*.MHK, *.MOV
              ┌────────────────▼───────────┐   ┌──────▼─────────────────┐
              │ ghidra, runtime-symbols,   │   │ assets extract/pack    │
              │ classes  (understand it)   │   │ (resources as source)  │
              └────────────────┬───────────┘   └──────┬─────────────────┘
                               │ names, types         │ assets/  (PNG, WAV,
              ┌────────────────▼───────────┐          │ MIDI, TOML, scenes)
 Borland CD ─▶│ decomp/*.cpp  (write C++)  │          │
              └───────┬────────────┬───────┘          │
        toolchain     │ match      │ build            │
   (BCC32 under Wine) │ (bytes ==  │ (TLINK32)        │
                      │  original?)▼                  │
                      │     build/rebuild/zoombi32.exe ─▶ VM (Windows 98)
                      │                               │
                      └──────────▶ port/ (CMake, SDL2, miniwin) ◀──────┘
                                       │
                                       ▼
                         WebAssembly site (build/port/dist/browser_wasm)
```

There are four loops, and most work happens in one of them.

1. **Understand the binary.** Ghidra holds the disassembly and decompiler output. `runtime-symbols` names Borland's runtime by matching its libraries against the game; `classes` recovers the engine's C++ classes from RTTI; `ghidra label` applies both, plus every name already chosen in `decomp/`. The original game runs in an emulated Windows 98 PC for dynamic analysis.
2. **Write and verify source.** A function is written as C++ in `decomp/`, marked with its address, compiled with the original compiler (Borland C++ 4.5 under Wine) and compared byte for byte with the original. Matching is the proof of correctness: a function that matches cannot hide a mistake. `match` and `match-data` do this for code and data.
3. **Treat resources as source.** The disc's Mohawk archives are converted into `assets/` in modern formats such that `assets pack` rebuilds the archives exactly. Resources are edited and diffed like code.
4. **Run it.** `build` links the decompiled code into an executable that runs in the VM; `port` compiles the same, unchanged sources against *miniwin*, a Win32 subset over SDL2, for the browser.

## Why this particular shape

- **The decompiled code is portable C++ that matches.** Both goals hold at once. Where only machine code could reproduce the original (inline assembly), the portable equivalent is marked *functional* instead. Nothing in `decomp/` uses inline assembly, `__emit__` or pseudo-registers, which is what lets `port/` compile it with clang unchanged.
- **Everything is reproducible.** Every setup step is a script; the only manual inputs are the bring-your-own files. Findings live in these docs or in code comments, not in someone's head.
- **The game is layered like its history.** It was written for the Mac and ported: a QuickDraw-style engine (the *Mohawk engine*) sits under the game, and the game sits on a thin Windows layer. The port exploits that: it replaces Windows, not the game. See [Architecture](../codebase/architecture.md).

## Vocabulary you'll meet

| Term | Meaning |
| --- | --- |
| match / matching | a decompiled function compiles to exactly the original's bytes (linker-filled fields masked) |
| near-miss | compiles to almost the original's bytes, nearly always only register allocation |
| functional | complete and portable, but not byte-exact by design (the original used assembly) |
| marker | the `/* @zoombi32 0x… */` comment before each function; the link to the binary |
| module | one original object file; `decomp/<module>.cpp` |
| Mohawk | Broderbund's engine and its archive format (`.MHK`) |
| scene | one screen of the game (a puzzle, the map, a camp) with open/close/frame/key callbacks |
| view | an animated thing on screen, driven by a script |
| snoid / Zoombini | a character; "snoid" is the code's name |
| miniwin | the Win32 subset the port implements |
