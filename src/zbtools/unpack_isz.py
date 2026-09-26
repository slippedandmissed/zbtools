"""Extract an InstallShield 3 ".Z" archive (e.g. ZBARCHIV.Z on the game disc).

Files in these archives are compressed with PKWARE DCL "implode"; the
decompressor below is a port of zlib's contrib/blast/blast.c. It's hand-written
because no maintained Python package provides it: `dclimplode` has no wheels
for current Python versions and `pwexplode` isn't on PyPI. No library reads the
InstallShield 3 archive format itself either.
"""

import os
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path, PureWindowsPath
from typing import Annotated, NamedTuple

import typer

MAGIC = 0x8C655D13


# --- PKWARE DCL explode -------------------------------------------------------

# Code-length tables from blast.c, in its compact run-length form.
# fmt: off
LITLEN = bytes([
    11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
    9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
    7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
    8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
    44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
    44, 173])
LENLEN = bytes([2, 35, 36, 53, 38, 23])
DISTLEN = bytes([2, 20, 53, 230, 247, 151, 248])
LEN_BASE = [3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264]
LEN_EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8]
# fmt: on
MAXBITS = 13


class Huffman(NamedTuple):
    """Canonical Huffman decoding table."""

    counts: list[int]  # number of codes of each bit length
    symbols: list[int]  # symbols ordered by code


def _huffman(rep: bytes) -> Huffman:
    """Build a decoding table from blast's compact code-length representation."""
    lengths: list[int] = []
    for b in rep:
        lengths += [b & 15] * ((b >> 4) + 1)
    count = [0] * (MAXBITS + 1)
    for n in lengths:
        count[n] += 1
    offs = [0] * (MAXBITS + 1)
    for n in range(1, MAXBITS):
        offs[n + 1] = offs[n] + count[n]
    symbol = [0] * len(lengths)
    for sym, n in enumerate(lengths):
        if n:
            symbol[offs[n]] = sym
            offs[n] += 1
    return Huffman(count, symbol)


LITCODE = _huffman(LITLEN)
LENCODE = _huffman(LENLEN)
DISTCODE = _huffman(DISTLEN)


class _BitReader:
    def __init__(self, data: bytes) -> None:
        self.data = data
        self.pos = 0
        self.buf = 0
        self.cnt = 0

    def bits(self, need: int) -> int:
        while self.cnt < need:
            if self.pos >= len(self.data):
                raise ValueError("unexpected end of compressed data")
            self.buf |= self.data[self.pos] << self.cnt
            self.pos += 1
            self.cnt += 8
        val = self.buf & ((1 << need) - 1)
        self.buf >>= need
        self.cnt -= need
        return val

    def decode(self, table: Huffman) -> int:
        count, symbol = table
        code = first = index = 0
        for n in range(1, MAXBITS + 1):
            code |= self.bits(1) ^ 1  # codes are stored bit-inverted
            if code < first + count[n]:
                return symbol[index + code - first]
            index += count[n]
            first = (first + count[n]) << 1
            code <<= 1
        raise ValueError("invalid Huffman code")


def explode(data: bytes) -> bytes:
    """Decompress a PKWARE DCL "implode" stream."""
    s = _BitReader(data)
    lit = s.bits(8)
    dict_bits = s.bits(8)
    if lit > 1 or not 4 <= dict_bits <= 6:
        raise ValueError(f"bad DCL header lit={lit} dict={dict_bits}")
    out = bytearray()
    while True:
        if s.bits(1):
            sym = s.decode(LENCODE)
            length = LEN_BASE[sym] + s.bits(LEN_EXTRA[sym])
            if length == 519:  # end of stream
                break
            shift = 2 if length == 2 else dict_bits
            dist = (s.decode(DISTCODE) << shift) + s.bits(shift) + 1
            if dist > len(out):
                raise ValueError("distance too far back")
            start = len(out) - dist
            for i in range(length):
                out.append(out[start + i])
        else:
            out.append(s.decode(LITCODE) if lit else s.bits(8))
    return bytes(out)


# --- InstallShield 3 archive --------------------------------------------------


def _dos_datetime(date: int, time: int) -> datetime | None:
    try:
        return datetime(
            1980 + (date >> 9),
            (date >> 5) & 15,
            date & 31,
            time >> 11,
            (time >> 5) & 63,
            (time & 31) * 2,
        )
    except ValueError:
        return None


def _u16(data: bytes, off: int) -> int:
    return int.from_bytes(data[off : off + 2], "little")


def _u32(data: bytes, off: int) -> int:
    return int.from_bytes(data[off : off + 4], "little")


@dataclass(frozen=True)
class ArchiveEntry:
    path: str  # relative path with "/" separators
    size: int
    compressed_size: int
    offset: int  # of the compressed data within the archive
    mtime: datetime | None


def read_archive(data: bytes) -> list[ArchiveEntry]:
    """Parse the archive's table of contents."""
    if _u32(data, 0) != MAGIC:
        raise ValueError("not an InstallShield 3 archive")
    file_count = _u16(data, 0x0C)
    toc = _u32(data, 0x29)
    dir_count = _u16(data, 0x31)

    pos = toc
    dirs: list[str] = []
    for _ in range(dir_count):
        chunk, name_len = _u16(data, pos + 2), _u16(data, pos + 4)
        dirs.append(data[pos + 6 : pos + 6 + name_len].decode("latin-1"))
        pos += chunk

    entries: list[ArchiveEntry] = []
    for _ in range(file_count):
        name_len = data[pos + 0x1D]
        name = data[pos + 0x1E : pos + 0x1E + name_len].decode("latin-1")
        directory = dirs[_u16(data, pos + 0x01)]
        entries.append(
            ArchiveEntry(
                path=PureWindowsPath(directory, name).as_posix(),
                size=_u32(data, pos + 0x03),
                compressed_size=_u32(data, pos + 0x07),
                offset=_u32(data, pos + 0x0B),
                mtime=_dos_datetime(_u16(data, pos + 0x0F), _u16(data, pos + 0x11)),
            )
        )
        pos += _u16(data, pos + 0x17)
    return entries


def extract(data: bytes, outdir: Path | None = None) -> None:
    """List the archive's files, extracting them into outdir if given."""
    for entry in read_archive(data):
        print(f"{entry.size:>9} {entry.mtime or '?'!s:>19}  {entry.path}")
        if outdir is None:
            continue
        raw = explode(data[entry.offset : entry.offset + entry.compressed_size])
        if len(raw) != entry.size:
            raise ValueError(f"{entry.path}: size {len(raw)} != {entry.size}")
        dest = outdir / entry.path
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(raw)
        if entry.mtime:
            ts = entry.mtime.timestamp()
            os.utime(dest, (ts, ts))


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    archive: Annotated[Path, typer.Argument(help="InstallShield 3 .Z archive")],
    outdir: Annotated[
        Path | None,
        typer.Argument(
            help="Extract into this directory (list only if omitted)", show_default=False
        ),
    ] = None,
) -> None:
    extract(archive.read_bytes(), outdir)
