# Zoombi32.CFG and installer settings

## Zoombi32.CFG

`Zoombi32.CFG` ships in `ZBARCHIV.Z` as an empty file. The game reads two keys from its `[INSTALL]` section at startup (the `config` module) and reports "unable to read file Zoombi32.CFG" if they're missing:

| Key | Meaning | Example |
| --- | --- | --- |
| `INSTALLFROMDIR` | Root of the game CD. The game appends `Data\` to find the `.MHK` archives, so it needs a trailing backslash. | `D:\` |
| `INSTALLTODIR` | Install directory. | `C:\Program Files\Zoombi32\` |

The original InstallShield script (`SETUP.INS`) writes these values. `vm install-game` and the port ship filled-in copies instead (`src/zbtools/game_install.py`, `port.py`'s `_CFG`). The same function checks that `<INSTALLFROMDIR>Data\Zoombini.mhk` can be opened, which is how the game decides whether the CD is in.

## QuickTime installer settings

The 32-bit QuickTime for Windows 2.1 installer on the disc (`QTWSET32/QT32B42.EXE`) reads its options from an INI named after itself, `QT32B42.INI`, not the `QT32INST.INI` shipped beside it (which belongs to the older installer in `OLD32INS.EXT/`). With `QT32B42.INI`, `PromptToBegin=0` and friends suppress every dialog, which is what lets `vm install-game` run unattended; the other name is ignored entirely.
