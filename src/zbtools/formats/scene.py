"""A movie as the port plays it: one `.SCN` file the player (port/glue/
quicktime.cpp) reads without parsing TOML or PNGs. The scene is the same
(`formats/qkbk.py`: each frame draws its sprites in slot order on a background
colour, in a palette), but it's flat and little-endian, and the sound has the
movie's edit list applied, so it's one track from time 0.

    "ZBSC", u32 version (1), u16 width, u16 height, u32 milliseconds a frame,
    u32 frames, u32 sound rate, u32 sound samples, u32 cast items
    frames: u8 background colour, u8 sprites, u16 palette (a cast item ID),
        then each sprite: u16 cast item ID, i16 x, i16 y (in slot order)
    cast items: u16 ID, u8 type, u8 0, then
        palette (1): 256 colours of u8 red, green, blue
        bitmap (2): u16 width, u16 height, u32 size, then the rows, each
            {u16 length, run-length data} as in a QkBk frame (formats/qkbk.py)
    sound: 8-bit unsigned mono samples

Only the movie's look and sound are here: what the codec does with the rest of
a frame's fields (what changed, the blocks) doesn't change what it draws.
"""

import struct

from zbtools.formats import qkbk

MAGIC = b"ZBSC"
VERSION = 1
_SILENCE = 0x80  # unsigned 8-bit


def _unsigned(samples: bytes) -> bytes:
    """8-bit signed samples (QuickTime's `twos`) as unsigned."""
    return samples.translate(bytes((i + 128) % 256 for i in range(256)))


def expand_edits(sound: bytes, edits: list[tuple[int, int]], timescale: int, rate: int) -> bytes:
    """The sound as the edit list plays it, unsigned: each edit is a duration
    (in `timescale` units a second) of the sound from a media time (in
    samples), or of silence for -1."""
    out = bytearray()
    elapsed = 0
    for duration, media_time in edits:
        elapsed += duration
        count = round(elapsed * rate / timescale) - len(out)
        if media_time < 0:
            out += bytes([_SILENCE]) * count
        else:
            piece = _unsigned(sound[media_time : media_time + count])
            out += piece + bytes([_SILENCE]) * (count - len(piece))
    return bytes(out)


def build(
    scene: qkbk.Movie,
    *,
    size: tuple[int, int],
    milliseconds: int,
    rate: int,
    sound: bytes,
) -> bytes:
    """The scene file: `sound` is the soundtrack as it plays, unsigned."""
    head = MAGIC + struct.pack(
        "<IHHIIIII",
        VERSION,
        size[0],
        size[1],
        milliseconds,
        len(scene.frames),
        rate,
        len(sound),
        len(scene.casts),
    )
    frames = bytearray()
    for frame in scene.frames:
        sprites = sorted(frame.sprites, key=lambda s: s.slot)
        frames += struct.pack("<BBH", frame.fill, len(sprites), frame.palette)
        for sprite in sprites:
            frames += struct.pack("<Hhh", sprite.cast, sprite.x, sprite.y)
    casts = bytearray()
    for cast_id, cast in scene.casts.items():
        if isinstance(cast, qkbk.Palette):
            casts += struct.pack("<HBB", cast_id, qkbk.CAST_PALETTE, 0)
            casts += b"".join(bytes.fromhex(colour[1:]) for colour in cast.colours)
        else:
            rows = b""
            for y in range(cast.height):
                row = qkbk.encode_row(cast.pixels[y * cast.width : (y + 1) * cast.width])
                rows += struct.pack("<H", len(row)) + row
            casts += struct.pack("<HBB", cast_id, qkbk.CAST_BITMAP, 0)
            casts += struct.pack("<HHI", cast.width, cast.height, len(rows)) + rows
    return head + bytes(frames) + bytes(casts) + sound
