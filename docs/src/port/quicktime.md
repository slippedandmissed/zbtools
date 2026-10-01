# QuickTime in the port

The original game plays one movie, `Data\Logo025.MOV`, through QuickTime for Windows 2.x, whose video decoder for the codec `QkBk` is a DLL (`qb32.qtc`) that only that QuickTime can run. The port cannot use QuickTime, so it plays a converted form of the movie itself.

## The scene file

`QkBk` is not a pixel codec but a *scene compositor*: each frame says which sprites (from a cast of bitmaps) sit where, in a palette, on a background colour ([Movies](../formats/movies.md)). So "decoding" is drawing sprites, and the port needs no decoder. `uv run port package` turns `assets/movies/LOGO025/` into a flat, little-endian **scene file** `DATA/LOGO025.SCN` (`src/zbtools/formats/scene.py`):

```text
"ZBSC", u32 version (1), u16 width, u16 height, u32 ms per frame, u32 frames,
u32 sound rate, u32 sound samples, u32 cast items
frames:     u8 background colour, u8 sprites, u16 palette (a cast item id),
            then each sprite: u16 cast item id, i16 x, i16 y   (slot order)
cast items: u16 id, u8 type, u8 0, then
              palette (1): 256 colours of u8 r, g, b
              bitmap  (2): u16 width, u16 height, u32 size, rows of {u16 length, RLE data}
sound:      8-bit unsigned mono samples, with the movie's edit list applied (one track from time 0)
```

Only the movie's look and sound are kept: what the codec does with the rest of a frame's fields doesn't change what it draws. The player's drawing is checked against the *original codec running under emulation*: `uv run movie-check` runs `qb32.qtc` in Unicorn on the movies built from `assets/` and compares every frame and palette.

## The player (`port/glue/quicktime.cpp`)

It replaces `glue/quicktime.cpp` and answers the calls the game makes of the QuickTime SDK glue, by selector (`qtim_…`, `cmgr_…`; names in `src/zbtools/quicktime.py`):

| Game's need | Selectors |
| --- | --- |
| open the file and make a movie of it | `qtim_2c`, `qtim_2a`, `qtim_02` |
| a controller over a window (`cmgr_0d` to reuse one), place it | `qtim_38`, `cmgr_0d`, `cmgr_0e` |
| play (`cmgr_01` with action 8), let it run (`cmgr_09` on each pass of the loop), ask whether it still plays (`cmgr_05`, flag `0x40`) | |
| stop and dispose | `qtim_31`, `qtim_07`, `qtim_37`, `qtim_0c` |

It answers `QTInitialize` with version `0x2300` (the least the game accepts). The sound is one track from time 0, played through waveOut; the video follows the clock (`frameMilliseconds`, 10 fps), drawing the sprites of the current frame with its palette. The `.MOV` files themselves are left out of the site.

## In the rebuild (VM) instead

`glue/quicktime.cpp` (not the port's) loads the real QuickTime and forwards each call through raw-byte stubs that put the selector in `bx`, as Apple's glue does; the rebuilt game plays the `.MOV` packed from `assets/` (byte for byte the disc's). See [QuickTime glue](../formats/quicktime-glue.md).
