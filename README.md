# Logical Journey Of The Zoombinis &mdash; Decompilation

This is a nostalgic game from my childhood and this repo is my latest attempt to getting it decompiled and runnable from source on modern systems.

The target is the 1996 Windows release by Broderbund. The game is a 16-bit Windows 3.x (NE) executable built with Borland C++ 4.x, with its assets stored in Mohawk archives (the same container format as Myst and Living Books).

## Status

Early days. Nothing is decompiled yet; see the [roadmap](#roadmap) for what's next.

## Getting Started

Download the disk image from [the Internet Archive](https://archive.org/details/logical-journey-of-the-zoombinis) and extract its contents into the gitignored `./data` directory. No game files are committed to this repository; you need your own copy.

After extracting, `data/ZOOMBINI._EX` and `data/DATA/*.MHK` should exist.

## What's on the disc

| File(s) | What it is |
| --- | --- |
| `ZOOMBINI._EX` | The game. Uncompressed despite the name: a Win16 NE executable, ~940 KB, 191 segments, Borland C++ 4.x runtime |
| `BRODFONT.DLL`, `BRODMIDI.DLL`, `BRODPGI.DLL`, `BRODREG.DLL`, `BRODUTIL.DLL` | Broderbund shared support libraries (Win16) |
| `DATA/*.MHK` | Mohawk resource archives, roughly one per puzzle/area |
| `DATA/*.MOV`, `*.QTC` | QuickTime movies |
| `MOHAWK.WIN`, `DATA/MOHAWK.MAC` | Mohawk engine configuration |
| `ZBARCHIV.Z` | InstallShield archive; contents not yet inventoried |
| `SYSTEM/WING*` | Microsoft WinG graphics library (third-party) |
| `QTWSETUP/`, `QTWSET32/` | QuickTime for Windows installers (third-party) |
| `MS*`, `_INST32I.EX_`, `SETUP.*`, `INSTALL.EXE`, `AUTORUN.EXE` | Installer and autorun components (ignored) |

## Approach

1. **Run the original** in an emulator (DOSBox-X with Windows 3.11, or 86Box with Windows 95) for dynamic analysis.
2. **Document and extract the assets.** Write a Mohawk extractor, using ScummVM's `mohawk` engine as a reference for the container format.
3. **Static analysis in Ghidra.** Name Windows API imports (by ordinal), identify Borland runtime library functions by signature, and map the 191 code segments, which probably correspond roughly to the original source modules.
4. **Toolchain spike.** Get Borland C++ 4.x running headless, pin down the exact version and compiler flags, and build a diff tool that compares compiled OMF objects against the original functions with relocations masked. The small `BRODUTIL.DLL` is the test target.
5. **Decompile function by function.** Functional (non-matching) decompilation comes first; byte-matching is a quality bar applied where practical, not a hard requirement.
6. **Port** the decompiled code off Win16/WinG/QuickTime onto a modern platform layer.

## Roadmap

- [ ] Inventory `ZBARCHIV.Z`
- [ ] NE structure dump tool (segments, imports, relocations)
- [ ] Mohawk archive lister / extractor
- [ ] Game running in an emulator
- [ ] Ghidra project with imports and runtime functions labeled
- [ ] Borland C++ toolchain + object diff tool validated on `BRODUTIL.DLL`
- [ ] First decompiled module

## Legal

This repository does not distribute the original game's binaries or assets. It exists for preservation and interoperability, and you must supply your own copy of the game.
