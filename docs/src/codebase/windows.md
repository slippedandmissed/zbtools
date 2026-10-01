# How the game talks to Windows

The decompiled code calls the Win32 API directly, in a small number of places. This chapter lists them; they are also exactly what `miniwin` has to implement for the [port](../port/miniwin.md).

## The window (`platform.cpp`)

- **One window.** `createMainWindow` registers a window class *named after the program's own file name* (that is also how a second instance finds the first with `FindWindow`) and creates a borderless popup covering the whole screen, shown maximised. The game's 640×480 area is centred on larger screens (`placeGamePort`, `alignRect`; `gameRect`/`shownGameRect`).
- **`mainWindowProc` (`0x45605e`)** handles: `WM_CHAR`/`WM_KEYDOWN` and `WM_L/R/MBUTTONDOWN` (turned into game events), `WM_SETCURSOR`, `WM_ACTIVATEAPP`/`WM_NCACTIVATE`/`WM_SETFOCUS`/`WM_KILLFOCUS` (`activateApp`), `WM_SYSCOMMAND` (screen savers and minimising are blocked or handled), `WM_PALETTECHANGED`/`WM_QUERYNEWPALETTE` (`realizeFullScreenPalette`), `WM_PAINT` (through `paintHook`), and `WM_CLOSE`/`WM_DESTROY`/`WM_QUIT`/`WM_ENDSESSION` (a fatal "error" that quits). While a movie is showing, QuickTime's component manager sees each message first. Messages are also logged to a ring of 1,024 (`logMessage`) and dumped to `msgNNN.txt` on request (`dumpMessages`).
- **The message pump** is the game's own: `PeekMessage`/`GetMessage` in `pumpMessage`, `handleNextMessage`, `waitWhilePaused`, called from the main loop and from every blocking wait. There is no `GetMessage` loop in `WinMain`.
- **Activation.** Deactivating (Alt-Tab) shuts down the screen port and the sound driver, and in some setups minimises the window; reactivating rebuilds them (`activateApp`, `gameActivated`), asking to retry while the sound device is missing.
- **Display modes.** `checkDisplayMode` asks for 640×480 at 256 colours (falling back to 512×384, or another depth) and `setDisplayMode` switches. The game requires a palettised mode. `canUseDisplayMode`/`getDisplayMode` in the engine use Windows 95's 0x94-byte `DEVMODE`.

## Graphics (GDI)

All drawing goes through the engine's ports (see [Graphics](engine-graphics.md)), which wrap GDI:

| Port | Backed by |
| --- | --- |
| `windowPort`, `displayPort` | a window or screen DC |
| `memoryPort` | a compatible bitmap |
| `DIBPort`, `DIB8Port` | a DIB section; `DIB8Port` also writes its bits directly (assembly blitters, ported as functional C++) |

The game draws every scene off screen into **`workPort`** and copies changed regions to **`screenPort`**; `e2MapSave` saves screen areas for restoring. **Palettes**: the game builds its own 256-colour logical palette so that realising it is the identity, animates `PC_RESERVED` entries in place for fades and colour cycling (`fadeInViews`, `cycleColors`), and brightens colours made for the Mac's lighter gamma (`brightenPalette`: `c + 31 - c/8`). A 20-colour static-colours question (does the system keep them?) decides whether 236 or 255 colours are realised.

## Sound and music (WINMM, DirectSound)

- **Wave sounds** go through WaveMix-style objects: either straight to `waveOut` (`wmxWaveOut`) or mixed in software (`wmxMixer`) into one output buffer per device, written through `waveOut` (`wavebufWO`, a polling thread) or a looping one-second **DirectSound** buffer (`wavebufDS`, when `[WaveMix] fEnableDirectSound` is on and `DSOUND.DLL` loads). Long sounds stream from their file in 4 KB buffers.
- **Music** is MIDI. The *engine* sequences it itself (`midisound.cpp`: tempo, loop and `Setup end` markers, program changes) and sends channel messages to `midiOut` as they fall due, through a MIDI map (`MIDIMAP.DAT`, controller resets for all 16 channels) that adapts to the device. The device list comes from the registry's Sound Mapper (`Software\Microsoft\Multimedia\Sound Mapper`) or `MOHAWK.INI`'s `[Audio]` section.
- **Timers** are multimedia timers (`timeSetEvent` at the finest resolution, `timeBeginPeriod`); their callbacks only post to a `DeferLock`, so procedures run on a thread that holds or releases it. `timeGetTime` is the clock.

## Files and the CD

`fileSpec`/`files` model Mac `FSSpec`s over Win32. Reads that could block (the CD) go through an `asyncAPI` family with a worker thread above normal priority; the port runs that on its cooperative scheduler. The game finds its data through `Zoombi32.CFG` and asks for the CD if `Data\Zoombini.mhk` is missing. `Drive::setLocked` locks and unlocks removable media through VWIN32's DOS IOCTL `440Dh`/`0848h`, whose 2-byte parameter block has to be a 2-byte local (a 1-byte one overwrote the saved frame pointer). `MOHAWK.W32` (installed beside the program; `mohawk.w32` in `assets/zoombi32/installed/`) is the INI that configures the engine.

## Memory

The engine's Mac-style memory manager sits on `GlobalAlloc`/`GlobalLock` (moveable blocks whose first word Win32 makes point at the memory, which the engine relies on). Startup checks virtual memory and, on anything newer than Windows 3.11, total physical memory (6 MB, via `GlobalMemoryStatus`). The game installs a grow-zone procedure that notes an out-of-memory condition.

## Threads and fibers

The Mohawk OS layer runs its own *cooperative* threads on the one Win32 thread, each on a Win32 **fiber** it finds in KERNEL32 at run time (Borland C++ 4.5's headers predate them; Windows 98 and NT 4 have them, Windows 95 doesn't). Its scheduler runs on calls into the layer and every 20 ms, picks the highest-priority runnable thread, and pumps messages while idle. Real threads exist only where Windows forces them: waveOut callbacks, multimedia timers, the preload thread. See [OS layer](engine-os-audio.md).

## Run-time loaded libraries

| DLL | Loaded by | Used for |
| --- | --- | --- |
| `DSOUND.DLL` | `LoadLibrary` in `wavebufDS` | DirectSound output |
| `QTIM32.DLL`, `CMGR32.DLL` | the QuickTime glue | the intro movie (QuickTime for Windows 2.x) |
| `VERSION.DLL` | the QuickTime glue | reading QuickTime's file version |
| KERNEL32 fiber functions | `GetProcAddress` | the engine's contexts |

The movie's video codec `qb32.qtc` is loaded by QuickTime, not by the game.

## Global atom

The atom `Zoombini` is the single-instance lock: added at startup, deleted once graphics are up and at exit.
