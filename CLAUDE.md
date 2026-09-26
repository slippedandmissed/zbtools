# CLAUDE.md

Decompilation of *Logical Journey of the Zoombinis* (Broderbund, 1996, Windows release). The long-term goal is source code that builds and runs on modern systems. See `README.md` for the public-facing overview and setup steps.

## Key facts about the target

- **Main binary (target):** `zoombi32.exe`, the Windows 95 build, extracted by `uv run extract-game` from the game ISO (via `ZBARCHIV.Z`, an InstallShield 3 archive) into `build/zoombi32/`. PE32, ~634 KB, sections `CODE`/`DATA`/`.idata`/`.edata`/`.reloc`/`.rsrc`.
- **Compiler:** Borland C++ 4.x, 32-bit (runtime string `Borland C++ - Copyright 1994 Borland Intl.`, `Borland32` marker, linker version 2.25 = TLINK32). The PE timestamp (2013) is junk, as is common for Borland's linker. Exact version (4.0 / 4.02 / 4.5) and flags not yet confirmed.
- **Imports:** KERNEL32, USER32, GDI32, ADVAPI32, WINMM, VERSION, DSOUND (DirectSound), QTIM32 + CMGR32 (QuickTime for Windows 2.x, 32-bit).
- **Assets:** `DATA/*.MHK` on the disc (`build/disc/DATA/` after extraction), Mohawk archives (`MHWK` magic, `RSRC` directory), read from the CD at runtime. ScummVM's `engines/mohawk/` is the reference for the container format; per-game resource types for Zoombinis are undocumented.
- **Other build:** `ZOOMBINI._EX` on the disc is the Windows 3.1 build (Win16 NE, same compiler family). Useful for cross-reference, not the primary target.
- **Ignore:** installer and autorun files (`MS*`, `_INST32I.EX_`, `SETUP.*`, `INSTALL.EXE`, `AUTORUN.EXE`).

## Constraints

- **Reproducibility:** every setup step a developer needs after cloning must be committed or performed by a script. The only manual inputs are bring-your-own files (the game in `data/`, Windows install media). Exploratory work is fine, but always finish by folding it into a script and documenting it in the README's Setup section.
- **Legal:** `data/` is gitignored and holds the user's ISOs (game disc and Windows 98 SE, default names in `src/zbtools/paths.py`); `.env` holds their Windows product key (`WINDOWS_PRODUCT_KEY`). Never commit game files, extracted assets, Windows media, or large verbatim disassembly of the original binaries.
- Treat `data/` as read-only. Write extracted or derived output to the gitignored `build/` directory.
- Host platform: macOS on Apple Silicon. Rosetta 2 cannot run 16-bit Windows code, so Wine cannot run the game (the QuickTime installer needs 16-bit code); use the emulated VM instead.

## Layout

- `src/zbtools/`: Python tooling package. Each tool is a module exposing `main()`, registered under `[project.scripts]` in `pyproject.toml` and run as `uv run <name>`.
- `src/zbtools/paths.py`: default locations of inputs and outputs, resolved from the repo root. New tools should take their defaults from here and allow overriding them with arguments.
- `build/`: gitignored output (`build/disc/` = disc contents, `build/zoombi32/` = Windows 95 build; VM disk images later).
- **Keep `uv run clean` up to date:** it deletes exactly the paths in `paths.GENERATED` (plus `__pycache__`). Whenever a tool starts generating files outside `build/`, add the path to `GENERATED`.

## Game plan

1. Scripted Windows 98 VM under QEMU that runs the game, for dynamic analysis (QEMU's gdbstub for debugging).
2. Mohawk extractor and catalog of resource types per archive.
3. Ghidra project for `zoombi32.exe`: label imports, identify Borland RTL functions via signatures.
4. Toolchain spike: Borland C++ 4.x 32-bit compiler run headless, plus a diff tool comparing compiled objects against original functions with relocations masked. Time-box this.
5. Decompile function by function, starting with small leaf functions. **Functional (non-matching) first**; byte-matching is a goal where practical, not a gate.
6. Port off QuickTime/DirectSound/256-colour GDI to a modern platform layer.

## Working notes

- Python tooling: stdlib-only where reasonable; add dependencies to `pyproject.toml` via `uv add` when needed.
- Record confirmed findings (formats, compiler flags) in `docs/` as they are established.
