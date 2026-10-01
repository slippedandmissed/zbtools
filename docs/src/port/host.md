# The host layer

`port/host/host.h` is the interface; `emscripten.cpp`, `posix.cpp` and `windows.cpp` implement it for the three kinds of target. miniwin uses SDL for everything SDL covers (window, input, audio, timing); the host layer is the rest.

| Function | Meaning |
| --- | --- |
| `hostFiberCurrent`, `hostFiberCreate(stackSize, entry, arg)`, `hostFiberSwitch`, `hostFiberDelete` | **fibers**: execution contexts switched cooperatively. They carry both the engine's threads and miniwin's Win32 threads |
| `hostYield(ms)` | give the host its turn (the browser's event loop) for about `ms` at most (0: just a turn) |
| `hostMessageBox(title, text, buttons, count)` | show a message and wait for a button; its index |
| `hostTrace`, `hostTraceStack` | diagnostics on the host's console |
| `hostFilesChanged` | called once miniwin has flushed files it wrote, to persist them where the host must (IndexedDB) |

Nothing in `port/host/` includes miniwin's headers, so a host file can use a host's own Win32 API.

## Per target

| Target | Fibers | `hostYield` | Persistence |
| --- | --- | --- | --- |
| web (`emscripten.cpp`) | Emscripten's fibers, built on **Asyncify** (each fiber has a C stack and an Asyncify stack of 256 KB, the game's deepest calls go through a few hundred frames) | returns to the browser through Asyncify and comes back | C: is mounted on IndexedDB (`pre.js`) and `hostFilesChanged` calls `Module.zbPersist` (at most once a second) |
| POSIX (`posix.cpp`) | `ucontext` | does nothing | the files are on disk |
| Windows (`windows.cpp`) | Win32 fibers | does nothing | disk |

## Why Asyncify

The game runs its own blocking loops (`waitForEventFor`, the message pump, preloads), which a browser can't wait out: the page would freeze and nothing would draw or play. Asyncify unwinds the call stack at a `hostYield`, returns to the browser, and rewinds on the next turn. The cost is code size and speed; the benefit is that the decompiled control flow stays exactly as it is.
