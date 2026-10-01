"""Mohawk resource archives (`MHWK`/`RSRC`): reading them into a list of
resources, and writing that list back out byte for byte as the game's tools did.

Hand-written: there is no maintained Python library for the format (ScummVM's
`engines/mohawk/` reads it, in C++). The layout, as the engine's resource
manager reads it (see docs/src/codebase/engine-memory-resources.md), all big-endian:

- a 0x1c-byte header: `MHWK`, the size of the rest of the file, `RSRC`,
  version 0x100, a flag set while the file may hold unused space, the file's
  size, the directory's offset, then the directory's size (the file table's
  offset within it) and the file table's size, both 16-bit;
- the resources' data, in file-table order;
- the directory: `u16` offset of the names, `u16` type count, a type table
  `{u32 type, u16 resource table, u16 name table}` sorted by type, then each
  type's resource table (`u16` count, `{u16 id, u16 file-table index}` sorted
  by id) followed by its name table (`u16` count, then `{u16 name, u16 index}`),
  the types' tables in the order the types first occur in the data;
- the file table: `u32` count, then `{u32 offset, 24-bit size, u8 flags,
  u16 0}` per resource.

Every archive of the game also has 8 unused bytes (`00 04 00 ...`) between the
header and the data, and no resource names. `read` rejects archives that
differ from this, so that whatever it accepts `write` reproduces exactly.
"""

from dataclasses import dataclass

_HEADER_SIZE = 0x1C
# Unused bytes after the header in every archive (compacting would drop them).
_PREAMBLE = bytes.fromhex("0004000000000000")
_VERSION = 0x100
_TYPE_ENTRY_SIZE = 8
_FILE_ENTRY_SIZE = 10
# The one file-table flag set on disk: the engine may purge the loaded data.
PURGEABLE = 0x80


class FormatError(ValueError):
    """An archive that isn't a Mohawk archive laid out as the game's are."""


@dataclass(frozen=True)
class Resource:
    type: bytes  # four bytes, e.g. b"tBMP" or b"\0SND"
    id: int
    data: bytes
    purgeable: bool = False


def _u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def _u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def _check_header(data: bytes) -> None:
    if len(data) < _HEADER_SIZE or data[:4] != b"MHWK" or data[8:12] != b"RSRC":
        raise FormatError("not a Mohawk resource archive")
    if _u32(data, 4) != len(data) - 8 or _u32(data, 16) != len(data):
        raise FormatError("sizes in the header don't match the file's")
    if _u16(data, 12) != _VERSION or _u16(data, 14) != 1:
        raise FormatError(f"unexpected version or flag: {data[12:16].hex()}")
    if data[_HEADER_SIZE : _HEADER_SIZE + len(_PREAMBLE)] != _PREAMBLE:
        raise FormatError("unexpected bytes after the header")


def read(data: bytes) -> list[Resource]:
    """The resources of an archive, in the order their data is stored."""
    _check_header(data)
    directory = _u32(data, 20)
    file_table = directory + _u16(data, 24)
    count = _u32(data, file_table)
    if _u16(data, 26) != 4 + count * _FILE_ENTRY_SIZE or file_table + _u16(data, 26) != len(data):
        raise FormatError("file table size doesn't match its count")

    files: list[tuple[int, int, int]] = []
    for i in range(count):
        entry = file_table + 4 + i * _FILE_ENTRY_SIZE
        size = _u16(data, entry + 4) | data[entry + 6] << 16
        files.append((_u32(data, entry), size, data[entry + 7]))

    owners: dict[int, tuple[bytes, int]] = {}
    for t in range(_u16(data, directory + 2)):
        entry = directory + 4 + t * _TYPE_ENTRY_SIZE
        tag = data[entry : entry + 4]
        table = directory + _u16(data, entry + 4)
        if _u16(data, directory + _u16(data, entry + 6)) != 0:
            raise FormatError(f"{tag!r} resources have names")
        for r in range(_u16(data, table)):
            ref = table + 2 + r * 4
            owners[_u16(data, ref + 2)] = (tag, _u16(data, ref))

    resources = []
    for index, (offset, size, flags) in enumerate(files, 1):
        if index not in owners:
            raise FormatError(f"file-table entry {index} isn't in the directory")
        if flags & ~PURGEABLE:
            raise FormatError(f"file-table entry {index} has flags {flags:#x}")
        tag, id_ = owners[index]
        resources.append(Resource(tag, id_, data[offset : offset + size], bool(flags)))

    result = write(resources)
    if result != data:
        raise FormatError("the archive's layout differs from the game's archives'")
    return resources


def write(resources: list[Resource]) -> bytes:
    """An archive holding `resources`, their data stored in the given order."""
    body = bytearray(_PREAMBLE)
    entries = bytearray()
    for r in resources:
        if len(r.type) != 4:
            raise ValueError(f"resource type {r.type!r} isn't four bytes")
        if len(r.data) >= 1 << 24:
            raise ValueError(f"{r.type!r} {r.id} is too big")
        offset = _HEADER_SIZE + len(body)
        size = len(r.data)
        flags = PURGEABLE if r.purgeable else 0
        entries += offset.to_bytes(4, "big") + (size & 0xFFFF).to_bytes(2, "big")
        entries += bytes([size >> 16, flags, 0, 0])
        body += r.data

    # Each type's resources, in order of first occurrence in the data.
    by_type: dict[bytes, list[tuple[int, int]]] = {}
    for index, r in enumerate(resources, 1):
        by_type.setdefault(r.type, []).append((r.id, index))
    tables = bytearray()
    table_offsets: dict[bytes, int] = {}
    types_size = 4 + len(by_type) * _TYPE_ENTRY_SIZE
    for tag, refs in by_type.items():
        refs.sort()
        if len({id_ for id_, _ in refs}) != len(refs):
            raise ValueError(f"{tag!r} has duplicate ids")
        table_offsets[tag] = types_size + len(tables)
        tables += len(refs).to_bytes(2, "big")
        for id_, index in refs:
            tables += id_.to_bytes(2, "big") + index.to_bytes(2, "big")
        tables += bytes(2)  # no names
    names = types_size + len(tables)
    directory = bytearray(names.to_bytes(2, "big") + len(by_type).to_bytes(2, "big"))
    for tag in sorted(by_type):
        resource_table = table_offsets[tag]
        name_table = resource_table + 2 + 4 * len(by_type[tag])
        directory += tag + resource_table.to_bytes(2, "big") + name_table.to_bytes(2, "big")
    directory += tables
    file_table = len(resources).to_bytes(4, "big") + entries

    directory_offset = _HEADER_SIZE + len(body)
    size = directory_offset + len(directory) + len(file_table)
    header = b"MHWK" + (size - 8).to_bytes(4, "big") + b"RSRC"
    header += _VERSION.to_bytes(2, "big") + (1).to_bytes(2, "big")
    header += size.to_bytes(4, "big") + directory_offset.to_bytes(4, "big")
    header += len(directory).to_bytes(2, "big") + len(file_table).to_bytes(2, "big")
    return bytes(header + body + directory + file_table)
