"""The game's resources as source: extracted from the disc's Mohawk archives
into assets/, converted to modern formats, and packed back into archives.

Each archive is a directory, assets/<name>/, holding `archive.toml` (where the
archive goes on the disc, and its resources in the order their data is stored)
and each resource in <type>/<id>.<extension>. `extract` writes them from the
disc, `pack` builds the archives from them into build/assets/ (laid out as on
the disc), and `verify` checks that packing reproduces the disc's archives byte
for byte.
"""

import re
import shutil
import tomllib
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated, Protocol

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import mohawk, paths

MANIFEST = "archive.toml"


class Format(Protocol):
    """How one type of resource is stored in assets/: as one or more files
    named after `stem` (<type>/<id>), converted from and back to its data."""

    def save(self, data: bytes, stem: Path) -> None: ...

    def load(self, stem: Path) -> bytes: ...


@dataclass(frozen=True)
class Raw:
    """The resource's data as it is, in <stem>.bin."""

    def save(self, data: bytes, stem: Path) -> None:
        stem.with_suffix(".bin").write_bytes(data)

    def load(self, stem: Path) -> bytes:
        return stem.with_suffix(".bin").read_bytes()


# Formats by resource type; types not listed are kept raw.
FORMATS: dict[bytes, Format] = {}


def format_of(tag: bytes) -> Format:
    return FORMATS.get(tag, Raw())


def type_name(tag: bytes) -> str:
    """A resource type as the manifest and directory names write it: without
    the NUL padding of three-letter types (b"\\0SND" is "SND")."""
    name = tag.strip(b"\0").decode("ascii")
    if not re.fullmatch(r"[A-Za-z0-9]+", name) or type_tag(name) != tag:
        raise ValueError(f"can't name resource type {tag!r}")
    return name


def type_tag(name: str) -> bytes:
    return name.encode("ascii").rjust(4, b"\0")


class Entry(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    type: str
    id: int
    purgeable: bool = False


class Manifest(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    path: str  # on the disc, e.g. DATA/BASECAMP.MHK
    resources: list[Entry]


def write_manifest(manifest: Manifest, path: Path) -> None:
    lines = [
        f"# Packed into {manifest.path} by `uv run assets pack`: its resources, in the",
        "# order their data is stored, each in <type>/<id>.<extension>.",
        f'path = "{manifest.path}"',
        "resources = [",
    ]
    for e in manifest.resources:
        purgeable = ", purgeable = true" if e.purgeable else ""
        lines.append(f'    {{ type = "{e.type}", id = {e.id}{purgeable} }},')
    lines.append("]")
    path.write_text("\n".join(lines) + "\n")


def read_manifest(path: Path) -> Manifest:
    return Manifest.model_validate(tomllib.loads(path.read_text()))


def disc_archives(disc_dir: Path) -> Iterator[Path]:
    """The Mohawk resource archives on the disc (in its root and DATA/)."""
    for directory in (disc_dir, disc_dir / "DATA"):
        for path in sorted(directory.iterdir()):
            if path.is_file():
                with path.open("rb") as f:
                    header = f.read(12)
                if header[:4] == b"MHWK" and header[8:12] == b"RSRC":
                    yield path


def extract_archive(archive: Path, disc_dir: Path, outdir: Path) -> int:
    """Writes an archive's resources and manifest into outdir; returns the count."""
    resources = mohawk.read(archive.read_bytes())
    entries = []
    for r in resources:
        name = type_name(r.type)
        (outdir / name).mkdir(parents=True, exist_ok=True)
        format_of(r.type).save(r.data, outdir / name / str(r.id))
        entries.append(Entry(type=name, id=r.id, purgeable=r.purgeable))
    manifest = Manifest(path=archive.relative_to(disc_dir).as_posix(), resources=entries)
    write_manifest(manifest, outdir / MANIFEST)
    return len(resources)


def pack_archive(directory: Path) -> tuple[Manifest, list[mohawk.Resource]]:
    """An archive's manifest and resources, loaded from its directory in assets/."""
    manifest = read_manifest(directory / MANIFEST)
    resources = []
    for e in manifest.resources:
        tag = type_tag(e.type)
        data = format_of(tag).load(directory / e.type / str(e.id))
        resources.append(mohawk.Resource(tag, e.id, data, e.purgeable))
    return manifest, resources


def asset_archives(assets_dir: Path) -> list[Path]:
    found = sorted(p.parent for p in assets_dir.glob(f"*/{MANIFEST}"))
    if not found:
        raise typer.BadParameter(f"no archives in {assets_dir} (run `uv run assets extract`)")
    return found


app = typer.Typer(add_completion=False, help=__doc__)

DiscOption = Annotated[Path, typer.Option(help="The extracted disc (uv run extract-game)")]
AssetsOption = Annotated[Path, typer.Option(help="The converted resources")]


@app.command()
def extract(
    disc_dir: DiscOption = paths.DISC_DIR,
    assets_dir: AssetsOption = paths.ASSETS_DIR,
    force: Annotated[
        bool, typer.Option(help="Replace archives already in assets/, discarding edits")
    ] = False,
) -> None:
    """Extract the disc's archives into assets/, converting their resources."""
    if not disc_dir.is_dir():
        raise typer.BadParameter(f"{disc_dir} not found (run `uv run extract-game`)")
    archives = list(disc_archives(disc_dir))
    existing = [a for a in archives if (assets_dir / a.stem).exists()]
    if existing and not force:
        names = ", ".join(a.stem for a in existing)
        raise typer.BadParameter(f"already in {assets_dir}: {names} (--force replaces them)")
    for archive in archives:
        outdir = assets_dir / archive.stem
        shutil.rmtree(outdir, ignore_errors=True)
        count = extract_archive(archive, disc_dir, outdir)
        print(f"{archive.relative_to(disc_dir)}: {count} resources")


@app.command()
def pack(
    assets_dir: AssetsOption = paths.ASSETS_DIR,
    out_dir: Annotated[
        Path, typer.Option(help="Where to write the archives, laid out as on the disc")
    ] = paths.PACKED_ASSETS_DIR,
) -> None:
    """Pack assets/ into Mohawk archives."""
    for directory in asset_archives(assets_dir):
        manifest, resources = pack_archive(directory)
        out = out_dir / manifest.path
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(mohawk.write(resources))
        print(f"{manifest.path}: {len(resources)} resources")


@app.command()
def verify(
    disc_dir: DiscOption = paths.DISC_DIR,
    assets_dir: AssetsOption = paths.ASSETS_DIR,
) -> None:
    """Check that packing assets/ reproduces the disc's archives exactly."""
    failed = 0
    for directory in asset_archives(assets_dir):
        manifest, resources = pack_archive(directory)
        original = disc_dir / manifest.path
        if mohawk.write(resources) == original.read_bytes():
            print(f"{manifest.path}: identical")
            continue
        failed += 1
        print(f"{manifest.path}: differs")
        try:
            before = {(r.type, r.id): r for r in mohawk.read(original.read_bytes())}
        except mohawk.FormatError as e:
            print(f"  (can't read the original: {e})")
            continue
        after = {(r.type, r.id): r for r in resources}
        for key in sorted(before.keys() | after.keys()):
            old, new = before.get(key), after.get(key)
            what = "added" if old is None else "removed" if new is None else None
            if what is None and old != new:
                what = "changed"
            if what:
                print(f"  {type_name(key[0])} {key[1]}: {what}")
        if list(before) != list(after):
            print("  (the resources' order differs)")
    if failed:
        raise typer.Exit(1)
