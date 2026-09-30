"""QuickTime movie files (`.MOV`) as the game's movies lay them out: a `moov`
atom, then `mdat`; two tracks, video then sound (8-bit mono `twos`), each a
sample table (`stts`, `stsc`, `stsz`, `stco`; the video's also `stss`) and, for
the sound, an edit list. The chunks of both tracks are interleaved in `mdat`,
after a few bytes nothing refers to.

`parse` reads a file into what can't be derived from the rest (`MovInfo`:
timestamps, the sync frames, the sound's edits, the order and size of the
chunks) and the video samples and the sound; `build` writes them back. Both
raise Unconvertible for anything the layout doesn't cover, and `parse` checks
that `build` gives back the same bytes. The fixed parts of the atoms
(handlers, matrices, the sample descriptions' constants) are in `build`.
"""

import re
from typing import NamedTuple

from pydantic import BaseModel, ConfigDict, field_validator

from zbtools.formats.base import Unconvertible, be16, be32, hex_bytes

_CHUNK = re.compile(r"[va]\d+|x(?:[0-9a-f]{2})+")
_IDENTITY = b"".join(v.to_bytes(4, "big") for v in (0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x40000000))
_FIXED = 0x10000  # 1.0 in a 16.16 fixed-point number


class Times(NamedTuple):
    """An atom's creation and modification times, in seconds since 1904."""

    created: int
    modified: int


class MovInfo(BaseModel):
    """What a movie file holds besides the frames and the sound."""

    model_config = ConfigDict(frozen=True, extra="forbid")
    movie: Times  # `mvhd`
    volume: int  # `mvhd`'s
    timescale: int  # units a second, for the movie and the video
    video_track: Times  # `tkhd`
    video_media: Times  # `mdhd`
    width: int
    height: int
    frame_duration: int  # in timescale units
    sync: list[int]  # the frames that stand alone (from 1), `stss`
    audio_track: Times
    audio_media: Times
    rate: int  # samples a second
    audio_edits: list[tuple[int, int]]  # the sound's `elst`: duration, media time (-1: none)
    head: bytes  # `mdat` starts with this, which no chunk uses (hex in TOML)
    chunks: list[str]  # `mdat`'s chunks in order: v<frames>, a<samples> or x<hex>, bytes no
    # chunk refers to

    @field_validator("head", mode="before")
    @classmethod
    def _head(cls, value: str | bytes) -> bytes:
        return hex_bytes(value)

    @field_validator("chunks")
    @classmethod
    def _chunks(cls, chunks: list[str]) -> list[str]:
        for c in chunks:
            if not _CHUNK.fullmatch(c):
                raise ValueError(f"chunk {c!r} isn't v<frames>, a<samples> or x<hex>")
        return chunks


def _u32(value: int) -> bytes:
    return (value & 0xFFFFFFFF).to_bytes(4, "big")


def _u16(value: int) -> bytes:
    return value.to_bytes(2, "big")


def _atom(tag: bytes, *payload: bytes) -> bytes:
    body = b"".join(payload)
    return _u32(8 + len(body)) + tag + body


def _children(data: bytes, start: int, end: int) -> list[tuple[bytes, int, int]]:
    """The atoms from `start` to `end`: tag, where its payload starts and ends."""
    found = []
    at = start
    while at < end:
        if at + 8 > end:
            raise Unconvertible("truncated atom")
        size = be32(data, at)
        if size < 8 or at + size > end:
            raise Unconvertible("atom size")
        found.append((data[at + 4 : at + 8], at + 8, at + size))
        at += size
    return found


def _only(atoms: list[tuple[bytes, int, int]], tag: bytes) -> tuple[int, int]:
    match = [(s, e) for t, s, e in atoms if t == tag]
    if len(match) != 1:
        raise Unconvertible(f"expected one {tag!r} atom")
    return match[0]


def _counted(data: bytes, start: int, end: int, size: int) -> list[bytes]:
    """A table atom's entries: version and flags, a count, then `size` bytes each."""
    count = be32(data, start + 4)
    if end - start != 8 + size * count:
        raise Unconvertible("table size")
    return [data[start + 8 + size * i : start + 8 + size * (i + 1)] for i in range(count)]


class _Track(NamedTuple):
    kind: bytes
    track: Times
    media: Times
    timescale: int
    edits: list[tuple[int, int]]
    description: bytes
    durations: list[tuple[int, int]]  # `stts`: count, duration
    sync: list[int]
    runs: list[tuple[int, int]]  # `stsc`: first chunk, samples a chunk
    sizes: list[int]
    offsets: list[int]


def _signed(value: int) -> int:
    return value - (1 << 32) if value >= 1 << 31 else value


def _track(data: bytes, start: int, end: int) -> _Track:
    atoms = _children(data, start, end)
    tkhd = _only(atoms, b"tkhd")[0]
    mdia = _children(data, *_only(atoms, b"mdia"))
    mdhd = _only(mdia, b"mdhd")[0]
    kind = data[_only(mdia, b"hdlr")[0] + 8 : _only(mdia, b"hdlr")[0] + 12]
    if kind not in (b"vide", b"soun"):
        raise Unconvertible("track kind")
    stbl = _children(data, *_only(_children(data, *_only(mdia, b"minf")), b"stbl"))
    edits: list[tuple[int, int]] = []
    if any(t == b"edts" for t, _, _ in atoms):
        elst = _only(_children(data, *_only(atoms, b"edts")), b"elst")
        edits = [(be32(e, 0), _signed(be32(e, 4))) for e in _counted(data, elst[0], elst[1], 12)]
    stsd = _only(stbl, b"stsd")
    stts = _only(stbl, b"stts")
    stsc = _only(stbl, b"stsc")
    stsz = _only(stbl, b"stsz")
    stco = _only(stbl, b"stco")
    sync: list[int] = []
    if any(t == b"stss" for t, _, _ in stbl):
        stss = _only(stbl, b"stss")
        sync = [be32(e, 0) for e in _counted(data, *stss, 4)]
    uniform, count = be32(data, stsz[0] + 4), be32(data, stsz[0] + 8)
    if uniform:
        sizes = [uniform] * count
        if stsz[1] - stsz[0] != 12:
            raise Unconvertible("stsz")
    else:
        if stsz[1] - stsz[0] != 12 + 4 * count:
            raise Unconvertible("stsz")
        sizes = [be32(data, stsz[0] + 12 + 4 * i) for i in range(count)]
    return _Track(
        kind=kind,
        track=Times(be32(data, tkhd + 4), be32(data, tkhd + 8)),
        media=Times(be32(data, mdhd + 4), be32(data, mdhd + 8)),
        timescale=be32(data, mdhd + 12),
        edits=edits,
        description=data[stsd[0] : stsd[1]],
        durations=[(be32(e, 0), be32(e, 4)) for e in _counted(data, *stts, 8)],
        sync=sync,
        runs=[(be32(e, 0), be32(e, 4)) for e in _counted(data, *stsc, 12)],
        sizes=sizes,
        offsets=[be32(e, 0) for e in _counted(data, *stco, 4)],
    )


def _chunks_of(track: _Track) -> list[tuple[int, int, int]]:
    """A track's chunks: offset, samples, bytes."""
    found = []
    first = 0
    for n, offset in enumerate(track.offsets):
        per = [p for c, p in track.runs if c <= n + 1][-1]
        found.append((offset, per, sum(track.sizes[first : first + per])))
        first += per
    if first != len(track.sizes):
        raise Unconvertible("chunks don't cover the samples")
    return found


class _Split(NamedTuple):
    head: bytes
    frames: list[bytes]
    sound: bytes
    chunks: list[str]


def _split(data: bytes, video: _Track, audio: _Track, mdat: tuple[int, int]) -> _Split:
    """`mdat`'s head, the video samples, the sound and the chunks' order."""
    located = sorted(
        [(o, b"v", n, size) for o, n, size in _chunks_of(video)]
        + [(o, b"a", n, size) for o, n, size in _chunks_of(audio)]
    )
    position = located[0][0]
    frames: list[bytes] = []
    sound = b""
    chunks = []
    at = position
    for offset, kind, count, size in located:
        if offset < at:
            raise Unconvertible("overlapping chunks")
        if offset > at:
            chunks.append(f"x{data[at:offset].hex()}")
        chunk = data[offset : offset + size]
        if kind == b"v":
            sizes = video.sizes[len(frames) : len(frames) + count]
            frames += [chunk[sum(sizes[:i]) : sum(sizes[: i + 1])] for i in range(count)]
        else:
            sound += chunk
        chunks.append(f"{kind.decode()}{count}")
        at = offset + size
    if at > mdat[1]:
        raise Unconvertible("chunks run past the end of mdat")
    if at < mdat[1]:
        chunks.append(f"x{data[at : mdat[1]].hex()}")
    return _Split(data[mdat[0] : position], frames, sound, chunks)


def parse(data: bytes) -> tuple[MovInfo, list[bytes], bytes]:
    """The movie's description, video samples and sound."""
    top = _children(data, 0, len(data))
    if [t for t, _, _ in top] != [b"moov", b"mdat"]:
        raise Unconvertible("expected moov then mdat")
    moov = _children(data, *_only(top, b"moov"))
    if [t for t, _, _ in moov] != [b"mvhd", b"trak", b"trak", b"udta"]:
        raise Unconvertible("expected mvhd, two traks and udta")
    mvhd = _only(moov, b"mvhd")[0]
    video, audio = (_track(data, s, e) for t, s, e in moov if t == b"trak")
    if video.kind != b"vide" or audio.kind != b"soun":
        raise Unconvertible("expected a video track, then a sound track")
    split = _split(data, video, audio, _only(top, b"mdat"))
    rate = be16(audio.description, 40)
    if len(video.durations) != 1 or video.durations[0][0] != len(split.frames):
        raise Unconvertible("frames of different durations")
    if video.timescale != be32(data, mvhd + 12) or audio.timescale != rate:
        raise Unconvertible("timescales")
    info = MovInfo(
        movie=Times(be32(data, mvhd + 4), be32(data, mvhd + 8)),
        volume=be16(data, mvhd + 24),
        timescale=video.timescale,
        video_track=video.track,
        video_media=video.media,
        width=be16(video.description, 40),
        height=be16(video.description, 42),
        frame_duration=video.durations[0][1],
        sync=video.sync,
        audio_track=audio.track,
        audio_media=audio.media,
        rate=rate,
        audio_edits=audio.edits,
        head=split.head,
        chunks=split.chunks,
    )
    if build(info, split.frames, split.sound) != data:
        raise Unconvertible("doesn't round-trip")
    return info, split.frames, split.sound


# Writing


def _hdlr(kind: bytes, subtype: bytes, mask: int, name: bytes) -> bytes:
    return _atom(
        b"hdlr", bytes(4), kind + subtype + b"appl", _u32(0x40000000), _u32(mask),
        bytes([len(name)]) + name,
    )  # fmt: skip


_VIDEO_HDLR = _hdlr(b"mhlr", b"vide", 0x00010022, b"Apple Video Media Handler")
_AUDIO_HDLR = _hdlr(b"mhlr", b"soun", 0x00010023, b"Apple Sound Media Handler")
_ALIAS_HDLR = _hdlr(b"dhlr", b"alis", 0x0001002B, b"Apple Alias Data Handler")
_DINF = _atom(b"dinf", _atom(b"dref", bytes(4), _u32(1), _atom(b"alis", bytes(3), b"\x01")))
_UDTA = _atom(b"udta", _atom(b"Mrks", bytes(2)), _atom(b"TpLf", bytes(4)), bytes(4))


def _table(tag: bytes, entries: list[bytes]) -> bytes:
    return _atom(tag, bytes(4), _u32(len(entries)), *entries)


def _video_stsd(width: int, height: int) -> bytes:
    # version 1, revision 9, vendor Brod, quality 0x200 both ways, 72 dpi, one
    # frame a sample, compressor name (a 31-byte Pascal string), depth 8,
    # no colour table
    entry = _atom(
        b"QkBk", bytes(6), _u16(1), _u16(1), _u16(9), b"Brod", _u32(0x200), _u32(0x200),
        _u16(width), _u16(height), _u32(72 * _FIXED), _u32(72 * _FIXED), bytes(4), _u16(1),
        b"\x07CDToons".ljust(32, b"\0"), _u16(8), _u16(0xFFFF),
    )  # fmt: skip
    return _atom(b"stsd", bytes(4), _u32(1), entry)


def _audio_stsd(rate: int) -> bytes:
    # one channel of 8-bit samples, uncompressed
    entry = _atom(
        b"twos", bytes(6), _u16(1), bytes(8), _u16(1), _u16(8), bytes(4), _u32(rate * _FIXED)
    )
    return _atom(b"stsd", bytes(4), _u32(1), entry)


def _sample_table(
    description: bytes,
    *,
    durations: list[tuple[int, int]],
    sync: list[int] | None,
    chunks: list[tuple[int, int]],
    sizes: list[int],
    uniform: bool,
    offsets: list[int],
) -> bytes:
    """`stbl`: the chunks are (samples, bytes) and `offsets` where they start."""
    runs: list[tuple[int, int]] = []
    for n, (samples, _) in enumerate(chunks, 1):
        if not runs or runs[-1][1] != samples:
            runs.append((n, samples))
    parts = [description, _table(b"stts", [_u32(c) + _u32(d) for c, d in durations])]
    if sync is not None:
        parts.append(_table(b"stss", [_u32(s) for s in sync]))
    parts.append(_table(b"stsc", [_u32(f) + _u32(p) + _u32(1) for f, p in runs]))
    if uniform:
        parts.append(_atom(b"stsz", bytes(4), _u32(sizes[0] if sizes else 1), _u32(len(sizes))))
    else:
        parts.append(_atom(b"stsz", bytes(8), _u32(len(sizes)), *(_u32(s) for s in sizes)))
    parts.append(_table(b"stco", [_u32(o) for o in offsets]))
    return _atom(b"stbl", *parts)


def _tkhd(times: Times, *, track: int, duration: int, volume: int, size: tuple[int, int]) -> bytes:
    return _atom(
        b"tkhd", _u32(0x0F), _u32(times.created), _u32(times.modified), _u32(track), bytes(4),
        _u32(duration), bytes(8), bytes(4), _u16(volume), bytes(2), _IDENTITY,
        _u32(size[0] * _FIXED), _u32(size[1] * _FIXED),
    )  # fmt: skip


def _mdhd(times: Times, timescale: int, duration: int) -> bytes:
    return _atom(
        b"mdhd", bytes(4), _u32(times.created), _u32(times.modified), _u32(timescale),
        _u32(duration), bytes(4),
    )  # fmt: skip


def _elst(edits: list[tuple[int, int]]) -> bytes:
    entries = [_u32(d) + _u32(m) + _u32(_FIXED) for d, m in edits]
    return _atom(b"edts", _table(b"elst", entries))


class _Piece(NamedTuple):
    kind: str  # v, a or x
    samples: int  # (x: bytes)
    data: bytes


def _pieces(info: MovInfo, frames: list[bytes], sound: bytes) -> list[_Piece]:
    """`mdat`'s chunks after its head, in order."""
    pieces = []
    frame = at = 0
    for c in info.chunks:
        if c[0] == "x":
            raw = bytes.fromhex(c[1:])
            pieces.append(_Piece("x", len(raw), raw))
        elif c[0] == "v":
            count = int(c[1:])
            pieces.append(_Piece("v", count, b"".join(frames[frame : frame + count])))
            frame += count
        else:
            count = int(c[1:])
            pieces.append(_Piece("a", count, sound[at : at + count]))
            at += count
    if (frame, at) != (len(frames), len(sound)):
        raise ValueError("the chunks don't hold exactly the frames and the sound")
    return pieces


def _moov(
    info: MovInfo, frames: list[bytes], sound: bytes, pieces: list[_Piece], base: int
) -> bytes:
    """`moov`, for chunks that start at `base`."""
    chunks: dict[str, list[tuple[int, int]]] = {"v": [], "a": []}
    offsets: dict[str, list[int]] = {"v": [], "a": []}
    at = base
    for piece in pieces:
        if piece.kind != "x":
            chunks[piece.kind].append((piece.samples, len(piece.data)))
            offsets[piece.kind].append(at)
        at += len(piece.data)
    duration = len(frames) * info.frame_duration
    video_table = _sample_table(
        _video_stsd(info.width, info.height),
        durations=[(len(frames), info.frame_duration)],
        sync=info.sync,
        chunks=chunks["v"],
        sizes=[len(f) for f in frames],
        uniform=False,
        offsets=offsets["v"],
    )
    audio_table = _sample_table(
        _audio_stsd(info.rate),
        durations=[(len(sound), 1)],
        sync=None,
        chunks=chunks["a"],
        sizes=[1] * len(sound),
        uniform=True,
        offsets=offsets["a"],
    )
    video_minf = _atom(
        b"minf", _atom(b"vmhd", _u32(1), _u16(0x40), _u16(0x8000) * 3), _ALIAS_HDLR, _DINF,
        video_table,
    )  # fmt: skip
    audio_minf = _atom(b"minf", _atom(b"smhd", bytes(8)), _ALIAS_HDLR, _DINF, audio_table)
    video = _atom(
        b"trak",
        _tkhd(
            info.video_track, track=1, duration=duration, volume=0, size=(info.width, info.height)
        ),
        _elst([(duration, 0)]),
        _atom(b"mdia", _mdhd(info.video_media, info.timescale, duration), _VIDEO_HDLR, video_minf),
    )
    audio = _atom(
        b"trak",
        _tkhd(info.audio_track, track=2, duration=duration, volume=0x100, size=(0, 0)),
        _elst(info.audio_edits),
        _atom(b"mdia", _mdhd(info.audio_media, info.rate, len(sound)), _AUDIO_HDLR, audio_minf),
    )
    mvhd = _atom(
        b"mvhd", bytes(4), _u32(info.movie.created), _u32(info.movie.modified),
        _u32(info.timescale), _u32(duration), _u32(_FIXED), _u16(info.volume), bytes(10),
        _IDENTITY, bytes(24), _u32(3),
    )  # fmt: skip
    return _atom(b"moov", mvhd, video, audio, _UDTA)


def build(info: MovInfo, frames: list[bytes], sound: bytes) -> bytes:
    """The movie file."""
    pieces = _pieces(info, frames, sound)
    size = len(_moov(info, frames, sound, pieces, 0))
    moov = _moov(info, frames, sound, pieces, size + 8 + len(info.head))
    return moov + _atom(b"mdat", info.head, *(p.data for p in pieces))
