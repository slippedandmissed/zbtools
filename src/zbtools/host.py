"""Everything that depends on the host operating system lives here.

Supported hosts: macOS (tested) and Linux (untested). Tools never install
host packages themselves; they check for them and print how to install them.
"""

import platform
import shutil
import sys

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


def qemu_display_args(headless: bool = False) -> list[str]:
    if headless:
        return ["-display", "none"]
    return ["-display", "cocoa" if SYSTEM == "Darwin" else "gtk"]


def qemu_audiodev(dev_id: str) -> str:
    """QEMU -audiodev value for the host's native sound system."""
    driver = "coreaudio" if SYSTEM == "Darwin" else "pa"
    return f"{driver},id={dev_id}"
