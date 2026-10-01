r"""Borland C++ toolchains (compiler, linker, libraries), run under Wine.

`setup` copies BIN, LIB and INCLUDE from each Borland C++ CD in data/ into
build/toolchain/<release>/ (from the CD's run-from-CD tree: BC45 for 4.5x,
BC5 for 5.02). Each release gets its own Wine drive (4.5 is T:, 4.52 is U:,
5.02 is V:; the repository is R:, keeping paths short), and its default
BCC32.CFG and TLINK32.CFG, which point at the CD drive, are rewritten to
point there instead. `run` runs a tool from a
release; `check` compiles, links and runs a small program with each release.
"""

import os
import shutil
import stat
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import typer

from zbtools import host, paths


@dataclass(frozen=True)
class Release:
    drive: str  # the Wine drive it's mapped to
    root: str  # its directory on the CD, holding BIN, LIB and INCLUDE


RELEASES = {
    "4.5": Release("T:", "BC45"),
    "4.52": Release("U:", "BC45"),
    "5.02": Release("V:", "BC5"),
}
# Wine drive the repository is mapped to, so paths in it stay short wherever
# it's cloned: the Borland tools truncate paths longer than 80 characters.
REPO_DRIVE = "R:"
_PARTS = ("BIN", "LIB", "INCLUDE")

_CHECK_SOURCE = r"""#include <stdio.h>

int add(int a, int b) { return a + b; }

int main(void)
{
    printf("hello from Borland C++, 2 + 2 = %d\n", add(2, 2));
    return 0;
}
"""


def root_dir(release: str) -> Path:
    """The release's root directory (with BIN, LIB and INCLUDE)."""
    return paths.TOOLCHAIN_DIR / release / RELEASES[release].root


def _windows_root(release: str) -> str:
    """The release's root directory as the Borland tools see it."""
    return f"{RELEASES[release].drive}\\{RELEASES[release].root}"


def library_path(release: str, name: str) -> str:
    """A file in a release's LIB directory (C0W32.OBJ, CW32.LIB), as the
    Borland tools see it."""
    return f"{_windows_root(release)}\\LIB\\{name}"


def windows_path(path: Path) -> str:
    """A host path as Wine sees it: under R: if it's in the repository, else
    under Z:, where Wine maps the host's root directory."""
    resolved = path.resolve()
    if resolved.is_relative_to(paths.REPO_ROOT.resolve()):
        relative = resolved.relative_to(paths.REPO_ROOT.resolve())
        return REPO_DRIVE + "\\" + str(relative).replace("/", "\\")
    return "Z:" + str(resolved).replace("/", "\\")


def installed_releases() -> list[str]:
    return [r for r in paths.BORLAND_ISOS if (root_dir(r) / "BIN" / "BCC32.EXE").exists()]


def _dos_text(text: str) -> bytes:
    return text.replace("\n", "\r\n").encode("ascii")


def _wine_env(path: str = "") -> dict[str, str]:
    """Environment for Wine; path is added to the Windows PATH (WINEPATH)."""
    return {
        **os.environ,
        "WINEPATH": path,
        "WINEPREFIX": str(paths.WINE_PREFIX),
        "WINEDEBUG": "-all",
        "MVK_CONFIG_LOG_LEVEL": "0",
        # Don't offer to install Mono or Gecko: nothing here needs .NET or HTML.
        "WINEDLLOVERRIDES": "mscoree,mshtml=",
    }


def _make_writable(root: Path) -> None:
    """Files copied off a CD are read-only; make them deletable by `clean`."""
    for path in [root, *root.rglob("*")]:
        path.chmod(path.stat().st_mode | stat.S_IWUSR)


def install(release: str, iso: Path) -> None:
    dest = paths.TOOLCHAIN_DIR / release
    if dest.exists():
        _make_writable(dest)
        shutil.rmtree(dest)
    subprocess.run(
        [host.sevenzip(), "x", "-y", "-bso0", "-bsp0", f"-o{dest}", str(iso)]
        + [f"{RELEASES[release].root}/{part}/*" for part in _PARTS],
        check=True,
    )
    _make_writable(dest)
    windows = _windows_root(release)
    bin_dir = root_dir(release) / "BIN"
    (bin_dir / "BCC32.CFG").write_bytes(_dos_text(f"-I{windows}\\INCLUDE\n-L{windows}\\LIB\n"))
    (bin_dir / "TLINK32.CFG").write_bytes(_dos_text(f"-L{windows}\\LIB\n"))


def ensure_prefix() -> None:
    """Create the Wine prefix if needed, and map each installed release's
    drive and the repository's."""
    bin_dir = host.wine_bin_dir()
    if not (paths.WINE_PREFIX / "system.reg").exists():
        print("Creating the Wine prefix (first run only)")
        paths.WINE_PREFIX.mkdir(parents=True, exist_ok=True)
        subprocess.run(
            [str(bin_dir / "wineboot"), "-i"], env=_wine_env(), check=True, capture_output=True
        )
        subprocess.run([str(bin_dir / "wineserver"), "-w"], env=_wine_env(), check=True)
    dosdevices = paths.WINE_PREFIX / "dosdevices"
    repo = dosdevices / REPO_DRIVE.lower()
    if not repo.is_symlink() or repo.resolve() != paths.REPO_ROOT.resolve():
        repo.unlink(missing_ok=True)
        repo.symlink_to(paths.REPO_ROOT.resolve(), target_is_directory=True)
    # Only relink what's wrong: parallel compiles all call this.
    for release in installed_releases():
        link = dosdevices / RELEASES[release].drive.lower()
        target = paths.TOOLCHAIN_DIR / release
        if not link.is_symlink() or link.resolve() != target.resolve():
            link.unlink(missing_ok=True)
            link.symlink_to(target, target_is_directory=True)


def wine(
    program: Path, args: list[str], cwd: Path, capture: bool = True, path: str = ""
) -> "subprocess.CompletedProcess[str]":
    """Run a Windows program under Wine in the project's prefix, with path
    added to its Windows PATH.

    Output is captured through temporary files, not pipes: Wine's background
    processes (wineserver and the services it starts) inherit the program's
    output handles and keep a pipe open until they exit, seconds later, so
    waiting for the pipe to close made every run take about five seconds."""
    ensure_prefix()
    command = [str(host.wine_bin_dir() / "wine"), str(program), *args]
    if not capture:
        return subprocess.run(command, cwd=cwd, env=_wine_env(path), text=True, check=False)
    with tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
        done = subprocess.run(
            command, cwd=cwd, env=_wine_env(path), stdout=out, stderr=err, check=False
        )
        out.seek(0)
        err.seek(0)
        return subprocess.CompletedProcess(
            command,
            done.returncode,
            out.read().decode(errors="replace"),
            err.read().decode(errors="replace"),
        )


def run_tool(
    release: str, tool: str, args: list[str], cwd: Path, capture: bool = True
) -> "subprocess.CompletedProcess[str]":
    """Run a Borland tool (e.g. "BCC32", "TLINK32", "TDUMP") from a release."""
    exe = root_dir(release) / "BIN" / f"{tool.upper()}.EXE"
    if not exe.exists():
        sys.exit(f"error: {exe} not found; run `uv run toolchain setup` first")
    # BCC32 starts TLINK32 by searching the PATH, so put the release's BIN on it.
    return wine(exe, args, cwd, capture, path=f"{_windows_root(release)}\\BIN")


def check_release(release: str) -> bool:
    """Compile, link and run a small program; print what happened."""
    workdir = paths.TOOLCHAIN_DIR / "check" / release
    shutil.rmtree(workdir, ignore_errors=True)
    workdir.mkdir(parents=True)
    (workdir / "hello.c").write_bytes(_dos_text(_CHECK_SOURCE))

    compiled = run_tool(release, "BCC32", ["-WC", "hello.c"], workdir)
    if compiled.returncode != 0 or not (workdir / "HELLO.EXE").exists():
        print(f"Borland C++ {release}: compile/link FAILED\n{compiled.stdout}{compiled.stderr}")
        return False
    obj = (workdir / "HELLO.OBJ").read_bytes()
    ran = wine(workdir / "HELLO.EXE", [], workdir)
    output = ran.stdout.strip()
    ok = ran.returncode == 0 and "2 + 2 = 4" in output
    status = "ok" if ok else "FAILED"
    print(
        f"Borland C++ {release}: {status}. OMF object {len(obj)} bytes; program printed {output!r}"
    )
    return ok


app = typer.Typer(help=__doc__, add_completion=False, no_args_is_help=True)


def _check_release_name(release: str) -> str:
    if release not in paths.BORLAND_ISOS:
        raise typer.BadParameter(f"choose from {', '.join(paths.BORLAND_ISOS)}")
    return release


@app.command()
def setup() -> None:
    """Extract every Borland C++ CD in data/ and check each release works."""
    isos = {release: iso for release, iso in paths.BORLAND_ISOS.items() if iso.is_file()}
    if not isos:
        expected = ", ".join(str(iso) for iso in paths.BORLAND_ISOS.values())
        sys.exit(f"error: no Borland C++ CD image found (expected one of: {expected})")
    for release, iso in isos.items():
        print(f"Extracting Borland C++ {release} from {iso.name}")
        install(release, iso)
    # Check every release, even after a failure.
    results = [check_release(release) for release in installed_releases()]
    if not all(results):
        sys.exit(1)


@app.command()
def check() -> None:
    """Compile, link and run a small program with each installed release."""
    releases = installed_releases()
    if not releases:
        sys.exit("error: no toolchain installed; run `uv run toolchain setup` first")
    results = [check_release(release) for release in releases]
    if not all(results):
        sys.exit(1)


@app.command(context_settings={"allow_extra_args": True, "ignore_unknown_options": True})
def run(
    ctx: typer.Context,
    release: Annotated[
        str,
        typer.Argument(help="Borland C++ release: 4.5, 4.52 or 5.02", callback=_check_release_name),
    ],
    tool: Annotated[str, typer.Argument(help="Tool in the release's BIN, e.g. BCC32, TDUMP")],
) -> None:
    """Run a Borland tool under Wine in the current directory; extra arguments
    are passed through (e.g. `uv run toolchain run 4.5 BCC32 -c foo.c`)."""
    result = run_tool(release, tool, list(ctx.args), Path.cwd(), capture=False)
    raise typer.Exit(result.returncode)
