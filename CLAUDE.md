# CLAUDE.md

Decompilation of *Logical Journey of the Zoombinis* (Broderbund, 1996, Windows release). The long-term goal is source code that builds and runs on modern systems. See `README.md` for the public-facing overview.

## Key facts about the target

- **Main binary:** `data/ZOOMBINI._EX`. Uncompressed Win16 **NE** executable, ~940 KB, 191 segments, 4 imported modules, expected Windows version 3.10.
- **Compiler:** Borland C++ 4.x (runtime string `Borland C++ - Copyright 1994 Borland Intl.`; NE linker version 6.1 = TLINK). Exact version (4.0 / 4.02 / 4.5) and flags (memory model, `-2`/`-3`, optimization) not yet confirmed.
- **Support DLLs:** `data/BROD*.DLL`, small Win16 NE libraries from Broderbund. `BRODUTIL.DLL` (~10 KB) is the designated toolchain test target.
- **Assets:** `data/DATA/*.MHK`, Mohawk archives (`MHWK` magic, `RSRC` directory). ScummVM's `engines/mohawk/` is the reference for the container format; per-game resource types for Zoombinis are undocumented.
- **Third-party runtime dependencies:** WinG (`data/SYSTEM/`), QuickTime for Windows (`.MOV`/`.QTC`), MIDI via `BRODMIDI.DLL`.
- **Ignore:** installer and autorun files (`MS*`, `_INST32I.EX_`, `SETUP.*`, `INSTALL.EXE`, `AUTORUN.EXE`, `QTWSETUP*/`).

## Constraints

- `data/` is gitignored and holds the user's copy of the game. **Never commit game files, extracted assets, or large verbatim disassembly** of the original binaries.
- Treat `data/` as read-only. Write extracted or derived output to a separate gitignored directory (e.g. `build/` or `out/`).

## Game plan

1. Run the original in an emulator (DOSBox-X + Windows 3.11, or 86Box + Windows 95) for dynamic analysis.
2. Build a Mohawk extractor and catalog resource types per archive.
3. Ghidra project for `ZOOMBINI._EX`: name Win16 imports by ordinal (KERNEL/USER/GDI/etc.), label Borland RTL functions via signatures, map segments. Each code segment probably corresponds to one original source module.
4. Toolchain spike: Borland C++ 4.x running headless (DOSBox), plus a diff tool that parses OMF `.obj` output and compares against original functions with relocations masked. Time-box this; if it drags, lean harder on non-matching decompilation.
5. Decompile function by function, starting with small leaf functions. **Functional (non-matching) first**; byte-matching is a goal where practical, not a gate.
6. Port off Win16/WinG/QuickTime to a modern platform layer (or a ScummVM-style reimplementation, decided later).

## Working notes

- Win16 calling conventions: `pascal` for exports and window/callback procs, `cdecl` by default. Expect `far` pointers and segment:offset addressing throughout.
- Tooling scripts should be Python 3 unless there's a reason otherwise.
- Record confirmed findings (formats, compiler flags, segment-to-module mappings) in `docs/` as they are established.
