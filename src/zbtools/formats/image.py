"""Images (`tBMP`): stored as indexed PNGs, with a TOML file of what the PNGs
can't say.

An image resource (ImageHeader in decomp/zoombinis.h) is a big-endian header,
u16 width, height and bytes per row (the width rounded up to 4) and u16 flags
(the game's are 8-bit, 2; 0x10 packed; 0x100 compressed), then its pixels:

- unpacked, rows of `rowBytes` bytes, top row first;
- packed (drawPackedPixels, decomp/realizepalette.cpp), each row a u16 size
  and packets: a header byte, bit 7 set for a run of the next byte, else that
  many literal bytes, the count being the low bits plus 1;
- compressed (decompressImage, decomp/decompressimage.cpp), the pixels after
  a u32 size, the compressed size and a u16 ring size (0x400) are LZSS
  (lzss.py).

A bank of images, as loadImageBank reads one (decomp/view.cpp), has the same
header, as if the bank were one image: its width the number of images, its
bytes per row that rounded up to 4, its height the bank's size divided by
that (truncated to 16 bits). Then come the offsets of its images (the first after the offsets), each
a header and pixels, 4-byte aligned; the whole bank may be compressed.

Broderbund's packer, reproduced here, runs colour 0 (transparent) always,
other colours from four pixels, splits runs at 128, and ends each row with a
run, however short. After a packed image's rows it left 2 bytes, plus padding
to a multiple of 4 in a bank; those bytes, and unpacked rows' padding, are
often junk from memory, kept in the TOML file where they aren't zero (and
used while they still fit: editing an image can change where its rows end).

A single image is <id>.png; a bank's images are <id>/<n>.png (from 1). The
PNGs' pixel values are what the game draws; their palette is the archive's
(from its first shape list and palette, or the one listing the image), for
viewing only: a palette's colours are edited in its own resource.
"""

import hashlib
import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from PIL import Image
from pydantic import BaseModel, ConfigDict

from zbtools.formats import lzss
from zbtools.formats.base import Unconvertible, be16, be32
from zbtools.mohawk import Resource

SUFFIX = ".png"
INFO_SUFFIX = ".toml"
_BITS = 10
_DEPTH_8 = 0x2
_PACKED = 0x10
_COMPRESSED = 0x100
_MAX_PACKET = 128
_MIN_RUN = 4

# The Windows palette's static colours (0-9 and 246-255), for viewing.
# fmt: off
_STATIC = {
    0: "000000", 1: "800000", 2: "008000", 3: "808000", 4: "000080", 5: "800080",
    6: "008080", 7: "c0c0c0", 8: "c0dcc0", 9: "a6caf0", 246: "fffbf0", 247: "a0a0a4",
    248: "808080", 249: "ff0000", 250: "00ff00", 251: "ffff00", 252: "0000ff",
    253: "ff00ff", 254: "00ffff", 255: "ffffff",
}
# fmt: on


class Padding(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    row: int
    bytes: str  # hex


class ImageInfo(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    packed: bool
    tail: str = ""  # hex: the bytes after a packed image's rows, if not zero
    padding: list[Padding] = []  # unpacked rows' padding, where not zero


class ImageFile(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    compressed: bool
    bank: bool
    images: list[ImageInfo]


@dataclass(frozen=True)
class Picture:
    width: int
    height: int
    pixels: bytes  # width * height, top row first


def _round4(n: int) -> int:
    return (n + 3) & ~3


def _header(width: int, height: int, row_bytes: int, flags: int) -> bytes:
    return b"".join(v.to_bytes(2, "big") for v in (width, height, row_bytes, flags))


def unpack_row(row: bytes, width: int) -> bytes:
    out = bytearray()
    at = 0
    while at < len(row):
        header = row[at]
        count = (header & 0x7F) + 1
        if header & 0x80:
            out += row[at + 1 : at + 2] * count
            at += 2
        else:
            out += row[at + 1 : at + 1 + count]
            at += 1 + count
    if len(out) != width or at != len(row):
        raise Unconvertible("a packed row doesn't match the width")
    return bytes(out)


def pack_row(pixels: bytes) -> bytes:
    out = bytearray()
    literal = bytearray()

    def flush() -> None:
        for i in range(0, len(literal), _MAX_PACKET):
            part = literal[i : i + _MAX_PACKET]
            out.append(len(part) - 1)
            out.extend(part)
        literal.clear()

    at = 0
    while at < len(pixels):
        end = at
        while end < len(pixels) and pixels[end] == pixels[at]:
            end += 1
        value, count = pixels[at], end - at
        if count > _MAX_PACKET:
            flush()
            out += bytes([0x80 | (_MAX_PACKET - 1), value])
            at += _MAX_PACKET
            continue
        if value == 0 or count >= _MIN_RUN or end == len(pixels):
            flush()
            out += bytes([0x80 | (count - 1), value])
        else:
            literal += pixels[at:end]
        at = end
    flush()
    return bytes(out)


def _parse_picture(data: bytes, at: int, end: int, bank: bool) -> tuple[Picture, ImageInfo]:
    """The image whose header is at `at`, its data ending at `end`."""
    width, height, row_bytes, flags = (be16(data, at + 2 * i) for i in range(4))
    if flags not in (_DEPTH_8, _DEPTH_8 | _PACKED) or row_bytes != _round4(width):
        raise Unconvertible(f"image flags {flags:#x}, {width} wide, {row_bytes} bytes a row")
    at += 8
    if not flags & _PACKED:
        if end - at != row_bytes * height:
            raise Unconvertible("pixels don't fill the rows")
        rows = [data[at + y * row_bytes : at + (y + 1) * row_bytes] for y in range(height)]
        padding = [
            Padding(row=y, bytes=r[width:].hex()) for y, r in enumerate(rows) if any(r[width:])
        ]
        pixels = b"".join(r[:width] for r in rows)
        return Picture(width, height, pixels), ImageInfo(packed=False, padding=padding)
    unpacked = []
    for _ in range(height):
        size = be16(data, at)
        unpacked.append(unpack_row(data[at + 2 : at + 2 + size], width))
        at += 2 + size
    tail = data[at:end]
    if len(tail) != _tail_size(at, bank):
        raise Unconvertible("unexpected bytes after the rows")
    info = ImageInfo(packed=True, tail=tail.hex() if any(tail) else "")
    return Picture(width, height, b"".join(unpacked)), info


def _tail_size(rows_end: int, bank: bool) -> int:
    """The bytes Broderbund's packer left after an image's rows."""
    return 2 + (-(rows_end + 2) % 4 if bank else 0)


def _picture_bytes(picture: Picture, info: ImageInfo, bank: bool, at: int) -> bytes:
    """An image, with its header, to be placed at offset `at`."""
    flags = _DEPTH_8 | (_PACKED if info.packed else 0)
    row_bytes = _round4(picture.width)
    out = bytearray(_header(picture.width, picture.height, row_bytes, flags))
    rows = [
        picture.pixels[y * picture.width : (y + 1) * picture.width] for y in range(picture.height)
    ]
    if info.packed:
        for row in rows:
            packed = pack_row(row)
            out += len(packed).to_bytes(2, "big") + packed
        # The recorded junk where it still fits (an edited image's may not).
        size = _tail_size(at + len(out), bank)
        tail = bytes.fromhex(info.tail)
        out += tail if len(tail) == size else bytes(size)
    else:
        size = row_bytes - picture.width
        padding = {p.row: bytes.fromhex(p.bytes) for p in info.padding}
        for y, row in enumerate(rows):
            pad = padding.get(y, b"")
            out += row + (pad if len(pad) == size else bytes(size))
    return bytes(out)


def _parse_bank(full: bytes) -> tuple[list[Picture], list[ImageInfo]]:
    count = be16(full, 0)
    offsets = [be32(full, 8 + 4 * i) for i in range(count)] + [len(full)]
    if count == 0 or offsets[0] != 8 + 4 * count:
        raise Unconvertible("not a bank")
    pictures, infos = [], []
    for i in range(count):
        if offsets[i] % 4 or offsets[i + 1] <= offsets[i]:
            raise Unconvertible("not a bank")
        picture, info = _parse_picture(full, offsets[i], offsets[i + 1], bank=True)
        pictures.append(picture)
        infos.append(info)
    return pictures, infos


def _bank_bytes(pictures: list[Picture], infos: list[ImageInfo], flags: int) -> bytes:
    count = len(pictures)
    body = bytearray()
    offsets = []
    start = 8 + 4 * count
    for picture, info in zip(pictures, infos, strict=True):
        offsets.append(start + len(body))
        body += _picture_bytes(picture, info, bank=True, at=start + len(body))
    size = start + len(body)
    # The height, as if the bank were an image, truncated to 16 bits as the
    # original tool did (MAZE2.MHK's 2 MB tBMP 11000).
    head = _header(count, size // _round4(count) & 0xFFFF, _round4(count), flags)
    return head + b"".join(o.to_bytes(4, "big") for o in offsets) + body


def parse(data: bytes) -> tuple[ImageFile, list[Picture]]:
    flags = be16(data, 6)
    compressed = bool(flags & _COMPRESSED)
    if compressed:
        size, compressed_size, window = be32(data, 8), be32(data, 12), be16(data, 16)
        if window != 1 << _BITS or compressed_size != len(data) - 18:
            raise Unconvertible("unexpected compression parameters")
        full = data[:8] + lzss.decompress(data[18:], size, _BITS)
    else:
        full = data
    if not flags & _PACKED:
        try:
            pictures, infos = _parse_bank(full)
        except (Unconvertible, IndexError):
            pass
        else:
            return ImageFile(compressed=compressed, bank=True, images=infos), pictures
    head = full[:6] + (flags & ~_COMPRESSED).to_bytes(2, "big") + full[8:]
    picture, info = _parse_picture(head, 0, len(head), bank=False)
    return ImageFile(compressed=compressed, bank=False, images=[info]), [picture]


def build(image: ImageFile, pictures: list[Picture], cache: Path | None = None) -> bytes:
    if image.bank:
        full = _bank_bytes(pictures, image.images, _DEPTH_8)
    elif len(pictures) == 1 == len(image.images):
        full = _picture_bytes(pictures[0], image.images[0], bank=False, at=0)
    else:
        raise ValueError("a single image must have one picture")
    if not image.compressed:
        return full
    payload = full[8:]
    compressed = _compress(payload, cache)
    flags = be16(full, 6) | _COMPRESSED
    head = full[:6] + flags.to_bytes(2, "big")
    head += len(payload).to_bytes(4, "big") + len(compressed).to_bytes(4, "big")
    return head + (1 << _BITS).to_bytes(2, "big") + compressed


def _compress(payload: bytes, cache: Path | None) -> bytes:
    """LZSS-compressed data, kept in `cache` (the encoder is slow in Python):
    only ever the encoder's output, so that `verify` still tests it."""
    if cache is None:
        return lzss.compress(payload, _BITS)
    key = hashlib.sha256(b"lzss-okumura 1 %d " % _BITS + payload).hexdigest()
    path = cache / key[:2] / key
    if path.exists():
        return path.read_bytes()
    compressed = lzss.compress(payload, _BITS)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(compressed)
    return compressed


def display_palette(archive: Sequence[Resource], id_: int) -> list[int]:
    """The palette to view image `id_` with: the static colours, the archive's
    first shape list's colours, then those of the list including the image
    and its palette. Colours none of them set are grey ramps."""
    colours = [bytes([i, i, i]) for i in range(256)]
    for i, hex_ in _STATIC.items():
        colours[i] = bytes.fromhex(hex_)

    def apply(palette: bytes) -> None:
        first, count = be16(palette, 0), be16(palette, 2)
        for i in range(min(count, 256 - first)):
            colours[first + i] = palette[4 + 4 * i : 7 + 4 * i]

    lists = sorted((r for r in archive if r.type == b"SHPL"), key=lambda r: r.id)
    palettes = {r.id: r.data for r in archive if r.type == b"tPAL"}
    chosen = [lists[0]] if lists else []
    chosen += [r for r in lists if be16(r.data, 0) <= id_ < be16(r.data, 0) + be16(r.data, 2)]
    for r in chosen:
        apply(r.data[4:])
        if r.id in palettes:
            apply(palettes[r.id])
    return [c for colour in colours for c in colour]


def _write_png(picture: Picture, path: Path, palette: list[int], transparent: bool) -> None:
    image = Image.frombytes("P", (picture.width, picture.height), picture.pixels)
    image.putpalette(palette)
    if transparent:
        image.save(path, transparency=0)
    else:
        image.save(path)


def _read_png(path: Path) -> Picture:
    with Image.open(path) as image:
        if image.mode not in ("P", "L"):
            raise ValueError(f"{path}: must be an indexed (8-bit) image, not {image.mode}")
        return Picture(image.width, image.height, image.tobytes())


def _toml(image: ImageFile, id_: int) -> str:
    what = (
        f"A bank of {len(image.images)} images, in {id_}/<n>.png"
        if image.bank
        else f"One image, {id_}.png"
    )
    lines = [f"# {what}.", f"compressed = {str(image.compressed).lower()}"]
    lines.append(f"bank = {str(image.bank).lower()}")
    lines.append("images = [")
    for info in image.images:
        fields = [f"packed = {str(info.packed).lower()}"]
        if info.tail:
            fields.append(f'tail = "{info.tail}"')
        if info.padding:
            pads = ", ".join(f'{{ row = {p.row}, bytes = "{p.bytes}" }}' for p in info.padding)
            fields.append(f"padding = [{pads}]")
        lines.append(f"    {{ {', '.join(fields)} }},")
    lines.append("]")
    return "\n".join(lines) + "\n"


@dataclass(frozen=True)
class ImageFormat:
    cache: Path | None = None  # for compressed data (see _compress)

    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        try:
            image, pictures = parse(data)
        except IndexError as e:
            raise Unconvertible("truncated") from e
        if build(image, pictures, self.cache) != data:
            raise Unconvertible("doesn't round-trip")
        id_ = int(stem.name)
        palette = display_palette(archive, id_)
        if image.bank:
            stem.mkdir(exist_ok=True)
            for n, picture in enumerate(pictures, 1):
                _write_png(picture, stem / f"{n}{SUFFIX}", palette, transparent=True)
        else:
            _write_png(pictures[0], stem.with_suffix(SUFFIX), palette, transparent=False)
        stem.with_suffix(INFO_SUFFIX).write_text(_toml(image, id_))

    def load(self, stem: Path) -> bytes:
        image = ImageFile.model_validate(tomllib.loads(stem.with_suffix(INFO_SUFFIX).read_text()))
        if image.bank:
            pictures = [_read_png(stem / f"{n}{SUFFIX}") for n in range(1, len(image.images) + 1)]
        else:
            pictures = [_read_png(stem.with_suffix(SUFFIX))]
        return build(image, pictures, self.cache)
