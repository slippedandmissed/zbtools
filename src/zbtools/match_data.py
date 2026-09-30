"""Check the decompiled code's data against the game's.

Each compiled object has its data in segments: its initialised globals and
statics (`_DATA`, with its string literals last), its uninitialised globals
(`_BSS`), and the vtables the compiler generates for its classes (a segment
each, which the linker includes once). TLINK32 puts each object's segments
after the previous object's in the game's DATA section. This finds where the
pieces of every data segment of decomp/'s objects are in the original, and
compares them with it.

A segment is cut into pieces where something is known to start: a global it
defines, and each place in it the code refers to (a static, a string
literal, an element of an array). Each piece is placed by votes for its
address in the original:

- a global a header declares with an address (`extern short primes[5];
  /* @data 0x4a0800 */`, or a `g_4a0800` name) is at that address;
- a reference to the piece from a function recorded as matching
  (decomp/matching.txt): the compiled instruction holds the offset into the
  segment, the original the absolute address;
- a pointer to the piece from a placed piece of data, which the original
  holds in the same place.

The most common vote wins; a piece nothing votes for isn't placed (while its
module's other globals are undefined, most of its literals are only placed
this way). Placed initialised pieces are compared byte for byte. Their
pointers (the bytes the linker fills in) must be relocated in the original
and, where what they point to has been placed (a marked function, a declared
global, a placed piece), point to it. Uninitialised pieces have no bytes to
compare.

Each reference to data in a function (recorded as matching, or in an
instruction that lines up with the original's, at the same offset, in one that isn't) must point
where the original's does: the global's declared address, or where its piece is
placed, plus the offset the instruction adds. `match` can't check this (it
masks the addresses the linker fills in), so a function can match while
reading the wrong global, or the wrong element of an array.

A segment matches when its pieces are all placed, one after the other as in
the compiled segment, and the same as the original. Until a module defines
all its data, the original has more between the pieces; but a piece the
original has before an earlier one (globals defined in another order), or in
the other kind of data (an uninitialised global the original initialises,
say), is misplaced. The layout only matters for rebuilding the original
exactly; the values are what the game needs.

The runtime library's data comes from its own objects at link time, so it's
never placed here; nor is any data that nothing in decomp/ defines yet.
"""

from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
from typing import Annotated

import typer

from zbtools import declarations, match, omf, paths
from zbtools.demangle import qualified_name
from zbtools.exe import Executable

_DATA_SECTION = "DATA"
_DATA_CLASSES = {"DATA", "BSS"}  # not _INIT_/_EXIT_: the runtime's startup tables
_UNINITIALISED = "BSS"
# How far before an array a reference can point (an array indexed from 1, or
# `a[i - 1]`, with elements up to this size) and still be taken as a
# reference to it. A reference only counts as right if it lands exactly.
_INDEX_SLACK = 256


@dataclass(frozen=True)
class Piece:
    offset: int  # in its segment
    size: int
    address: int | None  # in the original; None if nothing places it


@dataclass(frozen=True)
class Placement:
    """One compiled data segment and where its pieces are in the original."""

    source: Path
    segment: str
    size: int
    initialised: bool
    pieces: tuple[Piece, ...]
    mismatches: tuple[int, ...]  # offsets in placed pieces that differ
    notes: dict[int, str]  # offset of each mismatch -> the piece it's in
    misplaced: tuple[str, ...]  # pieces out of order, or in the other kind of data

    @property
    def address(self) -> int | None:
        """Where the segment starts in the original, by most of its placed bytes."""
        starts: Counter[int] = Counter()
        for piece in self.pieces:
            if piece.address is not None:
                starts[piece.address - piece.offset] += piece.size
        return starts.most_common(1)[0][0] if starts else None

    @property
    def placed_bytes(self) -> int:
        return sum(p.size for p in self.pieces if p.address is not None)

    @property
    def matches(self) -> bool:
        """Every piece placed, where the others are (nothing missing between
        them), and the same as the original."""
        base = self.address
        return (
            base is not None
            and all(p.address == base + p.offset for p in self.pieces)
            and not self.mismatches
        )


@dataclass(frozen=True)
class Declared:
    """A global a header declares with an address, and where it's defined."""

    name: str
    address: int
    initialised: bool  # in the original's initialised data
    source: Path | None  # the source defining it, if any


@dataclass(frozen=True)
class WrongReference:
    """A reference to data in a marked function that points somewhere
    other than the original's (which `match` can't see: it masks the
    addresses the linker fills in)."""

    source: Path
    function: str
    offset: int  # in the function
    target: str  # what it refers to: a global, or a segment and offset
    ours: int  # where that is in the original
    theirs: int  # where the original's instruction points


@dataclass(frozen=True)
class Checked:
    placements: list[Placement]
    declared: list[Declared]
    initialised_range: tuple[int, int]  # of the original's DATA section
    uninitialised_range: tuple[int, int]
    errors: dict[Path, str]  # sources that couldn't be compiled
    references: int  # data references checked in marked functions
    wrong_references: list[WrongReference]

    def _addresses(self, *, initialised: bool) -> tuple[set[int], set[int]]:
        """The original's addresses placed pieces cover that match, and that differ."""
        start, end = self.initialised_range if initialised else self.uninitialised_range
        same: set[int] = set()
        different: set[int] = set()
        for p in self.placements:
            if p.initialised != initialised:
                continue
            mismatches = set(p.mismatches)
            for piece in p.pieces:
                if piece.address is None:
                    continue
                for k in range(piece.size):
                    if start <= piece.address + k < end:
                        found = different if piece.offset + k in mismatches else same
                        found.add(piece.address + k)
        return same - different, different

    @property
    def initialised_size(self) -> int:
        return self.initialised_range[1] - self.initialised_range[0]

    @property
    def uninitialised_size(self) -> int:
        return self.uninitialised_range[1] - self.uninitialised_range[0]

    def percent(self, part: int, *, initialised: bool = True) -> float:
        whole = self.initialised_size if initialised else self.uninitialised_size
        return 100 * part / whole if whole else 0.0

    @property
    def matching_bytes(self) -> int:
        """Initialised bytes placed and the same as the original's."""
        return len(self._addresses(initialised=True)[0])

    @property
    def differing_bytes(self) -> int:
        return len(self._addresses(initialised=True)[1])

    @property
    def placed_uninitialised_bytes(self) -> int:
        return len(self._addresses(initialised=False)[0])

    @property
    def unplaced_bytes(self) -> int:
        """Compiled data that nothing places yet."""
        return sum(p.size - p.placed_bytes for p in self.placements)

    @property
    def misplaced(self) -> int:
        return sum(len(p.misplaced) for p in self.placements)

    def count(self, *, initialised: bool, defined: bool | None = None) -> int:
        return sum(
            d.initialised == initialised and (defined is None or (d.source is not None) == defined)
            for d in self.declared
        )


def plain_name(symbol: str) -> str:
    """A data symbol's name as the source writes it, upper-cased (under -p the
    compiler upper-cases it; a C one has a `_` prefix; a C++ one is mangled)."""
    name = qualified_name(symbol) if symbol.startswith("@") else symbol.removeprefix("_")
    return name.upper()


def _is_virtual(obj: omf.ObjectFile, segment: str) -> bool:
    """A segment the linker includes once however many objects have it (a
    vtable): the object reader gives it a public symbol of its own name."""
    return any(p.name == p.segment == segment for p in obj.publics)


def _data_segments(obj: omf.ObjectFile) -> list[omf.Segment]:
    return [s for s in obj.segments.values() if s.class_name in _DATA_CLASSES and s.data]


# A data segment: (its source, its name), or (None, its name) for a vtable,
# which is the same in every object that has it.
_Key = tuple[Path | None, str]


@dataclass
class _Segment:
    key: _Key
    source: Path
    obj: omf.ObjectFile
    segment: omf.Segment
    votes: dict[int, Counter[int]] = field(default_factory=dict)  # offset -> addresses
    starts: set[int] = field(default_factory=set)  # offsets something refers to
    pieces: list[Piece] = field(default_factory=list)

    @property
    def end(self) -> int:
        """The segment's size without the zeros that pad it to 4 bytes after
        its last string literal (the original has the next module's data
        there, or its literals end elsewhere)."""
        data = self.segment.data
        publics = [p.offset for p in self.obj.publics if p.segment == self.segment.name]
        last = max([0, *self.votes, *self.starts, *publics])
        if self.segment.name != "_DATA" or last in publics or len(data) % 4:
            return len(data)
        zeros = len(data) - len(data.rstrip(b"\0"))
        return len(data) - max(0, min(zeros - 1, 3))

    def vote(self, offset: int, address: int) -> None:
        if 0 <= offset < len(self.segment.data):
            self.votes.setdefault(offset, Counter())[address] += 1

    def cut(self) -> None:
        """Cut the segment into pieces where a global starts or something points."""
        publics = (p.offset for p in self.obj.publics if p.segment == self.segment.name)
        starts = sorted({0, *self.votes, *self.starts, *publics})
        ends = [*starts[1:], self.end]
        self.pieces = [
            Piece(start, end - start, self._chosen(start))
            for start, end in zip(starts, ends, strict=True)
            if end > start
        ]

    def _chosen(self, offset: int) -> int | None:
        counter = self.votes.get(offset)
        return counter.most_common(1)[0][0] if counter else None

    def piece_start(self, offset: int) -> int:
        return max(p.offset for p in self.pieces if p.offset <= offset)

    def address_of(self, offset: int) -> int | None:
        """Where an offset in the segment is in the original, if its piece is placed."""
        for piece in self.pieces:
            if piece.offset <= offset < piece.offset + piece.size and piece.address is not None:
                return piece.address + offset - piece.offset
        return None


def _key(source: Path, obj: omf.ObjectFile, segment: str) -> _Key:
    return (None if _is_virtual(obj, segment) else source), segment


def _field(data: bytes | bytearray, fixup: omf.Fixup) -> int:
    """What a 4-byte fixup adds to its target's address."""
    return fixup.displacement + int.from_bytes(data[fixup.offset : fixup.offset + 4], "little")


class _Matcher:
    def __init__(self, objects: dict[Path, omf.ObjectFile], exe: Executable) -> None:
        self.objects, self.exe = objects, exe
        declared = declarations.load()[0]
        self.declared = declared
        self.data = {g.name.upper(): g.address for g in declared}
        self.targets = {s: match.find_targets(s.read_text()) for s in objects}
        self.functions: dict[str, int] = {}  # marked functions' symbols -> addresses
        for source, obj in objects.items():
            for target in self.targets[source]:
                try:
                    self.functions[match.locate(obj, target)[0].name] = target.address
                except ValueError:
                    continue
        self.segments: dict[_Key, _Segment] = {}
        for source, obj in objects.items():
            for segment in _data_segments(obj):
                key = _key(source, obj, segment.name)
                self.segments.setdefault(key, _Segment(key, source, obj, segment))

    def in_data_section(self, address: int) -> bool:
        start, end = self.exe.sections[_DATA_SECTION]
        return start <= address < end

    def target_segment(self, source: Path, obj: omf.ObjectFile, target: str) -> _Segment | None:
        """The data segment a fixup refers to, if it's one."""
        if target not in obj.segments:
            return None
        return self.segments.get(_key(source, obj, target))

    def find_starts(self) -> None:
        """Where anything refers to in each data segment (a static, a literal, an
        element), from any code or data: whether it matches or not, the offset
        it holds is exact."""
        for source, obj in self.objects.items():
            for segment in obj.segments.values():
                for f in segment.fixups:
                    target = self.target_segment(source, obj, f.target)
                    if target is not None and f.size == 4 and not f.self_relative:
                        offset = _field(segment.data, f)
                        if 0 < offset < len(target.segment.data):
                            target.starts.add(offset)

    def vote_declared(self) -> None:
        for s in self.segments.values():
            for public in s.obj.publics:
                address = self.data.get(plain_name(public.name))
                if public.segment == s.segment.name and address is not None:
                    s.vote(public.offset, address)

    def vote_code(self, matching: set[int]) -> None:
        """Votes from the references of functions recorded as matching."""
        for source, obj in self.objects.items():
            for target in self.targets[source]:
                if target.address not in matching:
                    continue
                try:
                    public, first, last = match.locate(obj, target)
                except ValueError:
                    continue
                code = obj.segments[public.segment]
                for f in code.fixups:
                    if not first <= f.offset < last or f.self_relative or f.size != 4:
                        continue
                    segment = self.target_segment(source, obj, f.target)
                    at = target.address + f.offset - first
                    if segment is None or at not in self.exe.relocations:
                        continue
                    address = self.exe.pointer(at)
                    if self.in_data_section(address):
                        segment.vote(_field(code.data, f), address)

    def vote_data(self) -> None:
        """Votes from pointers in placed pieces of data."""
        for s in list(self.segments.values()):
            for f in s.segment.fixups:
                at = s.address_of(f.offset)
                target = self.target_segment(s.source, s.obj, f.target)
                if at is None or target is None or f.size != 4 or at not in self.exe.relocations:
                    continue
                address = self.exe.pointer(at)
                if self.in_data_section(address):
                    target.vote(_field(s.segment.data, f), address)

    def resolve(self, s: _Segment, fixup: omf.Fixup) -> int | None:
        """Where a pointer in a data segment should point in the original, if known."""
        added = _field(s.segment.data, fixup)
        segment = self.target_segment(s.source, s.obj, fixup.target)
        if segment is not None:
            return segment.address_of(added)
        base = self.functions.get(fixup.target, self.data.get(plain_name(fixup.target)))
        return None if base is None else base + added

    def compare(self, s: _Segment) -> set[int]:
        """The offsets where a segment's placed pieces differ from the original."""
        mismatches: set[int] = set()
        pointers: set[int] = set()
        for f in s.segment.fixups:
            at = s.address_of(f.offset)
            span = set(range(f.offset, f.offset + f.size))
            pointers |= span
            if at is None:
                continue
            if f.self_relative or f.size != 4 or at not in self.exe.relocations:
                mismatches |= span
                continue
            expected = self.resolve(s, f)
            if expected is not None and self.exe.pointer(at) != expected:
                mismatches |= span
        for piece in s.pieces:
            if piece.address is None:
                continue
            original = self.exe.read(piece.address, piece.size)
            for k in range(piece.size):
                offset = piece.offset + k
                if offset not in pointers and s.segment.data[offset] != original[k]:
                    mismatches.add(offset)
        return mismatches

    def placement(self, s: _Segment) -> Placement:
        initialised = s.segment.class_name != _UNINITIALISED
        mismatches = self.compare(s) if initialised else set()
        return Placement(
            source=s.source,
            segment=s.segment.name,
            size=s.end,
            initialised=initialised,
            pieces=tuple(s.pieces),
            mismatches=tuple(sorted(mismatches)),
            notes={i: _owner(s.obj, s.segment.name, s.piece_start(i)) for i in sorted(mismatches)},
            misplaced=self.misplaced(s),
        )

    def misplaced(self, s: _Segment) -> tuple[str, ...]:
        """Pieces the original has in another order, or in the other kind of data.
        (While a module's globals are only partly defined, the original has
        more between its pieces than the compiled segment: that's fine.)"""
        found = []
        initialised = s.segment.class_name != _UNINITIALISED
        start, end = initialised_range(self.exe)
        previous: Piece | None = None
        for piece in s.pieces:
            if piece.address is None:
                continue
            name = _owner(s.obj, s.segment.name, piece.offset)
            if (start <= piece.address < end) != initialised:
                kind = "uninitialised" if initialised else "initialised"
                found.append(f"{name} is {kind} in the original ({piece.address:#x})")
            elif previous is not None and previous.address is not None:
                if piece.address - previous.address < piece.offset - previous.offset:
                    before = _owner(s.obj, s.segment.name, previous.offset)
                    found.append(
                        f"{name} ({piece.address:#x}) should come before {before} "
                        f"({previous.address:#x})"
                    )
            previous = piece
        return tuple(found)

    def run(self, matching: set[int]) -> list[Placement]:
        self.find_starts()
        self.vote_declared()
        self.vote_code(matching)
        for s in self.segments.values():
            s.cut()
        self.vote_data()
        for s in self.segments.values():
            s.cut()
        return [self.placement(s) for s in self.segments.values()]

    def check_references(self, matching: set[int]) -> tuple[int, list[WrongReference]]:
        """Whether each data reference in a marked function points where the
        original's does: the global's declared address (or its placed piece)
        plus the offset the instruction adds. In a function that doesn't
        match, the references in instructions that line up with the
        original's."""
        count, wrong = 0, []
        for source, obj in self.objects.items():
            for target in self.targets[source]:
                try:
                    public, first, last = match.locate(obj, target)
                    offsets = None if target.address in matching else self.aligned(obj, target)
                except ValueError:
                    continue
                code = obj.segments[public.segment]
                for f in code.fixups:
                    if not first <= f.offset < last or f.self_relative or f.size != 4:
                        continue
                    if offsets is None:
                        at = target.address + f.offset - first
                    elif f.offset - first in offsets:
                        at = target.address + offsets[f.offset - first]
                    else:
                        continue
                    if at not in self.exe.relocations:
                        continue
                    found = self.reference(source, obj, f, _field(code.data, f))
                    if found is None:
                        continue
                    label, candidates = found
                    count += 1
                    theirs = self.exe.pointer(at)
                    if theirs not in candidates and not self.same_data(
                        source, obj, f, _field(code.data, f), theirs
                    ):
                        wrong.append(
                            WrongReference(
                                source, target.name, f.offset - first, label, candidates[0], theirs
                            )
                        )
        return count, wrong

    def aligned(self, obj: omf.ObjectFile, target: match.Target) -> dict[int, int]:
        """For a function that doesn't match: the offsets in its instructions
        that are at the same offset in the original's, and equal to them but
        for the addresses the linker fills in."""
        result = match.compare(target, obj, self.exe)
        found = {}
        for row in match.aligned_rows(result):
            ours, theirs = row.compiled, row.original
            # Only where nothing has shifted: once masked, instructions look
            # alike (every `push offset`), and the diff can pair the wrong ones.
            if (
                row.same
                and ours is not None
                and theirs is not None
                and ours.address == theirs.address
            ):
                for k in range(len(ours.raw)):
                    found[ours.address - target.address + k] = ours.address - target.address + k
        return found

    def same_data(
        self, source: Path, obj: omf.ObjectFile, f: omf.Fixup, added: int, theirs: int
    ) -> bool:
        """Whether a reference into the object's own data points to the same
        bytes as the original's does, placed elsewhere (the layout differs
        there, not what's referred to): the rest of its piece, but for the
        bytes the linker fills in."""
        segment = self.target_segment(source, obj, f.target)
        if segment is None or segment.segment.class_name == _UNINITIALISED:
            return False
        if not 0 <= added < segment.end:
            return False
        end = next(
            p.offset + p.size for p in segment.pieces if p.offset <= added < p.offset + p.size
        )
        ours = segment.segment.data[added:end]
        original = self.exe.read(theirs, len(ours))
        pointers = {k for x in segment.segment.fixups for k in range(x.offset, x.offset + x.size)}
        return all(ours[i] == original[i] for i in range(len(ours)) if added + i not in pointers)

    def reference(
        self, source: Path, obj: omf.ObjectFile, f: omf.Fixup, added: int
    ) -> tuple[str, list[int]] | None:
        """What a fixup refers to, and where that can be in the original: for a
        global declared with an address, one place; for an offset in a segment
        of the object, in its piece there or, just before a piece (an array
        indexed from 1, say), relative to any of the pieces that follow.
        None if unknown."""
        segment = self.target_segment(source, obj, f.target)
        if segment is None:
            address = self.data.get(plain_name(f.target))
            if address is None:
                return None
            name = qualified_name(f.target) if f.target.startswith("@") else f.target
            label = name + (
                f"{added - 2**32:+#x}" if added >= 2**31 else f"+{added:#x}" if added else ""
            )
            return label, [(address + added) & 0xFFFFFFFF]
        offset = added - 2**32 if added >= 2**31 else added
        candidates = []
        inside = segment.address_of(offset)
        if inside is not None:
            candidates.append(inside)
        candidates += [
            p.address - (p.offset - offset)
            for p in segment.pieces
            if p.address is not None and 0 < p.offset - offset <= _INDEX_SLACK
        ]
        if not candidates:
            return None
        return f"{f.target}{offset:+#x}", candidates

    def defined(self) -> dict[str, Path]:
        """The sources defining each global, by upper-cased name."""
        return {
            plain_name(p.name): source
            for source, obj in self.objects.items()
            for p in obj.publics
            if p.segment in obj.segments and obj.segments[p.segment].class_name in _DATA_CLASSES
        }


def _owner(obj: omf.ObjectFile, segment: str, offset: int) -> str:
    """The global an offset in a data segment is in, or what else it can be."""
    before = [p for p in obj.publics if p.segment == segment and p.offset <= offset]
    if not before:
        return f"{segment}+{offset:#x} (a static or string literal)"
    public = max(before, key=lambda p: p.offset)
    name = qualified_name(public.name) if public.name.startswith("@") else public.name
    return f"{name}+{offset - public.offset:#x}" if offset > public.offset else name


def initialised_range(exe: Executable) -> tuple[int, int]:
    """The original's initialised data. The file holds more: TLINK32 pads it
    with zeros to the file alignment (0x200 bytes), and the uninitialised data
    starts in those zeros (the first module's `_BSS` at `0x4aa410`, just after
    the last nonzero byte). So it ends at its last nonzero byte, rounded up to
    4 bytes, as the modules' segments are."""
    start, end = exe.initialised[_DATA_SECTION]
    data = exe.read(start, end - start)
    return start, start + -(-len(data.rstrip(b"\0")) // 4) * 4


def check(sources: list[Path], exe: Executable, *, use_cache: bool = True) -> Checked:
    """Compile each source (or reuse its cached object), and place and compare
    its data segments."""
    compiled = match.compile_sources(sources, use_cache=use_cache)
    objects = {s: c.obj for s, c in compiled.items() if not isinstance(c, str)}
    errors = {s: c for s, c in compiled.items() if isinstance(c, str)}
    matcher = _Matcher(objects, exe)
    matching = set(match.load_baseline())
    placements = matcher.run(matching)
    references, wrong = matcher.check_references(matching)
    defined = matcher.defined()
    initialised = initialised_range(exe)
    return Checked(
        placements=placements,
        declared=[
            Declared(
                g.name,
                g.address,
                initialised[0] <= g.address < initialised[1],
                defined.get(g.name.upper()),
            )
            for g in sorted(matcher.declared, key=lambda g: g.address)
        ],
        initialised_range=initialised,
        uninitialised_range=(initialised[1], exe.sections[_DATA_SECTION][1]),
        errors=errors,
        references=references,
        wrong_references=wrong,
    )


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    files: Annotated[
        list[Path] | None,
        typer.Argument(help="Source files to check (default: all of decomp/)", show_default=False),
    ] = None,
    *,
    quiet: Annotated[bool, typer.Option("--quiet", "-q", help="Only the summary")] = False,
    no_cache: Annotated[
        bool, typer.Option("--no-cache", help="Recompile everything, ignoring build/match-cache")
    ] = False,
) -> None:
    match.require_toolchain()
    exe = match.game_executable()
    result = check(files or match.decomp_sources(), exe, use_cache=not no_cache)
    for source, error in result.errors.items():
        print(f"  ERROR      {source.name}: {error}")
    if not quiet:
        for p in result.placements:
            _print_placement(p)
    for w in result.wrong_references:
        print(
            f"  WRONG REF  {w.source.name} {w.function}+{w.offset:#x}: {w.target} is at "
            f"{w.ours:#x}, the original points to {w.theirs:#x}"
        )
    _print_summary(result)
    if result.wrong_references:
        raise typer.Exit(1)


def _print_placement(p: Placement) -> None:
    where = f"{p.source.relative_to(paths.REPO_ROOT)} {p.segment}"
    base = p.address
    if base is None:
        print(f"  unplaced   {where}: {p.size} bytes, nothing places it")
        return
    at = f"{where} @ {base:#x}"
    unplaced = p.size - p.placed_bytes
    placed = f"{p.size} bytes" + (f", {unplaced} not placed" if unplaced else "")
    if not p.mismatches and not p.misplaced:
        print(f"  {'match' if not unplaced else 'placed':10} {at}: {placed}")
        return
    problems = [
        *([f"{len(p.mismatches)} differ"] if p.mismatches else []),
        *([f"{len(p.misplaced)} misplaced"] if p.misplaced else []),
    ]
    print(f"  differs    {at}: {placed}, {', '.join(problems)}")
    notes = list(dict.fromkeys(p.notes.values()))
    for owner in notes[:8]:
        print(f"               differs: {owner}")
    if len(notes) > 8:
        print(f"               ... and {len(notes) - 8} more")
    for line in p.misplaced:
        print(f"               misplaced: {line}")


def _print_summary(result: Checked) -> None:
    def percent(part: int, whole: int) -> str:
        return f"{100 * part / whole:.1f}%" if whole else "-"

    size = result.initialised_size
    print(
        f"Initialised data: {result.matching_bytes} of {size} bytes placed and matching "
        f"({percent(result.matching_bytes, size)}), {result.differing_bytes} differing."
    )
    size = result.uninitialised_size
    print(
        f"Uninitialised data: {result.placed_uninitialised_bytes} of {size} bytes placed "
        f"({percent(result.placed_uninitialised_bytes, size)})."
    )
    print(
        f"Compiled data not placed yet: {result.unplaced_bytes} bytes; "
        f"{result.misplaced} pieces out of place."
    )
    print(
        f"Data references in decompiled functions: {result.references} checked, "
        f"{len(result.wrong_references)} pointing elsewhere than the original's."
    )
    for initialised, kind in ((True, "initialised"), (False, "uninitialised")):
        defined = result.count(initialised=initialised, defined=True)
        total = result.count(initialised=initialised)
        print(f"Declared {kind} globals: {defined} of {total} defined.")
