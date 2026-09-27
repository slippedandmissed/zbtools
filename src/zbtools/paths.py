"""Default locations of bring-your-own inputs and build outputs."""

from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

DATA_DIR = REPO_ROOT / "data"
GAME_ISO = DATA_DIR / "Logical Journey of the Zoombinis.iso"
WINDOWS_ISO = DATA_DIR / "Windows 98 Second Edition.iso"
# Borland C++ CDs, by release. The game was built with 4.5 or 4.52; 5.02 is
# optional, for comparison (it fits the Mohawk engine worse than 4.5 does).
BORLAND_ISOS = {
    "4.5": DATA_DIR / "Borland C++ 4.5.iso",
    "4.52": DATA_DIR / "Borland C++ 4.52.iso",
    "5.02": DATA_DIR / "Borland C++ 5.02.iso",
}
ENV_FILE = REPO_ROOT / ".env"

BUILD_DIR = REPO_ROOT / "build"
DISC_DIR = BUILD_DIR / "disc"
GAME32_DIR = BUILD_DIR / "zoombi32"

# Decompiled C source, checked against the game by `uv run match`.
DECOMP_DIR = REPO_ROOT / "decomp"
# Names `uv run runtime-symbols` found for the Borland runtime code in the game.
SYMBOLS_DIR = BUILD_DIR / "symbols"
RUNTIME_SYMBOLS = SYMBOLS_DIR / "runtime.json"
# Classes `uv run classes` recovered from the game's RTTI.
CLASSES = SYMBOLS_DIR / "classes.json"
# The progress report `uv run report` writes (local only: it contains disassembly).
REPORT_DIR = BUILD_DIR / "report"
REPORT = REPORT_DIR / "index.html"
# Objects `uv run match` compiles, kept to reuse while their sources don't
# change: one directory per release.
MATCH_CACHE = BUILD_DIR / "match-cache"

# Borland C++ BIN, LIB and INCLUDE, one directory per release.
TOOLCHAIN_DIR = BUILD_DIR / "toolchain"
# Ghidra: the downloaded release, and the project holding the analysed game.
# The project also holds any work done in Ghidra's GUI (names, comments), so
# no default `clean` removes it.
GHIDRA_DIR = BUILD_DIR / "ghidra"
GHIDRA_DIST = GHIDRA_DIR / "dist"
GHIDRA_PROJECT_DIR = GHIDRA_DIR / "project"
GHIDRA_PROJECT_NAME = "zoombinis"
# Every function Ghidra found, exported by `uv run ghidra setup`.
GHIDRA_FUNCTIONS = GHIDRA_DIR / "functions.json"

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
    "extracted": [DISC_DIR, GAME32_DIR, SYMBOLS_DIR],
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
    "match-cache": [MATCH_CACHE],
    # Objects compiled by a toolchain are no use without it.
    "toolchain": [TOOLCHAIN_DIR, WINE_PREFIX, "match-cache"],
    "wine": [WINE_DIR],
    # Includes any work done in Ghidra's GUI: only removed when asked for.
    "ghidra-project": [GHIDRA_PROJECT_DIR, GHIDRA_FUNCTIONS],
    "ghidra": [GHIDRA_DIR],
    "report": [REPORT_DIR],
    "python": [REPO_ROOT / ".venv", REPO_ROOT / "src" / "**" / "__pycache__"],
    # The whole build/ directory, so stray files can't survive a full clean.
    "all": [BUILD_DIR, "python"],
}
# What `uv run clean` removes with no arguments: everything cheap to rebuild.
CLEAN_DEFAULT: list[str] = ["extracted", "vm-state", "toolchain", "report", "python"]
