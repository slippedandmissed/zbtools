# The web build, packaging and hosting

## The page

`port/web/shell.html` is the page around the game: a splash with a description of the project, a link to the repository, the disclaimer that it's an unofficial fan project, a loading status and a **Play** button. Clicking Play starts the game *and* lets the browser start sound (browsers wait for a user gesture). `port/web/pre.js` runs before the game (`--pre-js`):

1. mounts **`C:` on IndexedDB** (`FS.mount(IDBFS, …, '/c')`) and loads what was saved, so saved games survive a reload; `Module.zbPersist` writes it back at most once a second (called through `hostFilesChanged`);
2. waits for Play (a run dependency);
3. once the data packages have loaded, joins files that were split into parts, and copies the installed files the data package carries (`/c-default`) into `C:` where it lacks them (saved files win).

Query switches: `?noalert` sends the game's message boxes to the console instead of an alert (automated testing); `?screenshot` has the game write `/screenshot.bmp` to the page's file system. The last 5,000 lines the game prints are kept in `window.zbLog` for scripts.

## Build flags (`port/CMakeLists.txt`)

`-sASYNCIFY -sASYNCIFY_STACK_SIZE=262144 -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH -sFORCE_FILESYSTEM -lidbfs.js -sEXIT_RUNTIME=1`. The headless target uses the same Asyncify settings with `-sENVIRONMENT=node -sNODERAWFS`.

## `uv run port package`

Builds the web page and writes everything a static host needs, and nothing else, to `build/port/site/`:

| Piece | Notes |
| --- | --- |
| `index.html`, `zoombinis.js`, `zoombinis.wasm` | the build's page renamed so a host serves it at `/` |
| `favicon.ico`, `icon.png` | from the executable's icon in `assets/zoombi32/` |
| `zoombinis-config.js` | `Module.zbArguments`: the drives and the SoundFont |
| `zoombinis-data-<n>.data`, `zoombinis-data.js` | the game's files packed with Emscripten's file packager, and their loaders |

The data packages hold: **`D:`** = the CD's `DATA/` packed from `assets/` (as `uv run assets pack` does) plus the intro movie as a scene file (`DATA/LOGO025.SCN`; the `.MOV` files are left out), `MIDIMAP.DAT`; **`C:`** = `build/port/data/c/` laid out from `assets/zoombi32/installed/` (`mohawk.w32`, `CORNER.TTF`, `Zoombini.who`) and a `Zoombi32.CFG` pointing at `D:`; and the SoundFont. **`package` needs only the repository**, not the disc.

**File size limit.** No file in the site is over 24 MiB (`port.SITE_FILE_LIMIT`), for hosts that cap file sizes (Cloudflare Pages allows 25 MiB). Files are grouped into packages under that limit (first fit, largest first); a file bigger than the limit (the 32 MB SoundFont) is split into `NAME.part0`, `NAME.part1`, … that `pre.js` joins back at start-up. `package` checks that no file is over the limit.

The browser keeps the packages in IndexedDB, so a return visit doesn't download them again.

## Hosting

Upload `build/port/site/` as it is to any static host. CI deploys it to Cloudflare Pages on pushes to `main` (`wrangler-action`, secrets `CLOUDFLARE_API_TOKEN`, `CLOUDFLARE_ACCOUNT_ID`, variable `CLOUDFLARE_PAGES_PROJECT`); the book is separate.

## Native and headless

`uv run port build native` needs SDL2 (the system's, or a pinned release built from source by CMake). `uv run port run` lays out the drives from `build/port/data/` (C:) and the packed archives (D:), then runs either build.
