"""Pinned, checksum-verified downloads of the tools the project fetches itself."""

import hashlib
import sys
import tarfile
import urllib.request
import zipfile
from pathlib import Path


def fetch(url: str, sha256: str, dest: Path) -> None:
    """Download url to dest, exiting unless it has the expected SHA-256."""
    dest.parent.mkdir(parents=True, exist_ok=True)
    partial = dest.with_name(dest.name + ".part")
    digest = hashlib.sha256()
    print(f"Downloading {url}")
    with urllib.request.urlopen(url) as response, partial.open("wb") as out:
        while chunk := response.read(1 << 20):
            digest.update(chunk)
            out.write(chunk)
    if digest.hexdigest() != sha256:
        partial.unlink()
        sys.exit(f"error: {url} has SHA-256 {digest.hexdigest()}, expected {sha256}")
    partial.replace(dest)


def extract_zip(archive: Path, dest: Path) -> None:
    """Extract a zip, keeping Unix file permissions (zipfile alone drops them,
    losing the executable bits on programs)."""
    with zipfile.ZipFile(archive) as zf:
        for info in zf.infolist():
            path = Path(zf.extract(info, dest))
            mode = (info.external_attr >> 16) & 0o777
            if mode and not info.is_dir():
                path.chmod(mode)


def extract_tar(archive: Path, dest: Path) -> None:
    with tarfile.open(archive) as tar:
        tar.extractall(dest, filter="tar")
