# Python conventions

Run `uv run lint` before finishing any change to Python code (`--fix` applies ruff's fixes and formatting first). It runs ruff lint, ruff format check, mypy and pytest, and also as a pre-commit hook (`uv run pre-commit install`); never bypass the hook with `--no-verify`.

- **Strict typing.** mypy runs strict with `disallow_any_explicit`. Annotate every function and every variable whose type isn't inferred. Never write `Any`, `cast()` or `# type: ignore`; fix the types.
- **Model structured data with types, not dicts:** `@dataclass(frozen=True)` for records built in code, `NamedTuple` for small immutable tuples, `TypedDict` for dict-shaped data that must stay a dict, and **pydantic** models for data from outside the program (JSON, protocol messages, files) that needs validating at run time (the QMP models in `vm.py`).
- **CLIs use Typer**: each tool defines `app = typer.Typer(...)` with typed commands (`Annotated[..., typer.Option(...)]`).
- **Binary parsing:** prefer `int.from_bytes` over `struct.unpack_from` (untyped tuples). `struct.pack_into` is fine for writing.
- Use `pathlib`, not `os.path`. `# fmt: off`/`on` only around data tables formatting would ruin.
- **Don't reinvent wheels.** Check PyPI first for a maintained library with wheels for our Python (and types, or add a narrow mypy override as for `fontTools`). Current choices: pycdlib (ISO 9660), fontTools, Pillow, python-dotenv, `qemu.qmp`, Typer, pydantic, Unicorn. Where none exists, prefer a standard host tool routed through `host.py` (mtools for FAT floppies). Hand-write only when neither exists, and say why in the docstring (e.g. the DCL decompressor in `unpack_isz.py`).
- **Ghidra classes** can only be imported after `pyghidra.start()`: import them inside functions (`# noqa: PLC0415`) and under `TYPE_CHECKING` for annotations. The stubs don't model Java's nulls: annotate variables that can be null as `X | None`.
- **Tests** live in `tests/` (pytest) for anything checkable without the bring-your-own files: parsers, demangling, formats. Anything needing the ISOs or toolchain is verified by running the tools.
- **Dependencies:** `uv add` for runtime, `uv add --dev` for dev tools.
