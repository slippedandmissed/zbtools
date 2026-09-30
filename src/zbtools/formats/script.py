"""Scripts (`SCRB` feature scripts, `SCRS` Zoombini scripts), stored as TOML.

A script (as the views step through them, decomp/features.cpp and
scriptFrameOffset in decomp/view.cpp) is big-endian signed words: the number
of frames, for a Zoombini script the way it faces, then the frames. A frame
is the cels it draws, each an image of the view's bank (from 1; 0 draws
nothing) and its x and y, then an end word: 0xff00 plus an event (0: none) to
tell the view's owner, or 0xfe00 plus an event followed by a sound to play.
"""

import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from pydantic import BaseModel, ConfigDict

from zbtools.formats.base import Unconvertible
from zbtools.mohawk import Resource

SUFFIX = ".toml"
_END = 0xFF00
_END_SOUND = 0xFE00


class Frame(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    cels: list[tuple[int, int, int]]  # image, x, y
    event: int = 0
    sound: int | None = None


class Script(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    facing: int | None = None  # Zoombini scripts only
    frames: list[Frame]


def parse(data: bytes, zoombini: bool) -> Script:
    if len(data) % 2:
        raise Unconvertible("an odd number of bytes")
    words = [int.from_bytes(data[i : i + 2], "big", signed=True) for i in range(0, len(data), 2)]
    at = 2 if zoombini else 1
    frames: list[Frame] = []
    cels: list[tuple[int, int, int]] = []
    try:
        while at < len(words):
            word = words[at]
            if word >= 0:
                cels.append((word, words[at + 1], words[at + 2]))
                at += 3
                continue
            end = word & 0xFFFF
            if end & 0xFF00 == _END:
                frames.append(Frame(cels=cels, event=end & 0xFF))
                at += 1
            elif end & 0xFF00 == _END_SOUND:
                frames.append(Frame(cels=cels, event=end & 0xFF, sound=words[at + 1]))
                at += 2
            else:
                raise Unconvertible(f"unexpected word {end:#06x}")
            cels = []
    except IndexError as e:
        raise Unconvertible("truncated") from e
    if cels or len(frames) != words[0]:
        raise Unconvertible("frames don't match the count")
    return Script(facing=words[1] if zoombini else None, frames=frames)


def script_bytes(script: Script, zoombini: bool) -> bytes:
    if (script.facing is not None) != zoombini:
        raise ValueError("Zoombini scripts, and only they, have a facing")
    words = [len(script.frames)]
    if script.facing is not None:
        words.append(script.facing)
    for frame in script.frames:
        for cel in frame.cels:
            words += cel
        if not 0 <= frame.event <= 0xFF:
            raise ValueError(f"event {frame.event} isn't 0-255")
        if frame.sound is None:
            words.append((_END | frame.event) - 0x10000)
        else:
            words += [(_END_SOUND | frame.event) - 0x10000, frame.sound]
    return b"".join(w.to_bytes(2, "big", signed=True) for w in words)


def _toml(script: Script, zoombini: bool) -> str:
    what = "A Zoombini's script" if zoombini else "A feature script"
    lines = [
        f"# {what}: its frames, each the cels drawn ([image, x, y], from the view's",
        "# bank; image 0 draws nothing), the event it tells the view's owner of, and a",
        "# sound to play.",
    ]
    if script.facing is not None:
        lines.append(f"facing = {script.facing}")
    lines.append("frames = [")
    for frame in script.frames:
        cels = ", ".join(f"[{i}, {x}, {y}]" for i, x, y in frame.cels)
        fields = [f"cels = [{cels}]"]
        if frame.event:
            fields.append(f"event = {frame.event}")
        if frame.sound is not None:
            fields.append(f"sound = {frame.sound}")
        lines.append(f"    {{ {', '.join(fields)} }},")
    lines.append("]")
    return "\n".join(lines) + "\n"


@dataclass(frozen=True)
class ScriptFormat:
    zoombini: bool  # SCRS: with a facing

    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        script = parse(data, self.zoombini)
        if script_bytes(script, self.zoombini) != data:
            raise Unconvertible("doesn't round-trip")
        stem.with_suffix(SUFFIX).write_text(_toml(script, self.zoombini))

    def load(self, stem: Path) -> bytes:
        text = stem.with_suffix(SUFFIX).read_text()
        return script_bytes(Script.model_validate(tomllib.loads(text)), self.zoombini)
