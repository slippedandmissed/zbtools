"""Palettes (`tPAL`) and shape lists (`SHPL`), stored as TOML.

A palette (as applyPaletteResource reads it, decomp/e2memory.cpp) is a
big-endian u16 first colour and count, then Windows PALETTEENTRYs: red, green,
blue and flags (1: PC_RESERVED, which every real palette of the game's has on
all its colours). A shape list is the ID of its first shape (a `tBMP`), how
many there are, then a palette.

Palette file formats (GIMP's .gpl, JASC .pal) have no place for the first
colour or the flags, so these are TOML, the colours as hex strings.
"""

import re
import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from pydantic import BaseModel, ConfigDict, field_validator

from zbtools.formats.base import Unconvertible, be16
from zbtools.mohawk import Resource

SUFFIX = ".toml"
_PER_LINE = 8


class Palette(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    first: int
    flags: int
    colours: list[str]

    @field_validator("colours")
    @classmethod
    def _hex(cls, colours: list[str]) -> list[str]:
        for c in colours:
            if not re.fullmatch(r"#[0-9a-fA-F]{6}", c):
                raise ValueError(f"colour {c!r} isn't #rrggbb")
        return colours


class ShapeList(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    first_shape: int
    shapes: int
    palette: Palette


def parse_palette(data: bytes) -> Palette:
    first, count = be16(data, 0), be16(data, 2)
    if len(data) != 4 + 4 * count:
        raise Unconvertible("size doesn't match the count")
    entries = [data[4 + 4 * i : 8 + 4 * i] for i in range(count)]
    flags = {e[3] for e in entries}
    if len(flags) > 1:
        raise Unconvertible("colours with different flags")
    return Palette(
        first=first, flags=flags.pop() if flags else 0, colours=[f"#{e[:3].hex()}" for e in entries]
    )


def palette_bytes(palette: Palette) -> bytes:
    data = palette.first.to_bytes(2, "big") + len(palette.colours).to_bytes(2, "big")
    for c in palette.colours:
        data += bytes.fromhex(c[1:]) + bytes([palette.flags])
    return data


def _palette_toml(palette: Palette, table: str = "") -> list[str]:
    lines = [f"[{table}]"] if table else []
    lines += [
        f"first = {palette.first}  # the first colour's index",
        f"flags = {palette.flags}  # every colour's (1: PC_RESERVED)",
        "colours = [",
    ]
    for i in range(0, len(palette.colours), _PER_LINE):
        row = ", ".join(f'"{c.lower()}"' for c in palette.colours[i : i + _PER_LINE])
        lines.append(f"    {row},")
    lines.append("]")
    return lines


@dataclass(frozen=True)
class PaletteFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        palette = parse_palette(data)
        if palette_bytes(palette) != data:
            raise Unconvertible("doesn't round-trip")
        lines = ["# A palette: colours from `first` on.", *_palette_toml(palette)]
        stem.with_suffix(SUFFIX).write_text("\n".join(lines) + "\n")

    def load(self, stem: Path) -> bytes:
        text = stem.with_suffix(SUFFIX).read_text()
        return palette_bytes(Palette.model_validate(tomllib.loads(text)))


@dataclass(frozen=True)
class ShapeListFormat:
    def save(self, data: bytes, stem: Path, archive: Sequence[Resource]) -> None:
        shapes = ShapeList(
            first_shape=be16(data, 0), shapes=be16(data, 2), palette=parse_palette(data[4:])
        )
        if self._bytes(shapes) != data:
            raise Unconvertible("doesn't round-trip")
        lines = [
            "# A shape list: its shapes are tBMP resources first_shape on, and it sets",
            "# the palette's colours.",
            f"first_shape = {shapes.first_shape}",
            f"shapes = {shapes.shapes}",
            "",
            *_palette_toml(shapes.palette, "palette"),
        ]
        stem.with_suffix(SUFFIX).write_text("\n".join(lines) + "\n")

    def load(self, stem: Path) -> bytes:
        text = stem.with_suffix(SUFFIX).read_text()
        return self._bytes(ShapeList.model_validate(tomllib.loads(text)))

    @staticmethod
    def _bytes(shapes: ShapeList) -> bytes:
        head = shapes.first_shape.to_bytes(2, "big") + shapes.shapes.to_bytes(2, "big")
        return head + palette_bytes(shapes.palette)
