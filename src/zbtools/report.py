"""Write a progress report: statistics per region, every function with its
status, and for each decompiled function its original machine code, its C++
and its recompiled machine code side by side.

The report is a single self-contained HTML file in build/report/. It contains
disassembly of the game, so it's for local use: don't publish it.
"""

import datetime
import re
import webbrowser
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import jinja2
import typer

from zbtools import ghidra, inventory, match, paths
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
    status: Status
    source_file: str
    source: str
    rows: list[DetailRow]
    matching_percent: float | None  # of bytes; None if it couldn't be compiled
    error: str | None


def function_source(text: str, address: int) -> str:
    """The source of the function marked with this address: from its marker to
    the brace closing its body."""
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


def _detail(function: inventory.Function, checked: match.Checked, names: Names) -> Detail:
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
        source_file=str(checked.source.relative_to(paths.REPO_ROOT)),
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


def build(release: str) -> str:
    exe = match.game_executable()
    functions = inventory.load(exe)
    by_address = {f.address: f for f in functions}
    names = Names.of(functions)
    checked = match.check(match.decomp_sources(), release, exe)
    details = sorted(
        (
            _detail(by_address[c.target.address], c, names)
            for c in checked
            if c.target.address in by_address
        ),
        key=lambda d: d.address,
    )
    environment = jinja2.Environment(
        loader=jinja2.PackageLoader("zbtools", "templates"),
        autoescape=True,
        undefined=jinja2.StrictUndefined,
    )
    return environment.get_template("report.html.j2").render(
        generated=datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
        release=release,
        stats=_stats(functions),
        functions=functions,
        details=details,
        Status=Status,
    )


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    open_report: Annotated[
        bool, typer.Option("--open", help="Open the report in a web browser")
    ] = False,
) -> None:
    html = build(match.default_release())
    paths.REPORT_DIR.mkdir(parents=True, exist_ok=True)
    paths.REPORT.write_text(html)
    print(f"Report written to {paths.REPORT}")
    if open_report:
        webbrowser.open(Path(paths.REPORT).as_uri())
