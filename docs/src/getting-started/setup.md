# Setup

Each step below is scripted; run only the ones you need (the [previous page](prerequisites.md) says which need what).

## 1. Extract the game

```sh
uv run extract-game
```

Copies the disc into `build/disc/` and unpacks the Windows 95 build from the InstallShield 3 archive `ZBARCHIV.Z` into `build/zoombi32/`. Pass `--iso PATH` for a disc image elsewhere. `uv run unpack-isz build/disc/ZBARCHIV.Z [outdir]` lists or extracts an InstallShield archive directly.

## 2. The Borland C++ toolchain

```sh
uv run toolchain setup
```

Extracts `BIN`, `LIB` and `INCLUDE` from each Borland CD in `data/` into `build/toolchain/<release>/`, then compiles, links and runs a test program with each release under Wine (the first run downloads Wine on macOS, about 180 MB). Afterwards:

```sh
uv run toolchain check                    # rerun the test program with each release
uv run toolchain run 4.5 BCC32 -c foo.c   # any Borland tool, in the current directory
```

Under Wine, release 4.5 is drive `T:`, 4.52 is `U:`, 5.02 is `V:`, the repository is `R:` and the host root is `Z:`. The Borland tools truncate paths over 80 characters, which is why the repository gets its own short drive.

## 3. Ghidra

```sh
uv run ghidra setup                 # download, import and analyse zoombi32.exe (a few minutes)
uv run runtime-symbols              # name the Borland runtime functions
uv run classes                      # recover C++ classes from RTTI
uv run ghidra label                 # apply names, types, classes, calling conventions
uv run ghidra open                  # browse the project in Ghidra's GUI
uv run ghidra decompile 0x46be2e    # Ghidra's C for one function
```

`setup` downloads a pinned Ghidra into `build/ghidra/` and writes every function it finds to `build/ghidra/functions.json`. Close the project in the GUI before running `decompile`, which opens it headlessly. Your manual work in the GUI lives in the project and survives `label`; only `ghidra setup --force` or `clean ghidra-project` removes it. See [Ghidra notes](../reference/ghidra-notes.md).

## 4. The Windows 98 VM

```sh
uv run vm install        # unattended Windows 98 SE setup, 30-60 minutes
uv run vm install-game   # QuickTime and the game; about a minute, no input
uv run vm run            # boot with the game disc in D:
uv run vm reset          # discard everything since the installs
uv run vm screenshot     # PNG of the VM's screen
```

To play, type `zoombi32` in the Start menu's Run box. Depending on your Windows CD, setup may stop at a few wizard pages (licence, product key, user information) with the answers filled in; click Next on each.

The VM's disk is a stack of read-only layers: the Windows install (`win98-base.qcow2`), then QuickTime and the game (`win98-game.qcow2`), then a throwaway copy-on-write overlay that `vm run` boots and `vm reset` discards. `--force` redoes an install. See [The VM](../reference/vm.md).

## 5. Check the decompilation

```sh
uv run match                 # every marked function against the original, byte for byte
uv run match-data -q         # data and data references
uv run near-misses           # review what doesn't match
uv run report --open         # progress report (contains disassembly: keep it local)
```

## 6. Build and run

```sh
uv run build                                    # build/rebuild/zoombi32.exe
uv run vm run --exe build/rebuild/zoombi32.exe  # run it in the VM
uv run port run                                 # the port, on this machine (see `uv run port --help` for targets)
uv run port package browser_wasm && uv run port serve   # the web page: http://127.0.0.1:8000/
```

## 7. Resources

```sh
uv run assets extract    # the disc's archives and movies into assets/ (committed)
uv run assets pack       # assets/ back into archives, in build/assets/
uv run assets verify     # packing reproduces the disc byte for byte
```

## 8. This book

```sh
uv run book build        # build/book/index.html
uv run book serve        # live-reloading server
uv run book screenshots  # which screenshot placeholders still lack an image
```

## Cleaning up

`uv run clean [CATEGORY…]` deletes generated files by category and never touches `data/` or `.env`. With no arguments it removes everything cheap to rebuild and keeps the VM installs, the Wine, Emscripten and SoundFont downloads and the port's saved games. `--list` shows the categories; `--dry-run` shows what would go. [Cleaning up](../reference/cleaning.md) lists them all.

## Development checks

```sh
uv run lint         # ruff, ruff format, strict mypy, pytest
uv run lint --fix   # apply fixes and formatting first
uv run pre-commit install   # once: runs lint when Python code changes
```
