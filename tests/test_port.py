from pathlib import Path

from zbtools.port import SitePiece, site_packages, site_pieces

A, B, C = Path("a"), Path("b"), Path("c")


def test_small_files_stay_whole() -> None:
    assert site_pieces([(A, "/a", 10), (B, "/b", 3)], 10) == [
        SitePiece(A, "/a", 0, 10),
        SitePiece(B, "/b", 0, 3),
    ]


def test_big_files_are_split_into_parts() -> None:
    assert site_pieces([(A, "/sf/a.sf2", 25)], 10) == [
        SitePiece(A, "/sf/a.sf2.part0", 0, 10),
        SitePiece(A, "/sf/a.sf2.part1", 10, 10),
        SitePiece(A, "/sf/a.sf2.part2", 20, 5),
    ]


def test_packages_stay_under_the_limit() -> None:
    pieces = site_pieces([(A, "/a", 6), (B, "/b", 5), (C, "/c", 4)], 10)
    packages = site_packages(pieces, 10)
    assert all(sum(p.size for p in package) <= 10 for package in packages)
    assert sorted(p for package in packages for p in package) == sorted(pieces)
    assert len(packages) == 2


def test_packages_keep_the_given_order() -> None:
    pieces = site_pieces([(A, "/a", 1), (B, "/b", 3), (C, "/c", 2)], 10)
    assert site_packages(pieces, 10) == [pieces]
