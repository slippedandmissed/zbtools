"""Recover C++ classes from the RTTI type descriptors Borland C++ leaves in
zoombi32.exe: class names, sizes, base classes, destructors, vtables (with
their virtual methods) and constructors.

Borland's 32-bit type descriptor layout (worked out from the runtime library's
own descriptors, see docs/findings.md):

    +0x00  object size
    +0x04  flags (0x0001: a class; 0x0002: has a destructor and more fields)
    +0x06  offset of the class name within the descriptor
    +0x08  offset of the vtable pointer in objects (-1: no vtable)
    +0x10  offset of the base-class list: (descriptor, offset, flags) entries,
           ended by a null descriptor
    +0x28  the destructor

and in the data section, each vtable is preceded by a pointer to the class's
descriptor and two zero words; constructors store the vtable's address.
"""

import re
from collections import defaultdict

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import paths
from zbtools.exe import Executable

_CLASS, _HAS_DESTRUCTOR = 0x0001, 0x0002
_NAME = re.compile(rb"[A-Za-z_][A-Za-z0-9_:<>,*& ]{0,79}\x00")
_VTABLE_AFTER_DESCRIPTOR = 12


class Vtable(BaseModel):
    model_config = ConfigDict(frozen=True)
    address: int
    methods: list[int]  # virtual function addresses, in slot order


class ClassInfo(BaseModel):
    model_config = ConfigDict(frozen=True)
    name: str
    descriptor: int
    size: int
    vptr_offset: int | None  # where objects hold their vtable pointer
    bases: list[str]
    destructor: int | None
    vtables: list[Vtable]
    constructors: list[int]  # addresses of instructions storing a vtable pointer


class Classes(BaseModel):
    classes: list[ClassInfo]


def _u16(exe: Executable, address: int) -> int:
    return int.from_bytes(exe.read(address, 2), "little")


def _descriptor(exe: Executable, address: int, end: int) -> tuple[str, int, int] | None:
    """(name, flags, name offset) if a class descriptor starts at address."""
    flags, name_offset = _u16(exe, address + 4), _u16(exe, address + 6)
    full = flags & 0x0003 == 0x0003 and name_offset == 0x30
    short = flags & 0x0003 == _CLASS and name_offset == 0x10
    if not (full or short) or address + name_offset + 2 > end:
        return None
    size = exe.pointer(address)
    if not 0 < size < 0x10000:
        return None
    match = _NAME.match(exe.read(address + name_offset, min(81, end - address - name_offset)))
    return (match.group(0)[:-1].decode(), flags, name_offset) if match else None


def _code_pointers(exe: Executable, address: int, stop: set[int]) -> list[int]:
    """Consecutive relocated pointers into the code section, starting at address,
    up to one that points to an address in `stop`."""
    start, end = exe.code_range
    found = []
    while (
        address in exe.relocations
        and start <= exe.pointer(address) < end
        and exe.pointer(address) not in stop
    ):
        found.append(exe.pointer(address))
        address += 4
    return found


def find_classes(exe: Executable) -> list[ClassInfo]:
    code_start, code_end = exe.code_range
    data_start, data_end = exe.sections["DATA"]
    # Relocated pointers by the value they hold: who refers to what.
    referrers: dict[int, list[int]] = defaultdict(list)
    for site in exe.relocations:
        referrers[exe.pointer(site)].append(site)

    descriptors: dict[int, tuple[str, int, int]] = {}
    for start, end in (exe.code_range, exe.sections["DATA"]):
        for address in range(start, end - 0x40):
            found = _descriptor(exe, address, end)
            if found:
                descriptors[address] = found

    classes = []
    for address, (name, flags, _) in sorted(descriptors.items()):
        full = bool(flags & _HAS_DESTRUCTOR)
        vptr = int.from_bytes(exe.read(address + 8, 4), "little", signed=True)
        bases = []
        if full:
            entry = address + _u16(exe, address + 0x10)
            while exe.pointer(entry) in descriptors:
                bases.append(descriptors[exe.pointer(entry)][0])
                entry += 12
        destructor = exe.pointer(address + 0x28) if full else None
        vtables = []
        # A class without a vtable can still have its descriptor referenced from
        # data (e.g. exception tables), so only look for vtables where one exists.
        for site in sorted(referrers.get(address, []) if vptr != -1 else []):
            if data_start <= site < data_end:
                vtable = site + _VTABLE_AFTER_DESCRIPTOR
                # The next vtable's header (a descriptor pointer) ends this one.
                methods = _code_pointers(exe, vtable, stop=set(descriptors))
                vtables.append(Vtable(address=vtable, methods=methods))
        constructors = sorted(
            site
            for vtable in vtables
            for site in referrers.get(vtable.address, [])
            if code_start <= site < code_end
        )
        classes.append(
            ClassInfo(
                name=name,
                descriptor=address,
                size=exe.pointer(address),
                vptr_offset=None if vptr == -1 else vptr,
                bases=bases,
                destructor=destructor
                if destructor and code_start <= destructor < code_end
                else None,
                vtables=vtables,
                constructors=constructors,
            )
        )
    return classes


def load() -> Classes:
    if not paths.CLASSES.exists():
        raise typer.BadParameter(f"{paths.CLASSES} not found; run `uv run classes` first")
    return Classes.model_validate_json(paths.CLASSES.read_text())


def _print_tree(classes: list[ClassInfo]) -> None:
    by_name = {c.name: c for c in classes}
    children: dict[str, list[str]] = defaultdict(list)
    for c in classes:
        for base in c.bases:
            children[base].append(c.name)

    def show(name: str, depth: int) -> None:
        c = by_name[name]
        methods = max((len(v.methods) for v in c.vtables), default=0)
        details = f"size {c.size:#x}" + (f", {methods} virtual methods" if methods else "")
        print(f"  {'    ' * depth}{name} ({details})")
        for child in sorted(children[name]):
            show(child, depth + 1)

    for c in sorted(classes, key=lambda c: c.name.lower()):
        if not c.bases:
            show(c.name, 0)


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main() -> None:
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")
    classes = find_classes(Executable(game))
    paths.SYMBOLS_DIR.mkdir(parents=True, exist_ok=True)
    paths.CLASSES.write_text(Classes(classes=classes).model_dump_json(indent=1))
    print(f"Found {len(classes)} classes:")
    _print_tree(classes)
    print(f"Written to {paths.CLASSES}")
