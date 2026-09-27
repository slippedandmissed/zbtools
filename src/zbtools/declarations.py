"""What decomp/zoombinis.h declares, for tools that carry it elsewhere (`uv run
ghidra label` gives Ghidra our struct types and global names and types).

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

from zbtools import paths

HEADER = paths.DECOMP_DIR / "zoombinis.h"

_EXTERN = re.compile(r"^extern\s+([^;]*?)\s*\b(\w+)\s*(\[[^\]]*\])?\s*;(.*)$")
# `extern void (*g_4aa4c4)(Point *where);`: a function pointer.
_EXTERN_FUNCTION_POINTER = re.compile(r"^extern\s+[^;(]*\(\s*\*\s*(\w+)\s*\)\s*\([^;]*\)\s*;(.*)$")
_ADDRESS_NAME = re.compile(r"^g_([0-9a-fA-F]{6,8})$")
_DATA_MARKER = re.compile(r"/\*\s*@data\s+(0x[0-9a-fA-F]+)\s*\*/")
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
            type_, name, array, rest = "void *", pointer.group(1), None, pointer.group(2)
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
        found.append(Global(address, name, aliases.get(type_, type_), array is not None))
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


def load() -> tuple[list[Global], str]:
    text = HEADER.read_text()
    return globals_in(text), structs_as_c(text)
