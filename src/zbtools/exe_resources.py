"""The executable's resources as source: its icons, in assets/zoombi32/.

zoombi32.exe has one icon group (`APPICON`, the program's icon) of two
images. They're stored as `ICON/<id>.png` (formats/icon.py), with
`resources.toml` naming the groups and the ids of their images; the groups'
own data is derived from the images. `uv run assets extract` writes them,
`uv run assets verify` checks that they rebuild the executable's resources
exactly, and `uv run build` compiles them into the rebuilt executable (as
.ico files and a resource script, `resource_script`).
"""

import tomllib
from pathlib import Path

from pydantic import BaseModel, ConfigDict

from zbtools import paths
from zbtools.exe import Executable, PeResource
from zbtools.formats import icon
from zbtools.formats.base import Unconvertible

ASSETS = paths.ASSETS_DIR / "zoombi32"
MANIFEST = "resources.toml"
ICON = 3
GROUP_ICON = 14
_ICON_DIR = "ICON"


class IconGroup(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    name: str | int
    language: int
    icons: list[int]  # the ICON resources' ids, each in ICON/<id>.png


class Manifest(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    icon_groups: list[IconGroup]


def _group_ids(data: bytes) -> list[int]:
    """The ICON ids a GROUP_ICON resource lists."""
    count = int.from_bytes(data[4:6], "little")
    return [int.from_bytes(data[6 + 14 * i + 12 : 6 + 14 * i + 14], "little") for i in range(count)]


def extract(exe: Executable, outdir: Path = ASSETS) -> int:
    """Writes the executable's resources into outdir; returns how many."""
    kinds = {r.type for r in exe.resources}
    if not kinds <= {ICON, GROUP_ICON}:
        raise ValueError(f"resources of types {sorted(map(str, kinds))}: only icons are supported")
    icons = {(r.name, r.language): r.data for r in exe.resources if r.type == ICON}
    groups = []
    (outdir / _ICON_DIR).mkdir(parents=True, exist_ok=True)
    for r in (r for r in exe.resources if r.type == GROUP_ICON):
        ids = _group_ids(r.data)
        images = [(i, icon.parse(icons[(i, r.language)])) for i in ids]
        if icon.group(images) != r.data:
            raise Unconvertible(f"icon group {r.name} isn't derivable from its images")
        for i, image in images:
            icon.save_png(image, outdir / _ICON_DIR / f"{i}{icon.SUFFIX}")
        groups.append(IconGroup(name=r.name, language=r.language, icons=ids))
    write_manifest(Manifest(icon_groups=groups), outdir / MANIFEST)
    return len(exe.resources)


def write_manifest(manifest: Manifest, path: Path) -> None:
    lines = [
        "# Compiled into zoombi32.exe by `uv run build`: its icon groups, each a list of",
        "# icon images in ICON/<id>.png.",
    ]
    for g in manifest.icon_groups:
        name = f'"{g.name}"' if isinstance(g.name, str) else str(g.name)
        lines += ["", "[[icon_groups]]", f"name = {name}", f"language = {g.language}"]
        lines.append(f"icons = [{', '.join(map(str, g.icons))}]")
    path.write_text("\n".join(lines) + "\n")


def read_manifest(directory: Path = ASSETS) -> Manifest:
    path = directory / MANIFEST
    if not path.exists():
        raise FileNotFoundError(f"{path} not found (run `uv run assets extract`)")
    return Manifest.model_validate(tomllib.loads(path.read_text()))


def _icons(directory: Path, group: IconGroup) -> list[tuple[int, icon.Icon]]:
    return [(i, icon.load_png(directory / _ICON_DIR / f"{i}{icon.SUFFIX}")) for i in group.icons]


def app_icon(directory: Path = ASSETS) -> list[icon.Icon]:
    """The program's icon (the first icon group): its images, largest first."""
    images = [image for _, image in _icons(directory, read_manifest(directory).icon_groups[0])]
    return sorted(images, key=lambda image: -image.width * image.height)


def load(directory: Path = ASSETS) -> list[PeResource]:
    """The resources, rebuilt from assets/."""
    found = []
    for g in read_manifest(directory).icon_groups:
        images = _icons(directory, g)
        found += [PeResource(ICON, i, g.language, icon.to_bytes(image)) for i, image in images]
        found.append(PeResource(GROUP_ICON, g.name, g.language, icon.group(images)))
    return found


def verify(exe: Executable, directory: Path = ASSETS) -> list[str]:
    """What differs between the resources rebuilt from assets/ and the
    executable's: an empty list if nothing does."""
    ours = {(r.type, r.name, r.language): r.data for r in load(directory)}
    theirs = {(r.type, r.name, r.language): r.data for r in exe.resources}
    report = []
    for key in sorted(ours.keys() | theirs.keys(), key=str):
        kind = {ICON: "ICON", GROUP_ICON: "GROUP_ICON"}.get(int(key[0]), str(key[0]))
        if key not in theirs:
            report.append(f"  {kind} {key[1]}: added")
        elif key not in ours:
            report.append(f"  {kind} {key[1]}: missing")
        elif ours[key] != theirs[key]:
            report.append(f"  {kind} {key[1]}: changed")
    return report


def resource_script(out_dir: Path, directory: Path = ASSETS) -> str:
    """A resource script (.rc) for BRCC32, writing the .ico files it names
    into out_dir. BRCC32 numbers an ICON statement's images from 1 in the
    order of the .ico file, as the original's are."""
    lines = []
    for g in read_manifest(directory).icon_groups:
        images = _icons(directory, g)
        if [i for i, _ in images] != list(range(1, len(images) + 1)):
            raise ValueError(f"icon group {g.name}: BRCC32 would number its images 1 up")
        ico = out_dir / f"{g.name}.ico"
        ico.write_bytes(icon.ico_file([image for _, image in images]))
        lines.append(f'{g.name} ICON "{ico.name}"')
    return "\n".join(lines) + "\n"
