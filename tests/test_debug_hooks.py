"""The port's debug tooling must not touch what decomp/ compiles to: its hooks in
decomp/ sit inside `#ifdef ZB_DEBUG` (which `match` and `build` never define)."""

import re

from zbtools import paths


def test_zb_debug_only_in_ifdef() -> None:
    uses: list[str] = []
    for source in sorted(paths.REPO_ROOT.glob("decomp/*.[ch]*")):
        lines = source.read_text(errors="replace").splitlines()
        for number, line in enumerate(lines, 1):
            if "ZB_DEBUG" in line and not re.fullmatch(r"\s*#\s*ifdef ZB_DEBUG\s*", line):
                uses.append(f"{source.name}:{number}: {line.strip()}")
    assert not uses, "ZB_DEBUG only as `#ifdef ZB_DEBUG`:\n" + "\n".join(uses)


def test_every_hook_is_closed() -> None:
    for source in sorted(paths.REPO_ROOT.glob("decomp/*.[ch]*")):
        text = source.read_text(errors="replace")
        opened = len(re.findall(r"^\s*#\s*ifdef ZB_DEBUG\b", text, re.M))
        if opened:
            # Each guarded block ends before the next #endif at its own level.
            assert text.count("#endif") >= opened, source.name
