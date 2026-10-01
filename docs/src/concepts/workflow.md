# The decompilation workflow

## Choosing something to decompile

```sh
uv run worklist                    # functions ready to decompile, smallest first
uv run worklist --region engine
uv run worklist --module bridge    # one source module
uv run report --open               # statistics per region and module
```

A function is *ready* when everything it calls directly is done (matched, identified runtime code, or outside the region you're working on), so you work leaf-first and the callee names are already known. Regions are `startup`, `game`, `runtime`, `engine` and `quicktime`, derived from the recovered symbols. The report shows each function's recorded status and what is measured now, with a badge where they disagree. It contains the game's disassembly: keep it local.

## Writing the function

1. Read Ghidra's output: `uv run ghidra decompile 0x46be2e`. Remember its parameters are reversed for the game's Pascal functions ([Ghidra notes](../reference/ghidra-notes.md)).
2. Write it in `decomp/<module>.cpp`, in address order, preceded by `/* @zoombi32 0x0046be2e */`. Methods are marked the same way (`void Widget::set(long v)`).
3. Declare it in the module's header (`decomp/<module>.h`) and globals by address (`extern long mousePresent; /* @data 0x4a7f58 */`). Shared types and anything several modules use go in `decomp/zoombinis.h`; see [Naming and headers](naming.md).
4. Run `uv run match decomp/<module>.cpp`. A mismatch is shown as side-by-side disassembly.
5. Iterate with [the field guide](bcc32-quirks.md) until it matches, or note in a comment what still differs ("Not exact: register allocation…").
6. Name things as soon as their purpose is clear, and run `uv run ghidra label` so Ghidra shows the names.
7. `uv run match --update` records matches in `decomp/matching.txt` (the pre-commit hook does this when `decomp/` changes; if it rewrites the file, add it and commit again). From then on a regression fails `uv run match`.

## Rules for the code

- **Portable C++.** No inline assembly, `__emit__`, `_EAX`/`_EBP`, or reliance on the x86 stack layout. Calling the Windows API is fine; the port supplies it.
- **Typed pointers.** Keep pointers as pointers where the type is known, not `long`, for an eventual 64-bit build.
- **Don't replace what standard C++ reserves** (the global placement `operator new(size_t, void *)`); the game's zeroing placement new is `audioObj`'s.
- **Where only machine code reproduces the original**, write the portable equivalent, mark it `/* @zoombi32-functional 0x… */` and say what the original does. If there is none, leave a documented stub.
- **Compiler-generated functions** (an implicit destructor) have no definition to mark: name them in a marker of their own in the file whose object holds them, `/* @zoombi32-implicit 0x0048a6f9 DIB8Port::~DIB8Port */`. A global object's construction and destruction calls are `<startup>` and `<exit>`.
- **A function is only done when it matches** (or is functional). Match status is measured, never declared.

## Checking the data

`uv run match-data` places each module's data in the original's `DATA` section and compares it; `uv run define-data` defines declared-but-undefined globals with the original's initial values. See [Data layout](data-layout.md).

## Reviewing what's left

`uv run near-misses` classifies functions that don't match ([Near-misses](near-misses.md)). Review anything with more than allocation differences before trusting it.

## Recording findings

Confirmed facts about formats, flags or file meanings belong in this book (with the evidence: addresses, strings, experiments) or in the source comment of the code they explain. Rename freely as understanding improves; the marker holds the address, so the tools don't care.
