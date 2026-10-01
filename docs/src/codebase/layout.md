# Repository layout

```text
decomp/        the decompiled game and engine: <module>.cpp / <module>.h, zoombinis.h,
               modules.toml (the module map), matching.txt (what matches)
glue/          C++ standing in for what the game links but we don't have (QuickTime's SDK glue)
assets/        the game's resources in modern formats, one directory per Mohawk archive,
               plus movies/, and zoombi32/ (the executable's icon and installed files)
port/          the port: CMake, miniwin, the host layer, the web page
src/zbtools/   the Python tooling
tests/         pytest, for everything checkable without bring-your-own files
docs/          this book (book.toml, src/, theme/)
data/          your ISOs and .env's neighbour (gitignored, read-only)
build/         every generated file (gitignored): disc, VM disks, toolchain, Ghidra, report, port, book
```

## `decomp/`

One `.cpp` per original object file (about 150 in all: 42 game modules, 6 OS-layer modules, the engine's many small ones). Each has an optional `.h`. Special files:

| File | Role |
| --- | --- |
| `zoombinis.h` | shared types (`View`, `Snoid`, `Scene`, `Party`…), the engine's classes as the game's calls show them, and every global several modules use |
| `modules.toml` | the curated module map: start address, name, evidence |
| `matching.txt` | functions known to match, written by `uv run match --update` |
| `<module>.h` | that module's prototypes and the globals and types only it (and its callers) use |

See [Source modules](../concepts/modules.md) and [Modules and scenes](modules-and-scenes.md) for what each does.

## `glue/` and `port/glue/`

`glue/quicktime.cpp` re-creates Apple's QuickTime for Windows SDK glue (selector stubs written as raw `__emit__` bytes that load QuickTime and forward calls). It isn't decompiled code, has no markers, `match` ignores it, and it may use inline assembly. The port replaces it with `port/glue/quicktime.cpp`, which plays the intro movie itself. Any file in `port/glue/` or `port/decomp/` replaces the same-named file when the port builds.

## `assets/`

```text
assets/<ARCHIVE>/archive.toml        where the archive goes on the disc; resources in stored order
assets/<ARCHIVE>/<TYPE>/<id>.<ext>   one file per resource, in its modern format
assets/movies/<NAME>/                frames.toml, palettes.toml, bitmaps.toml, casts/, sound.wav, movie.toml
assets/zoombi32/                     ICON/, resources.toml, installed/ (mohawk.w32, CORNER.TTF, Zoombini.who)
```

Archives: `BASECAMP`, `BCTWO`, `BRIDGE`, `CAVES`, `FERRY`, `FLEENS`, `HOTEL`, `LILLY`, `MAP`, `MAZE2`, `MIDIMAP`, `MIDIMPC`, `NET`, `PICKER`, `PIZZA`, `SLIDES`, `SMOKE`, `TOWN`, `TUNNELS`, `XFER`, `ZOOMBINI`.

## `src/zbtools/`

See the [tool reference](../reference/tools.md).
