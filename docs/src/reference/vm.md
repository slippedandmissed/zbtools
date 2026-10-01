# The Windows 98 VM

The original game runs in an emulated Windows 98 SE PC under QEMU, for dynamic analysis and to test the rebuilt executable. (`src/zbtools/vm.py`; automation goes through QMP with `qemu.qmp`.)

## Layers

```text
 win98.qcow2          throwaway overlay: what `vm run` boots; `vm reset` discards it
   └ win98-game.qcow2   QuickTime for Windows 2.1 and the game      (vm install-game)
       └ win98-base.qcow2   the Windows 98 SE install, read-only     (vm install)
```

- **`install`** drives an unattended Windows 98 SE setup from a customised boot floppy and `MSBATCH.INF` (product key from `.env`), then answers the logon prompt itself with a blank password. Closing the QEMU window or pressing Ctrl-C aborts and deletes the partial disk.
- **`install-game`** builds a "tools CD" (`game_install.py`) holding the QuickTime installer (with its options file named `QT32B42.INI`; see [installer settings](../formats/install-config.md)), the game's files and a filled-in `Zoombi32.CFG`, boots the VM, runs the installer and powers off.
- **`run`** boots the overlay with the game disc in `D:`. `--exe PATH` carries an executable on a floppy, copies it into the game's directory as `REBUILT.EXE` and starts it. `--trace LOG` has QEMU log the executable's code as it runs, filtered to its addresses, stopping at 500 MB or two minutes; `uv run trace LOG` names the functions from the build's map.
- **`reset`** deletes the overlay. **`screenshot`** saves a PNG of the screen (this is also how to capture images of the original game).

## Things to know

- The game refuses to start while its global atom `Zoombini` exists, which a crash leaves behind. Reboot the VM before running again.
- `screen.py` recognises the logon prompt and idle desktop by their pixels, which is how automation knows when a step is finished.
- Rosetta 2 cannot run 16-bit code, so Wine can't replace the VM on Apple Silicon.
