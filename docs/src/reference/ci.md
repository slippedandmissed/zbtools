# Continuous integration

Workflows are in `.github/workflows/`.

**`pr.yml`** (pull requests), on GitHub's runners:

| Job | Does |
| --- | --- |
| `python` | `uv run lint`: ruff, ruff format, strict mypy, pytest (which includes the book's link and screenshot-index tests) |
| `book` | builds this handbook with `mdbook build docs`, so a broken chapter or `SUMMARY.md` fails the PR |
| `package` | `package.yml`: every target builds and packages (below) |
| `regressions` | `uv run extract-game`, `toolchain setup`, `match`, `match-data`: every recorded match still matches, and the data still matches. Needs the game CD and the Borland CD, so it runs on a **self-hosted runner** labelled `zbtools-data` |

**`main.yml`** (pushes to `main`):

- Builds every target (`package.yml`, below), then:
  - deploys the `browser_wasm` site it built to Cloudflare Pages with `wrangler-action`. Secrets: `CLOUDFLARE_API_TOKEN`, `CLOUDFLARE_ACCOUNT_ID`; variable `CLOUDFLARE_PAGES_PROJECT`;
  - creates a GitHub release, `v<version>-build.<run number>`, with the package of every player target attached (macOS, Windows x86 and x64, Linux x64 and arm64) and generated notes. The web page and `headless_wasm` aren't released: one is deployed, the other is the testing build.
- Deploys this handbook (`mdbook build docs`, output `build/book/`) to a second Cloudflare Pages project, with the same two secrets and the variable `CLOUDFLARE_DOCS_PAGES_PROJECT`.
- The progress report job (`uv run report` on the self-hosted runner, uploaded as an artifact) is commented out in `main.yml`. Two problems need solving before it is enabled: it needs the gitignored game files (the game CD and the Borland CD in `data/`), which GitHub's runners don't have; and the report contains the game's disassembly while the repository is public (anyone signed in to GitHub can download Actions artifacts), so the report must be modified not to include it before the upload step is enabled.

## `package.yml`

A reusable workflow (called by both of the above; it needs only the repository, since the game's data is in `assets/`) that runs `uv run port package <target>` for each target, on GitHub's runners:

| Job | Target | Runner | Notes |
| --- | --- | --- | --- |
| `players` | `macos_universal` | `macos-14` | smoke test: the packaged program runs for 8 seconds with SDL's dummy video and audio |
| | `windows_x86`, `windows_x64` | Ubuntu | cross-compiled with llvm-mingw (cached); not run (that would need Wine) |
| | `linux_x64` | Ubuntu | built in the container; same smoke test as macOS |
| | `linux_arm64` | `ubuntu-24.04-arm` | a native arm64 runner (free for public repositories), so the container runs without emulation; same smoke test as the others |
| `browser` | `browser_wasm` | Ubuntu | the site, uploaded as `site-browser_wasm` |
| `headless` | `headless_wasm` | Ubuntu | built only: it is the testing build |

Each player package is uploaded as the artifact `zoombinis-<target>` (kept 14 days), which is what the release attaches.

## The self-hosted runner

The jobs that need the bring-your-own files run on a runner labelled `zbtools-data`, with `ZBTOOLS_DATA` in its environment pointing at a directory holding the files that would be in `data/` (the workflow symlinks it), plus 7-Zip, Wine and JDK 21. They are skipped until the repository variable `GAME_DATA_RUNNER` is `true`.
