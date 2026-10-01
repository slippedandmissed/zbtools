# Source modules

TLINK32 lays each object file's code out contiguously, in link order, and pads it with zeros to a 4-byte boundary. Where a function is followed by that padding, a module ends. About one boundary in four is invisible (the code happened to be aligned), but the data follows the same order, so a boundary can show there instead: each module's initialised data follows the previous module's, with string literals last.

`decomp/modules.toml` is the curated map: 42 game modules, the Mohawk OS layer's six, and the engine's 149. `uv run modules` checks it against the padding and prints the strings and imports of each range, which is how modules are named: by archive (`bridge.mhk`, `Ferry.MHK`, `Tunnels.MHK`, `Net.MHK`), by characters (Pizza Pass's trolls), or by messages (`Zoombini.who`, `Zoombi32.CFG`, `e2AllocHandle`). Unknown ones are named by address (`module_4124a4`) with the evidence in `note`.

Each module's functions go in `decomp/<module>.cpp`, in address order (string literal pools make that order matter), with a `decomp/<module>.h` of its own declarations. The module map also drives `uv run worklist --module` and the report's per-module progress.

Boundaries the code doesn't show can be found in the data: `isle` was split from `net`, and `hotel` from `lilly`, because their globals and initialised data start in different places even though no padding separates their code.

The full list, with what each module does, is in [Modules and scenes](../codebase/modules-and-scenes.md).
