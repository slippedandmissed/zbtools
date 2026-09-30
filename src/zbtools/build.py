"""Build zoombi32.exe from source: `uv run build`.

Compiles decomp/ and glue/ (reusing `match`'s compiled objects, in
build/match-cache/), compiles the executable's resources from
assets/zoombi32/ (exe_resources.py) with BRCC32, and links it all with
TLINK32 into build/rebuild/zoombi32.exe, with a map of where everything went
(zoombi32.map).

The objects are linked in the original's order, so that the rebuilt
executable is laid out like it: Borland's startup code (C0W32.OBJ) first,
then each source in the order of its code in the original (the first
function it marks; glue/ at the code it stands in for), then the runtime
library (CW32.LIB) and the Windows imports (IMPORT32.LIB). The options are
the original's: a Windows GUI program, its sections aligned to 64 KB.

glue/ holds code standing in for what the game links but we don't have:
QuickTime for Windows' SDK glue (glue/quicktime.cpp, a stand-in that plays
no movies).
"""

import shutil
from pathlib import Path
from typing import Annotated

import typer

from zbtools import exe_resources, match, paths, quicktime, toolchain

EXE = "zoombi32.exe"
# Where glue/'s sources go in the link order: at the code they stand in for.
_GLUE_ADDRESSES = {"quicktime": quicktime.GLUE_START}
# TLINK32: a PE executable (-Tpe) for Windows' GUI (-aa), case-sensitive (-c),
# with a map of the segments and publics (-m, -s).
_LINK_OPTIONS = ["-Tpe", "-aa", "-c", "-m", "-s"]


class BuildError(RuntimeError):
    pass


def sources() -> list[Path]:
    """What goes into the executable: decomp/ and glue/."""
    return [*match.decomp_sources(), *sorted(paths.GLUE_DIR.glob("*.cpp"))]


def link_order(sources: list[Path]) -> list[Path]:
    """The sources in the order of their code in the original: by the first
    function each marks, glue/'s by the code it stands in for; any without
    either last, by name."""

    def first(source: Path) -> int:
        if source.parent == paths.GLUE_DIR:
            return _GLUE_ADDRESSES.get(source.stem, 2**32)
        targets = match.find_targets(source.read_text())
        return min((t.address for t in targets), default=2**32)

    return sorted(sources, key=lambda s: (first(s), s.name))


def compile_objects(sources: list[Path], out_dir: Path) -> list[Path]:
    """Compile the sources (or reuse their cached objects) and copy the
    objects into out_dir, named after their sources."""
    compiled = match.compile_sources(sources)
    errors = {s: c for s, c in compiled.items() if isinstance(c, str)}
    if errors:
        report = "\n".join(f"{s.relative_to(paths.REPO_ROOT)}: {e}" for s, e in errors.items())
        raise BuildError(f"these don't compile:\n{report}")
    stems = [s.stem.lower() for s in sources]
    if len(set(stems)) != len(stems):
        raise BuildError("two sources have the same name (their objects would collide)")
    shutil.rmtree(out_dir, ignore_errors=True)  # drop objects of removed sources
    out_dir.mkdir(parents=True)
    objects = []
    for source in sources:
        result = compiled[source]
        assert not isinstance(result, str)
        target = out_dir / f"{source.stem}.obj"
        shutil.copyfile(result.path, target)
        objects.append(target)
    return objects


def compile_resources(out_dir: Path, release: str) -> Path:
    """The executable's resources, compiled from assets/zoombi32/ into a .RES file."""
    script = out_dir / "zoombi32.rc"
    script.write_text(exe_resources.resource_script(out_dir))
    res = out_dir / "zoombi32.res"
    res.unlink(missing_ok=True)
    result = toolchain.run_tool(release, "BRCC32", ["-r", f"-fo{res.name}", script.name], out_dir)
    if result.returncode != 0 or not res.exists():
        raise BuildError(f"BRCC32 failed:\n{result.stdout}{result.stderr}")
    return res


def response_file(objects: list[Path], out_dir: Path, release: str) -> str:
    """TLINK32's response file: options, objects, executable, map, libraries,
    (no) definition file and resources, each field after a comma, lines
    continued with `+`."""
    startup = toolchain.library_path(release, "C0W32.OBJ")
    names = [startup, *(f"obj\\{o.name}" for o in objects)]
    libraries = [toolchain.library_path(release, lib) for lib in ("IMPORT32.LIB", "CW32.LIB")]
    return (
        " ".join(_LINK_OPTIONS)
        + " +\n"
        + " +\n".join(names)
        + f"\n{EXE}\nzoombi32.map\n"
        + " +\n".join(libraries)
        + "\n\nzoombi32.res\n"
    )


def link(objects: list[Path], out_dir: Path, release: str) -> Path:
    (out_dir / "link.rsp").write_text(
        response_file(objects, out_dir, release).replace("\n", "\r\n")
    )
    exe = out_dir / EXE
    exe.unlink(missing_ok=True)
    result = toolchain.run_tool(release, "TLINK32", ["@link.rsp"], out_dir)
    output = f"{result.stdout}{result.stderr}"
    if result.returncode != 0 or not exe.exists():
        raise BuildError(f"TLINK32 failed:\n{output}")
    if output.strip():
        print(output.strip())
    return exe


def build(out_dir: Path = paths.REBUILD_DIR, release: str = match.DEFAULT_RELEASE) -> Path:
    toolchain.ensure_prefix()
    objects = compile_objects(link_order(sources()), out_dir / "obj")
    compile_resources(out_dir, release)
    return link(objects, out_dir, release)


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    out_dir: Annotated[
        Path, typer.Option(help="Where to build the executable")
    ] = paths.REBUILD_DIR,
) -> None:
    match.require_toolchain()
    try:
        exe = build(out_dir)
    except BuildError as e:
        print(f"error: {e}")
        raise typer.Exit(1) from None
    print(f"Built {exe.relative_to(paths.REPO_ROOT)}")
