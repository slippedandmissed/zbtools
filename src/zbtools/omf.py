"""Minimal reader for the OMF object files (.OBJ) Borland C++ 4.5x writes.

Reads just what comparing compiled functions needs: each segment's bytes, its
fixups (relocations) with the symbol or segment they refer to, and the public
and external symbol names, from single objects or libraries (.LIB) of them.
It's hand-written because no Python library reads OMF (PyPI's `omf` is the
unrelated Open Mining Format). Reference: the Tool Interface Standard
"Relocatable Object Module Format (OMF) Specification" 1.1, plus Borland's
virtual segments (see _VIRTUAL).
"""

from collections.abc import Callable
from dataclasses import dataclass, field

# Bytes patched by each fixup location type.
_LOCATION_SIZES = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 4, 11: 6, 13: 4}
_COMMUNAL_LENGTH_SIZES = {0x81: 2, 0x84: 3, 0x88: 4}
# Borland: an index with this bit set refers to a "virtual segment", declared by
# the COMDEF entry with the remaining bits as its external number. Virtual
# segments hold C++ code and data the linker includes once (inline functions,
# template instances, RTTI type descriptors).
_VIRTUAL = 0x4000


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
    name: str  # the module name (THEADR), usually its source file
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
        self.name = ""
        self.lnames = [""]
        self.segments: list[Segment] = []
        self.groups: list[str] = []
        self.externals: list[str] = []
        self.publics: list[Public] = []
        self.virtual_segments: dict[int, Segment] = {}  # by external number
        self.last_data: tuple[Segment, int] | None = None
        self.target_threads = [(0, 0)] * 4
        self.handlers: dict[int, Callable[[_Record, int], None]] = {
            0x80: self.theadr,
            0x96: self.lnames_record,
            0x98: self.segdef,
            0x9A: self.grpdef,
            0x8C: self.extdef,
            0xB4: self.extdef,
            0xBC: self.cextdef,
            0xB0: self.comdef,
            0xB8: self.comdef,  # LCOMDEF
            0x90: self.pubdef,
            0xB6: self.pubdef,
            0xA0: self.ledata,
            0xA2: self.lidata,
            0x9C: self.fixupp,
        }

    def theadr(self, r: _Record, _: int) -> None:
        self.name = r.name()

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
        """Uninitialised globals, numbered along with the externals; in Borland's
        objects, also virtual segments (a data type naming the segment, e.g. _TEXT)."""
        while r.more():
            name = r.name()
            self.externals.append(name)
            r.index()  # type
            data_type = r.byte()
            if data_type == 0x61:  # far: element count, then element size
                r.communal_length()
            length = r.communal_length()
            if 0 < data_type < 0x60:  # Borland virtual segment, based on segment data_type
                base = self.segments[data_type - 1]
                segment = Segment(name, base.class_name, bytearray(length))
                self.virtual_segments[len(self.externals)] = segment
                self.publics.append(Public(name, name, 0, local=False))

    def segment(self, index: int) -> Segment:
        if index & _VIRTUAL:
            return self.virtual_segments[index & ~_VIRTUAL]
        return self.segments[index - 1]

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
        segment = self.segment(r.index())
        offset = r.offset()
        chunk = r.rest()
        segment.data[offset : offset + len(chunk)] = chunk
        self.last_data = (segment, offset)

    def lidata(self, r: _Record, _: int) -> None:
        """Iterated data: nested repeat blocks, e.g. a run of zero bytes."""
        segment = self.segment(r.index())
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
        if is_frame:  # F0-F2 have an index, F3 a frame number, F4-F6 nothing
            if method < 3:
                r.index()
            elif method == 3:
                r.uint(2)
        else:  # T0-T2 and T4-T6 (the same, without displacement) have an index
            self.target_threads[thread] = (method & 3, r.index())

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
        if method == 0 and index & _VIRTUAL:  # a virtual segment, named by its COMDEF
            method, index = 2, index & ~_VIRTUAL
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

    def parse(self, data: bytes, pos: int = 0) -> tuple[ObjectFile, int]:
        """Parse one module starting at pos; returns it and the offset after it."""
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
                try:
                    handler(_Record(body, bool(rtype & 1)), kind)
                except IndexError as e:
                    raise OmfError(f"malformed record {rtype:#04x} in {self.name!r}") from e
        segments = {s.name: s for s in [*self.segments, *self.virtual_segments.values()]}
        return ObjectFile(self.name, segments, self.publics, self.externals), pos


def read(data: bytes) -> ObjectFile:
    return _Parser().parse(data)[0]


def read_library(data: bytes) -> tuple[list[ObjectFile], list[str]]:
    """Read every module of a library (.LIB). Returns the modules read, and a
    description of each module skipped because it uses unsupported records."""
    if not data or data[0] != 0xF0:
        raise OmfError("not an OMF library")
    page_size = int.from_bytes(data[1:3], "little") + 3
    modules: list[ObjectFile] = []
    skipped: list[str] = []
    pos = page_size
    while pos < len(data) and data[pos] == 0x80:  # THEADR; the dictionary (0xF1) ends the modules
        start = pos
        try:
            module, pos = _Parser().parse(data, pos)
            modules.append(module)
        except OmfError as e:
            skipped.append(f"module at {start:#x}: {e}")
            pos = _skip_module(data, start)
        pos = -(-pos // page_size) * page_size  # modules start on page boundaries
    return modules, skipped


def _skip_module(data: bytes, pos: int) -> int:
    """Offset just past the MODEND record of the module starting at pos."""
    while pos + 3 <= len(data):
        rtype = data[pos]
        pos += 3 + int.from_bytes(data[pos + 1 : pos + 3], "little")
        if rtype & ~1 == 0x8A:
            break
    return pos
