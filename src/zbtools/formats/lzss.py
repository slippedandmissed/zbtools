"""Mohawk's LZ compression (LZSS), as the engine decompresses it
(lzDecompress, decomp/decompressimage.cpp) and as Broderbund's tools
compressed it: Haruhiko Okumura's LZSS.C (1989), with a ring of 1 << `bits`
bytes (the game's images all use 10) and matches of 3 to (1 << (16 - bits)) + 2
bytes, which reproduces every compressed image of the game byte for byte.

A stream is groups of a flag byte, low bit first, and eight items: for a 1 a
literal byte, for a 0 a big-endian word, its low `bits` bits a position in the
ring and the rest the match's length less 3. The ring starts zeroed, with
writing at (1 << bits) - ((1 << (16 - bits)) + 2).

Okumura's encoder finds the longest match through binary search trees of the
ring's strings; the choice between equally long matches depends on the trees'
shape, so this is a faithful port rather than any encoder producing valid
output. It never reinitialises the lookahead beyond a short input, where the
original tool had non-zero data (from whatever it compressed before): that
decides one match in the game (TOWN.MHK's tBMP 1200), and any non-zero byte
there but those of the input reproduces it, so this fills it with 0xff.

Hand-written: no library implements this variant (the zero-filled ring and
big-endian words differ from other LZSS formats).
"""

_STALE = 0xFF


def _limits(bits: int) -> tuple[int, int]:
    """The ring's size and the longest match."""
    return 1 << bits, (1 << (16 - bits)) + 2


def decompress(data: bytes, size: int, bits: int) -> bytes:
    ring_size, longest = _limits(bits)
    mask = ring_size - 1
    ring = bytearray(ring_size)
    position = ring_size - longest
    out = bytearray()
    at = 0
    flags = 0
    try:
        while len(out) < size:
            if not flags & 0xFF00:
                flags = data[at] | 0xFF00
                at += 1
            if flags & 1:
                byte = data[at]
                at += 1
                out.append(byte)
                ring[position] = byte
                position = (position + 1) & mask
            else:
                word = data[at] << 8 | data[at + 1]
                at += 2
                source = word & mask
                for _ in range(min((word >> bits) + 3, size - len(out))):
                    byte = ring[source]
                    source = (source + 1) & mask
                    ring[position] = byte
                    position = (position + 1) & mask
                    out.append(byte)
            flags >>= 1
    except IndexError as e:
        raise ValueError("compressed data ends early") from e
    return bytes(out)


def compress(data: bytes, bits: int) -> bytes:  # noqa: PLR0915
    """Okumura's Encode(): kept as one function, its trees in local lists,
    since that runs much faster in Python than objects would."""
    n, f = _limits(bits)
    nil = n
    # The ring, with a copy of its first f - 1 bytes after it so strings
    # don't wrap; the trees: each ring position's children and parent, and
    # the roots, one per first byte (rson[n + 1 + byte]).
    text = bytearray(n + f - 1)
    lson = [nil] * (n + 1)
    rson = [nil] * (n + 257)
    dad = [nil] * (n + 1)
    match_length = 0
    match_position = 0

    def insert(r: int) -> None:  # noqa: PLR0912 (a faithful port)
        nonlocal match_length, match_position
        cmp = 1
        p = n + 1 + text[r]
        rson[r] = lson[r] = nil
        match_length = 0
        second = text[r + 1]
        key = b""
        while True:
            if cmp >= 0:
                if rson[p] == nil:
                    rson[p] = r
                    dad[r] = p
                    return
                p = rson[p]
            else:
                if lson[p] == nil:
                    lson[p] = r
                    dad[r] = p
                    return
                p = lson[p]
            cmp = second - text[p + 1]
            if cmp:
                i = 1  # most strings differ here: skip the slow comparison
            else:
                # The first difference, found by comparing the strings as numbers.
                if not key:
                    key = bytes(text[r + 1 : r + f])
                diff = int.from_bytes(key, "big") ^ int.from_bytes(text[p + 1 : p + f], "big")
                if diff:
                    i = (len(key) * 8 - diff.bit_length()) // 8
                    cmp = key[i] - text[p + 1 + i]
                    i += 1
                else:
                    i = f
            if i > match_length:
                match_position = p
                match_length = i
                if i >= f:
                    break
        dad[r] = dad[p]
        lson[r] = lson[p]
        rson[r] = rson[p]
        dad[lson[p]] = r
        dad[rson[p]] = r
        if rson[dad[p]] == p:
            rson[dad[p]] = r
        else:
            lson[dad[p]] = r
        dad[p] = nil

    def delete(p: int) -> None:
        if dad[p] == nil:
            return
        if rson[p] == nil:
            q = lson[p]
        elif lson[p] == nil:
            q = rson[p]
        else:
            q = lson[p]
            if rson[q] != nil:
                while rson[q] != nil:
                    q = rson[q]
                rson[dad[q]] = lson[q]
                dad[lson[q]] = dad[q]
                lson[q] = lson[p]
                dad[lson[p]] = q
            rson[q] = rson[p]
            dad[rson[p]] = q
        dad[q] = dad[p]
        if rson[dad[p]] == p:
            rson[dad[p]] = q
        else:
            lson[dad[p]] = q
        dad[p] = nil

    if not data:
        return b""
    r = n - f
    length = min(f, len(data))
    text[r : r + length] = data[:length]
    text[r + length : r + f] = bytes([_STALE]) * (f - length)
    read = length
    for i in range(1, f + 1):
        insert(r - i)
    insert(r)
    out = bytearray()
    code = bytearray(1)
    mask = 1
    s = 0
    while length > 0:
        match = min(match_length, length)
        if match <= 2:
            match = 1
            code[0] |= mask
            code.append(text[r])
        else:
            code += ((match - 3) << bits | match_position).to_bytes(2, "big")
        mask = (mask << 1) & 0xFF
        if not mask:
            out += code
            code = bytearray(1)
            mask = 1
        i = 0
        while i < match and read < len(data):
            byte = data[read]
            read += 1
            delete(s)
            text[s] = byte
            if s < f - 1:
                text[s + n] = byte
            s = (s + 1) & (n - 1)
            r = (r + 1) & (n - 1)
            insert(r)
            i += 1
        while i < match:
            i += 1
            delete(s)
            s = (s + 1) & (n - 1)
            r = (r + 1) & (n - 1)
            length -= 1
            if length:
                insert(r)
    if len(code) > 1:
        out += code
    return bytes(out)
