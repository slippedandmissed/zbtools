"""QkBk, Broderbund's video codec (`qb32.qtc`): a movie frame is a scene, not
pixels. Sprites from a library of bitmaps are placed on a background, in 256
colours; the codec composites them, so a frame that changes nothing is 420
bytes. (The layout below was read from the codec; `docs/findings.md` has the
evidence.) Everything is big-endian.

A frame (one QuickTime sample):

    0x00 u16 version (9)            0x18 u32 flags
    0x02 u16 frame number (from 1)  0x1c u16 number of cast items
    0x04 u16 ?                      0x1e u16 offset of the cast items
    0x06 u8  ?                      0x20 u16 the movie's highest cast ID
    0x07 u8  background colour      0x22 u8  number of sprite slots
    0x08 rect: the sprites' bounds  0x23 u8  ?
    0x10 rect: what changed         0x24 u16 palette (a cast item's ID)
                                    0x26 u16 x3: zero
    0x2c sprite slots, 10 bytes each: u16 cast ID (0: empty), i16 y, i16 x,
         u16 height, u16 width (the cast bitmap's)
    then the cast items, the optional blocks the flags ask for, and a tail

A rect is {i16 top, left, bottom, right}. The bounds are the union of the
frame's sprites' rects, and what changed is the union of the old and new rects
of the slots that differ from the last frame's (all of them, in the first):
the codec redraws that much. (Both hold in every frame of the four movies; the
converter derives them.) The flags: 1 the last frame, 8 don't
clear the background before drawing, 0x10 an `XFrm` block follows, 0x20 a
`BckR` and 0x40 an `FrtR` block (2 would be a block of callbacks; none of the
game's movies has any). The blocks are {4-byte tag, u32 size including this
header}: `BckR` and `FrtR` have a u16 count and that many rects from offset 10;
`XFrm` has a u16 (0x020c), a rect and eight zero bytes. The tail is what's
left: in a movie's first frame a `Scal` block, which the codec never reads.

A cast item is {u16 ID, u16 type, u32 size, ...}, its ID unique in the movie
and its data sent once, in the frame that first needs it (the first frame sends
most). Type 1 is a palette: u16 x3 (1, the frame count, 0), then 256 colours of
three u16, which are a byte of each of red, green and blue and a zero byte.
Type 2 is a bitmap:

    0x08 u16 ?, u16 ?, u16 ?    0x12 u16 bits per pixel (8)
    0x0e u16 height, u16 width  0x14 u16 width, u16 height
    0x18 u16 bytes per row (the width rounded up to 4)    0x1a u16 18
    0x1c its rows, each {u16 length, run-length data}

Run-length data is a control byte and what it says: with its top bit clear, a
literal of (control + 1) pixels follows; with it set, (control & 0x7f) + 1
pixels of the one value that follows. A run of the colour 0 leaves the
background showing (0 is transparent: a literal never holds it). Broderbund's
encoder runs are at least 8 long, except zeros, which always run; a run
longer than 128 is cut there and the rest of it (under 8) goes into the next
literal; a literal is at most 128 long; a row is padded to an even length with
a zero byte. (Checked on all 3,156 bitmaps of the four movies: their 178,380
rows encode back byte for byte.)
"""

from collections.abc import Mapping
from typing import NamedTuple

from PIL import Image
from pydantic import BaseModel, ConfigDict, field_validator

from zbtools.formats.base import Unconvertible, be16, be32, hex_bytes

VERSION = 9
HEADER_SIZE = 0x2C
SPRITE_SIZE = 10
BITMAP_HEADER_SIZE = 0x1C
PALETTE_HEADER_SIZE = 14
PALETTE_COLOURS = 256

CAST_PALETTE = 1
CAST_BITMAP = 2

FLAG_LAST = 0x01
FLAG_KEEP_BACKGROUND = 0x08
FLAG_XFRM = 0x10
FLAG_BCKR = 0x20
FLAG_FRTR = 0x40
_BLOCK_FLAGS = FLAG_XFRM | FLAG_BCKR | FLAG_FRTR
USER_FLAGS = FLAG_LAST | FLAG_KEEP_BACKGROUND

MIN_RUN = 8  # the shortest run of a colour that isn't 0
MAX_TOKEN = 128

_XFRM_TAG = b"XFrm"
_XFRM_SIZE = 26
_XFRM_WORD = 0x020C


class Rect(NamedTuple):
    top: int
    left: int
    bottom: int
    right: int


class Sprite(NamedTuple):
    """A bitmap placed in one of a frame's slots (drawn in slot order)."""

    slot: int
    cast: int
    x: int
    y: int


class Palette(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    id: int
    head: tuple[int, int, int]  # the three words before the colours
    colours: list[str]  # #rrggbb


class Bitmap(BaseModel):
    """A sprite image: `pixels` are the rows top to bottom, one byte a pixel."""

    model_config = ConfigDict(frozen=True, extra="forbid")
    id: int
    head: tuple[int, int, int]  # the three words at 0x08
    width: int
    height: int
    pixels: bytes


Cast = Palette | Bitmap


class Frame(BaseModel):
    """A frame but for its number (its place in the movie), its bounds and
    what changed (derived from the sprites), the cast items it defines (only
    their IDs: `Cast` has them) and what the blocks' presence says about the
    flags."""

    model_config = ConfigDict(frozen=True, extra="forbid")
    f4: int
    f6: int
    fill: int  # the background colour
    flags: int  # the flags in USER_FLAGS
    palette: int
    sprites: list[Sprite] = []  # the slots in use
    casts: list[int] = []  # the IDs of the cast items in this frame, in order
    back: list[Rect] | None = None  # the `BckR` block
    front: list[Rect] | None = None  # the `FrtR` block
    xfrm: Rect | None = None  # the `XFrm` block
    tail: bytes = b""  # (hex in TOML)

    @field_validator("tail", mode="before")
    @classmethod
    def _tail(cls, value: str | bytes) -> bytes:
        return hex_bytes(value)


class Movie(BaseModel):
    """The scene data of a movie: its frames and the cast items they define."""

    model_config = ConfigDict(frozen=True, extra="forbid")
    slots: int  # sprite slots in every frame
    f23: int
    frames: list[Frame]
    casts: dict[int, Cast]


# Run-length rows


def encode_row(pixels: bytes) -> bytes:
    """A row's run-length data as Broderbund's encoder writes it."""
    out = bytearray()
    at = 0
    end = len(pixels)

    def run(start: int) -> int:
        stop = start
        while stop < end and pixels[stop] == pixels[start]:
            stop += 1
        return stop - start

    while at < end:
        length = run(at)
        if pixels[at] == 0 or length >= MIN_RUN:
            length = min(length, MAX_TOKEN)
            out += bytes([0x80 | (length - 1), pixels[at]])
            at += length
        else:
            stop = at
            while stop < end and stop - at < MAX_TOKEN:
                if pixels[stop] == 0 or run(stop) >= MIN_RUN:
                    break
                stop += 1
            out += bytes([stop - at - 1]) + pixels[at:stop]
            at = stop
    if len(out) % 2:
        out.append(0)
    return bytes(out)


def decode_row(data: bytes, width: int) -> bytes:
    """A row's pixels from its run-length data; Unconvertible unless the data
    is exactly what `encode_row` gives for them."""
    pixels = bytearray()
    at = 0
    while len(pixels) < width:
        if at >= len(data):
            raise Unconvertible("row data ends before the row does")
        control = data[at]
        count = (control & 0x7F) + 1
        if control & 0x80:
            if at + 1 >= len(data):
                raise Unconvertible("truncated run")
            pixels += bytes([data[at + 1]]) * count
            at += 2
        else:
            pixels += data[at + 1 : at + 1 + count]
            at += 1 + count
    if len(pixels) != width:
        raise Unconvertible("a run crosses the end of its row")
    if encode_row(bytes(pixels)) != data:
        raise Unconvertible("row isn't what the encoder writes")
    return bytes(pixels)


# Cast items


def _s16(data: bytes, offset: int) -> int:
    value = be16(data, offset)
    return value - 0x10000 if value >= 0x8000 else value


def _put16(value: int) -> bytes:
    return (value & 0xFFFF).to_bytes(2, "big")


def _rect(data: bytes, offset: int) -> Rect:
    return Rect(*(_s16(data, offset + 2 * i) for i in range(4)))


def _rect_bytes(rect: Rect) -> bytes:
    return b"".join(_put16(v) for v in rect)


def _row_stride(width: int) -> int:
    return (width + 3) // 4 * 4


def parse_palette(data: bytes) -> Palette:
    if (len(data) - PALETTE_HEADER_SIZE) % 6:
        raise Unconvertible("palette size")
    colours = []
    for at in range(PALETTE_HEADER_SIZE, len(data), 6):
        if any(data[at + 1 : at + 6 : 2]):
            raise Unconvertible("palette colour with a low byte")
        colours.append(f"#{data[at]:02x}{data[at + 2]:02x}{data[at + 4]:02x}")
    palette = Palette(
        id=be16(data, 0), head=(be16(data, 8), be16(data, 10), be16(data, 12)), colours=colours
    )
    if palette_bytes(palette) != data:
        raise Unconvertible("palette doesn't round-trip")
    return palette


def palette_bytes(palette: Palette) -> bytes:
    size = PALETTE_HEADER_SIZE + 6 * len(palette.colours)
    data = _put16(palette.id) + _put16(CAST_PALETTE) + size.to_bytes(4, "big")
    data += b"".join(_put16(v) for v in palette.head)
    for colour in palette.colours:
        for channel in bytes.fromhex(colour[1:]):
            data += bytes([channel, 0])
    return data


def parse_bitmap(data: bytes) -> Bitmap:
    height, width = be16(data, 0x0E), be16(data, 0x10)
    if (
        be16(data, 0x12),
        be16(data, 0x14),
        be16(data, 0x16),
        be16(data, 0x18),
        be16(data, 0x1A),
    ) != (8, width, height, _row_stride(width), 18):
        raise Unconvertible("bitmap header")
    at = BITMAP_HEADER_SIZE
    rows = []
    for _ in range(height):
        if at + 2 > len(data):
            raise Unconvertible("truncated bitmap")
        length = be16(data, at)
        rows.append(decode_row(data[at + 2 : at + 2 + length], width))
        at += 2 + length
    if at != len(data):
        raise Unconvertible("bitmap size doesn't match its rows")
    return Bitmap(
        id=be16(data, 0),
        head=(be16(data, 8), be16(data, 10), be16(data, 12)),
        width=width,
        height=height,
        pixels=b"".join(rows),
    )


def bitmap_bytes(bitmap: Bitmap) -> bytes:
    if len(bitmap.pixels) != bitmap.width * bitmap.height:
        raise ValueError(f"bitmap {bitmap.id}: {len(bitmap.pixels)} pixels for its size")
    rows = b""
    for y in range(bitmap.height):
        row = encode_row(bitmap.pixels[y * bitmap.width : (y + 1) * bitmap.width])
        rows += _put16(len(row)) + row
    size = BITMAP_HEADER_SIZE + len(rows)
    data = _put16(bitmap.id) + _put16(CAST_BITMAP) + size.to_bytes(4, "big")
    data += b"".join(_put16(v) for v in bitmap.head)
    data += _put16(bitmap.height) + _put16(bitmap.width)
    data += b"".join(
        _put16(v) for v in (8, bitmap.width, bitmap.height, _row_stride(bitmap.width), 18)
    )
    return data + rows


def cast_bytes(cast: Cast) -> bytes:
    return palette_bytes(cast) if isinstance(cast, Palette) else bitmap_bytes(cast)


# Frames


def _blocks_end(data: bytes, at: int, tag: bytes) -> tuple[list[Rect], int]:
    """A `BckR` or `FrtR` block's rects, and where the block ends."""
    if data[at : at + 4] != tag:
        raise Unconvertible(f"no {tag!r} block where the flags say")
    size, count = be32(data, at + 4), be16(data, at + 8)
    if size != 10 + 8 * count or at + size > len(data):
        raise Unconvertible(f"{tag!r} block size")
    return [_rect(data, at + 10 + 8 * i) for i in range(count)], at + size


def _rects_block(tag: bytes, rects: list[Rect]) -> bytes:
    size = 10 + 8 * len(rects)
    body = tag + size.to_bytes(4, "big") + _put16(len(rects))
    return body + b"".join(_rect_bytes(r) for r in rects)


class _Header(NamedTuple):
    flags: int
    slots: int
    cast_count: int
    cast_at: int
    f20: int
    f23: int


class ParsedFrame(NamedTuple):
    frame: Frame
    casts: list[Cast]  # the cast items it defines
    slots: int  # the movie-wide values in its header
    f20: int
    f23: int


def _parse_header(data: bytes, number: int) -> _Header:
    if len(data) < HEADER_SIZE or be16(data, 0) != VERSION or be16(data, 2) != number:
        raise Unconvertible("frame header")
    flags = be32(data, 0x18)
    if flags & ~(USER_FLAGS | _BLOCK_FLAGS):
        raise Unconvertible(f"flags {flags:#x}")
    slots, cast_at = data[0x22], be16(data, 0x1E)
    if cast_at != HEADER_SIZE + SPRITE_SIZE * slots or len(data) < cast_at:
        raise Unconvertible("frame header")
    if any(data[0x26:0x2C]):
        raise Unconvertible("frame header")
    return _Header(flags, slots, be16(data, 0x1C), cast_at, be16(data, 0x20), data[0x23])


def _parse_casts(data: bytes, at: int, count: int) -> tuple[list[Cast], int]:
    """`count` cast items from `at`, and where they end."""
    casts: list[Cast] = []
    for _ in range(count):
        if at + 8 > len(data):
            raise Unconvertible("truncated cast item")
        kind, size = be16(data, at + 2), be32(data, at + 4)
        item = data[at : at + size]
        if size < 8 or len(item) != size:
            raise Unconvertible("truncated cast item")
        if kind == CAST_PALETTE:
            casts.append(parse_palette(item))
        elif kind == CAST_BITMAP:
            casts.append(parse_bitmap(item))
        else:
            raise Unconvertible(f"cast item type {kind}")
        at += size
    return casts, at


def _parse_xfrm(data: bytes, at: int) -> Rect:
    block = data[at : at + _XFRM_SIZE]
    if (
        block[:4] != _XFRM_TAG
        or len(block) != _XFRM_SIZE
        or be32(block, 4) != _XFRM_SIZE
        or be16(block, 8) != _XFRM_WORD
        or any(block[18:])
    ):
        raise Unconvertible("XFrm block")
    return _rect(block, 10)


def _parse_sprites(data: bytes, slots: int) -> list[Sprite]:
    sprites = []
    for slot in range(slots):
        entry = data[HEADER_SIZE + SPRITE_SIZE * slot : HEADER_SIZE + SPRITE_SIZE * (slot + 1)]
        cast = be16(entry, 0)
        if cast == 0:
            if any(entry):
                raise Unconvertible("empty sprite slot with data")
        else:
            sprites.append(Sprite(slot, cast, _s16(entry, 4), _s16(entry, 2)))
    return sprites


def parse_frame(data: bytes, number: int) -> ParsedFrame:
    """A frame (the `number`th, from 1), the cast items it defines and the
    movie-wide values in its header. Unconvertible unless `build_frame` would
    give back `data`."""
    header = _parse_header(data, number)
    casts, at = _parse_casts(data, header.cast_at, header.cast_count)
    back = front = xfrm = None
    if header.flags & FLAG_BCKR:
        back, at = _blocks_end(data, at, b"BckR")
    if header.flags & FLAG_FRTR:
        front, at = _blocks_end(data, at, b"FrtR")
    if header.flags & FLAG_XFRM:
        xfrm = _parse_xfrm(data, at)
        at += _XFRM_SIZE
    frame = Frame(
        f4=be16(data, 4),
        f6=data[6],
        fill=data[7],
        flags=header.flags & USER_FLAGS,
        palette=be16(data, 0x24),
        sprites=_parse_sprites(data, header.slots),
        casts=[c.id for c in casts],
        back=back,
        front=front,
        xfrm=xfrm,
        tail=data[at:],
    )
    return ParsedFrame(frame, casts, header.slots, header.f20, header.f23)


def _union(rects: list[Rect]) -> Rect:
    if not rects:
        return Rect(0, 0, 0, 0)
    return Rect(
        min(r.top for r in rects),
        min(r.left for r in rects),
        max(r.bottom for r in rects),
        max(r.right for r in rects),
    )


def _sprite_rects(sprites: list[Sprite], casts: Mapping[int, Cast]) -> dict[int, tuple[int, Rect]]:
    """Each slot's cast item and rect."""
    found = {}
    for sprite in sprites:
        bitmap = casts.get(sprite.cast)
        if not isinstance(bitmap, Bitmap):
            raise ValueError(f"sprite {tuple(sprite)} isn't a bitmap")
        rect = Rect(sprite.y, sprite.x, sprite.y + bitmap.height, sprite.x + bitmap.width)
        found[sprite.slot] = (sprite.cast, rect)
    return found


def _changes(
    before: list[Sprite], after: list[Sprite], casts: Mapping[int, Cast]
) -> tuple[Rect, Rect]:
    """A frame's bounds and what changed since the sprites `before`."""
    old, new = _sprite_rects(before, casts), _sprite_rects(after, casts)
    changed = []
    for slot in old.keys() | new.keys():
        if old.get(slot) != new.get(slot):
            changed += [entry[1] for entry in (old.get(slot), new.get(slot)) if entry]
    return _union([rect for _, rect in new.values()]), _union(changed)


def build_frame(
    frame: Frame,
    number: int,
    *,
    before: list[Sprite],
    slots: int,
    f20: int,
    f23: int,
    casts: Mapping[int, Cast],
) -> bytes:
    """The frame as a sample, after one whose sprites were `before`. `casts`
    is every cast item the movie defines up to and including this frame: the
    sprites' sizes and what changed come from its bitmaps."""
    flags = frame.flags | (FLAG_BCKR if frame.back is not None else 0)
    flags |= (FLAG_FRTR if frame.front is not None else 0) | (
        FLAG_XFRM if frame.xfrm is not None else 0
    )
    bounds, dirty = _changes(before, frame.sprites, casts)
    head = _put16(VERSION) + _put16(number) + _put16(frame.f4) + bytes([frame.f6, frame.fill])
    head += _rect_bytes(bounds) + _rect_bytes(dirty) + flags.to_bytes(4, "big")
    head += _put16(len(frame.casts)) + _put16(HEADER_SIZE + SPRITE_SIZE * slots)
    head += _put16(f20) + bytes([slots, f23]) + _put16(frame.palette) + bytes(6)
    entries = [bytes(SPRITE_SIZE)] * slots
    for sprite in frame.sprites:
        bitmap = casts.get(sprite.cast)
        if not isinstance(bitmap, Bitmap) or not 0 <= sprite.slot < slots:
            raise ValueError(f"frame {number}: sprite {tuple(sprite)} isn't a bitmap in a slot")
        entries[sprite.slot] = (
            _put16(sprite.cast)
            + _put16(sprite.y)
            + _put16(sprite.x)
            + _put16(bitmap.height)
            + _put16(bitmap.width)
        )
    body = b"".join(cast_bytes(casts[i]) for i in frame.casts)
    if frame.back is not None:
        body += _rects_block(b"BckR", frame.back)
    if frame.front is not None:
        body += _rects_block(b"FrtR", frame.front)
    if frame.xfrm is not None:
        body += _XFRM_TAG + _XFRM_SIZE.to_bytes(4, "big") + _put16(_XFRM_WORD)
        body += _rect_bytes(frame.xfrm) + bytes(8)
    return head + b"".join(entries) + body + frame.tail


def parse_movie(samples: list[bytes]) -> Movie:
    """The scene data of a movie's video samples; Unconvertible unless
    `build_movie` would give them back."""
    frames: list[Frame] = []
    casts: dict[int, Cast] = {}
    movie_wide: set[tuple[int, int, int]] = set()  # slots, f20, f23
    for number, data in enumerate(samples, 1):
        parsed = parse_frame(data, number)
        movie_wide.add((parsed.slots, parsed.f20, parsed.f23))
        for cast in parsed.casts:
            if cast.id in casts:
                raise Unconvertible(f"cast item {cast.id} defined twice")
            casts[cast.id] = cast
        for sprite in parsed.frame.sprites:
            if not isinstance(casts.get(sprite.cast), Bitmap):
                raise Unconvertible(f"frame {number} places cast item {sprite.cast}, not a bitmap")
        palette = parsed.frame.palette
        if palette and not isinstance(casts.get(palette), Palette):
            raise Unconvertible(f"frame {number}: palette {palette} isn't defined yet")
        frames.append(parsed.frame)
    if len(movie_wide) != 1:
        raise Unconvertible("frames disagree on the slot count or the movie-wide fields")
    slots, f20, f23 = movie_wide.pop()
    if f20 != max(casts, default=0):
        raise Unconvertible("the frames' highest cast ID isn't the movie's")
    movie = Movie(slots=slots, f23=f23, frames=frames, casts=casts)
    if build_movie(movie) != samples:
        raise Unconvertible("frames don't round-trip")
    return movie


def build_movie(movie: Movie) -> list[bytes]:
    """The movie's video samples."""
    samples = []
    defined: dict[int, Cast] = {}
    before: list[Sprite] = []
    highest = max(movie.casts, default=0)
    for number, frame in enumerate(movie.frames, 1):
        for i in frame.casts:
            if i not in movie.casts or i in defined:
                raise ValueError(f"frame {number}: cast item {i} is missing or defined twice")
            defined[i] = movie.casts[i]
        samples.append(
            build_frame(
                frame,
                number,
                before=before,
                slots=movie.slots,
                f20=highest,
                f23=movie.f23,
                casts=defined,
            )
        )
        before = frame.sprites
    return samples


# Seeing a frame


def render_frame(movie: Movie, number: int, size: tuple[int, int]) -> Image.Image:
    """The `number`th frame (from 1) as a palette image of `size`: its sprites
    in slot order on the background colour, in the frame's palette. That's each
    frame drawn from scratch, where the codec draws over the last and clears
    only what changed; the two agree because what changed is worked out from the
    sprites (`uv run movie-check` compares them with the original codec's)."""
    frame = movie.frames[number - 1]
    image = Image.new("P", size, frame.fill)
    palette = movie.casts.get(frame.palette)
    if isinstance(palette, Palette):
        image.putpalette([c for colour in palette.colours for c in bytes.fromhex(colour[1:])])
    for sprite in sorted(frame.sprites, key=lambda s: s.slot):
        bitmap = movie.casts[sprite.cast]
        if not isinstance(bitmap, Bitmap):
            raise ValueError(f"frame {number}: sprite {tuple(sprite)} isn't a bitmap")
        picture = Image.frombytes("P", (bitmap.width, bitmap.height), bitmap.pixels)
        opaque = picture.point(lambda v: 255 if v else 0, "L")  # (colour 0 is transparent)
        image.paste(picture, (sprite.x, sprite.y), opaque)
    return image
