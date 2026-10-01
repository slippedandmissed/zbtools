# The game layer: views, scripts, animation and resources

Between the engine and the scenes sits the game's own framework. Learn this once and every puzzle's code reads the same way.

## Views (`view.cpp`)

A **view** (`View`, `0xbc+0x30` bytes) is anything on screen that can change: a Zoombini, a button, a troll, a plank, a piece of scenery. Views live in a doubly linked list between two sentinels, `viewHead` and `viewTail`, **drawn back to front**; `sortViews` orders them by depth when `requestViewSort` was called. Each has:

| Field | Meaning |
| --- | --- |
| `id` | looked up with `findView(id)`; scenes keep ids in `…Views` globals |
| `draw`, `update` | callbacks: `draw(view)` paints it; `update(view, region)` adds the area it changed to a region |
| `notify(view, event)` | told of its script's *events* (`-1`: the script ended) |
| `placed(view)` | called after its script places its cels (so a scene can adjust them) |
| `kind`, `body.script`, `body.scriptGroup` | its current `SCRB`/`SCRS` script and the image bank it uses |
| `body.cels[24]` | what it draws now: `{image, x, y}` triples |
| `interval`, `nextUpdate` | frame timing (in ticks); `flags` (1: a Zoombini, 2: large body, …) |

`updateViews` (called every frame) steps each view whose time has come (`runViewScript` for scripted views), collects the changed regions, redraws the affected views clipped to them into the work port, and copies the region to the screen. A scene's own code only creates views (`startView`, `addSmokeSnoidView`…), gives them scripts (`setViewScript`), reacts to their notifications and moves them (`moveView`, `groupViews`, `pairViews` for views that move together).

## Scripts (`SCRB`, `SCRS`)

A view's animation is a **script** from a Mohawk archive: a list of frames, each a list of cels (image number from the view's bank, x, y) and an end word that is either `0xff00 + event` (tell the view's owner) or `0xfe00 + event` followed by a sound to play. `runViewScript` and `runViewCels` step them; `queueViewSound` plays sounds a script asks for. A scene loads its scripts when it opens (`loadScripts(first, count)`) and its images as **banks** (`loadImageBank`), and finds a script by id (`findScript`). A `SCRS` script is a Zoombini's: the same, with the way the Zoombini faces. The format is documented in [Sounds, images and scripts](../formats/sound-images-scripts.md).

The pattern in every puzzle is therefore:

```text
 user click ──▶ <scene>Clicked(which)         set state, start a view's script
 script frame ──▶ view->notify(view, event)   the script reached a marked frame: the game's logic
 <scene>Frame() each tick                      sequencing, timers, "everyone has crossed", fidgets
```

Event numbers are the contract between the art (the script data) and the code (the `…Notify` switch). They are why the `…Notify` functions are full of numeric `case`s (`case 3:`, `case 20:`): the numbers come from the scripts in `assets/<ARCHIVE>/SCRB/`.

## Features and Zoombini layers (`features.cpp`, `snoids.cpp`)

Despite the name, `features.cpp` holds the *image banks by group*, cel drawing, and the dialogs; the "features" of a Zoombini (hair, eyes, nose, feet) are in `snoids.cpp`. A Zoombini on screen (`Snoid`) draws layers of images chosen from per-feature tables (`hairImages`, `eyesImages`, `noseImages`, `feetImages`, with alternate tables for the second walking pose and the other facing) and walks paths from the `NODE`/`PATH` tables.

## Animations (`anim.cpp`)

An older, self-contained animation player from the engine's lineage: a script of opcodes (see `stepAnim`) moving up to 32 sprites (*cast members*, images from resources) over a background, drawing through lists of changed rectangles. Nothing else in the decompiled game calls it (`playAnim` and `playAnimation` have no callers), so it is dead code kept by the linker; the game's moving things are all views.

## The e2 layer (`e2memory.cpp`, `loading.cpp`, `graphics.cpp`, `sound.cpp`)

Error messages name it: `e2AllocHandle`, `e2SetupAnim`, `e2GetShapes`, `e2MapSave`. It wraps the engine for the game's use:

- **Memory and resources** (`e2memory`): handles and pointers, resources (asking for the CD if it's missing), shapes and lists of them, palettes, sound lists, fonts; counts the memory used. `setFreeAtOnce` makes a scene free as it loads.
- **Graphics** (`graphics`): the screen port and the off-screen **work port**, the palette, images, saved screen areas (`e2MapSave`), clip regions.
- **Sound** (`sound`, `basecamp`): wave sounds by key (`loadWave`, `playWave`, `waitForWave`) and MIDI, in up to four channels per type (`soundChannels`), with `loadSound`/`unloadSounds` lists a scene uses.
- **Loading and errors** (`loading`): a small `printf`-like formatter (`%L` for text in locked resources) and the error reporter that composes "Unable to load …" messages (`joinText`, `reportJoinedError`).
- **Fades and wipes** (`basecamp`): `runWipe`/`startScreenWipe` and `runBlinds`/`startScreenBlinds` are the two screen transitions (wipe and venetian blinds); `fadeInViews`/`fadeOutViews` fade palettes.

## Hints and remarks

Each scene has a pool of sound ids for the remarks it makes (`bridgeSounds`, `pizzaSounds`, … in `net.cpp`, indexes of `'tWAV'` resources), and "used" bitmasks (`bridgeSoundsUsed`) so a remark isn't repeated until the pool is exhausted. `replayHint` (`features.cpp`) repeats the scene's introduction.

## Dialogs (`features.cpp`)

`loadDialogs`/`showDialog` and the `askKeepParty`, `askNewGame`, `askLoadGame`, `askSaveGame` and `askQuit` helpers build modal dialogs out of views and scripts from `ZOOMBINI.MHK`, with text from `dialogTexts` (289 strings, e.g. "THE CURRENT PARTY OF ZOOMBINIS WILL BE LOST IF YOU GO TO THE MAP"). While one is open, `dialogFlags` is set and `gameKey` routes keys to it.
