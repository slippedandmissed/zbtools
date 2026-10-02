# Continuous integration

Workflows are in `.github/workflows/`.

**`pr.yml`** (pull requests), on GitHub's runners:

| Job | Does |
| --- | --- |
| `python` | `uv run lint`: ruff, ruff format, strict mypy, pytest (which includes the book's link and screenshot-index tests) |
| `book` | builds this handbook with `mdbook build docs`, so a broken chapter or `SUMMARY.md` fails the PR |
| `package` | `package.yml`: every target builds and packages (below) |
| `regressions` | `uv run extract-game`, `toolchain setup`, `match`, `match-data`: every recorded match still matches, and the data still matches. Needs the game CD and the Borland CD, which the [game-data action](#the-game-data-action) fetches. Skipped for pull requests from forks |

**`main.yml`** (pushes to `main`):

| Job | Does |
| --- | --- |
| `package` | `package.yml`: every target builds and packages (below) |
| `deploy` | deploys the `browser_wasm` site it built to Cloudflare Pages with `wrangler-action`. Secrets: `CLOUDFLARE_API_TOKEN`, `CLOUDFLARE_ACCOUNT_ID`; variable `CLOUDFLARE_PAGES_PROJECT` |
| `release` | creates a GitHub release, `v<version>-build.<run number>`, with the package of every player target attached (macOS, Windows x86 and x64, Linux x64 and arm64) and generated notes. The web page and `headless_wasm` aren't released: one is deployed, the other is the testing build |
| `deploy-docs` | deploys this handbook (`mdbook build docs`, output `build/book/`) to a second Cloudflare Pages project, with the same two secrets and the variable `CLOUDFLARE_DOCS_PAGES_PROJECT` |
| `report` | the progress report: `extract-game`, `toolchain setup`, `ghidra setup`, `runtime-symbols`, `classes`, then `uv run report --no-embed-binary`, uploaded as the artifact `report`. Needs the game CD and the Borland CD, which the [game-data action](#the-game-data-action) fetches |

`--no-embed-binary` leaves the original's disassembly out of the report (its column keeps only offsets, and the rows that differ are still marked), so what anyone signed in to GitHub can download from this public repository holds none of the game's code: the page of a function shows our own C++ and what it compiles to. Never upload the report built without the flag.

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

## What the pipelines produce

| Output | From | Where | Kept | What it is |
| --- | --- | --- | --- | --- |
| `zoombinis-<target>` (five: `macos_universal`, `windows_x86`, `windows_x64`, `linux_x64`, `linux_arm64`) | pull requests and `main` (`package.yml`) | the run's artifacts | 14 days | the package for players of that target: a `.dmg` (macOS), `.zip` (Windows) or `.tar.gz` (Linux), with `game/` beside the program ([native builds](../port/native.md)). Zipped by GitHub, so a `.dmg` is inside a `.zip` |
| `site-browser_wasm` | pull requests and `main` | the run's artifacts | 14 days | the hostable web site (`uv run port package browser_wasm`), every file under Cloudflare Pages' limit ([the web build](../port/web.md)) |
| `gameplay-report.html` | pull requests (`gameplay`), always | the run's artifacts | 14 days | the [instrumented gameplay tests](../port/gameplay-tests.md)' report: one HTML file, uploaded unzipped, with the baseline, new picture and highlighted differences of each failing case. A summary of the failures is also on the run's page |
| `gameplay-pictures` | pull requests (`gameplay`), when it fails | the run's artifacts | 14 days | the failing pictures as files: the new one, and baseline, new and differences side by side |
| `report` | `main` (`report`) | the run's artifacts | 14 days | the progress report, without the original's disassembly: `index.html` (progress by region, module and file, and the data), `functions.html` (every function and its status) and a page per source file, with `style.css`. Build the full one locally with `uv run report` |
| a GitHub release, `v<version>-build.<n>` | `main` (`release`) | Releases | until deleted | the five `zoombinis-<target>` packages, unzipped, with generated notes |
| the web site | `main` (`deploy`) | Cloudflare Pages (the project in `CLOUDFLARE_PAGES_PROJECT`) | the latest | the game as played in a browser |
| the handbook | `main` (`deploy-docs`) | Cloudflare Pages (the project in `CLOUDFLARE_DOCS_PAGES_PROJECT`) | the latest | this book |

The `regressions` job (pull requests) produces nothing: it passes or fails. `python`, `book` and the `headless_wasm` build (`package.yml`, built only, as the testing build) produce nothing either.

Artifacts are visible to anyone signed in to GitHub, since the repository is public, so nothing in them may come from the bring-your-own files: the packages hold the game's resources only as converted in `assets/`, and the report is built without the disassembly. The cached ISOs (the `game-data` cache) are not an artifact, and the job logs print no file contents.

## The game-data action

`.github/actions/game-data` brings in the bring-your-own files that `match` and `report` need (the game CD and the Borland C++ 4.5 CD) and installs 7-Zip, Wine and JDK 21 on the Ubuntu runner. The files are in a private Cloud Storage bucket (`gs://zbtools-ci-2610-data`, in the Google Cloud project `zbtools-ci-2610`), never public; the job reads them as the service account `ci-data-reader` (role `storage.objectViewer` on the bucket only) through **Workload Identity Federation**: the job's `id-token: write` permission gives it GitHub's OIDC token, which the pool `github` / provider `github-actions` exchanges for the service account's credentials, if the token comes from this repository (the provider's attribute condition). There are no stored keys. The downloaded files are cached (`actions/cache`, key `game-data-v1`: bump the version to fetch again) so most runs don't download them.

GitHub gives no OIDC token to pull requests from forks, so the jobs that use the action are skipped for them; a maintainer can push the branch to this repository to run them. (If the `regressions` check is required for merging, such a pull request waits for that.)

To change what the bucket holds (another release, the other ISOs): `gcloud storage cp FILE gs://zbtools-ci-2610-data/`, then fetch it in the action and bump the cache key. The pieces were created with `gcloud` (a project with billing, the bucket with uniform access and public access prevention, the service account, the pool and provider); `gcloud iam workload-identity-pools providers describe github-actions --location=global --workload-identity-pool=github --project=zbtools-ci-2610` shows the provider's condition.

## Things learned running it on Linux

- Borland's tools name an output after the source file's case (`hello.c` gives `hello.exe`), and a case-sensitive file system keeps it: look files up without regard to case (as `toolchain.check_release` does), never as `HELLO.EXE`. The first CI run failed on this, and it looked like a Wine or `T:` drive problem (the tools probe for alternative spellings of a path, which Wine logs as missing files): check the output files' names before the environment.
- Wine can't run under QEMU's user-mode emulation (a `wineboot` abort in `anon_mmap_fixed`), so an amd64 container on an Apple silicon Mac can't reproduce the runner locally; push a branch and read its log instead.
