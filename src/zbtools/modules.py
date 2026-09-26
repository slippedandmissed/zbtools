"""The source modules of the game's own code: the curated map in
decomp/modules.toml, checked against the evidence in the executable.

TLINK32 lays each object file's code out contiguously and pads it with zeros to
a 4-byte boundary, so where a function is followed by 1-3 zero bytes up to an
aligned address, a module ends. (A module whose code is already aligned shows
no padding.) For each range of the map, this prints its size, whether its start
is confirmed by padding, the strings its code refers to and the imported
functions it calls, and it lists padding the map doesn't account for yet.
"""

import bisect
import collections
import tomllib
from dataclasses import dataclass

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import ghidra, paths, runtime_symbols
from zbtools.exe import Executable, disassemble

MODULES = paths.DECOMP_DIR / "modules.toml"
_MIN_STRING = 4


class Module(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    start: int
    name: str
    note: str = ""


class ModuleMap(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    module: list[Module]

    def of(self, address: int) -> Module | None:
        """The module an address in the game's code belongs to."""
        i = bisect.bisect_right([m.start for m in self.module], address) - 1
        return self.module[i] if i >= 0 else None


def load() -> ModuleMap:
    modules = ModuleMap.model_validate(tomllib.loads(MODULES.read_text()))
    starts = [m.start for m in modules.module]
    names = [m.name for m in modules.module]
    if starts != sorted(set(starts)) or len(names) != len(set(names)):
        raise typer.BadParameter(f"{MODULES}: starts must increase and names be unique")
    return modules


def padding_ends(
    exe: Executable, functions: list[ghidra.FunctionInfo], start: int, end: int
) -> set[int]:
    """Where the next module starts after zero padding, in [start, end): past the
    zeros up to a 4-byte boundary and any whole zero words after them."""
    found = set()
    for f in functions:
        after = f.address + f.size
        pad = exe.read(after, 3)
        zeros = len(pad) - len(pad.lstrip(b"\0"))
        if not (0 < zeros < 4 and (after + zeros) % 4 == 0):
            continue
        following = after + zeros
        while following < end and exe.read(following, 4) == bytes(4):
            following += 4
        if start <= following < end:
            found.add(following)
    return found


@dataclass(frozen=True)
class Evidence:
    functions: int
    strings: list[str]
    imports: list[str]


def _string(exe: Executable, address: int) -> str | None:
    text = exe.read(address, 64).split(b"\0")[0]
    if len(text) >= _MIN_STRING and all(32 <= b < 127 for b in text):
        return text.decode()
    return None


def evidence(
    exe: Executable, functions: list[ghidra.FunctionInfo], start: int, end: int
) -> Evidence:
    """What the code in [start, end) refers to: strings, and imported functions
    (called through their thunks)."""
    data_start, data_end = exe.sections["DATA"]
    thunks = {f.address: f.name for f in functions if f.thunk}
    strings: collections.Counter[str] = collections.Counter()
    imports: collections.Counter[str] = collections.Counter()
    inside = [f for f in functions if start <= f.address < end and not f.thunk]
    for f in inside:
        for ins in disassemble(exe.read(f.address, f.size), f.address):
            if ins.text.startswith("call 0x") and int(ins.text[5:], 16) in thunks:
                imports[thunks[int(ins.text[5:], 16)]] += 1
            for k in range(len(ins.raw) - 3):
                if ins.address + k in exe.relocations:
                    target = exe.pointer(ins.address + k)
                    text = _string(exe, target) if data_start <= target < data_end else None
                    if text:
                        strings[text] += 1
    return Evidence(
        functions=len(inside),
        strings=[s for s, _ in strings.most_common(4)],
        imports=[s for s, _ in imports.most_common(5)],
    )


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main() -> None:
    modules = load().module
    game = paths.GAME32_DIR / "zoombi32.exe"
    if not game.exists():
        raise typer.BadParameter(f"{game} not found; run `uv run extract-game` first")
    exe = Executable(game)
    functions = ghidra.load_functions().functions
    runtime = runtime_symbols.load()
    end = min(s.address for s in runtime.symbols if s.address > modules[0].start)
    padding = padding_ends(exe, functions, modules[0].start, end)
    for module, following in zip(modules, [*modules[1:], None], strict=True):
        stop = following.start if following else end
        found = evidence(exe, functions, module.start, stop)
        confirmed = "padded" if module.start in padding else "      "
        print(
            f"{module.start:#x}-{stop:#x} {confirmed} {module.name:15} "
            f"{found.functions:3} functions, {stop - module.start:6} bytes"
        )
        if found.strings:
            print(f"    strings: {found.strings}")
        if found.imports:
            print(f"    imports: {found.imports}")
    unmapped = sorted(padding - {m.start for m in modules})
    if unmapped:
        where = ", ".join(f"{a:#x}" for a in unmapped)
        print(f"\nPadding (a module end) not in {MODULES.name}: {where}")
