"""The movies' video codec, `qb32.qtc`, run under emulation (Unicorn): the
original's decoder, as the ground truth for ours.

QuickTime calls a codec component with a selector; the decoder's two calls
are `PreDecompress` (sizes the codec's storage for the image) and
`BandDecompress` (draws a frame into a pixel buffer, over what the last one
left). `Decoder` makes both calls on the DLL's code, with the few Windows
functions it uses (memory, rectangles, the palette) answered in Python and the
component's storage laid out as its Open call does. The buffer holds rows in
64 KB segments (a 16-bit leftover: 102 rows of 640 bytes, then a 256-byte gap),
which `decode` straightens.

`uv run movie-check` decodes the movies in assets/movies/ with it and compares
every frame with `formats/qkbk.py`'s drawing, and the palette the codec sets
with the frame's.
"""

import struct
from collections.abc import Callable
from concurrent.futures import ProcessPoolExecutor
from functools import partial
from pathlib import Path
from typing import Annotated, NamedTuple

import typer

from zbtools import movies, paths
from zbtools.exe import Executable
from zbtools.formats import qkbk
from zbtools.x86 import Emulator, Register

# Where the codec's handlers for QuickTime's calls are (the dispatcher at
# 0x10008568 jumps to them; `uv run ghidra codec` makes functions of them).
_PRE_DECOMPRESS = 0x10001B70
_BAND_DECOMPRESS = 0x10001DD0

_STUBS = 0x30000000  # where the imported functions are, a slot each
_HEAP, _HEAP_SIZE = 0x20000000, 0x4000000
_STACK, _STACK_SIZE = 0x7F000000, 0x100000
_RETURN = 0x30000800  # a halt, where calls return to
_STORAGE_SIZE = 0x64C0  # the component's per-instance storage
_SAMPLE_LIMIT = 0x400000
_STATIC_PALETTE = 1  # GetSystemPaletteUse's SYSPAL_STATIC
_RESERVED_COLOURS = 10  # the system keeps this many at each end of a static palette
_PALETTE_ENTRIES = 256 - 2 * _RESERVED_COLOURS

RGB = tuple[int, int, int]


class Decoded(NamedTuple):
    pixels: bytes  # the whole image, row by row
    palette: list[RGB | None] | None  # what the codec set: by pixel value, None: the system's


class _Import(NamedTuple):
    arguments: int  # stdcall: the function pops this many words
    behaviour: Callable[[list[int]], int]


def _signed(value: int) -> int:
    return value - (1 << 32) if value >= 1 << 31 else value


class Decoder:
    def __init__(self, dll: Path, size: tuple[int, int]) -> None:
        self.width, self.height = size
        exe = Executable(dll)
        self._cpu = Emulator()
        self._cpu.map(exe.base, (len(exe.image) + 0xFFF) & ~0xFFF)
        self._cpu.write(exe.base, exe.image)
        self._cpu.map(_HEAP, _HEAP_SIZE)
        self._cpu.map(_STACK, _STACK_SIZE)
        self._cpu.map(_STUBS, 0x1000)
        self._cpu.write(_RETURN, b"\xf4")
        self._brk = _HEAP + 0x1000
        self._palettes: list[list[RGB]] = []  # the codec's CreatePalette calls so far
        self._imports = self._link(exe)
        self._cpu.on_execute(self._call_import, _STUBS, _STUBS + 0x7FF)
        self._cpu.set_register(Register.ESP, _STACK + _STACK_SIZE - 0x1000)
        self._open(size)

    # The component's instance, as its Open call makes it

    def _open(self, size: tuple[int, int]) -> None:
        self._storage = self._allocate(_STORAGE_SIZE)
        self._write32(self._storage + 0x64B8, self._allocate(0xFF))  # a file name
        self._write32(self._storage + 0x3114, 1)  # the component instance
        self._cpu.write(self._storage + 0x2F20, b"\xff\xff")  # the last frame: none
        self._cpu.write(self._storage + 0x34, struct.pack("<H", 1000))  # cast items' room
        description = self._allocate(0x100)
        self._cpu.write(description + 0x20, struct.pack("<HH", *size))
        self._cpu.write(description + 0x52, struct.pack("<H", 8))  # the depth
        self._sample = self._allocate(_SAMPLE_LIMIT)
        self._buffer = self._allocate(size[0] * size[1] * 2)
        self._params = self._allocate(0x100)
        for offset, value in (
            (0x04, description),
            (0x08, self._sample),
            (0x24, self._allocate(0x40)),  # the capabilities it fills in
            (0x38, self._buffer),
            (0x74, 5),
            (0x80, 2),  # draw into the buffer
        ):
            self._write32(self._params + offset, value)
        if self._call(_PRE_DECOMPRESS) != 0:
            raise RuntimeError("the codec refused the image")
        self._stride = self._read16(self._storage + 0x1C)
        self._rows = self._read16(self._storage + 0x20)  # rows in a segment
        self._gap = self._read16(self._storage + 0x22)  # bytes between segments

    def decode(self, sample: bytes) -> Decoded:
        """Draws the next frame over the last, and gives the image."""
        if len(sample) > _SAMPLE_LIMIT:
            raise ValueError("sample too big for the emulator")
        self._cpu.write(self._sample, sample)
        self._write32(self._params + 0x70, len(sample))
        self._palettes.clear()
        if self._call(_BAND_DECOMPRESS) != 0:
            raise RuntimeError("the codec failed to decode a frame")
        raw = self._cpu.read(self._buffer, self.height * (self._stride + self._gap))
        rows = []
        for y in range(self.height):
            at = (
                y * self._stride + (y // self._rows) * self._gap if self._rows else y * self._stride
            )
            rows.append(raw[at : at + self.width])
        palette: list[RGB | None] | None = None
        if self._palettes:
            palette = [None] * 256
            for i, colour in enumerate(self._palettes[-1][:_PALETTE_ENTRIES]):
                palette[_RESERVED_COLOURS + i] = colour
        return Decoded(b"".join(rows), palette)

    # Running the DLL's code

    def _call(self, address: int) -> int:
        """Calls one of the handlers (cdecl, arguments: two unused, the
        storage, the parameters) and returns what it returns."""
        esp = self._cpu.register(Register.ESP) - 20
        for i, value in enumerate((_RETURN, 0, 0, self._storage, self._params)):
            self._write32(esp + 4 * i, value)
        self._cpu.set_register(Register.ESP, esp)
        self._cpu.set_register(Register.EBX, 0)  # (the selector, for the dispatcher)
        self._cpu.run(address, _RETURN, 100_000_000)
        self._cpu.set_register(Register.ESP, esp + 20)
        return self._cpu.register(Register.EAX)

    def _call_import(self, address: int) -> None:
        """An imported function is called: do it, and return from it."""
        esp = self._cpu.register(Register.ESP)
        function = self._imports[address]
        arguments = [self._read32(esp + 4 + 4 * i) for i in range(function.arguments)]
        self._cpu.set_register(Register.EAX, function.behaviour(arguments))
        self._cpu.set_register(Register.EIP, self._read32(esp))
        self._cpu.set_register(Register.ESP, esp + 4 + 4 * function.arguments)

    def _link(self, exe: Executable) -> dict[int, _Import]:
        """Points each import at a stub, for the functions this decoder can
        answer; anything else it calls stops the emulation."""
        behaviours: dict[str, _Import] = {
            "GlobalAlloc": _Import(2, lambda a: self._allocate(a[1])),
            "GlobalLock": _Import(1, lambda a: a[0]),  # (a handle is its memory's address)
            "GlobalHandle": _Import(1, lambda a: a[0]),
            "GlobalUnlock": _Import(1, lambda a: 0),
            "GlobalFree": _Import(1, lambda a: 0),
            "IntersectRect": _Import(3, self._intersect_rect),
            "UnionRect": _Import(3, self._union_rect),
            "IsRectEmpty": _Import(1, lambda a: int(self._empty(self._rect(a[0])))),
            "OffsetRect": _Import(3, self._offset_rect),
            "EqualRect": _Import(2, lambda a: int(self._rect(a[0]) == self._rect(a[1]))),
            "SetRect": _Import(5, self._set_rect),
            "GetCursorPos": _Import(1, self._get_cursor_position),
            "PtInRect": _Import(4, self._point_in_rect),
            "GetActiveWindow": _Import(0, lambda a: 1),
            "GetDC": _Import(1, lambda a: 2),
            "ReleaseDC": _Import(2, lambda a: 1),
            "GetSystemPaletteUse": _Import(1, lambda a: _STATIC_PALETTE),
            "CreatePalette": _Import(1, self._create_palette),
            "SelectPalette": _Import(3, lambda a: 0),
            "RealizePalette": _Import(1, lambda a: 256),
            "DeleteObject": _Import(1, lambda a: 1),
        }

        def unsupported(name: str) -> _Import:
            def stop(arguments: list[int]) -> int:
                raise RuntimeError(f"the codec called {name}")

            return _Import(0, stop)

        linked = {}
        for k, (slot, name) in enumerate(sorted(exe.imports.items())):
            stub = _STUBS + 16 * k
            self._cpu.write(stub, b"\xc3")
            self._write32(slot, stub)
            linked[stub] = behaviours.get(name) or unsupported(name)
        return linked

    # What the imports do

    def _allocate(self, size: int) -> int:
        """Zeroed memory (GlobalAlloc with GMEM_ZEROINIT; never reused)."""
        address = (self._brk + 15) & ~15
        self._brk = address + size + 16
        if self._brk > _HEAP + _HEAP_SIZE:
            raise MemoryError("the emulated heap is full")
        return address

    def _rect(self, address: int) -> tuple[int, int, int, int]:
        left, top, right, bottom = struct.unpack("<4i", self._cpu.read(address, 16))
        return left, top, right, bottom

    def _write_rect(self, address: int, rect: tuple[int, int, int, int]) -> None:
        self._cpu.write(address, struct.pack("<4i", *rect))

    @staticmethod
    def _empty(rect: tuple[int, int, int, int]) -> bool:
        return rect[2] <= rect[0] or rect[3] <= rect[1]

    def _intersect_rect(self, a: list[int]) -> int:
        r, s = self._rect(a[1]), self._rect(a[2])
        both = (max(r[0], s[0]), max(r[1], s[1]), min(r[2], s[2]), min(r[3], s[3]))
        empty = self._empty(both)
        self._write_rect(a[0], (0, 0, 0, 0) if empty else both)
        return int(not empty)

    def _union_rect(self, a: list[int]) -> int:
        r, s = self._rect(a[1]), self._rect(a[2])
        if self._empty(r) and self._empty(s):
            self._write_rect(a[0], (0, 0, 0, 0))
            return 0
        if self._empty(r):
            both = s
        elif self._empty(s):
            both = r
        else:
            both = (min(r[0], s[0]), min(r[1], s[1]), max(r[2], s[2]), max(r[3], s[3]))
        self._write_rect(a[0], both)
        return 1

    def _offset_rect(self, a: list[int]) -> int:
        left, top, right, bottom = self._rect(a[0])
        dx, dy = _signed(a[1]), _signed(a[2])
        self._write_rect(a[0], (left + dx, top + dy, right + dx, bottom + dy))
        return 1

    def _set_rect(self, a: list[int]) -> int:
        left, top, right, bottom = (_signed(v) for v in a[1:5])
        self._write_rect(a[0], (left, top, right, bottom))
        return 1

    def _point_in_rect(self, a: list[int]) -> int:
        left, top, right, bottom = self._rect(a[0])
        x, y = _signed(a[1]), _signed(a[2])
        return int(left <= x < right and top <= y < bottom)

    def _get_cursor_position(self, a: list[int]) -> int:
        self._cpu.write(a[0], bytes(8))  # the mouse is at 0, 0
        return 1

    def _create_palette(self, a: list[int]) -> int:
        """Keeps the colours the codec asks the system to realise."""
        (count,) = struct.unpack("<H", self._cpu.read(a[0] + 2, 2))
        raw = self._cpu.read(a[0] + 4, 4 * count)  # red, green, blue, flags
        self._palettes.append([(raw[4 * i], raw[4 * i + 1], raw[4 * i + 2]) for i in range(count)])
        return 3

    # Emulated memory

    def _read32(self, address: int) -> int:
        return self._cpu.read32(address)

    def _read16(self, address: int) -> int:
        return int.from_bytes(self._cpu.read(address, 2), "little")

    def _write32(self, address: int, value: int) -> None:
        self._cpu.write32(address, value)


def check_movie(directory: Path, dll: Path) -> list[str]:
    """Decodes a movie's frames, built from its files in assets/, with the
    original codec and compares them and their palettes with ours: what
    differs (empty: nothing)."""
    manifest, scene = movies.load_movie(directory)
    size = (manifest.container.width, manifest.container.height)
    decoder = Decoder(dll, size)
    problems = []
    palette_changes = 0
    for number, sample in enumerate(qkbk.build_movie(scene), 1):
        decoded = decoder.decode(sample)
        frame = scene.frames[number - 1]
        if decoded.pixels != qkbk.render_frame(scene, number, size).tobytes():
            problems.append(f"frame {number}: the codec draws something else")
        if decoded.palette is None:
            continue
        palette_changes += 1
        cast = scene.casts[frame.palette]
        if not isinstance(cast, qkbk.Palette):
            raise ValueError(f"frame {number}: palette {frame.palette} isn't a palette")
        for index, colour in enumerate(decoded.palette):
            if colour is not None and f"#{bytes(colour).hex()}" != cast.colours[index].lower():
                problems.append(f"frame {number}: the codec sets colour {index} differently")
                break
    expected = sum(
        1
        for before, frame in zip([None, *scene.frames], scene.frames, strict=False)
        if (before is None and frame.palette) or (before and before.palette != frame.palette)
    )
    if palette_changes != expected:
        problems.append(f"the codec set the palette {palette_changes} times, not {expected}")
    return problems


app = typer.Typer(add_completion=False, help=__doc__)


@app.command()
def main(
    names: Annotated[list[str] | None, typer.Argument(help="Movies (default: all)")] = None,
    assets_dir: Annotated[Path, typer.Option(help="The converted resources")] = paths.ASSETS_DIR,
    dll: Annotated[Path, typer.Option(help="The codec (uv run extract-game)")] = (
        paths.GAME32_DIR / "qb32.qtc"
    ),
) -> None:
    """Check that the original codec draws each frame of the movies in assets/movies/ as
    formats/qkbk.py does (about 20 seconds a movie)."""
    if not dll.is_file():
        raise typer.BadParameter(f"{dll} not found (run `uv run extract-game`)")
    directories = movies.movie_directories(assets_dir)
    if names:
        directories = [d for d in directories if d.name in names]
        if len(directories) != len(set(names)):
            raise typer.BadParameter(f"not all of {', '.join(names)} are in {assets_dir}/movies")
    failed = 0
    with ProcessPoolExecutor() as pool:
        results = pool.map(partial(check_movie, dll=dll), directories)
        for directory, problems in zip(directories, results, strict=True):
            if problems:
                failed += 1
                print(f"{directory.name}: {len(problems)} problems")
                print("\n".join(f"  {p}" for p in problems[:20]))
            else:
                print(f"{directory.name}: every frame and palette matches the original codec")
    if failed:
        raise typer.Exit(1)
