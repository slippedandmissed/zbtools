# About this book

The handbook is an [mdBook](https://rust-lang.github.io/mdBook/): Markdown chapters in `docs/src/`, configured by `docs/book.toml`, published at <https://docs.zoombinis.online>.

## Reading and building it

Install mdBook (`brew install mdbook`, or `cargo install mdbook`, or a [release binary](https://github.com/rust-lang/mdBook/releases)), then:

```sh
uv run book build [--open]   # renders to build/book/
uv run book serve            # http://localhost:3000, rebuilding as you edit
```

(`mdbook build docs` and `mdbook serve docs` work too; `uv run book` only adds an install hint and the screenshot tooling.) The output goes to `build/book/`, which `uv run clean book` removes.

## Writing in it

- A chapter is a Markdown file under `docs/src/<part>/`, and **must be listed in `docs/src/SUMMARY.md`**, which sets the sidebar order. Parts mirror the directories: `getting-started`, `concepts`, `reference`, `formats`, `codebase`, `gameplay`, `port`, `appendix`.
- Link between chapters with relative paths (`../codebase/startup.md`, `#anchors` are the lower-cased heading with punctuation dropped).
- Cite code by address and name (`openBridge`, `0x41a506`) so it survives renames; the marker comments make `grep -rn 0x41a506 decomp/` find it.
- State what is *confirmed* and how (addresses, strings, experiments); say when something is inferred. Keep chapters current: delete what stops being true rather than appending history.
- Gameplay chapters follow the template in [Gameplay and the code](../gameplay/index.md): on screen, scene facts, entry points, where the rules live, state, things to know.
- Screenshot placeholders and images are covered in [Screenshots](../appendix/screenshots.md).

## Checks

`uv run lint` runs `tests/test_book.py`, which fails if a chapter isn't in `SUMMARY.md`, a relative link to a chapter is broken, a screenshot id is duplicated, or the screenshot index is stale (fix with `uv run book screenshots --update`). CI also builds the book on every pull request.

## Publishing

Pushes to `main` build the book and deploy it to its own Cloudflare Pages project ([Continuous integration](ci.md)). There is nothing to do by hand.
