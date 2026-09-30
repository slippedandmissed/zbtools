# The intro movie: plan

The game plays one QuickTime movie, `Data\Logo025.MOV`: the intro logo, played once from `decomp/town.cpp` (`playMovie` and friends in `decomp/game.cpp`). Its video is in `QkBk`, Broderbund's own codec, whose only decoder is the QuickTime codec component the game installs (`qb32.qtc`). No off-the-shelf decoder handles it (ffmpeg's QuickTime tag table has no `QkBk`). The facts established so far are in `docs/findings.md` ("Movies").

The goal: treat the movie like every other resource. `uv run assets` converts it to a modern format that can be edited, and converts that back to a `.MOV` the unmodified game plays. Converting the disc's movie back must reproduce the disc's file byte for byte, the movie's equivalent of `assets verify`. That means reverse-engineering the codec, its decoder and an encoder, rather than dumping frames through QuickTime.

## Decisions

- **Assets:** the movie lives in `assets/` in its modern form, like the Mohawk resources, and only one copy is committed. `assets` extracts it from the disc, verifies it by rebuilding the `.MOV` and comparing it with the disc's byte for byte, and packs it into `build/`.
- **Windows 98 rebuild:** plays the `.MOV` packed from `assets/` (converted back to `QkBk`), through the QuickTime for Windows installed in the VM. `glue/quicktime.cpp` becomes real glue that calls `QTIM32.DLL` and `CMGR32.DLL` as Apple's did (see "QuickTime for Windows glue" in `docs/findings.md`). The glue isn't decompiled code, so **inline assembly is allowed there** where the calling convention needs it: each stub passes its selector in `bx`.
- **Ports:** don't use `QkBk` or QuickTime at all. `uv run port package` bundles the movie's modern form, and a port-specific replacement for `glue/quicktime.cpp` plays it directly behind the same `qtim_*`/`cmgr_*` functions the game calls. That needs a way for the port to replace a `glue/` file, like `port/decomp/` does for `decomp/`.

## Steps

1. **Reverse-engineer the codec** (`build/zoombi32/qb32.qtc`, 32-bit; `build/disc/QB.DEC` is the same codec for Win16, useful for cross-reference). Record what it establishes in `docs/findings.md`.
   - Find out first whether it contains a compressor as well as the decompressor. The Win16 copy's name suggests decompression only. If there's no compressor, the encoder has to be inferred from the data, as the LZSS and row-packer encoders were.
   - Its exports and strings (below) suggest `QkBk` is more than a plain frame codec: it may composite layers or a Mohawk background. Find out what a frame holds.
   - Use Ghidra: add the DLL to the existing project (never recreate the project: it holds manual work), or use a separate project. Don't commit or publish its disassembly, or the DLL.
2. **QuickTime container:** read and write the `.MOV` exactly as it's laid out (atoms, sample tables, chunk interleaving of the video and `twos` audio), rejecting anything it couldn't reproduce, as `mohawk.py` does for archives. Record what can't be derived (e.g. timestamps in `mvhd`/`tkhd`/`mdhd`, `udta`) in the movie's TOML.
3. **Decoder** (Python, in `src/zbtools/formats/`): `QkBk` frames to images. Check it against what QuickTime shows in the VM.
4. **Encoder:** frames back to `QkBk`, re-creating Broderbund's encoder so the disc's frames come out byte for byte, and producing valid `QkBk` the unmodified codec decodes for frames that were edited. Record in TOML what can't be derived.
5. **`uv run assets`:** extract, verify and pack the movie. **First milestone: the disc's `Logo025.MOV` round-trips exactly.**
6. **Glue for the rebuild:** real QuickTime glue in `glue/quicktime.cpp`, and a way for `uv run vm run --exe` to put the packed movie in the game's `Data` directory. Check the rebuilt game plays the intro in the VM.
7. **Port playback:** bundle the modern form, and add a port glue that plays it (video into the screen port at the centred rectangle, audio through a waveOut-like stream on the same clock, "playing" reported until the end).

## Open questions

- **The modern format.** It must be lossless, editable and round-trip exactly, and the port must be able to play it directly. The obvious choice, matching the other assets, is indexed PNG frames (with their palette) plus the soundtrack as WAV, and a TOML for timing and what can't be derived. A single video file would be easier to edit but harder to keep lossless and in 256 colours. Decide once the codec's output (colour depth, palette, per-frame timing) is known.
- **Only `Logo025.MOV` is used** (by both the Windows 95 and Win16 builds). The disc's three other `LOGO*.MOV` files use the same codec. Round-trip them too once the first one works, so `verify` covers every movie on the disc.
