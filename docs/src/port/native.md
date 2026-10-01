# Native builds

`uv run port build native` builds for the machine it runs on; `uv run port bundle` packs that build with the game's data for players, into `build/port/dist/`. Neither needs the game's disc: the data comes from `assets/`, and the SoundFont is downloaded by `port setup`.

## What a package holds

The program and a `game/` directory: `C/` (what the installer would have written: the configuration, `MIDIMAP.DAT` and the font), `D/DATA/` (the CD's archives, packed from `assets/`) and the SoundFont. Started without `--drive`, the program finds `game/` beside itself (`--data` names another), and keeps `C:` in the player's own directory (`~/Library/Application Support/Zoombinis/` on macOS, SDL's preferences path elsewhere), seeded from `game/C/` the first time: saved games stay there, so the package itself is never written to.

## macOS

`port bundle` makes `Zoombinis.app` (the program in `Contents/MacOS/`, `game/` in `Contents/Resources/`, an icon made from the program's own, `Info.plist`), signs it ad hoc (Apple silicon won't run an unsigned program) and puts it in a `.dmg`. The program is universal (`arm64;x86_64`, macOS 11 or later), so SDL2 is built from the pinned release rather than taken from Homebrew, whose copy is for one architecture (`-DZB_SYSTEM_SDL=ON` takes the system's). It is not notarised (that needs an Apple developer account): the first run needs Control-click, Open.

The `native` workflow builds and uploads the `.dmg` on every pull request.

**Not run yet.** The macOS build was written without a Mac: the 64-bit parts are tested on Linux ([64-bit targets](overview.md#64-bit-targets)), but the first macOS run may find more: the fibers (`port/host/posix.cpp` uses `ucontext`, which macOS deprecates but provides) and anything Apple's clang or libc treats differently. Float-to-integer conversions also saturate on arm64 where x86 gives `0x80000000`.
