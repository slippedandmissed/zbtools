"""Ghidra project for zoombi32.exe, driven headlessly through pyghidra.

`setup` downloads Ghidra, imports zoombi32.exe into a project in
build/ghidra/project, runs Ghidra's auto-analysis and exports every function it
found to build/ghidra/functions.json. `open` opens the project in Ghidra,
`decompile` prints Ghidra's C for one function, and `label` applies the names
the other tools have recovered (runtime library, classes, and the functions
decompiled in decomp/). `codec` does the same for the movies' video codec,
qb32.qtc, in a project of its own.
"""

import collections
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING, Annotated

import pyghidra
import typer
from pydantic import BaseModel, ConfigDict

from zbtools import (
    declarations,
    download,
    host,
    match,
    module_map,
    paths,
    quicktime,
    rtti,
    runtime_symbols,
)
from zbtools.demangle import qualified_name
from zbtools.exe import Executable

if TYPE_CHECKING:
    from ghidra.program.model.address import Address
    from ghidra.program.model.data import DataType
    from ghidra.program.model.listing import Function, Instruction, Program
    from ghidra.program.model.symbol import Namespace, SymbolTable

GHIDRA_VERSION = "12.1.4"
_URL = (
    "https://github.com/NationalSecurityAgency/ghidra/releases/download/"
    "Ghidra_12.1.4_build/ghidra_12.1.4_PUBLIC_20260921.zip"
)
_SHA256 = "ddac49f903da9d5bac833e5cc79395098b9c33cfd3279be5f31bd00387d2d4db"
PROGRAM_NAME = "zoombi32.exe"
CODEC_PROGRAM_NAME = "qb32.qtc"
# Where the codec's QuickTime component dispatches to: the entry point and the
# handlers for its selectors (open, preflight, decompress, ...). They pass the
# selector in a register, so Ghidra doesn't find them by itself.
CODEC_ENTRY_POINTS = (0x10001B40, 0x10001DD0, 0x10002360, 0x10002370, 0x10008568)
TYPES_CATEGORY = "/zoombinis"  # where the types from decomp/'s headers go


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
    addresses: Annotated[
        list[str], typer.Argument(help="Function addresses in zoombi32.exe, e.g. 0x46be2e")
    ],
) -> None:
    """Print Ghidra's C decompilation of functions: first drafts to rewrite
    into matching code, each after a `/* ==== address */` line. (Close the
    project in Ghidra's GUI first.)"""
    _start()
    # Ghidra's Java classes can only be imported once the JVM has started.
    from ghidra.app.decompiler import DecompInterface  # noqa: PLC0415

    with (
        pyghidra.open_project(paths.GHIDRA_PROJECT_DIR, paths.GHIDRA_PROJECT_NAME) as project,
        pyghidra.program_context(project, f"/{PROGRAM_NAME}") as program,
    ):
        space = program.getAddressFactory().getDefaultAddressSpace()
        decompiler = DecompInterface()
        decompiler.openProgram(program)
        for address in addresses:
            at = space.getAddress(int(address, 16))
            function: Function | None = program.getFunctionManager().getFunctionAt(at)
            if function is None:
                sys.exit(f"error: no function starts at {address}")
            result = decompiler.decompileFunction(function, 60, pyghidra.task_monitor())
            if not result.decompileCompleted():
                sys.exit(f"error: decompiling {address} failed: {result.getErrorMessage()}")
            if len(addresses) > 1:
                print(f"/* ==== {address} */")
            print(result.getDecompiledFunction().getC())


@app.command()
def codec() -> None:
    """Import the movies' video codec, qb32.qtc, into a Ghidra project of its own
    (build/ghidra/qb32/, analysed on first use) and write Ghidra's C for every
    function in it to build/ghidra/qb32.c: the source of docs/findings.md's
    account of the QkBk format. It's the original's code: never commit it."""
    dll = paths.GAME32_DIR / CODEC_PROGRAM_NAME
    if not dll.exists():
        sys.exit(f"error: {dll} not found; run `uv run extract-game` first")
    _start()
    from ghidra.app.decompiler import DecompInterface  # noqa: PLC0415

    directory = paths.GHIDRA_CODEC_PROJECT_DIR
    fresh = not (directory / f"{CODEC_PROGRAM_NAME}.gpr").exists()
    directory.mkdir(parents=True, exist_ok=True)
    with pyghidra.open_project(directory, CODEC_PROGRAM_NAME, create=True) as project:
        if fresh:
            loader = pyghidra.program_loader().project(project).source(str(dll))
            with loader.name(CODEC_PROGRAM_NAME).load() as results:
                results.save(pyghidra.task_monitor())
        with pyghidra.program_context(project, f"/{CODEC_PROGRAM_NAME}") as program:
            if fresh:
                print("Running Ghidra's auto-analysis")
                pyghidra.analyze(program)
                space = program.getAddressFactory().getDefaultAddressSpace()
                with pyghidra.transaction(program):
                    for entry in CODEC_ENTRY_POINTS:
                        _create_function(program, space.getAddress(entry))
                program.save("Auto-analysis", pyghidra.task_monitor())
            decompiler = DecompInterface()
            decompiler.openProgram(program)
            chunks = []
            for function in program.getFunctionManager().getFunctions(True):
                result = decompiler.decompileFunction(function, 60, pyghidra.task_monitor())
                body = result.getDecompiledFunction().getC() if result.decompileCompleted() else ""
                chunks.append(f"/* ==== {function.getEntryPoint()} {function.getName()} */\n{body}")
    paths.GHIDRA_CODEC_C.write_text("\n".join(chunks))
    print(f"Wrote {len(chunks)} functions to {paths.GHIDRA_CODEC_C}")


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


def _create_function(program: "Program", at: "Address") -> "Function | None":
    """Create a function at `at`, disassembling it first: FlatProgramAPI's
    createFunction doesn't, and on bytes Ghidra hasn't decoded it makes a
    function whose body is just its first byte. An instruction decoded out of
    step across `at` is cleared first."""
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415

    api, listing = FlatProgramAPI(program), program.getListing()
    if listing.getInstructionAt(at) is None:
        straddling: Instruction | None = listing.getInstructionContaining(at)
        if straddling is not None:
            listing.clearCodeUnits(straddling.getMinAddress(), straddling.getMaxAddress(), False)
        api.disassemble(at)
    return api.createFunction(at, None)


def _repair_empty_functions(program: "Program") -> list[int]:
    """Disassemble functions with no instruction at their entry (made by
    createFunction on undecoded bytes, before `_create_function`) and recompute
    their bodies. Returns the functions repaired."""
    from ghidra.app.cmd.function import CreateFunctionCmd  # noqa: PLC0415
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415

    api, listing = FlatProgramAPI(program), program.getListing()
    repaired = []
    for function in list(program.getFunctionManager().getFunctions(True)):
        entry = function.getEntryPoint()
        if function.isExternal() or listing.getInstructionAt(entry) is not None:
            continue
        api.disassemble(entry)
        CreateFunctionCmd.fixupFunctionBody(program, function, pyghidra.task_monitor())
        repaired.append(int(entry.getOffset()))
    return repaired


def _code_pointer(program: "Program", exe: Executable, site: int) -> bool:
    """Whether the relocated pointer at `site` may point to a function: it's
    outside the code section, or in an instruction of a function, except in a
    memory operand (`[eax*0x4 + table]`, `[eax + table]`: code reading data in
    the code section, such as a switch's tables). The others in the code
    section are data there, such as jump-table entries."""
    start, end = exe.code_range
    if not start <= site < end:
        return True
    at = program.getAddressFactory().getDefaultAddressSpace().getAddress(site)
    instruction: Instruction | None = program.getListing().getInstructionContaining(at)
    function: Function | None = program.getFunctionManager().getFunctionContaining(at)
    if instruction is None or function is None:
        return False
    operand = re.compile(rf"\[[^\]]*0x0*{exe.pointer(site):x}\]", re.IGNORECASE)
    return not operand.search(str(instruction))


def _label_runtime(program: "Program", found: runtime_symbols.RuntimeSymbols) -> tuple[int, int]:
    """Name the Borland runtime code; returns (functions named, hand-set names kept)."""
    from ghidra.app.util import NamespaceUtils  # noqa: PLC0415
    from ghidra.program.model.listing import CommentType  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    symbols_at: dict[int, list[runtime_symbols.RuntimeSymbol]] = collections.defaultdict(list)
    for symbol in found.symbols:
        symbols_at[symbol.address].append(symbol)
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
            function = manager.getFunctionAt(at) or _create_function(program, at)
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
    from ghidra.program.model.symbol import SourceType, SymbolType  # noqa: PLC0415

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
            else manager.getFunctionAt(at) or _create_function(program, at)
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


def _label_decompiled(program: "Program") -> tuple[int, int]:
    """Name functions as they're named in decomp/ (by their @zoombi32 markers), so
    Ghidra's view of their callers reads better. Returns (functions named,
    hand-set names kept)."""
    from ghidra.app.util import NamespaceUtils  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    space = program.getAddressFactory().getDefaultAddressSpace()
    manager, root = program.getFunctionManager(), program.getGlobalNamespace()
    named = kept = 0
    for source in match.decomp_sources():
        for target in match.find_targets(source.read_text()):
            if target.name.startswith("<"):  # <startup>, <exit>: unnamed
                continue
            at = space.getAddress(target.address)
            *scopes, name = target.name.split("::")
            name = name.replace(" ", "")  # Ghidra's names have no spaces: `operatornew`
            namespace = root
            if scopes:
                namespace = NamespaceUtils.createNamespaceHierarchy(
                    "::".join(scopes), root, program, SourceType.IMPORTED
                )
            # The stubs omit Java's nulls: both return None if there's no function.
            function: Function | None = manager.getFunctionAt(at)
            created = function is None
            if function is None:
                new: Function | None = _create_function(program, at)
                if new is None:
                    continue
                function = new
            symbol = function.getSymbol()
            # createFunction marks its names user-defined; other names set by hand are kept.
            if created or symbol.getSource() != SourceType.USER_DEFINED:
                symbol.setNameAndNamespace(name, namespace, SourceType.IMPORTED)
                named += 1
            elif function.getName() != name:
                kept += 1
    return named, kept


def _label_quicktime(program: "Program", exe: Executable) -> int:
    """Name QuickTime's glue: its loader functions and its dispatch stubs, most of
    which the game never calls, so Ghidra doesn't find them. Returns how many
    functions were named."""
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    space = program.getAddressFactory().getDefaultAddressSpace()
    manager, root = program.getFunctionManager(), program.getGlobalNamespace()
    named = 0
    for glue in quicktime.functions(exe):
        at = space.getAddress(glue.address)
        function: Function | None = manager.getFunctionAt(at)
        created = function is None
        if function is None:
            new: Function | None = _create_function(program, at)
            if new is None:
                continue
            function = new
        symbol = function.getSymbol()
        if created or symbol.getSource() != SourceType.USER_DEFINED:
            symbol.setNameAndNamespace(glue.name, root, SourceType.IMPORTED)
            named += 1
    return named


_PROLOGUE = b"\x55\x8b\xec"  # push ebp / mov ebp, esp


def _find_missed_functions(
    program: "Program", exe: Executable, not_code: set[int], module_starts: set[int]
) -> list[int]:
    """Create the functions Ghidra's analysis missed (e.g. WinMain, which only
    the startup code's table points to): code that a relocated pointer points
    to (see `_code_pointer`; except the addresses in `not_code`, data that
    lives in the code section); module starts; code that follows a function (after any zero
    padding) and starts with `push ebp / mov ebp, esp`; and the targets of
    direct calls that aren't in a function. Repeats until nothing new turns
    up, as new functions' calls lead to more. Returns the functions created."""

    space = program.getAddressFactory().getDefaultAddressSpace()
    manager, listing = program.getFunctionManager(), program.getListing()
    start, end = exe.code_range

    def missing(address: int) -> bool:
        containing: Function | None = manager.getFunctionContaining(space.getAddress(address))
        return start <= address < end and containing is None

    created: list[int] = []
    while True:
        pointed = {
            exe.pointer(site)
            for site in exe.relocations
            if start <= exe.pointer(site) < end and _code_pointer(program, exe, site)
        }
        candidates = {a for a in (pointed - not_code) | module_starts if missing(a)}
        for function in manager.getFunctions(True):
            if function.isThunk() or function.isExternal():
                continue
            after = int(function.getBody().getMaxAddress().getOffset()) + 1
            while after < end and exe.read(after, 1) == b"\0":
                after += 1
            if exe.read(after, 3) == _PROLOGUE and missing(after):
                candidates.add(after)
            for instruction in listing.getInstructions(function.getBody(), True):
                if instruction.getFlowType().isCall():
                    candidates.update(
                        int(flow.getOffset())
                        for flow in instruction.getFlows()
                        if missing(int(flow.getOffset()))
                    )
        new = []
        for candidate in sorted(candidates):
            if not missing(candidate):
                continue
            if _create_function(program, space.getAddress(candidate)) is not None:
                new.append(candidate)
        if not new:
            return created
        created += new


def _remove_false_functions(program: "Program", exe: Executable, not_code: set[int]) -> list[int]:
    """Remove functions made from data: any at the addresses in `not_code`, and
    ones an earlier version of `_find_missed_functions` created at switch cases
    (jump-table entries) or tables: still named automatically (or as a case,
    by Ghidra's switch analysis), not
    called, and pointed to only by data in the code section (see
    `_code_pointer`). The code they decoded goes too; `_recover_switches` then
    gives the cases back to their functions. Returns the functions removed."""
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    manager, references = program.getFunctionManager(), program.getReferenceManager()
    listing = program.getListing()
    sites: dict[int, list[int]] = collections.defaultdict(list)
    for site in exe.relocations:
        sites[exe.pointer(site)].append(site)
    removed = []
    for function in list(manager.getFunctions(True)):
        entry = function.getEntryPoint()
        pointers = sites.get(int(entry.getOffset()), [])
        if int(entry.getOffset()) in not_code:
            manager.removeFunction(entry)
            removed.append(int(entry.getOffset()))
            continue
        automatic = function.getSymbol().getSource() == SourceType.DEFAULT or str(
            function.getName()
        ).startswith(("caseD_", "switchD_"))
        # A switch case never starts with a stack frame's prologue.
        if not pointers or not automatic or exe.read(int(entry.getOffset()), 3) == _PROLOGUE:
            continue
        if any(_code_pointer(program, exe, site) for site in pointers):
            continue
        if any(r.getReferenceType().isCall() for r in references.getReferencesTo(entry)):
            continue
        body = function.getBody()
        manager.removeFunction(entry)
        listing.clearCodeUnits(body.getMinAddress(), body.getMaxAddress(), False)
        removed.append(int(entry.getOffset()))
    return removed


def _continue_after_breakpoints(program: "Program") -> list[int]:
    """Ghidra treats `int3` as never returning, so a function with a breakpoint
    compiled into it (e.g. 0x46db83) is cut short there. Where a function ends
    at an `int3` and what follows isn't another function, make the breakpoint
    fall through, disassemble the rest and recompute the function's body.
    Returns the functions fixed."""
    from ghidra.app.cmd.function import CreateFunctionCmd  # noqa: PLC0415
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415

    monitor = pyghidra.task_monitor()
    listing, manager = program.getListing(), program.getFunctionManager()
    fixed = []
    for function in list(manager.getFunctions(True)):
        last: Instruction | None = listing.getInstructionContaining(
            function.getBody().getMaxAddress()
        )
        if last is None or str(last.getMnemonicString()) != "INT3":
            continue
        after: Address | None = last.getMaxAddress().next()
        if after is None or manager.getFunctionAt(after) is not None:
            continue
        last.setFallThrough(after)
        FlatProgramAPI(program).disassemble(after)
        CreateFunctionCmd.fixupFunctionBody(program, function, monitor)
        fixed.append(int(function.getEntryPoint().getOffset()))
    return fixed


def _recover_switches(program: "Program") -> list[int]:
    """Where a function has a jump through a table Ghidra couldn't follow (a
    `switch`, whose table Borland puts right after the jump), decompile it
    again, now its callees' calling conventions are known, let the decompiler
    recover the table, and recompute the function's body; repeat while that
    uncovers more. Also decode known cases that have lost their code. Returns
    the functions fixed."""

    from ghidra.app.cmd.function import (  # noqa: PLC0415
        CreateFunctionCmd,
        DecompilerSwitchAnalysisCmd,
    )
    from ghidra.app.decompiler import DecompInterface  # noqa: PLC0415
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: PLC0415

    monitor = pyghidra.task_monitor()
    api = FlatProgramAPI(program)
    listing, manager = program.getListing(), program.getFunctionManager()
    decompiler = DecompInterface()
    decompiler.openProgram(program)
    fixed: list[int] = []
    for _ in range(4):
        found = 0
        for function in list(manager.getFunctions(True)):
            jumps = [
                i
                for i in listing.getInstructions(function.getBody(), True)
                if i.getFlowType().isJump() and i.getFlowType().isComputed()
            ]
            # Cases whose code was cleared (with a false function made at one)
            # still have their flows: decode them again.
            lost = [t for i in jumps for t in i.getFlows() if listing.getInstructionAt(t) is None]
            if not lost and all(i.getFlows() for i in jumps):
                continue
            size = function.getBody().getNumAddresses()
            for target in lost:
                api.disassemble(target)
            if not all(i.getFlows() for i in jumps):
                result = decompiler.decompileFunction(function, 60, monitor)
                DecompilerSwitchAnalysisCmd(result).applyTo(program, monitor)
            CreateFunctionCmd.fixupFunctionBody(program, function, monitor)
            if function.getBody().getNumAddresses() > size:
                found += 1
                fixed.append(int(function.getEntryPoint().getOffset()))
        if not found:
            break
    decompiler.dispose()
    return sorted(set(fixed))


def _merge_jump_labels(program: "Program", exe: Executable) -> list[int]:
    """Hand-written loops load a label's address into a register to jump to it
    later (`mov ebx, label` ... `jmp ebx`: copyPixels' inner loop, 0x4899ed,
    and drawPackedPixels', 0x48cfc9 and 0x48d03f), and Ghidra makes the label a
    function, as it's relocated. Where every relocated reference to an
    automatically named function is a `mov r32, imm32` inside the function
    before it (including labels merged into that one already), merge it into
    that function. A callback's address is stored or pushed instead, never
    loaded by `mov r32`. Returns the labels merged."""
    from ghidra.program.model.address import AddressSet  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    manager = program.getFunctionManager()
    sites: dict[int, list[int]] = collections.defaultdict(list)
    for site in exe.relocations:
        sites[exe.pointer(site)].append(site)
    merged = []
    owner: Function | None = None
    for function in list(manager.getFunctions(True)):
        entry = int(function.getEntryPoint().getOffset())
        references = sites.get(entry, [])
        automatic = function.getSymbol().getSource() == SourceType.DEFAULT
        if (
            owner is not None
            and automatic
            and references
            and all(
                0xB8 <= exe.read(site - 1, 1)[0] <= 0xBF
                and owner.getBody().contains(function.getEntryPoint().getNewAddress(site - 1))
                for site in references
            )
        ):
            body = AddressSet(owner.getBody())
            body.add(function.getBody())
            manager.removeFunction(function.getEntryPoint())
            owner.setBody(body)
            merged.append(entry)
            continue
        owner = function
    return merged


def _merge_fragments(program: "Program") -> list[int]:
    """Ghidra sometimes starts a function after another's first instructions:
    0x478572 is a 6-byte prologue that falls through into a "function" at
    0x478578. Where a function nothing refers to is entered only by falling
    through from the function before it (from an instruction other than a
    call, which might not return), merge it into that function. Names set by
    hand are left alone. Returns the fragments merged."""
    from ghidra.program.model.address import AddressSet  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    manager, listing = program.getFunctionManager(), program.getListing()
    references = program.getReferenceManager()
    merged = []
    for function in list(manager.getFunctions(True)):
        if function.isThunk() or function.isExternal():
            continue
        if function.getSymbol().getSource() == SourceType.USER_DEFINED:
            continue
        entry = function.getEntryPoint()
        if references.hasReferencesTo(entry):
            continue
        previous: Instruction | None = listing.getInstructionBefore(entry)
        if previous is None or previous.getFallThrough() != entry:
            continue
        if previous.getFlowType().isCall():
            continue
        owner: Function | None = manager.getFunctionContaining(previous.getMinAddress())
        if owner is None:
            continue
        body = AddressSet(owner.getBody())
        body.add(function.getBody())
        manager.removeFunction(entry)
        owner.setBody(body)
        merged.append(int(entry.getOffset()))
    return merged


@dataclass(frozen=True)
class Declared:
    """What `_apply_declarations` did."""

    types: int
    named: int
    typed: int
    kept: int  # globals whose name or type was set by hand, left alone


def _add_types(program: "Program", c: str) -> int:
    """Parse the structs in decomp/'s headers with Ghidra's C parser and put them in
    the program's /zoombinis category, replacing earlier versions in place (so
    data already typed with them follows). Windows types they use (such as
    PALETTEENTRY) come from Ghidra's own Windows type archive."""
    from ghidra.app.util.cparser.C import CParser  # noqa: PLC0415
    from ghidra.program.model.data import (  # noqa: PLC0415
        CategoryPath,
        DataTypeConflictHandler,
        FileDataTypeManager,
        StandAloneDataTypeManager,
    )

    dtm = program.getDataTypeManager()
    windows = FileDataTypeManager.openFileArchive(_windows_archive(), False)
    # Parse into a scratch manager: the parser's typedefs stay there.
    scratch = StandAloneDataTypeManager("zoombinis", dtm.getDataOrganization())
    transaction = scratch.startTransaction("parse")
    try:
        parser = CParser(scratch, False, [windows])
        parser.parse(c)
        composites = list(parser.getComposites().values())
        for composite in composites:
            composite.setCategoryPath(CategoryPath(TYPES_CATEGORY))
        for composite in composites:
            dtm.resolve(composite, DataTypeConflictHandler.REPLACE_HANDLER)
    finally:
        scratch.endTransaction(transaction, False)
        scratch.close()
        windows.close()
    return len(composites)


def _windows_archive() -> Path:
    """Ghidra's archive of 32-bit Windows types."""
    return install_dir() / "Ghidra/Features/Base/data/typeinfo/win32/windows_vs12_32.gdt"


def _ours(data_type: "DataType") -> bool:
    """Whether a type is one these tools apply: ours, a builtin or undefined."""
    path = str(data_type.getPathName())
    return path.startswith(f"{TYPES_CATEGORY}/") or path.count("/") == 1


# C builtin type names as Ghidra names them.
_BUILTINS = {"unsigned long": "ulong", "unsigned short": "ushort", "unsigned char": "uchar"}


def _data_type(program: "Program", text: str) -> "DataType | None":
    """A type from decomp/'s headers (a builtin, one of our structs, or a
    Windows type such as HWND from the types Ghidra imported with the program,
    with any number of `*`s) as a Ghidra data type; None if Ghidra has none."""
    from ghidra.program.model.data import BuiltInDataTypeManager  # noqa: PLC0415

    dtm = program.getDataTypeManager()
    base = text.replace("*", "").strip()
    found: DataType | None = dtm.getDataType(f"{TYPES_CATEGORY}/{base}")
    if found is None:
        builtin: DataType | None = BuiltInDataTypeManager.getDataTypeManager().getDataType(
            f"/{_BUILTINS.get(base, base)}"
        )
        found = builtin
    if found is None:
        # Windows' ANSI types are macros for their A forms (WNDCLASS is WNDCLASSA).
        names = {base, f"{base}A"}
        found = next((t for t in dtm.getAllDataTypes() if str(t.getName()) in names), None)
    if found is None:
        return None
    for _ in range(text.count("*")):
        found = dtm.getPointer(found)
    return found


def _apply_declarations(program: "Program") -> Declared:
    """Give Ghidra decomp/'s headers' types, and their globals' names and types.
    Names set by hand in Ghidra are kept, and so are types other than ours,
    builtins and undefined data."""
    from ghidra.program.model.data import DataUtilities  # noqa: PLC0415
    from ghidra.program.model.symbol import SourceType  # noqa: PLC0415

    globals_, c = declarations.load()
    types = _add_types(program, c)
    space = program.getAddressFactory().getDefaultAddressSpace()
    table, listing = program.getSymbolTable(), program.getListing()
    named = typed = kept = 0
    for declared in globals_:
        at = space.getAddress(declared.address)
        symbol = table.getPrimarySymbol(at)
        if symbol is None or symbol.getSource() == SourceType.DEFAULT:
            table.createLabel(at, declared.name, SourceType.IMPORTED)
            named += 1
        elif symbol.getSource() == SourceType.IMPORTED:
            if str(symbol.getName()) != declared.name:
                symbol.setName(declared.name, SourceType.IMPORTED)
                named += 1
        elif str(symbol.getName()) != declared.name:
            kept += 1
        data_type = _data_type(program, declared.type)
        if data_type is None:
            print(f"No Ghidra type {declared.type!r} for {declared.name}: left untyped.")
            continue
        existing = listing.getDataAt(at)
        if existing is not None and existing.getDataType().isEquivalent(data_type):
            continue
        if existing is not None and existing.isDefined() and not _ours(existing.getDataType()):
            kept += 1
            continue
        DataUtilities.createData(
            program, at, data_type, -1, DataUtilities.ClearDataMode.CLEAR_ALL_CONFLICT_DATA
        )
        typed += 1
    return Declared(types, named, typed, kept)


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
    classes`), the names of functions decompiled in decomp/, calling
    conventions, the types and globals declared in decomp/'s headers, the
    ends of functions Ghidra cut short at a breakpoint or a switch, and the
    fragments and jump labels it split off functions.
    Names you've set by hand are kept."""
    found = runtime_symbols.load()
    classes = rtti.load().classes
    exe = Executable(paths.GAME32_DIR / PROGRAM_NAME)
    _start()
    with (
        pyghidra.open_project(paths.GHIDRA_PROJECT_DIR, paths.GHIDRA_PROJECT_NAME) as project,
        pyghidra.program_context(project, f"/{PROGRAM_NAME}") as program,
    ):
        with pyghidra.transaction(program):
            descriptors = {c.descriptor for c in classes}
            cases = _remove_false_functions(program, exe, descriptors)
            repaired = _repair_empty_functions(program)
            missed = _find_missed_functions(
                program,
                exe,
                not_code=descriptors,
                module_starts={m.start for m in module_map.load().module},
            )
            runtime_named, kept = _label_runtime(program, found)
            class_named = _label_classes(program, classes)
            quicktime_named = _label_quicktime(program, exe)
            decomp_named, decomp_kept = _label_decompiled(program)
            declared = _apply_declarations(program)
            breakpoints = _continue_after_breakpoints(program)
            conventions = _set_calling_conventions(program)
            switches = _recover_switches(program)
            fragments = _merge_fragments(program)
            labels = _merge_jump_labels(program, exe)
        program.save("Recovered symbols", pyghidra.task_monitor())
        _write_functions(list_functions(program))
    print(
        f"Named {runtime_named} runtime functions (kept {kept} names set by hand) and "
        f"{class_named} class methods from {len(classes)} classes, {quicktime_named} "
        f"QuickTime glue functions and {decomp_named} "
        f"decompiled functions (kept {decomp_kept} names set by hand); set __stdcall on "
        f"{conventions} functions. Function list updated: {paths.GHIDRA_FUNCTIONS}"
    )
    print(
        f"From decomp/'s headers: {declared.types} types; named {declared.named} and typed "
        f"{declared.typed} globals (kept {declared.kept} names or types set by hand)."
    )
    if missed:
        where = ", ".join(f"{a:#x}" for a in missed)
        print(f"Created {len(missed)} functions Ghidra's analysis missed: {where}")
    if cases:
        where = ", ".join(f"{a:#x}" for a in cases)
        print(f"Removed {len(cases)} functions made from data: {where}")
    if repaired:
        where = ", ".join(f"{a:#x}" for a in repaired)
        print(f"Disassembled {len(repaired)} functions that had no code: {where}")
    if switches:
        where = ", ".join(f"{a:#x}" for a in switches)
        print(f"Recovered switch tables in {where}.")
    if breakpoints:
        where = ", ".join(f"{a:#x}" for a in breakpoints)
        print(f"Disassembled past a breakpoint (int3) in {where}.")
    if fragments:
        where = ", ".join(f"{a:#x}" for a in fragments)
        print(f"Merged {len(fragments)} fragments into the functions before them: {where}.")
    if labels:
        where = ", ".join(f"{a:#x}" for a in labels)
        print(f"Merged {len(labels)} jump labels into the functions that use them: {where}.")
