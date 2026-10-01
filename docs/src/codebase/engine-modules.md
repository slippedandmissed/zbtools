# The Mohawk engine's modules and options

TLINK32's padding splits the engine (`0x4764bc` to the import thunks) into 149 object files, many holding one function. QuickDraw-style helpers (`emptyRect`, `offsetRect`, `sectRect`) each sit alone, as in a library built for smart linking. They use the game's `-p -k-` (Pascal order, arguments popped, like the Mac Toolbox calls they imitate); the rest of the engine uses `-p -x-` (no `-k-`, so leaf functions get frames, and no exception frames) and its C++ classes' methods (about 240) don't pop their arguments, so they're `__cdecl`. Each module's `@flags` comment says which.

**Exceptions are off** in the engine: functions with `fileSpec` locals have no `__InitExceptBlock`. A `fileSpec` built from a directory and a name returns `this` in `eax`, which a by-value `operator+` wouldn't.

**Regions** are rectangle lists in a handle (`'Rngr'` tag, bounds, capacity grown 16 at a time, count, rectangles) with the last region call's error in `regionErrorCode`.

For compiler details see [The compiler](../concepts/compiler.md): the engine is built with 4.5 or something very close, probably against newer Windows headers, and its remaining near-misses are register-allocation differences.
