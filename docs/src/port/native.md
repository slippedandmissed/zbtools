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

## Linux

`uv run port bundle linux-x64` (or `linux-arm64`) builds in a container, so any host with Docker can make it (Docker Desktop on a Mac; `host.docker()` says how to install it), and writes `zoombinis-<version>-linux-x64.tar.gz` to `build/port/dist/`: `zoombinis` beside `game/`, like the Windows package. `port/docker/linux.Dockerfile` is the environment: Ubuntu 22.04 with clang and the libraries SDL2 is built against, and CMake from PyPI (the port needs 3.24; Ubuntu has 3.22). The repository is mounted at `/src` and the build goes in `build/port/linux-x64/` as for the other targets.

Two choices keep the program portable. SDL2 is built from source and linked statically (`-DZB_SYSTEM_SDL=OFF`), and SDL opens X11, Wayland, PulseAudio and ALSA with `dlopen` when it starts, so none of them has to be installed to run (a desktop has them). The C++ runtime is linked in. What's left is glibc, which the program needs at the version it was built on or later: 2.35, which is Ubuntu 22.04, Debian 12, Fedora 36 and SteamOS 3 or newer. A build for another architecture than the host's runs under emulation (Docker Desktop does this with Rosetta or QEMU), so it is slow.

Saved games go in `~/.local/share/zoombinis/Zoombinis/C` (SDL's preferences path).

**Status.** The Linux program itself (built natively with the same options) has been run through every scene; the container image has not been built (the sandbox this was written in has no Docker daemon).
