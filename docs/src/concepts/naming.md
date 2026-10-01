# Naming and headers

## Names

- Rename a function, global or struct field **as soon as its purpose is clear**, so its callers read better. Until then, name it after its address: `fn_46be2e`, `g_4a7f58`.
- Functions, variables and fields are camelCase (`isMousePresent`, `currentTimeMs`); types are PascalCase. Names recovered from RTTI or runtime symbols are used exactly (`basePort`, `fileSpec`, `DIB8Port`).
- Prefer honest, specific names over guesses at the originals. A field nothing reads keeps its offset name (`unknown66`).
- A renamed global keeps its address in a marker: `extern long mousePresent; /* @data 0x4a7f58 */`. Two functions that share a name each carry their address (`/* 0x46daca */`).
- `uv run ghidra label` copies `decomp/` names into Ghidra without overwriting names set by hand there; the worklist and report show them.

## Where declarations go

Declarations live in headers, globals by address.

| Declaration | Goes in |
| --- | --- |
| a module's functions, and the globals and types only its code uses | `decomp/<module>.h` |
| shared types, and everything else (engine, OS layer, runtime, globals several modules use) | `decomp/zoombinis.h` |

A module's source and its callers' sources include `zoombinis.h` first and then the module header. `uv run includes [files…]` sets each source's module-header includes to the ones declaring names it uses; run it after adding a call into another module.

This split exists because `match` caches objects by the headers each source includes: changing `zoombinis.h` recompiles every file, changing a module header recompiles only the files including it. Declare each function and global **once** (a test checks).

Keep headers plain (structs, `extern` globals, prototypes): `uv run ghidra label` reads them (`src/zbtools/declarations.py`) to give Ghidra the structs (category `/zoombinis`) and the globals' names and types.

## Struct fields

Fields carry their offsets in comments (`short tag; /* +0x1e: a word for its scene's use */`), which is how most of the structs in `zoombinis.h` were recovered. `Snoid` and `View` are the best-annotated; see [Entities](../codebase/entities.md).
