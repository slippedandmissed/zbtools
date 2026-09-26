"""Check whether decompiled functions compile to the original bytes.

The decompiled code is C++ (the game is: it uses C++ objects and exceptions).
Each function in decomp/*.cpp that should match the game is preceded by a
marker comment giving its address in zoombi32.exe:

    /* @zoombi32 0x0046be2e */
    void fn_46be2e(long value)

Methods are marked the same way (`void Widget::set(long v)`). The function is
found in the compiled object by its demangled, qualified name.

A function that is written but doesn't match exactly yet is marked
`/* @zoombi32-nonmatching 0x... */` instead: it's still compiled and compared,
and reported with how close it is, but doesn't count as a failure.

Each file is compiled with Borland C++ 4.5 using the game's options (-p: the
Pascal calling convention by default), or those given by a `/* @flags ... */`
comment in the file, and every marked function is compared with the original,
ignoring the bytes the linker fills in (relocated addresses and call targets).
Mismatches are shown side by side.
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
from zbtools.demangle import qualified_name
from zbtools.exe import Executable, Instruction, disassemble

# Compiler options used unless a file says otherwise (/* @flags ... */) or --flags
# is given: the game's code was compiled with -p (Pascal calling convention by
# default) and BCC32's other defaults (no optimisation, register variables, byte
# alignment); see docs/findings.md.
DEFAULT_FLAGS = "-p"
_FLAGS = re.compile(r"/\*\s*@flags\s+(.*?)\s*\*/")
# Release used unless --release is given. 4.5 and 4.52 generate identical code
# unless 4.52's -fp (Pentium FDIV workaround) is used, which the game doesn't.
DEFAULT_RELEASE = "4.5"

_MARKER = re.compile(r"/\*\s*@zoombi32(-nonmatching)?\s+(0x[0-9a-fA-F]+)\s*\*/")
# The (possibly qualified) name of the function defined after a marker.
_DEFINITION = re.compile(r"([A-Za-z_~][\w:~]*)\s*\(")


@dataclass(frozen=True)
class Target:
    name: str
    address: int
    nonmatching: bool = False  # marked as a known near-miss


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
        name = _DEFINITION.search(source, marker.end())
        if name is None:
            raise ValueError(f"no function after marker {marker.group(0)}")
        address = int(marker.group(2), 16)
        found.append(Target(name.group(1), address, nonmatching=bool(marker.group(1))))
    return found


def _find_public(obj: omf.ObjectFile, name: str) -> omf.Public:
    """The public symbol for a function name, qualified (`Widget::set`) or not."""

    def matches(public: omf.Public) -> bool:
        qualified = qualified_name(public.name)
        names = (qualified, qualified.split("::")[-1])
        if public.name.isupper():  # __pascal: the whole mangled name is upper-cased
            return name.upper() in names
        return name in names

    candidates = [p for p in obj.publics if matches(p)]
    if len(candidates) != 1:
        found = [qualified_name(p.name) for p in obj.publics]
        problem = "is ambiguous (qualify it or rename an overload)" if candidates else "not found"
        raise ValueError(f"{name} {problem} in the object file; it has {found}")
    return candidates[0]


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


def _flags_for(source: str, override: str | None) -> str:
    """The BCC32 options for a file: --flags, else its /* @flags ... */, else the default."""
    if override is not None:
        return override
    directive = _FLAGS.search(source)
    return directive.group(1) if directive else DEFAULT_FLAGS


def compile_source(release: str, source: Path, flags: str) -> omf.ObjectFile:
    out_dir = paths.MATCH_DIR / release
    out_dir.mkdir(parents=True, exist_ok=True)
    obj_path = out_dir / f"{source.stem}.obj"
    obj_path.unlink(missing_ok=True)
    # -I: the source's own directory, so it can include headers next to it.
    args = [
        "-c",
        f"-I{toolchain.windows_path(source.parent)}",
        *shlex.split(flags),
        f"-o{toolchain.windows_path(obj_path)}",
    ]
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
        typer.Argument(help="Source files to check (default: all of decomp/)", show_default=False),
    ] = None,
    release: Annotated[
        list[str] | None,
        typer.Option(
            "--release", "-r", help=f"Borland C++ release(s) to use (default: {DEFAULT_RELEASE})"
        ),
    ] = None,
    flags: Annotated[
        str | None,
        typer.Option(
            help=f"BCC32 options for all files (default: each file's @flags, or {DEFAULT_FLAGS})"
        ),
    ] = None,
    quiet: Annotated[
        bool, typer.Option("--quiet", "-q", help="Don't show disassembly of mismatches")
    ] = False,
) -> None:
    sources = files or sorted([*paths.DECOMP_DIR.rglob("*.cpp"), *paths.DECOMP_DIR.rglob("*.c")])
    installed = toolchain.installed_releases()
    if not installed:
        raise typer.BadParameter("no toolchain installed; run `uv run toolchain setup` first")
    releases = release or [DEFAULT_RELEASE if DEFAULT_RELEASE in installed else installed[0]]
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")
    exe = Executable(game)

    failed = 0
    for rel in releases:
        print(f"Borland C++ {rel}:")
        for source in sources:
            text = source.read_text()
            targets = find_targets(text)
            if not targets:
                continue
            obj = compile_source(rel, source.resolve(), _flags_for(text, flags))
            for target in targets:
                result = compare(target, obj, exe)
                where = f"{target.name} @ {target.address:#x}"
                size = len(result.compiled)
                if result.matches:
                    note = " (marked non-matching: remove the mark)" if target.nonmatching else ""
                    relocated = len(result.masked)
                    print(f"  match     {where}: {size} bytes ({relocated} relocated){note}")
                elif target.nonmatching:
                    differ = f"{len(result.mismatches)} of {size} bytes differ"
                    print(f"  nonmatch  {where}: {differ} (marked non-matching)")
                else:
                    failed += 1
                    print(f"  MISMATCH  {where}: {len(result.mismatches)} of {size} bytes differ")
                    if not quiet:
                        print("\n".join(_side_by_side(result)))
    if failed:
        raise typer.Exit(1)
