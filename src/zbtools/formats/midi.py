"""MIDI (`tMID`): Mohawk MIDI files, stored as standard MIDI files.

A Mohawk MIDI file is `MHWK`, a size, `MIDI`, then a standard MIDI file's
chunks (`MThd`, `MTrk`) padded to even lengths, with a `Prg#` chunk after
`MThd`: a u16 count, then {u16 program, u16 mask of the channels it's used
on} for each program the tracks change to, in order. The size is that of the
rest of the file without the last chunk's padding.

`Prg#` is derived from the tracks, so the .mid file leaves it out and packing
recomputes it, keeping it right when the music is edited.
"""

import io
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

import mido

from zbtools.formats.base import Unconvertible, be32, chunk, chunks
from zbtools.mohawk import Resource

SUFFIX = ".mid"


def programs(smf: bytes) -> bytes:
    """The `Prg#` chunk's body for a standard MIDI file."""
    masks: dict[int, int] = {}
    for track in mido.MidiFile(file=io.BytesIO(smf)).tracks:
        for message in track:
            if message.type == "program_change":
                program: int = message.program
                channel: int = message.channel
                masks[program] = masks.get(program, 0) | 1 << channel
    body = len(masks).to_bytes(2, "big")
    for program, mask in sorted(masks.items()):
        body += program.to_bytes(2, "big") + mask.to_bytes(2, "big")
    return body


def to_mohawk(smf: bytes) -> bytes:
    found = chunks(smf, 0, big_endian=True, padded=False)
    if not found or found[0][0] != b"MThd" or any(t != b"MTrk" for t, _ in found[1:]):
        raise ValueError("expected a MIDI header then tracks")
    content = b"MIDI" + chunk(b"MThd", found[0][1], big_endian=True)
    content += chunk(b"Prg#", programs(smf), big_endian=True)
    for _, track in found[1:]:
        content += chunk(b"MTrk", track, big_endian=True)
    size = len(content) - (len(found[-1][1]) & 1)
    return b"MHWK" + size.to_bytes(4, "big") + content


def to_smf(data: bytes) -> bytes:
    if data[:4] != b"MHWK" or data[8:12] != b"MIDI":
        raise Unconvertible("not a Mohawk MIDI file")
    found = chunks(data, 12, big_endian=True)
    if [t for t, _ in found[:2]] != [b"MThd", b"Prg#"]:
        raise Unconvertible("expected MThd then Prg#")
    if be32(data, 4) > len(data) - 8:
        raise Unconvertible("size runs past the end")
    return b"".join(
        chunk(t, body, big_endian=True, padded=False) for t, body in found if t != b"Prg#"
    )


@dataclass(frozen=True)
class MidiFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        smf = to_smf(data)
        try:
            same = to_mohawk(smf) == data
        except (ValueError, EOFError, OSError) as e:
            raise Unconvertible(str(e)) from e
        if not same:
            raise Unconvertible("doesn't round-trip")
        stem.with_suffix(SUFFIX).write_bytes(smf)

    def load(self, stem: Path) -> bytes:
        return to_mohawk(stem.with_suffix(SUFFIX).read_bytes())
