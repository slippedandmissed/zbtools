r"""The port: the decompiled game built for modern systems (port/).

A *target* is a platform and architecture to build for: `macos_universal`,
`windows_x86`, `windows_x64`, `linux_x64`, `linux_arm64`, `browser_wasm` (the
game in a web page) and `headless_wasm` (the same under Node, with no screen or
sound, for testing). Every command takes one, and defaults to the machine's own
(`macos_universal` on a Mac, `linux_x64` or `linux_arm64` on Linux), so any
target builds from any host: Windows with llvm-mingw, Linux in a container
(Docker), the WebAssembly ones with the Emscripten SDK, macOS with Xcode.

`setup` installs what a target needs (and the SoundFont the music plays with).
`build` compiles it with CMake (port/CMakeLists.txt) into build/port/<target>/,
with the debug tools. `package` builds it without them and packs it with the
game's data into build/port/dist/: for players (`zoombinis-<version>-<target>`:
a directory and its .zip, .tar.gz or .dmg), or for `browser_wasm` the site, only
the files a web server needs, each small enough for hosts that cap file sizes.
`run` runs a build on this machine (the machine's own, or `headless_wasm`) on
the game's drives; `serve` serves the site locally.

The game's drives: C: (the installed game, and what it saves) is laid out in
build/port/data/c/ from assets/zoombi32/installed/; D:'s archives come from
assets/ (packed by `uv run assets pack`).
"""

import contextlib
import http.server
import importlib.metadata
import os
import platform
import shutil
import subprocess
import sys
from dataclasses import dataclass
from functools import partial
from pathlib import Path
from typing import Annotated, Literal, NamedTuple

import typer

from zbtools import assets, bundle, debug_globals, download, exe_resources, host, movies, paths
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


@dataclass(frozen=True)
class Target:
    """A platform and architecture to build for."""

    name: str
    # How it's built: Xcode on a Mac, llvm-mingw, a Docker container, or Emscripten.
    toolchain: Literal["xcode", "mingw", "container", "emscripten"]
    # llvm-mingw's architecture, or Docker's platform; empty for the others.
    architecture: str = ""

    @property
    def is_site(self) -> bool:
        return self.name == "browser_wasm"

    @property
    def is_windows(self) -> bool:
        return self.toolchain == "mingw"

    @property
    def program(self) -> str:
        """The file the build makes."""
        return {
            "browser_wasm": WEB_PAGE,
            "headless_wasm": "zoombinis.js",
            "windows_x86": "zoombinis.exe",
            "windows_x64": "zoombinis.exe",
        }.get(self.name, "zoombinis")


TARGETS: dict[str, Target] = {
    t.name: t
    for t in (
        Target("macos_universal", "xcode"),
        Target("windows_x86", "mingw", "i686"),
        Target("windows_x64", "mingw", "x86_64"),
        Target("linux_x64", "container", "linux/amd64"),
        Target("linux_arm64", "container", "linux/arm64"),
        Target("browser_wasm", "emscripten"),
        Target("headless_wasm", "emscripten"),
    )
}
_LINUX_DOCKERFILE = paths.PORT_SOURCE_DIR / "docker" / "linux.Dockerfile"


def host_target() -> Target:
    """The target for this machine: a Mac's universal build, or Linux's own architecture."""
    machine = platform.machine().lower()
    if host.SYSTEM == "Darwin":
        return TARGETS["macos_universal"]
    if host.SYSTEM == "Linux":
        return TARGETS["linux_arm64" if machine in ("arm64", "aarch64") else "linux_x64"]
    raise PortError(f"no target for {host.SYSTEM}: name one")


def target_named(name: str | None) -> Target:
    """The target called `name`, or this machine's."""
    if name is None:
        return host_target()
    if name not in TARGETS:
        raise PortError(f"no target {name!r}: the targets are {', '.join(TARGETS)}")
    return TARGETS[name]


# llvm-mingw (clang and mingw-w64: https://github.com/mstorsjo/llvm-mingw), which
# builds for Windows from macOS, Linux or Windows. Its releases by host: the
# platform in the file name, and the SHA-256 to pin (empty until pinned: run
# `uv run port setup windows_x64`, which says what the download hashes to, and check
# that against the release page). ZB_MINGW_DIR names an existing installation instead.
_MINGW_VERSION = "20250114"
_MINGW_HOSTS: dict[tuple[str, str], tuple[str, str]] = {
    ("Darwin", "arm64"): (
        "macos-universal",
        "80b2e7ade71ba2dfe9e8d27fe47ae5738b1fb8d34e057faa2beef3070392f2d6",
    ),
    ("Darwin", "x86_64"): (
        "macos-universal",
        "80b2e7ade71ba2dfe9e8d27fe47ae5738b1fb8d34e057faa2beef3070392f2d6",
    ),
    ("Linux", "x86_64"): (
        "ubuntu-20.04-x86_64",
        "a16f52dee819797248e6c7d63b8b1e50a92119f45767ecd8e9633d1733b896e2",
    ),
    ("Linux", "aarch64"): (
        "ubuntu-20.04-aarch64",
        "3b7b675a17189621700b5796d745db0aea6e29756870352390112101ad38afff",
    ),
}
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


def build_dir(target: Target) -> Path:
    return paths.PORT_DIR / target.name


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


def mingw_dir() -> Path:
    """Where llvm-mingw is: ZB_MINGW_DIR, or the pinned release in build/llvm-mingw/."""
    given = os.environ.get("ZB_MINGW_DIR")
    return Path(given) if given else paths.MINGW_DIR


def setup_mingw(force: bool = False) -> Path:
    """Downloads the pinned llvm-mingw into build/llvm-mingw/, unless
    ZB_MINGW_DIR names one or it's there; its directory."""
    directory = mingw_dir()
    if os.environ.get("ZB_MINGW_DIR") or ((directory / "bin").is_dir() and not force):
        return directory
    key = (platform.system(), platform.machine())
    if key not in _MINGW_HOSTS:
        raise PortError(f"no llvm-mingw release is pinned for {key[0]} {key[1]}: set ZB_MINGW_DIR")
    name, sha256 = _MINGW_HOSTS[key]
    stem = f"llvm-mingw-{_MINGW_VERSION}-ucrt-{name}"
    url = f"https://github.com/mstorsjo/llvm-mingw/releases/download/{_MINGW_VERSION}/{stem}.tar.xz"
    archive = paths.BUILD_DIR / f"{stem}.tar.xz"
    download.fetch(url, sha256, archive)
    shutil.rmtree(directory, ignore_errors=True)
    unpacked = paths.BUILD_DIR / "llvm-mingw-unpack"
    shutil.rmtree(unpacked, ignore_errors=True)
    download.extract_tar(archive, unpacked)
    archive.unlink()
    (unpacked / stem).rename(directory)
    shutil.rmtree(unpacked)
    return directory


def _mingw_env() -> dict[str, str]:
    """The environment with llvm-mingw's compilers on the PATH."""
    env = dict(os.environ)
    env["PATH"] = os.pathsep.join([str(setup_mingw() / "bin"), env.get("PATH", "")])
    return env


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


def _build_in_container(target: Target, build_type: str, debug_tools: bool) -> None:
    """Builds a Linux target in a container (port/docker/linux.Dockerfile): the
    repository is mounted in it, and the build directory is in build/port/."""
    docker = host.docker()
    image = f"zbtools-port-{target.name}"
    subprocess.run(
        [
            docker,
            "build",
            "--platform",
            target.architecture,
            "-t",
            image,
            "-f",
            str(_LINUX_DOCKERFILE),
            str(_LINUX_DOCKERFILE.parent),
        ],
        check=True,
    )
    out = build_dir(target)
    out.mkdir(parents=True, exist_ok=True)
    # Inside, the repository is /src. SDL2 is built from source (statically: the
    # program opens the system's X11, Wayland and audio libraries itself) and the
    # C++ runtime is linked in, so the program depends on little but glibc.
    work = f"/src/{out.relative_to(paths.REPO_ROOT).as_posix()}"
    script = (
        f"cmake -S /src/port -B {work} -G Ninja -DCMAKE_BUILD_TYPE={build_type}"
        f" -DZB_DEBUG={'ON' if debug_tools else 'OFF'} -DZB_SYSTEM_SDL=OFF"
        " -DCMAKE_EXE_LINKER_FLAGS='-static-libstdc++ -static-libgcc'"
        f" && cmake --build {work}"
    )
    user = ["--user", f"{os.getuid()}:{os.getgid()}"] if host.SYSTEM == "Linux" else []
    subprocess.run(
        [
            docker,
            "run",
            "--rm",
            "--platform",
            target.architecture,
            *user,
            "-v",
            f"{paths.REPO_ROOT}:/src",
            image,
            "sh",
            "-c",
            script,
        ],
        check=True,
    )


def setup(target: Target, force: bool = False) -> None:
    """Installs what building `target` needs, and the SoundFont."""
    if target.toolchain == "emscripten":
        setup_emsdk(force)
    elif target.toolchain == "mingw":
        setup_mingw(force)
    elif target.toolchain == "container":
        host.docker()  # (the image is built when the target is)
    setup_soundfont(force)


def build(target: Target, build_type: str = "RelWithDebInfo", debug_tools: bool = True) -> Path:
    """Configures (the first time, or when the build type or the debug tools
    change) and builds the target; the program. `debug_tools` builds
    port/debug/ in (CMake's ZB_DEBUG); packages are built without."""
    out = build_dir(target)
    if debug_tools and debug_globals.write():
        # (The table is only found when it exists, so a build made before it did needs the
        # debug tools' source recompiled.)
        (paths.PORT_SOURCE_DIR / "debug" / "zbdebug.cpp").touch()
    if target.toolchain == "container":
        _build_in_container(target, build_type, debug_tools)
        return out / target.program
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
        f"-DZB_DEBUG={'ON' if debug_tools else 'OFF'}",
    ]
    if target.toolchain == "emscripten":
        setup_emsdk()
        env = _emsdk_env()
        toolchain = _emscripten_dir() / "cmake" / "Modules" / "Platform" / "Emscripten.cmake"
        configure.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
        configure.append(f"-DZB_HEADLESS={'ON' if target.name == 'headless_wasm' else 'OFF'}")
    elif target.toolchain == "mingw":
        env = _mingw_env()
        # The program's icon, for its .exe (port/CMakeLists.txt makes the resource).
        ico = paths.PORT_DIR / "zoombinis.ico"
        ico.parent.mkdir(parents=True, exist_ok=True)
        ico.write_bytes(icon.ico_file(exe_resources.app_icon()))
        configure += [
            f"-DCMAKE_TOOLCHAIN_FILE={paths.PORT_SOURCE_DIR / 'toolchains' / 'mingw.cmake'}",
            f"-DZB_MINGW_ARCH={target.architecture}",
            f"-DZB_WINDOWS_ICON={ico}",
        ]
    elif target.toolchain == "xcode":
        if host.SYSTEM != "Darwin":
            raise PortError("macos_universal builds on a Mac (it needs Xcode)")
        # One program for Apple silicon and Intel Macs.
        configure += [
            "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64",
            f"-DCMAKE_OSX_DEPLOYMENT_TARGET={bundle.MACOS_MINIMUM}",
        ]
    if (
        not (out / "build.ninja").exists()
        or _build_type(out) != build_type
        or _cache_value(out, "ZB_DEBUG") != ("ON" if debug_tools else "OFF")
    ):
        subprocess.run(configure, check=True, env=env)
    subprocess.run([_tool("cmake"), "--build", str(out)], check=True, env=env)
    return out / target.program


def package_player(target: Target, debug_tools: bool = False) -> Path:
    """Builds the program for `target` and packs it with the game for players
    into build/port/dist/ (see bundle.py): `zoombinis-<version>-<target>/` and its
    archive, which is the path: a .dmg holding Zoombinis.app (signed ad hoc) for
    macOS, a .zip for Windows, a .tar.gz for Linux."""
    program = build(target, "RelWithDebInfo", debug_tools)
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    soundfont = setup_soundfont()
    game = paths.PORT_DIST_STAGING / "game"
    shutil.rmtree(paths.PORT_DIST_STAGING, ignore_errors=True)
    lay_out_drives(c=game / "C")
    # (the .MOV files aren't played: the port plays the movie from its scene file)
    shutil.copytree(
        paths.PACKED_ASSETS_DIR / "DATA",
        game / "D" / "DATA",
        ignore=shutil.ignore_patterns("*.MOV", "*.mov"),
    )
    shutil.copyfile(soundfont, game / soundfont.name)
    version = importlib.metadata.version("zbtools")
    name = f"{bundle.APP_NAME.lower()}-{version}-{target.name}"
    dist = paths.PORT_DIST_DIR
    dist.mkdir(parents=True, exist_ok=True)
    if target.toolchain == "xcode":
        directory = dist / name
        shutil.rmtree(directory, ignore_errors=True)
        directory.mkdir()
        app = directory / f"{bundle.APP_NAME}.app"
        bundle.assemble_app(app, program, game, exe_resources.app_icon()[0], version)
        bundle.sign_ad_hoc(app)
        return bundle.make_dmg(app, dist / f"{name}.dmg")
    directory = bundle.assemble_directory(dist / name, program, game)
    return bundle.make_archive(
        directory, dist / f"{name}{'.zip' if target.is_windows else '.tar.gz'}"
    )


def package(target: Target, debug_tools: bool = False) -> Path:
    """Packs `target` for distribution: the site for `browser_wasm`, else a
    package for players."""
    if target.is_site:
        return package_web(debug_tools)
    if target.name == "headless_wasm":
        raise PortError("headless_wasm is for testing: there is nothing to package")
    return package_player(target, debug_tools)


def _cache_value(out: Path, name: str) -> str | None:
    """A variable's value in the build directory's CMake cache."""
    cache = out / "CMakeCache.txt"
    if not cache.exists():
        return None
    for line in cache.read_text().splitlines():
        if line.startswith(f"{name}:"):
            return line.split("=", 1)[1]
    return None


def _build_type(out: Path) -> str | None:
    return _cache_value(out, "CMAKE_BUILD_TYPE")


def lay_out_drives(
    installed: Path = paths.INSTALLED_ASSETS,
    packed: Path = paths.PACKED_ASSETS_DIR,
    c: Path | None = None,
) -> Path:
    r"""C: in build/port/data/c/: the game installed in C:\ZOOMBI32 (what it
    reads of it, and the configuration its installer would have written) and
    its font in C:\WINDOWS\FONTS. The installed files are assets/'s
    (`assets.INSTALLED_FILES`, and MIDIMAP.DAT, which `assets pack` builds),
    so nothing here needs the game's disc. What the game has saved there is
    kept. `c` is where to lay it out (build/port/data/c/ by default)."""
    midimap = packed / "MIDIMAP.DAT"
    if not (installed / _FONT).is_file():
        raise PortError(f"{installed} not found: run `uv run assets extract`")
    if not midimap.is_file():
        raise PortError(f"{midimap} not found: run `uv run assets pack`")
    files = assets.load_installed(installed)
    c = c or paths.PORT_DATA_DIR / "c"
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


def package_web(debug_tools: bool = False) -> Path:
    """Builds the web page and packs the game for it into build/port/site/,
    holding only what a web server needs, each file under SITE_FILE_LIMIT:
    the page (index.html, zoombinis.js, .wasm), its icon (favicon.ico and
    icon.png, from assets/zoombi32/), zoombinis-config.js (the game's
    drives and the SoundFont), and the page's file system in
    zoombinis-data-<n>.data packages, loaded by zoombinis-data.js. D:'s
    archives are packed from assets/, and the intro movie as a scene file
    (formats/scene.py; the .MOV files are left out: the port can't play QkBk).
    The debug tools are left out unless `debug_tools` (a development site)."""
    browser = TARGETS["browser_wasm"]
    web = build_dir(browser)
    build(browser, _build_type(web) or "RelWithDebInfo", debug_tools)
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    c = lay_out_drives()
    soundfont = setup_soundfont()
    pieces = site_pieces(
        _page_files(c, paths.PACKED_ASSETS_DIR / "DATA", soundfont), SITE_FILE_LIMIT
    )
    site, staging = paths.PORT_SITE_DIR, paths.PORT_DIST_STAGING
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

TargetArgument = Annotated[
    str | None,
    typer.Argument(help=f"One of {', '.join(TARGETS)}; the default is this machine's"),
]
DebugToolsOption = Annotated[
    bool, typer.Option(help="Build the debug tools in (port/debug/)", show_default=False)
]


def _fail(error: Exception) -> None:
    print(f"error: {error}")
    raise typer.Exit(1)


def _target(name: str | None) -> Target:
    try:
        return target_named(name)
    except PortError as e:
        _fail(e)
        raise  # (unreachable: _fail exits)


@app.command(name="setup")
def setup_command(
    target: TargetArgument = None,
    force: Annotated[bool, typer.Option(help="Reinstall")] = False,
) -> None:
    """Install what a target needs (the Emscripten SDK for the WebAssembly ones,
    llvm-mingw for Windows, Docker is checked for Linux) and the SoundFont."""
    chosen = _target(target)
    try:
        setup(chosen, force)
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"Ready to build {chosen.name}")


@app.command(name="build")
def build_command(
    target: TargetArgument = None,
    debug: Annotated[bool, typer.Option(help="Build without optimisation")] = False,
    debug_tools: DebugToolsOption = True,
) -> None:
    """Build the port for a target, into build/port/<target>/ (with the debug
    tools; `package` builds without)."""
    chosen = _target(target)
    try:
        program = build(chosen, "Debug" if debug else "RelWithDebInfo", debug_tools)
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"Built {program.relative_to(paths.REPO_ROOT)}")


@app.command(name="package")
def package_command(
    target: TargetArgument = None,
    debug_tools: DebugToolsOption = False,
) -> None:
    """Build a target and pack it with the game, into build/port/dist/: a
    package for players (a .dmg for macOS, a .zip for Windows, a .tar.gz for
    Linux), or for browser_wasm the site, everything a web server needs and
    nothing else."""
    chosen = _target(target)
    try:
        out = package(chosen, debug_tools)
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"Packaged {out.relative_to(paths.REPO_ROOT)}")


@app.command()
def serve(
    port: Annotated[int, typer.Option(help="The port to serve on")] = 8000,
) -> None:
    """Serve the browser_wasm site on localhost (after `package browser_wasm`)."""
    out = paths.PORT_SITE_DIR
    if not (out / SITE_PAGE).exists() or not (out / f"{WEB_DATA}.js").exists():
        _fail(PortError("package it first: uv run port package browser_wasm"))
    handler = partial(_Handler, directory=str(out))
    server = http.server.ThreadingHTTPServer(("127.0.0.1", port), handler)
    print(f"Serving http://127.0.0.1:{port}/ (Ctrl-C to stop)")
    with contextlib.suppress(KeyboardInterrupt):
        server.serve_forever()


def _test_arguments(
    *,
    record: Path | None,
    screenshot: Path | None,
    seconds: float | None,
    click: list[str] | None,
    cmd: list[str] | None,
    script: Path | None,
) -> list[str]:
    """The program's options for scripted runs (see main.cpp)."""
    arguments: list[str] = []
    if record:
        arguments += ["--record", str(record.resolve())]
    if screenshot:
        arguments += ["--screenshot", str(screenshot.resolve())]
    if seconds:
        arguments += ["--run-for", str(int(seconds * 1000))]
    for spec in click or []:
        arguments += ["--click", spec]
    for commands in cmd or []:
        arguments += ["--cmd", commands]
    if script:
        arguments += ["--script", str(script.resolve())]
    return arguments


@app.command()
def run(  # noqa: PLR0917 (a CLI's options)
    target: TargetArgument = None,
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
    cmd: Annotated[
        list[str] | None,
        typer.Option(help="Debug commands to run (repeatable; ';' separates; see `help`)"),
    ] = None,
    script: Annotated[
        Path | None, typer.Option(help="A file of debug commands, one per line")
    ] = None,
) -> None:
    """Build a target (with the debug tools) and run it on the game's drives
    (packed from assets/). The target is this machine's, or headless_wasm (under
    Node: no window, for testing)."""
    chosen = _target(target)
    if chosen != host_target() and chosen.name != "headless_wasm":
        _fail(PortError(f"{chosen.name} doesn't run on this machine: see `package`"))
    try:
        program = build(chosen)
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
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
    arguments += _test_arguments(
        record=record, screenshot=screenshot, seconds=seconds, click=click, cmd=cmd, script=script
    )
    if chosen.name == "headless_wasm":
        node = sorted((paths.EMSDK_DIR / "node").glob("*/bin/node"))
        command = [str(node[-1]), str(program), *arguments]
    else:
        command = [str(program), *arguments]
    raise typer.Exit(subprocess.run(command, check=False).returncode)
