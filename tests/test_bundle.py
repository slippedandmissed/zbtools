import plistlib
from pathlib import Path

from zbtools import bundle
from zbtools.formats import icon


def _icon() -> icon.Icon:
    palette = tuple((i * 16, 0, 255 - i * 16) for i in range(16))
    return icon.Icon(32, 32, palette, tuple(i % 16 for i in range(32 * 32)), (False,) * (32 * 32))


def _game(tmp_path: Path) -> Path:
    game = tmp_path / "game"
    (game / "C" / "ZOOMBI32").mkdir(parents=True)
    (game / "C" / "ZOOMBI32" / "ZOOMBI32.CFG").write_text("x")
    (game / "D" / "DATA").mkdir(parents=True)
    (game / "D" / "DATA" / "MAP.MHK").write_bytes(b"MHWK")
    return game


def test_info_plist_names_the_executable() -> None:
    plist = plistlib.loads(bundle.info_plist("1.2.3"))
    assert plist["CFBundleExecutable"] == bundle.EXECUTABLE
    assert plist["CFBundleShortVersionString"] == "1.2.3"
    assert plist["LSMinimumSystemVersion"] == bundle.MACOS_MINIMUM


def test_app_bundle_layout(tmp_path: Path) -> None:
    program = tmp_path / "zoombinis"
    program.write_bytes(b"\x7fELF")
    app = bundle.assemble_app(tmp_path / "Z.app", program, _game(tmp_path), _icon(), "1.0")
    assert (app / "Contents" / "MacOS" / bundle.EXECUTABLE).read_bytes() == b"\x7fELF"
    resources = app / "Contents" / "Resources"
    assert (resources / "game" / "D" / "DATA" / "MAP.MHK").is_file()
    assert (resources / "game" / "C" / "ZOOMBI32" / "ZOOMBI32.CFG").is_file()
    assert (resources / f"{bundle.APP_NAME}.icns").read_bytes()[:4] == b"icns"
    assert (app / "Contents" / "Info.plist").is_file()


def test_directory_and_archive(tmp_path: Path) -> None:
    program = tmp_path / "zoombinis"
    program.write_bytes(b"x")
    directory = bundle.assemble_directory(tmp_path / "out" / "z", program, _game(tmp_path))
    assert (directory / "game" / "D" / "DATA" / "MAP.MHK").is_file()
    archive = bundle.make_archive(directory, tmp_path / "z.tar.gz")
    assert archive.stat().st_size > 0
    assert bundle.make_archive(directory, tmp_path / "z.zip").stat().st_size > 0
