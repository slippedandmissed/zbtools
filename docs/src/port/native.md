# Native builds

`uv run port build native` builds for the machine it runs on; `uv run port bundle` packs that build with the game's data for players, into `build/port/dist/`. Neither needs the game's disc: the data comes from `assets/`, and the SoundFont is downloaded by `port setup`.

## What a package holds

The program and a `game/` directory: `C/` (what the installer would have written: the configuration, `MIDIMAP.DAT` and the font), `D/DATA/` (the CD's archives, packed from `assets/`) and the SoundFont. Started without `--drive`, the program finds `game/` beside itself (`--data` names another), and keeps `C:` in the player's own directory (`~/Library/Application Support/Zoombinis/` on macOS, SDL's preferences path elsewhere), seeded from `game/C/` the first time: saved games stay there, so the package itself is never written to.

## macOS

`port bundle` makes `Zoombinis.app` (the program in `Contents/MacOS/`, `game/` in `Contents/Resources/`, an icon made from the program's own, `Info.plist`), signs it ad hoc (Apple silicon won't run an unsigned program) and puts it in a `.dmg`. The program is universal (`arm64;x86_64`, macOS 11 or later), so SDL2 is built from the pinned release rather than taken from Homebrew, whose copy is for one architecture (`-DZB_SYSTEM_SDL=ON` takes the system's). It is not notarised (that needs an Apple developer account): the first run needs Control-click, Open.

The `native` workflow builds and uploads the `.dmg` on every pull request.

**Not run yet.** The macOS build was written without a Mac: the 64-bit parts are tested on Linux ([64-bit targets](overview.md#64-bit-targets)), but the first macOS run may find more: the fibers (`port/host/posix.cpp` uses `ucontext`, which macOS deprecates but provides) and anything Apple's clang or libc treats differently. Float-to-integer conversions also saturate on arm64 where x86 gives `0x80000000`.

## Windows

`uv run port bundle win32` (32-bit, which also runs on 64-bit Windows) or `win64` cross-compiles with [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) from macOS, Linux or Windows, and writes `zoombinis-<version>-windows-x86.zip` (or `x64`) to `build/port/dist/`: `zoombinis.exe` (the runtime is linked in, with the game's icon, and no console unless built with `--debug-tools`) beside `game/`. `uv run port setup-windows` installs the pinned llvm-mingw into `build/llvm-mingw/` (`ZB_MINGW_DIR` uses an existing installation, for any mingw-w64 clang); `port/toolchains/mingw.cmake` is the CMake toolchain file. Saved games go to `%APPDATA%\zoombinis\Zoombinis\C`.

The Windows build is the game on `miniwin` like the others: miniwin implements the Win32 subset the game uses over SDL, so nothing here calls the host's Win32 API except the fibers (`port/host/windows.cpp`). Where Windows' C library already has Borland's `itoa`, `stricmp` and the like, miniwin doesn't define them.

**Status.** The 64-bit program has been run under Wine (the intro and the map render); the 32-bit one is built but not run. The llvm-mingw release's checksums are not pinned yet (`_MINGW_HOSTS` in `port.py`): the first `setup-windows` prints what the download hashes to, which should be checked against the release page and pasted in.
