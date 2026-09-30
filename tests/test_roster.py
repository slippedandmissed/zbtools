from pathlib import Path

import pytest

from zbtools import assets
from zbtools.formats import Unconvertible, roster


def _file(version: int, next_id: int, games: list[tuple[str, str]]) -> bytes:
    data = b"".join(n.to_bytes(2, "little") for n in (version, next_id, len(games)))
    for name, file in games:
        data += name.encode().ljust(23, b"\0") + file.encode().ljust(9, b"\0")
    return data.ljust(roster.SIZE, b"\0")


def test_an_empty_roster_round_trips() -> None:
    data = _file(107, 1, [])
    assert roster.from_toml(roster.to_toml(data)) == data


def test_saved_games_round_trip() -> None:
    data = _file(107, 4, [('Ann "the" Great', "ZB000001"), ("", ""), ("Bo", "ZB000003")])
    text = roster.to_toml(data)
    assert text.count("[[games]]") == 3
    assert roster.from_toml(text) == data


def test_a_file_of_the_wrong_size_is_refused() -> None:
    with pytest.raises(Unconvertible):
        roster.to_toml(b"short")
    with pytest.raises(Unconvertible):
        roster.to_toml(_file(107, 1, []) + b"x")


def test_text_after_a_strings_end_is_refused() -> None:
    bad = bytearray(_file(107, 1, [("Bo", "ZB1")]))
    bad[6 + 5] = ord("x")  # in the name's padding
    with pytest.raises(Unconvertible):
        roster.to_toml(bytes(bad))


def test_installed_files_are_converted_and_verified(tmp_path: Path) -> None:
    game, out = tmp_path / "game", tmp_path / "out"
    game.mkdir()
    (game / "mohawk.w32").write_bytes(b"[INI]")
    (game / "CORNER.TTF").write_bytes(b"font")
    (game / "Zoombini.who").write_bytes(_file(107, 2, [("Bo", "ZB1")]))
    assets.extract_installed(game, out)
    assert (out / "Zoombini.who.toml").exists()
    assert not (out / "Zoombini.who").exists()
    assert assets.load_installed(out)["Zoombini.who"] == (game / "Zoombini.who").read_bytes()
    assert assets.verify_installed(game, out) == []
    (out / "mohawk.w32").write_bytes(b"[EDITED]")
    assert assets.verify_installed(game, out) == ["  mohawk.w32: changed"]
