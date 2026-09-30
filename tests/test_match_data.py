from pathlib import Path

from zbtools.match_data import Piece, Placement, _Segment, plain_name
from zbtools.omf import ObjectFile, Public, Segment


def _segment(data: bytes, publics: dict[str, int], name: str = "_DATA") -> _Segment:
    obj = ObjectFile(
        "test.cpp",
        {name: Segment(name, "DATA", bytearray(data))},
        [Public(symbol, name, offset, local=False) for symbol, offset in publics.items()],
        [],
    )
    return _Segment((Path("test.cpp"), name), Path("test.cpp"), obj, obj.segments[name])


def test_plain_names() -> None:
    assert plain_name("PRIMES") == "PRIMES"  # -p upper-cases
    assert plain_name("_primes") == "PRIMES"  # a C global
    assert plain_name("@Palette@current") == "PALETTE::CURRENT"


def test_pieces_start_at_globals_references_and_votes() -> None:
    s = _segment(b"\x02\x00\x03\x00abc\0de\0", {"PRIMES": 0})
    s.starts.add(4)  # a literal the code refers to
    s.vote(0, 0x4A0800)
    s.vote(8, 0x4A0900)
    s.cut()
    assert s.pieces == [Piece(0, 4, 0x4A0800), Piece(4, 4, None), Piece(8, 3, 0x4A0900)]
    assert s.address_of(2) == 0x4A0802
    assert s.address_of(5) is None


def test_most_votes_win() -> None:
    s = _segment(b"abcd", {})
    for address in (0x4A0000, 0x4A0010, 0x4A0010):
        s.vote(0, address)
    s.cut()
    assert s.pieces == [Piece(0, 4, 0x4A0010)]


def test_padding_after_the_last_literal_is_left_out() -> None:
    s = _segment(b"\x01\x00\x00\x00abc\0de\0\0", {"COUNT": 0})
    s.starts.add(4)
    s.starts.add(8)
    assert s.end == 11  # "de\0", without the byte padding it to 12


def test_a_zero_global_at_the_end_is_kept() -> None:
    s = _segment(b"abc\0\0\0\0\0", {"ZERO": 4})
    assert s.end == 8


def test_uninitialised_segments_have_no_padding() -> None:
    s = _segment(bytes(12), {}, name="_BSS")
    assert s.end == 12


def _placement(pieces: tuple[Piece, ...], mismatches: tuple[int, ...] = ()) -> Placement:
    return Placement(
        source=Path("test.cpp"),
        segment="_DATA",
        size=sum(p.size for p in pieces),
        initialised=True,
        pieces=pieces,
        mismatches=mismatches,
        notes={},
        misplaced=(),
    )


def test_a_segment_matches_when_whole_and_in_place() -> None:
    assert _placement((Piece(0, 4, 0x4A0000), Piece(4, 4, 0x4A0004))).matches
    # Something missing between the pieces (a global not defined yet).
    assert not _placement((Piece(0, 4, 0x4A0000), Piece(4, 4, 0x4A0010))).matches
    assert not _placement((Piece(0, 4, 0x4A0000), Piece(4, 4, None))).matches
    assert not _placement((Piece(0, 4, 0x4A0000),), mismatches=(1,)).matches


def test_a_segment_starts_where_most_of_its_bytes_say() -> None:
    placement = _placement((Piece(0, 4, 0x4A0000), Piece(4, 16, 0x4A0104)))
    assert placement.address == 0x4A0100
