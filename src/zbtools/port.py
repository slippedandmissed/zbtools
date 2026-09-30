r"""The port: the decompiled game built for modern systems (port/).

`setup` installs the pinned Emscripten SDK into build/emsdk/. `build` compiles
the game for a target with CMake (port/CMakeLists.txt): `web` (WebAssembly,
the default) into build/port/web/, or `native` into build/port/native/.
`package` lays out the game's C: drive (the installed game) from your copy of
the game in build/port/data/c/ and, for the web, packs it and the CD (D:,
build/disc/) next to the page; `serve` serves the page locally; `run` runs the native
build on the drives.

The game's files come from `uv run extract-game` (build/disc/ and
build/zoombi32/); they're never committed, and the packed data is only for
your own use.
"""

import contextlib
import http.server
import os
import shutil
import subprocess
import sys
from functools import partial
from pathlib import Path
from typing import Annotated

import typer

from zbtools import download, paths

# The pinned Emscripten SDK: emsdk's release tarball, which installs the SDK
# of the same version (its own downloads are pinned by emsdk's manifest).
EMSDK_VERSION = "6.0.10"
_EMSDK_URL = f"https://github.com/emscripten-core/emsdk/archive/refs/tags/{EMSDK_VERSION}.tar.gz"
_EMSDK_SHA256 = "09cbafdf00e5a7b4275fc27229d69befafc5ca6070543124219977101310dade"

TARGETS = ("web", "headless", "native")
PROGRAM = r"C:\ZOOMBI32\ZOOMBI32.EXE"
CD_LABEL = "ZOOMBINIS"
CD_SERIAL = 0x1996_0101
# What the game's installer would have written (see docs/findings.md).
_CFG = "[INSTALL]\r\nINSTALLFROMDIR=D:\\\r\nINSTALLTODIR=C:\\ZOOMBI32\\\r\n"
# The installed game's files it reads (the rest of build/zoombi32/ is unused).
_INSTALLED_FILES = ("MIDIMAP.DAT", "mohawk.w32", "ReadMe.txt", "Zoombini.who")
_FONT = "CORNER.TTF"
WEB_PAGE = "zoombinis.html"
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


def lay_out_drives(game: Path = paths.GAME32_DIR) -> Path:
    r"""C: in build/port/data/c/: the game installed in C:\ZOOMBI32 (what it
    reads of it, and the configuration its installer would have written) and
    its font in C:\WINDOWS\FONTS. What the game has saved there is kept."""
    if not (paths.DISC_DIR / "DATA").is_dir() or not (game / _FONT).is_file():
        raise PortError("the game isn't extracted: run `uv run extract-game` first")
    c = paths.PORT_DATA_DIR / "c"
    installed = c / "ZOOMBI32"
    installed.mkdir(parents=True, exist_ok=True)
    for name in _INSTALLED_FILES:
        target = installed / name
        if not target.exists():  # the roster, once the game has saved it, is the player's
            shutil.copyfile(game / name, target)
    (installed / "ZOOMBI32.CFG").write_bytes(_CFG.encode("ascii"))
    fonts = c / "WINDOWS" / "FONTS"
    fonts.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(game / _FONT, fonts / _FONT)
    (c / "WINDOWS" / "TEMP").mkdir(exist_ok=True)
    return c


def drive_arguments(c: str, d: str) -> list[str]:
    return ["--drive", f"C={c}", "--cdrom", f"D={d},{CD_LABEL},{CD_SERIAL}", "--program", PROGRAM]


def package_web() -> Path:
    """Packs the drives for the page: zoombinis-data.data (and its loader)
    holds D: as /d and C:'s first contents as /c-default (pre.js copies them
    into IndexedDB); zoombinis-config.js gives the game its drives."""
    c = lay_out_drives()
    out = build_dir("web")
    out.mkdir(parents=True, exist_ok=True)
    setup_emsdk()
    packager = _emscripten_dir() / "tools" / "file_packager.py"
    subprocess.run(
        [
            sys.executable,
            str(packager),
            f"{WEB_DATA}.data",
            "--preload",
            f"{(paths.DISC_DIR / 'DATA').resolve()}@/d/DATA",
            f"{c.resolve()}@/c-default",
            f"--js-output={WEB_DATA}.js",
            "--use-preload-cache",
            "--no-node",
        ],
        check=True,
        cwd=out,
        env=_emsdk_env(),
    )
    arguments = ", ".join(f'"{a}"' for a in drive_arguments("/c", "/d")).replace("\\", "\\\\")
    (out / WEB_CONFIG).write_text(f"Module.zbArguments = [{arguments}];\n")
    return out


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
    """Install the pinned Emscripten SDK into build/emsdk/."""
    setup_emsdk(force)
    print(f"Emscripten {EMSDK_VERSION} is in {paths.EMSDK_DIR.relative_to(paths.REPO_ROOT)}")


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
    """Lay out the game's drives from your copy of the game, and pack them
    for the web build."""
    try:
        out = package_web()
    except (PortError, subprocess.CalledProcessError) as e:
        _fail(e)
    print(f"Packed the game's data into {out.relative_to(paths.REPO_ROOT)}")


@app.command()
def serve(
    port: Annotated[int, typer.Option(help="The port to serve on")] = 8000,
) -> None:
    """Serve the web build on localhost (after `build` and `package`)."""
    out = build_dir("web")
    if not (out / WEB_PAGE).exists() or not (out / f"{WEB_DATA}.data").exists():
        _fail(PortError("build and package first: uv run port build && uv run port package"))
    handler = partial(_Handler, directory=str(out))
    server = http.server.ThreadingHTTPServer(("127.0.0.1", port), handler)
    print(f"Serving http://127.0.0.1:{port}/{WEB_PAGE} (Ctrl-C to stop)")
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
        typer.Option(help="Click at a point of the screen: MS:X,Y (ms after starting)"),
    ] = None,
) -> None:
    """Run the native build (or the headless one) on the game's drives."""
    target = "headless" if headless else "native"
    program = build(target) if headless else build_dir("native") / "zoombinis"
    if not program.exists():
        _fail(PortError("build it first: uv run port build native"))
    try:
        c = lay_out_drives()
    except PortError as e:
        _fail(e)
    arguments = drive_arguments(str(c.resolve()), str(paths.DISC_DIR.resolve()))
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
