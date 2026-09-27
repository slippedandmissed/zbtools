"""Write a progress report: statistics per region, every function with its
status, and for each decompiled function its original machine code, its C++
and its recompiled machine code side by side.

The report is a small static site in build/report/: an overview (index.html),
a list of every function (functions.html), and a page per source file with its
functions' code (files/). It contains disassembly of the game, so it's for
local use: don't publish it.
"""

import datetime
import re
import shutil
import webbrowser
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import jinja2
import typer

from zbtools import ghidra, inventory, match, module_map, paths
from zbtools.demangle import qualified_name
from zbtools.exe import Instruction
from zbtools.inventory import Region, Status


@dataclass(frozen=True)
class Tally:
    functions: int = 0
    size: int = 0

    def add(self, size: int) -> "Tally":
        return Tally(self.functions + 1, self.size + size)


@dataclass(frozen=True)
class RegionStats:
    region: Region
    total: Tally
    by_status: dict[Status, Tally]

    def percent(self, *statuses: Status) -> float:
        done = sum(self.by_status.get(s, Tally()).size for s in statuses)
        return 100 * done / self.total.size if self.total.size else 0.0


_IDENTIFIER = re.compile(r"[A-Za-z_]\w*")
_BRANCH = re.compile(r"^(?:call|j\w+) (0x[0-9a-f]+)$")


@dataclass(frozen=True)
class ModuleStats:
    name: str
    start: int
    total: Tally
    done: Tally  # matched, functional or library

    @property
    def percent(self) -> float:
        return 100 * self.done.size / self.total.size if self.total.size else 0.0


@dataclass(frozen=True)
class AsmLine:
    offset: str
    raw: str
    text: str
    note: str | None  # the name of what the instruction refers to, if known


@dataclass(frozen=True)
class DetailRow:
    original: AsmLine | None
    recompiled: AsmLine | None
    same: bool


@dataclass(frozen=True)
class Detail:
    name: str
    address: int
    size: int
    status: Status  # as recorded (decomp/matching.txt and the markers)
    outcome: match.Outcome  # as measured now
    source_file: str
    release: str  # the Borland C++ release it's compiled with
    source: str
    rows: list[DetailRow]
    matching_percent: float | None  # of bytes; None if it couldn't be compiled
    error: str | None


def function_source(text: str, address: int) -> str:
    """The source of the function marked with this address: from its marker to
    the brace closing its body (or, for a compiler-generated one, its marker)."""
    implicit = match.implicit_markers(text)
    if address in implicit:
        return implicit[address]
    for marker in match.marker_positions(text):
        if marker.address != address:
            continue
        body = text.find("{", marker.end)
        depth = 0
        for i in range(body, len(text)):
            depth += {"{": 1, "}": -1}.get(text[i], 0)
            if depth == 0:
                return text[marker.start : i + 1]
    return ""


@dataclass(frozen=True)
class Names:
    """What the report calls things: functions by address, and a lookup for the
    symbols the recompiled code refers to."""

    by_address: dict[int, str]
    by_upper: dict[str, str]  # -p upper-cases functions and globals: map them back

    @staticmethod
    def of(functions: list[inventory.Function]) -> "Names":
        by_address = {f.address: f.name for f in ghidra.load_functions().functions}
        by_address |= {f.address: f.name for f in functions}
        # Globals are only named in the sources.
        identifiers = {
            i for p in match.decomp_sources() for i in _IDENTIFIER.findall(p.read_text())
        }
        known = identifiers | set(by_address.values())
        return Names(by_address, {n.upper(): n for n in sorted(known)})

    def symbol(self, target: str) -> str:
        """A readable name for an object-file symbol (mangled, `_`-prefixed C, or plain)."""
        name = qualified_name(target) if target.startswith("@") else target.removeprefix("_")
        return self.by_upper.get(name.upper(), name) if target.isupper() else name


def _original_asm(ins: Instruction | None, base: int, size: int, names: Names) -> AsmLine | None:
    """An instruction of the original, naming the function it branches to."""
    if ins is None:
        return None
    branch = _BRANCH.match(ins.text)
    target = int(branch.group(1), 16) if branch else None
    outside = target is not None and not base <= target < base + size
    note = names.by_address.get(target) if outside and target is not None else None
    return AsmLine(f"{ins.address - base:04x}", ins.raw.hex(" "), ins.text, note)


def _compiled_asm(
    ins: Instruction | None, base: int, result: match.Result, names: Names
) -> AsmLine | None:
    """A recompiled instruction, naming the symbols the linker would fill in."""
    if ins is None:
        return None
    start = ins.address - base
    targets = [
        t for offset, t in result.references.items() if start <= offset < start + len(ins.raw)
    ]
    note = ", ".join(dict.fromkeys(names.symbol(t) for t in targets)) or None
    return AsmLine(f"{start:04x}", ins.raw.hex(" "), ins.text, note)


def _detail(
    function: inventory.Function, checked: match.Checked, found: match.Outcome, names: Names
) -> Detail:
    result = checked.result
    rows, percent = [], None
    if result is not None:
        base, size = result.target.address, len(result.original)
        rows = [
            DetailRow(
                _original_asm(row.original, base, size, names),
                _compiled_asm(row.compiled, base, result, names),
                row.same,
            )
            for row in match.aligned_rows(result)
        ]
        percent = 100 * (1 - len(result.mismatches) / max(1, len(result.compiled)))
    return Detail(
        name=checked.target.name,
        address=function.address,
        size=function.size,
        status=function.status,
        outcome=found,
        source_file=str(checked.source.relative_to(paths.REPO_ROOT)),
        release=match.release_for(checked.source.read_text()),
        source=function_source(checked.source.read_text(), function.address),
        rows=rows,
        matching_percent=percent,
        error=checked.error,
    )


def _stats(functions: list[inventory.Function]) -> list[RegionStats]:
    stats = []
    for region in Region:
        total, by_status = Tally(), dict[Status, Tally]()
        for f in functions:
            if f.region == region:
                total = total.add(f.size)
                by_status[f.status] = by_status.get(f.status, Tally()).add(f.size)
        stats.append(RegionStats(region, total, by_status))
    return stats


def _module_stats(functions: list[inventory.Function]) -> list[ModuleStats]:
    stats = []
    for module in module_map.load().module:
        total, done = Tally(), Tally()
        for f in functions:
            if f.module == module.name:
                total = total.add(f.size)
                if f.status in (Status.MATCHED, Status.FUNCTIONAL, Status.LIBRARY):
                    done = done.add(f.size)
        stats.append(ModuleStats(module.name, module.start, total, done))
    return stats


@dataclass(frozen=True)
class SourceGroup:
    """The decompiled functions of one source file."""

    source_file: str
    release: str
    details: list[Detail]

    def count(self, status: Status) -> int:
        return sum(d.status == status for d in self.details)

    @property
    def discrepancies(self) -> int:
        return sum(d.outcome.discrepancy for d in self.details)

    @property
    def page(self) -> str:
        """The group's page, relative to the report's root."""
        name = Path(self.source_file).with_suffix("").as_posix().removeprefix("decomp/")
        return f"files/{name.replace('/', '-')}.html"


def _groups(details: list[Detail]) -> list[SourceGroup]:
    """Details by source file, in the order of their first function."""
    by_file: dict[str, list[Detail]] = {}
    for detail in details:
        by_file.setdefault(detail.source_file, []).append(detail)
    return [SourceGroup(file, group[0].release, group) for file, group in by_file.items()]


def build() -> dict[str, str]:
    """The report's files, by path relative to its root."""
    exe = match.game_executable()
    functions = inventory.load(exe)
    by_address = {f.address: f for f in functions}
    names = Names.of(functions)
    checked = match.check(match.decomp_sources(), exe)
    baseline = match.load_baseline()
    details = sorted(
        (
            _detail(by_address[c.target.address], c, match.outcome(c, baseline), names)
            for c in checked
            if c.target.address in by_address
        ),
        key=lambda d: d.address,
    )
    # Recorded as matching but no longer marked: the report's other way to regress.
    marked = {c.target.address for c in checked}
    unmarked = {a: n for a, n in baseline.items() if a not in marked}
    groups = _groups(details)
    environment = jinja2.Environment(
        loader=jinja2.PackageLoader("zbtools", "templates"),
        autoescape=True,
        undefined=jinja2.StrictUndefined,
    )
    common = {
        "generated": datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
        "default_release": match.DEFAULT_RELEASE,
        "stats": _stats(functions),
        "pages": {d.address: g.page for g in groups for d in g.details},
        "Status": Status,
    }
    site = {
        "style.css": environment.get_template("style.css").render(),
        "index.html": environment.get_template("index.html.j2").render(
            common,
            root="",
            module_stats=_module_stats(functions),
            details=details,
            groups=groups,
            unmarked=unmarked,
        ),
        "functions.html": environment.get_template("functions.html.j2").render(
            common,
            root="",
            functions=functions,
            outcomes={d.address: d.outcome for d in details},
        ),
    }
    page = environment.get_template("file.html.j2")
    for group in groups:
        site[group.page] = page.render(common, root="../", group=group)
    return site


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    open_report: Annotated[
        bool, typer.Option("--open", help="Open the report in a web browser")
    ] = False,
) -> None:
    match.require_toolchain()
    site = build()
    shutil.rmtree(paths.REPORT_DIR, ignore_errors=True)  # drop pages of removed files
    for name, text in site.items():
        path = paths.REPORT_DIR / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
    print(f"Report written to {paths.REPORT}")
    if open_report:
        webbrowser.open(Path(paths.REPORT).as_uri())
