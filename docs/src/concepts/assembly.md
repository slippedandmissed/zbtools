# Functions written in assembly

BCC32 only compiles inline `asm` through TASM32, which isn't part of Borland C++ 4.5, so some of the original's functions were assembled separately, or compiled via TASM. They cannot come from C++, and decompiled code must be portable (no `__emit__`, no pseudo-registers). Each is therefore written as a portable equivalent marked `@zoombi32-functional`, or, where none exists, left as a documented stub.

| Where | What | Portable form |
| --- | --- | --- |
| OS layer | `atomicIncrement`/`Decrement`/`Exchange` (`lock inc`, `xchg`) | `Interlocked*` (functional) |
| OS layer | `initContext`, `resumeContext`, `abandonContext`, `switchContext`: carve a thread's stack and switch registers, `esp` and flags | functional with Win32 fibers (Windows 98/NT 4; not 95) |
| OS layer | `recordReturn`: walk stack frames | documented stub |
| OS layer | `fixedMul`, `fixedDiv`: 16.16 arithmetic | functional |
| OS layer | `debugBreak`: `int3` | `DebugBreak()` |
| WaveMix | the mixing inner loops (`xlat`, `jo` saturate, `rep movs`) | functional |
| Graphics | nearest-colour search; packed-pixel draw and read; the DIB8 blitters | functional |
| Graphics | `decompressImage`: differs from our compile only in TASM's accumulator encodings (`66 25 0f 00` for `and ax, 0xf`) | functional |

The assembly-built modules went through TASM, which is why a few differ only in instruction *encodings* from BCC32's output. Assembling through TASM does not change register allocation, so it doesn't explain other near-misses.

Packed pixels, the engine's run-length encoding of 8-bit images, are described in [Images](../formats/sound-images-scripts.md).
