"""The handbook's screenshots, captured from the headless port.

`docs/screenshots.toml` holds a recipe for each screenshot placeholder
(`uv run book screenshots` lists them): the debug commands (see the handbook's
"Debug tools") that take the game to the moment, how long to let it run, and
optionally a crop. `uv run shots` runs the recipes (several at once, each on a
fresh C: drive, so a recipe never sees what another saved) and writes
docs/src/images/<id>.png, from the 640x480 screen the game was showing when
its commands ended with `quit`. It needs the headless build
(`uv run port build headless`); the game's pictures and sounds are packed from
assets/ as `uv run port run` does.

The captures depend on timing (the game runs in real time and a few things in
it are random: the party's names, some idle animations), so a rerun gives
equivalent pictures, not identical ones.
"""

import subprocess
import tempfile
import tomllib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Annotated, NamedTuple

import typer
from PIL import Image
from pydantic import BaseModel, ConfigDict

from zbtools import assets, book, paths, port

RECIPES = paths.REPO_ROOT / "docs" / "screenshots.toml"

app = typer.Typer(add_completion=False, help=__doc__)


class Recipe(BaseModel):
    """How to capture one screenshot."""

    model_config = ConfigDict(extra="forbid")

    commands: str  # debug commands; they end with `quit`, which saves the last frame
    # More pictures from the same run, each taken with `screenshot ID` in the commands (the
    # recipe's own picture is the last frame, unless the commands take `screenshot <its id>`).
    also: list[str] = []
    seconds: float = 60  # the longest the game may run (a safety net)
    crop: tuple[int, int, int, int] | None = (
        None  # (left, top, right, bottom) of the 640x480 screen
    )


class Setup(NamedTuple):
    """What a capture runs on and writes to."""

    node: Path
    program: Path  # the headless build's zoombinis.js
    packed: Path  # D:, as `uv run assets pack` lays it out
    images: Path  # where the PNGs go


def load_recipes(path: Path = RECIPES) -> dict[str, Recipe]:
    """The recipes in `path`, by screenshot id."""
    with path.open("rb") as file:
        data = tomllib.load(file)
    return {id_: Recipe.model_validate(table) for id_, table in data.items()}


def game_command(setup: Setup, c: Path, recipe: Recipe, bmp: Path) -> list[str]:
    """The command line that runs `recipe` on the headless build."""
    return [
        str(setup.node),
        str(setup.program),
        "--drive",
        f"C={c}",
        "--cdrom",
        f"D={setup.packed.resolve()},{port.CD_LABEL},{port.CD_SERIAL}",
        "--program",
        port.PROGRAM,
        "--screenshot",
        str(bmp),
        "--run-for",
        str(int(recipe.seconds * 1000)),
        "--cmd",
        recipe.commands,
    ]


def capture(id_: str, recipe: Recipe, setup: Setup) -> str:
    """Runs one recipe; "" if the picture is in `setup.images`, else what went wrong."""
    with tempfile.TemporaryDirectory(prefix=f"shots-{id_}-") as work:
        c = port.lay_out_drives(c=Path(work) / "c")
        bmp = Path(work) / "screen.bmp"
        run = subprocess.run(
            game_command(setup, c, recipe, bmp),
            capture_output=True,
            text=True,
            timeout=recipe.seconds + 60,
            check=False,
        )
        problems = [
            line
            for line in (run.stdout + run.stderr).splitlines()
            if "RuntimeError" in line or "[zbdebug] error" in line or "assertion failed" in line
        ]
        if problems:
            return "; ".join(problems[:2])
        for name in [id_, *recipe.also]:
            taken = Path(work) / f"{name}.bmp"
            source = taken if taken.exists() else bmp
            if not source.exists() or (name != id_ and not taken.exists()):
                return f"no picture for {name} (the commands need `screenshot {name}`)"
            with Image.open(source) as screen:
                picture = screen.convert("RGB")
                if recipe.crop and name == id_:
                    picture = picture.crop(recipe.crop)
                # (The screen has at most 256 colours, so a palette keeps it exactly, much smaller.)
                picture.convert("P", palette=Image.Palette.ADAPTIVE, colors=256).save(
                    setup.images / f"{name}.png", optimize=True
                )
    return ""


@app.command()
def main(
    ids: Annotated[list[str] | None, typer.Argument(help="Which (default: all)")] = None,
    jobs: Annotated[int, typer.Option(help="How many to capture at once")] = 4,
    embed: Annotated[bool, typer.Option(help="Then show them above their placeholders")] = True,
) -> None:
    """Capture the screenshots whose recipes are in docs/screenshots.toml."""
    recipes = load_recipes()
    chosen = ids or list(recipes)
    unknown = [i for i in chosen if i not in recipes]
    if unknown:
        raise typer.BadParameter(f"no recipe for: {', '.join(unknown)}")
    program = port.build_dir("headless") / "zoombinis.js"
    nodes = sorted((paths.EMSDK_DIR / "node").glob("*/bin/node"))
    if not program.exists() or not nodes:
        print("build the headless port first: uv run port build headless")
        raise typer.Exit(1)
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in port.pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    book.IMAGES_DIR.mkdir(parents=True, exist_ok=True)

    setup = Setup(nodes[-1], program, paths.PACKED_ASSETS_DIR, book.IMAGES_DIR)

    def one(id_: str) -> tuple[str, str]:
        return id_, capture(id_, recipes[id_], setup)

    failed = 0
    with ThreadPoolExecutor(jobs) as pool:
        for id_, problem in pool.map(one, chosen):
            print(f"{'FAILED' if problem else 'ok':<8}{id_}  {problem}")
            failed += bool(problem)
    if embed:
        for item in book.placeholders():
            if book.has_image(item.id) and not book.embedded(item):
                book.embed(item)
        book.INDEX_PAGE.write_text(
            book.updated_index_page(book.INDEX_PAGE.read_text(), book.placeholders())
        )
    raise typer.Exit(1 if failed else 0)
