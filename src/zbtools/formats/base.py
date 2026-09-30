"""What a resource format is: how one type of resource is stored in assets/."""

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

from zbtools.mohawk import Resource


class Unconvertible(ValueError):
    """A resource a format can't represent exactly; it's kept raw instead."""


class Format(Protocol):
    """Converts a type of resource to files named after `stem` (<type>/<id>) and
    back. `load` must give back exactly the bytes `save` was given; `save`
    raises Unconvertible for data it couldn't give back. `archive` is all the
    archive's resources, for what only helps to view a resource (an image's
    palette), never for what `load` needs."""

    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None: ...

    def load(self, stem: Path) -> bytes: ...


RAW_SUFFIX = ".bin"


@dataclass(frozen=True)
class Raw:
    """The resource's data as it is, in <stem>.bin."""

    def save(self, data: bytes, stem: Path, archive: Sequence[Resource] = ()) -> None:
        stem.with_suffix(RAW_SUFFIX).write_bytes(data)

    def load(self, stem: Path) -> bytes:
        return stem.with_suffix(RAW_SUFFIX).read_bytes()


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def le16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little")


def le32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little")


def chunks(
    data: bytes, start: int, big_endian: bool, padded: bool = True
) -> list[tuple[bytes, bytes]]:
    """The chunks of an IFF-style file from `start`: {tag, u32 size, data},
    each padded to an even length unless not `padded` (as in MIDI files)."""
    found = []
    at = start
    while at < len(data):
        if at + 8 > len(data):
            raise Unconvertible("truncated chunk header")
        size = be32(data, at + 4) if big_endian else le32(data, at + 4)
        if at + 8 + size > len(data):
            raise Unconvertible(f"chunk {data[at : at + 4]!r} runs past the end")
        found.append((data[at : at + 4], data[at + 8 : at + 8 + size]))
        pad = size & 1 if padded else 0
        if pad and at + 8 + size < len(data) and data[at + 8 + size] != 0:
            raise Unconvertible("non-zero padding")
        at += 8 + size + pad
    if at != len(data):
        raise Unconvertible("padding missing at the end")
    return found


def chunk(tag: bytes, body: bytes, big_endian: bool, padded: bool = True) -> bytes:
    """A chunk, padded to an even length unless not `padded`."""
    size = len(body).to_bytes(4, "big" if big_endian else "little")
    return tag + size + body + bytes(len(body) & 1 if padded else 0)
