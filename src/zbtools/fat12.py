"""Minimal FAT12 editing for floppy images: read, replace and delete files in
the root directory. Enough to customize a boot floppy; subdirectories and long
file names are not supported.
"""

import struct
from collections.abc import Iterator

_EOC = 0xFFF  # end-of-chain marker written for new chains
_FIXED_DATE = (1999 - 1980) << 9 | 5 << 5 | 5  # 1999-05-05, keeps images reproducible
_DELETED = 0xE5
_ATTR_VOLUME = 0x08
_ATTR_ARCHIVE = 0x20


def _u16(buf: bytes | bytearray, off: int) -> int:
    return int.from_bytes(buf[off : off + 2], "little")


def _u32(buf: bytes | bytearray, off: int) -> int:
    return int.from_bytes(buf[off : off + 4], "little")


def _name83(name: str) -> bytes:
    base, _, ext = name.upper().partition(".")
    if not 1 <= len(base) <= 8 or len(ext) > 3:
        raise ValueError(f"not an 8.3 file name: {name}")
    return (base.ljust(8) + ext.ljust(3)).encode("ascii")


class Fat12Image:
    def __init__(self, data: bytes) -> None:
        self.img = bytearray(data)
        bps = _u16(self.img, 11)
        spc = self.img[13]
        rsv = _u16(self.img, 14)
        self.nfats = self.img[16]
        self.nroot = _u16(self.img, 17)
        total = _u16(self.img, 19) or _u32(self.img, 32)
        self.spf = _u16(self.img, 22)

        self.fat_off = rsv * bps
        self.fat_len = self.spf * bps
        self.root_off = self.fat_off + self.nfats * self.fat_len
        self.data_off = self.root_off + -(-self.nroot * 32 // bps) * bps
        self.csize = bps * spc
        nclusters = (total * bps - self.data_off) // self.csize
        if nclusters >= 4085:
            raise ValueError("not a FAT12 image")
        fat = self.img[self.fat_off : self.fat_off + self.fat_len]
        self.fat = [self._get(fat, n) for n in range(nclusters + 2)]

    @staticmethod
    def _get(fat: bytearray, n: int) -> int:
        v = _u16(fat, n * 3 // 2)
        return v >> 4 if n & 1 else v & 0xFFF

    def _flush_fat(self) -> None:
        fat = bytearray(self.img[self.fat_off : self.fat_off + self.fat_len])
        for n, v in enumerate(self.fat):
            i = n * 3 // 2
            if n & 1:
                fat[i] = (fat[i] & 0x0F) | (v << 4 & 0xF0)
                fat[i + 1] = v >> 4
            else:
                fat[i] = v & 0xFF
                fat[i + 1] = (fat[i + 1] & 0xF0) | v >> 8
        for k in range(self.nfats):
            off = self.fat_off + k * self.fat_len
            self.img[off : off + self.fat_len] = fat

    def _entries(self) -> Iterator[int]:
        """Offsets of root directory entries up to the end-of-directory marker."""
        for i in range(self.nroot):
            off = self.root_off + i * 32
            if self.img[off] == 0:
                return
            yield off

    def _is_file(self, off: int) -> bool:
        return self.img[off] != _DELETED and not self.img[off + 11] & _ATTR_VOLUME

    def _find(self, name: str) -> int | None:
        key = _name83(name)
        for off in self._entries():
            if self._is_file(off) and self.img[off : off + 11] == key:
                return off
        return None

    def _chain(self, cluster: int) -> Iterator[int]:
        while 2 <= cluster < 0xFF8:
            yield cluster
            cluster = self.fat[cluster]

    def listdir(self) -> list[str]:
        names = []
        for off in self._entries():
            if self._is_file(off):
                base = self.img[off : off + 8].decode().rstrip()
                ext = self.img[off + 8 : off + 11].decode().rstrip()
                names.append(f"{base}.{ext}" if ext else base)
        return names

    def read(self, name: str) -> bytes:
        off = self._find(name)
        if off is None:
            raise FileNotFoundError(name)
        out = bytearray()
        for c in self._chain(_u16(self.img, off + 26)):
            start = self.data_off + (c - 2) * self.csize
            out += self.img[start : start + self.csize]
        return bytes(out[: _u32(self.img, off + 28)])

    def remove(self, name: str) -> None:
        off = self._find(name)
        if off is None:
            raise FileNotFoundError(name)
        for c in list(self._chain(_u16(self.img, off + 26))):
            self.fat[c] = 0
        self.img[off] = _DELETED
        self._flush_fat()

    def write(self, name: str, data: bytes) -> None:
        """Create or replace a file in the root directory."""
        if self._find(name) is not None:
            self.remove(name)
        need = -(-len(data) // self.csize)
        free = [c for c in range(2, len(self.fat)) if self.fat[c] == 0][:need]
        if len(free) < need:
            raise OSError(f"not enough free space on floppy for {name}")
        slots = (self.root_off + i * 32 for i in range(self.nroot))
        slot = next((off for off in slots if self.img[off] in (0, _DELETED)), None)
        if slot is None:
            raise OSError("root directory is full")

        for i, c in enumerate(free):
            self.fat[c] = free[i + 1] if i + 1 < len(free) else _EOC
            chunk = data[i * self.csize : (i + 1) * self.csize]
            start = self.data_off + (c - 2) * self.csize
            self.img[start : start + self.csize] = chunk.ljust(self.csize, b"\0")
        self._flush_fat()

        entry = bytearray(32)
        entry[0:11] = _name83(name)
        entry[11] = _ATTR_ARCHIVE
        # creation date, access date, write time, write date, first cluster, size
        struct.pack_into("<HH", entry, 16, _FIXED_DATE, _FIXED_DATE)
        struct.pack_into("<HHHI", entry, 22, 0, _FIXED_DATE, free[0] if free else 0, len(data))
        self.img[slot : slot + 32] = entry

    def free_bytes(self) -> int:
        return sum(1 for v in self.fat[2:] if v == 0) * self.csize
