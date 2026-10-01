import re
from pathlib import Path

from zbtools import book, paths

SRC = book.SRC_DIR
_LINK = re.compile(r"\]\(([^)#]+\.md)(?:#[^)]*)?\)")


def _chapters() -> list[Path]:
    return sorted(SRC.rglob("*.md"))


def test_summary_links_exist_and_cover_every_chapter() -> None:
    summary = (SRC / "SUMMARY.md").read_text()
    linked = {(SRC / name).resolve() for name in _LINK.findall(summary)}
    assert all(path.exists() for path in linked), [p for p in linked if not p.exists()]
    chapters = {p.resolve() for p in _chapters() if p.name != "SUMMARY.md"}
    assert chapters == linked


def test_relative_links_resolve() -> None:
    broken: list[str] = []
    for chapter in _chapters():
        for target in _LINK.findall(chapter.read_text()):
            if target.startswith("http"):
                continue
            if not (chapter.parent / target).resolve().exists():
                broken.append(f"{chapter.relative_to(SRC)} -> {target}")
    assert not broken


def test_screenshot_ids_are_unique() -> None:
    ids = [item.id for item in book.placeholders()]
    assert len(ids) == len(set(ids))
    assert ids


def test_screenshot_index_is_current() -> None:
    text = book.INDEX_PAGE.read_text()
    assert book.updated_index_page(text, book.placeholders()) == text, (
        "run: uv run book screenshots --update"
    )


def test_placeholder_parsing() -> None:
    sample = SRC / "sample.md"
    chapter = "> 📷 **Screenshot: `a-b`**\n> *Shows it.*\n> *Capture:* do this.\n\ntext\n"
    items = book.placeholders_in(chapter, Path("x/sample.md"))
    assert items == [book.Placeholder("a-b", Path("x/sample.md"), "Shows it.", "do this.")]
    assert sample.name == "sample.md"


def test_image_line_depth() -> None:
    item = book.Placeholder("a-b", Path("gameplay/x.md"), "Shows it.", "")
    assert book.image_line(item) == "![Shows it.](../images/a-b.png)"
    top = book.Placeholder("a-b", Path("introduction.md"), "Shows it.", "")
    assert book.image_line(top) == "![Shows it.](images/a-b.png)"


def test_book_output_is_cleaned() -> None:
    assert paths.BOOK_DIR in paths.CLEAN_CATEGORIES["book"]
    assert "book" in paths.CLEAN_DEFAULT
