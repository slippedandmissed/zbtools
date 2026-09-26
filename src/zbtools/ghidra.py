"""Ghidra project for zoombi32.exe, driven headlessly through pyghidra.

`setup` downloads Ghidra, imports zoombi32.exe into a project in
build/ghidra/project, runs Ghidra's auto-analysis and exports every function it
found to build/ghidra/functions.json. `open` opens the project in Ghidra, and
`decompile` prints Ghidra's C for one function.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING, Annotated

import pyghidra
import typer
from pydantic import BaseModel, ConfigDict

from zbtools import download, host, paths

if TYPE_CHECKING:
    from ghidra.program.model.listing import Program

GHIDRA_VERSION = "12.1.4"
_URL = (
    "https://github.com/NationalSecurityAgency/ghidra/releases/download/"
    "Ghidra_12.1.4_build/ghidra_12.1.4_PUBLIC_20260921.zip"
)
_SHA256 = "ddac49f903da9d5bac833e5cc79395098b9c33cfd3279be5f31bd00387d2d4db"
PROGRAM_NAME = "zoombi32.exe"


class FunctionInfo(BaseModel):
    model_config = ConfigDict(frozen=True)
    address: int
    name: str
    size: int  # bytes in the function's body
    thunk: bool  # just a jump to another function (e.g. an imported one)


class FunctionList(BaseModel):
    program: str
    ghidra_version: str
    functions: list[FunctionInfo]


def install_dir() -> Path:
    """Ghidra's install directory, downloading the pinned release on first use."""
    home = paths.GHIDRA_DIST / f"ghidra_{GHIDRA_VERSION}_PUBLIC"
    if not (home / "ghidraRun").exists():
        archive = paths.GHIDRA_DIST / "ghidra.zip"
        print("Downloading Ghidra (~540 MB)")
        download.fetch(_URL, _SHA256, archive)
        download.extract_zip(archive, paths.GHIDRA_DIST)
        archive.unlink()
    _ensure_natives(home)
    return home


def _ensure_natives(home: Path) -> None:
    """The release ships native binaries (e.g. the decompiler) only for Linux and
    Windows on x86-64. Elsewhere, such as Apple Silicon, build them with the
    Gradle wrapper Ghidra includes; they land in each module's build/os/."""
    platform = host.ghidra_platform()
    decompiler = home / "Ghidra/Features/Decompiler"
    if any((decompiler / d / platform / "decompile").exists() for d in ("os", "build/os")):
        return
    host.require_native_build_tools()
    print(f"Building Ghidra's native components for {platform} (about a minute)")
    subprocess.run(
        ["./gradlew", "buildNatives", "--console=plain", "--quiet"],
        cwd=home / "support/gradle",
        env={**os.environ, "JAVA_HOME": str(host.java_home())},
        check=True,
    )


def _start() -> None:
    """Start Ghidra (headless) in this process."""
    os.environ["JAVA_HOME"] = str(host.java_home())
    if not pyghidra.started():
        pyghidra.start(install_dir=install_dir())


def list_functions(program: "Program") -> list[FunctionInfo]:
    return [
        FunctionInfo(
            address=int(function.getEntryPoint().getOffset()),
            name=str(function.getName()),
            size=int(function.getBody().getNumAddresses()),
            thunk=bool(function.isThunk()),
        )
        for function in program.getFunctionManager().getFunctions(True)
    ]


def load_functions() -> FunctionList:
    if not paths.GHIDRA_FUNCTIONS.exists():
        sys.exit(f"error: {paths.GHIDRA_FUNCTIONS} not found; run `uv run ghidra setup` first")
    return FunctionList.model_validate_json(paths.GHIDRA_FUNCTIONS.read_text())


app = typer.Typer(help=__doc__, add_completion=False, no_args_is_help=True)


@app.command()
def setup(
    force: Annotated[
        bool,
        typer.Option(
            "--force", help="Recreate the project, losing any work done in it in Ghidra's GUI"
        ),
    ] = False,
) -> None:
    """Import zoombi32.exe into a new Ghidra project and analyse it (a few minutes)."""
    exe = paths.GAME32_DIR / "zoombi32.exe"
    if not exe.exists():
        sys.exit(f"error: {exe} not found; run `uv run extract-game` first")
    if paths.GHIDRA_PROJECT_DIR.exists():
        if not force:
            sys.exit(
                f"error: the Ghidra project already exists ({paths.GHIDRA_PROJECT_DIR}); "
                "use --force to recreate it, losing any work done in it"
            )
        shutil.rmtree(paths.GHIDRA_PROJECT_DIR)
    paths.GHIDRA_PROJECT_DIR.mkdir(parents=True)

    _start()
    with pyghidra.open_project(
        paths.GHIDRA_PROJECT_DIR, paths.GHIDRA_PROJECT_NAME, create=True
    ) as project:
        loader = pyghidra.program_loader().project(project).source(str(exe)).name(PROGRAM_NAME)
        with loader.load() as results:
            results.save(pyghidra.task_monitor())
        with pyghidra.program_context(project, f"/{PROGRAM_NAME}") as program:
            print("Running Ghidra's auto-analysis (a few minutes)")
            pyghidra.analyze(program)
            program.save("Auto-analysis", pyghidra.task_monitor())
            functions = list_functions(program)

    listing = FunctionList(program=PROGRAM_NAME, ghidra_version=GHIDRA_VERSION, functions=functions)
    paths.GHIDRA_FUNCTIONS.write_text(listing.model_dump_json(indent=1))
    print(f"Found {len(functions)} functions; list written to {paths.GHIDRA_FUNCTIONS}")
    print("Next: `uv run ghidra open` to browse the project in Ghidra.")


@app.command(name="open")
def open_gui() -> None:
    """Open the project in Ghidra's GUI."""
    project_file = paths.GHIDRA_PROJECT_DIR / f"{paths.GHIDRA_PROJECT_NAME}.gpr"
    if not project_file.exists():
        sys.exit("error: no Ghidra project yet; run `uv run ghidra setup` first")
    env = {**os.environ, "JAVA_HOME": str(host.java_home())}
    subprocess.Popen([str(install_dir() / "ghidraRun"), str(project_file)], env=env)


@app.command()
def decompile(
    address: Annotated[str, typer.Argument(help="Function address in zoombi32.exe, e.g. 0x46be2e")],
) -> None:
    """Print Ghidra's C decompilation of a function: a first draft to rewrite
    into matching code. (Close the project in Ghidra's GUI first.)"""
    _start()
    # Ghidra's Java classes can only be imported once the JVM has started.
    from ghidra.app.decompiler import DecompInterface  # noqa: PLC0415

    with (
        pyghidra.open_project(paths.GHIDRA_PROJECT_DIR, paths.GHIDRA_PROJECT_NAME) as project,
        pyghidra.program_context(project, f"/{PROGRAM_NAME}") as program,
    ):
        space = program.getAddressFactory().getDefaultAddressSpace()
        function = program.getFunctionManager().getFunctionAt(space.getAddress(int(address, 16)))
        if function is None:
            sys.exit(f"error: no function starts at {address}")
        decompiler = DecompInterface()
        decompiler.openProgram(program)
        result = decompiler.decompileFunction(function, 60, pyghidra.task_monitor())
        if not result.decompileCompleted():
            sys.exit(f"error: decompiling failed: {result.getErrorMessage()}")
        print(result.getDecompiledFunction().getC())
