# Matching

`uv run match` is the project's definition of "correct". It compiles each file in `decomp/` with the original compiler and compares every marked function with the original, byte for byte.

## What it does

1. Finds the `/* @zoombi32 0x… */` markers in each source.
2. Compiles the file with **Borland C++ 4.5 under Wine**, with the game's usual options `-p -k-` unless the file says otherwise (`/* @flags … */`, `/* @release 5.02 */`; `--release` and `--flags` override every file's, to experiment).
3. Reads the object (`omf.py` is a minimal OMF reader) and the original (`exe.py` wraps pefile and capstone) and compares each marked function's bytes, **masking fields the linker fills in** (addresses and call targets). A call to another marked function in the same file must go to that function's address in the game. Every absolute address into the function itself (switch tables) must point to the same offset as the original's.
4. Prints side-by-side disassembly for mismatches and exits 1 if anything differs.

Compiled objects are cached in `build/match-cache/`, keyed on the source, the *local headers it includes*, its options and the release; `--no-cache` recompiles everything.

## Statuses

| Status | Meaning |
| --- | --- |
| matched | identical bytes, recorded in `decomp/matching.txt` |
| near-miss | compiles, differs (usually registers); a comment says what |
| functional | `@zoombi32-functional`: complete and portable, not byte-exact by design |
| implicit | `@zoombi32-implicit`: a compiler-generated function, measured like any other |
| todo | not yet decompiled |
| library | Borland runtime or QuickTime glue, not decompiled |

`decomp/matching.txt` is written by `uv run match --update`. `match` fails if a recorded match regresses and reports new ones.

## What it can't see, and what covers for it

`match` masks addresses, so a function can match while reading the wrong global. `uv run match-data` checks that every data reference points where the original's does. Hand-written assembly can't be reproduced from portable C++, so those are functional. Near-misses with more than allocation differences are reviewed with `near-misses`.

## Where the numbers stand

About 2,100 functions: 1,915 matched, 176 near-misses, 37 functional. The remaining near-misses are mostly engine functions whose register choices no source form reproduces; see [The compiler](compiler.md) and [Near-misses](near-misses.md).

## Hints, in one paragraph

Declaration order and block scope decide register assignment. `volatile` pins a local to the stack. Writing a global directly lets BCC32 hoist its address into a register. Comparison operand order follows the source. Write `switch` cases in the original's code order. The [field guide](bcc32-quirks.md) is the long version.
