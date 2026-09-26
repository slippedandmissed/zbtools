"""Minimal reader for the OMF object files (.OBJ) Borland C++ 4.5x writes.

Reads just what comparing compiled functions needs: each segment's bytes, its
fixups (relocations) with the symbol or segment they refer to, and the public
and external symbol names. It's hand-written because no Python library reads
OMF (PyPI's `omf` is the unrelated Open Mining Format). Reference: the Tool
Interface Standard "Relocatable Object Module Format (OMF) Specification" 1.1.
"""

from collections.abc import Callable
from dataclasses import dataclass, field

# Bytes patched by each fixup location type.
_LOCATION_SIZES = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 4, 11: 6, 13: 4}
_COMMUNAL_LENGTH_SIZES = {0x81: 2, 0x84: 3, 0x88: 4}


class OmfError(ValueError):
    pass


@dataclass(frozen=True)
class Fixup:
    offset: int  # within the segment
    size: int  # bytes the linker patches
    self_relative: bool  # e.g. a call's rel32 operand, vs an absolute address
    target: str  # symbol, segment or group name


@dataclass
class Segment:
    name: str
    class_name: str
    data: bytearray
    fixups: list[Fixup] = field(default_factory=list)


@dataclass(frozen=True)
class Public:
    name: str
    segment: str
    offset: int
    local: bool  # a static symbol (LPUBDEF)


@dataclass
class ObjectFile:
    segments: dict[str, Segment]
    publics: list[Public]
    externals: list[str]

    def extent(self, public: Public) -> tuple[int, int]:
        """A public symbol's (start, end) in its segment: it runs to the next
        symbol in the same segment, or the segment's end."""
        later = [
            p.offset
            for p in self.publics
            if p.segment == public.segment and p.offset > public.offset
        ]
        return public.offset, min(later, default=len(self.segments[public.segment].data))


class _Record:
    """Cursor over one record's body."""

    def __init__(self, body: bytes, is32: bool) -> None:
        self.body, self.pos, self.is32 = body, 0, is32

    def more(self) -> bool:
        return self.pos < len(self.body)

    def byte(self) -> int:
        self.pos += 1
        return self.body[self.pos - 1]

    def uint(self, size: int) -> int:
        self.pos += size
        return int.from_bytes(self.body[self.pos - size : self.pos], "little")

    def offset(self) -> int:
        """A 32-bit field in 32-bit records, else 16-bit."""
        return self.uint(4 if self.is32 else 2)

    def index(self) -> int:
        first = self.byte()
        return (first & 0x7F) << 8 | self.byte() if first & 0x80 else first

    def name(self) -> str:
        length = self.byte()
        self.pos += length
        return self.body[self.pos - length : self.pos].decode("latin-1")

    def rest(self) -> bytes:
        return self.body[self.pos :]

    def communal_length(self) -> int:
        """A COMDEF length: one byte up to 0x80, else a prefix giving its size."""
        first = self.byte()
        size = _COMMUNAL_LENGTH_SIZES.get(first)
        return self.uint(size) if size else first


class _Parser:
    def __init__(self) -> None:
        self.lnames = [""]
        self.segments: list[Segment] = []
        self.groups: list[str] = []
        self.externals: list[str] = []
        self.publics: list[Public] = []
        self.last_data: tuple[Segment, int] | None = None
        self.target_threads = [(0, 0)] * 4
        self.handlers: dict[int, Callable[[_Record, int], None]] = {
            0x96: self.lnames_record,
            0x98: self.segdef,
            0x9A: self.grpdef,
            0x8C: self.extdef,
            0xB4: self.extdef,
            0xBC: self.cextdef,
            0xB0: self.comdef,
            0x90: self.pubdef,
            0xB6: self.pubdef,
            0xA0: self.ledata,
            0xA2: self.lidata,
            0x9C: self.fixupp,
        }

    def lnames_record(self, r: _Record, _: int) -> None:
        while r.more():
            self.lnames.append(r.name())

    def segdef(self, r: _Record, _: int) -> None:
        if r.byte() >> 5 == 0:  # absolute segment: frame number and offset
            r.uint(3)
        size = r.offset()
        name, class_name = self.lnames[r.index()], self.lnames[r.index()]
        self.segments.append(Segment(name, class_name, bytearray(size)))

    def grpdef(self, r: _Record, _: int) -> None:
        self.groups.append(self.lnames[r.index()])

    def extdef(self, r: _Record, _: int) -> None:
        while r.more():
            self.externals.append(r.name())
            r.index()  # type

    def cextdef(self, r: _Record, _: int) -> None:
        while r.more():
            self.externals.append(self.lnames[r.index()])
            r.index()  # type

    def comdef(self, r: _Record, _: int) -> None:
        """Uninitialised globals, numbered along with the externals."""
        while r.more():
            self.externals.append(r.name())
            r.index()  # type
            if r.byte() == 0x61:  # far: element count, then element size
                r.communal_length()
            r.communal_length()

    def pubdef(self, r: _Record, kind: int) -> None:
        r.index()  # group
        segment_index = r.index()
        if segment_index == 0:
            r.uint(2)  # frame number
        segment = self.segments[segment_index - 1].name if segment_index else ""
        while r.more():
            name, offset = r.name(), r.offset()
            r.index()  # type
            self.publics.append(Public(name, segment, offset, local=kind == 0xB6))

    def ledata(self, r: _Record, _: int) -> None:
        segment = self.segments[r.index() - 1]
        offset = r.offset()
        chunk = r.rest()
        segment.data[offset : offset + len(chunk)] = chunk
        self.last_data = (segment, offset)

    def lidata(self, r: _Record, _: int) -> None:
        """Iterated data: nested repeat blocks, e.g. a run of zero bytes."""
        segment = self.segments[r.index() - 1]
        offset = r.offset()
        chunk = bytearray()
        while r.more():
            chunk += self.iterated_block(r)
        segment.data[offset : offset + len(chunk)] = chunk
        self.last_data = (segment, offset)

    def iterated_block(self, r: _Record) -> bytes:
        repeat = r.offset()
        blocks = r.uint(2)
        if blocks == 0:
            length = r.byte()
            content = r.body[r.pos : r.pos + length]
            r.pos += length
        else:
            content = b"".join(self.iterated_block(r) for _ in range(blocks))
        return content * repeat

    def fixupp(self, r: _Record, _: int) -> None:
        while r.more():
            first = r.byte()
            if first & 0x80:
                self.fixup(r, first)
            else:
                self.thread(r, first)

    def thread(self, r: _Record, first: int) -> None:
        """A THREAD subrecord: a frame or target remembered for later fixups."""
        is_frame, method, thread = first & 0x40, (first >> 2) & 7, first & 3
        index = 0
        if method < 3:
            index = r.index()
        elif is_frame and method == 3:
            r.uint(2)
        if not is_frame:
            self.target_threads[thread] = (method & 3, index)

    def fixup(self, r: _Record, first: int) -> None:
        if self.last_data is None:
            raise OmfError("fixup before any LEDATA record")
        locat = first << 8 | r.byte()
        fixdat = r.byte()
        if not fixdat & 0x80:  # frame given explicitly
            frame = (fixdat >> 4) & 7
            if frame < 3:
                r.index()
            elif frame == 3:
                r.uint(2)
        if fixdat & 0x08:  # target given by a thread
            method, index = self.target_threads[fixdat & 3]
        else:
            method, index = fixdat & 3, r.index()
        if not fixdat & 0x04:  # target displacement present
            r.offset()
        names = {0: [s.name for s in self.segments], 1: self.groups, 2: self.externals}.get(method)
        if names is None:
            raise OmfError(f"unsupported fixup target method {method}")
        segment, base = self.last_data
        segment.fixups.append(
            Fixup(
                offset=base + (locat & 0x3FF),
                size=_LOCATION_SIZES[(locat >> 10) & 0xF],
                self_relative=not locat & 0x4000,
                target=names[index - 1],
            )
        )

    def parse(self, data: bytes) -> ObjectFile:
        pos = 0
        while pos + 3 <= len(data):
            rtype = data[pos]
            length = int.from_bytes(data[pos + 1 : pos + 3], "little")
            body = data[pos + 3 : pos + 3 + length - 1]  # the last byte is a checksum
            pos += 3 + length
            kind = rtype & ~1  # odd record types are the 32-bit variants
            if kind == 0x8A:  # MODEND
                break
            if kind == 0xC2:  # COMDAT
                raise OmfError(f"unsupported OMF record {rtype:#04x}")
            handler = self.handlers.get(kind)
            if handler:
                handler(_Record(body, bool(rtype & 1)), kind)
        return ObjectFile({s.name: s for s in self.segments}, self.publics, self.externals)


def read(data: bytes) -> ObjectFile:
    return _Parser().parse(data)
