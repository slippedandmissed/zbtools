"""Check whether decompiled functions compile to the original bytes.

Each function in decomp/*.c that should match the game is preceded by a marker
comment giving its address in zoombi32.exe:

    /* @zoombi32 0x0046be2e */
    void __stdcall fn_46be2e(long value)

Each file is compiled with each installed Borland C++ release, and every marked
function is compared with the original, ignoring the bytes the linker fills in
(relocated addresses and call targets). Mismatches are shown side by side.
"""

import difflib
import itertools
import re
import shlex
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import typer

from zbtools import omf, paths, toolchain
from zbtools.exe import Executable, Instruction, disassemble

# Compiler flags used unless --flags is given. Not yet confirmed against the game.
DEFAULT_FLAGS = ""

_MARKER = re.compile(r"/\*\s*@zoombi32\s+(0x[0-9a-fA-F]+)\s*\*/")
_CALL_OR_DEFINITION = re.compile(r"\b([A-Za-z_]\w*)\s*\(")


@dataclass(frozen=True)
class Target:
    name: str
    address: int


@dataclass(frozen=True)
class Result:
    target: Target
    compiled: bytes
    original: bytes
    masked: frozenset[int]  # offsets in `compiled` the linker fills in
    original_masked: frozenset[int]  # offsets in `original` the linker filled in
    mismatches: tuple[int, ...]  # offsets that differ

    @property
    def matches(self) -> bool:
        return not self.mismatches


def find_targets(source: str) -> list[Target]:
    """Marked functions: the first `name(` after each @zoombi32 marker."""
    found = []
    for marker in _MARKER.finditer(source):
        name = _CALL_OR_DEFINITION.search(source, marker.end())
        if name is None:
            raise ValueError(f"no function after marker {marker.group(0)}")
        found.append(Target(name.group(1), int(marker.group(1), 16)))
    return found


def _find_public(obj: omf.ObjectFile, name: str) -> omf.Public:
    # C names get a leading underscore (cdecl) or are upper-cased (pascal).
    for public in obj.publics:
        if public.name in (name, f"_{name}", name.upper()):
            return public
    raise ValueError(f"{name} not found in the object file ({[p.name for p in obj.publics]})")


def compare(target: Target, obj: omf.ObjectFile, exe: Executable) -> Result:
    public = _find_public(obj, target.name)
    start, end = obj.extent(public)
    segment = obj.segments[public.segment]
    compiled = bytes(segment.data[start:end])
    original = exe.read(target.address, len(compiled))
    fixups = [f for f in segment.fixups if start <= f.offset < end]
    masked = frozenset(f.offset - start + k for f in fixups for k in range(f.size))
    mismatches = {i for i in range(len(compiled)) if i not in masked and compiled[i] != original[i]}
    # An absolute address in the compiled code must be relocated in the original too.
    for f in fixups:
        if not f.self_relative and f.size == 4:
            offset = f.offset - start
            if target.address + offset not in exe.relocations:
                mismatches.update(range(offset, offset + 4))
    relocated = frozenset(
        i
        for i in range(len(original))
        if any(target.address + i - k in exe.relocations for k in range(4))
    )
    return Result(target, compiled, original, masked, relocated, tuple(sorted(mismatches)))


def compile_source(release: str, source: Path, flags: str) -> omf.ObjectFile:
    out_dir = paths.MATCH_DIR / release
    out_dir.mkdir(parents=True, exist_ok=True)
    obj_path = out_dir / f"{source.stem}.obj"
    obj_path.unlink(missing_ok=True)
    args = ["-c", *shlex.split(flags), f"-o{toolchain.windows_path(obj_path)}"]
    result = toolchain.run_tool(release, "BCC32", [*args, toolchain.windows_path(source)], out_dir)
    if result.returncode != 0 or not obj_path.exists():
        raise RuntimeError(f"compiling {source.name} failed:\n{result.stdout}{result.stderr}")
    return omf.read(obj_path.read_bytes())


def _alignment_keys(
    instructions: list[Instruction], base: int, masked: frozenset[int]
) -> list[bytes]:
    """Instruction bytes with linker-filled fields (and call/jump targets) blanked,
    so instructions that differ only in addresses compare equal."""
    keys = []
    for ins in instructions:
        start = ins.address - base
        blank = set(masked)
        if ins.raw[0] in (0xE8, 0xE9) and len(ins.raw) == 5:  # call/jmp rel32
            blank.update(range(start + 1, start + 5))
        keys.append(bytes(0 if start + k in blank else b for k, b in enumerate(ins.raw)))
    return keys


def _side_by_side(result: Result) -> list[str]:
    """Disassembly of both versions, aligned so an inserted or missing
    instruction shows as one line rather than shifting everything after it."""
    base = result.target.address
    left = disassemble(result.compiled, base)
    right = disassemble(result.original, base)
    matcher = difflib.SequenceMatcher(
        None,
        _alignment_keys(left, base, result.masked),
        _alignment_keys(right, base, result.original_masked),
        autojunk=False,
    )

    def column(ins: Instruction | None) -> str:
        return f"{ins.address - base:04x} {ins.raw.hex():<14} {ins.text}" if ins else ""

    lines = [f"    {'compiled':<48} original"]
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        mark = " " if tag == "equal" else "*"
        for a, b in itertools.zip_longest(left[i1:i2], right[j1:j2]):
            lines.append(f"  {mark} {column(a):<48.48} {column(b)}")
    return lines


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    files: Annotated[
        list[Path] | None,
        typer.Argument(help="C files to check (default: all of decomp/)", show_default=False),
    ] = None,
    release: Annotated[
        list[str] | None,
        typer.Option(
            "--release", "-r", help="Borland C++ release(s) to use (default: all installed)"
        ),
    ] = None,
    flags: Annotated[str, typer.Option(help="BCC32 options, e.g. '-O2 -5'")] = DEFAULT_FLAGS,
    quiet: Annotated[
        bool, typer.Option("--quiet", "-q", help="Don't show disassembly of mismatches")
    ] = False,
) -> None:
    sources = files or sorted(paths.DECOMP_DIR.rglob("*.c"))
    releases = release or toolchain.installed_releases()
    if not releases:
        raise typer.BadParameter("no toolchain installed; run `uv run toolchain setup` first")
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")
    exe = Executable(game)

    failed = 0
    for rel in releases:
        print(f"Borland C++ {rel}{f' ({flags})' if flags else ''}:")
        for source in sources:
            targets = find_targets(source.read_text())
            if not targets:
                continue
            obj = compile_source(rel, source.resolve(), flags)
            for target in targets:
                result = compare(target, obj, exe)
                where = f"{target.name} @ {target.address:#x}"
                size = len(result.compiled)
                if result.matches:
                    print(f"  match     {where}: {size} bytes ({len(result.masked)} relocated)")
                else:
                    failed += 1
                    print(f"  MISMATCH  {where}: {len(result.mismatches)} of {size} bytes differ")
                    if not quiet:
                        print("\n".join(_side_by_side(result)))
    if failed:
        raise typer.Exit(1)
