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
and reported with how close it is, but doesn't count as a failure. One that is
complete but deliberately not byte-exact is marked `/* @zoombi32-functional
0x... */`: decompiled code must be portable C++, so where only machine code
(inline assembly, emitted bytes, pseudo-registers) would reproduce the
original, the function does the same thing portably instead.

Each file is compiled with Borland C++ 4.5 using the game's usual options
(`-p -k-`), or those given by a `/* @flags ... */` comment in the file, and
every marked function is compared with the original, ignoring the bytes the
linker fills in (relocated addresses and call targets). Calls the compiler
resolves itself, to other marked functions in the same file, must reach the
function at the callee's marker address. Mismatches are shown side by side.
"""

import difflib
import hashlib
import itertools
import re
import shlex
from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from zbtools import omf, paths, toolchain
from zbtools.demangle import qualified_name
from zbtools.exe import Executable, Instruction, disassemble

# Compiler options used unless a file says otherwise (/* @flags ... */) or --flags
# is given: most of the game's code was compiled with -p (Pascal calling
# convention by default) and -k- (no stack frame unless needed), otherwise BCC32's
# defaults (no optimisation, register variables, byte alignment). The support
# library just below the runtime used -p alone. See docs/findings.md.
DEFAULT_FLAGS = "-p -k-"
_FLAGS = re.compile(r"/\*\s*@flags\s+(.*?)\s*\*/")
# Release used unless --release is given. 4.5 and 4.52 generate identical code
# unless 4.52's -fp (Pentium FDIV workaround) is used, which the game doesn't.
DEFAULT_RELEASE = "4.5"

_MARKER = re.compile(r"/\*\s*@zoombi32(?:-(nonmatching|functional))?\s+(0x[0-9a-fA-F]+)\s*\*/")
# The (possibly qualified) name of the function defined after a marker.
_DEFINITION = re.compile(r"([A-Za-z_~][\w:~]*)\s*\(")


class Marker(StrEnum):
    """How a decompiled function is meant to compare with the original."""

    EXACT = "exact"  # @zoombi32: must match byte for byte
    NONMATCHING = "nonmatching"  # @zoombi32-nonmatching: not exact yet
    FUNCTIONAL = "functional"  # @zoombi32-functional: equivalent, not exact by design


@dataclass(frozen=True)
class Target:
    name: str
    address: int
    marker: Marker = Marker.EXACT


@dataclass(frozen=True)
class Result:
    target: Target
    compiled: bytes
    original: bytes
    masked: frozenset[int]  # offsets in `compiled` the linker fills in, or that call siblings
    original_masked: frozenset[int]  # offsets in `original` the linker filled in
    mismatches: tuple[int, ...]  # offsets that differ
    references: dict[int, str]  # offset in `compiled` of each fixup -> its target symbol

    @property
    def matches(self) -> bool:
        return not self.mismatches


@dataclass(frozen=True)
class MarkerPosition:
    address: int
    start: int  # of the marker comment in the source
    end: int


def marker_positions(source: str) -> list[MarkerPosition]:
    return [
        MarkerPosition(int(m.group(2), 16), m.start(), m.end()) for m in _MARKER.finditer(source)
    ]


def find_targets(source: str) -> list[Target]:
    """Marked functions: the first `name(` after each @zoombi32 marker."""
    found = []
    for marker in _MARKER.finditer(source):
        name = _DEFINITION.search(source, marker.end())
        if name is None:
            raise ValueError(f"no function after marker {marker.group(0)}")
        address = int(marker.group(2), 16)
        kind = Marker(marker.group(1)) if marker.group(1) else Marker.EXACT
        found.append(Target(name.group(1), address, kind))
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


@dataclass(frozen=True)
class Sibling:
    """Another marked function in the same object file."""

    address: int  # in the original
    symbol: str  # in the object file


def _local_calls(
    compiled: bytes,
    original: bytes,
    *,
    address: int,
    at: tuple[str, int],
    masked: frozenset[int],
    siblings: dict[tuple[str, int], Sibling],
) -> dict[int, str]:
    """Calls the compiler resolved itself, to functions in the same object: they
    have no fixup, and their displacement depends on our file's layout. Returns
    the ones that reach the right function (a marked sibling the original calls
    too), as the offset of their operand -> the callee's symbol."""
    segment, start = at
    found = {}
    for ins in disassemble(compiled, 0):
        after = ins.address + 5
        if ins.raw[0] != 0xE8 or len(ins.raw) != 5 or ins.address + 1 in masked:
            continue
        ours = start + after + int.from_bytes(ins.raw[1:], "little", signed=True)
        theirs = (
            address
            + after
            + int.from_bytes(original[ins.address + 1 : after], "little", signed=True)
        )
        callee = siblings.get((segment, ours))
        if callee is not None and callee.address == theirs:
            found[ins.address + 1] = callee.symbol
    return found


def compare(
    target: Target,
    obj: omf.ObjectFile,
    exe: Executable,
    siblings: dict[tuple[str, int], Sibling] | None = None,
) -> Result:
    """Compare a compiled function with the original. `siblings` gives the
    original addresses of the object's other marked functions, by (segment,
    offset), to check calls the compiler resolved itself."""
    public = _find_public(obj, target.name)
    start, end = obj.extent(public)
    segment = obj.segments[public.segment]
    compiled = bytes(segment.data[start:end])
    original = exe.read(target.address, len(compiled))
    fixups = [f for f in segment.fixups if start <= f.offset < end]
    masked = frozenset(f.offset - start + k for f in fixups for k in range(f.size))
    local = _local_calls(
        compiled,
        original,
        address=target.address,
        at=(public.segment, start),
        masked=masked,
        siblings=siblings or {},
    )
    masked |= {offset + k for offset in local for k in range(4)}
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
    references = {f.offset - start: f.target for f in fixups} | local
    return Result(
        target, compiled, original, masked, relocated, tuple(sorted(mismatches)), references
    )


def _flags_for(source: str, override: str | None) -> str:
    """The BCC32 options for a file: --flags, else its /* @flags ... */, else the default."""
    if override is not None:
        return override
    directive = _FLAGS.search(source)
    return directive.group(1) if directive else DEFAULT_FLAGS


# Bump when what's cached, or how it's keyed, changes.
_CACHE_VERSION = 1
_INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)


def local_headers(source: Path) -> list[Path]:
    """The headers a source includes with `#include "..."`, recursively,
    looked up next to the including file, then next to the source (as BCC32
    does with match's -I). System headers (`<...>`) come with the toolchain,
    which the cache key covers by its release."""
    found: set[Path] = set()
    pending = [source]
    while pending:
        current = pending.pop()
        for name in _INCLUDE.findall(current.read_text(errors="replace")):
            for base in (current.parent, source.parent):
                header = (base / name).resolve()
                if header.is_file():
                    if header not in found:
                        found.add(header)
                        pending.append(header)
                    break
    return sorted(found)


def cache_key(release: str, source: Path, flags: str) -> str:
    """What a compiled object depends on: the source, the headers it includes,
    the options and the release."""
    digest = hashlib.sha256()
    for part in (str(_CACHE_VERSION), release, flags):
        digest.update(part.encode() + b"\0")
    for path in [source, *local_headers(source)]:
        digest.update(path.name.encode() + b"\0" + path.read_bytes() + b"\0")
    return digest.hexdigest()


@dataclass(frozen=True)
class Compiled:
    obj: omf.ObjectFile
    cached: bool  # reused from build/match-cache rather than compiled


def compile_source(release: str, source: Path, flags: str, *, use_cache: bool = True) -> Compiled:
    """Compile a source, or reuse the object compiled from it before if nothing
    it depends on has changed (see `cache_key`)."""
    cache_dir = paths.MATCH_CACHE / release
    cache_dir.mkdir(parents=True, exist_ok=True)
    # One entry per source file, so the cache doesn't grow as sources change.
    name = f"{source.stem}-{hashlib.sha256(str(source).encode()).hexdigest()[:8]}"
    obj_path, key_path = cache_dir / f"{name}.obj", cache_dir / f"{name}.key"
    key = cache_key(release, source, flags)
    if use_cache and obj_path.exists() and key_path.exists() and key_path.read_text() == key:
        return Compiled(omf.read(obj_path.read_bytes()), cached=True)
    key_path.unlink(missing_ok=True)
    obj_path.unlink(missing_ok=True)
    # -I: the source's own directory, so it can include headers next to it.
    args = [
        "-c",
        f"-I{toolchain.windows_path(source.parent)}",
        *shlex.split(flags),
        f"-o{toolchain.windows_path(obj_path)}",
    ]
    result = toolchain.run_tool(
        release, "BCC32", [*args, toolchain.windows_path(source)], cache_dir
    )
    if result.returncode != 0 or not obj_path.exists():
        raise RuntimeError(f"compiling {source.name} failed:\n{result.stdout}{result.stderr}")
    key_path.write_text(key)
    return Compiled(omf.read(obj_path.read_bytes()), cached=False)


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


@dataclass(frozen=True)
class Row:
    """One line of the side-by-side comparison."""

    compiled: Instruction | None
    original: Instruction | None
    same: bool


def aligned_rows(result: Result) -> list[Row]:
    """Disassembly of both versions, aligned so an inserted or missing
    instruction shows as one row rather than shifting everything after it."""
    base = result.target.address
    left = disassemble(result.compiled, base)
    right = disassemble(result.original, base)
    matcher = difflib.SequenceMatcher(
        None,
        _alignment_keys(left, base, result.masked),
        _alignment_keys(right, base, result.original_masked),
        autojunk=False,
    )
    return [
        Row(a, b, tag == "equal")
        for tag, i1, i2, j1, j2 in matcher.get_opcodes()
        for a, b in itertools.zip_longest(left[i1:i2], right[j1:j2])
    ]


def _side_by_side(result: Result) -> list[str]:
    base = result.target.address

    def column(ins: Instruction | None) -> str:
        return f"{ins.address - base:04x} {ins.raw.hex():<14} {ins.text}" if ins else ""

    lines = [f"    {'compiled':<48} original"]
    for row in aligned_rows(result):
        mark = " " if row.same else "*"
        lines.append(f"  {mark} {column(row.compiled):<48.48} {column(row.original)}")
    return lines


def decomp_sources() -> list[Path]:
    return sorted([*paths.DECOMP_DIR.rglob("*.cpp"), *paths.DECOMP_DIR.rglob("*.c")])


def default_release() -> str:
    installed = toolchain.installed_releases()
    if not installed:
        raise typer.BadParameter("no toolchain installed; run `uv run toolchain setup` first")
    return DEFAULT_RELEASE if DEFAULT_RELEASE in installed else installed[0]


def game_executable() -> Executable:
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")
    return Executable(game)


@dataclass(frozen=True)
class Checked:
    """The outcome of checking one marked function."""

    source: Path
    target: Target
    result: Result | None  # None if it couldn't be compiled or found
    error: str | None = None
    cached: bool = False  # its source's object came from the cache


def check(
    sources: list[Path],
    release: str,
    exe: Executable,
    flags: str | None = None,
    *,
    use_cache: bool = True,
) -> list[Checked]:
    """Compile each source (or reuse its cached object) and compare every
    function marked in it."""
    checked = []
    for source in sources:
        text = source.read_text()
        targets = find_targets(text)
        if not targets:
            continue
        try:
            compiled = compile_source(
                release, source.resolve(), _flags_for(text, flags), use_cache=use_cache
            )
        except RuntimeError as e:
            checked += [Checked(source, t, None, str(e)) for t in targets]
            continue
        obj, cached = compiled.obj, compiled.cached
        siblings = {}
        for target in targets:
            try:
                public = _find_public(obj, target.name)
            except ValueError:
                continue
            siblings[public.segment, public.offset] = Sibling(target.address, public.name)
        for target in targets:
            try:
                result = compare(target, obj, exe, siblings)
                checked.append(Checked(source, target, result, cached=cached))
            except ValueError as e:
                checked.append(Checked(source, target, None, str(e), cached=cached))
    return checked


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
    no_cache: Annotated[
        bool, typer.Option("--no-cache", help="Recompile everything, ignoring build/match-cache")
    ] = False,
) -> None:
    sources = files or decomp_sources()
    releases = release or [default_release()]
    exe = game_executable()

    failed = 0
    for rel in releases:
        print(f"Borland C++ {rel}:")
        results = check(sources, rel, exe, flags, use_cache=not no_cache)
        for item in results:
            target, result = item.target, item.result
            where = f"{target.name} @ {target.address:#x}"
            if result is None:
                failed += 1
                print(f"  ERROR     {where}: {item.error}")
                continue
            size = len(result.compiled)
            if result.matches:
                note = (
                    f" (marked {target.marker}: remove the mark)"
                    if target.marker != Marker.EXACT
                    else ""
                )
                relocated = len(result.masked)
                print(f"  match     {where}: {size} bytes ({relocated} relocated){note}")
            elif target.marker != Marker.EXACT:
                differ = f"{len(result.mismatches)} of {size} bytes differ"
                label = "nonmatch " if target.marker == Marker.NONMATCHING else "functional"
                print(f"  {label:9} {where}: {differ} (marked {target.marker})")
            else:
                failed += 1
                print(f"  MISMATCH  {where}: {len(result.mismatches)} of {size} bytes differ")
                if not quiet:
                    print("\n".join(_side_by_side(result)))
        by_source = {item.source: item.cached for item in results}
        reused = sum(by_source.values())
        print(f"  ({len(by_source) - reused} files compiled, {reused} from the cache)")
    if failed:
        raise typer.Exit(1)
