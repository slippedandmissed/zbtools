# miniwin: Win32 over SDL

miniwin is to this project what DevilutionX's miniwin was to Diablo: a re-implementation of just the part of Win32 the game calls, so the game's source needn't change. The headers are in `port/include/` (`windows.h`, `mmsystem.h`, Borland's `dir.h`/`dos.h`; everything in `namespace miniwin`); the implementation is `port/miniwin/`.

## The model

**One program on one screen**, as the game expects of Windows 95 on a 640×480, 256-colour display.

- **One host thread.** Win32 threads are fibers scheduled cooperatively (`threads.cpp`), as on a uniprocessor: a thread runs until it waits or wakes a thread of higher priority, which then runs at once (the game's file worker, above normal priority, relies on this to finish its call before its caller looks).
- **`service()`** does what Windows does behind the program's back: pumps SDL events into the message queue, fires multimedia timers, mixes audio, and presents the screen. The functions a program waits or polls in call it: the message functions, `Sleep` and the waits, and the time functions. It runs at most about once a millisecond and gives the host a turn every 16 ms (`hostYield`).
- **The screen** is an 8-bit framebuffer shown through a system palette (`screen.cpp`, `palette.cpp`); GDI draws into it or into bitmaps in software.

## Files

| File | Covers |
| --- | --- |
| `miniwin.cpp` | startup, shutdown, `service()` |
| `user.cpp` | USER32: windows, the message queue (posted messages, then `WM_PAINT`, then `WM_TIMER`), timers, hooks, input state, cursors, metrics and system colours. Every retrieved message passes `WH_GETMESSAGE` hooks first |
| `gdi.cpp` | GDI objects and device contexts, bitmaps and DIB sections (all surfaces are 8-bit or monochrome; handles are the objects' addresses) |
| `blit.cpp` | colour matching, raster operations, `BitBlt`, `StretchBlt`, `StretchDIBits`, `PatBlt`, `ScrollDC`, `GetPixel` (on a palette device, GDI works in pixel values) |
| `draw.cpp` | `FillRect`, `FillRgn`, `InvertRect`, `Rectangle`, `Ellipse`, `Polygon`, lines |
| `region.cpp` | regions as bands of rectangles, combined band by band |
| `palette.cpp` | the system palette and logical palettes; realising puts colours where Windows does (static colours matched exactly, the rest in free entries from the first), animating `PC_RESERVED` entries in place |
| `text.cpp` | the game's TrueType font (CornerStone) with stb_truetype: unhinted, no antialiasing (a 256-colour display's text had none), GDI's layout rules, Windows-1252 |
| `screen.cpp` | the SDL window: the framebuffer scaled to the window in whole multiples where they fit, the game's cursor drawn into the image so it scales with it; SDL input to window messages and key state; screenshots |
| `threads.cpp` | Win32 threads and fibers on host fibers; priority-based wake-ups; events and waits |
| `kernel.cpp` | KERNEL32 and ADVAPI32's registry: errors, version (Windows 95), modules, memory, atoms, time |
| `files.cpp` | drives, paths, directories (see below) |
| `mmsystem.cpp` | WINMM: waveOut, midiOut, multimedia timers |
| `midi.cpp` | the General MIDI synthesizer (TinySoundFont) |
| `borland.cpp` | Borland runtime extras: `itoa`, case-blind compares, `getdisk`/`chdir`, `gettime` |
| `internal.h` | what the parts share (and the model, in its header comment) |

## Files and drives

Each drive letter `main()` adds is a host directory: `C:` the installed game and its saves, `D:` the CD (`--drive C=…`, `--cdrom D=…,LABEL,SERIAL`). Windows paths are matched against the host's names **without regard to case**, as Windows does, one component at a time; there is one current directory, which may be on any drive. `uv run port` lays `C:` out from `assets/zoombi32/installed/` (the settings `mohawk.w32`, the font `CORNER.TTF`, the saved-game list `Zoombini.who`, a `Zoombi32.CFG` that points at `D:`) and `D:` from `assets/` packed back into archives.

## Sound

Sound plays **by the wall clock**: `service()` mixes as many frames as time has passed from every open waveOut device (each through an SDL audio stream that converts format and rate) and queues them, so the game sees buffers finish in real time even where the host isn't playing yet (a browser waits for the user's first click). Finished headers are reported through the device's callback as a driver would from its interrupt. midiOut hands channel messages to the synth; the engine sequences MIDI itself, so the synth only sees what a MIDI port would. The game's MIDI map (`MIDIMAP.DAT`, `mohawk.w32`'s "unknown device (port)") treats the device as a General MIDI port: drums on channel 10, channels 6, 7 and 16 muted.

## Unsupported calls

A Win32 function the game calls that miniwin doesn't implement is reported once on the console (`miniwin::unsupported`), so new gaps show up while testing.

## Adding to miniwin

Implement only what the game calls. Match Windows 95's behaviour where the game relies on it: 16-bit `0xFFFF` counts and device ids, a click activating a window. Put code that differs per host in `port/host/`, never in miniwin, and never include miniwin's headers there.
