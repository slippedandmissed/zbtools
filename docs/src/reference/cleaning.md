# Cleaning up

```sh
uv run clean [CATEGORY ...]     # delete generated files by category
uv run clean --list             # show the categories
uv run clean --dry-run          # show what would be removed
```

`clean` never touches `data/` or `.env`. Each tool's output belongs to a category in `paths.CLEAN_CATEGORIES`.

| Category | What it removes | Rebuilt by |
| --- | --- | --- |
| `extracted` | `build/disc/`, `build/zoombi32/`, `build/symbols/` | `uv run extract-game`, `uv run runtime-symbols`, `uv run classes` |
| `vm-state` | the VM overlay and install leftovers | automatically on `vm run` |
| `vm-game` | the QuickTime and game install (and the overlay on it) | `uv run vm install-game` (1 min) |
| `vm-base` | the Windows 98 install (and everything layered on it) | `uv run vm install` (30-60 min) |
| `vm` | all of the above | |
| `match-cache` | objects `match` compiled (`build/match-cache/`) | automatically by `uv run match` |
| `toolchain` | the extracted Borland toolchains, the Wine prefix and `match-cache` | `uv run toolchain setup` |
| `wine` | the downloaded Wine build (macOS) and the Wine prefix | `uv run toolchain setup` (downloads ~180 MB) |
| `ghidra-project` | the Ghidra projects (the game's and the codec's), **including any work done in Ghidra's GUI**, and the function list and codec C | `uv run ghidra setup`, `uv run ghidra codec` |
| `ghidra` | all of Ghidra: the download, native build and project | `uv run ghidra setup` (downloads ~540 MB) |
| `report` | `build/report/` | `uv run report` |
| `book` | the rendered handbook (`build/book/`) | `uv run book build` |
| `rebuild` | the rebuilt executable and what went into it (`build/rebuild/`) | `uv run build` |
| `packed-assets` | the archives `assets pack` built (`build/assets/`) | `uv run assets pack` |
| `assets-cache` | compressed images (`build/assets-cache/`) | automatically by `uv run assets pack` or `verify` |
| `movie-frames` | the frames `assets frames` drew (`build/movie-frames/`) | `uv run assets frames` |
| `visual` | the visual tests' report and failing pictures (`build/visual/`) | `uv run visual` |
| `port` | the port's builds (`build/port/<target>/`), what it generates for them (`build/port/generated/`) and its packages and site (`build/port/dist/`) | `uv run port build`, `uv run port package` |
| `port-data` | the port's drives, **including what the native and headless builds saved** (`build/port/data/`) | `uv run port package` |
| `mingw` | llvm-mingw (`build/llvm-mingw/`) | `uv run port setup windows_x64` (downloads ~200 MB) |
| `emsdk` | the Emscripten SDK (`build/emsdk/`) | `uv run port setup browser_wasm` (downloads ~1 GB) |
| `soundfont` | the port's SoundFont (`build/soundfont/`) | `uv run port setup` (downloads 32 MB) |
| `python` | `.venv/`, `__pycache__` | automatically by `uv run` |
| `all` | all of the above plus anything else in `build/` | |

With no arguments it removes `extracted`, `vm-state`, `toolchain`, `report`, `rebuild`, `packed-assets`, `assets-cache`, `movie-frames`, `visual`, `book`, `port` and `python`: everything cheap to rebuild, keeping the VM installs, the Wine, Emscripten, llvm-mingw and SoundFont downloads, and the port's saved games. `uv run clean all` gets back to a fresh clone.

**Keep this table in sync with `paths.CLEAN_CATEGORIES`.** Every path a tool generates must belong to a category (categories may include others by name); add new ones to `CLEAN_DEFAULT` if they're cheap to rebuild.
