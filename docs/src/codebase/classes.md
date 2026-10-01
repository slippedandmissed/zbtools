# C++ classes (RTTI)

The Mohawk engine and OS layer are polymorphic C++; the game's own logic is not (only `fileSpec` among its types has methods, and it has no vtable). Borland C++ emits RTTI type descriptors for each polymorphic class, and `uv run classes` recovers 46 of them ([how](../reference/runtime-and-rtti.md)). The names are the engine's own and are used exactly.

```text
basePort                       ports: QuickDraw's GrafPort over a GDI DC (37-39 virtual methods)
 ├─ displayPort
 │   ├─ windowPort             a window's DC
 │   └─ memoryPort             an off-screen bitmap
 └─ DIBPort                    GDI into a DIB section
     └─ DIB8Port               also draws into the bits itself (8-bit blitters)
DIB                            helper around a DIB section (no vtable parent)

audioObj                       sounds (22-slot vtable; the base implements seven)
 ├─ midiObj
 ├─ waveObj
 └─ wavestreamObj              streamed from the resource's file
wavebuf                        a waveOut-like output buffer
 ├─ wavebufWO                  WaveOut, written ahead by a polling thread
 └─ wavebufDS                  a looping DirectSound buffer
wmxObject                      WaveMix
 ├─ wmxMixer                   software mixing
 └─ wmxWaveOut                 straight to waveOut

asyncAPI                       asynchronous file operations
 └─ async… (12 classes)        asyncCreateFile, asyncReadFile, asyncFindFirstFile, …

sync                           the OS layer's synchronisation objects
 ├─ event
 ├─ mutex
 └─ thread                     cooperative thread on a fiber

xmsg ─ xalloc, string::lengtherror, string::outofrange   Borland's exceptions
typeinfo, Bad_cast, Bad_typeid, string, TStringRef        Borland's runtime
fileSpec                       the game's file specifier (no vtable)
```

## Conventions that follow from them

- A vtable is in `DATA`, preceded by a pointer to its class descriptor and two zero words; slot 0 is the virtual destructor. Constructors store the vtable address into the object; destructors take a hidden flags argument (bit 0: also free).
- The engine's methods are `__cdecl` (they don't pop their arguments) and take `this` as the first stack argument.
- A class that declares no destructor under a base with a virtual one gets an implicit destructor, emitted in the *last* module that needs the class's vtable (`displayPort::~displayPort` lives in `windowPort`'s module). `@zoombi32-implicit` markers name them and `match` measures them.
- The port classes' sizes matter: `DIBPort` is declared under `#pragma pack(push, 4)` so `newPort` allocates `0xc8` bytes for it and `0xc6` for the derived `DIB8Port`.
- An object created by a global's constructor or destructor (`iniState`, `files`) is run by unnamed `<startup>`/`<exit>` functions listed in the object's `_INIT_`/`_EXIT_` segments.
