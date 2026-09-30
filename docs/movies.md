# The intro movie: plan

The game plays one QuickTime movie, `Data\Logo025.MOV`: the intro logo, played once from `decomp/town.cpp` (`playMovie` and friends in `decomp/game.cpp`). Its video is in `QkBk`, Broderbund's own codec, whose only decoder is the QuickTime codec component the game installs (`qb32.qtc`). No off-the-shelf decoder handles it (ffmpeg's QuickTime tag table has no `QkBk`). The facts established so far are in `docs/findings.md` ("Movies").

The goal: treat the movie like every other resource. `uv run assets` converts it to a modern format that can be edited, and converts that back to a `.MOV` the unmodified game plays. Converting the disc's movie back must reproduce the disc's file byte for byte, the movie's equivalent of `assets verify`. That means reverse-engineering the codec, its decoder and an encoder, rather than dumping frames through QuickTime.

## Status

Steps 1 to 6 are done: `uv run assets verify` rebuilds all four of the disc's movies from `assets/movies/` byte for byte. `QkBk` turned out to be a scene compositor (sprites from a cast of bitmaps, placed per frame), so the modern form is the scene: the frames' sprite tables as TOML, the bitmaps as indexed PNGs, the palettes as TOML, the sound as WAV, and the container's facts in a manifest (`formats/qkbk.py`, `formats/mov.py`, `movies.py`). Our drawing of every frame is checked against the original codec's (`uv run movie-check` runs `qb32.qtc` under emulation). What remains: step 7.

## Decisions

- **Assets:** the movie lives in `assets/` in its modern form, like the Mohawk resources, and only one copy is committed. `assets` extracts it from the disc, verifies it by rebuilding the `.MOV` and comparing it with the disc's byte for byte, and packs it into `build/`.
- **Windows 98 rebuild:** plays the `.MOV` packed from `assets/` (converted back to `QkBk`), through the QuickTime for Windows installed in the VM. `glue/quicktime.cpp` becomes real glue that calls `QTIM32.DLL` and `CMGR32.DLL` as Apple's did (see "QuickTime for Windows glue" in `docs/findings.md`). The glue isn't decompiled code, so **inline assembly is allowed there** where the calling convention needs it: each stub passes its selector in `bx`.
- **Ports:** don't use `QkBk` or QuickTime at all. `uv run port package` bundles the movie's modern form, and a port-specific replacement for `glue/quicktime.cpp` plays it directly behind the same `qtim_*`/`cmgr_*` functions the game calls. That needs a way for the port to replace a `glue/` file, like `port/decomp/` does for `decomp/`.

## Steps

1. **Reverse-engineer the codec (done)** (`build/zoombi32/qb32.qtc`, 32-bit; `build/disc/QB.DEC` is the same codec for Win16, useful for cross-reference). Record what it establishes in `docs/findings.md`.
   - Find out first whether it contains a compressor as well as the decompressor. The Win16 copy's name suggests decompression only. If there's no compressor, the encoder has to be inferred from the data, as the LZSS and row-packer encoders were.
   - Its exports and strings (below) suggest `QkBk` is more than a plain frame codec: it may composite layers or a Mohawk background. Find out what a frame holds.
   - Use Ghidra: add the DLL to the existing project (never recreate the project: it holds manual work), or use a separate project. Don't commit or publish its disassembly, or the DLL.
2. **QuickTime container (done, `formats/mov.py`):** read and write the `.MOV` exactly as it's laid out (atoms, sample tables, chunk interleaving of the video and `twos` audio), rejecting anything it couldn't reproduce, as `mohawk.py` does for archives. Record what can't be derived (e.g. timestamps in `mvhd`/`tkhd`/`mdhd`, `udta`) in the movie's TOML.
3. **Decoder (done)** (Python, in `src/zbtools/formats/`): `QkBk` frames to images (`qkbk.render_frame`, `uv run assets frames`): each frame drawn from scratch, sprites in slot order on the background colour, colour 0 transparent. The codec draws over the last frame and redraws only the rect the frame says changed; since that rect is the union of the changed sprites' old and new rects, the two agree. Checked against the original codec itself, run under x86 emulation (Unicorn, `qb32.py`), not QuickTime in the VM: `uv run movie-check` decodes all four movies and compares every frame and palette.
4. **Encoder (done):** the row encoder is inferred and exact on all 178,380 rows; frames are rebuilt from their fields, working out what can be (sprite sizes, each frame's bounds and changed rect), with the rest in TOML. Edited movies are valid `QkBk`: `movie-check` decodes them with the unmodified codec.
5. **`uv run assets` (done):** extract, verify, pack, and `frames` to look. **First milestone: the disc's `Logo025.MOV` round-trips exactly**, and so do the other three.
6. **Glue for the rebuild (done):** real QuickTime glue in `glue/quicktime.cpp` (the SDK's loader, re-created from the same library in `qb32.qtc`, and selector stubs written as raw bytes). The rebuilt game plays the intro through the VM's QuickTime, stalling on the same frame the original does (the VM's emulated sound or timing, not the glue). The game reads the movie from the CD (`D:\Data`, `INSTALLFROMDIR`), and the packed movie is byte for byte the disc's, so there's no need to deliver it another way.
7. **Port playback:** bundle the modern form, and add a port glue that plays it (video into the screen port at the centred rectangle, audio through a waveOut-like stream on the same clock, "playing" reported until the end).

## Open questions

- **The modern format: decided.** The frames aren't pictures, so the scene is stored: `frames.toml` (1,392 frames, 780 KB), `palettes.toml`, `casts/<id>.png`, `sound.wav` (a movie is 6.5 to 8 MB in all, 29 MB for the four). Editing a bitmap or a sprite's position is editing a PNG or a line of TOML. Changing how many frames or sounds there are means updating `movie.toml`'s `chunks` (the interleaving) and `sync`; `uv run assets pack` says when they don't add up.
- **Only `Logo025.MOV` is used** (by both the Windows 95 and Win16 builds); the other three round-trip too, so `verify` covers every movie on the disc.
- **For the port (step 7):** the modern form is a scene, so the port needs a small compositor (sprites over a background, in a palette, at 10 frames a second) rather than a video decoder, and `frames.toml` is big to parse at run time: have `port package` pack the scenes to a compact binary for it.
