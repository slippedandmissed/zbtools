# Findings

Confirmed facts about the game and its installer, with where they came from.

## Zoombi32.CFG (install locations)

`Zoombi32.CFG` ships in `ZBARCHIV.Z` as an empty file. The game treats it as an INI file and reads two keys from its `[INSTALL]` section at startup, showing "unable to read file Zoombi32.CFG" if they're missing:

| Key | Meaning | Example |
| --- | --- | --- |
| `INSTALLFROMDIR` | Root of the game CD. The game appends `Data\` to find the `.MHK` archives, so it needs a trailing backslash. | `D:\` |
| `INSTALLTODIR` | Install directory. | `C:\Program Files\Zoombi32\` |

The original InstallShield script (`SETUP.INS`, which references `InstallFromDir` and `InstallToDir`) writes these values. `vm install-game` ships a filled-in copy instead (`src/zbtools/game_install.py`).

Evidence (zoombi32.exe): one function, `0x446969` (Ghidra's decompilation, with the runtime functions named), reads both keys through the engine's INI reader at `0x480790` (section `INSTALL` at `0x4a5254`; keys at `0x4a3f06` / `0x4a3f1b`), reporting the string at `0x4a520c` on failure. It builds `<INSTALLFROMDIR>Data\Zoombini.mhk` (`strcpy`, `strcat` with `0x4a3f15`, a path helper at `0x46c990`, `strcat` with `0x4a526b`), passes it to `0x483420`, and depending on the result either sets the flag at `0x4a3e5c` or reads `INSTALLTODIR`. (An earlier version of this entry wrongly split this into two functions.) It sets up C++ exception handling (`__InitExceptBlock`) and constructs and destroys a string object via `0x4850f8` / `0x48533e`, which are in the Mohawk engine's code, not Borland's.

## QuickTime installer settings file

The 32-bit QuickTime for Windows 2.1 installer on the disc (`QTWSET32/QT32B42.EXE`) reads its options from an INI named after itself (`QT32B42.INI`), not the `QT32INST.INI` shipped next to it (which belongs to the older installer in `QTWSET32/OLD32INS.EXT/`). Confirmed in the VM: with the options file named `QT32B42.INI`, `PromptToBegin=0` etc. suppress every dialog; named `QT32INST.INI`, the installer ignored it entirely (it even created a Start-menu group despite `CreateGroups=0`).

## Compiler version: Borland C++ 4.5 or 4.52 (equivalent)

Both builds were made with **Borland C++ 4.5 or 4.52**. The two generate identical code for this game (see "Compiler settings" below), so which one doesn't matter; the tools use 4.5.

| Evidence | Implies |
| --- | --- |
| 16-bit `ZOOMBINI._EX`: NE header linker version 6.1 | TLINK 7.0a (Nov 1994), which shipped only with Borland C++ 4.5 and 4.52. BC++ 4.0's TLINK 6.00 writes 5.0; BC++ 5.0's TLINK 7.1 writes 7.1 (per the TLINK version table on [VOGONS](https://www.vogons.org/viewtopic.php?t=110504)) |
| Both builds: RTL string `Borland C++ - Copyright 1994 Borland Intl.` | a 4.x runtime library; BC++ 5.0's says 1996 |
| Both builds: C++ exception handling and RTTI (`**BCCxh1`, `Bad_typeid`, `typeinfo`, `xalloc`; `zoombi32.exe` exports `__GetExceptDLLinfo`) | BC++ 4.0 or later |
| Built January 1996 (disc file dates, README) | earlier than BC++ 5.0 |
| 32-bit `zoombi32.exe`: PE linker version 2.25, subsystem version 3.10, section names `CODE`/`DATA` | Borland TLINK32; doesn't distinguish 4.5 from 4.52 on its own |

4.5 and 4.52 share the same linker, so telling them apart needs the compilers themselves: compare their runtime library code (e.g. startup code and `CW32.LIB` routines) byte-for-byte with the code linked into `zoombi32.exe`.

### 4.5 vs 4.52: the runtime libraries don't decide it

Both CDs ship the full toolchain uncompressed under `BC45/` (the run-from-CD tree), so files can be compared directly:

- `TLINK32.EXE` and `C0W32.OBJ` (32-bit startup code) are byte-identical in 4.5 and 4.52.
- `CW32.LIB` (32-bit runtime library): 725 of 726 modules are the same code in both. `fsbskoff.cpp` differs, but neither version's copy is linked into `zoombi32.exe`. 4.52 adds `fdiv32.ASM`, and its compiler (`BCC32.EXE`, which still calls itself "Borland C++ 4.5") references `__fdiv`/`__fdivflag`: 4.52 adds the Pentium FDIV bug workaround. `zoombi32.exe` contains none of that code.
- 13,236 of the 24-byte code windows shared by both libraries occur in `zoombi32.exe`, confirming the linked runtime is Borland C++ 4.5x.

So the game's runtime code is consistent with either release. The remaining test is code generation: compile the same decompiled game function with each release's `BCC32.EXE` and see which reproduces the original bytes.

## Code layout and conventions (zoombi32.exe)

- `CODE` runs from `0x410000` to `0x494000`:
  - `0x410000`: Borland's Win32 startup code (`C0W32.OBJ`), the entry point.
  - up to about `0x46f7a4`: the game's own code (~390 KB).
  - about `0x46f7a4`-`0x478000`: the Borland C++ runtime library (`CW32.LIB`), identified module by module (`uv run runtime-symbols`).
  - about `0x478000`-`0x494000`: Broderbund's Mohawk engine (platform layer: graphics "ports", audio, WaveMix, an async file API). Almost no Borland library code; its C++ class names survive as RTTI type descriptors in the code (`displayPort`, `windowPort`, `memoryPort`, `DIB8Port`, `audioObj`, `wavestreamObj`, `wmxMixer`, ...).
- Game functions seen so far clean up their own arguments (`ret N`): `__stdcall` or `__pascal`, either declared explicitly or set as the default with a compiler flag.
- Functions always get a standard stack frame (`push ebp` / `mov ebp, esp`), even trivial ones. BCC32's default settings reproduce this.

## First matched functions

The first matches (then in `decomp/first.c`): `fn_46be2e` (stores its argument in the global at `0x4a7f58`) and `fn_455e85` (returns 0, ignoring two arguments). Both match byte for byte with BCC32's default options under both 4.5 and 4.52, so they don't distinguish the releases or flags.

## Ghidra's view of zoombi32.exe

Ghidra 12.1.4's auto-analysis finds 2,567 functions: 1,313 in the game's code (below `0x46f7c5`; median 110 bytes, 66 over 1 KB), 1,019 in the runtime library and 235 thunks. It agrees with `uv run match` on the two matched functions (15 and 9 bytes). Its decompiler doesn't yet know the functions clean up their own stack arguments (e.g. `fn_455e85`'s two arguments show as `void`).

## Borland runtime functions identified

`uv run runtime-symbols` matches the code segments of the 32-bit Borland libraries (`CW32.LIB`, `CW32MT.LIB`, `BIDSF.LIB`, `OWLWF.LIB`, `OCFWF.LIB`, the `C0*32.OBJ` startup objects) against the executable, with linker-filled bytes as wildcards. With 4.5's libraries it names 217 addresses (e.g. `_strcpy` at `0x46f8f4`, `_strcat` at `0x46f864`, `@__InitExceptBlock` at `0x4716c0`); 125 segments match in several places (mostly small C++ destructors instantiated in many modules) and are left unnamed. `uv run ghidra label` applies the names to the Ghidra project.

It also records the extent of each of the 116 segments it matches uniquely (27.6 KB). The linker put the libraries together, from the first runtime function (`0x46f7a4`) to the end of the last segment (`0x4764bc`), so everything in between is library code, including static helpers with no public name (e.g. the exception-handling internals in `xx.cpp`). What follows, up to the first engine class method (`0x47af1c`), isn't Borland's: 125 functions (18.7 KB) handling MIDI and wave-device mapping (`MidiMap`, `DefaultWaveDevice`, `Software\Microsoft\Multimedia\Sound Mapper`) and calling into the engine throughout. That's the Mohawk engine's C code, before its classes, so the engine starts at `0x4764bc`.

Borland's C++ objects use "virtual segments" (COMDEF entries whose data type is a segment index; references to them set bit `0x4000` in the index) for type descriptors (`@$xt$...`), inline functions and template instances. `omf.py` reads them as extra segments named after their symbol.

## The game is C++

Evidence: the Borland C++ exception-handling and RTTI runtime is linked in; game functions such as `0x446969` call `__InitExceptBlock` (set up by the compiler for any function with local objects that have destructors) and construct and destroy objects. So decompiled code is written as C++: functions like that can't be reproduced in C.

Borland C++ 4.5, 32-bit, as observed by compiling test code:

- Plain functions are mangled with their argument types only (`fn_46be2e(long)` is `@fn_46be2e$ql`, even when declared `__stdcall`); global variables keep C names (`_g_4a7f58`).
- Methods receive `this` as a hidden first stack argument (`[ebp+8]`), not in a register as with Microsoft's compilers; a `__stdcall` method with one argument returns with `ret 8`.
- Borland's TDUMP demangler has two quirks our demangler (`src/zbtools/demangle.py`) doesn't copy: it drops the parameter after a nested type (e.g. `streambuf::seekoff(long, ios::seek_dir, int)` loses the `int`), and it appends `const` to class type descriptors. Otherwise ours agrees with it on all 2,227 mangled names in `CW32.LIB`.

## C++ classes (RTTI)

Borland C++ emits a type descriptor for each polymorphic class, and for types used in exceptions, in the code section (they are virtual segments, `@$xt$...`). `uv run classes` (`src/zbtools/rtti.py`) finds them by their layout, worked out from the runtime library's own descriptors and confirmed against the game:

| Offset | Meaning |
| --- | --- |
| `+0x00` | object size |
| `+0x04` | flags: `0x0001` class, `0x0002` has a destructor and the fields below; `0x0010` pointer type |
| `+0x06` | offset of the name in the descriptor (`0x30`; `0x10` for classes without a destructor; `0x0c` for pointer types) |
| `+0x08` | offset of the vtable pointer in objects, `-1` if none (pointer types: the pointed-to type's descriptor) |
| `+0x10` | offset of the base-class list: `(descriptor, offset, flags)` entries, 12 bytes each, ended by a null descriptor |
| `+0x14` | probably the class's deallocation function (`0x4870d1` for the port classes, which their destructors call to free the object) |
| `+0x28` | the destructor |

Each vtable, in the data section, is preceded by a pointer to its class's descriptor and two zero words: the vtable starts 12 bytes after that pointer, and slot 0 is the virtual destructor. Constructors (and destructors) store the vtable's address into the object (`mov dword ptr [reg], vtable`). Destructors take a hidden second argument whose bit 0 means "also free the memory" (e.g. `displayPort::~displayPort` calls `basePort::~basePort(this, 0)`, then the deallocation function if the bit is set).

39 classes, nearly all in Broderbund's Mohawk engine:

- graphics: `basePort` -> `displayPort` -> `windowPort` / `memoryPort`, and `basePort` -> `DIBPort` -> `DIB8Port` (37-39 virtual methods each)
- audio: `wavebuf` -> `wavebufWO` (WaveOut) / `wavebufDS` (DirectSound); WaveMix: `wmxObject` -> `wmxMixer` / `wmxWaveOut`
- files: `asyncAPI` -> twelve `async*` operation classes (`asyncCreateFile`, `asyncReadFile`, `asyncFindFirstFile`, ...)
- threading: `sync` -> `event` / `mutex` / `thread`
- Borland's own: `xmsg` -> `xalloc` / `string::lengtherror` / `string::outofrange`, `typeinfo`, `Bad_cast`, `Bad_typeid`, `string`, `TStringRef`
- the game's own code: only `fileSpec` (no vtable). So the game's logic uses non-polymorphic classes or plain functions; names for it will have to come from elsewhere.

`uv run ghidra label` makes these Ghidra classes (descriptor, vtable, constructors, destructor, and `vfuncN` for virtual methods named after the class that introduces them) and sets `__stdcall` on the 1,365 functions that pop their own arguments.

## Compiler settings: `-p -k-`, otherwise BCC32's defaults

The game's code is reproduced by BCC32 with **`-p`** (the Pascal calling convention by default) and **`-k-`** (no standard stack frame unless needed), and otherwise its defaults: no optimisation (`-Od`), register variables on (`-r`), byte alignment. The support library just below the runtime (`0x46d754`-`0x46f7a4`) was compiled without `-k-`. Evidence, from compiling decompiled game functions (`decomp/`) and comparing:

- Four functions match byte for byte with the defaults, including two with loops, register-allocated locals and hoisted addresses (`fn_4572bf`, `indexOfLargestExcept` (`0x437390`)); what looks like optimisation in the game (registers for locals, rotated loops, no stack frame for argument-less functions) is BCC32's default code generation.
- `-O1` and `-O2` break two and three of them respectively; `-r-` (no register variables) breaks two.
- The CPU target (`-3`/`-4`/`-5`) and `-a4` don't change these functions, so they aren't pinned down (`-k-` is, below); the defaults are assumed. A structure with a pointer at the unaligned offset `0xe` (in `fn_4115f5`) fits the default byte alignment.
- 4.5 and 4.52 produce identical code, with the same settings, for the game functions and for test code with floating point, division and a `switch`. 4.52's one addition is `-fp` (the Pentium FDIV workaround; 4.5 rejects the option), and the game contains none of its support code.

Details that depend on how the source is written, found while matching:

- A comparison's operand order follows the source (`exclude != i` gives `cmp ax, si`; `i != exclude` gives `cmp si, ax`).
- Initialising several variables in the `for` header (`for (i = 1, best = 0, ...)`) versus in declarations changes the order of the setup instructions.
- A search loop written `while (p && !found) p = p->next;` compiles to the game's layout; a `for` with `break` doesn't.

### The Pascal calling convention (`-p`)

Two functions (`fn_4115f5`, `spliceList` (`0x43a772`)) at first matched in everything but the order their parameters were loaded and the registers they landed in. Experiments showed BCC32 loads a function's parameters last-declared first, and gives `eax` to the most-used variable (the first parameter on a tie). The originals load the parameter at `[ebp+8]` first, so it must be the *last* declared: parameters pushed left to right, which is the Pascal convention. It also pops its own arguments (`ret N`), so it looks like `__stdcall` from the return alone; with one parameter or none the two produce identical code.

Written as plain functions with their parameters in Pascal order and compiled with `-p`, both match; without `-p` they don't. Across the game's code, 785 functions with parameters pop them (`ret N`) against 49 that don't (probably variadic, like the error reporter at `0x41541a`), which is what a global `-p` looks like. The Mohawk engine is different (416 against 240): it was built separately and declares conventions per function.

Borland upper-cases the whole mangled name of `__pascal` functions (`@FN_4115F5$QSL`); `demangle.py` and `match` handle that.

Ghidra's 32-bit x86 support has no `__pascal` convention, so `ghidra label` marks these functions `__stdcall`: the stack clean-up is right, but **Ghidra numbers their parameters in reverse** (its `param_1` is the source's last parameter).

### Stack frames (`-k-`) and a separately compiled support library

`-k-` only shows in functions that make calls but have no parameters and no stack locals: without it they get a `push ebp` / `mov ebp, esp` frame; with it they don't. Functions with parameters (used or not) or stack locals get a frame either way, which is why it didn't show in the earlier tests.

`isMousePresent` (`0x455903`) (wraps `GetSystemMetrics(SM_MOUSEPRESENT)`) has no frame and matches only with `-k-`; `currentTimeMs` (`0x46dda5`) (wraps `timeGetTime()`) has one and matches only without. Counting the functions that make calls but never touch a parameter or stack local: in the game's own code 166 have no frame and 23 have one (callbacks that take a parameter they don't use); in the support library from `0x46d754` to the runtime (`0x46f7a4`), the stretch holding the `fileSpec` and threading classes, all 14 have one. (The QuickTime glue in between, below, is Apple's code and doesn't count.) So that support library was compiled with frames and the rest of the game with `-k-`. Its modules (`decomp/os_*.cpp`) set `/* @flags -p */`.

The game was evidently built with per-module options (as an IDE project allows), so other modules may turn out to differ too.

## QuickTime for Windows glue (`0x46cca0`-`0x46d754`)

Between the game's code and its support library sits the glue from Apple's QuickTime for Windows 2.x SDK, statically linked (`src/zbtools/quicktime.py`). It loads `QTIM32.DLL` (QuickTime) and `CMGR32.DLL` (its component manager) with `LoadLibrary` and fetches each one's single `_EntryPoint` (and `_CMgrInitialize`/`_CMgrTerminate`) with `GetProcAddress`; `QTIM32.DLL`'s file version is read through `version.dll`. Every API function is a hand-written assembly stub (16-bit register use gives it away) that loads a selector into `bx` and calls the dispatcher through a pointer: 102 QTIM stubs (13 bytes each, pointer at `0x4a7f84`) and 24 CMGR stubs (17 bytes, also setting `ax` = 1, pointer at `0x4a7f90`). Until a DLL is loaded its pointer holds a fallback (`0x46ce80`, `0x46cca0`) that calls `QTInitialize` and retries; `0x46d3dc` is `QTInitialize(long *version)` and `0x46d745` is `QTTerminate()`.

Nothing maps selectors to API names: `QTIM32.DLL` exports only `_EntryPoint`, a few helpers (`Flip16`, `GetMemory`, ...) and unnamed ordinals, and the Win16 build imports `QTIM.DLL` by ordinal. So the stubs are named by selector (`qtim_39`) until the way the game uses one identifies it. The glue isn't Broderbund's code: the inventory puts it in its own `quicktime` region, as library code, and `uv run ghidra label` names it.

## A breakpoint compiled into the game (`0x46db83`)

The function at `0x46db83` is `push ebp / mov ebp, esp / movsx eax, word ptr [ebp+8] / int3 / mov ax, word ptr [ebp+8] / pop ebp / ret 4`: it returns its (short) argument after stopping in the debugger. Ghidra models `int3` as never returning, so its analysis ended the function at the breakpoint (8 bytes, no return); `uv run ghidra label` now makes such breakpoints fall through, disassembles the rest and recomputes the function (16 bytes). It's the only function in the program that ended at an `int3`. `__emit__(0xcc)` and pseudo-register variants compile differently, so it was probably inline assembly (`asm int 3`), which BCC32 can only compile with TASM32, not part of Borland C++ 4.5; it's decompiled as `debugBreak`, calling `DebugBreak()` (functional).

## Switch tables cut functions short in Ghidra

BCC32 compiles a `switch` to `jmp dword ptr [reg*4 + table]` with the table right after the jump, inside the function. Ghidra's first analysis couldn't follow some of these (before the game's calling conventions were known) and ended the function at the jump, so the worklist listed large functions as tiny ones (`0x420a60` as 29 bytes rather than 518). `uv run ghidra label` now decompiles each function with an unresolved computed jump again, lets the decompiler's switch analysis recover the table, and recomputes the body; this recovered dozens of functions. It also explains some odd instructions in a linear disassembly (`in`, `out`, `cli` in `0x42365a`, `0x431ea0`, `0x436d39`): they're table bytes, not code.

## Functions written in assembly

BCC32 only compiles inline `asm` with TASM32, which isn't part of Borland C++ 4.5. These functions can't come from C++ at all, and as the decompiled code is kept portable (no emitted bytes or pseudo-registers), they're written as portable equivalents (`@zoombi32-functional`) or left as documented stubs (`@zoombi32-nonmatching`):

- `0x46dabb`, `0x46da9d`, `0x46daac` (`atomicIncrement`, `atomicDecrement`, `atomicExchange`): `lock inc`, `lock dec`, `xchg`, probably because Windows 95's `InterlockedIncrement`/`Decrement` only return the sign of the result. Written with the `Interlocked*` functions (functional). (They could be matched exactly with `__emit__` and pseudo-registers, as could `0x46f771`, which is how they were first matched.)
- `0x46f771`: walks `depth` stack frames up with a hand-written loop and records the return address there; no portable equivalent (a stub).
- `0x46f6f9`, `0x46f74f`: switch stacks (`mov esp, [eax+0x50]`, `jmp edx`). With `0x46f771`, the Mohawk OS layer's stack switching, which a port would replace (e.g. with Windows fibers).
- `0x46db83` (`debugBreak`): an `int3` mid-function; written with `DebugBreak()` (functional).
- Not yet checked: `0x46f6c9` and `0x46f70e` (`pushfd`/`popfd`), `0x4697f1` (`pushfd`/`popfd`, `sahf`).

## The game's source modules

TLINK32 lays out each object file's code contiguously, in link order, and pads it with zeros to a 4-byte boundary (sometimes followed by whole zero words). Where a function is followed by that padding, a module ends; a module whose code happens to be aligned shows none, so about one boundary in four is invisible. The data follows the same order: each module's uninitialised globals form their own range, which steps up at the padded boundaries. (A strict test that every module's data lies above the previous one's fails, as some strings and shared globals interleave, so padding is the evidence used.)

`decomp/modules.toml` maps the game's code into 42 modules this way (`uv run modules` checks it against the padding and prints each range's strings and imports). The strings name most of them: the puzzles by their Mohawk archives (`bridge.mhk` for the Allergic Cliffs, `Ferry.MHK`, `Tunnels.MHK`, `Picker.MHK`, `Net.MHK`; Pizza Pass by its trolls Arno, Willa and Shyler), and infrastructure by its messages (graphics, sound, animation, buttons, the saved-player roster `Zoombini.who`, `Zoombi32.CFG`, the Windows layer's display-mode checks; `WinMain` is at `0x4546f8`, in `game`). Several messages name functions `e2...` (`e2SetupAnim`, `e2AllocHandle`, `e2GetPoolValue`), a layer of the game over the engine. The support library below the runtime is the "Mohawk OS" layer (its window class is `MOHAWK OS Manager`): reference counting, local memory, timers and hooks, and the threading classes. Some ranges are large enough (`lilly`, 46 KB) to hold several modules with invisible boundaries between them.

## Functions Ghidra's analysis missed

Ghidra's auto-analysis only finds functions it reaches by calls from other functions, so it missed ones reached only through pointers: `WinMain` (`0x4546f8`, which only the startup code's table points to, at `0x4a0044` next to the math error handlers), window procedures, callbacks in data tables, frameless (`-k-`) functions that start whole modules. `uv run ghidra label` now creates them (code a relocated pointer points to, module starts, code after a function that starts with `push ebp / mov ebp, esp`, and the targets of direct calls that aren't in a function yet), about 230 so far. The pitfalls, each handled:

- Ghidra's `FlatProgramAPI.createFunction` doesn't disassemble: on undecoded bytes it makes a function whose body is its first byte. 278 functions (including uncalled QuickTime stubs and runtime functions created by earlier versions of `label`) were like that; they're disassembled now.
- Relocated pointers in the code section aren't always code: switch jump tables (address lists after the `jmp`, byte index tables after those) and the RTTI type descriptors live there too. Only pointers from data, or from an instruction in a function but not inside a memory operand (`[eax*4 + table]`, `[eax + table]`), count; functions an earlier version made from tables or descriptors are removed.
- `rtti.py` read one entry past the end of each vtable: the next vtable's header starts with a pointer to its class's type descriptor, which is in the code section. 18 classes had a bogus last virtual method; vtables now stop at a descriptor.

About 23 KB of non-zero bytes in the code section still belong to no function, mostly the cases of switches Ghidra can't recover.

## WinMain (`0x4546f8`) and what it shows about the source

- Only the startup code's table points to `WinMain` (`0x4a0044`), which is why Ghidra missed it. It checks it's the only copy running (a window whose class is the program's own file name, a previous instance, the global atom `Zoombini`), switches on debug mode if the command line starts with `d`, starts the engine step by step (each failure fatal, with a message naming the step), checks memory (and physical memory on anything newer than Windows 3.11) and sound devices, finds the game data (`findGameData`), opens a 640x480, 256-colour display, loads the CornerStone font at two sizes and cursors 1-5, requires QuickTime 2.1 (version >= 0x2300), and runs the main loop until a flag is set.
- **Its messages are named strings, not literals.** Borland C++ addresses a function's string literals from one base kept in a register once there are enough of them; the original pushes each message's own address, and the messages sit together in a table of their own (`0x4a4da4`-`0x4a4f7a`) along with messages the Windows game never uses: "Virtual Memory must be turned off.", "Requires Sound Manager 3.1 or later to be installed.", "GetVol failed.  Boot drive." (Mac OS terms). The game's code evidently came from the Mac version, and so did its message table. `WinMain`'s own literals (`"Zoombini"` three times, `".txt"`, `"CornerStone"` twice) are separate copies at `0x4a4996`-`0x4a49cd`.
- **Resource types are built Mac-style:** the cursors' type is `0x43555253` (`'C'` in the high byte). Borland's multi-character constant `'CURS'` has the bytes the other way round, so `decomp/zoombinis.h` has `RESOURCE_TYPE('C','U','R','S')`.
- `atexit` gets a 6-byte function that only calls `0x454caa`: `atexit` needs the C convention while the game's functions are Pascal, so the original had a `__cdecl` wrapper.
