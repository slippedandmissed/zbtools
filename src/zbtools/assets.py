"""The game's resources as source: extracted from the disc's Mohawk archives
into assets/, converted to modern formats, and packed back into archives.

Each archive is a directory, assets/<name>/, holding `archive.toml` (where the
archive goes on the disc, and its resources in the order their data is stored)
and each resource in <type>/<id>.<extension>: in a modern format where the
type has one (zbtools.formats), else as it is (.bin); only ever one copy.
`extract` writes them from the disc, `pack` builds the archives from them into
build/assets/ (laid out as on the disc), and `verify` checks that packing
reproduces the disc's archives byte for byte.

The executable's own resources (its icon) go in assets/zoombi32/ the same
way (exe_resources.py): `extract` writes them and `verify` checks them; `uv
run build` compiles them into the rebuilt executable.
"""

import re
import shutil
import tomllib
from collections.abc import Iterator
from concurrent.futures import ProcessPoolExecutor
from functools import partial
from pathlib import Path
from typing import Annotated

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import exe_resources, mohawk, paths
from zbtools.exe import Executable
from zbtools.formats import FORMATS, Raw, Unconvertible
from zbtools.formats.base import RAW_SUFFIX

MANIFEST = "archive.toml"


def save_resource(tag: bytes, data: bytes, stem: Path, archive: list[mohawk.Resource]) -> bool:
    """Writes a resource into assets/, converted if its format gives back
    exactly the same data from what it wrote, else raw; whether it was
    converted."""
    if tag in FORMATS:
        try:
            FORMATS[tag].save(data, stem, archive)
            if FORMATS[tag].load(stem) == data:
                return True
        except Unconvertible:
            pass
        _remove(stem)
    Raw().save(data, stem)
    return False


def _remove(stem: Path) -> None:
    """Removes a resource's files: <stem>.* and a directory <stem>."""
    for path in stem.parent.glob(f"{stem.name}.*"):
        path.unlink()
    shutil.rmtree(stem, ignore_errors=True)


def load_resource(tag: bytes, stem: Path) -> bytes:
    """A resource's data, from its file(s) in assets/: raw if there's a .bin."""
    raw = stem.with_suffix(RAW_SUFFIX)
    if tag not in FORMATS or raw.exists():
        if tag in FORMATS and any(p != raw for p in stem.parent.glob(f"{stem.name}.*")):
            raise typer.BadParameter(f"{stem}: stored both raw and converted; keep one")
        return Raw().load(stem)
    return FORMATS[tag].load(stem)


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


def extract_archive(archive: Path, disc_dir: Path, outdir: Path) -> tuple[int, int]:
    """Writes an archive's resources and manifest into outdir; returns how many
    resources it has, and how many of them were kept raw."""
    resources = mohawk.read(archive.read_bytes())
    entries = []
    raw = 0
    for r in resources:
        name = type_name(r.type)
        (outdir / name).mkdir(parents=True, exist_ok=True)
        raw += not save_resource(r.type, r.data, outdir / name / str(r.id), resources)
        entries.append(Entry(type=name, id=r.id, purgeable=r.purgeable))
    manifest = Manifest(path=archive.relative_to(disc_dir).as_posix(), resources=entries)
    write_manifest(manifest, outdir / MANIFEST)
    return len(resources), raw


def pack_archive(directory: Path) -> tuple[Manifest, list[mohawk.Resource]]:
    """An archive's manifest and resources, loaded from its directory in assets/."""
    manifest = read_manifest(directory / MANIFEST)
    resources = []
    for e in manifest.resources:
        tag = type_tag(e.type)
        data = load_resource(tag, directory / e.type / str(e.id))
        resources.append(mohawk.Resource(tag, e.id, data, e.purgeable))
    return manifest, resources


def asset_archives(assets_dir: Path) -> list[Path]:
    found = sorted(p.parent for p in assets_dir.glob(f"*/{MANIFEST}"))
    if not found:
        raise typer.BadParameter(f"no archives in {assets_dir} (run `uv run assets extract`)")
    return found


def _extract(archive: Path, disc_dir: Path, assets_dir: Path) -> str:
    outdir = assets_dir / archive.stem
    shutil.rmtree(outdir, ignore_errors=True)
    count, raw = extract_archive(archive, disc_dir, outdir)
    return f"{archive.relative_to(disc_dir)}: {count} resources ({raw} raw)"


def _pack(directory: Path, out_dir: Path) -> str:
    manifest, resources = pack_archive(directory)
    out = out_dir / manifest.path
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(mohawk.write(resources))
    return f"{manifest.path}: {len(resources)} resources"


def _verify(directory: Path, disc_dir: Path) -> list[str]:
    """What packing an archive gives, compared with the disc's: an empty list
    if they're identical, else what differs."""
    manifest, resources = pack_archive(directory)
    original = (disc_dir / manifest.path).read_bytes()
    if mohawk.write(resources) == original:
        return []
    report = [f"{manifest.path}: differs"]
    try:
        before = {(r.type, r.id): r for r in mohawk.read(original)}
    except mohawk.FormatError as e:
        return [*report, f"  (can't read the original: {e})"]
    after = {(r.type, r.id): r for r in resources}
    for key in sorted(before.keys() | after.keys()):
        old, new = before.get(key), after.get(key)
        what = "added" if old is None else "removed" if new is None else None
        if what is None and old != new:
            what = "changed"
        if what:
            report.append(f"  {type_name(key[0])} {key[1]}: {what}")
    if list(before) != list(after):
        report.append("  (the resources' order differs)")
    return report


app = typer.Typer(add_completion=False, help=__doc__)

DiscOption = Annotated[Path, typer.Option(help="The extracted disc (uv run extract-game)")]
ExeOption = Annotated[Path, typer.Option(help="The game's executable (uv run extract-game)")]
AssetsOption = Annotated[Path, typer.Option(help="The converted resources")]


@app.command()
def extract(
    disc_dir: DiscOption = paths.DISC_DIR,
    assets_dir: AssetsOption = paths.ASSETS_DIR,
    exe: ExeOption = paths.GAME32_DIR / "zoombi32.exe",
    force: Annotated[
        bool, typer.Option(help="Replace archives already in assets/, discarding edits")
    ] = False,
) -> None:
    """Extract the disc's archives and the executable's resources into assets/,
    converting them."""
    if not disc_dir.is_dir() or not exe.is_file():
        raise typer.BadParameter(f"{disc_dir} or {exe} not found (run `uv run extract-game`)")
    archives = list(disc_archives(disc_dir))
    exe_dir = assets_dir / exe_resources.ASSETS.name
    existing = [a.stem for a in archives if (assets_dir / a.stem).exists()]
    existing += [exe_dir.name] if exe_dir.exists() else []
    if existing and not force:
        names = ", ".join(existing)
        raise typer.BadParameter(f"already in {assets_dir}: {names} (--force replaces them)")
    with ProcessPoolExecutor() as pool:
        for line in pool.map(partial(_extract, disc_dir=disc_dir, assets_dir=assets_dir), archives):
            print(line)
    shutil.rmtree(exe_dir, ignore_errors=True)
    count = exe_resources.extract(Executable(exe), exe_dir)
    print(f"{exe.name}: {count} resources")


@app.command()
def pack(
    assets_dir: AssetsOption = paths.ASSETS_DIR,
    out_dir: Annotated[
        Path, typer.Option(help="Where to write the archives, laid out as on the disc")
    ] = paths.PACKED_ASSETS_DIR,
) -> None:
    """Pack assets/ into Mohawk archives."""
    with ProcessPoolExecutor() as pool:
        for line in pool.map(partial(_pack, out_dir=out_dir), asset_archives(assets_dir)):
            print(line)


@app.command()
def verify(
    disc_dir: DiscOption = paths.DISC_DIR,
    assets_dir: AssetsOption = paths.ASSETS_DIR,
    exe: ExeOption = paths.GAME32_DIR / "zoombi32.exe",
) -> None:
    """Check that packing assets/ reproduces the disc's archives, and the
    executable's resources, exactly."""
    directories = asset_archives(assets_dir)
    failed = 0
    with ProcessPoolExecutor() as pool:
        for directory, report in zip(
            directories, pool.map(partial(_verify, disc_dir=disc_dir), directories), strict=True
        ):
            if report:
                failed += 1
                print("\n".join(report))
            else:
                print(f"{directory.name}: identical")
    report = exe_resources.verify(Executable(exe), assets_dir / exe_resources.ASSETS.name)
    if report:
        failed += 1
        print("\n".join([f"{exe.name} resources: differ", *report]))
    else:
        print(f"{exe.name} resources: identical")
    if failed:
        raise typer.Exit(1)
