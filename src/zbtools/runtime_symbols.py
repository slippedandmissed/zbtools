"""Name the Borland runtime-library code linked into zoombi32.exe.

The game shipped without symbols, but its runtime code was copied unchanged
from the Borland C++ libraries, apart from the addresses the linker filled in.
For each code segment of each library module, this searches the executable for
the segment's bytes, treating linker-filled bytes as wildcards. Where a segment
matches in exactly one place, each of its public symbols is named there, and
the segment's extent is recorded: everything in it is library code, including
static functions that have no public name.
"""

from pathlib import Path
from typing import Annotated

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import omf, paths, toolchain
from zbtools.demangle import demangle
from zbtools.exe import Executable

# 32-bit ("flat model") libraries and startup objects, relative to BC45/LIB.
# Debug builds and import libraries are left out.
LIBRARIES = [
    "CW32.LIB", "CW32MT.LIB", "BIDSF.LIB", "OWLWF.LIB", "OCFWF.LIB",
    "C0W32.OBJ", "C0X32.OBJ", "C0D32.OBJ", "32BIT/*.OBJ",
]  # fmt: skip
# A segment with fewer bytes than this that aren't linker-filled could match by
# coincidence, so it isn't trusted.
MIN_FIXED_BYTES = 12
_MIN_ANCHOR = 4


class RuntimeSymbol(BaseModel):
    model_config = ConfigDict(frozen=True)
    address: int
    name: str  # as in the library: Borland-mangled for C++
    demangled: str  # e.g. "xmsg::xmsg(const string&)"; the name itself if not C++
    library: str
    module: str


class RuntimeSegment(BaseModel):
    """A library code segment found in the executable."""

    model_config = ConfigDict(frozen=True)
    address: int
    size: int
    library: str
    module: str


class RuntimeSymbols(BaseModel):
    release: str
    symbols: list[RuntimeSymbol]
    segments: list[RuntimeSegment]
    ambiguous: list[str]  # segments that matched in more than one place


def _anchor(data: bytes, masked: frozenset[int]) -> tuple[int, int]:
    """(start, length) of the longest run of bytes the linker doesn't touch."""
    best, run_start = (0, 0), 0
    for i in range(len(data) + 1):
        if i == len(data) or i in masked:
            if i - run_start > best[1]:
                best = (run_start, i - run_start)
            run_start = i + 1
    return best


def locate(segment: omf.Segment, exe: Executable) -> list[int]:
    """Every address where the segment's code occurs in the executable."""
    data = bytes(segment.data)
    masked = frozenset(f.offset + k for f in segment.fixups for k in range(f.size))
    absolute = [f.offset for f in segment.fixups if not f.self_relative and f.size == 4]
    anchor_at, anchor_len = _anchor(data, masked)
    if anchor_len < _MIN_ANCHOR:
        return []
    start, end = exe.code_range
    code = exe.read(start, end - start)
    anchor = data[anchor_at : anchor_at + anchor_len]
    found = []
    pos = code.find(anchor)
    while pos != -1:
        base = pos - anchor_at
        if base >= 0 and base + len(data) <= len(code):
            window = code[base : base + len(data)]
            if all(i in masked or window[i] == data[i] for i in range(len(data))) and all(
                start + base + offset in exe.relocations for offset in absolute
            ):
                found.append(start + base)
        pos = code.find(anchor, pos + 1)
    return found


def _modules(path: Path) -> list[omf.ObjectFile]:
    data = path.read_bytes()
    if path.suffix.upper() == ".LIB":
        return omf.read_library(data)[0]
    return [omf.read(data)]


def find_symbols(release: str, exe: Executable) -> RuntimeSymbols:
    lib_dir = toolchain.root_dir(release) / "LIB"
    symbols: dict[tuple[int, str], RuntimeSymbol] = {}
    segments: dict[int, RuntimeSegment] = {}
    ambiguous: set[str] = set()
    for pattern in LIBRARIES:
        for path in sorted(lib_dir.glob(pattern)):
            library = str(path.relative_to(lib_dir))
            for module in _modules(path):
                for name, segment in module.segments.items():
                    publics = [p for p in module.publics if p.segment == name]
                    masked = {f.offset + k for f in segment.fixups for k in range(f.size)}
                    fixed = len(segment.data) - len(masked)
                    if segment.class_name != "CODE" or fixed < MIN_FIXED_BYTES:
                        continue
                    places = locate(segment, exe)
                    if len(places) > 1:
                        ambiguous.add(f"{module.name} {name}: {len(places)} places")
                    elif places:
                        segments.setdefault(
                            places[0],
                            RuntimeSegment(
                                address=places[0],
                                size=len(segment.data),
                                library=library,
                                module=module.name,
                            ),
                        )
                        for public in publics:
                            address = places[0] + public.offset
                            symbols.setdefault(
                                (address, public.name),
                                RuntimeSymbol(
                                    address=address,
                                    name=public.name,
                                    demangled=demangle(public.name),
                                    library=library,
                                    module=module.name,
                                ),
                            )
    return RuntimeSymbols(
        release=release,
        symbols=sorted(symbols.values(), key=lambda s: (s.address, s.name)),
        segments=sorted(segments.values(), key=lambda s: s.address),
        ambiguous=sorted(ambiguous),
    )


def load() -> RuntimeSymbols:
    if not paths.RUNTIME_SYMBOLS.exists():
        raise typer.BadParameter(
            f"{paths.RUNTIME_SYMBOLS} not found; run `uv run runtime-symbols` first"
        )
    return RuntimeSymbols.model_validate_json(paths.RUNTIME_SYMBOLS.read_text())


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    release: Annotated[
        str | None, typer.Option(help="Borland C++ release whose libraries to use")
    ] = None,
) -> None:
    releases = toolchain.installed_releases()
    if not releases:
        raise typer.BadParameter("no toolchain installed; run `uv run toolchain setup` first")
    chosen = release or releases[0]
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")

    found = find_symbols(chosen, Executable(game))
    paths.SYMBOLS_DIR.mkdir(parents=True, exist_ok=True)
    paths.RUNTIME_SYMBOLS.write_text(found.model_dump_json(indent=1))
    addresses = {s.address for s in found.symbols}
    print(
        f"Named {len(addresses)} addresses ({len(found.symbols)} symbols) from Borland C++ "
        f"{chosen}'s libraries, in {len(found.segments)} code segments "
        f"({sum(s.size for s in found.segments)} bytes); "
        f"{len(found.ambiguous)} segments matched in several places."
    )
    print(f"Written to {paths.RUNTIME_SYMBOLS}")
