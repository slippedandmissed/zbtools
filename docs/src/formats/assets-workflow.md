# Working with the assets

The game's resources are kept as **source**, like `decomp/`, in `assets/` (committed). There is only ever **one copy** of each resource, in a modern form, or its original bytes (`.bin`) where no format fits (none of the game's needs that).

```sh
uv run assets extract    # the disc's archives and movies into assets/ (needs the disc; won't overwrite without --force)
uv run assets pack       # assets/ back into archives and movies, in build/assets/, laid out as on the disc
uv run assets verify     # packing reproduces the disc's archives and movies byte for byte
uv run assets frames LOGO025 1 701   # draw frames of a movie as PNGs in build/movie-frames/
uv run movie-check                   # our drawing of every frame against the original codec's
```

`pack` needs only `assets/`, so it works from a fresh clone; `extract` and `verify` need the disc (`uv run extract-game` first). Edit a resource and `pack` builds the archives with the change; `verify` names the resources that differ from the disc's.

## Layout

```text
assets/<ARCHIVE>/archive.toml        the archive's path on the disc, and its resources in stored order
assets/<ARCHIVE>/<TYPE>/<id>.<ext>   one file per resource
assets/movies/<NAME>/                a movie (see Movies)
assets/zoombi32/                     the executable's own resources and the installed game's files
```

| Type | What | Stored as |
| --- | --- | --- |
| `SND` | sounds | WAV (8-bit PCM; a loop as a `smpl` chunk) |
| `tBMP` | images, and banks of them (a sprite's frames) | indexed PNG (a bank's in `<id>/<n>.png`), with `<id>.toml` recording how each was packed |
| `tMID` | music | standard MIDI file |
| `SCRB`, `SCRS` | scripts animating features and Zoombinis | TOML: frames of cels (`[image, x, y]`), events and sounds |
| `SHPL`, `tPAL` | shape lists, palettes | TOML, colours as `#rrggbb` |
| `CURS` | cursors | Windows `.cur` |
| `REGS`, `NODE`, `PATH`, `SYSX` | tables: offsets, walking paths, MIDI messages | TOML |
| `ICON` (`zoombi32/`) | the executable's two icons | indexed PNG, with `resources.toml` naming the icon group |
| `Zoombini.who` (`zoombi32/installed/`) | the saved-game list | TOML (`formats/roster.py`) |
| `DATA/*.MOV` | four QuickTime movies | `frames.toml`, `palettes.toml`, `bitmaps.toml`, `casts/*.png`, `sound.wav`, `movie.toml` |

The installed game's other files the port needs are also there so the port builds without the disc: `mohawk.w32` (the engine's settings file) and `CORNER.TTF` (its font) as they are. The font, Cornerstone, is free for personal use and included on that basis, since this project is non-commercial.

## The rules for a format

A `Format` (`src/zbtools/formats/base.py`) converts one resource type to files named after a stem (`<type>/<id>`) and back:

- **It must round-trip exactly.** `extract` saves a resource, loads it back and keeps it raw if the bytes differ. `assets verify` rebuilds every archive and compares it with the disc: the `match` of the assets. A format that can't give back the original bytes raises `Unconvertible`.
- **It re-creates Broderbund's encoders**, not any valid encoding: Okumura's LZSS, the row packer, the movie's run-length encoder. Where something can't be derived (leftover junk bytes in memory), it is recorded in TOML.
- **It may use the rest of the archive only for viewing** (an image's display palette), never for what `load` needs. An image's pixel values are what the game draws; its PNG palette is only for looking at it. Colours are changed in the palette resources.
- Compressed images are cached in `build/assets-cache/` holding only the encoder's own output (never the disc's bytes, or `verify` would test nothing); compressing all of them takes about two minutes of CPU.

To add a format: implement `save`/`load` in `src/zbtools/formats/`, register it in `formats.FORMATS`, add a test in `tests/test_formats.py` for anything checkable without the disc, and run `extract` and `verify` against your copy.

## Icons

The executable's own resources (its icon) are in `assets/zoombi32/ICON/<id>.png` with `resources.toml`. `extract` writes them, `verify` checks them against `zoombi32.exe`'s, and `uv run build` compiles them into the rebuilt executable. Each icon must stay 16 colours (plus transparency), as Windows 95 icons are; the port reuses them for the page's favicon.
