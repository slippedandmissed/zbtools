# The compiler: Borland C++ 4.5

The game and its engine were built with **Borland C++ 4.5** (BCC32 for the code, TLINK32 for the link). 4.52 generates identical code for this game, so the tools use 4.5 (`data/Borland C++ 4.5.iso`; a 4.52 or 5.02 CD can be added for comparison).

## How we know

| Evidence | Implies |
| --- | --- |
| The 16-bit build's NE header says linker version 6.1 | TLINK 7.0a, which shipped only with BC++ 4.5 and 4.52 |
| Both builds contain `Borland C++ - Copyright 1994 Borland Intl.` | a 4.x runtime (5.0's says 1996) |
| C++ exception handling and RTTI (`**BCCxh1`, `Bad_typeid`, `xalloc`) | BC++ 4.0 or later |
| The PE has linker version 2.25 and sections named `CODE`/`DATA` | Borland's TLINK32 |
| Built January 1996 | before BC++ 5.0 |

4.5 and 4.52 differ only in the Pentium FDIV workaround (`-fp`, 4.52 only), none of whose support code is in the game; their runtime libraries are the same code module for module. The proof that matters is empirical: 1,915 functions compile to the original bytes with 4.5 (see [Matching](matching.md)).

The Mohawk engine (`0x4764bc` onwards) is the one place where 4.5 is not the whole story: the engine's code often keeps short-lived locals in saved registers where BCC32 4.5 keeps them in `eax` or on the stack, and it uses Windows 95's `DEVMODE` (0x94 bytes) where 4.5's headers have the older 0x7c-byte one. So the engine was built with 4.5 or something very close to it, probably against newer Windows headers. 5.02 is *not* it (compiling the engine with 5.02 matches far fewer functions: it builds frames for leaf functions and picks other scratch registers). Those functions remain [near-misses](near-misses.md).

## The game is C++

The Borland C++ exception-handling and RTTI runtime is linked in, and functions with local objects that have destructors call `__InitExceptBlock`, so the decompiled code is C++, not C. What BCC32 4.5 does with it:

- Plain functions are mangled with their argument types only (`setCurrentMap(long)` is `@setCurrentMap$ql`, even when declared `__stdcall`); global variables keep C names (`_g_4a7f58`). `__pascal` functions have the whole mangled name upper-cased (`@FN_4115F5$QSL`); `demangle.py` handles both.
- Methods receive `this` as a hidden first *stack* argument (`[ebp+8]`), not in a register as with Microsoft's compilers.
- Destructors take a hidden second argument whose bit 0 means "also free the memory".

## Options: `-p -k-`

BCC32 reproduces the game's code with:

- **`-p`**: the Pascal calling convention is the default. Parameters are pushed left to right and the callee pops them (`ret N`). The first parameter in the source is at the *highest* stack offset. A decompiled game function therefore has no calling-convention keyword and lists its parameters in Pascal order; **Ghidra shows them reversed** (its `param_1` is the last source parameter), because it has no `__pascal` and `ghidra label` marks these functions `__stdcall`.
- **`-k-`**: no standard stack frame unless one is needed. It only shows in functions that make calls but have no parameters and no stack locals.
- Everything else at BCC32's defaults: no optimisation (`-Od`), register variables (`-r`), byte alignment. What looks like optimisation in the original (registers for locals, rotated loops) is BCC32's ordinary code generation. `-O1`/`-O2` and `-r-` break functions that otherwise match.

Options were set per module, as an IDE project allows:

| Code | Options | Why |
| --- | --- | --- |
| The game (`0x41008c`-`0x46cca0`) | `-p -k-` | the default for every file |
| The Mohawk OS layer (`0x46d754`-`0x46f7a4`, `os_*.cpp`) | `-p` and `-x-` | built with frames; no exception frames |
| The Mohawk engine (`0x4764bc`-) | `-p -x-` per module | Pascal-order like the Mac Toolbox calls it imitates, but its C++ class methods are `__cdecl` |

A file overrides the defaults with `/* @flags ... */` and the release with `/* @release 5.02 */`. The engine's classes' methods don't pop their arguments, so they are declared `__cdecl` module by module; plain QuickDraw-style helpers (`emptyRect`, `offsetRect`) use the game's `-p -k-`.

See [Matching BCC32](bcc32-quirks.md) for what the source has to look like to reproduce the original's code.
