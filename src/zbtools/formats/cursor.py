"""Cursors (`CURS`): Mac cursors, stored as Windows cursor (.cur) files.

A Mac cursor (MacCursor in decomp/zoombinis.h) is a 16x16 1-bit image and mask,
16 big-endian words each, then the hot spot (v, h). A .cur file's XOR and AND
bitmaps say the same exactly: where the mask is set a pixel is black (image
bit 1) or white (0); where it's clear the pixel is transparent (0) or inverts
the screen (1). So AND = ~mask and XOR = image ^ mask.

Hand-written: Pillow reads .cur files but doesn't write them.
"""

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from zbtools.formats.base import Unconvertible, be16, le16, le32
from zbtools.mohawk import Resource

SUFFIX = ".cur"
_SIZE = 16
_MAC_SIZE = 68
_HEADER = 40  # BITMAPINFOHEADER
_ROW = 4  # 16 1-bit pixels, rows padded to 4 bytes


@dataclass(frozen=True)
class Cursor:
    image: tuple[int, ...]  # 16 rows, the leftmost pixel in the top bit
    mask: tuple[int, ...]
    hot_x: int
    hot_y: int


def parse_mac(data: bytes) -> Cursor:
    if len(data) != _MAC_SIZE:
        raise Unconvertible("not a Mac cursor")
    words = tuple(be16(data, 2 * i) for i in range(32))
    return Cursor(words[:16], words[16:], hot_x=be16(data, 66), hot_y=be16(data, 64))


def to_mac(cursor: Cursor) -> bytes:
    words = (*cursor.image, *cursor.mask, cursor.hot_y, cursor.hot_x)
    return b"".join(w.to_bytes(2, "big") for w in words)


def _bitmap(rows: tuple[int, ...]) -> bytes:
    """Rows bottom-up, each padded to 4 bytes."""
    return b"".join(row.to_bytes(2, "big") + bytes(2) for row in reversed(rows))


def to_cur(cursor: Cursor) -> bytes:
    xor = tuple(i ^ m for i, m in zip(cursor.image, cursor.mask, strict=True))
    and_ = tuple(~m & 0xFFFF for m in cursor.mask)
    bitmaps = _bitmap(xor) + _bitmap(and_)
    # size, width, height (both bitmaps), planes, bits, no compression, the
    # bitmaps' size, resolution, colours used and important
    fields = (
        (_HEADER, 4), (_SIZE, 4), (2 * _SIZE, 4), (1, 2), (1, 2), (0, 4),
        (len(bitmaps), 4), (0, 4), (0, 4), (2, 4), (0, 4),
    )  # fmt: skip
    header = b"".join(n.to_bytes(size, "little") for n, size in fields)
    colours = bytes(4) + b"\xff\xff\xff\0"  # black, white
    image = header + colours + bitmaps
    directory = (0).to_bytes(2, "little") + (2).to_bytes(2, "little") + (1).to_bytes(2, "little")
    entry = bytes([_SIZE, _SIZE, 2, 0]) + cursor.hot_x.to_bytes(2, "little")
    entry += cursor.hot_y.to_bytes(2, "little") + len(image).to_bytes(4, "little")
    entry += (len(directory) + 16).to_bytes(4, "little")
    return directory + entry + image


def parse_cur(data: bytes, name: str = "cursor") -> Cursor:
    """A cursor from a .cur file, which must hold one 16x16 1-bit image."""
    if le16(data, 0) != 0 or le16(data, 2) != 2 or le16(data, 4) != 1:
        raise ValueError(f"{name}: must be a cursor with one image")
    hot_x, hot_y, at = le16(data, 10), le16(data, 12), le32(data, 18)
    size, width, height = le32(data, at), le32(data, at + 4), le32(data, at + 8)
    bits = le16(data, at + 14)
    if (width, height, bits) != (_SIZE, 2 * _SIZE, 1) or le32(data, at + 16) != 0:
        raise ValueError(f"{name}: must be 16x16 with 1 bit per pixel")
    colours = data[at + size : at + size + 8]
    xor_start = at + size + 8
    and_start = xor_start + _SIZE * _ROW

    def rows(start: int) -> tuple[int, ...]:
        return tuple(be16(data, start + _ROW * (_SIZE - 1 - y)) for y in range(_SIZE))

    xor, and_ = rows(xor_start), rows(and_start)
    # Pixel value 1 is white, as written; an editor may swap the colours (XOR
    # with white inverts the screen, so this flips transparent and inverted
    # pixels too).
    if sum(colours[0:3]) > sum(colours[4:7]):
        xor = tuple(~r & 0xFFFF for r in xor)
    mask = tuple(~a & 0xFFFF for a in and_)
    image = tuple(x ^ m for x, m in zip(xor, mask, strict=True))
    return Cursor(image, mask, hot_x, hot_y)


@dataclass(frozen=True)
class CursorFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        cur = to_cur(parse_mac(data))
        if to_mac(parse_cur(cur)) != data:
            raise Unconvertible("doesn't round-trip")
        stem.with_suffix(SUFFIX).write_bytes(cur)

    def load(self, stem: Path) -> bytes:
        path = stem.with_suffix(SUFFIX)
        return to_mac(parse_cur(path.read_bytes(), str(path)))
