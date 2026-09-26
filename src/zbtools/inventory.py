"""Every function in zoombi32.exe, with where it is, what it calls and how far its
decompilation has got. Shared by `uv run worklist` and `uv run report`.

Regions, derived from the recovered symbols (see docs/findings.md):

    startup  Borland's Win32 startup code (C0W32.OBJ), at the entry point
    game     the game's own code
    quicktime  QuickTime for Windows' glue, linked from Apple's SDK (quicktime.py)
    runtime  the Borland C++ runtime library: from its first identified function
             to the end of the last library segment found in the game (the
             linker puts the libraries together, so all of it is library code,
             including static functions with no public name)
    engine   Broderbund's Mohawk engine: everything after the runtime (its C
             code, then its classes); also the methods of its classes below
             the runtime (the threading classes)

Type descriptors and vtables, which Ghidra sometimes mistakes for code, are left
out.
"""

import re
from collections.abc import Iterable
from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path

from zbtools import ghidra, match, quicktime, rtti, runtime_symbols
from zbtools.exe import Executable, disassemble

_DIRECT_CALL = re.compile(r"^call (0x[0-9a-f]+)$")
# Classes that come from the Borland runtime rather than Broderbund's engine.
_LIBRARY_CLASSES = {
    "typeinfo", "TStringRef", "xalloc", "xmsg", "string", "string::lengtherror",
    "string::outofrange", "Bad_cast", "Bad_typeid", "fileSpec",
}  # fmt: skip


class Region(StrEnum):
    STARTUP = "startup"
    GAME = "game"
    QUICKTIME = "quicktime"
    RUNTIME = "runtime"
    ENGINE = "engine"


class Status(StrEnum):
    MATCHED = "matched"  # decompiled and marked @zoombi32
    NONMATCHING = "nonmatching"  # decompiled, marked @zoombi32-nonmatching
    LIBRARY = "library"  # library code (runtime, QuickTime glue): nothing to decompile
    TODO = "todo"


@dataclass(frozen=True)
class Decompiled:
    """A function marked in decomp/."""

    source: Path
    target: match.Target


@dataclass(frozen=True)
class Function:
    address: int
    name: str  # as decompiled if it has been, else Ghidra's
    size: int
    region: Region
    calls: tuple[int, ...]  # direct call targets
    indirect_calls: int  # calls through registers or memory (vtables, pointers)
    status: Status
    decompiled: Decompiled | None


@dataclass(frozen=True)
class Regions:
    game_start: int
    runtime_start: int
    engine_start: int

    def of(self, address: int) -> Region:
        if address < self.game_start:
            return Region.STARTUP
        if address < self.runtime_start:
            return Region.GAME
        if address < self.engine_start:
            return Region.RUNTIME
        return Region.ENGINE


def _engine_methods(
    classes: list[rtti.ClassInfo], runtime: runtime_symbols.RuntimeSymbols
) -> set[int]:
    """Destructors and virtual methods of Broderbund's (non-library) classes,
    except runtime functions that fill vtable slots (e.g. the pure-virtual
    error handler)."""
    library = {s.address for s in runtime.symbols}
    return {
        address
        for c in classes
        if c.name not in _LIBRARY_CLASSES
        for address in [
            *(m for v in c.vtables for m in v.methods),
            *([c.destructor] if c.destructor else []),
        ]
        if address not in library
    }


def _regions(
    functions: list[ghidra.FunctionInfo],
    runtime: runtime_symbols.RuntimeSymbols,
    classes: list[rtti.ClassInfo],
) -> Regions:
    # The startup code is the first object linked: everything up to the second
    # function not named by the runtime libraries.
    ordered = sorted(functions, key=lambda f: f.address)
    named = {s.address for s in runtime.symbols}
    game_start = next(f.address for f in ordered[1:] if f.address not in named)
    runtime_start = min(s.address for s in runtime.symbols if s.address > game_start)
    runtime_end = max(s.address + s.size for s in runtime.segments)
    engine_start = min(f.address for f in ordered if f.address >= runtime_end)
    return Regions(game_start, runtime_start, engine_start)


def decompiled_targets(sources: Iterable[Path] | None = None) -> dict[int, Decompiled]:
    """Functions marked in decomp/ (or the given sources), by address."""
    found = {}
    for source in sources if sources is not None else match.decomp_sources():
        for target in match.find_targets(source.read_text()):
            found[target.address] = Decompiled(source, target)
    return found


def load(exe: Executable) -> list[Function]:
    functions = ghidra.load_functions().functions
    runtime = runtime_symbols.load()
    classes = rtti.load().classes
    regions = _regions(functions, runtime, classes)
    engine_methods = _engine_methods(classes, runtime)
    data = {c.descriptor for c in classes} | {v.address for c in classes for v in c.vtables}
    library = {s.address for s in runtime.symbols}
    decompiled = decompiled_targets()

    inventory = []
    for f in sorted(functions, key=lambda f: f.address):
        if f.thunk or f.address in data:
            continue
        calls, indirect = [], 0
        for ins in disassemble(exe.read(f.address, f.size), f.address):
            if ins.text.startswith("call"):
                direct = _DIRECT_CALL.match(ins.text)
                if direct:
                    calls.append(int(direct.group(1), 16))
                else:
                    indirect += 1
        region = regions.of(f.address)
        # Engine class methods below the runtime (the threading classes) are engine code.
        if f.address in engine_methods and region == Region.GAME:
            region = Region.ENGINE
        if quicktime.in_glue(f.address):
            region = Region.QUICKTIME
        done = decompiled.get(f.address)
        if done is not None:
            status = Status.NONMATCHING if done.target.nonmatching else Status.MATCHED
        elif f.address in library or region in (Region.RUNTIME, Region.QUICKTIME):
            status = Status.LIBRARY
        else:
            status = Status.TODO
        inventory.append(
            Function(
                address=f.address,
                name=done.target.name if done else f.name,
                size=f.size,
                region=region,
                calls=tuple(dict.fromkeys(calls)),
                indirect_calls=indirect,
                status=status,
                decompiled=done,
            )
        )
    return inventory
