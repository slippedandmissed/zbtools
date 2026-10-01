# Ghidra: its limits, and what `label` fixes

`uv run ghidra setup` imports and analyses the binary; `uv run ghidra label` then repairs and names what auto-analysis gets wrong. You only need these notes if you work in the Ghidra project or change `label`.

## What auto-analysis misses

- **Functions reached only through pointers**: `WinMain`, window procedures, callbacks in tables, frameless (`-k-`) functions that start modules. `label` creates functions at relocated pointers from data (and from instructions outside a memory operand), at module starts, after a function that begins `push ebp; mov ebp, esp`, and at direct call targets not yet in a function. It also disassembles them: `createFunction` alone makes a one-byte function.
- **Relocated pointers in `CODE` aren't always code**: switch tables, byte index tables and RTTI descriptors live there too; operands like `[eax*4 + table]` don't count.
- **Switch tables cut functions short**: BCC32's `jmp [reg*4 + table]` with the table inside the function made Ghidra end the function at the jump. `label` re-decompiles any function with an unresolved computed jump, lets the decompiler recover the table, and recomputes the body.
- **Fragments**: Ghidra sometimes splits a function after `push ebp; mov ebp, esp; add esp, -n`. `label` merges a function that nothing refers to into the previous one when that one falls through.
- **An `int3` ends a function**: `debugBreak` (`0x46db83`) is `int3` mid-function; `label` makes breakpoints fall through.
- A few inner loops of assembly routines (`0x4899ed`, `0x48cfc9`, `0x48d03f`) are reached through a register and look like functions; they're left as they are.

## Calling conventions

Ghidra's x86 has no `__pascal`. `label` marks every function that pops its own arguments `__stdcall` (1,365 of them), which gets the stack right but **numbers parameters in reverse**. Read `param_1` as the last parameter of the source.

## What `label` applies

Runtime library names (`runtime-symbols`), C++ classes, vtables, constructors and destructors (`classes`), QuickTime stub names, the names, types and globals declared in `decomp/` (headers' structs go in the category `/zoombinis`), calling conventions, and the repairs above. It never overwrites a name or type set by hand (source `USER_DEFINED`), so you can annotate freely; rerun it after renaming in `decomp/`.

The project lives in `build/ghidra/project/` and may hold your own work: never delete or recreate it without being asked (`uv run ghidra setup --force` and `clean ghidra-project` do).

## The movie codec

`uv run ghidra codec` imports `qb32.qtc` into its own project and writes its decompilation to `build/ghidra/qb32.c`. Ghidra doesn't find the codec's handlers because they take their selector in `bx`; the command creates them. The file is the original's code: never commit it.
