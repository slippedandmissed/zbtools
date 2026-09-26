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
