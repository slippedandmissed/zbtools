# Mohawk archives

A Mohawk archive (`MHWK` magic, `RSRC` directory) is the container for every resource the game loads. Its on-disk structure (header, data, type and resource tables, file table) is described under [the resource manager](../codebase/engine-memory-resources.md#the-resource-manager), the reader in `src/zbtools/mohawk.py` reads and writes it exactly as the game's archives are laid out (rejecting anything it couldn't reproduce), and ScummVM's `engines/mohawk/` is the reference for the container (it names the Zoombinis types but doesn't implement the game). This chapter is about what the game's archives contain.

## The game's archives

The disc's Mohawk archives are the 20 `DATA/*.MHK` files and `MIDIMAP.DAT` (in the disc's root, installed next to the program; it holds two `SYSX` resources, MIDI channel messages the MIDI map sends a device, by ID from `[MidiMap.TargetDeviceInfo]`: despite the name, not system-exclusive, but controller resets for all 16 channels). `BROEMIDI.TMI` starts `MHWK` too, but it's a Mohawk MIDI file (`MHWK`/`MIDI`), not an archive. All 21 archives are laid out the same way, so each is determined by its resources' types, IDs, data, flags and order (`uv run assets verify` rebuilds them from just that, byte for byte):

- The header says version 0x100 and `compacted` 1 (the file may hold unused space). The unused space is 8 bytes, `00 04 00 00 00 00 00 00`, between the header and the first resource's data; compacting the file (`closeResourceFile` with `compact`) would drop them.
- The data is stored in file-table order, with no gaps. The directory follows it, then the file table, which ends the file.
- The directory's type table is sorted by type (by its bytes, so `\0SND` comes first and the lower-case `t` types last). The types' resource tables follow in the order each type first occurs in the data, each followed by its (empty) name table. No resource has a name, so the names start where the directory ends.
- The only file-table flag set on disk is `RESOURCE_PURGEABLE` (0x80), on all 18 `tMID` resources in `MIDIMPC.MHK`.

What they hold (resource counts across the 20 `.MHK` archives):

| Type | Count | Bytes | Contents |
| --- | --- | --- | --- |
| `\0SND` | 1,333 | 43.8 MB | Sounds ([details](sound-images-scripts.md)) |
| `tBMP` | 175 | 20.5 MB | Images and banks of images ([details](sound-images-scripts.md)) |
| `SCRB` | 1,782 | 345 KB | Feature scripts: the scripts that animate views ([details](sound-images-scripts.md)) |
| `SCRS` | 719 | 499 KB | Zoombini ("snoid") scripts: the same, with the way the Zoombini faces |
| `REGS` | 79 | 29 KB | Tables of big-endian words, meaning what their users make of them (shapes' offsets, the maze's and Lilly's tables) |
| `SHPL` | 41 | 25 KB | Shape lists: the first shape's ID (a `tBMP`) and the count, then a palette (`e2memory.cpp`) |
| `tMID` | 18 | 77 KB | Music ([details](sound-images-scripts.md)) |
| `NODE`, `PATH` | 9 each | 0.9 KB | The graph of the paths Zoombinis walk: a count and points `{x, y}`; a count and 24-byte lists of nodes (0: none) |
| `tPAL` | 6 | 6 KB | Palettes (`MAZE2.MHK` only): u16 first colour and count, then `PALETTEENTRY`s. Every colour of every real palette has flags 1 (`PC_RESERVED`); 13 one-colour placeholder palettes in `SHPL`s have 0. |
| `CURS` | 5 | 340 B | Mac cursors: 16x16 image and mask, then the hot spot (`ZOOMBINI.MHK`) |

ScummVM's `engines/mohawk/resource.h` names the Zoombinis types (`SCRB` "Feature Script", `SCRS` "Snoid Script", `NODE` "Walk Node", `PATH` "Walk Path", `SHPL` "Shape List"), and detects the game but doesn't implement it.

The movies (`DATA/LOGO*.MOV`) are QuickTime files outside the archives: video in `QkBk` (the codec installed as `qb32.qtc`), sound in `twos` (PCM). See [Movies](movies.md).
