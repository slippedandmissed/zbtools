# Native builds

`uv run port package [TARGET]` builds a target and packs it with the game's data for players, into `build/port/dist/`: `zoombinis-<version>-<target>/` and its archive (a `.dmg`, `.zip` or `.tar.gz`). With no target it is the machine's own; the targets are in [the overview](overview.md#commands). It needs no game disc: the data comes from `assets/`, and the SoundFont is downloaded by `port setup`. (`port build` is the development build, with the debug tools, in `build/port/<target>/`.)

## What a package holds

The program and a `game/` directory: `C/` (what the installer would have written: the configuration, `MIDIMAP.DAT` and the font), `D/DATA/` (the CD's archives, packed from `assets/`) and the SoundFont. Started without `--drive`, the program finds `game/` beside itself (`--data` names another), and keeps `C:` in the player's own directory (`~/Library/Application Support/Zoombinis/` on macOS, SDL's preferences path elsewhere), seeded from `game/C/` the first time: saved games stay there, so the package itself is never written to.

## macOS

`port package macos_universal` (on a Mac: it needs Xcode) makes `Zoombinis.app` (the program in `Contents/MacOS/`, `game/` in `Contents/Resources/`, an icon made from the program's own, `Info.plist`), signs it ad hoc (Apple silicon won't run an unsigned program) and puts it in a `.dmg`. The program is universal (`arm64;x86_64`, macOS 11 or later), so SDL2 is built from the pinned release rather than taken from Homebrew, whose copy is for one architecture (`-DZB_SYSTEM_SDL=ON` takes the system's). It is not notarised (that needs an Apple developer account): the first run needs Control-click, Open.

CI builds every target on each pull request, and releases the packages on each push to `main` ([CI](../reference/ci.md)).

**Status.** Run on a Mac (Apple silicon) through the game. Float-to-integer conversions saturate on arm64 where x86 gives `0x80000000`, which the decompiled code might rely on somewhere not yet reached; the fibers (`port/host/posix.cpp`) use `ucontext`, which macOS deprecates but provides.

## Windows

`uv run port package windows_x86` (32-bit, which also runs on 64-bit Windows) or `windows_x64` cross-compiles with [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) from macOS, Linux or Windows, and writes `zoombinis-<version>-windows_x86.zip` (or `windows_x64`) to `build/port/dist/`: `zoombinis.exe` (the runtime is linked in, with the game's icon, and no console unless built with `--debug-tools`) beside `game/`. `uv run port setup windows_x64` installs the pinned llvm-mingw into `build/llvm-mingw/` (`ZB_MINGW_DIR` uses an existing installation, for any mingw-w64 clang); `port/toolchains/mingw.cmake` is the CMake toolchain file. Saved games go to `%APPDATA%\zoombinis\Zoombinis\C`.

The Windows build is the game on `miniwin` like the others: miniwin implements the Win32 subset the game uses over SDL, so nothing here calls the host's Win32 API except the fibers (`port/host/windows.cpp`). Where Windows' C library already has Borland's `itoa`, `stricmp` and the like, miniwin doesn't define them.

**Status.** The 64-bit program has been run under Wine (the intro and the map render); the 32-bit one is built but not run. llvm-mingw's release checksums are pinned in `_MINGW_HOSTS` in `port.py`.

## Linux

`uv run port package linux_x64` (or `linux_arm64`) builds in a container, so any host with Docker can make it (Docker Desktop on a Mac; `host.docker()` says how to install it), and writes `zoombinis-<version>-linux_x64.tar.gz` to `build/port/dist/`: `zoombinis` beside `game/`, like the Windows package. `port/docker/linux.Dockerfile` is the environment: Ubuntu 22.04 with clang and the libraries SDL2 is built against, and CMake from PyPI (the port needs 3.24; Ubuntu has 3.22). The repository is mounted at `/src` and the build goes in `build/port/linux_x64/` as for the other targets.

Two choices keep the program portable. SDL2 is built from source and linked statically (`-DZB_SYSTEM_SDL=OFF`), and SDL opens X11, Wayland, PulseAudio and ALSA with `dlopen` when it starts, so none of them has to be installed to run (a desktop has them). The C++ runtime is linked in. What's left is glibc, which the program needs at the version it was built on or later: 2.35, which is Ubuntu 22.04, Debian 12, Fedora 36 and SteamOS 3 or newer. A build for another architecture than the host's runs under emulation (Docker Desktop does this with Rosetta or QEMU), so it is slow.

Saved games go in `~/.local/share/zoombinis/Zoombinis/C` (SDL's preferences path).

**Status.** Built through the container on a Mac, and the program (built natively with the same options) has been run through every scene on Linux.
