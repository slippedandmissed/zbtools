# The rebuilt executable

`uv run build` compiles `decomp/` and `glue/` (reusing `match`'s objects), the icon (BRCC32) and links them with **TLINK32 1.50**, Borland C++ 4.5's linker, into `build/rebuild/zoombi32.exe` with a map.

- Linked in the original's order (each source by its first marked function; `glue/` where the code it stands in for was), with `-Tpe -aa -c`, the result has the original's six sections at the same addresses, the same export (`__GetExceptDLLinfo`) and the same icon resources, byte for byte.
- Its imports differ only where its code does: `DebugBreak` and the `Interlocked` functions replace the original's inline assembly, and `GetVersionExA` came from Apple's QuickTime glue, which `glue/quicktime.cpp` stands in for.
- It runs in the VM: `uv run vm run --exe build/rebuild/zoombi32.exe`. `--trace LOG` makes QEMU log the executable's code as it runs, and `uv run trace LOG` names the functions from the map; after a crash, the last one is where it happened.
- The game refuses to start while the global atom `Zoombini` exists, which a crash leaves behind: reboot the VM before running again.

A near-miss can be the bug: when the rebuild crashes, look at what differs in the functions it was in.
