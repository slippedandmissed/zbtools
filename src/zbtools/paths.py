"""Default locations of bring-your-own inputs and build outputs."""

from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

DATA_DIR = REPO_ROOT / "data"
GAME_ISO = DATA_DIR / "Logical Journey of the Zoombinis.iso"
WINDOWS_ISO = DATA_DIR / "Windows 98 Second Edition.iso"
ENV_FILE = REPO_ROOT / ".env"

BUILD_DIR = REPO_ROOT / "build"
DISC_DIR = BUILD_DIR / "disc"
GAME32_DIR = BUILD_DIR / "zoombi32"

VM_DIR = BUILD_DIR / "vm"
# Pristine Windows 98 install, written once by `vm install` and never modified.
WIN98_BASE = VM_DIR / "win98-base.qcow2"
# Where `vm install` builds the base; renamed to WIN98_BASE once setup succeeds.
WIN98_BASE_PARTIAL = VM_DIR / "win98-base.partial.qcow2"
# Copy-on-write layer on top of the base that the VM actually runs from.
WIN98_OVERLAY = VM_DIR / "win98.qcow2"
WIN98_SETUP_FLOPPY = VM_DIR / "win98-setup.img"
# QMP sockets: one for the command that started the VM, one for other commands
# (a QMP socket serves one client at a time).
VM_QMP = VM_DIR / "qmp.sock"
VM_QMP_CONTROL = VM_DIR / "qmp-control.sock"
# Scratch screenshot `vm install` uses to spot the Windows logon prompt.
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
        WIN98_SETUP_FLOPPY,
        VM_QMP,
        VM_QMP_CONTROL,
        VM_SCREEN_CHECK,
    ],
    "vm-base": [WIN98_BASE],
    "vm": ["vm-base", "vm-state"],
    "python": [REPO_ROOT / ".venv", REPO_ROOT / "src" / "**" / "__pycache__"],
    # The whole build/ directory, so stray files can't survive a full clean.
    "all": [BUILD_DIR, "python"],
}
# What `uv run clean` removes with no arguments: everything cheap to rebuild.
CLEAN_DEFAULT: list[str] = ["extracted", "vm-state", "python"]
