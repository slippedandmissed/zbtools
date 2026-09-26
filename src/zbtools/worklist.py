"""List the functions ready to decompile next, smallest first.

A function is ready when everything it calls directly is already done: matched
in decomp/, identified Borland runtime code, or outside the region being worked
on (for game code, calls into the runtime, the engine and Windows count as an
API). Calls through pointers don't block anything.
"""

from typing import Annotated

import typer

from zbtools import inventory, match
from zbtools.inventory import Region, Status

_DONE = {Status.MATCHED, Status.FUNCTIONAL, Status.LIBRARY}


def ready_and_blocked(
    functions: list[inventory.Function], region: Region, module: str | None = None
) -> tuple[list[inventory.Function], dict[int, list[int]]]:
    """Functions in the region (and module, if given) still to do that are
    ready, and for the rest, the unfinished same-region functions each one is
    waiting on."""
    by_address = {f.address: f for f in functions}
    ready, blocked = [], {}
    for f in functions:
        if f.region != region or f.status in _DONE or (module and f.module != module):
            continue
        waiting = [
            callee
            for callee in f.calls
            if callee != f.address
            and callee in by_address
            and by_address[callee].region == region
            and by_address[callee].status not in _DONE
        ]
        if waiting:
            blocked[f.address] = waiting
        else:
            ready.append(f)
    return sorted(ready, key=lambda f: (f.size, f.address)), blocked


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    region: Annotated[Region, typer.Option(help="Which code to work on")] = Region.GAME,
    limit: Annotated[int, typer.Option(help="How many ready functions to list")] = 25,
    module: Annotated[
        str | None, typer.Option(help="Only this source module (see `uv run modules`)")
    ] = None,
) -> None:
    functions = inventory.load(match.game_executable())
    ready, blocked = ready_and_blocked(functions, region, module)
    in_region = [f for f in functions if f.region == region and (not module or f.module == module)]
    done = [f for f in in_region if f.status in _DONE]
    where = f"{region} code" + (f" in {module}" if module else "")
    print(
        f"{where}: {len(in_region)} functions, {len(done)} done, "
        f"{len(ready)} ready to decompile, {len(blocked)} waiting on others."
    )
    print(f"\nReady, smallest first ({min(limit, len(ready))} of {len(ready)}):")
    for f in ready[:limit]:
        note = " (in progress: marked non-matching)" if f.status == Status.NONMATCHING else ""
        calls = f"{len(f.calls)} direct calls" + (
            f", {f.indirect_calls} indirect" if f.indirect_calls else ""
        )
        print(
            f"  {f.address:#010x}  {f.size:5} bytes  {f.name:24} {f.module or '':14} {calls}{note}"
        )
