"""This project's handbook: the mdBook in docs/ (docs/book.toml, docs/src/).

`build` renders it to build/book/, `serve` serves it with live reload, and
`screenshots` audits the screenshot placeholders in the chapters: each is a
block quote that starts `> 📷 **Screenshot: `id`**` and stays as the image's
caption. Saving docs/src/images/<id>.png and running `screenshots --embed`
puts the image above its placeholder (see the appendix chapter
"Screenshots"); `screenshots --update` rewrites the index in that chapter.
"""

import re
import subprocess
from pathlib import Path
from typing import Annotated, NamedTuple

import typer

from zbtools import host, paths

DOCS_DIR = paths.REPO_ROOT / "docs"
SRC_DIR = DOCS_DIR / "src"
IMAGES_DIR = SRC_DIR / "images"
INDEX_PAGE = SRC_DIR / "appendix" / "screenshots.md"
INDEX_START = "<!-- screenshots:start -->"
INDEX_END = "<!-- screenshots:end -->"

_PLACEHOLDER = re.compile(r"^> 📷 \*\*Screenshot: `([a-z0-9-]+)`\*\*\n((?:>.*\n?)*)", re.MULTILINE)
_CAPTURE = re.compile(r"\*Capture:\*\s*(.*)")

app = typer.Typer(add_completion=False, help=__doc__)


class Placeholder(NamedTuple):
    id: str
    chapter: Path  # relative to docs/src
    description: str
    capture: str


def mdbook() -> str:
    """The mdbook binary, or exit explaining how to install it."""
    return host.require("mdBook", "mdbook")


def placeholders_in(text: str, chapter: Path) -> list[Placeholder]:
    """The screenshot placeholders in one chapter's text."""
    found: list[Placeholder] = []
    for match in _PLACEHOLDER.finditer(text):
        lines = [line.removeprefix(">").strip() for line in match.group(2).splitlines()]
        body = " ".join(line for line in lines if line)
        capture = _CAPTURE.search(body)
        description = _CAPTURE.sub("", body).strip().strip("*").strip()
        found.append(
            Placeholder(
                match.group(1), chapter, description, capture.group(1).strip() if capture else ""
            )
        )
    return found


def placeholders(src: Path = SRC_DIR) -> list[Placeholder]:
    """Every screenshot placeholder in the chapters, file by file."""
    found: list[Placeholder] = []
    for chapter in sorted(src.rglob("*.md")):
        if chapter != INDEX_PAGE:
            found += placeholders_in(chapter.read_text(), chapter.relative_to(src))
    return found


def has_image(placeholder_id: str, images: Path = IMAGES_DIR) -> bool:
    return (images / f"{placeholder_id}.png").exists()


def image_line(item: Placeholder) -> str:
    """The Markdown that shows the image of `item` in its chapter."""
    prefix = "../" * (len(item.chapter.parts) - 1)
    return f"![{item.description}]({prefix}images/{item.id}.png)"


def embedded(item: Placeholder, src: Path = SRC_DIR) -> bool:
    return f"images/{item.id}.png)" in (src / item.chapter).read_text()


def embed(item: Placeholder, src: Path = SRC_DIR) -> None:
    """Puts the image line above `item`'s placeholder in its chapter."""
    chapter = src / item.chapter
    marker = f"> 📷 **Screenshot: `{item.id}`**"
    chapter.write_text(chapter.read_text().replace(marker, f"{image_line(item)}\n\n{marker}", 1))


def render_index(items: list[Placeholder]) -> str:
    """The table the appendix shows: one row per placeholder."""
    rows = ["| Id | Chapter | What it shows | How to capture it |", "| --- | --- | --- | --- |"]
    for item in items:
        link = f"[{item.chapter.with_suffix('').as_posix()}](../{item.chapter.as_posix()})"
        rows.append(
            f"| `{item.id}` | {link} | {item.description} | {item.capture or 'see the chapter'} |"
        )
    return "\n".join(rows)


def updated_index_page(text: str, items: list[Placeholder]) -> str:
    """`text` with the index between its markers replaced."""
    start = text.index(INDEX_START) + len(INDEX_START)
    end = text.index(INDEX_END)
    return f"{text[:start]}\n{render_index(items)}\n{text[end:]}"


@app.command()
def build(
    open_browser: Annotated[bool, typer.Option("--open", help="Open it afterwards")] = False,
) -> None:
    """Render the book to build/book/."""
    command = [mdbook(), "build", str(DOCS_DIR)]
    if open_browser:
        command.append("--open")
    subprocess.run(command, check=True)


@app.command()
def serve(
    port: Annotated[int, typer.Option(help="The port to serve on")] = 3000,
) -> None:
    """Serve the book with live reload."""
    subprocess.run([mdbook(), "serve", str(DOCS_DIR), "--port", str(port)], check=True)


@app.command()
def screenshots(
    update: Annotated[
        bool, typer.Option("--update", help="Rewrite the index in the appendix")
    ] = False,
    embed_images: Annotated[
        bool, typer.Option("--embed", help="Show captured images above their placeholders")
    ] = False,
) -> None:
    """List the screenshot placeholders and which still lack docs/src/images/<id>.png."""
    items = placeholders()
    ids = [item.id for item in items]
    duplicates = sorted({i for i in ids if ids.count(i) > 1})
    if duplicates:
        raise typer.BadParameter(f"duplicate screenshot ids: {', '.join(duplicates)}")
    if update:
        INDEX_PAGE.write_text(updated_index_page(INDEX_PAGE.read_text(), items))
    for item in items:
        if embed_images and has_image(item.id) and not embedded(item):
            embed(item)
    missing = [item for item in items if not has_image(item.id)]
    for item in items:
        if not has_image(item.id):
            status = "MISSING"
        elif embedded(item):
            status = "ok"
        else:
            status = "NOT EMBEDDED"
        print(f"{status:<12} {item.id}  ({item.chapter})")
    print(f"{len(items) - len(missing)} of {len(items)} captured")
