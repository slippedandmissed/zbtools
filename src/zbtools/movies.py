"""The game's movies as source: each `DATA/*.MOV` on the disc converted into
assets/movies/<NAME>/ and packed back into a `.MOV` that's the disc's, byte for
byte (`uv run assets` extracts, packs and verifies them with the archives).

A movie's video is scenes, not pictures (`formats/qkbk.py`): the directory
holds

- `movie.toml`: where the file goes on the disc, the container's facts that
  can't be derived (`formats/mov.py`) and the scene's constants;
- `frames.toml`: each frame's sprites and other fields, and which cast items
  it defines;
- `palettes.toml`: the palettes, cast items like the bitmaps;
- `bitmaps.toml` and `casts/<id>.png`: the sprite images, 8-bit PNGs whose
  colour 0 is transparent (shown in the first frame's palette; the movie's
  palettes are what counts), each with the header words nobody understands yet;
- `sound.wav`: the sound (8-bit; QuickTime's `twos` is signed, WAV's unsigned).
"""

import shutil
import tomllib
from collections.abc import Iterable, Sequence
from concurrent.futures import ProcessPoolExecutor
from functools import partial
from pathlib import Path

from PIL import Image
from pydantic import BaseModel, ConfigDict

from zbtools.formats import mov, qkbk, scene, sound
from zbtools.formats.base import Unconvertible

DIRECTORY = "movies"  # in assets/
MANIFEST = "movie.toml"
FRAMES = "frames.toml"
PALETTES = "palettes.toml"
BITMAPS = "bitmaps.toml"
CASTS = "casts"
SOUND = "sound.wav"

_SIGNED_TO_UNSIGNED = bytes((i + 128) % 256 for i in range(256))
_PER_LINE = 8  # colours and chunks


class Scene(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    slots: int  # sprite slots in every frame
    f23: int  # a byte of every frame's header, the same in all


class Manifest(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    path: str  # on the disc, e.g. DATA/LOGO025.MOV
    scene: Scene
    container: mov.MovInfo


# Writing TOML (by hand: no library writes it in the layout we want)


def _value(value: object) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, int):
        return str(value)
    if isinstance(value, str):
        return f'"{value}"'
    if isinstance(value, Sequence):
        return "[" + ", ".join(_value(v) for v in value) + "]"
    raise TypeError(f"can't write {value!r}")


def _rows(name: str, items: Sequence[object], per_line: int) -> list[str]:
    lines = [f"{name} = ["]
    for i in range(0, len(items), per_line):
        lines.append("    " + ", ".join(_value(v) for v in items[i : i + per_line]) + ",")
    return [*lines, "]"]


def manifest_toml(manifest: Manifest) -> str:
    c = manifest.container
    lines = [
        f"# Packed into {manifest.path} by `uv run assets pack`: the container's facts that",
        "# can't be derived from the frames and the sound, and the scene's constants.",
        f"path = {_value(manifest.path)}",
        "",
        "[scene]",
        f"slots = {manifest.scene.slots}  # sprite slots in every frame",
        f"f23 = {manifest.scene.f23}  # a byte of every frame's header; what it means is unknown",
        "",
        "[container]",
        f"movie = {_value(c.movie)}  # created and modified, in seconds since 1904",
        f"volume = {c.volume}",
        f"timescale = {c.timescale}  # units a second",
        f"video_track = {_value(c.video_track)}",
        f"video_media = {_value(c.video_media)}",
        f"width = {c.width}",
        f"height = {c.height}",
        f"frame_duration = {c.frame_duration}  # units",
        f"audio_track = {_value(c.audio_track)}",
        f"audio_media = {_value(c.audio_media)}",
        f"rate = {c.rate}  # sound samples a second",
        f"audio_edits = {_value(c.audio_edits)}  # duration, media time (-1: nothing plays)",
        f'head = "{c.head.hex()}"  # mdat starts with these bytes, which nothing refers to',
        *_rows("sync", c.sync, _PER_LINE * 2),
        "# mdat's chunks in order: v<frames>, a<samples> or x<hex> (bytes nothing refers to)",
        *_rows("chunks", c.chunks, _PER_LINE * 2),
    ]
    return "\n".join(lines) + "\n"


def frames_toml(frames: list[qkbk.Frame]) -> str:
    lines = [
        "# The movie's frames in order (the nth is frame n). A frame draws its sprites,",
        "# [slot, cast, x, y], in slot order on its background colour, in `palette`;",
        "# `casts` are the cast items it defines (in palettes.toml and casts/). The rects",
        "# [top, left, bottom, right] are for the codec's other blocks; what a frame",
        "# changes, and its bounds, are worked out from its sprites.",
    ]
    for frame in frames:
        lines += [
            "",
            "[[frame]]",
            f"f4 = {frame.f4}",
            f"f6 = {frame.f6}",
            f"fill = {frame.fill}",
            f"flags = {frame.flags}  # 1: the last frame, 8: keep the background",
            f"palette = {frame.palette}",
        ]
        if frame.xfrm is not None:
            lines.append(f"xfrm = {_value(frame.xfrm)}")
        if frame.back is not None:
            lines.append(f"back = {_value(frame.back)}")
        if frame.front is not None:
            lines.append(f"front = {_value(frame.front)}")
        if frame.casts:
            lines += _rows("casts", frame.casts, _PER_LINE)
        if frame.sprites:
            lines += _rows("sprites", [tuple(s) for s in frame.sprites], 3)
        if frame.tail:
            lines.append(f'tail = "{frame.tail.hex()}"')
    return "\n".join(lines) + "\n"


class _Frames(BaseModel):
    model_config = ConfigDict(extra="forbid")
    frame: list[qkbk.Frame]


class _Palettes(BaseModel):
    model_config = ConfigDict(extra="forbid")
    palette: list[qkbk.Palette]


class _Bitmaps(BaseModel):
    model_config = ConfigDict(extra="forbid")
    head: dict[int, tuple[int, int, int]]


def palettes_toml(palettes: Iterable[qkbk.Palette]) -> str:
    lines = ["# The movie's palettes: cast items like the bitmaps. Colours are #rrggbb."]
    for p in palettes:
        lines += ["", "[[palette]]", f"id = {p.id}", f"head = {_value(p.head)}"]
        lines += _rows("colours", [c.lower() for c in p.colours], _PER_LINE)
    return "\n".join(lines) + "\n"


def bitmaps_toml(bitmaps: Iterable[qkbk.Bitmap]) -> str:
    lines = [
        "# The header words of each bitmap (by cast ID) that nobody understands yet;",
        "# its image is casts/<id>.png.",
        "",
        "[head]",
    ]
    lines += [f"{b.id} = {_value(b.head)}" for b in bitmaps]
    return "\n".join(lines) + "\n"


# Images and sound


def _flat_palette(palette: qkbk.Palette) -> list[int]:
    return [channel for colour in palette.colours for channel in bytes.fromhex(colour[1:])]


def write_bitmap(bitmap: qkbk.Bitmap, palette: qkbk.Palette | None, path: Path) -> None:
    image = Image.frombytes("P", (bitmap.width, bitmap.height), bitmap.pixels)
    if palette is not None:
        image.putpalette(_flat_palette(palette))
    image.save(path, transparency=0)


def read_bitmap(path: Path, cast: int, head: tuple[int, int, int]) -> qkbk.Bitmap:
    with Image.open(path) as image:
        if image.mode != "P":
            raise ValueError(f"{path}: must be an 8-bit palette image (index 0 is transparent)")
        return qkbk.Bitmap(
            id=cast,
            head=head,
            width=image.width,
            height=image.height,
            pixels=image.tobytes(),
        )


def sound_wav(samples: bytes, rate: int) -> bytes:
    return sound.to_wav(
        sound.Sound(rate=rate, channels=1, bits=8, samples=samples.translate(_SIGNED_TO_UNSIGNED))
    )


def read_sound(path: Path) -> bytes:
    wav = sound.parse_wav(path.read_bytes(), str(path))
    if wav.channels != 1:
        raise ValueError(f"{path}: must be mono")
    return wav.samples.translate(_SIGNED_TO_UNSIGNED)  # (swapping the sign bit undoes itself)


# Whole movies


def extract_movie(disc_file: Path, disc_dir: Path, out_dir: Path) -> str:
    """Writes the movie in `disc_file` into `out_dir`; a line about it."""
    data = disc_file.read_bytes()
    info, samples, samples_sound = mov.parse(data)
    scene = qkbk.parse_movie(samples)
    manifest = Manifest(
        path=disc_file.relative_to(disc_dir).as_posix(),
        scene=Scene(slots=scene.slots, f23=scene.f23),
        container=info,
    )
    shutil.rmtree(out_dir, ignore_errors=True)
    (out_dir / CASTS).mkdir(parents=True)
    (out_dir / MANIFEST).write_text(manifest_toml(manifest))
    (out_dir / FRAMES).write_text(frames_toml(scene.frames))
    palettes = [c for c in scene.casts.values() if isinstance(c, qkbk.Palette)]
    bitmaps = [c for c in scene.casts.values() if isinstance(c, qkbk.Bitmap)]
    (out_dir / PALETTES).write_text(palettes_toml(palettes))
    (out_dir / BITMAPS).write_text(bitmaps_toml(bitmaps))
    display = scene.casts.get(scene.frames[0].palette)
    for bitmap in bitmaps:
        write_bitmap(
            bitmap, display if isinstance(display, qkbk.Palette) else None,
            out_dir / CASTS / f"{bitmap.id}.png",
        )  # fmt: skip
    (out_dir / SOUND).write_bytes(sound_wav(samples_sound, info.rate))
    if pack_movie(out_dir)[1] != data:
        raise Unconvertible(f"{disc_file.name} doesn't round-trip through assets/")
    return (
        f"{manifest.path}: {len(scene.frames)} frames, {len(palettes)} palettes, "
        f"{len(bitmaps)} bitmaps"
    )


def load_movie(directory: Path) -> tuple[Manifest, qkbk.Movie]:
    """A movie's manifest and scene, loaded from its directory in assets/."""
    manifest = Manifest.model_validate(tomllib.loads((directory / MANIFEST).read_text()))
    frames = _Frames.model_validate(tomllib.loads((directory / FRAMES).read_text())).frame
    palettes = _Palettes.model_validate(tomllib.loads((directory / PALETTES).read_text())).palette
    heads = _Bitmaps.model_validate(tomllib.loads((directory / BITMAPS).read_text())).head
    casts: dict[int, qkbk.Cast] = {}
    for palette in palettes:
        casts[palette.id] = palette
    for cast, head in heads.items():
        casts[cast] = read_bitmap(directory / CASTS / f"{cast}.png", cast, head)
    scene = qkbk.Movie(
        slots=manifest.scene.slots, f23=manifest.scene.f23, frames=frames, casts=casts
    )
    return manifest, scene


def pack_movie(directory: Path) -> tuple[Manifest, bytes]:
    """A movie's manifest and file, built from its directory in assets/."""
    manifest, scene = load_movie(directory)
    samples = qkbk.build_movie(scene)
    return manifest, mov.build(manifest.container, samples, read_sound(directory / SOUND))


def port_scene(directory: Path) -> tuple[str, bytes]:
    """A movie as the port plays it (formats/scene.py): its path on the disc
    with the extension .SCN, and the file."""
    manifest, movie = load_movie(directory)
    c = manifest.container
    track = scene.expand_edits(read_sound(directory / SOUND), c.audio_edits, c.timescale, c.rate)
    data = scene.build(
        movie,
        size=(c.width, c.height),
        milliseconds=c.frame_duration * 1000 // c.timescale,
        rate=c.rate,
        sound=track,
    )
    return str(Path(manifest.path).with_suffix(".SCN")), data


def render_frames(directory: Path, numbers: list[int], out_dir: Path) -> list[Path]:
    """Writes the given frames (numbered from 1) of a movie as PNGs in out_dir."""
    manifest, scene = load_movie(directory)
    size = (manifest.container.width, manifest.container.height)
    out_dir.mkdir(parents=True, exist_ok=True)
    written = []
    for number in numbers:
        if not 1 <= number <= len(scene.frames):
            raise ValueError(f"{directory.name} has frames 1 to {len(scene.frames)}, not {number}")
        path = out_dir / f"{directory.name}-{number:04d}.png"
        qkbk.render_frame(scene, number, size).convert("RGB").save(path)
        written.append(path)
    return written


def movie_directories(assets_dir: Path) -> list[Path]:
    return sorted(p.parent for p in (assets_dir / DIRECTORY).glob(f"*/{MANIFEST}"))


def disc_movies(disc_dir: Path) -> list[Path]:
    """The QuickTime movies on the disc."""
    return sorted((disc_dir / "DATA").glob("*.MOV"))


def extract_all(disc_dir: Path, assets_dir: Path) -> list[str]:
    files = disc_movies(disc_dir)
    with ProcessPoolExecutor() as pool:
        return list(
            pool.map(
                partial(_extract, disc_dir=disc_dir, assets_dir=assets_dir),
                files,
            )
        )


def _extract(disc_file: Path, disc_dir: Path, assets_dir: Path) -> str:
    return extract_movie(disc_file, disc_dir, assets_dir / DIRECTORY / disc_file.stem)


def _pack(directory: Path, out_dir: Path) -> str:
    manifest, data = pack_movie(directory)
    out = out_dir / manifest.path
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(data)
    return f"{manifest.path}: {len(data):,} bytes"


def pack_all(assets_dir: Path, out_dir: Path) -> list[str]:
    with ProcessPoolExecutor() as pool:
        return list(pool.map(partial(_pack, out_dir=out_dir), movie_directories(assets_dir)))


def _verify(directory: Path, disc_dir: Path) -> list[str]:
    """What's wrong with a movie's directory: empty if it packs to the disc's file."""
    manifest, data = pack_movie(directory)
    if data == (disc_dir / manifest.path).read_bytes():
        return []
    return [f"{manifest.path}: differs"]


def verify_all(disc_dir: Path, assets_dir: Path) -> list[tuple[str, list[str]]]:
    directories = movie_directories(assets_dir)
    with ProcessPoolExecutor() as pool:
        reports = pool.map(partial(_verify, disc_dir=disc_dir), directories)
        return [(d.name, r) for d, r in zip(directories, reports, strict=True)]
