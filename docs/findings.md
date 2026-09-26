# Findings

Confirmed facts about the game and its installer, with where they came from.

## Zoombi32.CFG (install locations)

`Zoombi32.CFG` ships in `ZBARCHIV.Z` as an empty file. The game treats it as an INI file and reads two keys from its `[INSTALL]` section at startup, showing "unable to read file Zoombi32.CFG" if they're missing:

| Key | Meaning | Example |
| --- | --- | --- |
| `INSTALLFROMDIR` | Root of the game CD. The game appends `Data\` to find the `.MHK` archives, so it needs a trailing backslash. | `D:\` |
| `INSTALLTODIR` | Install directory. The game appends `Zoombini.mhk`, presumably to look for a hard-disk copy of the data. | `C:\Program Files\Zoombi32\` |

The original InstallShield script (`SETUP.INS`, which references `InstallFromDir` and `InstallToDir`) writes these values. `vm install-game` ships a filled-in copy instead (`src/zbtools/game_install.py`).

Evidence (zoombi32.exe): the two functions at `0x446969` and around `0x446a00` build the name `Zoombi32.CFG` and call the game's INI reader at `0x480790` with section `INSTALL` (`0x4a5254`) and keys `INSTALLFROMDIR` (`0x4a3f06`) / `INSTALLTODIR` (`0x4a3f1b`); on failure they report the string at `0x4a520c`. The first then appends `Data\` (`0x4a3f15`) to the value; the second appends `Zoombini.mhk` (`0x4a526b`).

## QuickTime installer settings file

The 32-bit QuickTime for Windows 2.1 installer on the disc (`QTWSET32/QT32B42.EXE`) reads its options from an INI named after itself (`QT32B42.INI`), not the `QT32INST.INI` shipped next to it (which belongs to the older installer in `QTWSET32/OLD32INS.EXT/`). Confirmed in the VM: with the options file named `QT32B42.INI`, `PromptToBegin=0` etc. suppress every dialog; named `QT32INST.INI`, the installer ignored it entirely (it even created a Start-menu group despite `CreateGroups=0`).

## Compiler version: Borland C++ 4.5 or 4.52

Both builds were made with **Borland C++ 4.5 or 4.52**; which of the two is still open.

| Evidence | Implies |
| --- | --- |
| 16-bit `ZOOMBINI._EX`: NE header linker version 6.1 | TLINK 7.0a (Nov 1994), which shipped only with Borland C++ 4.5 and 4.52. BC++ 4.0's TLINK 6.00 writes 5.0; BC++ 5.0's TLINK 7.1 writes 7.1 (per the TLINK version table on [VOGONS](https://www.vogons.org/viewtopic.php?t=110504)) |
| Both builds: RTL string `Borland C++ - Copyright 1994 Borland Intl.` | a 4.x runtime library; BC++ 5.0's says 1996 |
| Both builds: C++ exception handling and RTTI (`**BCCxh1`, `Bad_typeid`, `typeinfo`, `xalloc`; `zoombi32.exe` exports `__GetExceptDLLinfo`) | BC++ 4.0 or later |
| Built January 1996 (disc file dates, README) | earlier than BC++ 5.0 |
| 32-bit `zoombi32.exe`: PE linker version 2.25, subsystem version 3.10, section names `CODE`/`DATA` | Borland TLINK32; doesn't distinguish 4.5 from 4.52 on its own |

4.5 and 4.52 share the same linker, so telling them apart needs the compilers themselves: compare their runtime library code (e.g. startup code and `CW32.LIB` routines) byte-for-byte with the code linked into `zoombi32.exe`.
