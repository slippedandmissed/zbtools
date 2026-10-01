# Compiling Borland's dialect with clang

The decompiled code is Borland C++ 4.5 C++. `port/CMakeLists.txt` builds it with clang so that **`decomp/` needs no changes**:

| Mechanism | Effect |
| --- | --- |
| `-include miniwin/prelude.h` | force-included before every decompiled source |
| `#pragma pack(1)` in the prelude (after the standard headers, which keep the host's packing) | the game's structs are byte-packed as BCC32 packs by default; they mirror data files and each other's sizes, so their layout must be the original's |
| `miniwin/types.h` | Borland's calling-convention and memory-model keywords (`__pascal`, `__cdecl`, `__stdcall`, `far`, `near`, `huge`, `_export`…) are defined away; Win32 types have Win32's sizes on every host (`DWORD` 32 bits even where `long` is 64) and handles are pointers to distinct incomplete types |
| `miniwin/borland.h` | the Borland runtime's extras (`itoa`, `stricmp`, `getdisk`/`setdisk`, `getcwd`/`chdir`, `gettime`, `_dos_getdate`; Borland's global `struct time` shares a name with the C library's function) |
| `#define fopen miniwin::fopen` (and `getcwd`, `chdir`) | the game's Windows paths go through miniwin's drives |
| `-fsigned-char` | BCC32's `char` is signed (it isn't by default on ARM) |
| `-fno-exceptions -fno-rtti` | the game uses neither: BCC32's exception frames were compiled into the original's objects, but the decompiled code only constructs and destroys objects |
| `-fno-strict-aliasing -fwrapv` | the code aliases freely and relies on wrapping signed overflow |
| `-w -Wno-register -Wno-c++11-narrowing` | modern C++ warns about much BCC32 accepted; `register` and narrowing in case labels are errors by default and are turned off |

The consequence is that `-w` hides real warnings; the matcher, not the compiler, is the check of correctness.

## Things the dialect hides

- **`long` is 4 bytes in the original.** The decompiled code keeps pointers in `long`s in places, which is why the port is [32-bit only](overview.md#limits). Typing those pointers as pointers (a standing rule for new code) is the way out.
- **The game's own placement `operator new`** (zeroing) is `audioObj`'s, because standard C++ reserves the global placement `operator new(size_t, void *)` and compilers skip replacements of it.
- **`__emit__` and inline assembly** only exist in `glue/` (not built for the port; `port/glue/quicktime.cpp` replaces it).
