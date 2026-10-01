r"""The port: the decompiled game built for modern systems (port/).

`setup` installs the pinned Emscripten SDK into build/emsdk/ and the SoundFont
the music plays with (GeneralUser GS) into build/soundfont/. `build` compiles
the game for a target with CMake (port/CMakeLists.txt): `web` (WebAssembly,
the default) into build/port/web/, or `native` into build/port/native/.
`package` builds the web page and packs the game for it into build/port/site/:
only the files a web server needs, each small enough for hosts that cap file
sizes, ready to upload. `serve` serves the site locally; `run` runs the native
(or headless) build on the drives.

The game's drives: C: (the installed game, and what it saves) is laid out in
build/port/data/c/ from build/zoombi32/ (`uv run extract-game`); D:'s
archives come from assets/ (packed by `uv run assets pack`) for the site, and
from the extracted disc (build/disc/) for `run`.
"""

import contextlib
import http.server
import os
import shutil
import subprocess
import sys
from functools import partial
from pathlib import Path
from typing import Annotated, NamedTuple

import typer

from zbtools import assets, download, exe_resources, movies, paths
from zbtools.formats import icon

# The pinned Emscripten SDK: emsdk's release tarball, which installs the SDK
# of the same version (its own downloads are pinned by emsdk's manifest).
EMSDK_VERSION = "6.0.10"
_EMSDK_URL = f"https://github.com/emscripten-core/emsdk/archive/refs/tags/{EMSDK_VERSION}.tar.gz"
_EMSDK_SHA256 = "09cbafdf00e5a7b4275fc27229d69befafc5ca6070543124219977101310dade"

# The General MIDI SoundFont the music plays with: GeneralUser GS v2.0.3 by
# S. Christian Collins, free to use and redistribute in software (its
# licence: https://github.com/mrbumpy409/GeneralUser-GS/blob/main/documentation/LICENSE.txt).
_SOUNDFONT_COMMIT = "684543d5e5efaef08d02be50dcda8d552478fa60"
_SOUNDFONT_URL = (
    f"https://raw.githubusercontent.com/mrbumpy409/GeneralUser-GS/{_SOUNDFONT_COMMIT}"
    "/GeneralUser-GS.sf2"
)
_SOUNDFONT_SHA256 = "9575028c7a1f589f5770fccc8cff2734566af40cd26ed836944e9a5152688cfe"
SOUNDFONT = paths.SOUNDFONT_DIR / "GeneralUser-GS.sf2"
# Where the web build finds it.
_WEB_SOUNDFONT = "/soundfont/GeneralUser-GS.sf2"

TARGETS = ("web", "headless", "native")
PROGRAM = r"C:\ZOOMBI32\ZOOMBI32.EXE"
CD_LABEL = "ZOOMBINIS"
CD_SERIAL = 0x1996_0101
# What the game's installer would have written (see docs/src/formats/install-config.md).
_CFG = "[INSTALL]\r\nINSTALLFROMDIR=D:\\\r\nINSTALLTODIR=C:\\ZOOMBI32\\\r\n"
_FONT = "CORNER.TTF"
WEB_PAGE = "zoombinis.html"
# The site's page (the web build's, renamed so a host serves it at /), and the
# program's icon as its favicon and as a PNG for the page to show.
SITE_PAGE = "index.html"
SITE_FAVICON = "favicon.ico"
SITE_ICON = "icon.png"
WEB_DATA = "zoombinis-data"
WEB_CONFIG = "zoombinis-config.js"


class PortError(RuntimeError):
    pass


def build_dir(target: str) -> Path:
    return {"web": paths.PORT_WEB_DIR, "headless": paths.PORT_HEADLESS_DIR}.get(
        target, paths.PORT_NATIVE_DIR
    )


def _emsdk_ready() -> bool:
    return (paths.EMSDK_DIR / ".emscripten").exists() and _emscripten_dir().exists()


def _emscripten_dir() -> Path:
    return paths.EMSDK_DIR / "upstream" / "emscripten"


def setup_emsdk(force: bool = False) -> None:
    """Downloads emsdk and installs and activates the pinned SDK in it."""
    if _emsdk_ready() and not force:
        return
    shutil.rmtree(paths.EMSDK_DIR, ignore_errors=True)
    archive = paths.BUILD_DIR / "emsdk.tar.gz"
    download.fetch(_EMSDK_URL, _EMSDK_SHA256, archive)
    unpacked = paths.BUILD_DIR / "emsdk-unpack"
    shutil.rmtree(unpacked, ignore_errors=True)
    download.extract_tar(archive, unpacked)
    archive.unlink()
    (unpacked / f"emsdk-{EMSDK_VERSION}").rename(paths.EMSDK_DIR)
    shutil.rmtree(unpacked)
    emsdk = paths.EMSDK_DIR / "emsdk.py"
    for step in ("install", "activate"):
        print(f"emsdk {step} {EMSDK_VERSION}")
        subprocess.run(
            [sys.executable, str(emsdk), step, EMSDK_VERSION], check=True, cwd=paths.EMSDK_DIR
        )


def setup_soundfont(force: bool = False) -> Path:
    """Downloads the SoundFont, unless it's there; its path."""
    if force or not SOUNDFONT.exists():
        download.fetch(_SOUNDFONT_URL, _SOUNDFONT_SHA256, SOUNDFONT)
    return SOUNDFONT


def _emsdk_env() -> dict[str, str]:
    """The environment emsdk_env.sh would set up."""
    env = dict(os.environ)
    node = sorted((paths.EMSDK_DIR / "node").glob("*/bin"))
    extra = [str(_emscripten_dir()), *(str(n) for n in node)]
    env["PATH"] = os.pathsep.join([*extra, env.get("PATH", "")])
    env["EMSDK"] = str(paths.EMSDK_DIR)
    env["EM_CONFIG"] = str(paths.EMSDK_DIR / ".emscripten")
    return env


def _tool(name: str) -> str:
    """cmake or ninja, from the project's environment."""
    found = shutil.which(name, path=str(Path(sys.executable).parent)) or shutil.which(name)
    if not found:
        raise PortError(f"{name} not found (it comes with `uv sync`)")
    return found


def build(target: str, build_type: str = "RelWithDebInfo") -> Path:
    """Configures (the first time) and builds the target; the program."""
    out = build_dir(target)
    env = dict(os.environ)
    configure = [
        _tool("cmake"),
        "-S",
        str(paths.PORT_SOURCE_DIR),
        "-B",
        str(out),
        "-G",
        "Ninja",
        f"-DCMAKE_MAKE_PROGRAM={_tool('ninja')}",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]
    if target in ("web", "headless"):
        setup_emsdk()
        env = _emsdk_env()
        toolchain = _emscripten_dir() / "cmake" / "Modules" / "Platform" / "Emscripten.cmake"
        configure.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
        configure.append(f"-DZB_HEADLESS={'ON' if target == 'headless' else 'OFF'}")
    if not (out / "build.ninja").exists() or _build_type(out) != build_type:
        subprocess.run(configure, check=True, env=env)
    subprocess.run([_tool("cmake"), "--build", str(out)], check=True, env=env)
    return out / {"web": WEB_PAGE, "headless": "zoombinis.js"}.get(target, "zoombinis")


def _build_type(out: Path) -> str | None:
    cache = out / "CMakeCache.txt"
    if not cache.exists():
        return None
    for line in cache.read_text().splitlines():
        if line.startswith("CMAKE_BUILD_TYPE:"):
            return line.split("=", 1)[1]
    return None


def lay_out_drives(
    installed: Path = paths.INSTALLED_ASSETS, packed: Path = paths.PACKED_ASSETS_DIR
) -> Path:
    r"""C: in build/port/data/c/: the game installed in C:\ZOOMBI32 (what it
    reads of it, and the configuration its installer would have written) and
    its font in C:\WINDOWS\FONTS. The installed files are assets/'s
    (`assets.INSTALLED_FILES`, and MIDIMAP.DAT, which `assets pack` builds),
    so nothing here needs the game's disc. What the game has saved there is
    kept."""
    midimap = packed / "MIDIMAP.DAT"
    if not (installed / _FONT).is_file():
        raise PortError(f"{installed} not found: run `uv run assets extract`")
    if not midimap.is_file():
        raise PortError(f"{midimap} not found: run `uv run assets pack`")
    files = assets.load_installed(installed)
    c = paths.PORT_DATA_DIR / "c"
    target_dir = c / "ZOOMBI32"
    target_dir.mkdir(parents=True, exist_ok=True)
    fonts = c / "WINDOWS" / "FONTS"
    fonts.mkdir(parents=True, exist_ok=True)
    (fonts / _FONT).write_bytes(files.pop(_FONT))
    files["MIDIMAP.DAT"] = midimap.read_bytes()
    for name, data in files.items():
        target = target_dir / name
        if not target.exists():  # the roster, once the game has saved it, is the player's
            target.write_bytes(data)
    (target_dir / "ZOOMBI32.CFG").write_bytes(_CFG.encode("ascii"))
    (c / "WINDOWS" / "TEMP").mkdir(exist_ok=True)
    return c


# The movie the game plays (Data\\Logo025.MOV): the port plays it from its scene
# file, Data\\Logo025.SCN.
MOVIES = ("LOGO025",)


def pack_scenes(assets_dir: Path, packed: Path) -> list[str]:
    """Writes the port's movies (`MOVIES`) as scene files into the packed
    drive, beside where the .MOV would be; a line about each."""
    lines = []
    for name in MOVIES:
        target, data = movies.port_scene(assets_dir / movies.DIRECTORY / name)
        (packed / target).parent.mkdir(parents=True, exist_ok=True)
        (packed / target).write_bytes(data)
        lines.append(f"{target}: {len(data):,} bytes")
    return lines


def game_arguments(c: str, d: str, soundfont: str) -> list[str]:
    """The program's arguments: its drives, and the SoundFont."""
    return [
        "--drive",
        f"C={c}",
        "--cdrom",
        f"D={d},{CD_LABEL},{CD_SERIAL}",
        "--program",
        PROGRAM,
        "--soundfont",
        soundfont,
    ]


# Hosts cap the size of a file (Cloudflare Pages at 25 MiB): the site's files
# stay under this.
SITE_FILE_LIMIT = 24 * 1024 * 1024
# The page's files the web build makes, which the site serves under these names.
_WEB_PROGRAM = {
    WEB_PAGE: SITE_PAGE,
    "zoombinis.js": "zoombinis.js",
    "zoombinis.wasm": "zoombinis.wasm",
}


class SitePiece(NamedTuple):
    """Part of a file for the page's file system: `size` bytes of `source`
    from `offset`, at `target` (with `.part<n>` added if the file is split;
    pre.js joins the parts)."""

    source: Path
    target: str
    offset: int
    size: int


def site_pieces(files: list[tuple[Path, str, int]], limit: int) -> list[SitePiece]:
    """Files (source, target, size) as pieces of at most `limit` bytes."""
    pieces = []
    for source, target, size in files:
        if size <= limit:
            pieces.append(SitePiece(source, target, 0, size))
            continue
        for n, offset in enumerate(range(0, size, limit)):
            pieces.append(SitePiece(source, f"{target}.part{n}", offset, min(limit, size - offset)))
    return pieces


def site_packages(pieces: list[SitePiece], limit: int) -> list[list[SitePiece]]:
    """Pieces grouped into packages of at most `limit` bytes (first fit,
    largest first), each package's pieces in the order given."""
    order = {piece: i for i, piece in enumerate(pieces)}
    packages: list[list[SitePiece]] = []
    sizes: list[int] = []
    for piece in sorted(pieces, key=lambda p: -p.size):
        for i, used in enumerate(sizes):
            if used + piece.size <= limit:
                packages[i].append(piece)
                sizes[i] += piece.size
                break
        else:
            packages.append([piece])
            sizes.append(piece.size)
    return [sorted(package, key=order.__getitem__) for package in packages]


def _page_files(c: Path, data: Path, soundfont: Path) -> list[tuple[Path, str, int]]:
    """What the page's file system holds: C:'s first contents as /c-default
    (pre.js copies them into IndexedDB), D:'s DATA directory as /d/DATA, and
    the SoundFont."""
    found = [(f, f"/c-default/{f.relative_to(c).as_posix()}") for f in sorted(c.rglob("*"))]
    # (`uv run assets pack` also packs the .MOV files, which the port can't play: it has scenes)
    found += [
        (f, f"/d/DATA/{f.name}") for f in sorted(data.iterdir()) if f.suffix.lower() != ".mov"
    ]
    found.append((soundfont, _WEB_SOUNDFONT))
    return [(f, target, f.stat().st_size) for f, target in found if f.is_file()]


def package_web() -> Path:
    """Builds the web page and packs the game for it into build/port/site/,
    holding only what a web server needs, each file under SITE_FILE_LIMIT:
    the page (index.html, zoombinis.js, .wasm), its icon (favicon.ico and
    icon.png, from assets/zoombi32/), zoombinis-config.js (the game's
    drives and the SoundFont), and the page's file system in
    zoombinis-data-<n>.data packages, loaded by zoombinis-data.js. D:'s
    archives are packed from assets/, and the intro movie as a scene file
    (formats/scene.py; the .MOV files are left out: the port can't play QkBk)."""
    web = build_dir("web")
    build("web", _build_type(web) or "RelWithDebInfo")
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    c = lay_out_drives()
    soundfont = setup_soundfont()
    pieces = site_pieces(
        _page_files(c, paths.PACKED_ASSETS_DIR / "DATA", soundfont), SITE_FILE_LIMIT
    )
    site, staging = paths.PORT_SITE_DIR, paths.PORT_SITE_STAGING
    for directory in (site, staging):
        shutil.rmtree(directory, ignore_errors=True)
        directory.mkdir(parents=True)
    for name, published in _WEB_PROGRAM.items():
        shutil.copyfile(web / name, site / published)
    images = exe_resources.app_icon()
    (site / SITE_FAVICON).write_bytes(icon.ico_file(images))
    icon.save_png(images[0], site / SITE_ICON)
    setup_emsdk()
    packager = _emscripten_dir() / "tools" / "file_packager.py"
    loaders = []
    for n, package in enumerate(site_packages(pieces, SITE_FILE_LIMIT)):
        preload = []
        for piece in package:
            source = piece.source
            if piece.size != source.stat().st_size:  # part of a split file
                source = staging / f"{n}-{Path(piece.target).name}"
                with piece.source.open("rb") as f:
                    f.seek(piece.offset)
                    source.write_bytes(f.read(piece.size))
            preload.append(f"{source.resolve()}@{piece.target}")
        loader = staging / f"{WEB_DATA}-{n}.js"
        subprocess.run(
            [
                sys.executable,
                str(packager),
                f"{WEB_DATA}-{n}.data",
                "--preload",
                *preload,
                f"--js-output={loader}",
                "--use-preload-cache",
                "--no-node",
            ],
            check=True,
            cwd=site,
            env=_emsdk_env(),
        )
        loaders.append(loader.read_text())
    # Each loader is a script of its own, so one file can hold them all.
    (site / f"{WEB_DATA}.js").write_text("\n".join(loaders))
    arguments = ", ".join(f'"{a}"' for a in game_arguments("/c", "/d", _WEB_SOUNDFONT))
    arguments = arguments.replace("\\", "\\\\")
    (site / WEB_CONFIG).write_text(f"Module.zbArguments = [{arguments}];\n")
    shutil.rmtree(staging)
    too_big = [f.name for f in site.iterdir() if f.stat().st_size > SITE_FILE_LIMIT]
    if too_big:
        raise PortError(f"over {SITE_FILE_LIMIT} bytes: {', '.join(too_big)}")
    return site


class _Handler(http.server.SimpleHTTPRequestHandler):
    """Serves the build uncached, WebAssembly with its type."""

    def guess_type(self, path: str | os.PathLike[str]) -> str:
        if str(path).endswith(".wasm"):
            return "application/wasm"
        return super().guess_type(path)

    def end_headers(self) -> None:
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


app = typer.Typer(add_completion=False, help=__doc__)


def _fail(error: Exception) -> None:
    print(f"error: {error}")
    raise typer.Exit(1)


@app.command()
def setup(force: Annotated[bool, typer.Option(help="Reinstall")] = False) -> None:
    """Install the pinned Emscripten SDK into build/emsdk/, and the SoundFont
    into build/soundfont/."""
    setup_emsdk(force)
    print(f"Emscripten {EMSDK_VERSION} is in {paths.EMSDK_DIR.relative_to(paths.REPO_ROOT)}")
    soundfont = setup_soundfont(force)
    print(f"The SoundFont is {soundfont.relative_to(paths.REPO_ROOT)}")


@app.command(name="build")
def build_command(
    target: Annotated[str, typer.Argument(help="web or native")] = "web",
    debug: Annotated[bool, typer.Option(help="Build without optimisation")] = False,
) -> None:
    """Build the port for a target."""
    if target not in TARGETS:
        raise typer.BadParameter(f"the targets are {', '.join(TARGETS)}")
    try:
        program = build(target, "Debug" if debug else "RelWithDebInfo")
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"Built {program.relative_to(paths.REPO_ROOT)}")


@app.command()
def package() -> None:
    """Build the web page and pack the game for it into build/port/site/:
    everything a web server needs, and nothing else."""
    try:
        out = package_web()
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"The site is in {out.relative_to(paths.REPO_ROOT)}")


@app.command()
def serve(
    port: Annotated[int, typer.Option(help="The port to serve on")] = 8000,
) -> None:
    """Serve the site on localhost (after `package`)."""
    out = paths.PORT_SITE_DIR
    if not (out / SITE_PAGE).exists() or not (out / f"{WEB_DATA}.js").exists():
        _fail(PortError("package it first: uv run port package"))
    handler = partial(_Handler, directory=str(out))
    server = http.server.ThreadingHTTPServer(("127.0.0.1", port), handler)
    print(f"Serving http://127.0.0.1:{port}/ (Ctrl-C to stop)")
    with contextlib.suppress(KeyboardInterrupt):
        server.serve_forever()


@app.command()
def run(
    headless: Annotated[
        bool, typer.Option(help="Run the headless build under Node (for testing)")
    ] = False,
    screenshot: Annotated[
        Path | None, typer.Option(help="Write the screen to this BMP about once a second")
    ] = None,
    seconds: Annotated[float | None, typer.Option(help="Quit after this many seconds")] = None,
    click: Annotated[
        list[str] | None,
        typer.Option(
            help="Click at a point of the screen: MS:X,Y (ms after starting); "
            "MS:X,Y:press, :move and :release make a drag"
        ),
    ] = None,
    record: Annotated[
        Path | None, typer.Option(help="Write what's played to this WAV file")
    ] = None,
) -> None:
    """Run the native build (or the headless one) on the game's drives (packed from
    assets/, as `package` does)."""
    target = "headless" if headless else "native"
    program = build(target) if headless else build_dir("native") / "zoombinis"
    if not program.exists():
        _fail(PortError("build it first: uv run port build native"))
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    try:
        c = lay_out_drives()
    except PortError as e:
        _fail(e)
    soundfont = setup_soundfont()
    arguments = game_arguments(
        str(c.resolve()), str(paths.PACKED_ASSETS_DIR.resolve()), str(soundfont.resolve())
    )
    if record:
        arguments += ["--record", str(record.resolve())]
    if screenshot:
        arguments += ["--screenshot", str(screenshot.resolve())]
    if seconds:
        arguments += ["--run-for", str(int(seconds * 1000))]
    for spec in click or []:
        arguments += ["--click", spec]
    if headless:
        node = sorted((paths.EMSDK_DIR / "node").glob("*/bin/node"))
        command = [str(node[-1]), str(program), *arguments]
    else:
        command = [str(program), *arguments]
    raise typer.Exit(subprocess.run(command, check=False).returncode)
