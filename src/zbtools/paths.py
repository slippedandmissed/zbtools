"""Default locations of bring-your-own inputs and build outputs."""

from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

DATA_DIR = REPO_ROOT / "data"
GAME_ISO = DATA_DIR / "Logical Journey of the Zoombinis.iso"
WINDOWS_ISO = DATA_DIR / "Windows 98 Second Edition.iso"
# Borland C++ CDs, by release. The game was built with 4.5 or 4.52.
BORLAND_ISOS = {
    "4.5": DATA_DIR / "Borland C++ 4.5.iso",
    "4.52": DATA_DIR / "Borland C++ 4.52.iso",
}
ENV_FILE = REPO_ROOT / ".env"

BUILD_DIR = REPO_ROOT / "build"
DISC_DIR = BUILD_DIR / "disc"
GAME32_DIR = BUILD_DIR / "zoombi32"

# Borland C++ BIN, LIB and INCLUDE, one directory per release.
TOOLCHAIN_DIR = BUILD_DIR / "toolchain"
# Wine: the downloaded build (macOS only) and the prefix the compilers run in.
WINE_DIR = BUILD_DIR / "wine"
WINE_DIST = WINE_DIR / "dist"
WINE_PREFIX = WINE_DIR / "prefix"

VM_DIR = BUILD_DIR / "vm"
# Pristine Windows 98 install, written once by `vm install` and never modified.
WIN98_BASE = VM_DIR / "win98-base.qcow2"
# Where `vm install` builds the base; renamed to WIN98_BASE once setup succeeds.
WIN98_BASE_PARTIAL = VM_DIR / "win98-base.partial.qcow2"
# QuickTime and the game installed on top of the base by `vm install-game`.
WIN98_GAME = VM_DIR / "win98-game.qcow2"
WIN98_GAME_PARTIAL = VM_DIR / "win98-game.partial.qcow2"
# Copy-on-write layer the VM actually runs from, on top of the game layer (or
# the base, if the game isn't installed).
WIN98_OVERLAY = VM_DIR / "win98.qcow2"
# CD image that `vm install-game` builds to install QuickTime and the game.
VM_TOOLS_ISO = VM_DIR / "tools.iso"
WIN98_SETUP_FLOPPY = VM_DIR / "win98-setup.img"
# QMP sockets: one for the command that started the VM, one for other commands
# (a QMP socket serves one client at a time).
VM_QMP = VM_DIR / "qmp.sock"
VM_QMP_CONTROL = VM_DIR / "qmp-control.sock"
# Scratch screenshot the VM commands take to recognise what is on screen.
VM_SCREEN_CHECK = VM_DIR / "screen-check.png"

# An entry in a clean category: a path (may contain * wildcards, matched from
# the repo root) or the name of another category.
type CleanEntry = Path | str

# Categories of generated files for `uv run clean`. Every path the tooling
# generates must belong to a category. Bring-your-own inputs (data/, .env) are
# never listed.
CLEAN_CATEGORIES: dict[str, list[CleanEntry]] = {
    "extracted": [DISC_DIR, GAME32_DIR],
    "vm-state": [
        WIN98_OVERLAY,
        WIN98_BASE_PARTIAL,
        WIN98_GAME_PARTIAL,
        WIN98_SETUP_FLOPPY,
        VM_TOOLS_ISO,
        VM_QMP,
        VM_QMP_CONTROL,
        VM_SCREEN_CHECK,
    ],
    # Each disk layer includes the layers built on top of it.
    "vm-game": [WIN98_GAME, WIN98_OVERLAY],
    "vm-base": [WIN98_BASE, "vm-game"],
    "vm": ["vm-base", "vm-state"],
    "toolchain": [TOOLCHAIN_DIR, WINE_PREFIX],
    "wine": [WINE_DIR],
    "python": [REPO_ROOT / ".venv", REPO_ROOT / "src" / "**" / "__pycache__"],
    # The whole build/ directory, so stray files can't survive a full clean.
    "all": [BUILD_DIR, "python"],
}
# What `uv run clean` removes with no arguments: everything cheap to rebuild.
CLEAN_DEFAULT: list[str] = ["extracted", "vm-state", "toolchain", "python"]
