"""Name the code a traced run executed: `uv run trace`.

`uv run vm run --exe build/rebuild/zoombi32.exe --trace <log>` has QEMU log
every block of the executable's code it runs (its `exec` log, filtered to
the code's addresses). This reads that log and the executable's map from
`uv run build` (zoombi32.map), and prints the functions the blocks were in,
as they changed: the end of the list is where the program was when the log
stopped, so after a crash, the function that crashed (then the exception
handling, if any).
"""

import bisect
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Annotated

import typer

from zbtools import paths

_CODE_SEGMENT = "0001"
# A public in the map's "Publics by Value": `0001:00000074  Idle freeanim(anim**)`.
_PUBLIC = re.compile(rf"^\s*{_CODE_SEGMENT}:([0-9A-F]{{8}})\s+(?:Idle\s+)?(.+?)\s*$")
# A logged block: `Trace 0: 0x311b5ef00 [00000000/0000000000410074/...]`, the
# second field its address.
_BLOCK = re.compile(r"^Trace \d+: 0x[0-9a-f]+ \[[0-9a-f]+/([0-9a-f]+)/", re.MULTILINE)


@dataclass(frozen=True)
class Map:
    addresses: list[int]  # of the code's publics, in order
    names: list[str]

    def function(self, address: int) -> str:
        """The public an address is in (the last one at or before it)."""
        i = bisect.bisect_right(self.addresses, address) - 1
        return self.names[i] if i >= 0 else f"{address:#x}"


def read_map(path: Path, code_start: int) -> Map:
    """The code's publics from a TLINK32 map (their offsets from code_start)."""
    publics: list[tuple[int, str]] = []
    in_publics = False
    for line in path.read_text(errors="replace").splitlines():
        in_publics = in_publics or "Publics by Value" in line
        found = _PUBLIC.match(line) if in_publics else None
        if found:
            publics.append((code_start + int(found.group(1), 16), found.group(2)))
    publics.sort()
    return Map([a for a, _ in publics], [n for _, n in publics])


def functions(log: str, code_map: Map) -> list[tuple[int, str]]:
    """The functions the logged blocks were in, as they changed, each with the
    address of its first block there."""
    found: list[tuple[int, str]] = []
    for block in _BLOCK.finditer(log):
        address = int(block.group(1), 16)
        name = code_map.function(address)
        if not found or found[-1][1] != name:
            found.append((address, name))
    return found


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    log: Annotated[Path, typer.Argument(help="The trace, from `uv run vm run --trace`")],
    last: Annotated[int, typer.Option(help="How many of the last functions to show (0: all)")] = 40,
    map_file: Annotated[
        Path, typer.Option("--map", help="The executable's map, from `uv run build`")
    ] = paths.REBUILD_DIR / "zoombi32.map",
    code_start: Annotated[
        str, typer.Option(help="Where the code segment starts in memory")
    ] = "0x410000",
) -> None:
    code_map = read_map(map_file, int(code_start, 0))
    found = functions(log.read_text(errors="replace"), code_map)
    print(f"{len(found)} changes of function")
    for address, name in found[-last:] if last else found:
        print(f"{address:#010x}  {name}")
