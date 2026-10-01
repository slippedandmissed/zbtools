# Entities and how they relate

## The game's data model

```text
                         gameState (0xae05 bytes: the saved game)
                              │
        ┌─────────────────────┼──────────────────────────────┐
        │                     │                              │
   Party ───────────┐    records (16)                  sceneFlags / puzzleLevels
   count             │    (group, level, date)          (per-scene progress)
   Traveller[32]     │
        │            │
        │ 1 : 1      │ waitingParties / savedParty (camps, isle)
        ▼            ▼
   Traveller ──features──▶ hair, eyes, nose, feet (1-5 each: 625 kinds)
        │  place, onboard, name
        │
        │ one per Traveller, created by each scene when it opens
        ▼
     Snoid  (a View's body + more: 0x103 bytes)
        │  layers[16]: images chosen from the feature tables
        │  path / pathIndex: where it is walking on NODE/PATH tables
        │  action, pose, facingLeft, chosen (in the party), idleTicks
        ▼
     View ──────────────▶ ViewBody: cels[24], script, frame, group, clip
        │  draw / update / notify / placed callbacks
        │  prev / next: the view list (viewHead … viewTail), drawn back to front
        ▼
     Script (SCRB / SCRS from a Mohawk archive) ──events──▶ notify(view, event)
```

```text
   Scene ── open / close / frame / key ──▶ owns its views, its scripts, its image banks,
     │                                      its sounds, and one GroupList of input items
     │
     ├── GroupList ─ Group ─ InputItem[]    on-screen controls (focus.cpp)
     ├── SceneButton[]                     (the same items, with their rectangles)
     └── puzzle state                      FeatureRule(s), level tables, counters
```

## Type reference

| Type | Defined in | What it is |
| --- | --- | --- |
| `Scene` | `zoombinis.h` | `{open, close, frame, unknownC, key}`: five callbacks. `scenes[22]` is the table. |
| `View`, `ViewBody`, `ViewCel` | `zoombinis.h` | an animated thing; see [Game layer](game-layer.md) |
| `Snoid` | `zoombinis.h` | a Zoombini's view body: features, layers, path, action, pose, name |
| `Traveller`, `Party` | `zoombinis.h` | a Zoombini on the journey and the group of them (19 bytes; 0x266 bytes) |
| `CampSlot`, `Camp` | `zoombinis.h` | a stored Zoombini and the camp's 625 slots with their scroll row |
| `FeatureRule`, `FeatureRules` | `zoombinis.h` | a puzzle's rule(s) about features: which side, how many features, which values (bridge, tunnels) |
| `ImageBank` | `zoombinis.h` | images in one block: count and each one's offset (from 1) |
| `InputItem`, `Group`, `GroupList`, `InputHandlers` | `zoombinis.h` | the focus system's items, groups and callbacks |
| `SceneButton` | `zoombinis.h` | a button's rectangle plus 0x1c bytes of focus-system fields |
| `SavedGame`, `SavedGameList` | `zoombinis.h` | the saved-game list's entries |
| `SoundEntry`, `SoundChannel`, `SoundChannels` | `zoombinis.h` | loaded sounds, the four channels per type, and the sounds views asked for in an update |
| `Wipe`, `Blinds` | `zoombinis.h` | screen transitions in progress |
| `DisplayMode`, `MemoryInfo` | `zoombinis.h` | what `WinMain` asks of the display and learns of memory |
| `Rect`, `ShortRect`, `Point`, `Color` | `zoombinis.h` | the engine's QuickDraw-style value types, as the game's calls show them |

## Ownership and lifetime

- A **scene** creates its views, groups and resources in `open…` and destroys them in `close…`. `clearViews`/`removeDeadViews`/`closeViews` free views; `freeScripts`, `freeFeatureGroups`, `unloadSounds` free the rest. Scenes keep **view ids** in globals (not pointers) and look them up with `findView`, so a view can be deleted underneath them safely.
- A **Snoid** is part of its view: `viewSnoid(view)` casts the view's body, `snoidView(snoid)` goes back (`-0x30`).
- **Resources** are loaded through the engine's resource manager as handles that the game locks while in use and the engine may purge; archives are opened per scene (`openGameFile(&hotelFile, "Hotel.MHK")`) and closed when it closes.
- A **party** is value data in `gameState`; scenes copy it into travellers and write back.

## Pairs and groups of views

Several puzzles move views together: `groupViews(a…f)` sets a shared `group` (1-16) so a script's motion applies to all, `pairViews(a, b)` links two. The camp's and the book's second input groups hold `campAreaItems` and `bookAreaItems`, the areas Zoombinis are dragged onto.
