"""Define the globals decomp/'s headers declare but no source defines.

Each global a header declares with an address (`extern short primes[5];
/* @data 0x4a0800 */`) is defined in the source of the module that owns it,
with its initial value read from the original:

- **Size:** the type's size by our headers, checked against BCC32's own
  `sizeof` (a probe file compiled into build/match-cache/). A global declared
  without a length (`extern char name[];`) runs to the next declared global
  or placed piece of data (`uv run match-data`).
- **Owner:** the module whose code refers to it in the original. A global
  several modules use, or none, goes to the module its neighbours in the
  original belong to (each module's data is contiguous there).
- **Value:** uninitialised globals (the original's zero-filled data) are
  defined plainly; initialised ones get an initialiser rendered from the
  original's bytes by their type: numbers, text as string literals, pointers
  as what they point to (a function, a global or element of one, or a
  string), structs and arrays in braces.

Definitions go among the source's other definitions of the same kind, in
address order (so the compiled layout follows the original's), or before its
first function. Then each changed source's includes are updated (`uv run
includes`) and it's compiled, to report what doesn't compile yet.

A global is left for a person, with the reason, when this can't do it
exactly: its type doesn't match the space it has in the original (it runs
into the next global), a type or pointer can't be rendered, or its owner is
unclear.
"""

import bisect
import itertools
import re
from collections import Counter
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import typer

from zbtools import declarations, includes, inventory, match, match_data, paths
from zbtools.exe import Executable

# fmt: off
_SCALARS: dict[str, tuple[int, bool]] = {
    "char": (1, True), "signed char": (1, True), "unsigned char": (1, False), "BYTE": (1, False),
    "short": (2, True), "unsigned short": (2, False), "WORD": (2, False),
    "int": (4, True), "long": (4, True), "BOOL": (4, True), "LONG": (4, True),
    "unsigned": (4, False), "unsigned int": (4, False), "unsigned long": (4, False),
    "DWORD": (4, False), "UINT": (4, False), "COLORREF": (4, False), "time_t": (4, True),
}
# fmt: on
_HANDLES = {"HWND", "HINSTANCE", "HANDLE", "HDC", "HPALETTE", "HBITMAP", "HMENU", "HCURSOR"}
# Windows structs the headers use by value.
_WINDOWS_STRUCTS = {
    "RGBQUAD": "BYTE rgbBlue; BYTE rgbGreen; BYTE rgbRed; BYTE rgbReserved;",
    "PALETTEENTRY": "BYTE peRed; BYTE peGreen; BYTE peBlue; BYTE peFlags;",
}
_WIDTH = 100


class Unrenderable(ValueError):
    """Why a global can't be defined automatically."""


@dataclass(frozen=True)
class Scalar:
    name: str
    size: int
    signed: bool


@dataclass(frozen=True)
class Pointer:
    pointee: str  # the type pointed to, as written ("" for a function pointer)


@dataclass(frozen=True)
class Array:
    element: "CType"
    count: int | None  # None: declared without a length


@dataclass(frozen=True)
class Field:
    name: str
    type: "CType"


@dataclass(frozen=True)
class Struct:
    name: str
    fields: tuple[Field, ...]


CType = Scalar | Pointer | Array | Struct


def size_of(ctype: CType) -> int:
    match ctype:
        case Scalar():
            return ctype.size
        case Pointer():
            return 4
        case Array():
            if ctype.count is None:
                raise Unrenderable("an array without a length")
            return ctype.count * size_of(ctype.element)
        case Struct():
            return sum(size_of(f.type) for f in ctype.fields)  # byte packing


def _strip_comments(text: str) -> str:
    return re.sub(r"//[^\n]*", "", re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL))


# `void (*name)(Point *where)` or `short (*grid)[12]`: a pointer, however declared.
_FUNCTION_POINTER = re.compile(r"^(?P<base>[^()]*?)\(\s*\*\s*(?P<name>\w+)\s*\)\s*[(\[].*$", re.S)
_DECLARATOR = re.compile(r"^(?P<stars>[\s*]*)(?P<name>\w+)\s*(?P<dims>(?:\[[^\]]*\]\s*)*)$")


class Types:
    """The types our headers define, as sizes and fields."""

    def __init__(self, text: str) -> None:
        self.text = _strip_comments(text)
        self.structs = {
            m.group(1): m.group(2)
            for m in re.finditer(r"^struct\s+(\w+)\s*\n\{(.*?)\n\};", self.text, re.M | re.S)
        }
        self.structs |= _WINDOWS_STRUCTS
        self.function_pointers = set(
            re.findall(r"^typedef\s[^;(]*\(\s*\w*\s*\*\s*(\w+)\s*\)", self.text, re.M)
        )
        self.typedefs = {
            m.group(2): m.group(1).strip()
            for m in re.finditer(r"^typedef\s+([^;(]*?)\s*\b(\w+)\s*;", self.text, re.M)
        }
        self.cache: dict[str, Struct] = {}

    def declaration(self, text: str) -> tuple[str, CType]:
        """A declaration's name and type: `short primes[5]` gives primes, short[5]."""
        text = re.sub(r"\b(const|volatile|static)\s+", "", text.strip())
        pointer = _FUNCTION_POINTER.match(text)
        if pointer:
            return pointer.group("name"), Pointer(pointer.group("base").strip())
        base, _, declarator = text.rpartition(" ")
        if not base:
            raise Unrenderable(f"can't read the declaration {text!r}")
        found = _DECLARATOR.match(declarator)
        if found is None:
            raise Unrenderable(f"can't read the declaration {text!r}")
        base_type = self.resolve(base.strip() + " " + found.group("stars").replace(" ", ""))
        ctype: CType = base_type
        for dim in reversed(re.findall(r"\[([^\]]*)\]", found.group("dims"))):
            ctype = Array(ctype, self.length(dim) if dim.strip() else None)
        return found.group("name"), ctype

    def length(self, text: str) -> int:
        try:
            return int(text.strip(), 0)
        except ValueError:
            raise Unrenderable(f"an array length that isn't a number: {text!r}") from None

    def resolve(self, name: str) -> CType:
        name = " ".join(re.sub(r"\b(const|volatile|struct)\b", "", name).split())
        if name.endswith("*"):
            return Pointer(name[:-1].strip())
        if name in _SCALARS:
            size, signed = _SCALARS[name]
            return Scalar(name, size, signed)
        if name in self.function_pointers or name in _HANDLES:
            return Pointer("")
        if name in self.typedefs:
            return self.resolve(self.typedefs[name])
        if name in self.structs:
            return self.struct(name)
        raise Unrenderable(f"a type this can't lay out: {name}")

    def struct(self, name: str) -> Struct:
        if name not in self.cache:
            body = self.structs[name]
            if re.search(r"\b(union|struct|class)\b|:\s*\d", body):
                raise Unrenderable(f"struct {name} has unions, nested structs or bit fields")
            fields: list[Field] = []
            for part in body.split(";"):
                member = part.strip()
                if not member:
                    continue
                if _FUNCTION_POINTER.match(member):
                    fields.append(Field(*self.declaration(member)))
                    continue
                first, *more = member.split(",")
                base = first.strip().rpartition(" ")[0]
                fields.append(Field(*self.declaration(first)))
                fields += [Field(*self.declaration(f"{base} {d.strip()}")) for d in more]
            self.cache[name] = Struct(name, tuple(fields))
        return self.cache[name]


@dataclass(frozen=True)
class Global:
    name: str
    address: int
    declaration: str  # as the header has it, without `extern` and the semicolon
    type: CType | None  # None: one this can't lay out (fine uninitialised)
    problem: str | None = None  # why the type can't be laid out
    module: str | None = None  # the module whose header declares it

    @property
    def unsized(self) -> bool:
        """Declared without a length: `extern char name[];`."""
        return bool(re.search(r"\w\s*\[\s*\]", self.declaration))


_EXTERN = re.compile(r"^extern\s+(.*?);", re.MULTILINE)


def declared(types: Types) -> list[Global]:
    """The globals declared with addresses."""
    addresses = {g.name: g.address for g in declarations.load()[0]}
    found = []
    for header in declarations.headers():
        for m in _EXTERN.finditer(header.read_text()):
            text = m.group(1).strip()
            try:
                name, ctype = types.declaration(text)
                problem = None
            except Unrenderable as e:
                names = [n for n in re.findall(r"\w+", text) if n in addresses]
                if not names:
                    continue
                name, ctype, problem = names[-1], None, str(e)
            module = None if header == declarations.HEADER else header.stem
            if name in addresses:
                found.append(Global(name, addresses[name], text, ctype, problem, module))
    return sorted(found, key=lambda g: g.address)


def _defined_names() -> set[str]:
    """Names defined at the top level of some source (not `extern`)."""
    plain = re.compile(
        r"^(?!extern\b|typedef\b|return\b)[A-Za-z_][^;(=]*?\b(\w+)\s*(?:\[[^\]]*\]\s*)*(?:=|;)",
        re.M,
    )
    # `void (*hook)(short active) = 0;` or `short (*grid)[12];`
    pointer = re.compile(r"^(?!extern\b|typedef\b)[A-Za-z_][^;(=\n]*\(\s*\*\s*(\w+)\s*\)", re.M)
    found: set[str] = set()
    for source in match.decomp_sources():
        text = _strip_comments(source.read_text())
        found |= set(plain.findall(text)) | set(pointer.findall(text))
    return found


def probe_sizes(names: list[str], probe_name: str = "sizes") -> dict[str, int]:
    """BCC32's sizeof for each global (or type), from a probe file including
    every header."""
    probe = paths.MATCH_CACHE / "probe" / f"{probe_name}.cpp"
    probe.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        f'#include "{Path("../../..") / "decomp" / header.name}"'
        for header in declarations.headers()
    ]
    lines += ["unsigned long sizes[] = {", *(f"    sizeof({n})," for n in names), "};"]
    probe.write_text("\n".join(lines) + "\n")
    compiled = match.compile_source(match.DEFAULT_RELEASE, probe.resolve(), match.DEFAULT_FLAGS)
    data = compiled.obj.segments["_DATA"].data
    return {n: int.from_bytes(data[4 * i : 4 * i + 4], "little") for i, n in enumerate(names)}


def references(
    exe: Executable, globals_: list[Global], sizes: dict[str, int]
) -> dict[str, Counter[str]]:
    """The modules whose code refers to each global in the original."""
    starts = [g.address for g in globals_]
    found: dict[str, Counter[str]] = {}
    data_start, data_end = exe.sections["DATA"]
    for function in inventory.load(exe):
        if function.module is None:
            continue
        for at in range(function.address, function.address + function.size):
            if at not in exe.relocations:
                continue
            target = exe.pointer(at)
            if not data_start <= target < data_end:
                continue
            i = bisect.bisect_right(starts, target) - 1
            if i < 0:
                continue
            g = globals_[i]
            if target < g.address + max(sizes.get(g.name, 1), 1):
                found.setdefault(g.name, Counter())[function.module] += 1
    return found


def owners(
    globals_: list[Global], refs: dict[str, Counter[str]], boundary: int
) -> dict[str, str | None]:
    """Each global's module: the one using it; if several or none do, the
    module of its nearest neighbours (in the same kind of data) that are
    used by one module, when those agree with each other or with a user."""
    single = {g.name: next(iter(refs[g.name])) for g in globals_ if len(refs.get(g.name, {})) == 1}
    found: dict[str, str | None] = {}
    for kind in (True, False):
        ordered = [g for g in globals_ if (g.address < boundary) == kind]
        for i, g in enumerate(ordered):
            if g.name in single:
                found[g.name] = single[g.name]
                continue
            before = next((single[o.name] for o in reversed(ordered[:i]) if o.name in single), None)
            after = next((single[o.name] for o in ordered[i + 1 :] if o.name in single), None)
            users = refs.get(g.name, Counter())
            candidates = [m for m in (before, after) if m is not None and (not users or m in users)]
            if before is not None and before == after and (not users or before in users):
                found[g.name] = before
            elif len(set(candidates)) == 1:
                found[g.name] = candidates[0]
            else:
                found[g.name] = _fallback_owner(g, users, [m for m in (before, after) if m])
    return found


def _fallback_owner(g: Global, users: Counter[str], near: list[str]) -> str | None:
    """A module for a global its neighbours don't settle: the one whose header
    declares it, if that one uses it or is next to it; else the user (next to
    it, if any is) that uses it most; else a neighbour. Only the layout
    depends on it: any module would do for the game."""
    if g.module is not None and (g.module in users or g.module in near or not users):
        return g.module
    if users:
        close = [m for m in users if m in near]
        return max(close or list(users), key=lambda m: users[m])
    return near[0] if near else None


@dataclass
class Renderer:
    exe: Executable
    types: Types
    functions: dict[int, str]  # marked plain functions by address
    globals_: list[Global]
    sizes: dict[str, int]

    def value(self, ctype: CType, address: int, top: bool = False) -> str:
        """An initialiser for a value of this type at an address in the original."""
        size = size_of(ctype)
        data = self.exe.read(address, size)
        if not any(data) and not self._relocated(address, size):
            return "0" if isinstance(ctype, Scalar | Pointer) else "{0}"
        match ctype:
            case Scalar():
                if self._relocated(address, size):
                    raise Unrenderable(f"a number at {address:#x} holds an address")
                return _number(int.from_bytes(data, "little", signed=ctype.signed), ctype)
            case Pointer():
                return self.pointer(ctype, address)
            case Array():
                if isinstance(ctype.element, Scalar) and ctype.element.name == "char":
                    text = _text(data)
                    if text is not None:
                        return text
                items = [
                    self.value(ctype.element, address + i * size_of(ctype.element))
                    for i in range(size // size_of(ctype.element))
                ]
                return _braces(_trimmed(items), top)
            case Struct():
                items, offset = [], 0
                for f in ctype.fields:
                    items.append(self.value(f.type, address + offset))
                    offset += size_of(f.type)
                return _braces(_trimmed(items), top)

    def _relocated(self, address: int, size: int) -> bool:
        return any(address + k in self.exe.relocations for k in range(size))

    def pointer(self, ctype: Pointer, address: int) -> str:
        if address not in self.exe.relocations:
            raise Unrenderable(f"a pointer at {address:#x} holds a number")
        target = self.exe.pointer(address)
        if target in self.functions:
            return self.functions[target]
        g = self.containing(target)
        if g is not None:
            return self.address_of(g, target, ctype)
        is_text = ctype.pointee.removeprefix("const ") in ("char", "unsigned char")
        text = _text(self.exe.read(target, 256), whole=False, empty=is_text)
        if text is not None:
            return text
        raise Unrenderable(f"a pointer at {address:#x} to {target:#x}, which has no name")

    def containing(self, target: int) -> Global | None:
        for g in self.globals_:
            size = self.sizes.get(g.name)
            if size is not None and g.address <= target < g.address + max(size, 1):
                return g
        return None

    def address_of(self, g: Global, target: int, ctype: Pointer) -> str:
        """An expression for an address in a global: `&x`, `list`, `&list[3]`."""
        offset = target - g.address
        if g.type is None:
            if offset == 0:  # its start needs no layout
                return (
                    f"&{g.name}"
                    if ctype.pointee in ("", "void")
                    else f"({ctype.pointee} *)&{g.name}"
                )
            raise Unrenderable(f"a pointer into {g.name}, whose type this can't lay out")
        element: CType = g.type
        name = g.name
        while isinstance(element, Array):
            step = size_of(element.element)
            if offset % step and not isinstance(element.element, Array):
                raise Unrenderable(f"a pointer into the middle of {g.name}'s elements")
            name += f"[{offset // step}]"
            offset %= step
            element = element.element
        if offset:
            raise Unrenderable(f"a pointer into {g.name} past its start")
        expression = (
            name.removesuffix("[0]")
            if name.endswith("[0]") and name.count("[") == 1
            else f"&{name}"
        )
        pointee = _type_name(element)
        if ctype.pointee in ("", "void", pointee):
            return expression
        return f"({ctype.pointee} *){expression}"


def pointer_fields(ctype: CType, address: int) -> Iterator[tuple[Pointer, int]]:
    """The pointers in a value of this type at an address, with theirs."""
    match ctype:
        case Pointer():
            yield ctype, address
        case Array():
            step = size_of(ctype.element)
            for i in range(size_of(ctype) // step):
                yield from pointer_fields(ctype.element, address + i * step)
        case Struct():
            offset = 0
            for f in ctype.fields:
                yield from pointer_fields(f.type, address + offset)
                offset += size_of(f.type)
        case Scalar():
            return


@dataclass
class Unnamed:
    """Finds the data initialised pointers point to that nothing declares, and
    declares it: an array of what the pointer points to (`Group g_4a0df4[1]`),
    up to the next known address, in the module whose data it's among (see
    `neighbours`). What it holds is followed the same way."""

    renderer: Renderer
    starts: list[int]  # every known address in the data, sorted
    boundary: int  # the end of the initialised data
    owned: dict[str, str | None]
    found: list[Global]
    data_ends: dict[str, int]  # where each module's placed initialised data ends

    def follow(self, globals_: list[Global]) -> None:
        pending = [g for g in globals_ if g.address < self.boundary and g.type is not None]
        while pending:
            g = pending.pop()
            ctype = g.type
            assert ctype is not None
            if isinstance(ctype, Array) and ctype.count is None:
                size = self.renderer.sizes.get(g.name, 0)
                ctype = Array(ctype.element, size // size_of(ctype.element))
            try:
                fields = list(pointer_fields(ctype, g.address))
            except Unrenderable:
                continue
            for pointer, at in fields:
                target = self.declare(pointer, at, self.owned.get(g.name))
                if target is not None:
                    pending.append(target)

    def split(self, g: Global, target: int, pointee: CType) -> bool:
        """Whether a pointer of another type into one of the arrays found here
        ends that array: it was sized before what follows it was known."""
        if g not in self.found or not isinstance(g.type, Array) or g.type.element == pointee:
            return False
        step = size_of(g.type.element)
        count = (target - g.address) // step
        if count < 1 or (target - g.address) % step:
            return False
        shorter = Global(
            g.name,
            g.address,
            g.declaration.replace(f"[{g.type.count}]", f"[{count}]"),
            Array(g.type.element, count),
        )
        for items in (self.found, self.renderer.globals_):
            items[items.index(g)] = shorter
        self.renderer.sizes[g.name] = count * step
        return True

    def neighbours(self, address: int, referrer: str | None) -> str | None:
        """The module of new data at an address: each module's data is
        contiguous, so it's that of the globals on one side or the other. That
        of both, if they agree; else the referrer's, if it's one of them; else
        the one before (whose data can run on past its last known global),
        unless that one's data, which ends with its string literals, has
        already ended."""
        before = [g for g in self.renderer.globals_ if g.address < address]
        after = [g for g in self.renderer.globals_ if g.address > address]
        below = next(
            (self.owned.get(g.name) for g in reversed(before) if self.owned.get(g.name)), None
        )
        above = next((self.owned.get(g.name) for g in after if self.owned.get(g.name)), None)
        if below is not None and self.data_ends.get(below, address + 1) <= address:
            return above or below  # past the end of below's data, literals and all
        if below == above or referrer not in (below, above):
            return below or above
        return referrer

    def target(self, pointer: Pointer, at: int) -> tuple[int, CType] | None:
        """Where an initialised pointer points and to what, if that's data this
        can declare: in the initialised data, typed, not text (a string
        literal) nor a function."""
        exe, renderer = self.renderer.exe, self.renderer
        if at not in exe.relocations:
            return None
        target = exe.pointer(at)
        start = exe.sections["DATA"][0]
        untyped = pointer.pointee.removeprefix("const ") in ("", "void", "char", "unsigned char")
        if untyped or not start <= target < self.boundary or target in renderer.functions:
            return None
        try:
            pointee = renderer.types.resolve(pointer.pointee)
            size_of(pointee)
        except Unrenderable:
            return None
        return target, pointee

    def declare(self, pointer: Pointer, at: int, owner: str | None) -> Global | None:
        renderer = self.renderer
        found = self.target(pointer, at) if owner is not None else None
        if found is None:
            return None
        target, pointee = found
        step = size_of(pointee)
        inside = renderer.containing(target)
        if inside is not None and not self.split(inside, target, pointee):
            return None
        # At least one element: the pointer's type says so, whatever else seems
        # to start closer (a piece `match-data` placed wrong, say).
        later = [s for s in self.starts if s >= target + step]
        count = ((later[0] if later else self.boundary) - target) // step
        if count < 1:
            return None
        name = f"g_{target:x}"
        g = Global(name, target, f"{pointer.pointee} {name}[{count}]", Array(pointee, count))
        owner = self.neighbours(target, owner)
        bisect.insort(self.starts, target)
        bisect.insort(renderer.globals_, g, key=lambda g: g.address)
        renderer.sizes[name] = count * step
        self.owned[name] = owner
        self.found.append(g)
        return g


def _type_name(ctype: CType) -> str:
    match ctype:
        case Scalar():
            return ctype.name
        case Struct():
            return ctype.name
        case _:
            return ""


def _number(value: int, ctype: Scalar) -> str:
    if ctype.size == 4:
        chars = value.to_bytes(4, "big", signed=value < 0)
        if chars[0:1].isalpha() and all(32 <= c < 127 and chr(c) not in "'\\" for c in chars):
            return "RESOURCE_TYPE(" + ", ".join(f"'{chr(c)}'" for c in chars) + ")"
    if -1000 < value < 1000:
        return str(value)
    return f"-{-value:#x}" if value < 0 else f"{value:#x}"


def _text(data: bytes, *, whole: bool = True, empty: bool = False) -> str | None:
    """A string literal for text ending in a zero (then only zeros, if `whole`);
    an empty one only if `empty` (a pointer to text: elsewhere, zeros are
    more likely data)."""
    end = data.find(b"\0")
    if end < 0 or (whole and any(data[end:])) or (not whole and end == 0 and not empty):
        return None
    text = data[:end]
    if not all(32 <= c < 127 or c in (9, 10, 13) or c >= 0xA0 for c in text):
        return None
    return '"' + "".join(_escape(c) for c in text) + '"'


# Octal for the rest: a hex escape would swallow a following hex digit.
_ESCAPES = {ord("\\"): "\\\\", ord('"'): '\\"', 9: "\\t", 10: "\\n", 13: "\\r"}


def _escape(byte: int) -> str:
    """One byte of a string literal (Windows-1252 text beyond ASCII as octal)."""
    if byte in _ESCAPES:
        return _ESCAPES[byte]
    return chr(byte) if 32 <= byte < 127 else f"\\{byte:03o}"


def _trimmed(items: list[str]) -> list[str]:
    """Without trailing zeros (the rest of an aggregate is zeroed anyway)."""
    while len(items) > 1 and items[-1] in ("0", "{0}"):
        items.pop()
    return items


def _braces(items: list[str], top: bool) -> str:
    inline = "{" + ", ".join(items) + "}"
    if not top or len(inline) <= _WIDTH - 40:
        return inline
    lines, line = [], "   "
    for item in items:
        if len(line) + len(item) + 2 > _WIDTH and line.strip():
            lines.append(line.rstrip())
            line = "   "
        line += f" {item},"
    lines.append(line.rstrip())
    return "{\n" + "\n".join(lines) + "\n}"


@dataclass(frozen=True)
class Definition:
    glob: Global
    source: Path
    initialised: bool
    text: str


def _definition_text(g: Global, value: str | None, length: int | None) -> str:
    declaration = g.declaration
    if length is not None:
        declaration = re.sub(r"\[\s*\]", f"[{length}]", declaration, count=1)
    return f"{declaration} = {value};" if value is not None else f"{declaration};"


# A top-level definition of a name: its first line.
def _definition_start(name: str) -> re.Pattern[str]:
    return re.compile(
        rf"^(?!extern\b)[A-Za-z_][^;(=\n]*?(?:\b{name}\s*(?:\[[^\]]*\]\s*)*(?:=|;)"
        rf"|\(\s*\*\s*{name}\s*\))",
        re.MULTILINE,
    )


def _span(text: str, start: int) -> int:
    """Where the definition starting at `start` ends (after its semicolon)."""
    depth = 0
    for i in range(start, len(text)):
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        if text[i] == ";" and depth == 0:
            return i + 1
    return len(text)


def _before_comment(text: str, position: int) -> int:
    """The start of the comment directly above a position, if there's one."""
    lines = text[:position].split("\n")
    i = len(lines) - 1
    while i > 0 and (lines[i - 1].startswith(("/*", " *", "   ")) or lines[i - 1].endswith("*/")):
        i -= 1
    return len("\n".join(lines[:i])) + (1 if i else 0)


def insert(text: str, definition: Definition, existing: list[tuple[int, bool, str]]) -> str:
    """The source with a definition among its others of the same kind by address.
    `existing`: (address, initialised, name) of the globals it defines."""
    same = sorted((a, n) for a, k, n in existing if k == definition.initialised)
    after = [(a, n) for a, n in same if a > definition.glob.address]
    before = [(a, n) for a, n in same if a < definition.glob.address]
    if after:
        found = _definition_start(after[0][1]).search(text)
        if found:
            at = _before_comment(text, found.start())
            return text[:at] + definition.text + "\n" + text[at:]
    if before:
        found = _definition_start(before[-1][1]).search(text)
        if found:
            end = _span(text, found.start())
            return text[:end] + "\n" + definition.text + text[end:]
    marker = re.search(r"^/\*\s*@zoombi32", text, re.MULTILINE)
    at = _before_comment(text, marker.start()) if marker else len(text)
    return (
        text[:at] + definition.text + "\n\n" + text[at:]
        if marker
        else text + "\n" + definition.text + "\n"
    )


@dataclass(frozen=True)
class Plan:
    definitions: list[Definition]
    skipped: dict[str, str]  # name -> why
    declarations: dict[Path, list[Global]]  # new globals for each module header


def plan(exe: Executable) -> Plan:
    types = Types("\n".join(h.read_text() for h in declarations.headers()))
    globals_ = declared(types)
    boundary = match_data.initialised_range(exe)[1]
    sizes = probe_sizes([g.name for g in globals_ if not g.unsized])
    placed = match_data.check(match.decomp_sources(), exe)
    starts = sorted(
        {g.address for g in globals_}
        | {p.address for s in placed.placements for p in s.pieces if p.address is not None}
        | {boundary}
    )
    for g in globals_:  # a global without a length runs to whatever's next
        later = [s for s in starts if s > g.address]
        if g.name not in sizes and later:
            sizes[g.name] = later[0] - g.address
    owned = owners(globals_, references(exe, globals_, sizes), boundary)
    functions = {
        t.address: t.name
        for source in match.decomp_sources()
        for t in match.find_targets(source.read_text())
        if "::" not in t.name and not t.name.startswith("<")
    }
    renderer = Renderer(exe, types, functions, globals_, sizes)
    unnamed = Unnamed(renderer, starts, boundary, owned, [], _data_ends(placed))
    unnamed.follow(list(globals_))
    skipped: dict[str, str] = {}
    new_declarations = _new_declarations(unnamed.found, owned, skipped)
    following = {a.name: b.address for a, b in itertools.pairwise(globals_)}
    defined = _defined_names()
    definitions = []
    for g in globals_:
        if g.name in defined or g.name in skipped:
            continue
        reason = _check(g, sizes, following, owned, initialised=g.address < boundary)
        found = reason or _definition(g, renderer, owned, boundary)
        if isinstance(found, str):
            skipped[g.name] = found
        else:
            definitions.append(found)
    return Plan(definitions, skipped, new_declarations)


def _data_ends(placed: match_data.Checked) -> dict[str, int]:
    """Where each module's placed initialised data ends."""
    return {
        p.source.stem: max(x.address + x.size for x in p.pieces if x.address is not None)
        for p in placed.placements
        if p.segment == "_DATA" and any(x.address is not None for x in p.pieces)
    }


def _new_declarations(
    found: list[Global], owned: dict[str, str | None], skipped: dict[str, str]
) -> dict[Path, list[Global]]:
    """The newly found globals by the module header to declare them in; those
    that can't be declared go in `skipped`, with the reason."""
    new: dict[Path, list[Global]] = {}
    checked = _check_types(found)
    for g in found:
        header = paths.DECOMP_DIR / f"{owned[g.name]}.h"
        if g.name in checked:
            skipped[g.name] = checked[g.name]
        elif not header.exists():
            skipped[g.name] = f"its module, {owned[g.name]}, has no header to declare it in"
        else:
            new.setdefault(header, []).append(g)
    return new


def _definition(
    g: Global, renderer: Renderer, owned: dict[str, str | None], boundary: int
) -> Definition | str:
    """A global's definition, or why it can't be written."""
    owner = owned[g.name]
    assert owner is not None  # _check made sure
    initialised = g.address < boundary
    length = None
    ctype = g.type
    if isinstance(ctype, Array) and ctype.count is None:
        length = renderer.sizes[g.name] // size_of(ctype.element)
        ctype = Array(ctype.element, length)
    value = None
    if initialised:
        assert ctype is not None  # _check made sure
        try:
            value = renderer.value(ctype, g.address, top=True)
        except Unrenderable as e:
            return str(e)
    if value is not None and value.startswith('"') and length is not None:
        length = None  # `char name[] = "text"` sizes itself
    text = _definition_text(g, value, length)
    return Definition(g, paths.DECOMP_DIR / f"{owner}.cpp", initialised, text)


def _check_types(found: list[Global]) -> dict[str, str]:
    """Why the new globals whose element type BCC32 lays out differently from
    our model can't be declared."""
    elements = {g.declaration.rpartition(" ")[0] for g in found}
    if not elements:
        return {}
    theirs = probe_sizes(sorted(elements), "types")
    problems = {}
    for g in found:
        assert isinstance(g.type, Array)
        element = g.declaration.rpartition(" ")[0]
        ours = size_of(g.type.element)
        if ours != theirs[element]:
            problems[g.name] = f"our layout of {element} ({ours} bytes) isn't BCC32's"
    return problems


def declare(new: dict[Path, list[Global]]) -> None:
    """Add declarations to module headers, before their closing #endif."""
    for header, found in new.items():
        text = header.read_text()
        lines = "".join(
            f"extern {g.declaration}; /* pointed to by initialised data */\n"
            for g in sorted(found, key=lambda g: g.address)
        )
        end = text.rfind("\n#endif")
        header.write_text(text[:end] + "\n" + lines + text[end:] if end >= 0 else text + lines)


def _check(
    g: Global,
    sizes: dict[str, int],
    following: dict[str, int],
    owned: dict[str, str | None],
    *,
    initialised: bool,
) -> str | None:
    """Why a global can't be defined automatically, if it can't."""
    try:
        return _problem(g, sizes, following, initialised=initialised) or _owner_problem(
            owned.get(g.name)
        )
    except Unrenderable as e:
        return str(e)


def _problem(
    g: Global, sizes: dict[str, int], following: dict[str, int], *, initialised: bool
) -> str | None:
    """What's wrong with a global's type or size, if anything."""
    if g.type is None and (initialised or g.unsized):
        return g.problem
    ours = None if g.type is None or g.unsized else size_of(g.type)
    size = sizes.get(g.name)
    if ours is not None and ours != size:
        return f"our layout of its type ({ours} bytes) isn't BCC32's ({size})"
    if size is None:
        return "its length is unknown"
    if g.name in following and g.address + size > following[g.name]:
        return f"its type ({size} bytes) runs into the next global, at {following[g.name]:#x}"
    return None


def _owner_problem(owner: str | None) -> str | None:
    if owner is None:
        return "its module is unclear (several use it, or none, and its neighbours disagree)"
    if not (paths.DECOMP_DIR / f"{owner}.cpp").exists():
        return f"its module, {owner}, has no source file"
    return None


def _existing(
    source: Path, by_name: dict[str, Global], boundary: int
) -> list[tuple[int, bool, str]]:
    text = _strip_comments(source.read_text())
    return [
        (g.address, g.address < boundary, g.name)
        for g in by_name.values()
        if _definition_start(g.name).search(text)
    ]


def apply(definitions: list[Definition], boundary: int, globals_: list[Global]) -> list[Path]:
    """Write the definitions into their sources; returns the sources changed."""
    by_source: dict[Path, list[Definition]] = {}
    for d in definitions:
        by_source.setdefault(d.source, []).append(d)
    by_name = {g.name: g for g in globals_}
    for source, group in by_source.items():
        text = source.read_text()
        existing = _existing(source, by_name, boundary)
        for d in sorted(group, key=lambda d: d.glob.address):
            text = insert(text, d, existing)
            existing.append((d.glob.address, d.initialised, d.glob.name))
        source.write_text(text)
    headers = includes.module_headers()
    for source in by_source:
        text = source.read_text()
        source.write_text(includes.with_includes(text, includes.needed(text, source.stem, headers)))
    return sorted(by_source)


def _iter_errors(sources: list[Path]) -> Iterator[tuple[Path, str]]:
    for source, compiled in match.compile_sources(sources).items():
        if isinstance(compiled, str):
            yield source, compiled


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    dry_run: Annotated[
        bool, typer.Option("--dry-run", help="Print the definitions instead of writing them")
    ] = False,
) -> None:
    match.require_toolchain()
    exe = match.game_executable()
    result = plan(exe)
    for name, reason in sorted(result.skipped.items()):
        print(f"  skipped    {name}: {reason}")
    if dry_run:
        for header, found in result.declarations.items():
            for g in found:
                print(f"{header.name}: extern {g.declaration};")
        for d in result.definitions:
            print(f"{d.source.name}: {d.text}")
    else:
        declare(result.declarations)
        types = Types("\n".join(h.read_text() for h in declarations.headers()))
        changed = apply(result.definitions, match_data.initialised_range(exe)[1], declared(types))
        for source, error in _iter_errors(changed):
            print(f"  ERROR      {source.name}: {error}")
    print(
        f"{len(result.definitions)} globals defined, {len(result.skipped)} left to define by hand."
    )
