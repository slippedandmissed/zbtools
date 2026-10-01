"""Packages the native build for players: a macOS application bundle, or a
directory with the program beside its data on other systems.

Both hold the program and a `game/` directory (what the port reads without
being told its drives: `C/`, what the installer would have written, copied to
the player's own directory on the first run; `D/`, the CD; and the SoundFont),
which is `Contents/Resources/game/` in a bundle. Nothing here needs the game's
disc: the data comes from assets/ (`uv run port`'s `bundle`).
"""

import plistlib
import shutil
import subprocess
import tarfile
import zipfile
from pathlib import Path

from PIL import Image

from zbtools import host
from zbtools.formats import icon

APP_NAME = "Zoombinis"
EXECUTABLE = "zoombinis"
# macOS 11 is the first with Apple silicon, and what a universal build targets.
MACOS_MINIMUM = "11.0"
# The sizes an .icns needs for the Dock and Finder; the game's own icon is
# 32 pixels, so these enlarge it by whole steps without smoothing.
_ICNS_SIZES = (16, 32, 64, 128, 256, 512)


def info_plist(version: str) -> bytes:
    """The bundle's Info.plist."""
    return plistlib.dumps(
        {
            "CFBundleName": APP_NAME,
            "CFBundleDisplayName": "Logical Journey of the Zoombinis",
            "CFBundleIdentifier": "online.zoombinis.player",
            "CFBundleExecutable": EXECUTABLE,
            "CFBundleIconFile": APP_NAME,
            "CFBundlePackageType": "APPL",
            "CFBundleShortVersionString": version,
            "CFBundleVersion": version,
            "LSMinimumSystemVersion": MACOS_MINIMUM,
            "LSApplicationCategoryType": "public.app-category.kids-games",
            "NSHighResolutionCapable": True,
        }
    )


def write_icns(app_icon: icon.Icon, path: Path) -> None:
    """The program's icon as an .icns (the Dock's and Finder's)."""
    png = path.with_suffix(".png")
    icon.save_png(app_icon, png)
    with Image.open(png) as opened:
        base = opened.convert("RGBA")
    png.unlink()
    largest = max(_ICNS_SIZES)
    base = base.resize((largest, largest), Image.Resampling.NEAREST)
    base.save(path, format="ICNS", sizes=[(s, s) for s in _ICNS_SIZES])


def _copy_game(game: Path, target: Path) -> None:
    shutil.rmtree(target, ignore_errors=True)
    shutil.copytree(game, target)


def assemble_app(
    app: Path, executable: Path, game: Path, app_icon: icon.Icon | None, version: str
) -> Path:
    """Makes the application bundle `app` from the program, the game's data
    directory and the icon."""
    shutil.rmtree(app, ignore_errors=True)
    macos, resources = app / "Contents" / "MacOS", app / "Contents" / "Resources"
    macos.mkdir(parents=True)
    resources.mkdir(parents=True)
    shutil.copy2(executable, macos / EXECUTABLE)
    _copy_game(game, resources / "game")
    if app_icon:
        write_icns(app_icon, resources / f"{APP_NAME}.icns")
    (app / "Contents" / "Info.plist").write_bytes(info_plist(version))
    return app


def assemble_directory(target: Path, executable: Path, game: Path) -> Path:
    """Makes the directory `target` with the program and `game/` beside it."""
    shutil.rmtree(target, ignore_errors=True)
    target.mkdir(parents=True)
    shutil.copy2(executable, target / executable.name)
    _copy_game(game, target / "game")
    return target


def sign_ad_hoc(app: Path) -> None:
    """Signs the bundle with no identity, which is what Apple silicon needs to
    run it at all (Gatekeeper still asks the first time: notarising needs a
    developer account)."""
    subprocess.run(["codesign", "--force", "--deep", "--sign", "-", str(app)], check=True)


def make_dmg(app: Path, dmg: Path) -> Path:
    """A disk image holding the bundle."""
    dmg.unlink(missing_ok=True)
    subprocess.run(
        ["hdiutil", "create", "-volname", APP_NAME, "-srcfolder", str(app), "-ov", str(dmg)],
        check=True,
        stdout=subprocess.DEVNULL,
    )
    return dmg


def make_archive(directory: Path, archive: Path) -> Path:
    """A .zip (Windows) or .tar.gz (elsewhere) of the directory, under its own name."""
    archive.unlink(missing_ok=True)
    if archive.suffix == ".zip":
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as z:
            for f in sorted(directory.rglob("*")):
                z.write(f, Path(directory.name) / f.relative_to(directory))
    else:
        with tarfile.open(archive, "w:gz") as t:
            t.add(directory, arcname=directory.name)
    return archive


def is_macos() -> bool:
    return host.SYSTEM == "Darwin"
