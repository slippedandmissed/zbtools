# Logical Journey Of The Zoombinis &mdash; Decompilation

This is a nostalgic game from my childhood and this repo is my latest attempt to getting it decompiled and runnable from source on modern systems.

The target is the 1996 Windows release by Broderbund. The disc ships two builds of the game, and this project targets the **32-bit Windows 95 build** (`zoombi32.exe`, a PE32 executable built with Borland C++ 4.x). Game assets are stored in Mohawk archives (the same container format as Myst and Living Books).

## Status

Early days. Nothing is decompiled yet; see the [roadmap](#roadmap) for what's next.

## Setup

Supported hosts: macOS on Apple Silicon (tested) and Linux (should work, untested).

### Prerequisites

- [uv](https://docs.astral.sh/uv/) for the Python tooling
- [QEMU](https://www.qemu.org/) for the Windows 98 VM: `brew install qemu` on macOS; `qemu-system-x86` and `qemu-utils` (Debian/Ubuntu) or `qemu-system-x86` and `qemu-img` (Fedora) on Linux
- [mtools](https://www.gnu.org/software/mtools/) for editing the setup floppy image: `brew install mtools`, or the `mtools` package on Linux
- [7-Zip](https://www.7-zip.org/) for reading the Borland C++ CDs: `brew install sevenzip`, or the `7zip` package on Linux
- JDK 21, for Ghidra: `brew install openjdk@21`, or `openjdk-21-jdk` (Debian/Ubuntu) / `java-21-openjdk-devel` (Fedora)
- A C/C++ compiler and `make`, to build Ghidra's native decompiler where its release doesn't include one (e.g. Apple Silicon): `xcode-select --install`, or `build-essential` on Linux
- Wine, to run the Borland compiler: on macOS the tools download a pinned build into `build/wine/` themselves (it needs [Rosetta 2](https://support.apple.com/en-us/102527)); on Linux, install the `wine` package

The tools tell you if anything is missing and how to install it.
- Your own copies of the game and Windows 98 SE (see below)

### Bring-your-own files

No game files or Windows media are committed to this repository; you need your own copies. Put them in the gitignored `data/` directory under these names (other paths can be passed to the tools as arguments):

| File | What it is |
| --- | --- |
| `data/Logical Journey of the Zoombinis.iso` | The game CD, e.g. from [the Internet Archive](https://archive.org/details/logical-journey-of-the-zoombinis) |
| `data/Windows 98 Second Edition.iso` | Windows 98 SE install CD, for the emulated PC |
| `data/Borland C++ 4.5.iso` | Borland C++ 4.5 CD, for the compiler the game was built with. (4.52 generates identical code and also works, as `data/Borland C++ 4.52.iso`; see `docs/findings.md`) |

Then create a gitignored `.env` file in the repo root with your Windows product key:

```sh
WINDOWS_PRODUCT_KEY=XXXXX-XXXXX-XXXXX-XXXXX-XXXXX
```

### Extract the game

```sh
uv run extract-game
```

This copies the disc's contents into `build/disc/` and unpacks the Windows 95 build from the InstallShield archive `ZBARCHIV.Z` into `build/zoombi32/`. Pass `--iso PATH` to use a disc image from somewhere else.

To inspect an InstallShield archive directly, `uv run unpack-isz build/disc/ZBARCHIV.Z` lists its contents (add an output directory to extract them).

### Windows 98 VM

The original game runs in an emulated Windows 98 PC under QEMU. Install Windows once:

```sh
uv run vm install
```

This installs Windows 98 SE from your ISO using an answer file, with the product key from `.env`. It takes 30-60 minutes and opens a QEMU window you can watch; the VM powers itself off when setup has finished. Closing the window or pressing Ctrl-C aborts the install and deletes the partial disk.

**Depending on your Windows CD, setup may stop on a few wizard pages** (license, product key, user information) with the answers already filled in. OEM and upgrade CDs do this by design; just click Next on each one. At the end, the script answers Windows' logon prompt itself (with a blank password, so it never appears again) and the VM shuts down.

Then install QuickTime and the game into it:

```sh
uv run vm install-game
```

This takes about a minute and needs no input: the VM boots, runs an installer from a generated CD, and powers itself off. After that:

```sh
uv run vm run          # boot Windows 98 with the game disc in drive D:
uv run vm reset        # discard every change made since the installs
uv run vm screenshot   # save a PNG of the running VM's screen
```

To play, type `zoombi32` in the Start menu's Run box.

The VM's disk is a stack of read-only layers: the Windows install (`build/vm/win98-base.qcow2`), then QuickTime and the game (`build/vm/win98-game.qcow2`). The VM runs from a throwaway copy-on-write overlay on top (`build/vm/win98.qcow2`), so `vm reset` gets you back to a freshly installed game in seconds. `--force` redoes either install.

### Borland C++ toolchain

```sh
uv run toolchain setup
```

Copies the compiler, linker, libraries and headers from each Borland C++ CD in `data/` into `build/toolchain/<release>/`, then compiles, links and runs a test program with each release under Wine. The first run downloads Wine on macOS (~180 MB). After that:

```sh
uv run toolchain check                       # re-run the test program with each release
uv run toolchain run 4.5 BCC32 -c foo.c      # run any Borland tool, in the current directory
```

Under Wine, release 4.5 is drive `T:` and 4.52 is drive `U:` (e.g. `T:\BC45\INCLUDE`); host files are on `Z:`.

### Ghidra

```sh
uv run ghidra setup                  # import and analyse zoombi32.exe (a few minutes)
uv run ghidra open                   # browse the project in Ghidra's GUI
uv run ghidra decompile 0x46be2e     # print Ghidra's C for one function
```

`setup` downloads a pinned Ghidra release into `build/ghidra/` (building its native decompiler first if the release has none for your machine), imports `zoombi32.exe` into a project in `build/ghidra/project/`, runs Ghidra's auto-analysis and writes every function it found to `build/ghidra/functions.json`. Close the project in the GUI before running `decompile`, which opens it headlessly.

To name what the tools can recover: the Borland runtime-library functions (strcpy, memcpy, the C++ support code, ...), and the game's C++ classes, whose names, base classes, vtables, constructors and destructors survive in its RTTI:

```sh
uv run runtime-symbols      # find them: matches the toolchain's libraries against the game
uv run classes              # recover C++ classes from RTTI: names, hierarchy, vtables, constructors
uv run ghidra label         # apply both to the Ghidra project, with QuickTime's SDK glue and the
                            # names of functions decompiled in decomp/, the types and globals in
                            # decomp/zoombinis.h, and set calling conventions (never overwrites
                            # names or types you've set by hand); also fixes functions
                            # Ghidra cut short at a switch table or a breakpoint
```

Work you do in Ghidra's GUI (names, comments, types) lives in the project, so no default `clean` removes it; `uv run ghidra setup --force` recreates the project from scratch.

### Matching decompiled functions

Decompiled code lives in `decomp/` and is C++, like the game (it uses C++ objects and exceptions). Each function that should reproduce the game's code is preceded by a marker comment with its address in `zoombi32.exe`; methods are marked the same way (`void Widget::set(long v)`):

```c
/* @zoombi32 0x0046be2e */
void fn_46be2e(long value)
{
    g_4a7f58 = value;
}
```

```sh
uv run match                      # check every marked function in decomp/
uv run match decomp/platform.cpp -r 4.5 --flags "-O2"   # one file, one release, extra BCC32 options
```

Functions are named for what they do once that's clear (`isMousePresent`), and after their address until then (`fn_46be2e`; globals `g_4a7f58`); the marker keeps the address either way. Rerun `uv run ghidra label` after renaming to carry the names into Ghidra.

A function that's written but not yet an exact match is marked `/* @zoombi32-nonmatching 0x... */`: it's still compiled and reported with how close it is, but doesn't fail the run. The decompiled code is portable C++, so a function the original wrote in machine code (inline assembly) is written portably and marked `/* @zoombi32-functional 0x... */`: complete, but not byte-exact by design.

`match` compiles each file with Borland C++ 4.5 and the game's usual options (`-p -k-`; a file can set its own with a `/* @flags ... */` comment, as some modules were built differently) and compares every marked function byte for byte with the original, ignoring the fields the linker fills in (addresses and call targets); a call to another marked function in the same file must go to that function's address in the game. Mismatches are shown as side-by-side disassembly; the command exits with status 1 if anything differs. It needs `uv run extract-game` and `uv run toolchain setup` first.

### Choosing what to decompile, and tracking progress

```sh
uv run worklist                # game functions ready to decompile next, smallest first
uv run worklist --region engine
uv run worklist --module bridge  # just one source module
uv run report --open           # progress report in your browser
uv run modules                 # the game's source modules, with the evidence for each
```

The game's code is divided into its original source modules (object files) in `decomp/modules.toml`, a map maintained by hand: TLINK32 pads each module's code with zeros to a 4-byte boundary, and `uv run modules` checks the map against that padding and shows what each module's code refers to (strings, Windows functions), which is how modules get their names. Decompiled functions go in `decomp/<module>.cpp`.

A function is *ready* when everything it calls directly is done (matched, identified runtime code, or outside the region you're working on). `report` writes `build/report/index.html`: statistics per region (by function and by bytes), every function with its status and name (as decompiled, else Ghidra's), and for each decompiled function its original machine code, C++ and recompiled machine code side by side, with differences highlighted and calls and globals annotated with their names. It contains the game's disassembly, so keep it local.

Both need `uv run ghidra setup`, `uv run runtime-symbols` and `uv run classes` to have run.

### Cleaning up

```sh
uv run clean [CATEGORY ...]
```

Deletes generated files by category, never touching `data/` or `.env`:

| Category | What it removes | Rebuilt by |
| --- | --- | --- |
| `extracted` | `build/disc/`, `build/zoombi32/`, `build/symbols/` | `uv run extract-game`, `uv run runtime-symbols`, `uv run classes` |
| `vm-state` | the VM overlay and install leftovers | automatically on `vm run` |
| `vm-game` | the QuickTime and game install (and the overlay on it) | `uv run vm install-game` (1 min) |
| `vm-base` | the Windows 98 install (and everything layered on it) | `uv run vm install` (30-60 min) |
| `vm` | all of the above | |
| `toolchain` | the extracted Borland toolchains and the Wine prefix | `uv run toolchain setup` |
| `wine` | the downloaded Wine build (macOS) and the Wine prefix | `uv run toolchain setup` (downloads ~180 MB) |
| `ghidra-project` | the Ghidra project, **including any work done in Ghidra's GUI**, and its function list | `uv run ghidra setup` |
| `ghidra` | all of Ghidra: the download, native build and project | `uv run ghidra setup` (downloads ~540 MB) |
| `report` | `build/report/` | `uv run report` |
| `python` | `.venv/`, `__pycache__` | automatically by `uv run` |
| `all` | all of the above plus anything else in `build/` | |

With no arguments it removes `extracted`, `vm-state`, `toolchain`, `report` and `python`: everything that's cheap to rebuild, keeping the VM installs and the Wine download. `uv run clean all` gets back to a fresh clone. Use `--dry-run` to see what would be removed and `--list` to show the categories.

## Development

```sh
uv run lint         # ruff lint, ruff format check, strict mypy, and the tests (pytest)
uv run lint --fix   # apply ruff fixes and formatting, then check
```

The same checks run as a git pre-commit hook. Install it once after cloning:

```sh
uv run pre-commit install
```

All Python code is strictly typed; see `CLAUDE.md` for the conventions.

## What's on the disc

Paths are relative to the disc root (`build/disc/` after extraction).

| File(s) | What it is |
| --- | --- |
| `ZBARCHIV.Z` | InstallShield 3 archive containing the Windows 95 build: `zoombi32.exe` (PE32, ~634 KB, Borland C++ 4.x), the `qb32.qtc` QuickTime codec, and config files |
| `ZOOMBINI._EX` | The Windows 3.1 build. Uncompressed despite the name: a Win16 NE executable, ~940 KB, 191 segments, Borland C++ 4.x runtime |
| `BRODFONT.DLL`, `BRODMIDI.DLL`, `BRODPGI.DLL`, `BRODREG.DLL`, `BRODUTIL.DLL` | Broderbund shared support libraries (Win16) |
| `DATA/*.MHK` | Mohawk resource archives, roughly one per puzzle/area; shared by both builds and read from the CD at runtime |
| `DATA/*.MOV`, `*.QTC` | QuickTime movies and codecs |
| `MOHAWK.WIN`, `DATA/MOHAWK.MAC` | Mohawk engine configuration |
| `QTWSET32/`, `QTWSETUP/` | QuickTime for Windows 2.x installers, 32-bit and 16-bit (third-party) |
| `SYSTEM/WING*` | Microsoft WinG graphics library, used by the Windows 3.1 build (third-party) |
| `MS*`, `_INST32I.EX_`, `SETUP.*`, `INSTALL.EXE`, `AUTORUN.EXE` | Installer and autorun components (ignored) |

## Approach

1. **Run the original** in an emulated Windows 98 PC (QEMU) for dynamic analysis. Wine is not an option on Apple Silicon: the QuickTime installer needs 16-bit code, which Rosetta 2 cannot run.
2. **Document and extract the assets.** Write a Mohawk extractor, using ScummVM's `mohawk` engine as a reference for the container format.
3. **Static analysis in Ghidra.** Name Windows API imports and identify Borland runtime library functions by signature.
4. **Toolchain spike.** Get Borland C++ 4.x's 32-bit compiler running headless, pin down the exact version and compiler flags, and build a diff tool that compares compiled objects against the original functions with relocations masked.
5. **Decompile function by function.** Functional (non-matching) decompilation comes first; byte-matching is a quality bar applied where practical, not a hard requirement.
6. **Port** the decompiled code off Win32-era APIs (QuickTime, DirectSound, 256-colour GDI) onto a modern platform layer.

## Roadmap

- [x] Extract the disc and the Windows 95 build (`uv run extract-game`)
- [x] Scripted Windows 98 VM (`uv run vm install` / `run` / `reset`)
- [x] Scripted QuickTime and game install in the VM (`uv run vm install-game`); game reaches its title screen
- [ ] Game verified playable in the VM (sound, music, movies)
- [ ] Mohawk archive lister / extractor
- [x] Borland C++ 4.5 and 4.52 toolchains running under Wine (`uv run toolchain`)
- [x] Function matcher (`uv run match`) and the first matching functions
- [x] Ghidra project with auto-analysis, function list and decompiler (`uv run ghidra`)
- [x] Label the Borland runtime functions in Ghidra (`uv run runtime-symbols`, `uv run ghidra label`)
- [x] Switch `decomp/` to C++; demangle Borland names
- [x] Harvest class names (RTTI) and vtables; teach Ghidra the calling conventions
- [x] Settle the compiler: Borland C++ 4.5 (4.52 is identical) with default options
- [ ] Decompile the game, function by function

## Legal

This repository does not distribute the original game's binaries or assets. It exists for preservation and interoperability, and you must supply your own copy of the game.
