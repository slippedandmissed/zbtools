"""Run the code quality checks: ruff lint, ruff formatting, and mypy.

With --fix, applies ruff's safe fixes and formatting before checking.
"""

import argparse
import subprocess
import sys

from zbtools import paths


class _Args(argparse.Namespace):
    fix: bool


def main() -> None:
    doc = __doc__ or ""
    parser = argparse.ArgumentParser(description=doc.splitlines()[0])
    parser.add_argument("--fix", action="store_true", help="apply fixes and formatting first")
    args = parser.parse_args(namespace=_Args())

    if args.fix:
        steps = [["ruff", "check", "--fix"], ["ruff", "format"], ["mypy"]]
    else:
        steps = [["ruff", "check"], ["ruff", "format", "--check"], ["mypy"]]

    failed = []
    for step in steps:
        print(f"$ {' '.join(step)}", flush=True)
        if subprocess.run(
            [sys.executable, "-m", *step], cwd=paths.REPO_ROOT, check=False
        ).returncode:
            failed.append(" ".join(step))
    if failed:
        sys.exit(f"failed: {', '.join(failed)}")


if __name__ == "__main__":
    main()
