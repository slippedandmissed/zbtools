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

- Builds every target (`package.yml`, below), then:
  - deploys the `browser_wasm` site it built to Cloudflare Pages with `wrangler-action`. Secrets: `CLOUDFLARE_API_TOKEN`, `CLOUDFLARE_ACCOUNT_ID`; variable `CLOUDFLARE_PAGES_PROJECT`;
  - creates a GitHub release, `v<version>-build.<run number>`, with the package of every player target attached (macOS, Windows x86 and x64, Linux x64 and arm64) and generated notes. The web page and `headless_wasm` aren't released: one is deployed, the other is the testing build.
- Deploys this handbook (`mdbook build docs`, output `build/book/`) to a second Cloudflare Pages project, with the same two secrets and the variable `CLOUDFLARE_DOCS_PAGES_PROJECT`.
- The progress report (`uv run report --no-embed-binary`, after `toolchain setup`, `ghidra setup`, `runtime-symbols` and `classes`), uploaded as the artifact `report` (kept 14 days). `--no-embed-binary` leaves the original's disassembly out (its column keeps only offsets), so what anyone signed in to GitHub can download from this public repository holds none of the game's code.

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

## The game-data action

`.github/actions/game-data` brings in the bring-your-own files that `match` and `report` need (the game CD and the Borland C++ 4.5 CD) and installs 7-Zip, Wine (with its 32-bit support) and JDK 21 on the Ubuntu runner. The files are in a private Cloud Storage bucket (`gs://zbtools-ci-2610-data`, in the Google Cloud project `zbtools-ci-2610`), never public; the job reads them as the service account `ci-data-reader` (role `storage.objectViewer` on the bucket only) through **Workload Identity Federation**: the job's `id-token: write` permission gives it GitHub's OIDC token, which the pool `github` / provider `github-actions` exchanges for the service account's credentials, if the token comes from this repository (the provider's attribute condition). There are no stored keys. The downloaded files are cached (`actions/cache`, key `game-data-v1`: bump the version to fetch again) so most runs don't download them.

GitHub gives no OIDC token to pull requests from forks, so the jobs that use the action are skipped for them; a maintainer can push the branch to this repository to run them. (If the `regressions` check is required for merging, such a pull request waits for that.)

To change what the bucket holds (another release, the other ISOs): `gcloud storage cp FILE gs://zbtools-ci-2610-data/`, then fetch it in the action and bump the cache key. The pieces were created with `gcloud` (a project with billing, the bucket with uniform access and public access prevention, the service account, the pool and provider); `gcloud iam workload-identity-pools providers describe github-actions --location=global --workload-identity-pool=github --project=zbtools-ci-2610` shows the provider's condition.
