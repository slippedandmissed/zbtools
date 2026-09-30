"""What decomp/'s headers declare, for tools that carry it elsewhere (`uv run
ghidra label` gives Ghidra our struct types and global names and types):
zoombinis.h (the shared types and declarations) and each module's own
header (`decomp/<module>.h`: its functions and the globals only it uses).

Globals are found from their `extern` declarations; a global's address is in
its name (`g_4a7f58`) until it's renamed, and then in a marker comment on the
same line or the line before: `extern long mousePresent; /* @data 0x4a7f58 */`.
Function pointer globals (`extern void (*g_4aa4c4)(Point *where);`) are
passed on as `void *`. Structs are passed on as C, with the function pointer
typedefs they may use: Ghidra's C parser doesn't know C++'s implicit `struct`
type names, so each struct also gets a typedef. C++ classes (with virtual
functions) aren't passed on, only declared (as empty structs), so that
structs can point to them. Other typedefs (`typedef Chunk **Block;`) are
passed on, and anonymous unions, which Ghidra's C parser rejects, are named
(`union { ... } u1;`).

A regular-expression reader is enough here because the header is ours and
written plainly; a C++ parser would be a heavy dependency for it.
"""

import re
from dataclasses import dataclass
from pathlib import Path

from zbtools import paths

HEADER = paths.DECOMP_DIR / "zoombinis.h"


def headers() -> list[Path]:
    """The shared header first, then the modules' headers."""
    return [HEADER, *sorted(p for p in paths.DECOMP_DIR.glob("*.h") if p != HEADER)]


_EXTERN = re.compile(r"^extern\s+([^;]*?)\s*\b(\w+)\s*((?:\[[^\]]*\]\s*)*);(.*)$")
# `extern void (*g_4aa4c4)(Point *where);`: a function pointer.
_EXTERN_FUNCTION_POINTER = re.compile(r"^extern\s+[^;(]*\(\s*\*\s*(\w+)\s*\)\s*\([^;]*\)\s*;(.*)$")
_ADDRESS_NAME = re.compile(r"^g_([0-9a-fA-F]{6,8})$")
# `/* @data 0x4a7f58 */`, or with a note: `/* @data 0x4a7f58: the mouse is present */`.
_DATA_MARKER = re.compile(r"/\*\s*@data\s+(0x[0-9a-fA-F]+)\s*(?::.*?)?\*/")
_STRUCT = re.compile(r"^struct\s+(\w+)\s*\n\{.*?\n\};", re.MULTILINE | re.DOTALL)
# `typedef Chunk **Block;`: a typedef that isn't a function pointer.
_TYPEDEF = re.compile(r"^typedef\s+[^;(]*;", re.MULTILINE)
# `class basePort;` or `class Palette\n{`: a C++ class, declared or defined.
_CLASS = re.compile(r"^class\s+(\w+)\b", re.MULTILINE)
# `typedef void (*Callback)();`: a function pointer type, passed on as a pointer.
_FUNCTION_POINTER = re.compile(r"^typedef\s[^;(]*\(\s*\*\s*(\w+)\s*\)[^;]*;", re.MULTILINE)
_CONST = re.compile(r"\bconst\s+")
# Borland types that aren't C builtins, as the builtins they are.
_ALIASES = {"time_t": "long"}


@dataclass(frozen=True)
class Global:
    address: int
    name: str
    type: str  # C type, e.g. "long", "Counted *"
    array: bool  # declared with [] (its length isn't known)


def globals_in(text: str) -> list[Global]:
    pointers: list[str] = _FUNCTION_POINTER.findall(text)
    aliases: dict[str, str] = {**_ALIASES, **dict.fromkeys(pointers, "void *")}
    found = []
    lines = text.splitlines()
    for i, line in enumerate(lines):
        pointer = _EXTERN_FUNCTION_POINTER.match(line.strip())
        declaration = _EXTERN.match(line.strip())
        if pointer:
            type_, name, array, rest = "void *", pointer.group(1), "", pointer.group(2)
        elif declaration:
            type_, name, array, rest = declaration.groups()
        else:
            continue
        named = _ADDRESS_NAME.match(name)
        above = lines[i - 1].strip() if i else ""
        marker = _DATA_MARKER.search(rest) or (
            _DATA_MARKER.fullmatch(above) if above.startswith("/*") else None
        )
        if marker:
            address = int(marker.group(1), 16)
        elif named:
            address = int(named.group(1), 16)
        else:
            continue
        type_ = _CONST.sub("", type_)  # Ghidra's types have no const
        found.append(Global(address, name, aliases.get(type_, type_), bool(array)))
    return found


def unaddressed_globals(text: str) -> list[str]:
    """The globals declared with neither an address name nor an @data marker."""
    addressed = {g.name for g in globals_in(text)}
    return [name for name in extern_names_in(text) if name not in addressed]


def extern_names_in(text: str) -> list[str]:
    """The names of the globals declared (addressed or not)."""
    found = []
    for line in text.splitlines():
        match = _EXTERN_FUNCTION_POINTER.match(line.strip()) or _EXTERN.match(line.strip())
        if match:
            found.append(match.group(1) if match.re is _EXTERN_FUNCTION_POINTER else match.group(2))
    return found


def structs_as_c(text: str) -> str:
    """The header's structs as C, with byte packing like Borland's default."""
    structs = [m.group(0) for m in _STRUCT.finditer(text)]
    names = [m.group(1) for m in _STRUCT.finditer(text)]
    classes = dict.fromkeys(name for name in _CLASS.findall(text) if name not in names)
    typedefs = [f"typedef struct {name} {name};" for name in [*names, *classes]]
    pointers = [m.group(0) for m in _FUNCTION_POINTER.finditer(text)]
    plain = [m.group(0) for m in _TYPEDEF.finditer(text)]
    structs = [_name_anonymous_unions(struct) for struct in structs]
    parts = ["#pragma pack(push, 1)", *typedefs, *pointers, *plain, *structs, "#pragma pack(pop)"]
    return "\n".join(parts) + "\n"


def _name_anonymous_unions(struct: str) -> str:
    """Name each anonymous union in a struct u1, u2, ...: its closing `};` is
    the first one after `union {` that's indented like it."""
    lines = struct.splitlines()
    count = 0
    for i, line in enumerate(lines):
        if line.strip() != "union {":
            continue
        indent = line[: len(line) - len(line.lstrip())]
        for j in range(i + 1, len(lines)):
            if lines[j] == f"{indent}}};":
                count += 1
                lines[j] = f"{indent}}} u{count};"
                break
    return "\n".join(lines)


# A function's prototype at the top level: `short foo(short a);`, possibly
# over several lines, with an optional address comment (`/* 0x46daca */`).
_PROTOTYPE = re.compile(
    r"^(?!typedef|struct|class|union|enum|inline|static|extern|return|#|/)"
    r"[A-Za-z_][\w\s*&:,]*?\b(\w+)\s*\([^;{]*\)\s*;(.*)$",
    re.MULTILINE,
)
_ADDRESS_COMMENT = re.compile(r"/\*\s*(0x[0-9a-fA-F]+)\s*\*/")


@dataclass(frozen=True)
class Prototype:
    name: str
    address: int | None  # from an address comment, if it has one


def prototypes_in(text: str) -> list[Prototype]:
    """The functions a header declares outside any braces (not class members)."""
    found = []
    depth = 0
    position = 0
    for match in _PROTOTYPE.finditer(text):
        depth += text.count("{", position, match.start()) - text.count("}", position, match.start())
        position = match.start()
        if depth:
            continue
        comment = _ADDRESS_COMMENT.search(match.group(2))
        found.append(Prototype(match.group(1), int(comment.group(1), 16) if comment else None))
    return found


def declared_twice(texts: list[str]) -> list[str]:
    """Functions declared more than once across headers. Two functions may
    share a name (overloads) only if each declaration has its own address
    comment; otherwise one of them is a stale or conflicting copy."""
    by_name: dict[str, list[int | None]] = {}
    for text in texts:
        for prototype in prototypes_in(text):
            by_name.setdefault(prototype.name, []).append(prototype.address)
    return sorted(
        name
        for name, addresses in by_name.items()
        if len(addresses) > 1 and (None in addresses or len(set(addresses)) < len(addresses))
    )


def globals_declared_twice(texts: list[str]) -> list[str]:
    """Globals declared more than once across headers."""
    counts: dict[str, int] = {}
    for text in texts:
        for name in extern_names_in(text):
            counts[name] = counts.get(name, 0) + 1
    return sorted(name for name, count in counts.items() if count > 1)


def load() -> tuple[list[Global], str]:
    text = "\n".join(path.read_text() for path in headers())
    return globals_in(text), structs_as_c(text)
