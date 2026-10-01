# Tool reference

Every tool is a module in `src/zbtools/` exposing a Typer `app`, registered in `pyproject.toml` under `[project.scripts]` and run as `uv run <name>`. Defaults for inputs and outputs come from `src/zbtools/paths.py`; any can be overridden with arguments. The only code that knows about the host OS is `host.py` (binary locations, install hints, QEMU display and audio backends).

| Command | Module | Purpose |
| --- | --- | --- |
| `extract-game` | `extract_game.py` | disc contents to `build/disc/`, the Windows 95 build to `build/zoombi32/` |
| `unpack-isz` | `unpack_isz.py` | list/extract InstallShield 3 archives (hand-written: includes a DCL decompressor no Python library offers) |
| `vm` | `vm.py` | the Windows 98 VM: `install`, `install-game`, `run`, `reset`, `screenshot` ([the VM](vm.md)) |
| `toolchain` | `toolchain.py` | Borland C++ under Wine: `setup`, `check`, `run` |
| `ghidra` | `ghidra.py` | `setup`, `open`, `decompile`, `label`, `codec` ([Ghidra notes](ghidra-notes.md)) |
| `runtime-symbols` | `runtime_symbols.py` | name Borland runtime code by matching the toolchain's libraries ([runtime and RTTI](runtime-and-rtti.md)) |
| `classes` | `rtti.py` | recover C++ classes from RTTI into `build/symbols/classes.json` |
| `match` | `match.py` | compile and compare marked functions ([Matching](../concepts/matching.md)) |
| `match-data` | `match_data.py` | data placement and data references ([Data layout](../concepts/data-layout.md)) |
| `define-data` | `define_data.py` | define declared-but-undefined globals with the original's initial values |
| `near-misses` | `near_misses.py` | classify functions that don't match ([Near-misses](../concepts/near-misses.md)) |
| `worklist` | `worklist.py` | what's ready to decompile next |
| `report` | `report.py` | HTML progress report in `build/report/` (Jinja2 templates in `src/zbtools/templates/`). **Contains disassembly: never publish**, unless built with `--no-embed-binary`, which leaves the original's instructions out (only offsets remain) so the report is safe to distribute. |
| `modules` | `modules.py` | the module map and its evidence ([Modules](../concepts/modules.md)) |
| `includes` | `includes.py` | set each source's module-header includes |
| `assets` | `assets.py` | `extract`, `pack`, `verify`, `frames` ([Formats](../formats/mohawk.md)) |
| `movie-check` | `qb32.py` | run the original movie codec under emulation and compare every frame with ours |
| `build` | `build.py` | compile `decomp/` and `glue/` and link `build/rebuild/zoombi32.exe` ([Rebuilt executable](../concepts/rebuilt-exe.md)) |
| `trace` | `trace.py` | name the functions in a QEMU execution trace from the map |
| `port` | `port.py` | `setup`, `build`, `package`, `serve`, `run` ([The port](../port/overview.md)) |
| `book` | `book.py` | build, serve and audit this book |
| `clean` | `clean.py` | delete generated files by category |
| `lint` | `lint.py` | ruff, ruff format, mypy, pytest |

## Supporting modules

| Module | Role |
| --- | --- |
| `omf.py` | minimal OMF object reader (Borland `.OBJ`), including "virtual segments" for type descriptors and templates |
| `exe.py` | typed pefile/capstone wrappers |
| `demangle.py` | Borland C++ demangler (`demangle`, `qualified_name`), checked against Borland's TDUMP on all 2,227 mangled names in `CW32.LIB` |
| `declarations.py` | reads the headers' structs, globals and prototypes (for Ghidra and `define-data`) |
| `inventory.py` | every function with region, calls, module and status; backs `worklist` and `report` |
| `module_map.py` | loads `decomp/modules.toml` |
| `quicktime.py` | the QuickTime glue's stubs and selectors |
| `mohawk.py`, `formats/`, `movies.py` | the archive container, per-type formats and movie conversion |
| `x86.py` | typed wrapper for Unicorn (x86 emulation) |
| `screen.py` | recognisers for Windows screens in the VM (logon prompt, idle desktop) |
| `game_install.py` | builds the "tools CD" that `vm install-game` runs inside the VM |
| `download.py` | the shared pinned, checksum-verified download helper (Wine, Ghidra, Emscripten, SoundFont) |
| `env.py`, `paths.py` | `.env` and every path the tools use, including `CLEAN_CATEGORIES` |

## Cleaning

Every path a tool generates must belong to a `paths.CLEAN_CATEGORIES` entry (categories may include others by name); add new ones there, and to `CLEAN_DEFAULT` if cheap to rebuild, and keep the cleaning table (docs/src/reference/cleaning.md) in sync.

## Where the work is cached

| Cache | Keyed on |
| --- | --- |
| `build/match-cache/` | source, local headers, options, release |
| `build/assets-cache/` | the encoder's own output for compressed images (never the disc's bytes, or `verify` would test nothing) |
| `build/ghidra/functions.json` | the Ghidra project's analysis |
