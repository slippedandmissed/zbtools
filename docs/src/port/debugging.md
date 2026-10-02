# Debugging the port

The **headless** build is the main tool: the same WebAssembly under Node, with no screen or sound, reading the drives' directories directly, so a whole run can be scripted and its output examined.

```sh
uv run port run headless_wasm --seconds 30 --screenshot build/port/shot.bmp
uv run port run headless_wasm --seconds 40 --click 12000:320,240 --click 14000:600,420 --screenshot build/port/shot.bmp
uv run port run headless_wasm --seconds 20 --record build/port/audio.wav
```

| Option | Does |
| --- | --- |
| `--seconds N` | quit after N seconds (`--run-for` ms for `zoombinis`) |
| `--screenshot F.bmp` | write the 640×480 screen to a BMP about once a second (and at exit) |
| `--click MS:X,Y` | click at a point of the screen MS ms after starting; `MS:X,Y:press`, `:move`, `:release` make a drag |
| `--record F.wav` | write everything the game plays to a WAV file |
| `--soundfont F.sf2` | the General MIDI SoundFont (without one the music is silent) |

The `zoombinis` executable itself takes `--drive C=<dir>`, `--cdrom D=<dir>[,label[,serial]]`, `--program <Windows path>` and `-- <game command line>` (for example `-- d` for the game's debug switch). `uv run port run` fills these in.

`uv run gameplay` plays scripted flows and compares their screenshots with checked-in baselines ([Instrumented gameplay tests](gameplay-tests.md)).

To get into a state quickly (a scene, a level, a party), the build includes [debug tools](debug-tools.md): `--cmd "scene 9; party 8"`, `--script`, `?cmd=` in the URL.

## What to look at when something is wrong

- **The console** prints what miniwin doesn't support (`unsupported`, once each), no-display notices, and the game's own error messages (`showError`). In a browser they're in the developer console and `window.zbLog`.
- **A crash or hang in the game's code:** the engine's fibers make stacks unusual; `hostTraceStack` prints the call stack where the host can.
- **A wrong picture:** take a headless screenshot and compare with the VM's (`uv run vm screenshot`) at the same moment. Differences in colours usually mean palette realisation (`palette.cpp`), in text GDI layout (`text.cpp`), in pixels a raster operation (`blit.cpp`).
- **Timing:** miniwin's clock is the wall clock; scripted clicks use the same clock, so slow builds need more `--seconds`.
- **Decompiled code that doesn't match the original** can show up in the port as a behaviour difference; check `uv run match` and `uv run near-misses` for the function.

## Tests

`tests/test_port.py` covers the site packer (splitting, grouping, drive layout). Everything that needs the Emscripten SDK is checked by building (`uv run port build web`, which CI runs on every pull request) and by headless runs.
