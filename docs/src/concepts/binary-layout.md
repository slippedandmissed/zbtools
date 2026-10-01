# Code layout of zoombi32.exe

`zoombi32.exe` is a PE32 executable (about 634 KB) with sections `CODE`, `DATA`, `.idata`, `.edata`, `.reloc` and `.rsrc`.

## CODE (`0x410000`-`0x494000`)

| Range | What |
| --- | --- |
| `0x410000` | Borland's Win32 startup code (`C0W32.OBJ`); the entry point |
| `0x41008c`-`0x46cca0` | the game's own code (42 modules, see [Source modules](modules.md)) |
| `0x46cca0`-`0x46d754` | QuickTime for Windows' SDK glue ([QuickTime glue](../formats/quicktime-glue.md)) |
| `0x46d754`-`0x46f7a4` | the Mohawk OS layer (`os_*.cpp`) |
| `0x46f7a4`-`0x4764bc` | the Borland C++ runtime (`CW32.LIB`), named by `uv run runtime-symbols` |
| `0x4764bc`-`0x494000` | the Mohawk engine: ports, audio, WaveMix, files, resources, memory (149 small modules) |

The three Windows API families the code imports are KERNEL32, USER32, GDI32, ADVAPI32 and WINMM; DSOUND, QTIM32, CMGR32 and VERSION are loaded by name at run time.

## What the layout says about the source

- **The game's code came from the Mac version.** Its message table (`0x4a4da4`-`0x4a4f7a`) holds messages the Windows game never shows ("Requires Sound Manager 3.1…", "GetVol failed. Boot drive."), the memory manager and resource manager imitate the Mac Toolbox, resource types are Mac four-character codes, and rectangles, regions and ports follow QuickDraw.
- **The data section follows the code's module order.** Each module's initialised data comes after the previous module's, with its string literals last, then its uninitialised data in the same order; see [Data layout](data-layout.md). This is what lets the project find module boundaries and place globals.
- **Switch tables, vtables and RTTI descriptors live in `CODE`**, which is why a linear disassembly shows odd instructions (`in`, `out`, `cli`) inside some functions: those are table bytes.
- **`atexit`** is given a six-byte `__cdecl` wrapper, because the game's functions are Pascal and `atexit` wants the C convention.

## Entry

`WinMain` (`0x4546f8`, in the `game` module) is reached only through the startup code's table at `0x4a0044`. See [Startup](../codebase/startup.md).
