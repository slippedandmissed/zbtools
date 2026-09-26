# Logical Journey Of The Zoombinis &mdash; Decompilation

This is a nostalgic game from my childhood and this repo is my latest attempt to getting it decompiled and runnable from source on modern systems.

The target is the 1996 Windows release by Broderbund. The disc ships two builds of the game, and this project targets the **32-bit Windows 95 build** (`zoombi32.exe`, a PE32 executable built with Borland C++ 4.x). Game assets are stored in Mohawk archives (the same container format as Myst and Living Books).

## Status

Early days. Nothing is decompiled yet; see the [roadmap](#roadmap) for what's next.

## Setup

Supported host: macOS on Apple Silicon (for now).

### Prerequisites

- [uv](https://docs.astral.sh/uv/) for the Python tooling
- Your own copies of the game and Windows 98 SE (see below)

### Bring-your-own files

No game files or Windows media are committed to this repository; you need your own copies. Put them in the gitignored `data/` directory under these names (other paths can be passed to the tools as arguments):

| File | What it is |
| --- | --- |
| `data/Logical Journey of the Zoombinis.iso` | The game CD, e.g. from [the Internet Archive](https://archive.org/details/logical-journey-of-the-zoombinis) |
| `data/Windows 98 Second Edition.iso` | Windows 98 SE install CD, for the emulated PC |

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

### Start over

```sh
uv run clean
```

Deletes everything the tooling has generated (`build/`, `.venv/`, caches), leaving only `data/` and `.env`. Use `--dry-run` to see what would be removed.

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
- [ ] Scripted Windows 98 VM running the game
- [ ] Mohawk archive lister / extractor
- [ ] Ghidra project with imports and runtime functions labeled
- [ ] Borland C++ toolchain + object diff tool
- [ ] First decompiled function

## Legal

This repository does not distribute the original game's binaries or assets. It exists for preservation and interoperability, and you must supply your own copy of the game.
