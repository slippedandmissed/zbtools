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

The tools tell you if QEMU or mtools is missing and how to install it.
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

### Cleaning up

```sh
uv run clean [CATEGORY ...]
```

Deletes generated files by category, never touching `data/` or `.env`:

| Category | What it removes | Rebuilt by |
| --- | --- | --- |
| `extracted` | `build/disc/`, `build/zoombi32/` | `uv run extract-game` |
| `vm-state` | the VM overlay and install leftovers | automatically on `vm run` |
| `vm-game` | the QuickTime and game install (and the overlay on it) | `uv run vm install-game` (1 min) |
| `vm-base` | the Windows 98 install (and everything layered on it) | `uv run vm install` (30-60 min) |
| `vm` | all of the above | |
| `python` | `.venv/`, `__pycache__` | automatically by `uv run` |
| `all` | all of the above plus anything else in `build/` | |

With no arguments it removes `extracted`, `vm-state` and `python`: everything that's cheap to rebuild, keeping the Windows install. `uv run clean all` gets back to a fresh clone. Use `--dry-run` to see what would be removed and `--list` to show the categories.

## Development

```sh
uv run lint         # ruff lint, ruff format check, and strict mypy
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
- [ ] Ghidra project with imports and runtime functions labeled
- [ ] Borland C++ toolchain + object diff tool
- [ ] First decompiled function

## Legal

This repository does not distribute the original game's binaries or assets. It exists for preservation and interoperability, and you must supply your own copy of the game.
