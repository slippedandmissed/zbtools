"""Everything that depends on the host operating system lives here.

Supported hosts: macOS (tested) and Linux (untested). Tools never install
host packages themselves; they check for them and print how to install them.
The exception is Wine on macOS, which has no maintained package: a pinned,
checksum-verified build is downloaded into build/wine/ instead.
"""

import hashlib
import platform
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

from zbtools import paths

SYSTEM = platform.system()  # "Darwin" or "Linux"

_INSTALL_HINTS: dict[str, dict[str, str]] = {
    "qemu": {
        "Darwin": "brew install qemu",
        "Linux": "sudo apt install qemu-system-x86 qemu-utils   # Debian/Ubuntu\n"
        "  sudo dnf install qemu-system-x86 qemu-img     # Fedora",
    },
    "mtools": {
        "Darwin": "brew install mtools",
        "Linux": "sudo apt install mtools   # Debian/Ubuntu\n"
        "  sudo dnf install mtools     # Fedora",
    },
    "7-Zip": {
        "Darwin": "brew install sevenzip",
        "Linux": "sudo apt install 7zip   # Debian/Ubuntu\n  sudo dnf install 7zip   # Fedora",
    },
    "Wine": {
        "Linux": "sudo apt install wine   # Debian/Ubuntu\n  sudo dnf install wine   # Fedora",
    },
}


def require(package: str, binary: str) -> str:
    """Return the path to binary, or exit explaining how to install package."""
    path = shutil.which(binary)
    if path:
        return path
    hint = _INSTALL_HINTS.get(package, {}).get(SYSTEM)
    msg = f"error: '{binary}' not found."
    if hint:
        msg += f" Install it with:\n  {hint}"
    sys.exit(msg)


def qemu_system() -> str:
    return require("qemu", "qemu-system-i386")


def qemu_img() -> str:
    return require("qemu", "qemu-img")


def mtools(tool: str) -> str:
    """Path to one of the mtools programs (mcopy, mdel, mdir, ...)."""
    return require("mtools", tool)


def sevenzip() -> str:
    """7-Zip, which reads the Borland CDs (pycdlib rejects their path tables)."""
    return shutil.which("7zz") or require("7-Zip", "7z")


# Gcenx's macOS Wine build (what Homebrew's wine-stable cask installed before
# Homebrew disabled it for failing Gatekeeper checks). Intel-only: needs Rosetta.
_MACOS_WINE_URL = (
    "https://github.com/Gcenx/macOS_Wine_builds/releases/download/"
    "11.0_1/wine-stable-11.0_1-osx64.tar.xz"
)
_MACOS_WINE_SHA256 = "b50dc50ec7f41d58b115a6b685d4d1315ba3c797bd3aa0f49213f2703cb82388"
_MACOS_WINE_BIN = paths.WINE_DIST / "Wine Stable.app" / "Contents" / "Resources" / "wine" / "bin"


def _download_macos_wine() -> None:
    if platform.machine() == "arm64" and (
        subprocess.run(["arch", "-x86_64", "/usr/bin/true"], check=False).returncode != 0
    ):
        sys.exit(
            "error: Wine needs Rosetta 2. Install it with:\n"
            "  softwareupdate --install-rosetta --agree-to-license"
        )
    paths.WINE_DIST.mkdir(parents=True, exist_ok=True)
    archive = paths.WINE_DIST / "wine.tar.xz"
    print(f"Downloading Wine (~180 MB) from {_MACOS_WINE_URL}")
    urllib.request.urlretrieve(_MACOS_WINE_URL, archive)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != _MACOS_WINE_SHA256:
        archive.unlink()
        sys.exit(f"error: Wine download has SHA-256 {digest}, expected {_MACOS_WINE_SHA256}")
    with tarfile.open(archive) as tar:
        tar.extractall(paths.WINE_DIST, filter="tar")
    archive.unlink()


def wine_bin_dir() -> Path:
    """Directory holding the `wine`, `wineboot` and `wineserver` programs. On
    macOS, downloads the pinned Wine build on first use."""
    if SYSTEM == "Darwin":
        if not (_MACOS_WINE_BIN / "wine").exists():
            _download_macos_wine()
        return _MACOS_WINE_BIN
    return Path(require("Wine", "wine")).parent


def qemu_display_args(headless: bool = False) -> list[str]:
    if headless:
        return ["-display", "none"]
    return ["-display", "cocoa" if SYSTEM == "Darwin" else "gtk"]


def qemu_audiodev(dev_id: str) -> str:
    """QEMU -audiodev value for the host's native sound system."""
    driver = "coreaudio" if SYSTEM == "Darwin" else "pa"
    return f"{driver},id={dev_id}"
