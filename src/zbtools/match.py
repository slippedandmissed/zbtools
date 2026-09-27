"""Check whether decompiled functions compile to the original bytes.

The decompiled code is C++ (the game is: it uses C++ objects and exceptions).
Each function in decomp/*.cpp that should match the game is preceded by a
marker comment giving its address in zoombi32.exe:

    /* @zoombi32 0x0046be2e */
    void fn_46be2e(long value)

Methods are marked the same way (`void Widget::set(long v)`). The function is
found in the compiled object by its demangled, qualified name.

Whether a function matches is measured, not declared. decomp/matching.txt,
written by `--update` (the pre-commit hook runs it), records which functions
match; `match` fails if one of them stops matching (a regression), and
reports new matches. A function that is complete but deliberately not
byte-exact is marked `/* @zoombi32-functional 0x... */`: decompiled code must
be portable C++, so where only machine code (inline assembly, emitted bytes,
pseudo-registers) would reproduce the original, the function does the same
thing portably instead. A function the compiler generates by itself (an
implicit destructor, say) has no definition to mark; a marker naming it,
`/* @zoombi32-implicit 0x0048a6f9 DIB8Port::~DIB8Port */`, has it measured
from the object of the file the marker is in. A global object's constructor
and destructor calls are compiled into unnamed functions, which the object's
_INIT_ and _EXIT_ segments list: `<startup>` and `<exit>` name them (`<startup
2>` the second, and so on).

Each file is compiled (in parallel, one Wine process per file) with
Borland C++ 4.5, or the release a `/* @release 5.02 */` comment in the file
names, using the game's usual options
(`-p -k-`), or those a `/* @flags ... */` comment gives; `--release` and
`--flags` override them for every file (to explore: the record of what
matches only applies without `--release`). Every marked function is
compared with the original, ignoring the bytes the linker fills in
(relocated addresses and call targets). Calls the compiler resolves itself,
to other marked functions in the same file, must reach the function at the
callee's marker address. Mismatches are shown side by side.
"""

import difflib
import hashlib
import itertools
import os
import re
import shlex
import tempfile
from collections.abc import Collection
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from zbtools import omf, paths, toolchain
from zbtools.demangle import demangle, qualified_name
from zbtools.exe import Executable, Instruction, disassemble

# Compiler options used unless a file says otherwise (/* @flags ... */) or --flags
# is given: most of the game's code was compiled with -p (Pascal calling
# convention by default) and -k- (no stack frame unless needed), otherwise BCC32's
# defaults (no optimisation, register variables, byte alignment). The support
# library just below the runtime used -p alone. See docs/findings.md.
DEFAULT_FLAGS = "-p -k-"
_FLAGS = re.compile(r"/\*\s*@flags\s+(.*?)\s*\*/")
# Release used unless a file says otherwise (/* @release ... */) or --release is
# given. 4.5 and 4.52 generate identical code unless 4.52's -fp (Pentium FDIV
# workaround) is used, which the game doesn't.
DEFAULT_RELEASE = "4.5"
_RELEASE = re.compile(r"/\*\s*@release\s+(\S+)\s*\*/")

_MARKER = re.compile(r"/\*\s*@zoombi32(?:-(functional))?\s+(0x[0-9a-fA-F]+)\s*\*/")
# A compiler-generated function, named in the marker: `@zoombi32-implicit 0x... A::~A`.
_IMPLICIT = re.compile(
    r"/\*\s*@zoombi32-implicit\s+(0x[0-9a-fA-F]+)\s+"
    r"((?:[A-Za-z_]\w*::)*~?[A-Za-z_]\w*|<(?:startup|exit)(?: \d+)?>)\s*\*/"
)
# The (possibly qualified) name of the function defined after a marker.
_DEFINITION = re.compile(
    r"((?:[A-Za-z_][\w:]*::)?operator\s*(?:new|delete|\[\]|\(\)|[-+*/%^&|~!=<>]+)(?:\[\])?"
    r"|[A-Za-z_~][\w:~]*)\s*\("
)
# How the demangler writes an operator's name: `operator =`, `operator new`.
_OPERATOR = re.compile(r"operator\s*")


class Marker(StrEnum):
    """What a decompiled function's marker says about it."""

    DECOMPILED = "decompiled"  # @zoombi32: whether it matches is measured
    FUNCTIONAL = "functional"  # @zoombi32-functional: equivalent, not exact by design


@dataclass(frozen=True)
class Target:
    name: str
    address: int
    marker: Marker = Marker.DECOMPILED
    # The parameters' types, to tell overloads apart (see `parameter_types`).
    parameters: str | None = None


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
    """Where the markers of functions defined in a source are."""
    return [
        MarkerPosition(int(m.group(2), 16), m.start(), m.end()) for m in _MARKER.finditer(source)
    ]


def implicit_markers(source: str) -> dict[int, str]:
    """The markers of compiler-generated functions in a source, by address."""
    return {int(m.group(1), 16): m.group(0) for m in _IMPLICIT.finditer(source)}


# Words that can end a parameter's type; any other last word is its name.
_TYPE_WORDS = frozenset(
    [
        "void",
        "char",
        "short",
        "int",
        "long",
        "unsigned",
        "signed",
        "float",
        "double",
        "const",
        "volatile",
    ]
)


def parameter_types(parameters: str) -> str:
    """A parameter list's types, as the demangler writes them without spaces
    (`const Color &color, short n` gives `constColor&,short`)."""
    types = []
    for parameter in parameters.split(","):
        tokens = re.findall(r"[A-Za-z_]\w*|[*&]", parameter)
        if len(tokens) > 1 and tokens[-1][0].isalpha() and tokens[-1] not in _TYPE_WORDS:
            tokens = tokens[:-1]
        types.append("".join(tokens))
    return "" if types == ["void"] else ",".join(t for t in types if t)


def find_targets(source: str) -> list[Target]:
    """Marked functions: the first `name(` after each @zoombi32 marker, then
    the functions @zoombi32-implicit markers name."""
    found = []
    for marker in _MARKER.finditer(source):
        name = _DEFINITION.search(source, marker.end())
        if name is None:
            raise ValueError(f"no function after marker {marker.group(0)}")
        address = int(marker.group(2), 16)
        kind = Marker(marker.group(1)) if marker.group(1) else Marker.DECOMPILED
        close = source.find(")", name.end())
        parameters = parameter_types(source[name.end() : close]) if close >= 0 else None
        found.append(Target(_OPERATOR.sub("operator ", name.group(1)), address, kind, parameters))
    for marker in _IMPLICIT.finditer(source):
        found.append(Target(marker.group(2), int(marker.group(1), 16)))
    return found


def _find_public(obj: omf.ObjectFile, name: str, parameters: str | None = None) -> omf.Public:
    """The public symbol for a function name, qualified (`Widget::set`) or not;
    overloads are told apart by their parameters' types."""

    def matches(public: omf.Public) -> bool:
        qualified = qualified_name(public.name)
        names = (qualified, qualified.split("::")[-1])
        if public.name.isupper():  # __pascal: the whole mangled name is upper-cased
            return name.upper() in names
        return name in names

    candidates = [p for p in obj.publics if matches(p)]
    if len(candidates) > 1 and parameters is not None:
        candidates = [p for p in candidates if _demangled_parameters(p.name) == parameters]
    if len(candidates) != 1:
        found = [qualified_name(p.name) for p in obj.publics]
        problem = "is ambiguous (qualify it or rename an overload)" if candidates else "not found"
        raise ValueError(f"{name} {problem} in the object file; it has {found}")
    return candidates[0]


# A global object's constructor or destructor calls: `<startup>`, `<exit 2>`.
_STARTUP = re.compile(r"<(startup|exit)(?: (\d+))?>")


def _startup_functions(obj: omf.ObjectFile) -> dict[str, list[tuple[str, int]]]:
    """The functions an object's _INIT_ and _EXIT_ segments list, as (segment,
    offset) in order: their 6-byte entries are a calling convention byte, a
    priority byte and the function's address."""
    return {
        name: sorted(
            (f.target, int.from_bytes(obj.segments[name].data[f.offset : f.offset + 4], "little"))
            for f in obj.segments[name].fixups
        )
        for name in ("_INIT_", "_EXIT_")
        if name in obj.segments
    }


def locate(obj: omf.ObjectFile, target: Target) -> tuple[omf.Public, int, int]:
    """Where a target is in an object: its symbol, and its (start, end) in the
    symbol's segment. A function runs to the next symbol or startup or exit
    function. Those have no symbol: they're found through the _INIT_ and
    _EXIT_ segments."""
    listed = _startup_functions(obj)
    startup = _STARTUP.fullmatch(target.name)
    if startup is None:
        public = _find_public(obj, target.name, target.parameters)
        segment, start = public.segment, public.offset
    else:
        entries = listed.get("_INIT_" if startup.group(1) == "startup" else "_EXIT_", [])
        number = int(startup.group(2) or 1)
        if not 0 < number <= len(entries):
            raise ValueError(f"{target.name} not found: the object lists {len(entries)}")
        segment, start = entries[number - 1]
        public = omf.Public(target.name, segment, start, local=True)
    others = [o for ts in listed.values() for t, o in ts if t == segment and o > start]
    _, end = obj.extent(omf.Public(target.name, segment, start, local=True))
    return public, start, min([*others, end])


def _demangled_parameters(symbol: str) -> str:
    signature = demangle(symbol)
    return signature[signature.find("(") + 1 : signature.rfind(")")].replace(" ", "")


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
    too), as the offset of their operand -> the callee's symbol.

    Calls are found by scanning for the opcode at every offset, not by
    disassembling: a switch's jump table inside a function would throw a
    linear disassembly out of step. A stray 0xE8 byte doesn't count unless
    both our code and the original resolve it to the same marked sibling."""
    segment, start = at
    found = {}
    for offset in range(len(compiled) - 4):
        operand = range(offset + 1, offset + 5)
        if compiled[offset] != 0xE8 or any(i in masked for i in operand):
            continue
        after = offset + 5
        ours = start + after + int.from_bytes(compiled[offset + 1 : after], "little", signed=True)
        theirs = (
            address + after + int.from_bytes(original[offset + 1 : after], "little", signed=True)
        )
        callee = siblings.get((segment, ours))
        if callee is not None and callee.address == theirs:
            found[offset + 1] = callee.symbol
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
    public, start, end = locate(obj, target)
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


def release_for(source: str, override: str | None = None) -> str:
    """The release to compile a file with: --release, else its
    /* @release ... */, else the default."""
    if override is not None:
        return override
    directive = _RELEASE.search(source)
    return directive.group(1) if directive else DEFAULT_RELEASE


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
    # Each compile runs in a directory of its own: BCC32 writes scratch files
    # into its working directory, and parallel compiles sharing one clobber
    # each other's (a compile then fails without a message).
    with tempfile.TemporaryDirectory(dir=cache_dir) as workdir:
        result = toolchain.run_tool(
            release, "BCC32", [*args, toolchain.windows_path(source)], Path(workdir)
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


def require_toolchain() -> None:
    if not toolchain.installed_releases():
        raise typer.BadParameter("no toolchain installed; run `uv run toolchain setup` first")


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


def _compile_all(
    jobs: list[tuple[str, Path, str]], *, use_cache: bool
) -> list[Compiled | RuntimeError]:
    """Compile (or fetch from the cache) each (release, source, flags), in
    parallel: each compile is a separate Wine process, mostly waiting."""

    def one(job: tuple[str, Path, str]) -> Compiled | RuntimeError:
        release, source, flags = job
        try:
            return compile_source(release, source, flags, use_cache=use_cache)
        except RuntimeError as e:
            return e

    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        results = list(pool.map(one, jobs))
    # Now and then a compile under Wine dies without a word (no errors, no
    # object) when many run at once; those are compiled again, one at a time.
    return [
        one(job) if isinstance(result, RuntimeError) and _silent(result) else result
        for job, result in zip(jobs, results, strict=True)
    ]


def _silent(error: RuntimeError) -> bool:
    """Whether a failed compile reported no errors or warnings."""
    return not re.search(r"^(Error|Warning|Fatal)\b", str(error), re.MULTILINE)


def check(
    sources: list[Path],
    exe: Executable,
    release: str | None = None,
    flags: str | None = None,
    *,
    use_cache: bool = True,
) -> list[Checked]:
    """Compile each source (or reuse its cached object) with its release, or
    `release` if given, and compare every function marked in it."""
    checked = []
    installed = toolchain.installed_releases()
    toolchain.ensure_prefix()  # once, before the compiles run in parallel
    pending: list[tuple[Path, list[Target], tuple[str, Path, str]]] = []
    for source in sources:
        text = source.read_text()
        targets = find_targets(text)
        if not targets:
            continue
        file_release = release_for(text, release)
        if file_release not in installed:
            error = f"Borland C++ {file_release} isn't installed (uv run toolchain setup)"
            checked += [Checked(source, t, None, error) for t in targets]
            continue
        pending.append((source, targets, (file_release, source.resolve(), _flags_for(text, flags))))
    compiled_all = _compile_all([job for _, _, job in pending], use_cache=use_cache)
    for (source, targets, _), compiled in zip(pending, compiled_all, strict=True):
        if isinstance(compiled, RuntimeError):
            checked += [Checked(source, t, None, str(compiled)) for t in targets]
            continue
        obj, cached = compiled.obj, compiled.cached
        siblings = {}
        for target in targets:
            try:
                public, _, _ = locate(obj, target)
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


BASELINE = paths.DECOMP_DIR / "matching.txt"
_BASELINE_HEADER = """\
# Functions that match zoombi32.exe byte for byte, by address. Written by
# `uv run match --update` (the pre-commit hook runs it); `uv run match` fails
# if one of them stops matching.
"""


def load_baseline(path: Path = BASELINE) -> dict[int, str]:
    """The functions recorded as matching: address -> name."""
    if not path.exists():
        return {}
    found = {}
    for line in path.read_text().splitlines():
        entry = line.split("#")[0].split()
        if entry:
            found[int(entry[0], 16)] = entry[1]
    return found


def write_baseline(matching: dict[int, str], path: Path = BASELINE) -> None:
    lines = [f"{address:#010x} {name}" for address, name in sorted(matching.items())]
    path.write_text(_BASELINE_HEADER + "".join(f"{line}\n" for line in lines))


class Outcome(StrEnum):
    """How a checked function compares with its expected state."""

    MATCH = "match"  # exact, and recorded as matching
    NEW_MATCH = "new match"  # exact, not recorded yet
    NONMATCHING = "nonmatching"  # differs (not recorded as matching)
    FUNCTIONAL = "functional"  # differs, marked functional
    FUNCTIONAL_EXACT = "exact but marked functional"
    REGRESSED = "regressed"  # recorded as matching, but differs
    ERROR = "error"  # couldn't be compiled or found

    @property
    def failure(self) -> bool:
        return self in (Outcome.REGRESSED, Outcome.ERROR)

    @property
    def discrepancy(self) -> bool:
        """Whether the measured state disagrees with the recorded one."""
        return self in (Outcome.NEW_MATCH, Outcome.FUNCTIONAL_EXACT, Outcome.REGRESSED)


def outcome(item: Checked, baseline: Collection[int]) -> Outcome:
    result, target = item.result, item.target
    if result is None:
        return Outcome.ERROR
    if result.matches:
        if target.marker == Marker.FUNCTIONAL:
            return Outcome.FUNCTIONAL_EXACT
        return Outcome.MATCH if target.address in baseline else Outcome.NEW_MATCH
    if target.address in baseline:
        return Outcome.REGRESSED
    return Outcome.FUNCTIONAL if target.marker == Marker.FUNCTIONAL else Outcome.NONMATCHING


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    files: Annotated[
        list[Path] | None,
        typer.Argument(help="Source files to check (default: all of decomp/)", show_default=False),
    ] = None,
    *,
    release: Annotated[
        list[str] | None,
        typer.Option(
            "--release",
            "-r",
            help="Borland C++ release(s) to use for every file (default: each file's @release, "
            f"or {DEFAULT_RELEASE})",
        ),
    ] = None,
    flags: Annotated[
        str | None,
        typer.Option(
            help=f"BCC32 options for all files (default: each file's @flags, or {DEFAULT_FLAGS})"
        ),
    ] = None,
    quiet: Annotated[bool, typer.Option("--quiet", "-q", help="Don't show disassembly")] = False,
    no_cache: Annotated[
        bool, typer.Option("--no-cache", help="Recompile everything, ignoring build/match-cache")
    ] = False,
    update: Annotated[
        bool, typer.Option("--update", help=f"Record what matches in {BASELINE.name}")
    ] = False,
    allow_regressions: Annotated[
        bool,
        typer.Option(help="With --update, drop functions that no longer match from the record"),
    ] = False,
) -> None:
    if update and (files or release or flags):
        raise typer.BadParameter("--update checks everything with the default options")
    sources = files or decomp_sources()
    require_toolchain()
    releases: list[str | None] = [*release] if release else [None]
    exe = game_executable()
    recorded = load_baseline()

    failures = 0
    for rel in releases:
        print(f"Borland C++ {rel}:" if rel else "Borland C++ (each file's release):")
        # The record is of each file's own release; an override is just compared.
        baseline = recorded if rel is None else {}
        results = check(sources, exe, rel, flags, use_cache=not no_cache)
        outcomes = [(item, outcome(item, baseline)) for item in results]
        for item, found in outcomes:
            failures += found.failure
            _print_outcome(item, found, show=not quiet and (found.failure or bool(files)))
        checked = {item.target.address for item in results}
        missing = {a: n for a, n in baseline.items() if a not in checked} if not files else {}
        for address, name in sorted(missing.items()):
            failures += 1
            print(f"  REGRESSED  {name} @ {address:#x}: recorded as matching, but no longer marked")
        by_source = {item.source: item.cached for item in results}
        reused = sum(by_source.values())
        print(f"  ({len(by_source) - reused} files compiled, {reused} from the cache)")
        new = [item for item, found in outcomes if found == Outcome.NEW_MATCH]
        if update:
            _update(outcomes, recorded, failures, allow_regressions=allow_regressions)
            return
        if new:
            print(f"{len(new)} new matches: record them with `uv run match --update`.")
    if failures:
        raise typer.Exit(1)


def _print_outcome(item: Checked, found: Outcome, *, show: bool) -> None:
    target, result = item.target, item.result
    where = f"{target.name} @ {target.address:#x}"
    if result is None:
        print(f"  ERROR      {where}: {item.error}")
        return
    size = len(result.compiled)
    if result.matches:
        notes = {
            Outcome.NEW_MATCH: " (new)",
            Outcome.FUNCTIONAL_EXACT: " (marked functional: remove the mark)",
        }
        detail = f"{size} bytes ({len(result.masked)} relocated){notes.get(found, '')}"
        print(f"  match      {where}: {detail}")
        return
    label = {Outcome.REGRESSED: "REGRESSED", Outcome.FUNCTIONAL: "functional"}.get(
        found, "nonmatch"
    )
    print(f"  {label:10} {where}: {len(result.mismatches)} of {size} bytes differ")
    if show:
        print("\n".join(_side_by_side(result)))


def _update(
    outcomes: list[tuple[Checked, Outcome]],
    recorded: dict[int, str],
    failures: int,
    *,
    allow_regressions: bool,
) -> None:
    """Write the record of what matches; refuse to drop regressions unless allowed."""
    if failures and not allow_regressions:
        print(f"Not updating {BASELINE.name}: fix the regressions, or use --allow-regressions.")
        raise typer.Exit(1)
    matching = {
        item.target.address: item.target.name
        for item, found in outcomes
        if found in (Outcome.MATCH, Outcome.NEW_MATCH)
    }
    added = sorted(set(matching) - set(recorded))
    removed = sorted(set(recorded) - set(matching))
    if matching != recorded:
        write_baseline(matching)
    print(
        f"{BASELINE.name}: {len(matching)} matching ({len(added)} added, {len(removed)} removed)."
    )
