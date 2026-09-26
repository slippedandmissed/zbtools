"""Delete generated files by category, never touching data/ or .env.

With no categories, removes the default set (everything cheap to rebuild);
`uv run clean all` removes everything. See --list for the categories.
"""

import argparse
import shutil
from pathlib import Path

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


class _Args(argparse.Namespace):
    categories: list[str]
    dry_run: bool
    list: bool


def main() -> None:
    doc = __doc__ or ""
    parser = argparse.ArgumentParser(
        description=doc.splitlines()[0], epilog="\n".join(doc.splitlines()[2:])
    )
    parser.add_argument(
        "categories",
        nargs="*",
        metavar="CATEGORY",
        help=f"what to remove (default: {' '.join(paths.CLEAN_DEFAULT)})",
    )
    parser.add_argument(
        "-n",
        "--dry-run",
        action="store_true",
        help="list what would be deleted without deleting it",
    )
    parser.add_argument("-l", "--list", action="store_true", help="list the categories and exit")
    args = parser.parse_args(namespace=_Args())

    if args.list:
        for name, entries in paths.CLEAN_CATEGORIES.items():
            default = " (default)" if name in paths.CLEAN_DEFAULT else ""
            print(f"{name}{default}: {', '.join(_describe(e) for e in entries)}")
        return

    unknown = [c for c in args.categories if c not in paths.CLEAN_CATEGORIES]
    if unknown:
        parser.error(
            f"unknown category: {', '.join(unknown)} "
            f"(choose from {', '.join(paths.CLEAN_CATEGORIES)})"
        )

    targets = expand(args.categories or paths.CLEAN_DEFAULT)
    if not targets:
        print("nothing to remove")
    for p in targets:
        print(f"{'would remove' if args.dry_run else 'removing'} {_describe(p)}")
        if not args.dry_run:
            if p.is_dir() and not p.is_symlink():
                shutil.rmtree(p)
            else:
                p.unlink()


if __name__ == "__main__":
    main()
