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
- `build/`: gitignored output (`build/disc/` = disc contents, `build/zoombi32/` = Windows 95 build, `build/vm/` = VM disks).
- `src/zbtools/host.py`: the only place that knows about the host OS (binary locations and install hints, QEMU display/audio backends). Route any new host-specific behaviour through it.
- `src/zbtools/vm.py`: `uv run vm`. `install` builds a read-only Windows base disk (unattended Windows 98 SE setup driven by a customized boot floppy + MSBATCH.INF); `install-game` layers QuickTime and the game on it; `run` boots a throwaway qcow2 overlay on top; `reset` discards the overlay. VM automation (watching the screen, typing) goes through QMP with `qemu.qmp`.
- `src/zbtools/game_install.py`: builds the "tools CD" that `install-game` runs inside the VM. `src/zbtools/screen.py`: recognisers for Windows screens (logon prompt, idle desktop).
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

Run `uv run lint` (ruff lint, ruff format check, mypy) before finishing any change to Python code; `uv run lint --fix` applies ruff's fixes and formatting first. It must pass cleanly. It also runs as a pre-commit hook (`.pre-commit-config.yaml`, installed with `uv run pre-commit install`); never bypass it with `--no-verify`.

- **Strict typing:** mypy runs in strict mode with `disallow_any_explicit`. Annotate every function, and every variable whose type isn't inferred. Never write `Any`, `cast()` or `# type: ignore` to silence mypy; fix the types instead.
- **Model structured data with types, not dicts:** `@dataclass(frozen=True)` for records built in code, `NamedTuple` for small immutable tuples, `TypedDict` for dict-shaped data that must stay a dict, and **pydantic models** wherever data from outside the program (JSON, protocol messages, files) needs validating at runtime (e.g. the QMP models in `vm.py`).
- **CLIs use Typer:** each tool module defines `app = typer.Typer(...)` with typed command functions (`Annotated[..., typer.Option(...)]`), registered in `[project.scripts]` as `zbtools.<module>:app`.
- **Binary parsing:** prefer typed helpers such as `int.from_bytes` over `struct.unpack_from`, which returns untyped tuples. `struct.pack_into` is fine for writing.
- Use `pathlib` rather than `os.path`. Use `# fmt: off`/`# fmt: on` only around data tables that formatting would make unreadable.
- **Don't reinvent wheels.** Before hand-writing a parser, file-format reader, protocol client or similar, check PyPI for a maintained library that ships wheels for our Python version (and type information, or add a narrow mypy override as for `fontTools`). Current choices: pycdlib (ISO 9660), fontTools (fonts), Pillow (images), python-dotenv (`.env`), `qemu.qmp` (QEMU control), Typer (CLIs), pydantic (validation). Where no good Python library exists, prefer a standard host tool routed through `host.py` (e.g. mtools for FAT floppy images; `pyfatfs` and `fatfs` are unmaintained or lack wheels). Only hand-write code when neither exists, and say why in its docstring (e.g. the DCL decompressor in `unpack_isz.py`).
- Dependencies: add runtime ones with `uv add`, dev tools with `uv add --dev`.

## Working notes

- Record confirmed findings (formats, compiler flags, file meanings) in `docs/findings.md` as they are established, with the evidence.
