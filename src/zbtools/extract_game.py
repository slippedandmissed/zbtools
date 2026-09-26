"""Extract the game disc ISO and the Windows 95 build inside it.

Copies every file on the ISO into build/disc/, then unpacks the InstallShield
archive ZBARCHIV.Z (which holds zoombi32.exe) into build/zoombi32/.
"""

import argparse
import os
import shutil
from datetime import UTC, datetime, timedelta, timezone
from pathlib import Path

import pycdlib
from pycdlib.dr import DirectoryRecord

from zbtools import paths, unpack_isz


def _record_mtime(record: DirectoryRecord) -> float | None:
    d = record.date
    tz = timezone(timedelta(minutes=15 * d.gmtoffset)) if d.gmtoffset else UTC
    try:
        return datetime(
            1900 + d.years_since_1900,
            d.month,
            d.day_of_month,
            d.hour,
            d.minute,
            d.second,
            tzinfo=tz,
        ).timestamp()
    except ValueError:
        return None


def extract_iso(iso_path: Path, outdir: Path) -> int:
    """Copy all files from the ISO's ISO 9660 tree into outdir; returns the count."""
    iso = pycdlib.PyCdlib()
    iso.open(str(iso_path))
    try:
        count = 0
        for root, _, files in iso.walk(iso_path="/"):
            for name in files:
                src = f"{root.rstrip('/')}/{name}"
                dest = outdir / src.lstrip("/").split(";")[0]
                dest.parent.mkdir(parents=True, exist_ok=True)
                iso.get_file_from_iso(str(dest), iso_path=src)
                record = iso.get_record(iso_path=src)
                mtime = _record_mtime(record) if isinstance(record, DirectoryRecord) else None
                if mtime is not None:
                    os.utime(dest, (mtime, mtime))
                count += 1
    finally:
        iso.close()
    return count


class _Args(argparse.Namespace):
    iso: Path
    disc_dir: Path
    game_dir: Path


def main() -> None:
    doc = __doc__ or ""
    parser = argparse.ArgumentParser(description=doc.splitlines()[0])
    parser.add_argument(
        "--iso",
        type=Path,
        default=paths.GAME_ISO,
        help="game disc image (default: %(default)s)",
    )
    parser.add_argument(
        "--disc-dir",
        type=Path,
        default=paths.DISC_DIR,
        help="where to copy the disc contents (default: %(default)s)",
    )
    parser.add_argument(
        "--game-dir",
        type=Path,
        default=paths.GAME32_DIR,
        help="where to unpack the Windows 95 build (default: %(default)s)",
    )
    args = parser.parse_args(namespace=_Args())

    if not args.iso.is_file():
        parser.error(f"game ISO not found: {args.iso} (see README: Setup)")

    for d in (args.disc_dir, args.game_dir):
        shutil.rmtree(d, ignore_errors=True)

    count = extract_iso(args.iso, args.disc_dir)
    print(f"Extracted {count} files from {args.iso.name} into {args.disc_dir}")

    archive = args.disc_dir / "ZBARCHIV.Z"
    print(f"Unpacking {archive.name} into {args.game_dir}")
    unpack_isz.extract(archive.read_bytes(), args.game_dir)


if __name__ == "__main__":
    main()
