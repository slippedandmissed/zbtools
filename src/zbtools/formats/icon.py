"""Windows icons (the executable's `ICON` resources), stored as indexed PNGs.

An icon image is a device-independent bitmap: a BITMAPINFOHEADER (its
height doubled, for the two bitmaps), a palette of `RGBQUAD`s, the XOR
bitmap (the pixels' palette indices, rows bottom-up, padded to 4 bytes) and
the AND bitmap (1 bit per pixel, set where the screen shows through). The
game's two, 32x32 and 16x16, have 4 bits per pixel and the standard 16-colour
palette.

As a PNG, an icon is an indexed image whose first 16 palette entries are the
icon's colours, plus a 17th, fully transparent, for the pixels the mask lets
the screen through. A masked pixel must be colour 0 (the icon would otherwise
invert the screen there, which a PNG can't show); the game's are. An edited
PNG may use any 16 colours: indexed, its first 16 entries are the palette
(padded with black); otherwise its colours, in the order they first appear.

The icon group (`GROUP_ICON`) that lists the images is derived from them;
`group` builds it, and `ico_file` the .ico file BRCC32 compiles an `ICON`
statement from.

Hand-written: Pillow reads and writes .ico files, but converts their images
to its own modes and writes PNG-compressed or 32-bit images, not these bytes.
"""

from dataclasses import dataclass
from pathlib import Path

from PIL import Image

from zbtools.formats.base import Unconvertible, le16, le32

SUFFIX = ".png"
_HEADER = 40  # BITMAPINFOHEADER
_COLOURS = 16
_BITS = 4
_TRANSPARENT = _COLOURS  # the PNG's palette entry for masked pixels
_TRANSPARENT_RGB = (255, 0, 255)  # its colour, for editors that show it


@dataclass(frozen=True)
class Icon:
    width: int
    height: int
    palette: tuple[tuple[int, int, int], ...]  # 16 (red, green, blue)
    pixels: tuple[int, ...]  # palette indices, top row first
    mask: tuple[bool, ...]  # True where the screen shows through


def _row_bytes(width: int, bits: int) -> int:
    return (width * bits + 31) // 32 * 4


def parse(data: bytes) -> Icon:
    """An icon image from its resource data, which must be one this format
    gives back exactly."""
    if len(data) < _HEADER or le32(data, 0) != _HEADER:
        raise Unconvertible("not a BITMAPINFOHEADER")
    width, doubled = le32(data, 4), le32(data, 8)
    height = doubled // 2
    if le16(data, 12) != 1 or le16(data, 14) != _BITS or le32(data, 16) != 0:
        raise Unconvertible("not an uncompressed 4-bit icon")
    xor_row, and_row = _row_bytes(width, _BITS), _row_bytes(width, 1)
    at = _HEADER + 4 * _COLOURS
    xor, and_ = data[at : at + xor_row * height], data[at + xor_row * height :]
    if len(and_) != and_row * height:
        raise Unconvertible("the bitmaps' size doesn't match the image's")
    palette = tuple(
        (data[_HEADER + 4 * i + 2], data[_HEADER + 4 * i + 1], data[_HEADER + 4 * i])
        for i in range(_COLOURS)
    )
    pixels, mask = [], []
    for y in range(height):
        row = height - 1 - y  # bottom-up
        for x in range(width):
            byte = xor[row * xor_row + x // 2]
            pixels.append(byte >> 4 if x % 2 == 0 else byte & 0xF)
            mask.append(bool(and_[row * and_row + x // 8] >> (7 - x % 8) & 1))
    icon = Icon(width, height, palette, tuple(pixels), tuple(mask))
    if to_bytes(icon) != data:
        raise Unconvertible("doesn't round-trip (masked colours, or header fields)")
    return icon


def to_bytes(icon: Icon) -> bytes:
    """An icon image's resource data."""
    xor_row, and_row = _row_bytes(icon.width, _BITS), _row_bytes(icon.width, 1)
    xor, and_ = bytearray(), bytearray()
    for row in range(icon.height - 1, -1, -1):
        line = icon.pixels[row * icon.width : (row + 1) * icon.width]
        masked = icon.mask[row * icon.width : (row + 1) * icon.width]
        packed = bytes(
            (line[x] << 4) | (line[x + 1] if x + 1 < icon.width else 0)
            for x in range(0, icon.width, 2)
        )
        xor += packed.ljust(xor_row, b"\0")
        bits = bytes(
            sum(1 << (7 - k) for k in range(8) if x + k < icon.width and masked[x + k])
            for x in range(0, icon.width, 8)
        )
        and_ += bits.ljust(and_row, b"\0")
    # size, width, height (both bitmaps), planes, bits, no compression, the
    # bitmaps' size, resolution, colours used and important (0: all)
    fields = (
        (_HEADER, 4), (icon.width, 4), (2 * icon.height, 4), (1, 2), (_BITS, 2), (0, 4),
        (len(xor) + len(and_), 4), (0, 4), (0, 4), (0, 4), (0, 4),
    )  # fmt: skip
    header = b"".join(n.to_bytes(size, "little") for n, size in fields)
    palette = b"".join(bytes((b, g, r, 0)) for r, g, b in icon.palette)
    return header + palette + bytes(xor) + bytes(and_)


def save_png(icon: Icon, path: Path) -> None:
    if any(m and p for m, p in zip(icon.mask, icon.pixels, strict=True)):
        raise Unconvertible("a masked pixel has a colour (it would invert the screen)")
    image = Image.new("P", (icon.width, icon.height))
    image.putpalette([c for rgb in (*icon.palette, _TRANSPARENT_RGB) for c in rgb])
    image.putdata([_TRANSPARENT if m else p for m, p in zip(icon.mask, icon.pixels, strict=True)])
    image.save(path, transparency=_TRANSPARENT)


def load_png(path: Path) -> Icon:
    """An icon from a PNG: indexed, as `save_png` writes it, or edited (see the
    module's description)."""
    with Image.open(path) as opened:
        image = opened.copy()
    width, height = image.size
    if image.mode == "P":
        return _from_indexed(image, path)
    rgba = image.convert("RGBA")
    colours: list[tuple[int, int, int]] = []
    pixels, mask = [], []
    data = rgba.tobytes()
    for at in range(0, len(data), 4):
        r, g, b, a = data[at : at + 4]
        if a < 128:
            pixels.append(0)
            mask.append(True)
            continue
        if (r, g, b) not in colours:
            colours.append((r, g, b))
        pixels.append(colours.index((r, g, b)))
        mask.append(False)
    if len(colours) > _COLOURS:
        raise ValueError(f"{path}: has {len(colours)} colours; an icon can have {_COLOURS}")
    palette = tuple(colours) + ((0, 0, 0),) * (_COLOURS - len(colours))
    return Icon(width, height, palette, tuple(pixels), tuple(mask))


def _from_indexed(image: Image.Image, path: Path) -> Icon:
    raw = image.getpalette() or []
    entries = [(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, len(raw), 3)]
    transparency = image.info.get("transparency")
    if isinstance(transparency, int):
        transparent = {transparency}
    elif isinstance(transparency, bytes):
        transparent = {i for i, alpha in enumerate(transparency) if alpha < 128}
    else:
        transparent = set()
    pixels, mask = [], []
    for index in image.tobytes():
        if index in transparent:
            pixels.append(0)
            mask.append(True)
        elif index >= _COLOURS:
            raise ValueError(f"{path}: uses colour {index}; an icon has {_COLOURS} (0-15)")
        else:
            pixels.append(index)
            mask.append(False)
    palette = tuple(entries[:_COLOURS]) + ((0, 0, 0),) * max(0, _COLOURS - len(entries))
    return Icon(image.width, image.height, palette, tuple(pixels), tuple(mask))


def _directory_entry(icon: Icon, size: int) -> bytes:
    """What a group or .ico directory says of an image, before its id or offset:
    width, height, colours, reserved, planes, bits and size."""
    return (
        bytes((icon.width % 256, icon.height % 256, _COLOURS, 0))
        + (1).to_bytes(2, "little")
        + _BITS.to_bytes(2, "little")
        + size.to_bytes(4, "little")
    )


def group(icons: list[tuple[int, Icon]]) -> bytes:
    """A GROUP_ICON resource listing icon images, by their ICON resource ids."""
    data = (0).to_bytes(2, "little") + (1).to_bytes(2, "little") + len(icons).to_bytes(2, "little")
    for rid, icon in icons:
        data += _directory_entry(icon, len(to_bytes(icon))) + rid.to_bytes(2, "little")
    return data


def ico_file(icons: list[Icon]) -> bytes:
    """An .ico file holding icon images, in order."""
    images = [to_bytes(icon) for icon in icons]
    data = (0).to_bytes(2, "little") + (1).to_bytes(2, "little") + len(icons).to_bytes(2, "little")
    at = 6 + 16 * len(icons)
    for icon, image in zip(icons, images, strict=True):
        data += _directory_entry(icon, len(image)) + at.to_bytes(4, "little")
        at += len(image)
    return data + b"".join(images)
