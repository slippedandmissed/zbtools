import platform
from pathlib import Path

import pytest

from zbtools import assets, host, paths, port
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


def test_drives_are_laid_out_from_assets(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    game, installed, packed = tmp_path / "game", tmp_path / "installed", tmp_path / "packed"
    for directory in (game, packed):
        directory.mkdir()
    monkeypatch.setattr(paths, "PORT_DATA_DIR", tmp_path / "data")
    with pytest.raises(port.PortError, match="assets extract"):
        port.lay_out_drives(installed, packed)
    (game / "mohawk.w32").write_bytes(b"settings")
    (game / "CORNER.TTF").write_bytes(b"font")
    (game / "Zoombini.who").write_bytes(b"not a roster")  # kept as it is
    assets.extract_installed(game, installed)
    with pytest.raises(port.PortError, match="assets pack"):
        port.lay_out_drives(installed, packed)
    (packed / "MIDIMAP.DAT").write_bytes(b"midimap")
    c = port.lay_out_drives(installed, packed)
    assert (c / "ZOOMBI32" / "MIDIMAP.DAT").read_bytes() == b"midimap"
    assert (c / "ZOOMBI32" / "mohawk.w32").read_bytes() == b"settings"
    assert (c / "ZOOMBI32" / "Zoombini.who").read_bytes() == b"not a roster"
    assert (c / "WINDOWS" / "FONTS" / "CORNER.TTF").read_bytes() == b"font"
    assert assets.verify_installed(game, installed) == []


def test_targets_match_the_paths_that_clean_removes() -> None:
    assert tuple(port.TARGETS) == paths.PORT_TARGETS


def test_the_default_target_is_the_machines(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(host, "SYSTEM", "Darwin")
    assert port.target_named(None).name == "macos_universal"
    monkeypatch.setattr(host, "SYSTEM", "Linux")
    monkeypatch.setattr(platform, "machine", lambda: "aarch64")
    assert port.target_named(None).name == "linux_arm64"
    monkeypatch.setattr(platform, "machine", lambda: "x86_64")
    assert port.target_named(None).name == "linux_x64"


def test_an_unknown_target_lists_the_known_ones() -> None:
    with pytest.raises(port.PortError, match="windows_x64"):
        port.target_named("win64")


def test_each_target_names_its_program() -> None:
    assert port.TARGETS["windows_x86"].program == "zoombinis.exe"
    assert port.TARGETS["browser_wasm"].program == port.WEB_PAGE
    assert port.TARGETS["linux_x64"].program == "zoombinis"
