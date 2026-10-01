# Glossary

| Term | Meaning |
| --- | --- |
| **Asyncify** | Emscripten's transformation that lets code with blocking loops hand control back to the browser and resume; what makes the port's fibers and `hostYield` work |
| **BCC32 / TLINK32** | Borland C++ 4.5's 32-bit compiler and linker |
| **bank** (image bank) | many images in one `tBMP` resource, each with its own header; a feature's frames |
| **cel** | one image placed by a script frame: `{image, x, y}` |
| **e2** | the game's layer over the engine (`e2AllocHandle`, `e2SetupAnim`…) |
| **feature** | a Zoombini's hair, eyes, nose or feet (value 1-5); also, in the code, a view with a script |
| **frame hook** | the callback the main loop calls once a pass: `gameFrame` |
| **functional** | decompiled, complete and portable but not byte-exact (`@zoombi32-functional`) |
| **group** (input) | a set of input items with handlers, listed in a group list |
| **group** (puzzle) | a set of three puzzles, each with four levels |
| **handle** | the engine's Mac-style movable memory block reference |
| **marker** | the `/* @zoombi32 0x… */` comment before a function |
| **match** | a decompiled function compiles to the original's exact bytes |
| **MHK / Mohawk** | Broderbund's archive format and engine |
| **miniwin** | the port's Win32 subset over SDL |
| **near-miss** | a function that nearly matches |
| **pending / due scene** | `pendingScene` is the next scene to enter; `sceneDue` is a scene's own request to leave |
| **port** (engine) | a QuickDraw-style drawing surface (`basePort` and subclasses); (the port) the WebAssembly/native build |
| **QkBk** | Broderbund's QuickTime video codec: a scene compositor, not a pixel codec |
| **RTTI** | run-time type information; how the engine's class names survive |
| **scene** | one screen of the game, with open/close/frame/key callbacks |
| **script** (SCRB/SCRS) | an animation: frames of cels with events and sounds |
| **snoid** | the code's word for a Zoombini |
| **traveller** | a Zoombini on the journey (`Traveller`) |
| **view** | an animated thing on screen, in the view list |
| **WaveMix** | the software mixer inside the engine's audio |
| **work port** | the off-screen port the game draws into; changed regions are copied to the screen port |
