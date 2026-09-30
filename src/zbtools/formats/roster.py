"""`Zoombini.who`, the list of saved games (SavedGameList in decomp/zoombinis.h),
stored as TOML.

Little-endian `short` version (107), next id and count, then 50 slots of a
23-byte name and a 9-byte file name (without ".txt"), each NUL-padded. Slots
the list doesn't reach are empty; a file with anything else in it (text after
a NUL, a different size) is kept as it is.
"""

import json
import tomllib
from pathlib import Path

from pydantic import BaseModel, ConfigDict

from zbtools.formats.base import Unconvertible, le16

SUFFIX = ".toml"
SLOTS = 50
_NAME = 23
_FILE = 9
_SLOT = _NAME + _FILE
_HEADER = 6
SIZE = _HEADER + SLOTS * _SLOT


class SavedGame(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    name: str
    file: str


class Roster(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    version: int
    next_id: int
    count: int
    games: list[SavedGame] = []


def _text(field: bytes) -> str:
    text, _nul, rest = field.partition(b"\0")
    if any(rest):
        raise Unconvertible("text after a string's end")
    try:
        return text.decode("ascii")
    except UnicodeDecodeError as e:
        raise Unconvertible("non-ASCII text") from e


def _field(text: str, size: int) -> bytes:
    raw = text.encode("ascii")
    if len(raw) >= size:
        raise ValueError(f"{text!r} doesn't fit in {size - 1} characters")
    return raw.ljust(size, b"\0")


def to_toml(data: bytes) -> str:
    """The saved-game list as TOML; raises Unconvertible for a file it
    couldn't give back."""
    if len(data) != SIZE:
        raise Unconvertible(f"{len(data)} bytes, not {SIZE}")
    games = []
    for i in range(SLOTS):
        slot = data[_HEADER + i * _SLOT : _HEADER + (i + 1) * _SLOT]
        games.append((_text(slot[:_NAME]), _text(slot[_NAME:])))
    while games and games[-1] == ("", ""):
        games.pop()
    lines = [
        "# The saved games (Zoombini.who): each one's name and its file's name, without .txt.",
        f"version = {le16(data, 0)}",
        f"next_id = {le16(data, 2)}",
        f"count = {le16(data, 4)}",
    ]
    for name, file in games:
        lines += ["", "[[games]]", f"name = {json.dumps(name)}", f"file = {json.dumps(file)}"]
    return "\n".join(lines) + "\n"


def from_toml(text: str) -> bytes:
    roster = Roster.model_validate(tomllib.loads(text))
    if len(roster.games) > SLOTS:
        raise ValueError(f"at most {SLOTS} saved games")
    data = b"".join(n.to_bytes(2, "little") for n in (roster.version, roster.next_id, roster.count))
    for game in roster.games:
        data += _field(game.name, _NAME) + _field(game.file, _FILE)
    return data.ljust(SIZE, b"\0")


def converted(path: Path) -> Path:
    """Where the TOML of the file `path` goes: Zoombini.who.toml."""
    return path.with_name(path.name + SUFFIX)


def save(data: bytes, path: Path) -> None:
    converted(path).write_text(to_toml(data))


def load(path: Path) -> bytes:
    return from_toml(converted(path).read_text())
