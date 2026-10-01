# Prerequisites

Supported hosts: **macOS on Apple Silicon** (tested) and **Linux** (should work, untested). The tools check for anything missing and tell you how to install it; they never install host packages themselves.

## Software

| Needed for | Software | Install |
| --- | --- | --- |
| everything | [uv](https://docs.astral.sh/uv/) | see its site |
| the Windows 98 VM | QEMU | `brew install qemu`; Debian/Ubuntu `qemu-system-x86 qemu-utils`; Fedora `qemu-system-x86 qemu-img` |
| the VM's setup floppy | mtools | `brew install mtools`, or the `mtools` package |
| reading the Borland CDs | 7-Zip | `brew install sevenzip`, or `7zip` |
| Ghidra | JDK 21 | `brew install openjdk@21`, or `openjdk-21-jdk` / `java-21-openjdk-devel` |
| Ghidra's native decompiler where there's no prebuilt one (Apple Silicon) | a C/C++ compiler and `make` | `xcode-select --install`, or `build-essential` |
| the Borland compiler | Wine | macOS: downloaded for you into `build/wine/` (needs [Rosetta 2](https://support.apple.com/en-us/102527)); Linux: the `wine` package |
| the port | the Emscripten SDK | downloaded for you by `uv run port setup` (~1.8 GB) |
| this book | [mdBook](https://rust-lang.github.io/mdBook/) | `brew install mdbook` or `cargo install mdbook` |

You don't need most of this for every task. Working only with `assets/` or the port needs just `uv` (and the Emscripten SDK for the port); the matcher needs Wine, 7-Zip and the Borland CD; the VM needs QEMU and a Windows CD.

**Why a VM, and why Wine for the compiler only?** Rosetta 2 cannot run 16-bit Windows code, and the QuickTime installer the game needs contains some, so Wine can't run the game on Apple Silicon. The 32-bit Borland command-line tools run fine under Wine, so those use it.

## Bring-your-own files

The original game and Windows media aren't in the repository. Put them in the gitignored `data/` directory under these names (other paths can be passed as arguments):

| File | What it is | Needed for |
| --- | --- | --- |
| `data/Logical Journey of the Zoombinis.iso` | the game CD (e.g. from the Internet Archive) | `extract-game`, `ghidra`, `match`, `assets extract`/`verify`, the VM |
| `data/Windows 98 Second Edition.iso` | Windows 98 SE install CD | `vm install` |
| `data/Borland C++ 4.5.iso` | the compiler the game was built with (4.52 also works as `Borland C++ 4.52.iso`) | `toolchain`, `match`, `build` |
| `data/Borland C++ 5.02.iso` | optional: for comparing the engine with a later compiler | `match --release 5.02` |

Create a gitignored `.env` in the repository root with your Windows product key:

```sh
WINDOWS_PRODUCT_KEY=XXXXX-XXXXX-XXXXX-XXXXX-XXXXX
```

`data/` is read-only to the tools. Everything they write goes to `build/` (also gitignored), and `uv run clean` removes it by category.

## What works from a fresh clone

No bring-your-own files are needed for `uv run assets pack`, `uv run port setup/build/package/serve`, `uv run lint` and this book. The resources and the installed game's few files are committed under `assets/`, which is why the port builds without the disc.
