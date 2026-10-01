# Continuous integration

Workflows are in `.github/workflows/`.

**`pr.yml`** (pull requests), on GitHub's runners:

| Job | Does |
| --- | --- |
| `python` | `uv run lint`: ruff, ruff format, strict mypy, pytest (which includes the book's link and screenshot-index tests) |
| `book` | builds this handbook with mdBook |
| `wasm` | `uv run port setup` and `uv run port build web`: the decompiled game compiles for WebAssembly (needs only the source) |
| `regressions` | `uv run extract-game`, `toolchain setup`, `match`, `match-data`: every recorded match still matches, and the data still matches. Needs the game CD and the Borland CD, so it runs on a **self-hosted runner** labelled `zbtools-data` |

**`main.yml`** (pushes to `main`):

- Deploys the port (`uv run port package`) to Cloudflare Pages with `wrangler-action`, on a GitHub runner (it needs only the repository). Secrets: `CLOUDFLARE_API_TOKEN`, `CLOUDFLARE_ACCOUNT_ID`; variable `CLOUDFLARE_PAGES_PROJECT`.
- Runs `uv run report` on the self-hosted runner and uploads it as an artifact. The report contains the game's disassembly, so keep the repository private.

## The self-hosted runner

The jobs that need the bring-your-own files run on a runner labelled `zbtools-data`, with `ZBTOOLS_DATA` in its environment pointing at a directory holding the files that would be in `data/` (the workflow symlinks it), plus 7-Zip, Wine and JDK 21. They are skipped until the repository variable `GAME_DATA_RUNNER` is `true`.
