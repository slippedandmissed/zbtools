# Summary

[Introduction](introduction.md)

# Getting started

- [Prerequisites](getting-started/prerequisites.md)
- [Setup](getting-started/setup.md)

# How the decompilation works

- [How the pieces fit](concepts/overview.md)
- [The workflow](concepts/workflow.md)
- [Matching](concepts/matching.md)
  - [The compiler: Borland C++ 4.5](concepts/compiler.md)
  - [Matching BCC32: a field guide](concepts/bcc32-quirks.md)
  - [Functions written in assembly](concepts/assembly.md)
  - [Near-misses](concepts/near-misses.md)
- [Naming and headers](concepts/naming.md)
- [Source modules](concepts/modules.md)
- [How data is laid out, and matched](concepts/data-layout.md)
- [Code layout of zoombi32.exe](concepts/binary-layout.md)
- [The rebuilt executable](concepts/rebuilt-exe.md)

# Tools

- [Tool reference](reference/tools.md)
- [Ghidra: its limits, and what label fixes](reference/ghidra-notes.md)
- [Runtime symbols and C++ classes](reference/runtime-and-rtti.md)
- [The Windows 98 VM](reference/vm.md)
- [Python conventions](reference/python-conventions.md)

# File formats

- [Mohawk archives](formats/mohawk.md)
- [Working with the assets](formats/assets-workflow.md)
- [Sounds, music, images and scripts](formats/sound-images-scripts.md)
- [Small resource types and installed files](formats/other-resources.md)
- [Movies and the QkBk codec](formats/movies.md)
- [QuickTime for Windows glue](formats/quicktime-glue.md)
- [Zoombi32.CFG and installer settings](formats/install-config.md)

# The decompiled codebase

- [Repository layout](codebase/layout.md)
- [Architecture](codebase/architecture.md)
- [Startup: WinMain](codebase/startup.md)
- [The main loop, events and input](codebase/main-loop.md)
- [How the game talks to Windows](codebase/windows.md)
- [The game layer](codebase/game-layer.md)
- [Entities and how they relate](codebase/entities.md)
- [Game state, saved games and the roster](codebase/game-state.md)
- [Modules and scenes](codebase/modules-and-scenes.md)
- [C++ classes (RTTI)](codebase/classes.md)
- [Engine: modules and options](codebase/engine-modules.md)
- [Engine: memory and resources](codebase/engine-memory-resources.md)
- [Engine: graphics ports](codebase/engine-graphics.md)
- [Engine: OS layer, timers and audio](codebase/engine-os-audio.md)

# Gameplay and the code

- [Overview and quick lookup](gameplay/index.md)
- [Starting the game](gameplay/start.md)
- [Zoombini Isle](gameplay/isle.md)
- [The map and the journey](gameplay/map-and-journey.md)
- [The camps](gameplay/camps.md)
- [Group 1: Cliffs, Caves, Pizza](gameplay/group1.md)
- [Group 2: Ferry, Toads, Stone Rise](gameplay/group2.md)
- [Group 3: Fleens, Hotel, Mudball Wall](gameplay/group3.md)
- [Group 4: Lion, Mirror, Bubblewonder](gameplay/group4.md)
- [Zoombiniville](gameplay/zoombiniville.md)
- [Hidden scenes and debug keys](gameplay/hidden.md)

# The port

- [Overview](port/overview.md)
- [miniwin: Win32 over SDL](port/miniwin.md)
- [The host layer](port/host.md)
- [Compiling Borland's dialect](port/borland-dialect.md)
- [QuickTime in the port](port/quicktime.md)
- [The web build and packaging](port/web.md)
- [Debugging the port](port/debugging.md)
- [Quirks](port/quirks.md)

# Appendix

- [Screenshots](appendix/screenshots.md)
- [Glossary](appendix/glossary.md)
