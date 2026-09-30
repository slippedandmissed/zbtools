"""Tables of numbers, stored as TOML: `REGS` word tables, the walk graph
(`NODE`, `PATH`) and `SYSX` MIDI messages.

- `REGS`: big-endian signed words, meaning what their users make of them
  (loadShortTable in decomp/snoids.cpp, the maze's and Lilly's tables:
  shapes' offsets, positions).
- `NODE` (PathNodes in decomp/zoombinis.h): a u16 count, then the points
  Zoombinis' paths join, {x, y}, numbered from 1.
- `PATH` (Paths): a u16 count, then each path's nodes, 24 bytes (0: none).
- `SYSX`: MIDI messages the MIDI map sends a device (by ID from MOHAWK.INI's
  [MidiMap.TargetDeviceInfo]): channel messages, without running status.
"""

import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from pydantic import BaseModel, ConfigDict

from zbtools.formats.base import Unconvertible, be16
from zbtools.mohawk import Resource

SUFFIX = ".toml"
_PER_LINE = 16
_PATH_NODES = 24


def _words(data: bytes) -> list[int]:
    if len(data) % 2:
        raise Unconvertible("an odd number of bytes")
    return [int.from_bytes(data[i : i + 2], "big", signed=True) for i in range(0, len(data), 2)]


def _word_bytes(words: Sequence[int]) -> bytes:
    return b"".join(w.to_bytes(2, "big", signed=True) for w in words)


def _list_lines(name: str, items: Sequence[str], per_line: int) -> list[str]:
    lines = [f"{name} = ["]
    for i in range(0, len(items), per_line):
        lines.append("    " + ", ".join(items[i : i + per_line]) + ",")
    return [*lines, "]"]


def _write(stem: Path, comment: str, lines: list[str]) -> None:
    stem.with_suffix(SUFFIX).write_text("\n".join([f"# {comment}", *lines]) + "\n")


def _read(stem: Path) -> dict[str, object]:
    return tomllib.loads(stem.with_suffix(SUFFIX).read_text())


class WordTable(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    words: list[int]


class Nodes(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    nodes: list[tuple[int, int]]


class Paths(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    paths: list[list[int]]


class Messages(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    messages: list[str]


@dataclass(frozen=True)
class WordTableFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        words = _words(data)
        _write(stem, "A table of words.", _list_lines("words", [str(w) for w in words], _PER_LINE))

    def load(self, stem: Path) -> bytes:
        return _word_bytes(WordTable.model_validate(_read(stem)).words)


@dataclass(frozen=True)
class NodesFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        words = _words(data)
        if not words or words[0] * 2 + 1 != len(words):
            raise Unconvertible("count doesn't match")
        nodes = [f"[{words[i]}, {words[i + 1]}]" for i in range(1, len(words), 2)]
        comment = "The points Zoombinis' paths join, [x, y], numbered from 1."
        _write(stem, comment, _list_lines("nodes", nodes, 8))

    def load(self, stem: Path) -> bytes:
        nodes = Nodes.model_validate(_read(stem)).nodes
        return _word_bytes([len(nodes), *(c for node in nodes for c in node)])


@dataclass(frozen=True)
class PathsFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        count = be16(data, 0)
        if len(data) != 2 + _PATH_NODES * count:
            raise Unconvertible("count doesn't match")
        lines = ["paths = ["]
        for i in range(count):
            nodes = list(data[2 + _PATH_NODES * i : 2 + _PATH_NODES * (i + 1)])
            while nodes and not nodes[-1]:
                nodes.pop()
            lines.append(f"    [{', '.join(str(n) for n in nodes)}],")
        lines.append("]")
        comment = f"The paths Zoombinis walk: the nodes each joins (up to {_PATH_NODES})."
        _write(stem, comment, lines)

    def load(self, stem: Path) -> bytes:
        paths = Paths.model_validate(_read(stem)).paths
        data = bytearray(len(paths).to_bytes(2, "big"))
        for nodes in paths:
            if len(nodes) > _PATH_NODES:
                raise ValueError(f"{stem}: a path joins at most {_PATH_NODES} nodes")
            data += bytes(nodes) + bytes(_PATH_NODES - len(nodes))
        return bytes(data)


def _message_length(status: int) -> int:
    if status < 0x80 or status >= 0xF0:
        raise Unconvertible(f"not a channel message: {status:#x}")
    return 2 if status >> 4 in (0xC, 0xD) else 3


@dataclass(frozen=True)
class MessagesFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        messages = []
        at = 0
        while at < len(data):
            length = _message_length(data[at])
            if at + length > len(data) or any(b >= 0x80 for b in data[at + 1 : at + length]):
                raise Unconvertible("a truncated message")
            messages.append(f'"{data[at : at + length].hex(" ")}"')
            at += length
        _write(stem, "MIDI messages, in hex.", _list_lines("messages", messages, 8))

    def load(self, stem: Path) -> bytes:
        return b"".join(bytes.fromhex(m) for m in Messages.model_validate(_read(stem)).messages)
