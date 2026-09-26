"""Extract an InstallShield 3 ".Z" archive (e.g. ZBARCHIV.Z on the game disc).

Files in these archives are compressed with PKWARE DCL "implode"; the
decompressor below is a port of zlib's contrib/blast/blast.c.
"""
import argparse
import os
import struct
from datetime import datetime

MAGIC = 0x8C655D13


# --- PKWARE DCL explode -------------------------------------------------------

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
MAXBITS = 13


def _huffman(rep):
    """Build (count, symbol) canonical Huffman tables from blast's compact form."""
    lengths = []
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
    return count, symbol


LITCODE = _huffman(LITLEN)
LENCODE = _huffman(LENLEN)
DISTCODE = _huffman(DISTLEN)


class _Bits:
    def __init__(self, data):
        self.data = data
        self.pos = 0
        self.buf = 0
        self.cnt = 0

    def bits(self, need):
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

    def decode(self, table):
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


def explode(data):
    s = _Bits(data)
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

def _dos_datetime(date, time):
    try:
        return datetime(1980 + (date >> 9), (date >> 5) & 15, date & 31,
                        time >> 11, (time >> 5) & 63, (time & 31) * 2)
    except ValueError:
        return None


def read_archive(data):
    magic, = struct.unpack_from("<I", data, 0)
    if magic != MAGIC:
        raise ValueError("not an InstallShield 3 archive")
    file_count, = struct.unpack_from("<H", data, 0x0C)
    toc, = struct.unpack_from("<I", data, 0x29)
    dir_count, = struct.unpack_from("<H", data, 0x31)

    pos = toc
    dirs = []
    for _ in range(dir_count):
        _, chunk, name_len = struct.unpack_from("<HHH", data, pos)
        dirs.append(data[pos + 6:pos + 6 + name_len].decode("latin-1"))
        pos += chunk

    files = []
    for _ in range(file_count):
        dir_index, usize, csize, offset, date, time, _, chunk = \
            struct.unpack_from("<xHIIIHHIH", data, pos)
        name_len = data[pos + 0x1D]
        name = data[pos + 0x1E:pos + 0x1E + name_len].decode("latin-1")
        files.append(dict(dir=dirs[dir_index], name=name, usize=usize,
                          csize=csize, offset=offset,
                          mtime=_dos_datetime(date, time)))
        pos += chunk
    return files


def extract(data, outdir=None):
    """List the archive's files, extracting them into outdir if given."""
    for f in read_archive(data):
        path = os.path.join(f["dir"].replace("\\", "/"), f["name"])
        print(f"{f['usize']:>9} {str(f['mtime'] or '?'):>19}  {path}")
        if outdir:
            raw = explode(data[f["offset"]:f["offset"] + f["csize"]])
            if len(raw) != f["usize"]:
                raise ValueError(f"{path}: size {len(raw)} != {f['usize']}")
            dest = os.path.join(outdir, path)
            os.makedirs(os.path.dirname(dest) or ".", exist_ok=True)
            with open(dest, "wb") as out:
                out.write(raw)
            if f["mtime"]:
                ts = f["mtime"].timestamp()
                os.utime(dest, (ts, ts))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("archive", help="InstallShield 3 .Z archive")
    parser.add_argument("outdir", nargs="?",
                        help="extract into this directory (list only if omitted)")
    args = parser.parse_args()
    with open(args.archive, "rb") as f:
        extract(f.read(), args.outdir)


if __name__ == "__main__":
    main()
