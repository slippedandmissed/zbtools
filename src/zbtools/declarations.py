"""What decomp/zoombinis.h declares, for tools that carry it elsewhere (`uv run
ghidra label` gives Ghidra our struct types and global names and types).

Globals are found from their `extern` declarations; a global's address is in
its name (`g_4a7f58`) until it's renamed, and then in a marker comment on the
same line or the line before: `extern long mousePresent; /* @data 0x4a7f58 */`.
Structs are passed on as C: Ghidra's C parser doesn't know C++'s implicit
`struct` type names, so each struct also gets a typedef. C++ classes (with
virtual functions) aren't passed on.

A regular-expression reader is enough here because the header is ours and
written plainly; a C++ parser would be a heavy dependency for it.
"""

import re
from dataclasses import dataclass

from zbtools import paths

HEADER = paths.DECOMP_DIR / "zoombinis.h"

_EXTERN = re.compile(r"^extern\s+([^;]*?)\s*\b(\w+)\s*(\[[^\]]*\])?\s*;(.*)$")
_ADDRESS_NAME = re.compile(r"^g_([0-9a-fA-F]{6,8})$")
_DATA_MARKER = re.compile(r"/\*\s*@data\s+(0x[0-9a-fA-F]+)\s*\*/")
_STRUCT = re.compile(r"^struct\s+(\w+)\s*\n\{.*?\n\};", re.MULTILINE | re.DOTALL)
# Borland types that aren't C builtins, as the builtins they are.
_ALIASES = {"time_t": "long"}


@dataclass(frozen=True)
class Global:
    address: int
    name: str
    type: str  # C type, e.g. "long", "Counted *"
    array: bool  # declared with [] (its length isn't known)


def globals_in(text: str) -> list[Global]:
    found = []
    lines = text.splitlines()
    for i, line in enumerate(lines):
        declaration = _EXTERN.match(line.strip())
        if not declaration:
            continue
        type_, name, array, rest = declaration.groups()
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
        found.append(Global(address, name, _ALIASES.get(type_, type_), array is not None))
    return found


def structs_as_c(text: str) -> str:
    """The header's structs as C, with byte packing like Borland's default."""
    structs = [m.group(0) for m in _STRUCT.finditer(text)]
    names = [m.group(1) for m in _STRUCT.finditer(text)]
    typedefs = [f"typedef struct {name} {name};" for name in names]
    return "\n".join(["#pragma pack(push, 1)", *typedefs, *structs, "#pragma pack(pop)"]) + "\n"


def load() -> tuple[list[Global], str]:
    text = HEADER.read_text()
    return globals_in(text), structs_as_c(text)
