# Runtime symbols and C++ classes (RTTI)

## Borland runtime functions

`uv run runtime-symbols` matches the code segments of the 32-bit Borland libraries (`CW32.LIB`, `CW32MT.LIB`, `BIDSF.LIB`, `OWLWF.LIB`, `OCFWF.LIB`, the `C0*32.OBJ` startup objects) against the executable, with linker-filled bytes as wildcards. With 4.5's libraries it names 217 addresses (e.g. `_strcpy` at `0x46f8f4`, `_strcat` at `0x46f864`, `@__InitExceptBlock` at `0x4716c0`); 125 segments match in several places (mostly small C++ destructors instantiated in many modules) and are left unnamed. `uv run ghidra label` applies the names to the Ghidra project.

It also records the extent of each of the 116 segments it matches uniquely (27.6 KB). The linker put the libraries together, from the first runtime function (`0x46f7a4`) to the end of the last segment (`0x4764bc`), so everything in between is library code, including static helpers with no public name (e.g. the exception-handling internals in `xx.cpp`). What follows, up to the first engine class method (`0x47af1c`), isn't Borland's: 125 functions (18.7 KB) handling MIDI and wave-device mapping (`MidiMap`, `DefaultWaveDevice`, `Software\Microsoft\Multimedia\Sound Mapper`) and calling into the engine throughout. That's the Mohawk engine's C code, before its classes, so the engine starts at `0x4764bc`.

Borland's C++ objects use "virtual segments" (COMDEF entries whose data type is a segment index; references to them set bit `0x4000` in the index) for type descriptors (`@$xt$...`), inline functions and template instances. `omf.py` reads them as extra segments named after their symbol.

## C++ classes (RTTI)

Borland C++ emits a type descriptor for each polymorphic class, and for types used in exceptions, in the code section (they are virtual segments, `@$xt$...`). `uv run classes` (`src/zbtools/rtti.py`) finds them by their layout, worked out from the runtime library's own descriptors and confirmed against the game:

| Offset | Meaning |
| --- | --- |
| `+0x00` | object size |
| `+0x04` | flags: `0x0001` class, `0x0002` has a destructor and the fields below; `0x0010` pointer type |
| `+0x06` | offset of the name in the descriptor (`0x30`; `0x20` for classes with a base-class list but no destructor, such as `audioObj`; `0x10` for classes with neither; `0x0c` for pointer types) |
| `+0x08` | offset of the vtable pointer in objects, `-1` if none (pointer types: the pointed-to type's descriptor) |
| `+0x10` | offset of the base-class list: `(descriptor, offset, flags)` entries, 12 bytes each, ended by a null descriptor |
| `+0x14` | probably the class's deallocation function (`0x4870d1` for the port classes, which their destructors call to free the object) |
| `+0x28` | the destructor (name at `0x30`) |

Each vtable, in the data section, is preceded by a pointer to its class's descriptor and two zero words: the vtable starts 12 bytes after that pointer, and slot 0 is the virtual destructor. Constructors (and destructors) store the vtable's address into the object (`mov dword ptr [reg], vtable`). Destructors take a hidden second argument whose bit 0 means "also free the memory" (e.g. `displayPort::~displayPort` calls `basePort::~basePort(this, 0)`, then the deallocation function if the bit is set).

46 classes, nearly all in Broderbund's Mohawk engine; the hierarchy is in [C++ classes](../codebase/classes.md). The game's own code has only `fileSpec` (no vtable): its logic is plain functions and structs.

`uv run ghidra label` makes these Ghidra classes (descriptor, vtable, constructors, destructor, and `vfuncN` for virtual methods named after the class that introduces them) and sets `__stdcall` on the 1,365 functions that pop their own arguments.
