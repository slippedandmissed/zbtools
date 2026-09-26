"""Default locations of bring-your-own inputs and build outputs."""
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

DATA_DIR = REPO_ROOT / "data"
GAME_ISO = DATA_DIR / "Logical Journey of the Zoombinis.iso"
WINDOWS_ISO = DATA_DIR / "Windows 98 Second Edition.iso"

BUILD_DIR = REPO_ROOT / "build"
DISC_DIR = BUILD_DIR / "disc"
GAME32_DIR = BUILD_DIR / "zoombi32"

# Everything the tooling generates, removed by `uv run clean`. Add any new
# generated path here; bring-your-own inputs in data/ and .env are never listed.
GENERATED = [
    BUILD_DIR,
    REPO_ROOT / ".venv",
]
