"""Delete generated files by category, never touching data/ or .env.

With no categories, removes the default set (everything cheap to rebuild);
`uv run clean all` removes everything. See --list for the categories.
"""

import shutil
from pathlib import Path
from typing import Annotated

import typer

from zbtools import paths


def expand(names: list[str], seen: set[str] | None = None) -> list[Path]:
    """Resolve category names (recursively) to the existing paths they cover."""
    seen = set() if seen is None else seen
    found: list[Path] = []
    for name in names:
        if name in seen:
            continue
        seen.add(name)
        for entry in paths.CLEAN_CATEGORIES[name]:
            if isinstance(entry, str):
                found += expand([entry], seen)
            elif "*" in str(entry):
                found += paths.REPO_ROOT.glob(str(entry.relative_to(paths.REPO_ROOT)))
            elif entry.exists() or entry.is_symlink():
                found.append(entry)
    # Drop duplicates and paths inside another path being removed.
    unique = sorted(set(found))
    return [p for p in unique if not any(q in p.parents for q in unique)]


def _describe(entry: paths.CleanEntry) -> str:
    return entry if isinstance(entry, str) else str(entry.relative_to(paths.REPO_ROOT))


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    categories: Annotated[
        list[str] | None,
        typer.Argument(
            metavar="CATEGORY...",
            help=f"What to remove (default: {' '.join(paths.CLEAN_DEFAULT)})",
            show_default=False,
        ),
    ] = None,
    dry_run: Annotated[
        bool, typer.Option("--dry-run", "-n", help="List what would be deleted without deleting it")
    ] = False,
    list_categories: Annotated[
        bool, typer.Option("--list", "-l", help="List the categories and exit")
    ] = False,
) -> None:
    if list_categories:
        for name, entries in paths.CLEAN_CATEGORIES.items():
            default = " (default)" if name in paths.CLEAN_DEFAULT else ""
            print(f"{name}{default}: {', '.join(_describe(e) for e in entries)}")
        return

    unknown = [c for c in categories or [] if c not in paths.CLEAN_CATEGORIES]
    if unknown:
        raise typer.BadParameter(
            f"unknown category: {', '.join(unknown)} "
            f"(choose from {', '.join(paths.CLEAN_CATEGORIES)})",
            param_hint="CATEGORY",
        )

    targets = expand(categories or paths.CLEAN_DEFAULT)
    if not targets:
        print("nothing to remove")
    for p in targets:
        print(f"{'would remove' if dry_run else 'removing'} {_describe(p)}")
        if not dry_run:
            if p.is_dir() and not p.is_symlink():
                shutil.rmtree(p)
            else:
                p.unlink()
