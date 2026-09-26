"""Ghidra project for zoombi32.exe, driven headlessly through pyghidra.

`setup` downloads Ghidra, imports zoombi32.exe into a project in
build/ghidra/project, runs Ghidra's auto-analysis and exports every function it
found to build/ghidra/functions.json. `open` opens the project in Ghidra,
`decompile` prints Ghidra's C for one function, and `label` names the Borland
runtime-library code found by `uv run runtime-symbols`.
"""

import collections
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING, Annotated

import pyghidra
import typer
from pydantic import BaseModel, ConfigDict

from zbtools import download, host, paths, rtti, runtime_symbols
from zbtools.demangle import qualified_name

if TYPE_CHECKING:
    from ghidra.program.model.address import Address
    from ghidra.program.model.listing import Function, Program
    from ghidra.program.model.symbol import Namespace, SymbolTable

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

    _write_functions(functions)
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


def _write_functions(functions: list[FunctionInfo]) -> None:
    listing = FunctionList(program=PROGRAM_NAME, ghidra_version=GHIDRA_VERSION, functions=functions)
    paths.GHIDRA_FUNCTIONS.write_text(listing.model_dump_json(indent=1))


def ghidra_name(symbol: runtime_symbols.RuntimeSymbol) -> tuple[str, str, str | None]:
    """How a library symbol is named in Ghidra: (namespace path, name, full
    signature). C++ classes become namespaces; the argument list moves to the
    signature, shown as a comment; spaces go, as Ghidra doesn't allow them."""
    text = symbol.demangled
    if text == symbol.name:
        return "", text, None
    if text.startswith("vtable for "):
        return text.removeprefix("vtable for "), "vtable", text
    *scopes, name = qualified_name(symbol.name).split("::")
    return "::".join(scopes), name.replace(" ", ""), text


def _add_secondary_labels(table: "SymbolTable", at: "Address", names: list[str]) -> None:
    """Add names as extra labels at an address, skipping ones already there, and
    drop any extra label that just repeats the primary name."""
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    existing = {str(s.getName()) for s in table.getSymbols(at)}
    for name in names:
        if name not in existing:
            table.createLabel(at, name, SourceType.IMPORTED)
    primary = table.getPrimarySymbol(at)
    for extra in table.getSymbols(at):
        if not extra.isPrimary() and primary and extra.getName() == primary.getName():
            extra.delete()


def _label_runtime(program: "Program", found: runtime_symbols.RuntimeSymbols) -> tuple[int, int]:
    """Name the Borland runtime code; returns (functions named, hand-set names kept)."""
    from ghidra.app.util import NamespaceUtils  # noqa: PLC0415
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415
    from ghidra.program.model.listing import CommentType  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    symbols_at: dict[int, list[runtime_symbols.RuntimeSymbol]] = collections.defaultdict(list)
    for symbol in found.symbols:
        symbols_at[symbol.address].append(symbol)
    api = FlatProgramAPI(program)
    space = program.getAddressFactory().getDefaultAddressSpace()
    manager, table, listing = (
        program.getFunctionManager(),
        program.getSymbolTable(),
        program.getListing(),
    )
    root = program.getGlobalNamespace()
    named = kept = 0
    for address, symbols in sorted(symbols_at.items()):
        at = space.getAddress(address)
        scope, name, signature = ghidra_name(symbols[0])
        namespace = root
        if scope:
            namespace = NamespaceUtils.createNamespaceHierarchy(
                scope, root, program, SourceType.IMPORTED
            )
        # Every name this tool may have given the address, to tell them from
        # names set by hand (createFunction marks names as user-defined).
        ours = {s.name for s in symbols} | {ghidra_name(s)[1] for s in symbols}
        # Type descriptors (@$xt$...) and vtables are data in the code.
        is_data = symbols[0].name.startswith("@$xt$") or name == "vtable"
        function: Function | None = None
        if not is_data:
            # The stubs omit Java's nulls: both return None if there's no function.
            function = manager.getFunctionAt(at) or api.createFunction(at, None)
        if function is not None:
            symbol = function.getSymbol()
            if symbol.getSource() != SourceType.USER_DEFINED or function.getName() in ours:
                symbol.setNameAndNamespace(name, namespace, SourceType.IMPORTED)
                named += 1
            else:
                kept += 1
        elif not any(str(s.getName()) == name for s in table.getSymbols(at)):
            table.createLabel(at, name, namespace, SourceType.IMPORTED)
        if signature:
            listing.setComment(at, CommentType.PLATE, signature)
        _add_secondary_labels(table, at, [s.name for s in symbols])
    return named, kept


def _depths(classes: list[rtti.ClassInfo]) -> dict[str, int]:
    """Each class's depth in the hierarchy (0 for classes without bases)."""
    by_name = {c.name: c for c in classes}
    depth: dict[str, int] = {}

    def of(name: str) -> int:
        if name not in depth:
            bases = by_name[name].bases if name in by_name else []
            depth[name] = 1 + max((of(b) for b in bases), default=-1)
        return depth[name]

    return {c.name: of(c.name) for c in classes}


def _label_classes(program: "Program", classes: list[rtti.ClassInfo]) -> int:
    """Turn RTTI classes into Ghidra classes; returns how many functions were
    named. Only functions still named automatically (FUN_...) are renamed."""
    from ghidra.app.util import NamespaceUtils  # noqa: PLC0415
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType, SymbolType  # noqa: PLC0415

    api = FlatProgramAPI(program)
    space = program.getAddressFactory().getDefaultAddressSpace()
    manager, table = program.getFunctionManager(), program.getSymbolTable()
    root = program.getGlobalNamespace()
    named = 0

    def ghidra_class(name: str) -> "Namespace":
        namespace = NamespaceUtils.createNamespaceHierarchy(
            name, root, program, SourceType.ANALYSIS
        )
        if namespace.getSymbol().getSymbolType() != SymbolType.CLASS:
            return NamespaceUtils.convertNamespaceToClass(namespace)
        return namespace

    def name_function(address: int, name: str, namespace: "Namespace", containing: bool) -> None:
        nonlocal named
        at = space.getAddress(address)
        # The stubs omit Java's nulls: these return None if there's no function.
        function: Function | None = (
            manager.getFunctionContaining(at)
            if containing
            else manager.getFunctionAt(at) or api.createFunction(at, None)
        )
        if function is not None and function.getSymbol().getSource() == SourceType.DEFAULT:
            function.getSymbol().setNameAndNamespace(name, namespace, SourceType.ANALYSIS)
            named += 1

    def label(address: int, name: str, namespace: "Namespace") -> None:
        at = space.getAddress(address)
        if not any(str(s.getName()) == name for s in table.getSymbols(at)):
            table.createLabel(at, name, namespace, SourceType.ANALYSIS)

    depth = _depths(classes)
    # Constructors and destructors most-derived first: a derived constructor also
    # stores its bases' vtables, so it must be claimed by its own class first.
    for cls in sorted(classes, key=lambda c: -depth[c.name]):
        namespace = ghidra_class(cls.name)
        short = cls.name.split("::")[-1]
        label(cls.descriptor, "__tpdsc__", namespace)
        for i, vtable in enumerate(cls.vtables):
            label(vtable.address, "vtable" if i == 0 else f"vtable{i}", namespace)
        if cls.destructor is not None:
            name_function(cls.destructor, f"~{short}", namespace, containing=False)
        for site in cls.constructors:
            name_function(site, short, namespace, containing=True)
    # Virtual methods base first, so each is named after the class introducing it.
    for cls in sorted(classes, key=lambda c: depth[c.name]):
        namespace = ghidra_class(cls.name)
        for vtable in cls.vtables:
            for slot, method in enumerate(vtable.methods):
                name_function(method, f"vfunc{slot}", namespace, containing=False)
    return named


def _set_calling_conventions(program: "Program") -> int:
    """Mark functions that pop their own arguments (`ret N`) as __stdcall, so the
    decompiler shows their parameters. Borland passes `this` on the stack, so
    this covers methods too. Returns how many functions changed."""
    changed = 0
    for function in program.getFunctionManager().getFunctions(True):
        if function.isThunk() or function.isExternal():
            continue
        purge = function.getStackPurgeSize()
        if 0 < purge < 0x10000 and function.getCallingConventionName() in ("unknown", "default"):
            function.setCallingConvention("__stdcall")
            changed += 1
    return changed


@app.command()
def label() -> None:
    """Apply everything the tools have recovered to the Ghidra project: Borland
    runtime names (`uv run runtime-symbols`), C++ classes from RTTI (`uv run
    classes`), and calling conventions. Names you've set by hand are kept."""
    found = runtime_symbols.load()
    classes = rtti.load().classes
    _start()
    with (
        pyghidra.open_project(paths.GHIDRA_PROJECT_DIR, paths.GHIDRA_PROJECT_NAME) as project,
        pyghidra.program_context(project, f"/{PROGRAM_NAME}") as program,
    ):
        with pyghidra.transaction(program):
            runtime_named, kept = _label_runtime(program, found)
            class_named = _label_classes(program, classes)
            conventions = _set_calling_conventions(program)
        program.save("Recovered symbols", pyghidra.task_monitor())
        _write_functions(list_functions(program))
    print(
        f"Named {runtime_named} runtime functions (kept {kept} names set by hand) and "
        f"{class_named} class methods from {len(classes)} classes; set __stdcall on "
        f"{conventions} functions. Function list updated: {paths.GHIDRA_FUNCTIONS}"
    )
