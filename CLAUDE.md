# CLAUDE.md

Decompilation of *Logical Journey of the Zoombinis* (Broderbund, 1996, Windows release). The long-term goal is source code that builds and runs on modern systems. See `README.md` for the public-facing overview and setup steps.

## Key facts about the target

- **Main binary (target):** `zoombi32.exe`, the Windows 95 build, extracted by `uv run extract-game` from the game ISO (via `ZBARCHIV.Z`, an InstallShield 3 archive) into `build/zoombi32/`. PE32, ~634 KB, sections `CODE`/`DATA`/`.idata`/`.edata`/`.reloc`/`.rsrc`.
- **Compiler:** Borland C++ 4.5 or 4.52, which generate identical code for this game; the tools use 4.5. **Options: `-p -k-`** (Pascal calling convention by default; no stack frame unless needed) plus BCC32's defaults (no optimisation, register variables, byte alignment), confirmed by matching functions (see `docs/findings.md`). So decompiled game functions have no calling-convention keyword and list their parameters in Pascal order: **Ghidra shows them reversed** (its `param_1` is the last parameter). Options were set per module: the support library at `0x46ce80`-`0x46f7a4` used `-p` alone (see `decomp/support.cpp`), and the Mohawk engine was built without `-p` and declares conventions per function; such files set their own options with `/* @flags ... */`. Linked by TLINK32 (PE linker version 2.25); the PE timestamp (2013) is junk.
- **Imports:** KERNEL32, USER32, GDI32, ADVAPI32, WINMM, VERSION, DSOUND (DirectSound), QTIM32 + CMGR32 (QuickTime for Windows 2.x, 32-bit).
- **Assets:** `DATA/*.MHK` on the disc (`build/disc/DATA/` after extraction), Mohawk archives (`MHWK` magic, `RSRC` directory), read from the CD at runtime. ScummVM's `engines/mohawk/` is the reference for the container format; per-game resource types for Zoombinis are undocumented.
- **Other build:** `ZOOMBINI._EX` on the disc is the Windows 3.1 build (Win16 NE, same compiler family). Useful for cross-reference, not the primary target.
- **Ignore:** installer and autorun files (`MS*`, `_INST32I.EX_`, `SETUP.*`, `INSTALL.EXE`, `AUTORUN.EXE`).

## Constraints

- **Reproducibility:** every setup step a developer needs after cloning must be committed or performed by a script. The only manual inputs are bring-your-own files (the game in `data/`, Windows install media). Exploratory work is fine, but always finish by folding it into a script and documenting it in the README's Setup section.
- **Legal:** `data/` is gitignored and holds the user's ISOs (game disc and Windows 98 SE, default names in `src/zbtools/paths.py`); `.env` holds their Windows product key (`WINDOWS_PRODUCT_KEY`). Never commit game files, extracted assets, Windows media, or large verbatim disassembly of the original binaries.
- Treat `data/` as read-only. Write extracted or derived output to the gitignored `build/` directory.
- Host platform: macOS on Apple Silicon. Rosetta 2 cannot run 16-bit Windows code, so Wine cannot run the game (the QuickTime installer needs 16-bit code); use the emulated VM instead. Wine *is* used for the 32-bit Borland command-line tools (`uv run toolchain`).

## Layout

- `src/zbtools/`: Python tooling package. Each tool is a module exposing `main()`, registered under `[project.scripts]` in `pyproject.toml` and run as `uv run <name>`.
- `src/zbtools/paths.py`: default locations of inputs and outputs, resolved from the repo root. New tools should take their defaults from here and allow overriding them with arguments.
- `build/`: gitignored output (`build/disc/` = disc contents, `build/zoombi32/` = Windows 95 build, `build/vm/` = VM disks).
- `src/zbtools/host.py`: the only place that knows about the host OS (binary locations and install hints, QEMU display/audio backends). Route any new host-specific behaviour through it.
- `src/zbtools/vm.py`: `uv run vm`. `install` builds a read-only Windows base disk (unattended Windows 98 SE setup driven by a customized boot floppy + MSBATCH.INF); `install-game` layers QuickTime and the game on it; `run` boots a throwaway qcow2 overlay on top; `reset` discards the overlay. VM automation (watching the screen, typing) goes through QMP with `qemu.qmp`.
- `src/zbtools/toolchain.py`: `uv run toolchain`. Extracts `BC45/{BIN,LIB,INCLUDE}` from each Borland CD (with 7-Zip) into `build/toolchain/<release>/`, maps each release to a Wine drive (4.5 = `T:`, 4.52 = `U:`, host root = `Z:`) and rewrites its BCC32.CFG/TLINK32.CFG to match; runs tools under Wine with the release's `BIN` on the Windows PATH (BCC32 finds TLINK32 via PATH). Borland `.OBJ` files embed timestamps and include paths in COMENT records, so object comparisons must ignore those.
- `src/zbtools/game_install.py`: builds the "tools CD" that `install-game` runs inside the VM. `src/zbtools/screen.py`: recognisers for Windows screens (logon prompt, idle desktop).
- `decomp/`: decompiled C++ (`.cpp`; the game is C++). Mark each function that should match the game with `/* @zoombi32 0x<address> */` immediately before it; `uv run match` (`src/zbtools/match.py`) compiles each file with each installed Borland release and compares marked functions byte for byte, masking linker-filled fields (it uses `omf.py`, a minimal OMF object reader, and `exe.py`, typed pefile/capstone wrappers). Name functions whose purpose is unknown after their address (`fn_46be2e`) and globals likewise (`g_4a7f58`); declare globals `extern` unless the function's own file defines them. A decompiled function is only done when `uv run match` reports it as matching; one that's written but not yet exact is marked `/* @zoombi32-nonmatching 0x... */` with a comment saying what still differs. Headers next to a source file can be included (`match` adds the source's directory to the include path).
- `src/zbtools/ghidra.py`: `uv run ghidra`. Pinned Ghidra release (built natives on platforms without prebuilt ones, e.g. Apple Silicon, via Ghidra's Gradle wrapper), driven headlessly with `pyghidra`; `ghidra-stubs` (same version) types the Ghidra API for mypy. Ghidra classes can only be imported after `pyghidra.start()`, so import them inside functions (with `# noqa: PLC0415`) and under `TYPE_CHECKING` for annotations. The stubs don't model Java's nulls: where a Ghidra method can return null, annotate the variable as `X | None` explicitly. The project in `build/ghidra/project/` may hold the user's manual reverse-engineering work: never delete or recreate it without being asked. `download.py` holds the shared pinned-download helper (Wine and Ghidra).
- `src/zbtools/runtime_symbols.py`: `uv run runtime-symbols` names the Borland runtime code in the game by matching library code segments (see `docs/findings.md`); `uv run ghidra label` applies the names, never overwriting names set by hand.
- `src/zbtools/rtti.py`: `uv run classes` recovers C++ classes from Borland RTTI type descriptors (layout in the module docstring and `docs/findings.md`) into `build/symbols/classes.json`; `uv run ghidra label` turns them into Ghidra classes and sets `__stdcall` where functions pop their own arguments. Automated naming only ever renames functions still named `FUN_...` (source DEFAULT).
- `src/zbtools/demangle.py`: Borland C++ demangler (`demangle`, `qualified_name`), checked against Borland's TDUMP; used by `match`, `runtime-symbols` and `ghidra label`.
- `src/zbtools/inventory.py`: every function with its region (startup / game / runtime / engine, derived from the recovered symbols), direct and indirect calls, and status (matched / nonmatching / library / todo). Used by `uv run worklist` (what's ready to decompile, leaf-first) and `uv run report` (HTML progress report in `build/report/`, built with Jinja2 from `src/zbtools/templates/`). The report contains the game's disassembly: never publish it (no Artifacts, uploads or commits).
- `docs/findings.md`: confirmed facts about the game and its installer, with the evidence (addresses, strings, experiments).
- **Keep `uv run clean` up to date:** it deletes by category, from `paths.CLEAN_CATEGORIES`. Every path a tool generates must belong to a category (categories may include other categories by name); add new ones there, and to `CLEAN_DEFAULT` if they're cheap to rebuild. Keep the README's cleaning table in sync.

## Game plan

1. Scripted Windows 98 VM under QEMU that runs the game, for dynamic analysis (QEMU's gdbstub for debugging).
2. Mohawk extractor and catalog of resource types per archive.
3. Ghidra project for `zoombi32.exe`: label imports, identify Borland RTL functions via signatures.
4. Toolchain spike: Borland C++ 4.x 32-bit compiler run headless, plus a diff tool comparing compiled objects against original functions with relocations masked. Time-box this.
5. Decompile function by function, starting with small leaf functions. **Functional (non-matching) first**; byte-matching is a goal where practical, not a gate.
6. Port off QuickTime/DirectSound/256-colour GDI to a modern platform layer.

## Code quality

Run `uv run lint` (ruff lint, ruff format check, mypy, pytest) before finishing any change to Python code; `uv run lint --fix` applies ruff's fixes and formatting first. It must pass cleanly. It also runs as a pre-commit hook (`.pre-commit-config.yaml`, installed with `uv run pre-commit install`); never bypass it with `--no-verify`.

- **Strict typing:** mypy runs in strict mode with `disallow_any_explicit`. Annotate every function, and every variable whose type isn't inferred. Never write `Any`, `cast()` or `# type: ignore` to silence mypy; fix the types instead.
- **Model structured data with types, not dicts:** `@dataclass(frozen=True)` for records built in code, `NamedTuple` for small immutable tuples, `TypedDict` for dict-shaped data that must stay a dict, and **pydantic models** wherever data from outside the program (JSON, protocol messages, files) needs validating at runtime (e.g. the QMP models in `vm.py`).
- **CLIs use Typer:** each tool module defines `app = typer.Typer(...)` with typed command functions (`Annotated[..., typer.Option(...)]`), registered in `[project.scripts]` as `zbtools.<module>:app`.
- **Binary parsing:** prefer typed helpers such as `int.from_bytes` over `struct.unpack_from`, which returns untyped tuples. `struct.pack_into` is fine for writing.
- Use `pathlib` rather than `os.path`. Use `# fmt: off`/`# fmt: on` only around data tables that formatting would make unreadable.
- **Don't reinvent wheels.** Before hand-writing a parser, file-format reader, protocol client or similar, check PyPI for a maintained library that ships wheels for our Python version (and type information, or add a narrow mypy override as for `fontTools`). Current choices: pycdlib (ISO 9660), fontTools (fonts), Pillow (images), python-dotenv (`.env`), `qemu.qmp` (QEMU control), Typer (CLIs), pydantic (validation). Where no good Python library exists, prefer a standard host tool routed through `host.py` (e.g. mtools for FAT floppy images; `pyfatfs` and `fatfs` are unmaintained or lack wheels). Only hand-write code when neither exists, and say why in its docstring (e.g. the DCL decompressor in `unpack_isz.py`).
- Tests live in `tests/` (pytest). Add them for logic that can be checked without bring-your-own files (parsers, demangling); anything needing the ISOs or toolchain is verified by running the tools.
- Dependencies: add runtime ones with `uv add`, dev tools with `uv add --dev`.

## Working notes

- Record confirmed findings (formats, compiler flags, file meanings) in `docs/findings.md` as they are established, with the evidence.
