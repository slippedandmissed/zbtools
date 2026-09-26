"""Extract the game disc ISO and the Windows 95 build inside it.

Copies every file on the ISO into build/disc/, then unpacks the InstallShield
archive ZBARCHIV.Z (which holds zoombi32.exe) into build/zoombi32/.
"""

import os
import shutil
from datetime import UTC, datetime, timedelta, timezone
from pathlib import Path
from typing import Annotated

import pycdlib
import typer
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


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    iso: Annotated[Path, typer.Option(help="Game disc image")] = paths.GAME_ISO,
    disc_dir: Annotated[
        Path, typer.Option(help="Where to copy the disc contents")
    ] = paths.DISC_DIR,
    game_dir: Annotated[
        Path, typer.Option(help="Where to unpack the Windows 95 build")
    ] = paths.GAME32_DIR,
) -> None:
    if not iso.is_file():
        raise typer.BadParameter(
            f"game ISO not found: {iso} (see README: Setup)", param_hint="--iso"
        )

    for d in (disc_dir, game_dir):
        shutil.rmtree(d, ignore_errors=True)

    count = extract_iso(iso, disc_dir)
    print(f"Extracted {count} files from {iso.name} into {disc_dir}")

    archive = disc_dir / "ZBARCHIV.Z"
    print(f"Unpacking {archive.name} into {game_dir}")
    unpack_isz.extract(archive.read_bytes(), game_dir)
