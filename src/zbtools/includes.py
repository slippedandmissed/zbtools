"""Keep each decomp/ source's module-header includes in step with what it uses.

Each game module declares its functions (and the globals and types only it
uses) in `decomp/<module>.h`; a source includes its own module's header and
those declaring a name it uses, right after `zoombinis.h`. `uv run includes`
rewrites that list for the sources given (all by default): it adds headers a
new call needs, and drops ones no longer needed, which keeps `match`'s cache
from recompiling a source for changes to headers it doesn't use.

Names are found by scanning for identifiers, which may include a header
whose name only appears in a comment; that costs a recompile now and then,
never a wrong build.
"""

import re
from pathlib import Path
from typing import Annotated

import typer

from zbtools import declarations, paths

app = typer.Typer(add_completion=False, help=__doc__.split("\n\n")[0])

_SHARED = '#include "zoombinis.h"\n'
_MODULE_INCLUDE = re.compile(r'^#include "(\w+)\.h"\n', re.MULTILINE)
_STRUCT = re.compile(r"^struct\s+(\w+)", re.MULTILINE)
_IDENTIFIER = re.compile(r"\b\w+\b")
_INLINE = re.compile(r"^inline\b[^(;{]*?\b(\w+)\s*\(", re.MULTILINE)


def declared_names(header: str) -> set[str]:
    """What a module header declares: functions (inline ones too), globals
    and structs."""
    names = {prototype.name for prototype in declarations.prototypes_in(header)}
    names |= set(declarations.extern_names_in(header))
    names |= set(_STRUCT.findall(header))
    names |= set(_INLINE.findall(header))
    return names


def needed(source: str, own: str, headers: dict[str, set[str]]) -> list[str]:
    """The module headers a source needs: its own, if there is one, and each
    declaring a name the source uses."""
    identifiers = set(_IDENTIFIER.findall(source))
    return sorted(
        module for module, names in headers.items() if module == own or identifiers & names
    )


def with_includes(source: str, modules: list[str]) -> str:
    """The source with its module-header includes replaced by `modules`'."""
    if _SHARED not in source:
        return source
    kept = _MODULE_INCLUDE.sub(lambda m: m.group(0) if m.group(1) == "zoombinis" else "", source)
    includes = "".join(f'#include "{module}.h"\n' for module in modules)
    return kept.replace(_SHARED, _SHARED + includes, 1)


def module_headers() -> dict[str, set[str]]:
    return {
        path.stem: declared_names(path.read_text())
        for path in declarations.headers()
        if path != declarations.HEADER
    }


@app.command()
def main(
    sources: Annotated[
        list[Path] | None, typer.Argument(help="Sources to update (default: all of decomp/)")
    ] = None,
) -> None:
    headers = module_headers()
    for path in sources or sorted(paths.DECOMP_DIR.glob("*.cpp")):
        text = path.read_text()
        updated = with_includes(text, needed(text, path.stem, headers))
        if updated != text:
            path.write_text(updated)
            print(f"updated {path}")
