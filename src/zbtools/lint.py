"""Run the code quality checks: ruff lint, ruff formatting, mypy, and the tests.

With --fix, applies ruff's safe fixes and formatting before checking.
"""

import subprocess
import sys
from typing import Annotated

import typer

from zbtools import paths

app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    fix: Annotated[bool, typer.Option("--fix", help="Apply fixes and formatting first")] = False,
) -> None:
    if fix:
        steps = [["ruff", "check", "--fix"], ["ruff", "format"], ["mypy"], ["pytest", "-q"]]
    else:
        steps = [["ruff", "check"], ["ruff", "format", "--check"], ["mypy"], ["pytest", "-q"]]

    failed = []
    for step in steps:
        print(f"$ {' '.join(step)}", flush=True)
        if subprocess.run(
            [sys.executable, "-m", *step], cwd=paths.REPO_ROOT, check=False
        ).returncode:
            failed.append(" ".join(step))
    if failed:
        sys.exit(f"failed: {', '.join(failed)}")
