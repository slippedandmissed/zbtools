"""Sounds (`\\0SND`): Mohawk wave files, stored as WAV.

A Mohawk wave file (as the engine's waveObj reads it, decomp/wavesound.cpp) is
`MHWK`, the size of the rest, `WAVE`, then big-endian chunks padded to even
lengths: an optional `Cue#` (a u16 count of named positions) and `Data`:

    u16 sample rate, u32 sample count, u8 bits per sample, u8 channels,
    u16 encoding (0: PCM), u16 loop count (0: none, 0xffff: forever),
    u32 loop start, u32 loop end (in samples, the end exclusive), samples

The WAV file has the same samples (8-bit PCM is unsigned in both), a `smpl`
chunk for the loop (a play count of 0 is forever; its end is inclusive) and an
empty `cue ` chunk where the sound has an empty `Cue#`. The game's sounds are
all 8-bit PCM and no `Cue#` lists any positions; others are kept raw.

Hand-written: Python's `wave` module ignores the `smpl` and `cue ` chunks.
"""

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from zbtools.formats.base import Unconvertible, be16, be32, chunk, chunks, le16, le32
from zbtools.mohawk import Resource

SUFFIX = ".wav"
_FOREVER = 0xFFFF
_PCM = 1  # WAVE_FORMAT_PCM


@dataclass(frozen=True)
class Sound:
    rate: int
    channels: int
    bits: int
    samples: bytes
    loops: int = 0  # plays of the loop: 0 none, 0xffff forever
    loop_start: int = 0
    loop_end: int = 0  # exclusive
    cues: bool = False  # has an (empty) cue list

    @property
    def count(self) -> int:
        return len(self.samples) // (self.channels * self.bits // 8)


def parse_mohawk(data: bytes) -> Sound:
    if data[:4] != b"MHWK" or data[8:12] != b"WAVE" or be32(data, 4) != len(data) - 8:
        raise Unconvertible("not a Mohawk wave file")
    found = chunks(data, 12, big_endian=True)
    tags = [tag for tag, _ in found]
    if tags not in ([b"Data"], [b"Cue#", b"Data"]):
        raise Unconvertible(f"chunks {tags}")
    if len(found) == 2 and found[0][1] != bytes(2):
        raise Unconvertible("cue points")
    body = found[-1][1]
    sound = Sound(
        rate=be16(body, 0),
        channels=body[7],
        bits=body[6],
        samples=body[20:],
        loops=be16(body, 10),
        loop_start=be32(body, 12),
        loop_end=be32(body, 16),
        cues=len(found) == 2,
    )
    if sound.bits != 8 or be16(body, 8) != 0 or sound.channels not in (1, 2):
        raise Unconvertible("not 8-bit PCM")
    if len(sound.samples) % sound.channels or be32(body, 2) != sound.count:
        raise Unconvertible("sample count doesn't match the data")
    if sound.loops == 0 and (sound.loop_start or sound.loop_end):
        raise Unconvertible("loop bounds without a loop")
    if sound.loops and not sound.loop_start < sound.loop_end <= sound.count:
        raise Unconvertible("invalid loop")
    return sound


def to_mohawk(sound: Sound) -> bytes:
    body = sound.rate.to_bytes(2, "big") + sound.count.to_bytes(4, "big")
    body += bytes([sound.bits, sound.channels]) + bytes(2)
    body += sound.loops.to_bytes(2, "big")
    body += sound.loop_start.to_bytes(4, "big") + sound.loop_end.to_bytes(4, "big")
    body += sound.samples
    content = b"WAVE"
    if sound.cues:
        content += chunk(b"Cue#", bytes(2), big_endian=True)
    content += chunk(b"Data", body, big_endian=True)
    return b"MHWK" + len(content).to_bytes(4, "big") + content


def to_wav(sound: Sound) -> bytes:
    block = sound.channels * sound.bits // 8
    fmt = _PCM.to_bytes(2, "little") + sound.channels.to_bytes(2, "little")
    fmt += sound.rate.to_bytes(4, "little") + (sound.rate * block).to_bytes(4, "little")
    fmt += block.to_bytes(2, "little") + sound.bits.to_bytes(2, "little")
    content = b"WAVE" + chunk(b"fmt ", fmt, big_endian=False)
    if sound.cues:
        content += chunk(b"cue ", bytes(4), big_endian=False)
    content += chunk(b"data", sound.samples, big_endian=False)
    if sound.loops:
        plays = 0 if sound.loops == _FOREVER else sound.loops
        smpl = b"".join(
            n.to_bytes(4, "little")
            # manufacturer, product, sample period (ns), MIDI unity note and
            # pitch fraction, SMPTE format and offset, one loop, no extra data
            for n in (0, 0, 1_000_000_000 // sound.rate, 60, 0, 0, 0, 1, 0)
        )
        # the loop: its ID, type (forward), start, end (inclusive), fraction, plays
        for n in (0, 0, sound.loop_start, sound.loop_end - 1, 0, plays):
            smpl += n.to_bytes(4, "little")
        content += chunk(b"smpl", smpl, big_endian=False)
    return b"RIFF" + len(content).to_bytes(4, "little") + content


def parse_wav(data: bytes, name: str = "WAV file") -> Sound:
    """A sound from a WAV file, which must be 8-bit PCM (other chunks, such as
    the metadata sound editors add, are ignored)."""
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError(f"{name}: not a WAV file")
    found = dict(chunks(data[: 8 + le32(data, 4)], 12, big_endian=False))
    if b"fmt " not in found or b"data" not in found:
        raise ValueError(f"{name}: no fmt or data chunk")
    fmt = found[b"fmt "]
    if le16(fmt, 0) != _PCM or le16(fmt, 14) != 8 or le16(fmt, 2) not in (1, 2):
        raise ValueError(f"{name}: must be 8-bit PCM, mono or stereo")
    loops = start = end = 0
    if b"smpl" in found and le32(found[b"smpl"], 28):
        smpl = found[b"smpl"]
        if le32(smpl, 28) != 1:
            raise ValueError(f"{name}: the game plays at most one loop")
        start, end, plays = le32(smpl, 44), le32(smpl, 48) + 1, le32(smpl, 56)
        loops = _FOREVER if plays == 0 else plays
    if b"cue " in found and le32(found[b"cue "], 0):
        raise ValueError(f"{name}: cue points aren't supported yet")
    return Sound(
        rate=le32(fmt, 4),
        channels=le16(fmt, 2),
        bits=8,
        samples=found[b"data"],
        loops=loops,
        loop_start=start,
        loop_end=end,
        cues=b"cue " in found,
    )


@dataclass(frozen=True)
class SoundFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        wav = to_wav(parse_mohawk(data))
        if to_mohawk(parse_wav(wav)) != data:
            raise Unconvertible("doesn't round-trip")
        stem.with_suffix(SUFFIX).write_bytes(wav)

    def load(self, stem: Path) -> bytes:
        path = stem.with_suffix(SUFFIX)
        return to_mohawk(parse_wav(path.read_bytes(), str(path)))
